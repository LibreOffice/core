/***************************************************************************
    utils.hxx  -  Utility functions for EQC - header file
                             -------------------
    begin                : Mon Oct 22 2001
    copyright            : (C) 2001 by Jan Rheinlaender
    email                : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UTILS_H
#define UTILS_H

/**
 * @author Jan Rheinlaender
 * @short Some utitlity functions for eqc
 */

#include <set>
#include <string>
#include <stdexcept>
#ifndef _MSC_VER
  #include <backward/hash_fun.h>
#endif
#include "expression.hxx"
#ifdef _MSC_VER
  #include <ginac/crc32.h>
#endif

// make_unique that was forgotten in C++11 standard
// see https://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
// MSVC has std::make_unique() at least since version 19 (from VS 2017)
#if !defined _MSC_VER && __cplusplus < 201402L
namespace std {
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}
}
#endif

namespace GiNaC {
  extern const ex _ex0;
  extern const expression _expr0;
  extern const ex _ex1_4;
  extern const expression _expr1_4;
  extern const numeric* _num1_3_p;
  extern const ex _ex1_3;
  extern const expression _expr1_3;
  extern const numeric* _num1_2_p;
  extern const ex _ex1_2;
  extern const expression _expr1_2;
  extern const numeric* _num1_p;
  extern const ex _ex1;
  extern const expression _expr1;
  extern const numeric* _num2_p;
  extern const ex _ex2;
  extern const expression _expr2;
  extern const numeric* _num3_p;
  extern const ex _ex3;
  extern const expression _expr3;
  extern const ex _ex4;
  extern const expression _expr4;
  extern const numeric* _num_1_2_p;
  extern const numeric* _num_1_p;
  extern const ex _ex_1;
  extern const expression _expr_1;
  extern const ex _ex_1_2;
  extern const expression _expr_1_2;
  extern const expression _exprPi;
  extern const constant Euler_number;
  extern const expression _exprEulerNumber;
  extern const expression _exprI;

  // Static initialization for DLLs is not stable, so we do it manually
  void init_utils();

  // Defined in GiNaC crc32.h
  #ifndef _MSC_VER
  extern unsigned crc32(const char* c, const unsigned len, const unsigned crcinit);
  #endif
  // Copied from GiNaC utils.h
  inline unsigned golden_ratio_hash(uintptr_t n) {
    return (unsigned)(n * UINT64_C(0x4f1bbcdd));
  }
  inline unsigned rotate_left(unsigned n) {
    return (n & 0x80000000U) ? (n << 1 | 0x00000001U) : (n << 1);
  }
  // Copied from GiNaC hash_seed.h
  #include <typeinfo>
  #include <cstring>
  #ifndef _MSC_VER
  static inline unsigned make_hash_seed(const std::type_info& tinfo) {
    const void* mangled_name_ptr = (const void*)tinfo.name();
    unsigned v = golden_ratio_hash((uintptr_t)mangled_name_ptr);
    return v;
  }
  #else
  static unsigned make_hash_seed(const std::type_info& tinfo) {
    const char* mangled_name = tinfo.name();
    return crc32(mangled_name, (int)std::strlen(mangled_name), 0);
  }
  #endif

  // Copied from GiNaC 1.7 basic.h
  #if ((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION < 7))
  template<class B, typename... Args>
  inline B & dynallocate(Args &&... args) {
    return const_cast<B &>(static_cast<const B &>((new B(std::forward<Args>(args)...))->setflag(status_flags::dynallocated)));
  }
  #endif
}

/// This structure is necessary to use sets of const char*. It defines the string comparison
struct ltstr {
  /// Compare the strings
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) < 0;
  }
};

//--------------------- Miscellaneous functions -----------------------------------------

/// Return the basis of the power
// !!!This code relies on the canonical ordering not being changed!!!
inline GiNaC::ex get_basis(const GiNaC::power &p) { return (p.op(0)); }

/// Return the exponent of the power
// !!!This code relies on the canonical ordering not being changed!!!
inline GiNaC::ex get_exp(const GiNaC::power &p) { return (p.op(1)); }

/// Checks whether the argument is equal to a given integer within the limits of digits
bool is_equal_int(const GiNaC::numeric &n, const int i, const unsigned digits);

/// Checks whether a matrix is empty
bool is_empty(const GiNaC::matrix& m);

/// Check if the expression is linear in sym. sym can be a symbol or a function
bool is_linear(const GiNaC::ex& e, const GiNaC::ex& sym, GiNaC::ex& factor);

/// Try to guess whether this is a polynomial, and if yes, return the variable. Strict guessing does not allow double occurences of the same degree of the variable.
bool check_polynomial(const GiNaC::ex& e, GiNaC::ex& sym, const bool strict = true);

/**
 * Checks whether a GiNaC expression is a negative power. This is the case if the exponent
 * is a negative expression (@see is_negex())
 * @param p The power to be inspected
 * @returns True if this is a negative power
 */
bool is_negpower(const GiNaC::ex &p);

/**
 * Checks whether a GiNaC expression is negative. This is not as easy as it appears because a simple
 * comparison e < 0 does not work for expressions like -x.
 * @param e The expression to be inspected
 * @returns True if this is a negative expression
 */
bool is_negex(const GiNaC::ex &e);

/**
 * Checks whether an expression is a quantity.
 * @param quantity The expression to inspect
 * @returns True if the expression is a quantity
 */
bool is_quantity (const GiNaC::expression &quantity);

/// Check whether the expression is a symbol or a matrix of symbols
bool is_symbolic (const GiNaC::expression &e);

/// Check whether the expression is a unit or a product of (powers of) units
bool is_unit(const GiNaC::expression &e);

/// Return the expression as a GiNaC lst object
inline GiNaC::lst make_lst(const GiNaC::expression &e) {
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  return (GiNaC::is_a<GiNaC::lst>(e) ? GiNaC::ex_to<GiNaC::lst>(e) : GiNaC::lst{e});
#else
  return (GiNaC::is_a<GiNaC::lst>(e) ? GiNaC::ex_to<GiNaC::lst>(e) : GiNaC::lst(e));
#endif
} // make_lst()

/**
  * Create a list from the matrix.
  * @param e The expression to convert to a list
  * @param force If true, then a column vector will also become a list of elements, otherwise it will
  *        simply become a list with a single element (the vector)
  * @returns The list
  */
GiNaC::lst make_lst_from_matrix(const GiNaC::expression &e, const bool force = false);

/// Return the expression as a GiNaC matrix object
GiNaC::matrix make_matrix(const GiNaC::expression &e);

/// Return the expression as an exvector object
GiNaC::exvector make_exvector(const GiNaC::expression &e);

/// Extract the numeric value from the expression, or throw an exception
GiNaC::numeric get_val_from_ex(const GiNaC::ex &e);

/// Convert a numeric to an unsigned integer (without plausibility testing!)
unsigned numeric_to_uint(const GiNaC::numeric &n);

/// Convert a numeric to a signed integer (without plausibility testing!)
int numeric_to_int(const GiNaC::numeric &n);

std::string extostr(const GiNaC::expression& e);

/**
 * Substitutes all units in the expression with the users preferred units.
 * If this function does not give the
 * desired result, check whether the preferred units should be registered in a different order!
 * @param e The expression in which the units are to be substituted
 * @returns An expression with the result of the substitution
 */
GiNaC::expression subst_units(const GiNaC::expression &e, const GiNaC::unitvec& unitlist);

//--------------------- map functions for simplifications -------------------
/// Expand powers with non-integer exponents and reduce double powers of units
struct expand_real_powers : public GiNaC::map_function {
  /// Map the expression
  GiNaC::ex operator()(const GiNaC::ex &e);
};

/// Reduce double powers (this is unsafe, since sqrt(x^2) = +/- x
struct reduce_double_powers : public GiNaC::map_function {
  /// Map the expression
  GiNaC::ex operator()(const GiNaC::ex &e);
};

/// Gather single square roots in a mul into one root
struct gather_sqrt : public GiNaC::map_function {
  /// Map the expression
  GiNaC::ex operator()(const GiNaC::ex &e);
};

/// Evaluate numerically and return the real nth root instead of the root on the principal branch ( GiNaC gives (-1)^(1/3) = 1/2 + 1/2 * sqrt(3) * I )
// Conditions:
// basis is a negative real number
// exponent is a rational number with denominator > 1
struct evalf_real_roots : public GiNaC::map_function {
  /// Map the expression
  GiNaC::ex operator()(const GiNaC::ex &e);
};

/// Collect matching differentials into exderivative objects, for correct differentiation and printing
struct match_differentials : public GiNaC::map_function {
  GiNaC::ex operator()(const GiNaC::ex& e);
};

// Remove all units from an expression
struct remove_units : public GiNaC::map_function {
  GiNaC::ex operator()(const GiNaC::ex &e);
};

// Remove units from certain functions, e.g. logarithms
struct special_ignore_units : public GiNaC::map_function {
  GiNaC::ex operator()(const GiNaC::ex &e);
};

/**
 * Check if the expression is valid as a modulus. Currently the following modulus are accepted:
 * - A symbol
 * - A positive integer
 * - A complex integer that is not zero
 **/
bool check_modulus(const GiNaC::expression& e);

/// Apply modulus to expression. It is assumed that the modulus is valid
GiNaC::expression apply_modulus(const GiNaC::expression& e, const GiNaC::expression& mod);

// Return the operator as a string
std::string get_oper(const GiNaC::print_context &c, const unsigned o, const GiNaC::expression& mod);

#endif
