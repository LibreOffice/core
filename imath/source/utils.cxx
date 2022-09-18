/***************************************************************************
                          utils.cpp  -  Utility functions for EQC
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

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4099)
#endif
#include <cln/cln.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef INSIDE_SM
#include <imath/utils.hxx>
#include <imath/msgdriver.hxx>
#include <imath/equation.hxx>
#include <imath/func.hxx>
#include <imath/unit.hxx>
#include <imath/differential.hxx>
#else
#include "utils.hxx"
#include "msgdriver.hxx"
#include "equation.hxx"
#include "func.hxx"
#include "unit.hxx"
#include "differential.hxx"
#endif
#include "operands.hxx"
#include "exderivative.hxx"

using namespace GiNaC;

namespace GiNaC {
  const expression _expr0;
  const expression _expr1_4;
  const expression _expr1_3;
  const expression _expr1_2;
  const expression _expr1;
  const expression _expr2;
  const expression _expr3;
  const expression _expr4;
  const expression _expr_1;
  const expression _expr_1_2;
  const expression _exprPi;
  const expression _exprEulerNumber;
  const expression _exprI;

  void init_utils() {
    new((void*)&_expr0) expression(_ex0);
    new((void*)&_expr1_4) expression(_ex1_4);
    new((void*)&_expr1_3) expression(_ex1_3);
    new((void*)&_expr1_2) expression(_ex1_2);
    new((void*)&_expr1) expression(_ex1);
    new((void*)&_expr2) expression(_ex2);
    new((void*)&_expr3) expression(_ex3);
    new((void*)&_expr4) expression(_ex4);
    new((void*)&_expr_1) expression(_ex_1);
    new((void*)&_expr_1_2) expression(_ex_1_2);
    new((void*)&_exprPi) expression(Pi);
    new((void*)&_exprEulerNumber) expression(Euler_number);
    new((void*)&_exprI) expression(I);
  }
}

bool is_negex(const ex &e) {
  if (is_a<numeric>(e)) {
    if (e.info(info_flags::real) || imag_part(e).is_zero())
      return real_part(e).info(info_flags::negative); // real number
    else if (real_part(e).is_zero())
      return imag_part(e).info(info_flags::negative); // purely imaginary number
  } else if (is_a<add>(e)) {
    const add& a = ex_to<add>(e);
    for (unsigned t = 0; t < a.nops(); ++t) {
      if (!is_negex(a.op(t)))
        return false;
    }
    return true;
  } else if (is_a<expairseq>(e)) {
    ex coeff = e.op(e.nops());

    if (coeff.info(info_flags::real))
      return coeff.info(info_flags::negative);
    else if (real_part(coeff).is_zero())
      return imag_part(coeff).info(info_flags::negative);
  }

  return false;
} // is_negex()

bool is_negpower(const ex &p) {
  if (!is_a<power>(p)) return false;
  return (is_negex(ex_to<power>(p).op(1)));
} // is_negpower()

bool is_equal_int(const numeric &n, const int i, const unsigned digits) {
  MSG_INFO(5, "is_equal_int: " << n << " == " << i << " to " << digits << endline);
  if (!n.info(info_flags::real)) return false;
  return cln::the<cln::cl_R>(abs(n - i).to_cl_N()) < cln::recip(cln::expt(cln::cl_float(10.0, cln::default_float_format), cln::cl_I(digits)));
}

bool is_empty(const matrix& m) {
  return ((m.rows() == 1) && (m.cols() == 1) && (m(0,0).is_zero()));
}

bool is_linear(const ex& e, const ex& sym, ex& factor) {
  // Check if the expression is linear in sym
  // This can be a symbol or a function
  MSG_INFO(1, "Checking linearity of " << e << " in variable " << sym << endline);

  if (!e.has(sym)) {
    factor = e;
    return true;
  }

  ex arg = e.expand().collect(sym);
  if ((arg.ldegree(sym) != 1) && (arg.degree(sym) != 1)) return false;
  factor = arg.coeff(sym, 1);
  MSG_INFO(1, "Expression is linear, factor: " << factor << endline);
  return true;
}

/// Try to guess whether this is a polynomial, and if yes return the variable
bool check_polynomial(const ex& e, ex& sym, const bool strict) {
  MSG_INFO(3, "Checking if " << e << " is a polynomial" << endline);
  ex expr = e;
  if (!strict) expr = expr.expand(); // Ensure that the expression is expanded, coeff() may not work properly on unexpanded polynomials

  exset powers;
  int occurrences = 0;
  bool double_occurrence = false;
  exvector syms_seen;

  if (expr.find(power(wild(1), wild(2)), powers)) {
    for (const auto& i : powers) {
      MSG_INFO(3, "Found power " << i << endline);
      const power& p = ex_to<power>(i);
      ex s = get_basis(p);

      bool seen = false;
      for (const auto& ss : syms_seen)
        if (ss.is_equal(s)) {
          seen = true;
          break;
      }
      if (seen) continue; // We already looked at this variable
      syms_seen.emplace_back(s);

      if (!(is_a<func>(s) || is_a<function>(s))) { // ? but otherwise polynomials of e.g. tan(x) can be discovered!
        int new_occurrences = 0;

        if (expr.is_polynomial(s) && ((expr.degree(s) > 1) || (expr.ldegree(s) < -1))) {
          MSG_INFO(3, "Polynomial in " << s << ", degrees " << expr.degree(s) << " : " << expr.ldegree(s) << endline);
          for (int ii = expr.ldegree(s); ii <= expr.degree(s); ++ii)
            if (!expr.coeff(s, ii).is_zero() && (ii != 0))
              new_occurrences++;

          if (new_occurrences > occurrences) {
            // Found other variable with more occurrences
            occurrences = new_occurrences;
            sym = s;
            double_occurrence = false;
          } else if (strict && (new_occurrences == occurrences)) {
            // This check is important for usage in printing
            // If we have a x^2 + a^2 x + a b x + c then we might randomly print the polynomial in a or in x.
            MSG_INFO(2, "Double occurrence found for different variables" << endline);
            double_occurrence = true;
          }
        }
      }
    }
  }

  if (double_occurrence) return false; // its unclear what the variable should be

  // Do some sanity checks, especially concerning non-expanded polynomials
  if (occurrences <= 1) return false;

  if (strict) {
    // If we have e.g. "a x^2 + b x^2 + c x + d", we want to return false, otherwise we would print "(a + b) x^2 + c x + d"
    // But we should not reformat an expression while printing! This must be done with COLLECT() by the user himself
    ex agg_poly;
    for (int i = expr.ldegree(sym); i<= expr.degree(sym); ++i)
      agg_poly += expr.coeff(sym,i) * power(sym, i);
    if (!agg_poly.is_equal(expr)) {
      MSG_INFO(3, "Rejected positive result because polynomial is not in expanded form" << endline);
      return false;
    }
  }

  return (occurrences > 1);
} // check_polynomial

// Test whether the argument is a quantity (i.e., a numeric multiplied
// with any number of units or powers of units)
bool is_quantity (const expression &quantity) {
  MSG_INFO(2, "Testing expression " << quantity << endline);

  if (is_a<mul>(quantity)) {
    for (const auto& i : quantity) {
      if (is_a<power>(i)) {
        if (!is_a<Unit>(get_basis(ex_to<power>(i)))) return false;
        if (!get_exp(ex_to<power>(i)).info(info_flags::real)) return false;
      } else if (!(is_a<numeric>(i) || is_a<Unit>(i))) {
        return false;
      }
    }
    return true;
  } else if (is_a<matrix>(quantity)) {
    for (unsigned i = 0; i < ex_to<matrix>(quantity).rows(); i++)
      for (unsigned j = 0; j < ex_to<matrix>(quantity).cols(); j++)
        if (!is_quantity(ex_to<matrix>(quantity)(i,j))) return false;
    return true;
  } else if (is_a<power>(quantity)) {
    if (!is_a<Unit>(get_basis(ex_to<power>(quantity)))) return false;
    if (!get_exp(ex_to<power>(quantity)).info(info_flags::real)) return false;
    return true;
  }

  return(is_a<numeric>(quantity) || is_a<Unit>(quantity));
} // is_quantity()

bool is_symbolic (const expression &e) {
  if (is_a<symbol>(e))
    return true;

  if (is_a<matrix>(e)) {
    for (const auto& elem : e)
      if (!is_a<symbol>(elem)) return false;
    return true;
  }

  return false;
}

bool is_unit(const expression &e) {
  MSG_INFO(0, "Checking if " << e << " is an (extended) unit" << endline);
  if (e.is_equal(_expr1) || e.is_equal(_exprPi))
    return true; // Exception for radians etc.

  if (is_a<Unit>(e))
    return true;

  if (is_a<power>(e))
    return (is_a<Unit>(get_basis(ex_to<power>(e))) && get_exp(ex_to<power>(e)).info(info_flags::real));

  if (is_a<mul>(e)) {
    for (const auto& ee : e)
      if (!is_unit(ee) && !is_a<numeric>(ee)) return false;
    return true;
  }

  return false;
}

lst make_lst_from_matrix(const expression &e, const bool force) {
  lst result;
  if (is_a<matrix>(e)) {
    matrix m = ex_to<matrix>(e);
    if (m.rows() > 1) {
      if (force) {
        m = m.transpose();
      } else {
        result.append(m);
        MSG_INFO(1, "Create lst from matrix: argument is multi-row matrix, unchanged" << endline);
        return result;
      }
    }

    for (unsigned i = 0; i < m.cols(); i++)
      result.append(m(0,i));
    MSG_INFO(1, "Create lst from matrix " << m << ", result: " << result << endline);
  } else if (is_a<lst>(e)) {
    result = ex_to<lst>(e);
 } else {
   result.append(e);
 }
 return result;
}

exvector make_exvector(const expression &e) {
  if (is_a<lst>(e)) {
    const lst& l = ex_to<lst>(e);
    return exvector(l.begin(), l.end());
  } else {
    return exvector({e});
  }
} // make_exvector()

matrix make_matrix(const expression &e) {
  if (is_a<matrix>(e)) {
    return ex_to<matrix>(e);
  } else if (is_a<lst>(e)) {
    return matrix (1, (unsigned)ex_to<lst>(e).nops(), ex_to<lst>(e));
  } else {
    lst elements;
    elements.append(e);
    return matrix (1, (unsigned)elements.nops(), elements);
  }
} // make_matrix()

numeric get_val_from_ex(const ex &e) {
  ex v;
  if (!is_a<numeric>(e))
    v = expression(e).evalf();
  else
    v = e;
  if (!is_a<numeric>(v))
    throw std::invalid_argument("Option requires a numeric value");
  return (ex_to<numeric>(v));
} // get_val_from_ex()

unsigned numeric_to_uint(const numeric &n) {
  return cln::cl_I_to_uint(cln::floor1(cln::cl_F(n.to_double())));
}

int numeric_to_int(const numeric &n) {
  return cln::cl_I_to_int(cln::floor1(cln::cl_F(n.to_double())));
}

// Helper function to convert to units that are numerics
struct convert_to_unit : public map_function {
  ex operator()(const ex &e) {
    MSG_INFO(3, "Converting units to " << conversion << " in " << e << endline);

    if (is_a<numeric>(e)) {
      return expression(e) / conversion;
    }

    return e.map(*this);
  }
  expression conversion;
  convert_to_unit(const relational& c) {
    conversion = expression(c.lhs()).evalf() / expression(c.rhs());
  }
};

expression subst_units(const expression &e, const unitvec& unitlist) {
  MSG_INFO(1, "Substituting units in " << e << endline);
  expression result = e;

  // Note that we can NOT substitute all units at once because there might be inter-dependencies
  // between the preferred units (e.g., \mm and N/mm^2)
  // Ginac documentation says all substitutions are done in parallel, but it doesn't work
  for (const auto& i : unitlist) {
    MSG_INFO(2, "Substituting unit " << i << endline);

    if (is_a<constant>(i.lhs()) && e.has(ex_to<constant>(i.lhs()))) {
      // handles Pi = 180\degree
      result = result.subs(i);
    } else if (is_a<numeric>(expression(i.lhs()).evalf())) {  // handles evalf()'ed cases for Pi = 180\degree
      convert_to_unit c_to_unit(i);

      if (is_a<equation>(e)) {
        const equation& eq = ex_to<equation>(e);
        result = equation(c_to_unit(eq.lhs()), c_to_unit(eq.rhs()), eq.getop(), eq.getmod());
      } else {
        result = c_to_unit(result);
      }
    } else {
      result = result.subs(i);
    }
    MSG_INFO(2, "Current result: " << result << endline);
  }

  return result;
}

// Map functions ================

// Helper function for expand_real_powers
// Reduce double powers in units. This can be done safely since a unit is
// always positive (otherwise sqrt(x^2) could be +x or -x)
power check_double_power(const ex &e, const ex &exp) {
  if (is_a<power>(e) && is_a<Unit>(get_basis(ex_to<power>(e))))
    return power(get_basis(ex_to<power>(e)), get_exp(ex_to<power>(e)) * exp);
  else
    return power(e, exp);
} // check_double_power()

ex expand_real_powers::operator()(const ex &e) {
  MSG_INFO(3, "Special expanding powers in " << e << endline);

  if (is_a<power>(e)) {
    expand_real_powers expand_power;
    ex the_basis = expand_power(get_basis(ex_to<power>(e)));
    ex the_exp   = expand_power(get_exp(ex_to<power>(e)));

    if (is_a<mul>(the_basis) && !the_exp.info(info_flags::integer)) {
      expression result(_expr1);
      for (const auto& i : the_basis) {
        result = result * check_double_power(i, the_exp);
      }
      return std::move(result);
    } else if (is_a<power>(the_basis)) {
      return check_double_power(the_basis, the_exp);
    }
  }

  return e.map(*this);
} // expand_real_powers::operator()

ex reduce_double_powers::operator()(const ex &e) {
  MSG_INFO(2, "Reducing double powers in " << e << endline);

  if (is_a<power>(e)) {
    reduce_double_powers reduce_powers;
    ex the_basis = reduce_powers(get_basis(ex_to<power>(e)));
    ex the_exp   = reduce_powers(get_exp(ex_to<power>(e)));
    if (is_a<power>(the_basis)) {
      const power& ebasis = ex_to<power>(the_basis);
      return dynallocate<power>(get_basis(ebasis), get_exp(ebasis) * the_exp);
    } else {
      return dynallocate<power>(the_basis, the_exp);
    }
  } else if (is_a<mul>(e)) {
    ex result(_ex1);
    exhashmap<ex> powers;

    for (const auto& i : e) {
      reduce_double_powers reduce_powers;
      ex the_e = reduce_powers(i);

      if (is_a<power>(the_e)) {
        ex the_basis = get_basis(ex_to<power>(the_e));
        ex the_exp   = get_exp(ex_to<power>(the_e));

        if (powers.find(the_basis) != powers.end())
          powers[the_basis] = powers[the_basis] + the_exp;
        else
          powers[the_basis] = the_exp;
      } else {
        result *= the_e;
      }
    }

    for (const auto& p : powers)
      result *= power(p.first, p.second);

    return result;
  }
  return e.map(*this);
} // reduce_double_powers::operator()

ex gather_sqrt::operator()(const ex &e) {
  MSG_INFO(2, "Gathering square roots in " << e << endline);

  if (is_a<mul>(e)) {
    ex sqrts(_ex1);
    ex non_sqrt(_ex1);
    gather_sqrt gather_sqrts;

    for (const auto& i : e) {
      if (is_a<power>(i) && get_exp(ex_to<power>(i)).is_equal(_ex1_2)) {
        ex the_basis = gather_sqrts(get_basis(ex_to<power>(i)));
        sqrts *= the_basis;
      } else if (is_a<power>(i) && get_exp(ex_to<power>(i)).is_equal(_ex_1_2)) {
        ex the_basis = gather_sqrts(get_basis(ex_to<power>(i)));
        sqrts *= dynallocate<power>(the_basis, _ex_1);
      } else {
        non_sqrt *= gather_sqrts(i);
      }
    }

    return sqrt(sqrts) * non_sqrt;
  }

  return e.map(*this);
} // gather_sqrt::operator()

ex evalf_real_roots::operator()(const ex &e) {
  MSG_INFO(2, "Numerically evaluating nth roots to real values where possible " << e << endline);

  if (is_a<power>(e)) {
    evalf_real_roots eval_roots;
    ex the_basis = eval_roots(get_basis(ex_to<power>(e)));
    ex the_exp   = eval_roots(get_exp(ex_to<power>(e)));

    if (the_exp.info(info_flags::rational)) {
      // Condition 1: exponent is rational
      const numeric& n_exp = ex_to<numeric>(the_exp);
      int denom = n_exp.denom().to_int();

      if ((denom > 1) && (denom % 2 == 1)) {
        // Condition 2: denominator of exponent is odd
        ex e_basis = the_basis.evalf();
        if (e_basis.info(info_flags::negative)) {
          // Condition 3: basis is negative and real
          int numer = n_exp.numer().to_int();
          if (numer % 2 == 0)
            return dynallocate<power>(-the_basis, the_exp); // numerator is even
          else
            return _ex_1 * power(-the_basis, the_exp); // numerator is odd
        }
      }
    }

    return dynallocate<power>(the_basis, the_exp);
  }

  return e.map(*this);
}

/// Structure for storing a complete differential, e.g. d²f / (dx dy)
struct diff_fraction {
  differential numer;
  ex remaining_grade; // Unmatched remaining grade of the numerator
  ex diffpower; // Power of the differential, e.g. (d²f)³ has grade 2 and power 3
  ex denom;
  diff_fraction();
#ifdef DEBUG_CONSTR_DESTR
  diff_fraction(const diff_fraction& other);
  diff_fraction& operator=(const diff_fraction& other);
  ~diff_fraction();
#endif
  void print_imath(const imathprint& c) const;
  ex differentiate() const;
};

typedef std::list<diff_fraction> sorted_differentials;

diff_fraction::diff_fraction() : remaining_grade(_ex0), diffpower(_ex1), denom(_ex1) {
  MSG_INFO(3, "Constructing empty diff_fraction" << endline);
}

#ifdef DEBUG_CONSTR_DESTR
  diff_fraction::diff_fraction(const diff_fraction& other)
      : numer(other.numer), remaining_grade(other.remaining_grade), diffpower(other.diffpower), denom(other.denom) {
    MSG_INFO(3, "Copying diff_fraction from " << other.numer << "/" << other.denom << endline);
  }
  diff_fraction& diff_fraction::operator=(const diff_fraction& other) {
    MSG_INFO(3, "Assigning diff_fraction from " << other.numer << "/" << other.denom << endline);
    numer = other.numer;
    remaining_grade = other.remaining_grade;
    diffpower = other.diffpower;
    denom = other.denom;
    return *this;
  }
  diff_fraction::~diff_fraction() {
    MSG_INFO(3, "Destructing diff_fraction" << endline);
  }
#endif

// Try to match diff in the denominator to one of the entries in the list in the numerator
// The denominator always has grade 1, higher grades have been multiplied into dpower
ex match_differential(sorted_differentials& dlist, const differential& d, const ex& dpower, const bool matchparents) {
  MSG_INFO(2, "Trying to match numerators with denominator " << pow(d, dpower) <<
              " by " << (matchparents ? "parent" : "diffvar") << endline);

  if (matchparents && d.get_parent().is_equal(_ex0))
    return dynallocate<power>(d, dpower); // No parent is known

  ex diffvar = d.argument();
  //if (!is_a<symbol>(diffvar) && !is_a<func>(diffvar)) return d; // Can't match this properly
  ex restpower = dpower;

  // Match the denominator with numerators, until no more powers of the denominator are left
  // Preference is given to a match of the powers, e.g. (d²f)³ matches dx³ leaving one remaining grade of (d²f)³
  for (auto& l : dlist) {
    MSG_INFO(2, "Testing numerator " << pow(l.numer, l.diffpower) << " with remaining grade " << l.remaining_grade << endline);
    if (l.numer.is_partial() != d.is_partial())
      continue; // Type doesn't match
    if (l.remaining_grade.is_zero())
      continue; // Nothing left to match in this numerator

    ex diffarg = l.numer.argument();
    bool match;
    if (matchparents) {
      match = diffarg.is_equal(d.get_parent());
      MSG_INFO(2, "Parent of denominator matches numerator: " << (match ? "true" : "false") << endline);
    } else {
      // TODO: We should give preference to diffargs which contain the diffvar directly, that is
      //  g(x,y) -> matches dx
      // and in a third (!) iteration accept indirect matches
      // but g(x(t), y(t)) -> matches dt
      match = diffarg.has(diffvar);
      MSG_INFO(2, diffarg << " in numerator contains " << diffvar << " of denominator: " << (match ? "true" : "false") << endline);
    }

    if (match) {
      MSG_INFO(2, "Remaining power of denominator: " << restpower << endline);
      if (restpower.is_equal(l.remaining_grade * l.diffpower)) {
        MSG_INFO(2, "Complete match" << endline);
        l.denom *= dynallocate<differential>(diffvar, d.is_partial(), l.remaining_grade, diffarg);
        l.remaining_grade = _ex0;
        return _ex1;
      }

      ex p_quot = restpower / l.diffpower; // How many grades can we consume?

      if (!p_quot.info(info_flags::real)) {
        MSG_INFO(2, "At least one power is non-numeric" << endline);
        ex g_quot = restpower / l.remaining_grade; // How many powers can we consume?
        // Note: The case p_quot == l.remaining_grade and g_quot == l.diffpower has already been handled!

        if ((l.diffpower.info(info_flags::real) && g_quot.info(info_flags::real)) || l.diffpower.has(g_quot)) {
          MSG_INFO(2, "Splitting power of numerator and consuming denominator" << endline);
          dlist.emplace_back(l);
          diff_fraction& remainder = dlist.back();
          remainder.diffpower = l.diffpower - g_quot;
          MSG_INFO(2, "Remaining numerator: " << pow(remainder.numer, remainder.diffpower) << endline);

          l.denom *= dynallocate<differential>(diffvar, d.is_partial(), l.remaining_grade, diffarg);
          l.diffpower = g_quot;
          l.remaining_grade = _ex0;
          MSG_INFO(2, "Matched numerator: " << pow(l.numer/l.denom, l.diffpower) << endline);
          return _ex1; // The denominator was completely matched
        } else {
          MSG_INFO(2, "Grades and powers are non-numeric, not matching" << endline);
          continue;
        }
      }

      if (!l.remaining_grade.info(info_flags::posint)) {
        MSG_INFO(2, "Numerator has non-numeric grade, not matching" << endline);
        continue;
      }

      MSG_INFO(2, "Grades of numerator and denominator are both real" << endline);
      int g = numeric_to_int(ex_to<numeric>(p_quot)); // Find integer part of consumable grades

      if (g >= numeric_to_int(ex_to<numeric>(l.remaining_grade))) {
        MSG_INFO(2, "Consuming all remaining " << l.remaining_grade << " grades of the numerator" << endline);
        l.denom *= dynallocate<differential>(diffvar, d.is_partial(), l.remaining_grade, diffarg);
        restpower -= l.remaining_grade * l.diffpower; // This cannot be zero since that case was already handled!
        l.remaining_grade = _ex0;
      } else if (g > 0) {
        MSG_INFO(2, "Consuming " << g << " grades of the numerator" << endline);
        l.denom *= dynallocate<differential>(diffvar, d.is_partial(), g, diffarg);
        l.remaining_grade -= g;
        restpower -= g * l.diffpower; // This cannot be zero since that case was already handled!
      } // otherwise no match (0 < p_quot < 1)
    }
  }

  return dynallocate<power>(d, restpower);
}

differential extract_differential(const ex& d, ex& diffpower) {
  MSG_INFO(3, "Extracting differential from " << d << endline);
  differential result;

  if (is_a<power>(d)) {
    result = ex_to<differential>(get_basis(ex_to<power>(d)));
    diffpower = get_exp(ex_to<power>(d));
    MSG_INFO(3, "Found differential: " << result << " with power " << diffpower << endline);
  } else if (is_a<differential>(d)) {
    // Note: Differentials of differentials e.g. d(d(x)) cannot occur because they are eliminated in differential::eval()
    result = ex_to<differential>(d);
    diffpower = _ex1;
    MSG_INFO(3, "Found differential: " << result << endline);
  }

  return result;
}

ex match_negdiffs(sorted_differentials& result, const ex& negdiffs, const bool matchparents) {
  if (negdiffs.is_equal(_ex1)) return _ex1;

  ex negrest = _ex1;

  for (const auto& n : (is_a<mul>(negdiffs) ? negdiffs : lst{negdiffs})) {
    ex npow;
    differential ndiff = extract_differential(n, npow);
    negrest = negrest * match_differential(result, ndiff, npow, matchparents);
  }

  return negrest;
}

ex match_differentials::operator()(const ex& e) {
  MSG_INFO(3, "match_differentials() for " << e << endline);
  // TODO: Check for is_a<exderivative>(e) and return straight away?

  if (is_a<mul>(e)) {
    ex mapped_e = e.map(*this); // First handle sub-expressions

    // Collect differentials with positive and negative powers
    ex rest(_ex1);
    ex posdiffs(_ex1);
    ex negdiffs(_ex1);
    for (const auto& m : mapped_e) {
      if (is_a<differential>(m)) {
        posdiffs *= m;
      } else if (is_a<power>(m)) {
        const power& p = ex_to<power>(m);
        if (is_a<differential>(get_basis(p))) {
          if (is_negpower(p)) {
            const differential& d = ex_to<differential>(get_basis(p));
            // Move all grades of negdiffs into the power, so that same differentials will be grouped into a single power
            negdiffs *= dynallocate<power>(dynallocate<differential>(d.argument(), d.is_partial(), 1, d.get_parent()),
                                           d.get_grade() * _ex_1 * get_exp(p));
          } else {
            posdiffs *= m;
          }
        } else {
          rest *= m;
        }
      } else {
        rest *= m;
      }
    }

    MSG_INFO(2, "Sorting differentials: numerator: " << posdiffs << ", denominator: " << negdiffs << endline);
    if (negdiffs.is_equal(_ex1) || posdiffs.is_equal(_ex1))
      return mapped_e;

    sorted_differentials sorteddiffs;

    // First we assume that everything will match
    if (is_a<mul>(posdiffs)) {
      for (const auto& d : posdiffs) {
        sorteddiffs.emplace_back();
        diff_fraction& df = sorteddiffs.back();
        df.numer = extract_differential(d, df.diffpower);
        df.remaining_grade = df.numer.get_grade(); // extract numeric grade for later matching
      }
    } else {
      sorteddiffs.emplace_back();
      diff_fraction& df = sorteddiffs.back();
      df.numer = extract_differential(posdiffs, df.diffpower);
      df.remaining_grade = df.numer.get_grade();
    }

    // Now we try to find the corresponding negdiffs
    ex negrest2 = match_negdiffs(sorteddiffs, negdiffs, true); // Match by parent of the negative differential
    MSG_INFO(2, "Negative differentials left-over after matching by parent: " << negrest2 << endline);
    ex negrest  = match_negdiffs(sorteddiffs, negrest2, false); // Match by diffvar of the negative differential
    MSG_INFO(2, "Negative differentials left-over after matching by diffvar: " << negrest << endline);

    // Now remove incomplete entries
    ex posrest(_ex1);
    ex result(_ex1);

    for (auto& l : sorteddiffs) {
      if (l.denom.is_equal(_ex1)) {
        // No denominator
        l.numer.set_numerator(true);
        posrest = posrest * dynallocate<power>(l.numer, l.diffpower);
      } else if (!l.remaining_grade.is_zero()) {
        // Grades of numerator and denominator are not equal
        l.numer.set_numerator(true);
        posrest = posrest * dynallocate<power>(l.numer, l.diffpower);
        negrest = negrest * dynallocate<power>(l.denom, l.diffpower);
      }  else {
        result *= dynallocate<power>(dynallocate<exderivative>(l.numer, l.denom), l.diffpower);
      }
    }

    MSG_INFO(2, "Matched differentials: " << result << endline);
    MSG_INFO(2, "Negative differentials left-over: " << negrest << endline);
    MSG_INFO(2, "Positive differentials left-over: " << posrest << endline);

    return result * posrest / negrest * rest;
  }

  return e.map(*this);
}

ex remove_units::operator()(const ex &e) {
  if (is_a<Unit>(e))
    return _ex1;

  return e.map(*this);
}

ex special_ignore_units::operator()(const ex &e) {
  if (is_a<func>(e)) {
    const func& f = ex_to<func>(e);

    if (f.get_name() == "log" || f.get_name() == "ln") {
      remove_units remove_u;
      return remove_u(f);
    }
  }

  return e.map(*this);
}

bool check_modulus(const expression& e) {
  MSG_INFO(2, "Checking whether " << e << " is a valid modulus" << endline);
  if (is_a<numeric>(e))
    return e.info(info_flags::posint) ||
           (e.info(info_flags::cinteger) && !e.is_zero());
  else if (is_a<symbol>(e))
    return true;

  return false;
}

expression apply_modulus(const expression& e, const expression& mod) {
  // Note: this only handles powers, products and sums correctly!
  if (mod.is_zero())
    return e;

  MSG_INFO(2, "Applying modulus " << mod << " on " << e << endline);

  if (e.info(info_flags::cinteger) && is_a<numeric>(mod)) {
    if (e.info(info_flags::integer) && mod.info(info_flags::posint))
      return GiNaC::mod(ex_to<numeric>(e), ex_to<numeric>(mod));
    else {
      // This handles only congruence to zero, that is, e is a multiple of the modulus
      numeric factor = ex_to<numeric>(e) / ex_to<numeric>(mod);

      if ((mod.info(info_flags::posint) && factor.info(info_flags::posint)) ||
          (mod.info(info_flags::cinteger) && factor.info(info_flags::cinteger)))
        return _expr0;
      else
        return e;
    }
  } else if (is_a<symbol>(e) && e.is_equal(mod)) {
    return _expr0; // Replace the symbol with 0, since s ~= 0 (mod s)
  } else if (is_a<power>(e)) {
    const power& p = ex_to<power>(e);
    expression applied = apply_modulus(get_basis(p),mod);
    expression exponent = get_exp(p);
    if (applied.is_zero() && is_a<numeric>(exponent) && ex_to<numeric>(exponent).is_negative())
      return e; // avoid division by zero
    else
      return dynallocate<power>(applied, exponent);
  } else if (is_a<mul>(e)) {
    expression result(_expr1);
    for (const auto& f : e) result = result * apply_modulus(f, mod);
    return result;
  } else if (is_a<add>(e)) {
    expression result(_expr0);
    for (const auto& s : e) result = result + apply_modulus(s, mod);
    return result;
  }

  return e;
}

std::string get_oper(const print_context &c, const unsigned o, const expression& mod) {
  if (is_a<imathprint>(c)) {
    switch(o) {
      case relational::equal:            return (mod.is_zero() ? "=" : " EQUIV ");
      case relational::not_equal:        return "<>";
      case relational::less:             return "<";
      case relational::less_or_equal:    return "<=";
      case relational::greater:          return ">";
      case relational::greater_or_equal: return ">=";
      default: return"[invalid operator]";
    }
  } else {
    switch(o) {
      case relational::equal:            return (mod.is_zero() ? "=" : "~=");
      case relational::not_equal:        return "!=";
      case relational::less:             return "<";
      case relational::less_or_equal:    return "<=";
      case relational::greater:          return ">";
      case relational::greater_or_equal: return ">=";
      default: return"[invalid operator]";
    }
  }
}

