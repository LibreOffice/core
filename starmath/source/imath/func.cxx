/*************************************************************************
    func.cpp  -  class func, extending the GiNaC function class
                             -------------------
    begin                : Fri Feb 13 2004
    copyright            : (C) 2004 by Jan Rheinlaender
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

#ifdef HAVE_CONFIG_H
 #include "../config/config.h"
#endif
#include <cmath>
#include "func.hxx"
#include "printing.hxx"
#include "msgdriver.hxx"
#include "equation.hxx"
#include "utils.hxx"
#include "extintegral.hxx"
#include "exderivative.hxx"
#include "differential.hxx"
#include "operands.hxx"
#include "stringex.hxx"
#include "unit.hxx"
#include <sstream>

namespace GiNaC {

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(func, exprseq, print_func<print_context>(&func::do_print).print_func<imathprint>(&func::do_print_imath));

// Define static members------------------------------------------------------------------
  std::map<const std::string, funcrec> func::functions;
  std::map<const std::string, std::string> func::hard_names;
  std::map<const std::string, std::string> func::hard_names_rev;
  std::map<std::string, std::string> func::func_inv;

// Required constructors and destructors and other GiNaC-specific methods-----------------
// Default constructor etc.
func::func() : name("") {
  MSG_INFO(3, "Constructing empty func" << endline);
}

#ifdef DEBUG_CONSTR_DESTR
  func::func(const func& other) : exprseq(other.seq), name(other.name), serial(other.serial), vars(other.vars), definition(other.definition), hard(other.hard), hints(other.hints) {
    MSG_INFO(3, "Copying func from " << other.name << endline);
  }
  func& func::operator=(const func& other) {
    MSG_INFO(3, "Assigning func from " << other.name << endline);
    seq = other.seq;
    name = other.name;
    serial = other.serial;
    vars = other.vars;
    definition = other.definition;
    hard = other.hard;
    hints = other.hints;
    return *this;
  }
  func::~func() {
    MSG_INFO(3, "Destructing func " << name << endline);
  }
#endif

void func::initAttributes() {
  MSG_INFO(3, "Initializing arguments of " << name << endline);
  funcrec_cit frec = functions.find(name);
  if (frec == functions.end()) throw std::runtime_error("Function " + name + " has not been registered!");
  serial = frec->second.serial;
  vars = frec->second.vars;
  definition = frec->second.definition;
  hard = frec->second.hard;
  hints = frec->second.hints;
  printname = frec->second.printname;

  MSG_INFO(3, "Checking variables" << endline);
  if (!seq.empty() && (seq.size() != vars.size())) {
    MSG_WARN(0, "Warning: Number of arguments does not match for " << name
     << ". Expected " << (long)vars.size() << " arguments, found " << (long)seq.size()
      << " arguments. Using default arguments instead." << endline);
    seq = vars;
  }
}

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
func::func(const std::string &n, const exvector &args) : exprseq(args), name(n) {
#else
func::func(const std::string &n, const exvector &args, bool discardable)
    : exprseq(args, discardable), name(n) {
#endif
  MSG_INFO(3, "Constructing func from exvector with name " << n << " and arguments " << seq << endline);
  initAttributes();
}
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
func::func(const std::string &n, exvector &&args) : exprseq(std::move(args)), name(n) {
#else
func::func(const std::string &n, exvector &&args, bool discardable) : exprseq(std::move(args), discardable), name(n) {
#endif
  MSG_INFO(3, "Moving func from exvector with name " << n << " and arguments " << seq << endline);
  initAttributes();
}

func::func(const std::string &n, const exprseq &args) : exprseq(args), name(n) {
  MSG_INFO(3, "Constructing func from exprseq with name " << n << " and arguments " << seq << endline);
  initAttributes();
}
func::func(const std::string &n, exprseq &&args) : exprseq(std::move(args)), name(n) {
  MSG_INFO(3, "Moving func from exprseq with name " << n << " and arguments " << seq << endline);
  initAttributes();
}

func::func(const std::string &n, const expression &e) : exprseq({e}), name(n) {
  MSG_INFO(3, "Constructing func with name " << n << " and argument " << e << endline);
  if (is_a<function>(e)) throw std::runtime_error("Invalid function argument to func::func()");
  initAttributes();
}

void func::printseq(const print_context & c, const std::string &openbracket, const std::string& delim,
      const std::string &closebracket) const {
  MSG_INFO(5, "func::printseq() with print_context called" << endline);
  c.s << openbracket;

  if (!seq.empty()) {
    exvector::const_iterator it = seq.begin();
    while (it != seq.end()) {
      it->print(c, 0);
      if (it < seq.end() - 1) c.s << delim;
      ++it;
    }
  }

  c.s << closebracket;
}

void func::do_print(const print_context &c, unsigned level) const {
  (void)level;
  c.s << name;
  if (seq.empty()) c.s << vars;
  exprseq::printseq(c, '(', ',', ')', exprseq::precedence(), func::precedence());
}

void func::do_print_imath(const imathprint &c, unsigned level) const {
  print_imath(c, 1, level);
}

void func::print_imath(const imathprint&c, const ex& p, unsigned level) const {
  MSG_INFO(2, "func::print_imath() called for " << name << endline);

  // Set the argument delimiter. The spaces are required because otherwise in (x_1, x_2) the comma moves into the subscript
  // Note that this depends on the locale being set to "C" (default)
  std::string delim = " , ";
  if (imathprint::decimalpoint != ".")
    delim = " \%fc ";

  if (hard) {
    if ((hints & FUNCHINT_PRINT) && !seq.empty()) {
      // Case 1: Hardcoded functions that come with GiNaC and should use their own print function)
      // Case 2: Hardcoded functions that come with iMath (always use their own print function)
      MSG_INFO(3, "Printing hardcoded function through its own print function" << endline);
      function(serial, seq).print(c, level+1); // Fall through to the hard-coded print function
      return;
    } else {
      // Case 3: Hardcoded functions that come with GiNaC and require special handling
      MSG_INFO(3, "Printing hardcoded function with special handling" << endline);
      // TODO: Is it possible to register a print context handler on functions?
      if (name == "fact") {
        c.s << "fact ";
        if (seq.empty()) { // Avoid crash
          c.s << "()";
          return;
        }
        if (is_a<numeric>(op(0)) || is_a<symbol>(op(0)) || is_a<power>(op(0)) ||
            is_a<Unit>(op(0)) || (is_a<func>(op(0)) && !ex_to<func>(op(0)).is_lib())) // do not print brackets
          seq[0].print(c);
        else
          printseq(c, "(", delim, ")");
        return;
      } else if (name == "conjugate") {
        // The hardcoded function prints conjugate() which is not nice
        c.s << "overline";
        printseq(c, "{", delim, "}");
        return;
      } else if (name == "ln") {
        // Note: The hardcoded print function for ln prints log!!
        c.s << "ln";
        printseq(c, "(", delim, ")");
        return;
      }
      // All other hardcoded functions are handled like soft-coded functions
    }
  }

  // Case 4: All other hardcoded functions and user-defined functions
  MSG_INFO(3, "Printing user-defined function or other hardcoded function" << endline);
  if (p != 1) { // Print the exponent of a trigonometric function directly after the function name
    c.s << printname << "^";
    p.print(c, level+1);
    if (seq.empty()) return;
  } else {
    if (!hard && is_lib() && name[0] != '%') // func does not work in combination with e.g. %alpha
      c.s << "func "; // so that sec() etc. are not printed in italics like user-defined functions
    c.s << printname;
    if (seq.empty()) return;
  }

  if (is_nobracket()) { // do not print brackets
    printseq(c, "{", delim, "}");
  } else if (is_trig() && (seq.size() == 1) &&
             (is_a<numeric>(seq[0]) || is_a<symbol>(seq[0]) || is_a<power>(seq[0]) || is_a<Unit>(seq[0]) || (is_a<func>(seq[0]) && !ex_to<func>(seq[0]).is_lib()))) {
    c.s << "{";
    seq[0].print(c); // do not print brackets for trigonometric functions of a simple argument
    c.s << "}";
  } else {
    printseq(c, "(", delim, ")");
  }
}

void func::print_diff_line(const ex& g, const int gr, const print_context& c) const {
  // TODO: All this is mostly copied from func::print()
  if (hard && (hints & FUNCHINT_PRINT))
    MSG_WARN(0, "Warning: Cannot use hardcoded print function for difftype line" << endline);

  // Concatenate the lines
  std::string lines = "";
  std::string diffchar = "%d1"; // Special character from Math catalog 'iMath', see iMathFormatting.xcu
  if (gr > 0)
    for (int i = 0; i < gr; i++) lines += diffchar;
  lines = "^{" + lines + "}";

  // Set the argument delimiter
  std::string delim = " , ";
  if (imathprint::decimalpoint != ".")
    delim = " \%fc ";

  std::string fname = printname;
  if (!hard && is_lib())
    fname = "func " + fname; // so that sec() etc. are not printed in italics like user-defined functions

  if (!is_lib() || seq.empty()) {
    // Print e.g. as f'(x)
    c.s << fname;
    if ((gr > 0) && (gr < 4))
      c.s << lines;
    else
      c.s << "^(" << g << ")";
  } else
    // Print e.g. as (sin x)'
    c.s << "(" << fname;

  if (seq.empty()) return;

  if (is_nobracket()) { // do not print brackets
    printseq(c, "{", delim, "}");
  } else if (is_trig() && (seq.size() == 1) &&
             (is_a<numeric>(seq[0]) || is_a<symbol>(seq[0]) || is_a<power>(seq[0]) || is_a<Unit>(seq[0]) || (is_a<func>(seq[0]) && !ex_to<func>(seq[0]).is_lib()))) {
    c.s << "{";
    seq[0].print(c); // do not print brackets for trigonometric functions of a simple argument
    c.s << "}";
  } else {
    printseq(c, "(", delim, ")");
  }

  if (is_lib()) {
    c.s << ")";
    if ((gr > 0) && (gr < 4))
      c.s << lines;
    else
      c.s << "^(" << g << ")";
  }
}

expression func::expand_definition() const {
  if (definition.is_empty() && !hard)
    throw(std::logic_error("Warning: Function " + name + " has no definition! Not expanded."));

  if (seq.empty()) { // There is nothing to expand
    if (hard)
      return this->setflag(status_flags::expanded);
    else
      return definition;
  }

  if (hard)
    return this->setflag(status_flags::expanded);

  // Substitute the arguments in the definition
  exmap subst;
  for (unsigned i = 0; i < seq.size(); i++)
    subst.emplace(vars[i], seq[i]);
  expression result = definition.subs(subst);
  MSG_INFO(2, "Expanding user function " << name << "(" << subst << ") = "
                     << definition << endline
                     << "Expansion result: " << result << endline);
  return(result);
}

ex func::expand(unsigned options) const {
  MSG_INFO(2, "Expanding " << name << endline);
  func result;

  if (options & expand_options::expand_function_args) { // Only expand arguments when asked to do so
    if (name == "mindex") { // Special treatment, because we don't want the indices to became floats...
      MSG_INFO(3,  "Expanding mindex: " << *this << endline);
      result = *this;
    } else {
      result = ex_to<func>(inherited::expand(options));
    }
  } else {
    result = *this;
  }

  if (!definition.is_empty() && !(options & no_expand_function_definition)) { // is there a function definition that can be expanded?
    try {
      return (result.expand_definition());//TODO: should we do another expand() on this result,
        // since the definition itself has not been expanded yet?
        // Should this depend on expand_options::expand_function_args ?
    } catch (std::runtime_error &e) {
      MSG_ERROR(0, e.what() << endline);
    } catch (std::logic_error &e) {
      MSG_ERROR(0, e.what() << endline);
    }
    return result;
  } else {
    return (options == 0) ? result.setflag(status_flags::expanded) : ex(result);
  }
} // func::expand()

ex func::conjugate() const {
  if (hard) { // Fall through to GiNaC::function::conjugate()
    ex result = function(serial, seq).setflag(status_flags::evaluated).conjugate(); // Note: Omitting setting the flag results in an infinite loop!
    // TODO: Compare if something changed, e.g. if (are_ex_trivially_equal(funct, result)) return *this;
    replace_function_by_func replace_functions; // get rid of GiNaC functions that might have been introduced
    return replace_functions(result).eval();
  }

  return dynallocate<func>("conjugate", ex(*this)).hold();
}

ex func::real_part() const {
  if (hard) { // Fall through to GiNaC::function::real_part()
    ex result = function(serial, seq).setflag(status_flags::evaluated).real_part(); // Note: Omitting setting the flag results in an infinite loop!
    replace_function_by_func replace_functions; // get rid of GiNaC functions that might have been introduced
    return replace_functions(result).eval();
  }

  return dynallocate<func>("Re", ex(*this)).hold();
}

ex func::imag_part() const {
  if (hard) { // Fall through to GiNaC::function::imag_part()
    ex result = function(serial, seq).setflag(status_flags::evaluated).imag_part(); // Note: Omitting setting the flag results in an infinite loop!
    replace_function_by_func replace_functions; // get rid of GiNaC functions that might have been introduced
    return replace_functions(result).eval();
  }

  return dynallocate<func>("Im", ex(*this)).hold();
}

bool func::has(const ex & other, unsigned options) const {
  MSG_INFO(3, "Checking if " << *this << " has " << other << endline);
  if (is_a<func>(other) && this->is_equal_same_type(ex_to<func>(other)))
    return true;
  else if (seq.empty()) {
    for (const auto& v : vars) {
      MSG_INFO(3, "Checking if " << v << " has " << other << endline);
      if (v.has(other, options)) return true;
    }
    return false;
  } else
    return basic::has(other, options);
}

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
 ex func::eval() const {
#else
ex func::eval(int level) const {
#endif
  // If the function is hardcoded, drop through to the GiNaC::function::eval() method
  MSG_INFO(3, "Doing eval of " << name << "(" << seq << ")" << endline);
  if (hard) {
    MSG_INFO(3, "Function is hard-coded with serial " << serial << endline);
    if (seq.size() == 1) {
      if (is_exactly_a<func>(seq[0])) {
        const func& f0 = ex_to<func>(seq[0]);

        if (f0.hard) {
          // Take advantage of the hard-coded GiNac eval rules, e.g. tan(atan(x)) = x
          ex func_arg = function(f0.serial, f0.seq);
  #if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
          ex result = function(serial, func_arg).eval();
  #else
          ex result = function(serial, func_arg).eval(level);
  #endif
  #ifndef _MSC_VER
          // Note: MSVC does not guarantee the order of initialization so this test sometimes fails
          if (serial >= round_SERIAL::serial) return result;
  #endif
          if (is_a<function>(result) && ex_to<function>(result).get_serial() == serial)
            return this->hold(); // Nothing appears to have happened

          // Don't introduce any GiNaC::function into the system!
          replace_function_by_func replace_functions;
          return replace_functions(result);
        }
      } else if (name =="ln") {
        if (is_a<constant>(seq[0]) && seq[0].is_equal(Euler_number)) { // handle ln e = 1
          return _ex1;
        } else if (is_a<power>(seq[0])) {
          const power& p = ex_to<power>(seq[0]);
          if (get_basis(p).is_equal(Euler_number)) // handle ln e^x = x
            return get_exp(p);
        }
      }
    }
    if (!seq.empty()) {
      MSG_INFO(3, "Drop through to GinaC eval rules" << endline);
      // Take advantage of the hard-coded GiNaC eval rules, e.g. sin(-2) = -sin(2)
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
      ex result = function(serial, seq).eval();
#else
      ex result = function(serial, seq).eval(level);
#endif
#ifndef _MSC_VER
      if (serial >= round_SERIAL::serial) return result;
#endif
      replace_function_by_func replace_functions;
      return replace_functions(result);
    } else {
      return (this->hold());
    }
  }

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  // Apparently this is not necessary any more
  // for (auto & it : result.seq) it = it.eval();
  if (is_expand() && !definition.is_empty())
    return this->expand_definition(); // no full expansion! Just the function, not the arguments
  else
    return this->hold();
#else
  func result = *this;
  if (level > 1) result.seq = evalchildren(level);
  if (is_expand() & !definition.is_empty())
    return result.expand_definition(); // no full expansion! Just the function, not the arguments
  else
    return result->hold();
#endif
}

ex func::subs(const exmap & m, unsigned options) const {
  MSG_INFO(2, "Substituting exmap " << m << " in " << *this << endline);

  if (!seq.empty()) {
    return exprseq::subs(m, options);
  } else {
    // Substitute into the vars. Required for exderivate::derivative() of a pure function
    exvector newvars;
    bool changed = false;

    for (auto& v : vars) {
      newvars.push_back(v.subs(m, options));
      if (!are_ex_trivially_equal(v, newvars.back())) changed = true;
    }

    if (changed) {
      // We must only return a new object if something has actually changed, otherwise cancelling in muls will fail
      func f(*this);
      f.vars = newvars;
      return f.subs_one_level(m, options);
    } else {
      return subs_one_level(m, options);
    }
  }
}

ex func::map(map_function& f) const {
  MSG_INFO(2, "Func: Mapping " << *this << endline);

  if (!seq.empty()) {
    // Avoid infinite loop that might occur when simply returning exprseq::map(f). This code duplicates basic::map()
    // The loop occurs e.g. when constructing conjugate(f(x)) where conjugate() is hard-coded and f() is an iMath function
    func* copy = nullptr;
    for (size_t i = 0; i < seq.size(); i++) {
      const ex& o = seq[i];
      const ex& n = f(o);
      if (!are_ex_trivially_equal(o, n)) {
        if (copy == nullptr)
          copy = duplicate();
        copy->seq[i] = n;
      }
    }

    if (copy) {
      copy->clearflag(status_flags::hash_calculated | status_flags::expanded);
      return *copy;
    } else
      return *this;
  } else {
    // Map into the vars. Required for partial derivatives of a pure function in expression::pdiff()
    exvector newvars;
    bool changed = false;

    for (const auto& v : vars) {
      newvars.push_back(f(v));
      if (!are_ex_trivially_equal(v, newvars.back())) changed = true;
    }

    if (changed) {
      // We only return a new object if something has actually changed
      func newf(*this);
      newf.vars = newvars;
      return newf;
    } else {
      return *this;
    }
  }
}

bool func::find_integral(const ex& sym, ex& result) const {
  MSG_INFO(1, "Finding integral for " << *this << endline);
  if (seq.size() > 1) return false;
  if (seq.empty()) return false;

  // Check if the argument of the function is linear in the symbol
  ex factor;
  if (!is_linear(seq[0], sym, factor))
    return false;

  MSG_INFO(4, "Function name: '" << name << "'" << endline);
  if (name == "cos")
    result = func("sin", seq[0]) / factor;
  else if (name == "sin")
    result = _ex_1 * func("cos", seq[0]) / factor;
  else if (name == "tan")
    result = _ex_1 * func("ln", ex(func("abs", ex(func("cos", seq[0]))))) / factor;
  else if (name == "cot")
    result = func("ln", ex(func("abs", ex(func("sin", seq[0]))))) / factor;
  else if (name == "arcsin")
    result = (seq[0] * func("arcsin", seq[0]) + pow(_ex1 - pow(seq[0], _ex2), _ex1_2)) / factor;
  else if (name == "arccos")
    result = (seq[0] * func("arccos", seq[0]) - pow(_ex1 - pow(seq[0], _ex2), _ex1_2)) / factor;
  else if (name == "arctan")
    result = (seq[0] * func("arctan", seq[0]) - _ex1_2 * func("ln", _ex1 + pow(seq[0], _ex2))) / factor;
  else if (name == "arccot")
    result = (seq[0] * func("arccot", seq[0]) + _ex1_2 * func("ln", _ex1 + pow(seq[0], _ex2))) / factor;
  else if (name == "cosh")
    result = func("sinh", seq[0]) / factor;
  else if (name == "sinh")
    result = func("cosh", seq[0]) / factor;
  else if (name == "tanh")
    result = func("ln", ex(func("cosh", seq[0]))) / factor;
  else if (name == "coth")
    result = func("ln", ex(func("abs", ex(func("sinh", seq[0]))))) / factor;
  else if (name == "arsinh")
    result = (seq[0] * func("arsinh", seq[0]) - pow(_ex1 + pow(seq[0], _ex2), _ex1_2)) / factor;
  else if (name == "arcosh")
    result = (seq[0] * func("arcosh", seq[0]) - pow(pow(seq[0], _ex2) - _ex1, _ex1_2)) / factor;
  else if (name == "artanh")
    result = (seq[0] * func("artanh", seq[0]) + _ex1_2 * func("ln", _ex1 - pow(seq[0], _ex2))) / factor;
  else if (name == "arcoth")
    result = (seq[0] * func("arcoth", seq[0]) + _ex1_2 * func("ln", pow(seq[0], _ex2) - _ex1)) / factor;
  else if (name == "exp")
    result = func("exp", seq[0]) / factor;
  else if (name == "log")
    result = _ex1 / (factor * func("ln", numeric(10))) * (seq[0] * func("ln", seq[0]) - seq[0]);
  else if (name == "ln")
    result = (seq[0] * func("ln", seq[0]) - seq[0]) / factor;
  else
    return false;

  return true;
}

ex func::reduce_double_funcs::operator()(const ex &e) {
  MSG_INFO(2, "Reducing pairs of function/inverse function in " << e << endline);

  if (is_a<function>(e)) {
    const function& f = ex_to<function>(e);
    MSG_INFO(3,  "Found G-function " << f.get_name() << ". This should not happen!!!!" << endline);
  }
  if (is_a<func>(e)) {
    const func& f = ex_to<func>(e);

    if (f.seq.size() == 0) return e; // Nothing to reduce
    ex me = e.map(*this); // map arguments first
    if (f.get_numargs() > 1) return me; // cannot work on functions with multiple arguments
    if (!is_a<func>(me)) return me; // Something was contracted
    const func& mf = ex_to<func>(me);

    if (is_a<func>(mf.seq[0])) { // The argument is a function
      const func& argf = ex_to<func>(mf.seq[0]);
      std::string fnamebare = mf.name;
      std::string argfnamebare = argf.name;

      if (func_inv[fnamebare] == argfnamebare) {
        if (argf.seq.size() > 1) { // The function has several arguments
          return lst(argf.seq.begin(), argf.seq.end());
        } else {
          return argf.seq[0];
        }
      }
    }

    return me;
  }

  return e.map(*this);
} // reduce_double_funcs::operator()

ex func::replace_function_by_func::operator()(const ex &e) {
  MSG_INFO(3, "Replacing all GiNaC functions by EQC funcs in " << e << endline);

  if (is_a<fderivative>(e)) {
    const fderivative& f = ex_to<fderivative>(e);

    // Get indices of which arguments of the function to differeniate by - the interface doesn't offer any direct way!
    // TODO: It does - fderivative::derivatives()
    std::ostringstream temp;
    temp << dflt << f;
    std::string deriv = temp.str();
    MSG_INFO(2, "fderivative: indices = ");
    size_t bclosepos = deriv.find("]");
    std::string args = deriv.substr(2, bclosepos-2);
    std::multiset<unsigned> parset;
    std::size_t start = 0, end = 0;
    while ((end = args.find(",", start)) != std::string::npos) {
      parset.insert(atoi(args.substr(start, end - start).c_str()));
      MSG_INFO(2, atoi(args.substr(start, end - start).c_str()) << ",");
      start = end + 1;
    }
    parset.insert(atoi(args.substr(start).c_str()));
    MSG_INFO(2, atoi(args.substr(start).c_str()));
    if (parset.empty()) throw std::runtime_error("Internal error: fderivative with no parameters in paramset");

    // Name of the function being differentiated
    std::string name = f.get_name();
    std::map<std::string, std::string>::const_iterator fname_it = hard_names_rev.find(name);
    if (fname_it == hard_names_rev.end())
      throw std::runtime_error("Internal error: Unknown hard-coded function in expression");
    std::string fname = fname_it->second;
    MSG_INFO(2, "; function name: " << fname << "; arguments: ");

    // Arguments of function itself
    exprseq fargs(f.begin(), f.end());
    if (msg::info().checkprio(2)) for (const auto& i : f) msg::info() << i << ", ";
    MSG_INFO(2, endline);

    if (parset.size() > 1) throw std::runtime_error("Internal error: Multiple parameters to fderivative not yet supported");
    func the_function = func(fname, fargs);
    if (*parset.begin() > the_function.get_numargs()) throw std::runtime_error("Error: Differentiation to non-existent variable");
    return dynallocate<exderivative>(dynallocate<differential>(the_function, false, 1),
                                     dynallocate<differential>(fargs[*parset.begin()], false, 1, the_function));
  } else if (is_a<function>(e)) {
    ex result = e.map(*this);
    if (!is_a<function>(result)) return result;

    std::string name = ex_to<function>(e).get_name();
    std::map<std::string, std::string>::const_iterator fname_it = hard_names_rev.find(name);
    if (fname_it == hard_names_rev.end())
      throw std::runtime_error("Internal error: Unknown hard-coded function in expression");

    const function& f = ex_to<function>(result);
    return dynallocate<func>(fname_it->second, exprseq(f.begin(), f.end())).setflag(status_flags::evaluated); // Avoid infinite recursion in func::eval()
  }
  return e.map(*this);
}

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
ex func::evalf() const {
#else
ex func::evalf (int level) const {
#endif
  MSG_INFO(3, "Evaluating " << *this << endline);
  if (seq.size() != get_numargs()) // Avoid crash for illegal syntax sin^2(x)
    throw std::runtime_error("Function with insufficient number of arguments cannot be evaluated");

  // If the function is hardcoded, drop through to the GiNaC::function::evalf() method
  // TODO: expression::evalf() will not be called here on the arguments
  if (hard) {
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
    ex result = function(serial, seq).evalf();
#else
    ex result = function(serial, seq).evalf(level);
#endif
    replace_function_by_func replace_functions;
    return replace_functions(result).eval();
  }

  // Evaluate children first
  exvector eseq;
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  eseq.reserve(seq.size());
  for (auto & it : seq) {
    eseq.emplace_back(expression(it).evalf());
  }
#else
  if (level == 1)
    eseq = seq;
  else if (level == -max_recursion_level)
    throw(std::runtime_error("max recursion level reached"));
  else
    eseq.reserve(seq.size());

  --level;
  for (const auto& it : seq)
    eseq.emplace_back(expression(it).evalf(level));
#endif

  if (!definition.is_empty()) {
    // Note: The code in GiNaC::functions.cpp::evalf() seems to drop eseq here!
    //if (level == 1) // evaluate only one level. Is this correct?
    //  return (expand_definition(args));
    try {
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
      return func(name, eseq).expand_definition().evalf();
#else
      return func(name, eseq).expand_definition().evalf(level);
#endif
    } catch (std::exception &e) { // The function cannot be expanded
      MSG_ERROR(0, e.what() << endline);
    }
  }

  return func(name, eseq).hold();
}

ex func::evalm() const {
  // Evaluate children first
  exvector eseq;
  eseq.reserve(seq.size());

  for (const auto& it : seq)
    eseq.emplace_back(expression(it).evalm());

  return func(name, eseq).hold();
}

unsigned func::calchash(void) const {
  unsigned v = golden_ratio_hash(make_hash_seed(typeid(*this)) ^ serial);
  for (size_t i=0; i<nops(); i++) {
    v = rotate_left(v);
    v ^= this->op(i).gethash();
  }

  if (flags & status_flags::evaluated) {
    setflag(status_flags::hash_calculated);
    hashvalue = v;
  }
  MSG_INFO(3, "Hash value of " << name << " (serial " << serial << "): " << v << endline);
  return v;
}

ex func::series(const relational &r, int order, unsigned options) const {
  // Not implemented yet. Returns basic::series
  return basic::series(r, order, options);
}

ex func::thiscontainer(const exvector &v) const {
  return func(name, v);
}

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
ex func::thiscontainer(exvector &&v) const {
  return func(name, std::move(v));
}
#else
ex func::thiscontainer(std::auto_ptr<exvector> vp) const {
  return func(name, vp);
}
#endif

ex func::derivative(const symbol & s) const {
  MSG_INFO(2, "Calculating derivative of " << *this << " to " << s << endline);

  if (hard) { // Fall through to GiNaC::function::diff(). derivative() is protected!
    // Special handling of sum function - hard-coded derivative function is unusable because differentiation symbol s cannot be passed to it
    if ((name == "sum") && (seq.size() == 3))
      return dynallocate<func>(name, exprseq{seq[0], seq[1], expression(seq[2]).diff(s)});

    ex result = function(serial, seq).setflag(status_flags::evaluated).diff(s); // Note: Omitting setting the flag results in an infinite loop!
    replace_function_by_func replace_functions; // get rid of GiNaC functions that might have been introduced
    return replace_functions(result).eval();
  }

  ex result(_ex0);

  if (seq.empty()) {
    if (name == s.get_name()) return _ex1; // Handle case dr() / dr()
    for (unsigned i = 0; i < get_numargs(); i++) {
      ex arg_diff = expression(vars[i]).diff(s);
      if (!arg_diff.is_zero()) result += pderivative(i)*arg_diff;
      MSG_INFO(2, "Derivative of " << *this << " stage " << i << ": " << result << endline);
    }
  } else {
    for (unsigned i = 0; i < seq.size(); i++) {
      ex arg_diff = expression(seq[i]).diff(s);
      if (!arg_diff.is_zero()) result += pderivative(i)*arg_diff;
      MSG_INFO(2, "Derivative of " << *this << " stage " << i << ": " << result << endline);
    }
  }
  return result;
}

int func::compare_same_type(const basic &other) const {
  // This function is important because it is used to simplify expressions!
  // If (*this == other) then the expression "*this / other" will be 1.
  // TODO: What is the point of comparing > and < ?
  const func &o = static_cast<const func &>(other);
  MSG_INFO(3, "Comparing " << *this << " and " << o << endline);
  if (name == o.name) {
    if (seq.empty())
      // This is required to make substitution into vars last - otherwise it is discarded
      return (std::equal(vars.begin(), vars.end(), o.vars.begin()) ? 0 : 1);
    else
      return exprseq::compare_same_type(o);
  }
  else if (name < o.name)
    return -1;
  else
    return 1;
}

bool func::is_equal_same_type(const basic & other) const {
  // This function seems to be used for substitution: If a.is_equal_same_type(b),
  // then b==c will be substituted
  const func & o = static_cast<const func &>(other);
  MSG_INFO(3, "Checking equality of " << *this << " and " << o << endline);

  if (name != o.name)
    return false;
  else
    return (exprseq::is_equal_same_type(o));
}

bool func::match_same_type(const basic &other) const {
  const func & o = static_cast<const func &>(other);
  MSG_INFO(3, "Checking match of " << *this << " and " << o << endline);
  return name == o.name;
}

unsigned func::return_type(void) const {
  MSG_INFO(4, "Return type of " << *this << " requested." << endline);
  if (seq.empty())
    return return_types::commutative;
  else
    return seq.begin()->return_type();
}

ex func::pderivative(unsigned diff_param) const {
  MSG_INFO(2, "Calculating partial derivative of " << *this <<
                 " to parameter " << diff_param << endline);

  ex result;
  bool partial = get_numargs() > 1;

  // We assume that no hardcoded functions are called with this method!
  if (definition.is_empty() || !(hints & FUNCHINT_DEFDIFF)) {
    if (seq.size() > diff_param)
      result = dynallocate<exderivative>(dynallocate<differential>(*this, partial),
                                         dynallocate<differential>(seq[diff_param], partial, 1, *this));
    else if (get_numargs() > diff_param)
      result = dynallocate<exderivative>(dynallocate<differential>(*this, partial),
                                         dynallocate<differential>(vars[diff_param], partial, 1, *this));
    else
      throw std::logic_error("The requested dependant variable does not exist in " + name + "()");
  } else {
    if (diff_param >= get_numargs())
      throw std::logic_error("The requested dependant variable does not exist in " + name + "()");

    ex diffvar = vars[diff_param];
    result = definition.diff(diffvar); // pdiff is not necessary here, because we are differentiating directly to the functions' variables

    if (!seq.empty()) { // Substitute the function arguments in the result
      exmap subs_map;
      for (unsigned i = 0; i < get_numargs(); i++) subs_map.emplace(vars[i], seq[i]);
      result = result.subs(subs_map);
    }
  }

  MSG_INFO(2, "Partial derivative #" << diff_param << " of " << *this << ": "
                     << result << endline);
  return result;
}

//--------------------------------------------------------------------------------------

bool func::initialized = false;


void func::remove(const std::string& fname) {
  functions.erase(fname);
}

void func::clear() {
  MSG_INFO(2, "Clearing functions..." << endline);
  if (msg::info().checkprio(3)) {
    msg::info() << "List of functions" << endline;
    for (const auto& it_func : functions) {
      msg::info() << it_func.first << " = ";
      if (it_func.second.hard)
        msg::info() << "[hard-coded]";
      else
        msg::info() << it_func.second.definition;
      msg::info() << endline;
    }
  }

  funcrec_it it_func = functions.begin();
  while (it_func != functions.end()) {
    if (!(it_func->second.hints & FUNCHINT_LIB)) {
      MSG_INFO(3, "Deleting " << it_func->first << endline);
      it_func = functions.erase(it_func);
    } else
      it_func++;
  }
}

void func::clearall() {
  functions.clear();
}

unsigned func::hint(const std::string &s) {
    std::string str = s;

    if (s == "no_bracket")  {
      MSG_WARN(0, "Warning: Replace deprecated function hint 'no_bracket' with 'nobracket'" << endline);
      str = "nobracket";
    }

    if (funchint_map.find(str) == funchint_map.end()) {
      MSG_WARN(0, "Warning: The hint " << str << " is not defined." << endline);
      return(0);
    } else {
      return funchint_map.at(str);
    }
  }

  const unsigned max_unsigned = 1 << (sizeof(unsigned) * 8 - 1); // *** TODO: 1 << ...*8 gives warning

void func::registr(const std::string &n, exvector &args, const unsigned h, const std::string& printname) {
  MSG_INFO(3, "Registering function " << n << " '" << printname << "' with arguments " << args << endline);
  if (functions.find(n) != functions.end())
    throw (std::invalid_argument("Function " + n + " already exists"));

  funcrec_it f = functions.emplace(n, h).first;
  f->second.printname = (printname == "" ? n : printname);

  std::string nbare = n;

  if (hard_names.find(nbare) == hard_names.end()) {
    // This precaution avoids confusion of ln and log (LaTeX and OOo use ln for the natural logarithm, but GiNaC uses log)
    // It also means that hard-coded functions that are not in hard_names[] will not be recognized as such
    f->second.serial = max_unsigned - (unsigned)functions.size();
    f->second.hard = false;
  } else {
    try { // Is the function hard-coded into GiNaC?
      f->second.serial = function::find_function(hard_names.at(nbare), (unsigned)args.size());
      f->second.hard = true;
      MSG_INFO(3, "Function " << hard_names.at(nbare) << " is hard-coded into iMath." << endline);
    } catch (std::exception &e) {
      (void)e;
      f->second.serial = max_unsigned - (unsigned)functions.size();
      f->second.hard = false;
    }
  }

  for (const auto& i : args) {
    if ( !(is_a<symbol>(i) || (is_a<func>(i) && ex_to<func>(i).is_pure())) ) {
      functions.erase(f);
      throw (std::invalid_argument("Argument of " + n + " is no symbol! "));
    }
  }
  f->second.vars.swap(args);

  MSG_INFO(2, "Registered function " << f->second.printname << "(" << f->second.vars << ") with serial " << f->second.serial <<
    ((h & FUNCHINT_LIB) ? ", which is a library function." : ".")  << endline);
}

void func::define(const std::string &n, const expression &def) {
  funcrec_it f = functions.find(n);
  if (f == functions.end())
   throw (std::invalid_argument("Function " + n + " does not exist! Please register it first."));

  // TODO: handle the case that the function has been differentiated!
  f->second.definition = def;
  MSG_INFO(2, "Defined function " << n <<
    "(" << f->second.vars << ")  = " << def << endline);
}

bool func::is_a_func(const std::string &fname) {
  return (functions.find(fname) != functions.end());
}

bool func::is_expand(const std::string& fname) {
  funcrec_cit f = functions.find(fname);
  if (f != functions.end())
    return f->second.hints & FUNCHINT_EXPAND;
  return false;
}

bool func::is_lib(const std::string& fname) {
  funcrec_cit f = functions.find(fname);
  if (f != functions.end())
    return f->second.hints & FUNCHINT_LIB;
  return false;
}

exvector func::get_args() const {
  if (seq.empty())
    return vars;
  else
    return seq;
}

// ------------------------------------------------------------------------------
// hard-coded functions

static ex round_eval(const ex &e, const ex &n) {
  if (!is_a<numeric>(expression(e).evalf()))
    return dynallocate<func>("round", exprseq{e, n}).hold();
  numeric num = ex_to<numeric>(expression(e).evalf());
  if (!num.info(info_flags::real))
    throw std::runtime_error("Can only round real numbers");

  numeric digits;
  if (!is_a<numeric>(n)) {
    ex ndig = expression(n).evalf(); // Maybe n becomes a numeric now? Note: Doing evalf() right away converts integers to floats

    if (!is_a<numeric>(ndig)) {
      throw std::runtime_error("Number of digits to round to must be an integer");
    } else {
      digits = ex_to<numeric>(ndig);
    }
  } else {
    digits = ex_to<numeric>(n);
  }
  // Testing for nonnegative is OK, since we expect the original number (before evalf()) to have been an integer
  if (!digits.info(info_flags::nonnegative))
    throw std::runtime_error("Number of digits to round to must be a positive integer");
  // Note: Converting digits to an int limits the number of decimal places that can be rounded to 2^32
  int idigits = std::lround(digits.to_double());
  cln::cl_F m = 1.0;
  if (idigits > 0)
    m = cln::the<cln::cl_F>(cln::expt(cln::cl_float(10.0, cln::default_float_format), cln::the<cln::cl_I>(idigits)));
  return dynallocate<numeric>(truncate1(cln::the<cln::cl_R>(num.to_cl_N()) * m + csgn(num) * cln::cl_float(0.5, cln::default_float_format)) / m);
}

REGISTER_FUNCTION(round, eval_func(round_eval));

static ex floor_eval(const ex &e, const ex &n) {
  if (!is_a<numeric>(expression(e).evalf()))
    return dynallocate<func>("floor", exprseq{e, n}).hold();
  numeric num = ex_to<numeric>(expression(e).evalf());
  if (!num.info(info_flags::real))
    throw std::runtime_error("Can only floor real numbers");

  numeric digits;
  if (!is_a<numeric>(n)) {
    ex ndig = expression(n).evalf(); // Maybe n becomes a numeric now? Note: Doing evalf() right away converts integers to floats

    if (!is_a<numeric>(ndig)) {
      throw std::runtime_error("Number of digits to floor to must be an integer");
    } else {
      digits = ex_to<numeric>(ndig);
    }
  } else {
    digits = ex_to<numeric>(n);
  }
  // Testing for nonnegative is OK, since we expect the original number (before evalf()) to have been an integer
  if (!digits.info(info_flags::nonnegative))
    throw std::runtime_error("Number of digits to floor to must be a positive integer");

  int idigits = std::lround(digits.to_double());
  cln::cl_F m = 1.0;
  if (idigits > 0)
    m = cln::the<cln::cl_F>(cln::expt(cln::cl_float(10.0, cln::default_float_format), cln::the<cln::cl_I>(idigits)));
  return dynallocate<numeric>(floor1(cln::the<cln::cl_R>(num.to_cl_N()) * m) / m);
}

REGISTER_FUNCTION(floor, eval_func(floor_eval));

static ex ceil_eval(const ex &e, const ex &n) {
  if (!is_a<numeric>(expression(e).evalf()))
    return dynallocate<func>("ceil", exprseq{e, n}).hold();
  numeric num = ex_to<numeric>(expression(e).evalf());
  if (!num.info(info_flags::real))
    throw std::runtime_error("Can only ceil real numbers");

  numeric digits;
  if (!is_a<numeric>(n)) {
    ex ndig = expression(n).evalf(); // Maybe n becomes a numeric now? Note: Doing evalf() right away converts integers to floats

    if (!is_a<numeric>(ndig)) {
      throw std::runtime_error("Number of digits to ceil to must be an integer");
    } else {
      digits = ex_to<numeric>(ndig);
    }
  } else {
    digits = ex_to<numeric>(n);
  }

  // Testing for nonnegative is OK, since we expect the original number (before evalf()) to have been an integer
  if (!digits.info(info_flags::nonnegative))
    throw std::runtime_error("Number of digits to ceil to must be a positive integer");
  int idigits = std::lround(digits.to_double());
  cln::cl_F m = 1.0;
  if (idigits > 0)
    m = cln::the<cln::cl_F>(cln::expt(cln::cl_float(10.0, cln::default_float_format), cln::the<cln::cl_I>(idigits)));
  return dynallocate<numeric>(ceiling1(cln::the<cln::cl_R>(num.to_cl_N()) * m) / m);
}

REGISTER_FUNCTION(ceil, eval_func(ceil_eval));

static ex sum_eval(const ex &lower, const ex &higher, const ex &e) {
  MSG_INFO(3, "Doing hard eval of sum" << endline);
  if (!is_a<relational>(lower))
    throw std::runtime_error("Lower bound must be an equation");
  if (!is_a<symbol>((ex_to<relational>(lower)).lhs()))
    throw std::runtime_error("Lower bound must assign a value to a symbol");
  if (lower.is_equal(higher)) return e;
  // TODO: check if higher < lower
  if (e.is_zero()) return _ex0;
  return dynallocate<func>("sum", exprseq{lower, higher, e}).hold();
}

static void sum_print_imath(const ex &lower, const ex &higher, const ex &e, const print_context& c) {
  c.s << "sum from {";
  lower.print(c);
  c.s << "} to {";
  higher.print(c);
  c.s << "} {";
  if (is_a<add>(e))
    c.s << "(";
  e.print(c);
  if (is_a<add>(e))
    c.s << ")";
  c.s << "}";
}

REGISTER_FUNCTION(sum, eval_func(sum_eval).
                       print_func<imathprint>(sum_print_imath));

 ex func::expand_sum::operator()(const ex &e) {
  MSG_INFO(2, "Expanding sum in " << e << endline);

  if (is_a<func>(e) && (ex_to<func>(e).name == "sum")) {
    func f = ex_to<func>(e);

    // Handle children first
    expand_sum expand_s;
    for (auto& i : f.seq)
      i = expand_s(i);

    MSG_INFO(0,  "Lower bound: " << f.seq[0] << endline);
    const symbol& var = ex_to<symbol>(ex_to<equation>(f.seq[0]).lhs());
    expression lbound = ex_to<equation>(f.seq[0]).rhs();
    expression hbound = f.seq[1];
    MSG_INFO(0,  "Summing up " << f.seq[2] << " from " << var << " = " << lbound
                       << " to " << hbound << endline);

    int l, h;
    if (!is_a<numeric>(lbound)) {
      throw std::logic_error("Lower bound of sum must be an integer");
    } else {
      l = numeric_to_int(ex_to<numeric>(lbound));
    }
    if (!is_a<numeric>(hbound)) {
      throw std::logic_error("Higher bound of sum must be an integer");
    } else {
      h = numeric_to_int(ex_to<numeric>(hbound));
    }

    expression result;
    while (l <= h) {
      MSG_INFO(0,  "Summing up: current value: " << result << endline);
      result = result + expression(f.seq[2].subs(var == l));
      l++;
    }
    return std::move(result);
  }

  return e.map(*this);
}

static ex mindex_eval(const ex &e, const ex &r, const ex &c) {
  // Immediately evaluate if e is a matrix and r and c are integers
  MSG_INFO(3, "mindex eval: " << e << ", " << r << ", " << c << endline);
  ex unchanged = dynallocate<func>("mindex", exprseq{e, r, c}).hold();

  if (is_a<matrix>(e)) {
    const matrix& m = ex_to<matrix>(e);
    int row = 0;
    int col = 0;

    if (is_a<numeric>(r)) {
      const numeric& rnum = ex_to<numeric>(r);

      if  (rnum.info(info_flags::posint)) {
        row = rnum.to_int();
      } else { // try harder, since non-integer indixes make no sense
        row = numeric_to_int(rnum);
      }
      row--; // Adjust index to count from 0
      if (row >= (int)m.rows() || row < 0) {
        MSG_WARN(0,  "mindex: Warning: Row index " << row+1 << " out of bounds for " << m << endline);
        return unchanged;
      }
    } else if (is_a<wildcard>(r)) {
      row = -1;
    } else {
      return unchanged;
    }

    MSG_INFO(2, "mindex row: " << row << ", rows(): " << m.rows() << endline);

    if (is_a<numeric>(c)) {
      const numeric& cnum = ex_to<numeric>(c);

      if (cnum.info(info_flags::posint)) {
        col = cnum.to_int();
      } else {
        col = numeric_to_int(cnum);
      }
      col--; // Adjust index to count from 0
      if (col >= (int)m.cols() || (col < 0 && col != -1000)) {
        MSG_WARN(0, "mindex: Warning: Column index " << col+1 << " out of bounds for " << m << endline);
        return unchanged;
      }
    } else if (is_a<wildcard>(c)) {
      col = -1;
    } else {
      return unchanged;
    }

    MSG_INFO(2, "mindex col: " << col << ", cols(): " << m.cols() << endline);

    // Handle special case of vector index where row and column are not distinguished
    if (col == -1000) {
      if (m.rows() == 1)
        return m(0, row);
      else
        return m(row, 0);
    }

    if (row == -1) {
      if (col == -1) {
        return e; // m[wild, wild] returns complete matrix
      } else {
        if (m.rows() == 1) {
          return m(0, col); // Special treatment for vector: Return one element only
        } else {
          matrix result(m.rows(), 1);
          for (unsigned i = 0; i < m.rows(); i++) result(i, 0) = m(i, col);
          return result;
        }
      }
    } else {
      if (col == -1) {
        if (m.cols() == 1) {
          return m(row, 0); // Special treatment for vector: Return one element only
        } else {
          matrix result(1, m.cols());
          for (unsigned i = 0; i < m.cols(); i++) result(0, i) = m(row, i);
          return result;
        }
      } else {
        return m(row, col);
      }
    }
  } else {
    return unchanged;
  }
}

static void mindex_print_imath(const ex &e, const ex &row, const ex &col, const print_context& c) {
  e.print(c);
  c.s << "[";
  if (is_a<wildcard>(row)) {
    c.s << "\"*\"";
  } else {
    row.print(c);
  }
  if (col == numeric(-999)) {
    // Leave away column index for vectors from smathparser.yxx
  } else {
    c.s << ",";
    if (is_a<wildcard>(col))
      c.s << "\"*\"";
    else
      col.print(c);
  }
  c.s << "]";
}

REGISTER_FUNCTION(mindex, eval_func(mindex_eval).print_func<imathprint>(mindex_print_imath));

static ex hadamard_eval(const ex &e1, const ex &e2, const ex& op) {
  if (!op.info(info_flags::real))
    throw std::invalid_argument("Hadamard-operation must be 0, 1 or 2");
  hadamard_operation h_op;
  if (is_equal_int(ex_to<numeric>(op), 0, Digits))
    h_op = h_product;
  else if (is_equal_int(ex_to<numeric>(op), 1, Digits))
    h_op = h_division;
  else if (is_equal_int(ex_to<numeric>(op), 2, Digits))
    h_op = h_power;
  else
    throw std::invalid_argument("Hadamard-operation must be 0, 1 or 2");

  // Immediately evaluate if both expressions are matrices with matching dimensions
  MSG_INFO(1, "hadamard eval: " << e1 << (h_op == h_product ? "*" : (h_op == h_division ? "/" : "^")) << e2 << endline);
  ex unchanged = dynallocate<func>("hadamard", exprseq{e1, e2, op}).hold();

  ex e1_e = e1.evalm();
  ex e2_e = e2.evalm();

  if (is_a<matrix>(e1_e) && is_a<matrix>(e2_e)) {
    const matrix& m1 = ex_to<matrix>(e1_e);
    const matrix& m2 = ex_to<matrix>(e2_e);
    unsigned rows = m1.rows();
    unsigned cols = m1.cols();
    matrix result(rows, cols);

    if (rows == m2.rows() && cols == m2.cols()) {
      for (unsigned r = 0; r < rows; ++r) {
        for (unsigned c = 0; c < cols; ++c) {
          switch (h_op) {
            case h_product:  result(r, c) = m1(r, c) * m2(r, c); break;
            case h_division: {
              if (m2(r ,c).is_zero())
                result(r, c) = stringex("NaN");
              else
                result(r, c) = m1(r, c) / m2(r, c);
              break;
            }
            case h_power:    result(r, c) = pow(m1(r, c), m2(r, c)); break;
          }
        }
      }

      return result;
    }
  }

  return unchanged;
}

static void hadamard_print_imath(const ex &e1, const ex &e2, const ex& op, const print_context& c) {
  if (!op.info(info_flags::integer))
    throw std::invalid_argument("Hadamard-operation must be 0, 1 or 2");
  hadamard_operation h_op = (hadamard_operation)ex_to<numeric>(op).to_int();
  if (h_op < h_product || h_op > h_power)
    throw std::invalid_argument("Hadamard-operation must be 0, 1 or 2");

  if (is_a<add>(e1)) c.s << "(";
  e1.print(c);
  if (is_a<add>(e1)) c.s << ")";
  switch (h_op) {
    case h_product: c.s << " ⊗ "; break;
    case h_division: c.s << " ⊘ "; break;
    case h_power: c.s << " ⓔ "; break;
  }
  if (is_a<add>(e2)) c.s << "(";
  e2.print(c);
  if (is_a<add>(e2)) c.s << ")";
}

REGISTER_FUNCTION(hadamard, eval_func(hadamard_eval).print_func<imathprint>(hadamard_print_imath));

static ex transpose_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "transpose: " << e << endline);

  if (is_a<matrix>(e)) {
    return ex_to<matrix>(e).transpose();
  } else {
    return dynallocate<func>("transpose", exprseq{e}).hold();
  }
}

static void transpose_print_imath(const ex &e, const print_context& c) {
  e.print(c);
  c.s << "^T";
}

REGISTER_FUNCTION(transpose, eval_func(transpose_eval).
                             print_func<imathprint>(transpose_print_imath));

static ex vecprod_eval(const ex& e1, const ex& e2) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "vecprod: " << e1 << ", " << e2 << endline);

  if (is_a<matrix>(e1) && is_a<matrix>(e2)) {
    matrix v1 = ex_to<matrix>(e1);
    matrix v2 = ex_to<matrix>(e2);

    if (!( ((v1.cols() == 1) && (v2.cols() == 1) && (v1.rows() == v2.rows()) && (v1.rows() == 3)) ||
           ((v1.rows() == 1) && (v2.rows() == 1) && (v1.cols() == v2.cols()) && (v1.cols() == 3)) ))
      throw std::invalid_argument("Error: The vector product can only be calculated with two row or column vectors of three components each");

    if (v1.rows() < v1.cols()) v1 = v1.transpose();
    if (v2.rows() < v2.cols()) v2 = v2.transpose();
    matrix t(v1.rows(), 3);
    symbol e = symbol("e");

    for (unsigned i = 0; i < v1.rows(); i++) {
      t(i,0) = pow(e, i);
      t(i,1) = v1(i,0);
      t(i,2) = v2(i,0);
    }

    ex tdet = t.determinant();
    matrix result(v1.rows(), 1);

    for (unsigned i = 0; i < v1.rows(); i++)
      result(i,0) = tdet.coeff(e, i);

    return result;
  } else {
    return dynallocate<func>("vecprod", exprseq{e1, e2}).hold();
  }
}

static void vecprod_print_imath(const ex &e1, const ex& e2, const print_context& c) {
  e1.print(c);
  c.s << " times ";
  e2.print(c);
}

REGISTER_FUNCTION(vecprod, eval_func(vecprod_eval).
                           print_func<imathprint>(vecprod_print_imath));

static ex scalprod_eval(const ex& e1, const ex& e2) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "scalprod: " << e1 << ", " << e2 << endline);
  ex unchanged = dynallocate<func>("scalprod", exprseq{e1, e2}).hold();

  if (is_a<matrix>(e1) && is_a<matrix>(e2)) {
    const matrix& v1 = ex_to<matrix>(e1);
    const matrix& v2 = ex_to<matrix>(e2);

    if (!( (v1.rows() == 1) && (v2.cols() == 1) && (v1.rows() == v2.cols())))
      return unchanged; // or should we throw an exception?

    matrix result = ex_to<matrix>((e1 * e2).evalm());
    return result(0,0); // Return a numeric instead of a 1x1 matrix
  } else {
    return unchanged;
  }
}

static void scalprod_print_imath(const ex &e1, const ex& e2, const print_context& c) {
  e1.print(c);
  c.s << " ";
  e2.print(c);
}

REGISTER_FUNCTION(scalprod, eval_func(scalprod_eval).
                            print_func<imathprint>(scalprod_print_imath));

// Here false = 0, true = !false, and -1 means the condition does not evaluate to an integer
int eval_condition(const ex& condition) {
  if (is_a<relational>(condition)) {
    const relational& cond = ex_to<relational>(condition);

    if (is_a<stringex>(cond.lhs()) && is_a<stringex>(cond.rhs())) {
        std::string lhs = ex_to<stringex>(cond.lhs()).get_string();
        std::string rhs = ex_to<stringex>(cond.rhs()).get_string();

        if (cond.info(info_flags::relation_equal))
            return lhs.compare(rhs) == 0;
        else if (cond.info(info_flags::relation_not_equal))
            return lhs.compare(rhs) != 0;
        else if (cond.info(info_flags::relation_less))
            return lhs.compare(rhs) < 0;
        else if (cond.info(info_flags::relation_less_or_equal))
            return lhs.compare(rhs) <= 0;
        else if (cond.info(info_flags::relation_greater))
            return lhs.compare(rhs) > 0;
        else if (cond.info(info_flags::relation_greater_or_equal))
            return lhs.compare(rhs) >= 0;
    } else {
        ex df = cond.lhs() - cond.rhs();
        operands o1(GINAC_MUL), o2(GINAC_MUL);
        operands::split_ex(df, o1, o2);

        if (o1.is_quantity() && o2.is_quantity()) {
            ex coeff = o1.get_coefficient();

            if (cond.info(info_flags::relation_equal))
                return coeff.is_zero();
            else if (cond.info(info_flags::relation_not_equal))
                return !coeff.is_zero();
            else if (cond.info(info_flags::relation_less))
                return coeff.info(info_flags::negative);
            else if (cond.info(info_flags::relation_less_or_equal))
                return coeff.info(info_flags::negative) || coeff.is_zero();
            else if (cond.info(info_flags::relation_greater))
                return coeff.info(info_flags::positive);
            else if (cond.info(info_flags::relation_greater_or_equal))
                return coeff.info(info_flags::nonnegative);
        }
    }
  } else if (is_a<numeric>(condition)) {
    const numeric& c = ex_to<numeric>(condition);
    if (c.info(info_flags::positive))
      return 1;
    else if (c.info(info_flags::nonnegative))
      return 0;
  }

  return -1;
}

static ex ifelse_eval(const ex& condition, const ex& e1, const ex& e2) {
  // Immediately evaluate if condition involves numerics or quantities
  // Do not evaluate otherwise! Reason: If you use ifelse in a user-defined function
  // definition, then the condition will be evaluated already there, e.g.
  // max(x;y) = ifelse(x < y; y; x) will always return false...
  MSG_INFO(3, "eval ifelse: "  << condition << " ? " << e1 << " : " << e2 << endline);

  int result = eval_condition(condition);
  if (result == -1)
    return dynallocate<func>("ifelse", exprseq{condition, e1, e2}).hold();

  return result ? e1 : e2;
}

static ex ifelse_evalf(const ex& condition, const ex& e1, const ex& e2) {
  // We must re-implement evalf() because otherwise the condition will not be checked
  MSG_INFO(3, "evalf ifelse: "  << condition << " ? " << e1 << " : " << e2 << endline);

  ex e_condition = condition.evalf();
  int result = eval_condition(e_condition);
  if (result == -1)
    return dynallocate<func>("ifelse", exprseq{e_condition, e1.evalf(), e2.evalf()}).hold();

  return result ? e1.evalf() : e2.evalf();
}

REGISTER_FUNCTION(ifelse, eval_func(ifelse_eval).evalf_func(ifelse_evalf));

static ex vmax_eval(const ex& v) {
  // Immediately evaluate if vector contains a smallest member
  MSG_INFO(3, "vmax: "  << v << endline);
  ex unchanged = dynallocate<func>("vmax", exprseq{v}).hold();
  if (is_a<matrix>(v)) {
    const matrix& m = ex_to<matrix>(v);

    if (!is_a<numeric>(m.op(0)))
      return unchanged;

    numeric result = ex_to<numeric>(m.op(0));

    for (unsigned i = 1; i < m.nops(); i++) {
      if (!is_a<numeric>(m.op(i)))
        return unchanged;

      const numeric& n = ex_to<numeric>(m.op(i));
      if (n > result)
        result = n;
    }

    return result;
  } else {
    return unchanged;
  }
}

static void vmax_print_imath(const ex& v, const print_context& c) {
  c.s << "max left lbrace stack{";

  if (is_a<matrix>(v)) {
    const matrix& m = ex_to<matrix>(v);
    for (unsigned i = 0; i < m.nops()-1; i++) {
      m.op(i).print(c);
      c.s << " # ";
    }

    m.op(m.nops()-1).print(c);
  } else {
    v.print(c);
  }

  c.s << "} right none";
}

REGISTER_FUNCTION(vmax, eval_func(vmax_eval).
                        print_func<imathprint>(vmax_print_imath));

static ex vmin_eval(const ex& v) {
  // Immediately evaluate if vector contains a smallest member (works only for numerics)
  MSG_INFO(1, "vmin: "  << v << endline);
  ex unchanged = dynallocate<func>("vmin", exprseq{v}).hold();

  if (is_a<matrix>(v)) {
    const matrix& m = ex_to<matrix>(v);

    if (!is_a<numeric>(m.op(0)))
      return unchanged;

    numeric result = ex_to<numeric>(m.op(0));

    for (unsigned i = 1; i < m.nops(); i++) {
      if (!is_a<numeric>(m.op(i)))
        return unchanged;

      const numeric& n = ex_to<numeric>(m.op(i));
      if (n < result)
        result = n;
    }

    return result;
  } else {
    return unchanged;
  }
}

static void vmin_print_imath(const ex& v, const print_context& c) {
  c.s << "min left lbrace stack{";

  if (is_a<matrix>(v)) {
    const matrix& m = ex_to<matrix>(v);
    for (unsigned i = 0; i < m.nops()-1; i++) {
      m.op(i).print(c);
      c.s << " # ";
    }

    m.op(m.nops()-1).print(c);
  } else {
    v.print(c);
  }

  c.s << "} right none";
}

REGISTER_FUNCTION(vmin, eval_func(vmin_eval).
                        print_func<imathprint>(vmin_print_imath));

static ex concat_eval(const ex& v1, const ex& v2) {
  // Immediately evaluate if expressions are matrices or lists
  MSG_INFO(3, "concat: "  << v1 << ", " << v2 << endline);
  ex unchanged = dynallocate<func>("concat", exprseq{v1, v2}).hold();

  if (is_a<matrix>(v1)) {
    const matrix& m1 = ex_to<matrix>(v1);

    if (is_a<matrix>(v2)) {
      const matrix& m2 = ex_to<matrix>(v2);
      if (m1.cols() != m2.cols()) return unchanged;
      matrix m(m1.rows() + m2.rows(), m1.cols());
      for (unsigned r = 0; r < m.rows(); ++r)
        for (unsigned c = 0; c < m.cols(); ++c)
          m(r, c) = (r < m1.rows() ? m1(r, c) : m2(r - m1.rows(), c));
      return m;
    } else if (is_a<lst>(v2)) {
      const lst& l2 = ex_to<lst>(v2);
      if (l2.nops() != m1.cols()) return unchanged;
      matrix m(m1.rows() + 1, m1.cols());
      for (unsigned r = 0; r < m.rows(); ++r)
        for (unsigned c = 0; c < m.cols(); ++c)
          m(r, c) = (r < m1.rows() ? m1(r, c) : l2.op(c));
      return m;
    } else {
      return unchanged;
    }
  } else if (is_a<lst>(v1)) {
    const lst& l1 = ex_to<lst>(v1);

    if (is_a<matrix>(v2)) {
      const matrix& m2 = ex_to<matrix>(v2);
      if (l1.nops() != m2.cols()) return unchanged;
      matrix m(1 + (unsigned)m2.rows(), (unsigned)l1.nops());
      for (unsigned r = 0; r < m.rows(); ++r)
        for (unsigned c = 0; c < m.cols(); ++c)
          m(r, c) = (r == 0 ? l1.op(r) : m2(r - 1, c));
      return m;
    } else if (is_a<lst>(v2)) {
      const lst& l2 = ex_to<lst>(v2);
      lst result(l1);
      for (const auto& e : l2) result.append(e);
      return result;
    } else {
      return unchanged;
    }
  } else {
    return unchanged;
  }
}

REGISTER_FUNCTION(concat, eval_func(concat_eval));

static ex diagmatrix_eval(const ex &e) {
  // Immediately evaluate if e is a list or a vector
  MSG_INFO(3, "diag: " << e << endline);

  if (is_a<matrix>(e)) {
    const matrix& m = ex_to<matrix>(e);
    if (m.rows() == 1 || m.cols() == 1)
      return diag_matrix(make_lst_from_matrix(ex_to<matrix>(e), true));
  }

  return dynallocate<func>("diag", exprseq{e}).hold();
}

static void diagmatrix_print_imath(const ex &e, const print_context& c) {
  c.s << "func diag( ";
  e.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(diagmatrix, eval_func(diagmatrix_eval).
                              print_func<imathprint>(diagmatrix_print_imath));

static ex identmatrix_eval(const ex &r, const ex &c) {
  // Immediately evaluate if r and c reduce to positive integers
  MSG_INFO(3, "ident: " << r << ", " << c << endline);

  if (is_a<numeric>(r) && is_a<numeric>(c)) {
    const numeric& rows = ex_to<numeric>(r);
    const numeric& cols = ex_to<numeric>(c);
    if (rows.info(info_flags::posint) && cols.info(info_flags::posint))
      return unit_matrix(rows.to_int(), cols.to_int());
  }

  return dynallocate<func>("ident", exprseq{r,c}).hold();
}

static void identmatrix_print_imath(const ex &r, const ex& col, const print_context& c) {
  c.s << "func ident( ";
  r.print(c);
  c.s << ", ";
  col.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(identmatrix, eval_func(identmatrix_eval).
                               print_func<imathprint>(identmatrix_print_imath));

static ex onesmatrix_eval(const ex &r, const ex &c) {
  // Immediately evaluate if r and c reduce to positive integers
  MSG_INFO(3, "ones: " << r << ", " << c << endline);

  if (is_a<numeric>(r) && is_a<numeric>(c)) {
    const numeric& rows = ex_to<numeric>(r);
    const numeric& cols = ex_to<numeric>(c);
    if (rows.info(info_flags::posint) && cols.info(info_flags::posint)) {
      matrix result(rows.to_int(), cols.to_int());
      for (size_t rr = 0; rr < result.rows(); ++rr)
        for (size_t cc = 0; cc < result.cols(); ++cc)
          result(rr,cc) = _ex1;
      return result;
    }
  }

  return dynallocate<func>("ones", exprseq{r,c}).hold();
}

static void onesmatrix_print_imath(const ex &r, const ex& col, const print_context& c) {
  c.s << "func ones( ";
  r.print(c);
  c.s << ", ";
  col.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(onesmatrix, eval_func(onesmatrix_eval).
                              print_func<imathprint>(onesmatrix_print_imath));

static ex submatrix_eval(const ex &e, const ex &r, const ex &nr, const ex& c, const ex& nc) {
  // Immediately evaluate if e is a matrix and r, nr, c, nc reduce to positive integers
  MSG_INFO(3, "submatrix: " << e << ", " << r << ", " << nr << ", " << c << ", " << nc << endline);

  if (is_a<matrix>(e) && is_a<numeric>(r) && is_a<numeric>(nr) && is_a<numeric>(c) && is_a<numeric>(nc)) {
    const matrix& m = ex_to<matrix>(e);
    const numeric& row = ex_to<numeric>(r);
    const numeric& nrows = ex_to<numeric>(nr);
    const numeric& col = ex_to<numeric>(c);
    const numeric& ncols = ex_to<numeric>(nc);
    if (row.info(info_flags::posint) && nrows.info(info_flags::posint) && col.info(info_flags::posint) && ncols.info(info_flags::posint))
      return sub_matrix(m, row.to_int() - 1, nrows.to_int(), col.to_int() - 1, ncols.to_int());
  }

  return dynallocate<func>("submatrix", exprseq{e,r,nr,c,nc}).hold();
}

REGISTER_FUNCTION(submatrix, eval_func(submatrix_eval));

static ex reducematrix_eval(const ex &e, const ex &r, const ex& c) {
  // Immediately evaluate if e is a matrix and r and c reduce to positive integers
  MSG_INFO(3, "reducematrix: " << e << ", " << r << ", " << c << endline);

  if (is_a<matrix>(e) && is_a<numeric>(r) && is_a<numeric>(c)) {
    const matrix& m = ex_to<matrix>(e);
    const numeric& rows = ex_to<numeric>(r);
    const numeric& cols = ex_to<numeric>(c);
    if (rows.info(info_flags::posint) && cols.info(info_flags::posint))
      return reduced_matrix(m, rows.to_int() - 1, cols.to_int() - 1);
  }

  return dynallocate<func>("reducematrix", exprseq{e,r,c}).hold();
}

REGISTER_FUNCTION(reducematrix, eval_func(reducematrix_eval));

static ex determinant_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "determinant: " << e << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).determinant();

  return dynallocate<func>("det", exprseq{e}).hold();
}

static void determinant_print_imath(const ex &e, const print_context& c) {
  c.s << "lline ";
  e.print(c);
  c.s << " rline";
}

REGISTER_FUNCTION(determinant, eval_func(determinant_eval).
                               print_func<imathprint>(determinant_print_imath));

static ex trace_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "trace: " << e << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).trace();

  return dynallocate<func>("tr", exprseq{e}).hold();
}

static void trace_print_imath(const ex &e, const print_context& c) {
  c.s << "func tr(";
  e.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(trace, eval_func(trace_eval).
                         print_func<imathprint>(trace_print_imath));

static ex charpoly_eval(const ex &e, const ex& var) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "charpoly: " << e << " in variable " << var << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).charpoly(var);

  return dynallocate<func>("charpoly", exprseq{e, var}).hold();
}

static void charpoly_print_imath(const ex &e, const ex& var, const print_context& c) {
  c.s << "func charpoly( ";
  e.print(c);
  c.s << ", ";
  var.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(charpoly, eval_func(charpoly_eval).
                            print_func<imathprint>(charpoly_print_imath));

static ex rank_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "rank: " << e << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).rank();

  return dynallocate<func>("rank", exprseq{e}).hold();
}

REGISTER_FUNCTION(rank, eval_func(rank_eval));

static ex solvematrix_eval(const ex &e, const ex& vars, const ex& rhs) {
  // Immediately evaluate if arguments are matching matrices
  MSG_INFO(3, "solvematrix: " << e << " for " << vars << " with right-hand side " << rhs << endline);

  if (is_a<matrix>(e) && is_a<matrix>(vars) && is_a<matrix>(rhs)) {
    const matrix& m = ex_to<matrix>(e); // Dimensions m x n
    const matrix& v = ex_to<matrix>(vars); // Dimensions n x p
    const matrix& r = ex_to<matrix>(rhs); // Dimensions m x p

    if (m.cols() == v.rows() && r.cols() == v.cols() && r.rows() == m.rows())
      return m.solve(v, r);
  }

  return dynallocate<func>("solvematrix", exprseq{e,vars,rhs}).hold();
}

REGISTER_FUNCTION(solvematrix, eval_func(solvematrix_eval));

static ex invertmatrix_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "invertmatrix: " << e << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).inverse();

  return dynallocate<func>("invertmatrix", exprseq{e}).hold();
}

static void invertmatrix_print_imath(const ex &e, const print_context& c) {
  c.s << "func inv(";
  e.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(invertmatrix, eval_func(invertmatrix_eval).
                                print_func<imathprint>(invertmatrix_print_imath));

static ex matrixrows_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "matrixrows: " << e << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).rows();

  return dynallocate<func>("matrixrows", exprseq{e}).hold();
}

static void matrixrows_print_imath(const ex &e, const print_context& c) {
  c.s << "func rows(";
  e.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(matrixrows, eval_func(matrixrows_eval).
                              print_func<imathprint>(matrixrows_print_imath));

static ex matrixcols_eval(const ex &e) {
  // Immediately evaluate if e is a matrix
  MSG_INFO(3, "matrixcols: " << e << endline);

  if (is_a<matrix>(e))
    return ex_to<matrix>(e).cols();

  return dynallocate<func>("matrixcols", exprseq{e}).hold();
}

static void matrixcols_print_imath(const ex &e, const print_context& c) {
  c.s << "func cols(";
  e.print(c);
  c.s << ")";
}

REGISTER_FUNCTION(matrixcols, eval_func(matrixcols_eval).
                              print_func<imathprint>(matrixcols_print_imath));

void func::init() {
  if (initialized) return;
  functions = std::map<const std::string, funcrec>();
  hard_names =  std::map<const std::string, std::string>();
  hard_names_rev = std::map<const std::string, std::string>();
  func_inv = std::map<std::string, std::string>();

  // Initialize map of functions known by Latex and/or GiNaC.
  // TODO: complete this list both from the list of functions starmath recognizes and the list of functions GiNaC has built-in
  MSG_INFO(2, "Initializing function names..." << endline);

  hard_names["abs"] = "abs";
  hard_names["arccos"] = "acos";
  hard_names["arcosh"] = "acosh";
  hard_names["arcsin"] = "asin";
  hard_names["arsinh"] = "asinh";
  hard_names["arctan"] = "atan";
  //hard_names["", aa, "atan2", 2);
  hard_names["artanh"] = "atanh";
  //hard_names["arg"]    = "";
  //hard_names["binom"] = "binomial";
  hard_names["ceil"] = "ceil";
  hard_names["charpoly"] = "charpoly";
  hard_names["concat"] = "concat";
  hard_names["conjugate"] = "conjugate";
  hard_names["cos"] = "cos";
  hard_names["cosh"] = "cosh";
  //hard_names["csgn"] = "csgn";
  //hard_names["deg"] = "";
  hard_names["det"]    = "determinant";
  hard_names["diag"]   = "diagmatrix";
  //hard_names["dim"]    = "";
  hard_names["exp"] = "exp";
  hard_names["fact"] = "factorial";
  hard_names["floor"] = "floor";
  //hard_names["gcd"]    = "";
  hard_names["hadamard"] = "hadamard";
  //hard_names["hom"]    = "";
  hard_names["ident"] = "identmatrix";
  hard_names["ifelse"] = "ifelse";
  hard_names["im"] = "imag_part";
  hard_names["Im"] = "imag_part";
  //hard_names["inf"]    = "";
  hard_names["invertmatrix"] = "invertmatrix";
  //hard_names["ker"]    = "";
  //hard_names["lg"]     = "";
  //hard_names["lim"]    = "";
  //hard_names["liminf"] = "";
  //hard_names["limsup"] = "";
  hard_names["ln"] = "log";
  hard_names["matrixcols"] = "matrixcols";
  hard_names["matrixrows"] = "matrixrows";
  //hard_names["max"]    = ""; // see vmax
  //hard_names["min"]    = ""; // see vmin
  hard_names["mindex"] = "mindex";
  hard_names["ones"] = "onesmatrix";
  //hard_names["Pr"]     = "";
  hard_names["rank"] = "rank";
  hard_names["re"] = "real_part";
  hard_names["Re"] = "real_part";
  hard_names["reducematrix"] = "reducematrix";
  hard_names["round"] = "round";
  hard_names["scalprod"] = "scalprod";
  hard_names["sin"] = "sin";
  hard_names["sinh"] = "sinh";
  hard_names["solvematrix"] = "solvematrix";
  hard_names["submatrix"] = "submatrix";
  hard_names["sum"] = "sum";
  //hard_names["sup"]    = "";
  //hard_names["step"] = "step";
  hard_names["tan"] = "tan";
  hard_names["tanh"] = "tanh";
  hard_names["tr"] = "trace";
  hard_names["transpose"] = "transpose";
  hard_names["vecprod"] = "vecprod";
  hard_names["vmax"] = "vmax";
  hard_names["vmin"] = "vmin";
  // !!!!!!!!!!!! Remember to add these functions to init.imath!!!!!!!!

  // Create reverse map
  for (const auto& i : hard_names) hard_names_rev.emplace(i.second, i.first);

  // Initialize list of inverse functions
  func_inv["sin"]  = "arcsin";
  func_inv["sinh"] = "arsinh";
  func_inv["cos"]  = "arccos";
  func_inv["cosh"] = "arcosh";
  func_inv["tan"]  = "arctan";
  func_inv["tanh"] = "artanh";
  func_inv["cot"]  = "arccot";
  func_inv["ln"]   = "exp";
  func_inv["conjugate"] = "conjugate";
  func_inv["transpose"] = "transpose";
  func_inv["invertmatrix"] = "invertmatrix";
  for (const auto& i : func_inv)
    func_inv.emplace(i.second, i.first);

  initialized = true;
}

func_unarchiver::func_unarchiver() {}
func_unarchiver::~func_unarchiver() {}

funcrec::funcrec(const unsigned h) : hints(h) {
  MSG_INFO(3, "Constructing empty funcrec with hints" << endline);
}

#ifdef DEBUG_CONSTR_DESTR
  funcrec::funcrec() {
    MSG_INFO(3, "Constructing empty funcrec" << endline);
  }
  funcrec::funcrec(const funcrec& other) {
    MSG_INFO(3, "Copying funcrec from serial " << other.serial << endline);
    serial = other.serial;
    vars = exvector(other.vars.begin(), other.vars.end());
    definition = other.definition;
    hard = other.hard;
    hints = other.hints;
  }
  funcrec& funcrec::operator=(const funcrec& other) {
    MSG_INFO(3, "Assigning funcrec from serial " << other.serial << endline);
    serial = other.serial;
    vars = exvector(other.vars.begin(), other.vars.end());
    definition = other.definition;
    hard = other.hard;
    hints = other.hints;
    return *this;
  }
  funcrec::~funcrec() {
    MSG_INFO(3, "Destructing funcrec with serial " << serial << endline);
  }
#endif

}
