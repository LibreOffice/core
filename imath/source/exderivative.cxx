/*******************************************************
    exderivative.cxx  -  Class for extending GiNaC to handle derivatives of arbitrary expressions
                             -------------------
    begin                : Mon 24 Apr 2017
    copyright            : (C) 2017 by Jan Rheinlaender
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

#include "exderivative.hxx"
#ifdef INSIDE_SM
#include <imath/msgdriver.hxx>
#include <imath/func.hxx>
#else
#include "msgdriver.hxx"
#include "func.hxx"
#endif

namespace GiNaC {

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(exderivative, basic,
  print_func<print_context>(&exderivative::do_print).
  print_func<imathprint>(&exderivative::do_print_imath));

// Required constructors and destructors and other GiNaC-specific methods
exderivative::exderivative() : numer(_ex0), denom(_ex0) {
  MSG_INFO(3, "Constructing empty exderivative" << endline);
}

exderivative::exderivative(const exderivative& other) : basic(other), numer(other.numer), denom(other.denom) {
  MSG_INFO(3, "Copying exderivative from " << other.numer << " / " << other.denom << endline);
}

exderivative::exderivative(const differential& n, const ex& d) : numer(n), denom(_ex1) {
  MSG_INFO(3, "Constructing exderivative from " << n << " / " << d << endline);

  // Reduces grades of differentials to powers of differentials, e.g. differential(x, 2) -> differential(x)^2
  exhashmap<ex> d_args;
  for (const auto& f : (is_a<mul>(d) ? d : lst{d})) {
    if (is_a<power>(f)) {
        const power& p = ex_to<power>(f);
        if (!is_a<differential>(get_basis(p)))
          throw std::invalid_argument("exderivative: Denominator may only contain differentials and their powers");

        const differential& df = ex_to<differential>(get_basis(p));
        d_args[df.argument()] += df.get_grade() * get_exp(p);
      } else if (is_a<differential>(f)) {
        const differential& df = ex_to<differential>(f);
        d_args[df.argument()] += df.get_grade();
      } else {
        throw std::invalid_argument("exderivative: Denominator may only contain differentials and their powers");
      }
  }

  // Count grade of differentials in the denominator and build the denominator
  ex grade(_ex0);
  for (const auto& f : d_args) {
    denom *= dynallocate<power>(dynallocate<differential>(f.first, numer.is_partial(), 1, numer.argument()), f.second);
    grade += f.second;
  }

  if (!numer.get_grade().is_equal(grade))
    throw std::invalid_argument("exderivative: Grades of numerator and denominator must match");

  numer.set_numerator(true);
}

#ifdef DEBUG_CONSTR_DESTR
exderivative& exderivative::operator=(const exderivative& other) {
  MSG_INFO(3, "Assigning exderivative from " << other  << endline);
  numer = other.numer;
  denom = other.denom;
  return *this;
}
exderivative::~exderivative() {
  MSG_INFO(3, "Destructing exderivative for " << *this << endline);
}
#endif

exderivative_unarchiver::exderivative_unarchiver() {}
exderivative_unarchiver::~exderivative_unarchiver() {}

int exderivative::compare_same_type(const basic &other) const {
  const exderivative &o = static_cast<const exderivative &>(other);
  int compval = numer.compare(o.numer);
  if (compval == 0)
    return denom.compare(o.denom);
  else
    return compval;
}

void exderivative::do_print(const print_context &c, unsigned level) const {
  (void)level;
  c.s << numer << " / {" << denom << "}";
}

// Helper function to extract argument from one factor in the denominator
ex denom_get_argument(const ex& f) {
  if (is_a<power>(f))
    return ex_to<differential>(get_basis(ex_to<power>(f))).argument();
  else
    return ex_to<differential>(f).argument();
}

void exderivative::do_print_imath(const imathprint &c, unsigned level) const {
  MSG_INFO(3, "Printing exderivative: " << numer << " / " << denom << endline);
  (void)level;
  std::string difftype = "dfdt";
  if ((c.poptions->find(o_difftype) != c.poptions->end()) && ((*c.poptions)[o_difftype].value.str != NULL)) difftype = *(*c.poptions)[o_difftype].value.str;

  if (difftype == "dot") {
      if (numer.is_partial()) throw std::logic_error("Diff type 'dot' makes no sense for partial differentials");
      if (is_a<mul>(denom)) throw std::logic_error("Diff type 'dot' is not possible for differentials of more than one variable");
      numer.do_print_imath(c, 0, true, true);
  } else if (difftype == "line") {
    if (is_a<mul>(denom)) throw std::logic_error("Diff type 'line' is not possible for differentials of more than one variable");

    if (numer.is_partial()) {
      ex arg = denom_get_argument(denom);
      if (is_a<symbol>(arg) || (is_a<func>(arg) && ex_to<func>(arg).is_pure()))
        numer.do_print_imath(c, 0, true, arg);
      else
        throw std::logic_error("Warning: Partial differentials with diff type 'line' must differentiate to a symbol or function");
    } else {
      numer.do_print_imath(c, 0, true);
    }
  } else {
    // Everything else is printed in 'dfdt' style
    c.s << "{{alignc ";
    numer.do_print_imath(c, 0, true);
    c.s << "} over {alignc";
    for (const auto& f : (is_a<mul>(denom) ? denom : lst{denom})) {
      // Denom is a mul of different powers of differentials
      c.s << " ";
      if (is_a<power>(f)) {
        const power& p = ex_to<power>(f);
        const differential& df = ex_to<differential>(get_basis(p));
        differential(df.argument(), df.is_partial(), get_exp(p)).do_print_imath(c, 0, true);
      } else {
        ex_to<differential>(f).do_print_imath(c, 0, true);
      }
    }
    c.s << "}}";
  }
}

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
ex exderivative::eval() const {
  MSG_INFO(3, "Eval of exderivative" << endline);
  if (flags & status_flags::evaluated)
    return *this;

  if (is_a<differential>(denom)) {
    const differential& d = ex_to<differential>(denom);
    if (numer.argument().is_equal(d.argument()) && numer.get_grade().is_equal(d.get_grade()))
      return _ex1;
  }

  return this->hold();
}
#else
ex exderivative::eval(int level) const {
  if ((level==1) && (flags & status_flags::evaluated))
    return *this;

  if (level == -max_recursion_level)
    throw(std::runtime_error("max recursion level reached"));

  ex nn = (level==1) ? numer : numer.eval(level-1);
  ex dd = (level==1) ? denom : denom.eval(level-1);

  if (is_a<differential>(dd)) {
    const differential& d = ex_to<differential>(dd);
    if (nn.argument().is_equal(d.argument()) && nn.get_grade().is_equal(d.get_grade())
      return _ex1;
  }

  if (are_ex_trivially_equal(nn,numer) && are_ex_trivially_equal(dd, denom))
    return this->hold();

  return (new exderivative(nn, dd))->setflag(status_flags::dynallocated | status_flags::evaluated);
}
#endif

size_t exderivative::nops() const {
  return 2;
}

ex exderivative::op(size_t i) const {
  if (i == 0) return numer;
  if (i == 1) return denom;
  throw (std::out_of_range("exderivative::op() out of range"));
}

// Required because basic::map() requires let_op() and that is not possible for numer because it is not an ex
ex exderivative::map(map_function & f) const {
  const ex &mapped_n = f(numer);
  const ex &mapped_d = f(denom);

  if (are_ex_trivially_equal(numer, mapped_n) && are_ex_trivially_equal(denom, mapped_d))
    return *this;

  if (!is_a<differential>(mapped_n))
    return mapped_n/mapped_d;
  else
    return dynallocate<exderivative>(ex_to<differential>(mapped_n), mapped_d);
}

ex exderivative::subs(const exmap & m, unsigned options) const {
  MSG_INFO(2, "Substituting exmap " << m << " in " << *this << endline);

  const ex &subsed_n = numer.subs(m, options);
  const ex &subsed_d = denom.subs(m, options);

  if (are_ex_trivially_equal(numer, subsed_n) && are_ex_trivially_equal(denom, subsed_d))
    return subs_one_level(m, options);

  if (!is_a<differential>(subsed_n))
    return subsed_n/subsed_d;
  else
    return exderivative(ex_to<differential>(subsed_n), subsed_d).subs_one_level(m, options);
}

// Exderivatives can only occur in iMath in two cases
// 1. By differentiation a func()
// 2. By matching of user-defined differentials
ex exderivative::derivative(const symbol & s) const {
  MSG_INFO(2, "Derivative of " << *this << " to " << s << endline);
  ex argument = numer.argument();
  if (!argument.has(s)) return _ex0;

  if (is_a<add>(argument)) {
    // Differentiation is additive
    ex result(_ex0);

    for (const auto& a : argument)
      result += exderivative(dynallocate<differential>(a, numer.is_partial(), numer.get_grade()), denom);

    return result.diff(s);
  } else if (is_a<mul>(argument)) {
    // Apply product rule
    exvector addseq;
    addseq.reserve(argument.nops());

    for (const auto& m : argument) {
      ex result(_ex1);
      for (const auto& m2 : argument) {
        if (m == m2)
          result *= exderivative(dynallocate<differential>(m2, numer.is_partial(), numer.get_grade()), denom);
        else
          result *= m;
      }
      addseq.push_back(result);
    }

    return dynallocate<add>(addseq).diff(s);
  } else if (is_a<power>(argument)) {
    // Apply the power rule
    const power& p = ex_to<power>(argument);
    ex result = get_exp(p) * dynallocate<power>(get_basis(p), get_exp(p) - _ex1) *
           exderivative(dynallocate<differential>(get_basis(p), numer.is_partial(), numer.get_grade()), denom);
    return result.diff(s);
  } else if (is_a<func>(argument)) {
    // Chain rule: Treat the exderivative df(x;y)/dx like a function g(x;y) where g=df/dx
    // Compare with func::derivative()
    ex result = _ex0;
    exvector args = ex_to<func>(argument).get_args();

    for (const auto& arg: args) {
      ex arg_diff = expression(arg).diff(s);
      if (!arg_diff.is_zero())
        result += this->pderivative(arg) * arg_diff;
      MSG_INFO(2, "Derivative of " << *this << " to " << arg << ": " << result << endline);
    }

    return result;
  } else {
    // Chain rule: Treat the exderivative as a function of all differentiation variables in the denominator
    // because we do not know what dependant variables are contained in numer.argument()
    ex arg_diff = expression(argument).diff(s);
    if (arg_diff.is_zero()) return _ex0;

    ex result = _ex0;
    for (const auto& f : (is_a<mul>(denom) ? denom : lst{denom}))
        result += this->pderivative(denom_get_argument(f)) * arg_diff;
    return result;
  }
}

ex exderivative::pderivative(const ex& v) const {
  // TODO: Normalize denom, combine grades of same differentials !!!!!!!!!!
  return dynallocate<exderivative>(
    dynallocate<differential>(numer.argument(), numer.is_partial(), numer.get_grade() + 1),
    denom * dynallocate<differential>(v, numer.is_partial(), 1, numer.argument()));
}

ex exderivative::eval_diff() const {
  MSG_INFO(2, "Evaluating exderivative " << *this << endline);
  // This method is used in order to handle the following cases
  // 1. Substitution of a symbolic diff grade with a numeric grade
  // 2. Composure of exderivative objects manually with differential objects (should be avoided wherever possible)

  // TODO: For cases like d(d(f(x,y))) / (dx dy) it is not defined whether differentiation is first to x and then to y or vice versa
  //       According to http://de.wikipedia.org/wiki/Satz_von_Schwarz this doesn't matter in most :-) cases
  ex the_differential = numer / denom; // The not differentiated exderivative, will be returned if differentiating is not possible
  if (numer.get_ngrade() == -1) return the_differential; // non-numeric grades can't be differentiated

  // Extract the differentials we want to differentiate to
  std::vector<differential> vars;

  for (const auto& f: (is_a<mul>(denom) ? denom : lst{denom})) {
    if (is_a<power>(f)) {
      const power& p = ex_to<power>(f);
      if (!get_exp(p).info(info_flags::posint)) return the_differential;
      vars.emplace_back(ex_to<differential>(get_basis(p)));
      differential& df = vars.back();
      df.set_grade(get_exp(p));
    } else {
      vars.emplace_back(ex_to<differential>(f)); // Must have grade 1
    }
  }

  // Differentiate to all in turn
  bool partial = numer.is_partial(); // We rely on numer and denom being consistent
  ex result = numer.argument();

  for (const auto& v : vars) {
    ex diffarg = v.argument();
    ex grade = v.get_grade();
    MSG_INFO(2, "Differentiating " << result << " to " << diffarg << " grade " << grade << endline);

    if (is_a<symbol>(diffarg) || is_a<func>(diffarg)) {
      if (partial)
        result = expression(result).pdiff(diffarg, grade);
      else
        result = expression(result).diff(diffarg, grade);
    } else {
      result = dynallocate<exderivative>(dynallocate<differential>(result, partial, grade), v);
    }

    MSG_INFO(2, "Result after differentiating " << grade << " times to " << diffarg << ": " << result << endline);
  }

  return result;
}

}
