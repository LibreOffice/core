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

boost::rational<long> *test_return_pointer() {
    return nullptr;
}

boost::rational<long> test_return() {
    return boost::rational<long>();
}

const boost::rational<long> test_return_const() {
    return boost::rational<long>();
}

const boost::rational<long> *test_return_const_pointer() {
    return nullptr;
}

const boost::rational<long>& test_return_const_reference() {
    static boost::rational<long> inner_value(1, 2);
    return inner_value;
}

void test_declarations() {
    boost::rational<long> ctor_without_args;
    boost::rational<long> ctor_with_long_arg_imm(1L);
    boost::rational<long> ctor_with_long_args_imm(1L, 2L);
    boost::rational<long> ctor_with_double_args_imm(1.0);
    double val = 1.0;
    boost::rational<long> ctor_with_double_args(val);

    boost::rational<long> *qual_pointer;
    const boost::rational<long> *qual_const_pointer;

    const boost::rational<long>& qual_const_reference = test_return_const_reference();

    boost::rational<long> decl_1, decl_2;

    boost::rational<long> mix_decl_11, *mix_decl_12, &mix_decl_13 = decl_1;
    boost::rational<long> *mix_decl_21, &mix_decl_22 = decl_1, mix_decl_23;
    boost::rational<long>&mix_decl_31 = decl_1, mix_decl_32, *mix_decl_33;

    const boost::rational<long> const_mix_decl_1, *const_mix_decl_2, &const_mix_decl_3 = decl_1;
}

void test_method_replace(Fraction f, Fraction g) {
    true /* LOPLUGIN:FRACTREPLACE FIXME: need manual refactor: 'f.IsValid()' always is true when use boost::rational<long> */;
    f.numerator();
    f.denominator();
    // TODO: Fraction::long()
    // TODO: Fraction::double()
    reduceInaccurate(f, 1);
    // TODO: ReadFraction(...)
    // TODO: WriteFraction(...)
}

void test_casts(Fraction f) {
    long casted_long_value;
    casted_long_value = boost::rational_cast<long>(f);
    casted_long_value = boost::rational_cast<long>(f);

    double casted_double_value;
    casted_double_value = boost::rational_cast<double>(f);
    casted_double_value = boost::rational_cast<double>(f);
}

void test_assign_and_create(long &l, const Fraction &f) {
    l = /* LOPLUGIN:FRACTREPLACE check if cast to long exists and remove it */ boost::rational_cast<long>(class Fraction(1, 1) * f); // similar to svx/source/form/fmvwimp.cxx:1453
}

#define INTERNAL_DBG_ASSERT(cond) (cond)
#define INTERNAL_SCALEPOINT(frac) (frac.GetNumerator())


void test_macro_expansion(Fraction f) {
    INTERNAL_DBG_ASSERT( f.GetNumerator() );
    INTERNAL_SCALEPOINT( f );
}

#pragma clang diagnostic pop
