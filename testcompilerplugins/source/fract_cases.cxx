#include <tools/fract.hxx>
#include "fract_cases.hxx"

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

Fraction *test_return_pointer() {
    return nullptr;
}

Fraction test_return() {
    return Fraction();
}

const Fraction test_return_const() {
    return Fraction();
}

const Fraction *test_return_const_pointer() {
    return nullptr;
}

const Fraction &test_return_const_reference() {
    static Fraction inner_value(1, 2);
    return inner_value;
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

    const Fraction& qual_const_reference = test_return_const_reference();

    Fraction decl_1, decl_2;

    Fraction mix_decl_11, *mix_decl_12, &mix_decl_13 = decl_1;
    Fraction *mix_decl_21, &mix_decl_22 = decl_1, mix_decl_23;
    Fraction &mix_decl_31 = decl_1, mix_decl_32, *mix_decl_33;

    const Fraction const_mix_decl_1, *const_mix_decl_2, &const_mix_decl_3 = decl_1;
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

void test_casts(Fraction f) {
    long casted_long_value;
    casted_long_value = (long) f;
    casted_long_value = long(f);

    double casted_double_value;
    casted_double_value = (double) f;
    casted_double_value = double(f);
}

void test_assign_and_create(long &l, const Fraction &f) {
    l = Fraction(1, 1) * f; // similar to svx/source/form/fmvwimp.cxx:1453
}

#define INTERNAL_DBG_ASSERT(cond) (cond)
#define INTERNAL_SCALEPOINT(frac) (frac.GetNumerator())


void test_macro_expansion(Fraction f) {
    INTERNAL_DBG_ASSERT( f.GetNumerator() );
    INTERNAL_SCALEPOINT( f );
}

#pragma clang diagnostic pop
