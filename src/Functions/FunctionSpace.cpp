#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <Functions/IFunction.h>
#include <Functions/FunctionFactory.h>
#include <Functions/FunctionHelpers.h>
#include <Core/Field.h>
#include <Columns/ColumnsNumber.h>
#include <Columns/ColumnString.h>
#include <Interpreters/castColumn.h>


namespace DB {

namespace ErrorCodes
{
    extern const int ILLEGAL_COLUMN;
    extern const int ILLEGAL_TYPE_OF_ARGUMENT;
    extern const int NUMBER_OF_ARGUMENTS_DOESNT_MATCH;
}


class FunctionSpace : public IFunction {
public:
    static constexpr auto name = "space";
    static FunctionPtr create(ContextPtr) {return std::make_shared<FunctionSpace>(); }
    String getName() const override { return name; }
    bool useDefaultImplementationForConstants() const override { return true; }
    bool isSuitableForShortCircuitArgumentsExecution(const DataTypesWithConstInfo &) const override { return false; }
    size_t getNumberOfArguments() const override { return 1; }
    DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override 
    {
        if (arguments.size() != 1)
            throw Exception("Number of arguments for function " + getName() + " doesn't match: passed "
                + toString(arguments.size()) + ", should be at least 1.",
                ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH);

        if (!isInteger(arguments[0]) && !isFloat(arguments[0]) && !isString(arguments[0]))
                throw Exception("Illegal type " + arguments[0]->getName() + " of argument of function " + getName()
                                + ", must be Integer",
                                ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);

        return std::make_shared<DataTypeString>(); 
    }
    ColumnPtr executeImpl(const ColumnsWithTypeAndName & arguments, const DataTypePtr &, size_t input_rows_count) const override
    {        
        auto string_type = std::make_shared<DataTypeString>();
        auto casted_column = castColumn(std::move(arguments[0]), string_type);
        
        const ColumnString * col = checkAndGetColumn<ColumnString>(casted_column.get()); 

        auto result_column = ColumnString::create();

        const ColumnString::Chars & vec_src = col-> getChars();
        const ColumnString::Offsets & offsets_src = col-> getOffsets();
        size_t prev_offset = 0;

        for (size_t i = 0; i < input_rows_count; ++i)
        {
            uint32_t res = 0;
            const uint8_t start = '0';
            for (size_t j = prev_offset; j < vec_src.size(); j++) {
                if (vec_src[j] >= '0' && vec_src[j] <= '9') {
                    res = res * 10 + *reinterpret_cast<const uint8_t*>(&vec_src[j]) - start;
                } else {
                    break;
                }
            }
            std::string ans;
            ans.resize(res, ' ');
            result_column->insertData(ans.data(), ans.size());
            prev_offset = offsets_src[i];
        }
        return result_column;
    }
};

void registerFunctionSpace(FunctionFactory & factory) {
    factory.registerFunction<FunctionSpace>();   
}

}
