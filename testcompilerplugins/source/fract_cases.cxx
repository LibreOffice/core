#include "fract.hxx"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"

void test_param(Fraction f) {
}

void test_param_reference(Fraction& f) {
}

void test_param_constreference(const Fraction& f) {
}

void test_param_pointer(Fraction *f) {
}

void test_param_constpointer(const Fraction *f) {
}

Fraction *test_param_return() {
    return nullptr;
}

void test_declarations() {
    Fraction ctor_without_args;
    Fraction ctor_with_long_arg_imm(1L);
    Fraction ctor_with_long_args_imm(1L, 2L);
    Fraction ctor_with_double_args_imm(1.0);
    double val = 1.0;
    Fraction ctor_with_double_args(val);

    Fraction *qual_pointer;
    const Fraction *qual_const_pointer;
}

void test_method_replace(Fraction f, Fraction g) {
    f.IsValid();
    f.GetNumerator();
    f.GetDenominator();
    // TODO: Fraction::long()
    // TODO: Fraction::double()
    f.ReduceInaccurate(1);
    // TODO: ReadFraction(...)
    // TODO: WriteFraction(...)
}

#pragma clang diagnostic pop
