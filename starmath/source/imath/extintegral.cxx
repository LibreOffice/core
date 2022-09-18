/*************************************************************************
    extintegral.cpp  -  class extintegral, extending the GiNaC integral class
                             -------------------
    begin                : Sun Jan 26 2014
    copyright            : (C) 2014 by Jan Rheinlaender
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

#include "extintegral.hxx"
#include "func.hxx"
#include "differential.hxx"
#include "printing.hxx"
#include "exderivative.hxx"
#include <ginac/symbol.h>
#include <ginac/wildcard.h>
#include <ginac/operators.h>
#include <ginac/add.h>
#include <ginac/mul.h>
#include <ginac/relational.h>
#include <ginac/power.h>
#include "func.hxx"
#include "msgdriver.hxx"

using namespace std;

namespace GiNaC {

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(extintegral, integral,
  print_func<print_context>(&extintegral::do_print).
  print_func<imathprint>(&extintegral::do_print_imath))


//////////
// default constructor
//////////

extintegral::extintegral()
  : integral()
{
  MSG_INFO(3, "Constructing empty extintegral" << endline);
  C = dynallocate<symbol>("C");
  hasboundaries = false;
}


//////////
// other constructors
//////////

// public

extintegral::extintegral(const ex & x_, const ex & a_, const ex & b_, const ex & f_)
    :  integral()
{
  MSG_INFO(3, "Constructing extintegral with boundaries from " << f_ << endline);
  let_op(0) = x_; // Avoid exception if x_ is not a symbol
  let_op(1) = a_;
  let_op(2) = b_;
  let_op(3) = f_;
  C = dynallocate<symbol>("C");
  hasboundaries = true;
}

extintegral::extintegral(const ex & x_, const ex & f_, const ex& C_)
  : integral()
{
  MSG_INFO(3, "Constructing extintegral without boundaries from " << f_ << endline);
  let_op(0) = x_; // Avoid exception if x_ is not a symbol
  let_op(1) = _ex0;
  let_op(2) = _ex0;
  let_op(3) = f_;
  C = C_;
  hasboundaries = false;
}

extintegral::extintegral(const ex & f__, const ex& C_)
    : integral()
{
  MSG_INFO(3, "Constructing extintegral from expression " << f__ << endline);
  hasboundaries = false;
  C = C_;

  ex f_ = GiNaC::collect_common_factors(f__);

  for (const auto& m : (is_a<mul>(f_) ? f_ : lst{f_})) {
    if (is_a<differential>(m)) {
      let_op(0) = ex_to<differential>(m).argument();
      let_op(3) = f_ / m;
      MSG_INFO(2, "Created extintegral of expression " << op(3) << " to variable " << op(0) << endline);
      return;
    } else if (is_a<power>(m)) {
      ex basis = get_basis(ex_to<power>(m));
      if (is_a<differential>(basis)) {
        let_op(0) = ex_to<differential>(basis).argument();
        let_op(3) = f_ / basis;
        MSG_INFO(2, "Created extintegral of expression " << op(3) << " to variable " << op(0) << endline);
        return;
      }
    }
  }

  throw std::runtime_error("Cannot create integral from an expression that contains no differential");
}

extintegral::extintegral(const integral& other)
    : integral(other.op(0), other.op(1), other.op(2), other.op(3))
{
  MSG_INFO(3, "Constructing extintegral from integral " << other << endline);
  hasboundaries = true;
  C = dynallocate<symbol>("C");
}

#ifdef DEBUG_CONSTR_DESTR
  extintegral::extintegral(const extintegral& other) : integral(other.op(0), other.op(1), other.op(2), other.op(3)) {
    MSG_INFO(3, "Copying extintegral from " << other << endline);
    C = other.C;
    hasboundaries = other.hasboundaries;
  }
  extintegral& extintegral::operator=(const extintegral& other) {
    MSG_INFO(3, "Assigning extintegral from " << other << endline);
    integral::operator=(other);
    hasboundaries = other.hasboundaries;
    return *this;
  }
  extintegral::~extintegral() {
    MSG_INFO(3, "Destructing extintegral " << *this << endline);
  }
#endif

//////////
// functions overriding virtual functions from base classes
//////////

void extintegral::do_print(const print_context & c, unsigned level) const
{
  if (hasboundaries)
    return integral::do_print(c, level);

  c.s << "extintegral(";
  op(0).print(c);
  c.s << ",";
  op(3).print(c);
  c.s << ")";
}

void extintegral::do_print_imath(const imathprint& c, unsigned level) const
{
  if (!hasboundaries) {
    c.s << "int {";
  } else {
    c.s << "int from {";
    op(0).print(c, level+1);
    c.s << " = ";
    op(1).print(c, level+1);
    c.s << "} to {";
    op(2).print(c, level+1);
    c.s << "} {";
  }
  if (is_a<add>(op(3))) c.s << "(";
  op(3).print(c, level+1);
  if (is_a<add>(op(3))) c.s << ")";
  c.s << "} ";
  differential(op(0)).do_print_imath(c, level+1, false);
}

int extintegral::compare_same_type(const basic & other) const
{
  const extintegral &o = static_cast<const extintegral &>(other);
  MSG_INFO(3, "Comparing " << *this << " and " << o << endline);

  if (hasboundaries == o.hasboundaries)
    return integral::compare_same_type(o);
  else
    return 1;
}

void extintegral::set_integration_constant(const symbol& newconstant)
{
  C = newconstant;
  MSG_INFO(1, "Set new integration constant: " << C << endline);
}

ex ensure_extintegral(const ex& e, const unsigned& flags)
{
  (void)flags;
  if (is_a<integral>(e))
    // Note: The only flags passed in the code are ::dynallocated
    return dynallocate<extintegral>(ex_to<integral>(e));
  else
    return e;
}

// Note: The default GiNaC eval() behaviour has been changed to NEVER evaluate
// an integral here (meaning that the expression returned is always an extintegral)
// This is for consistency in the user interface: Evaluation will always take
// place in a simplify(..., "integrate") call
#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
ex extintegral::eval() const {
  MSG_INFO(2, "Doing eval of extintegral" << endline);
  if (flags & status_flags::evaluated) return *this;

  if (op(3).is_zero()) return C;

  return this->hold();
}
#else
ex extintegral::eval(int level) const {
  if ((level==1) && (flags & status_flags::evaluated))
    return *this;
  if (level == -max_recursion_level)
    throw(std::runtime_error("max recursion level reached"));

  ex eintvar = (level==1) ? op(0) : op(0).eval(level-1);
  ex ef      = (level==1) ? op(3) : op(3).eval(level-1);

  if (ef.is_zero())
    return ef;

  if (hasboundaries) {
    ex ea      = (level==1) ? op(1) : op(1).eval(level-1);
    ex eb      = (level==1) ? op(2) : op(2).eval(level-1);

    if (are_ex_trivially_equal(eintvar,op(0)) && are_ex_trivially_equal(ea,op(1))
                    && are_ex_trivially_equal(eb,op(2)) && are_ex_trivially_equal(ef,op(3)))
            return this->hold();
    return (new extintegral(eintvar, ea, eb, ef))
            ->setflag(status_flags::dynallocated | status_flags::evaluated);
  } else {
    if (are_ex_trivially_equal(eintvar,op(0)) && are_ex_trivially_equal(ef,op(3)))
      return this->hold();
    return (new extintegral(eintvar, ef, C))
      ->setflag(status_flags::dynallocated | status_flags::evaluated);
  }
}
#endif

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
ex extintegral::evalf() const {
  if (!hasboundaries)
    throw(runtime_error("Integral without boundaries cannot be evaluated numerically"));

  // TODO: This can potentially throw an exception if the integral does not evaluate and x is not a symbol
  // TODO: expression::evalf() will not be called here
  return ensure_extintegral(integral::evalf(), status_flags::dynallocated);
}
#else
ex extintegral::evalf(int level) const
{
  if (!hasboundaries)
    throw(runtime_error("Integral without boundaries cannot be evaluated numerically"));

  // TODO: This can potentially throw an exception if the integral does not evaluate and x is not a symbol
  // TODO: expression::evalf() will not be called here
  return ensure_extintegral(integral::evalf(level), status_flags::dynallocated);
}
#endif

ex extintegral::evalm() const {
  ex eintvar = expression(op(0)).evalm();
  ex ef      = expression(op(3)).evalm();

  if (ef.is_zero())
    return ef;

  if (hasboundaries) {
    ex ea = expression(op(1)).evalm();
    ex eb = expression(op(2)).evalm();

    if (are_ex_trivially_equal(eintvar,op(0)) && are_ex_trivially_equal(ea,op(1))
                    && are_ex_trivially_equal(eb,op(2)) && are_ex_trivially_equal(ef,op(3)))
      return this->hold();
    return dynallocate<extintegral>(eintvar, ea, eb, ef);
  } else {
    if (are_ex_trivially_equal(eintvar,op(0)) && are_ex_trivially_equal(ef,op(3)))
      return this->hold();
    return dynallocate<extintegral>(eintvar, ef, C);
  }
}

int extintegral::degree(const ex & s) const
{
  if (hasboundaries && is_a<symbol>(op(0)))
    return integral::degree(s);
  else
    return op(3).degree(s);
}

int extintegral::ldegree(const ex & s) const
{
  if (hasboundaries && is_a<symbol>(op(0)))
    return integral::degree(s);
  else
    return op(3).ldegree(s);
}

ex extintegral::expand(unsigned options) const
{
  if (options==0 && (flags & status_flags::expanded))
    return *this;

  ex newf = op(3).expand(options);

  if (hasboundaries) {
    ex newa = op(1).expand(options);
    ex newb = op(2).expand(options);

    if (are_ex_trivially_equal(op(1), newa) && are_ex_trivially_equal(op(2), newb) && are_ex_trivially_equal(op(3), newf)) {
      if (options==0)
              this->setflag(status_flags::expanded);
      return *this;
    }

    const basic & newint = (new extintegral(op(0), newa, newb, newf))->setflag(status_flags::dynallocated);
    if (options == 0)
      newint.setflag(status_flags::expanded);
    return newint;
  } else {
    if (are_ex_trivially_equal(op(3), newf)) {
      if (options==0)
        this->setflag(status_flags::expanded);
      return *this;
    }

    const basic & newint = (new extintegral(op(0), newf, C))->setflag(status_flags::dynallocated);
    if (options == 0)
      newint.setflag(status_flags::expanded);
    return newint;
  }
}

ex extintegral::derivative(const symbol & s) const
{
  if (s==op(0))
    throw(logic_error("differentiation with respect to dummy variable"));

  if (hasboundaries) {
    ex _x = op(0);
    ex _a = op(1);
    ex _b = op(2);
    ex _f = op(3);
    return
    expression(_b).diff(s)*expression(_f.subs(_x==_b))-expression(_a).diff(s)*expression(_f.subs(_x==_a))+extintegral(_x, _a, _b, expression(_f).diff(s));
  } else {
    return dynallocate<extintegral>(op(0), expression(op(3)).diff(s), C);
  }
}

ex extintegral::conjugate() const
{
  if (hasboundaries && is_a<symbol>(op(0)))
    return ensure_extintegral(integral::conjugate(), status_flags::dynallocated);

  ex conjf = op(3).conjugate().subs(op(0).conjugate()==op(0));

  if (are_ex_trivially_equal(op(3), conjf))
    return *this;

  return dynallocate<extintegral>(op(0), conjf, C);
}

namespace integral_table {
symbol x;
ex a;
ex b;
ex c;
ex d;
ex e;
ex f;
ex g;
ex X1;
ex X2;
ex a2pX2;
ex a2mX2;
ex X2ma2;
ex a3pX3;
ex a3mX3;
ex a2pb2X;
ex a2mb2X;
ex n;
ex DELTA1;
ex DELTA2;
ex Y1p;
ex Y2p;
ex Y2m;
exmap integrals; // Cannot use exhashmap because it does not understand the initializer list

void init_table() {
  // Note: Doing this with static initializers fails for DLLs
  x = symbol("x");
  a = wild(1);
  b = wild(2);
  c = wild(3);
  d = wild(4);
  e = wild(5);
  f = wild(6);
  g = wild(7);
  X1 = a * x + b;
  X2 = a * pow(x, 2) + b * x + c;
  a2pX2 = pow(a,2) + pow(x,2);
  a2mX2 = pow(a,2) - pow(x,2);
  X2ma2 = pow(x,2) - pow(a,2);
  a3pX3 = pow(a,3) + pow(x,3);
  a3mX3 = pow(a,3) - pow(x,3);
  a2pb2X = pow(a,2) + pow(b,2) * x;
  a2mb2X = pow(a,2) + pow(b,2) * x;
  n = wild(8);
  DELTA1 = b * f - a * g;
  DELTA2 = 4 * a * c - pow(b,2);
  Y1p = func("arctan", x/a);
  Y2p = func("arctan", b * GiNaC::sqrt(x) / a);
  Y2m = func("ln", (a + b * GiNaC::sqrt(x)) / (a - b * GiNaC::sqrt(x))) / 2;

  // Taken from Bronstein/Semendjajew, Taschenbuch der Mathematik
  // Beware: The compiler views 1/2 as an integer expression with value 0!
  // Note: If sometime this table is statically initalized, we must use GiNaC::log() etc. and replace with
  // func objects at runtime, otherwise there is an exception "Function ... has not been registered"
  // Note: There is no guarantee that the table will be iterated in this order!
  integrals = {
        { a,                a * x },
        { pow(x, n),        1 / (n+1) * pow(x, n+1) },
/*001*/  { pow(X1, n),       1 / (a * (n+1)) * pow(X1, n+1) },
/*002*/  { pow(X1, -1),       1 / a * func("ln", X1) },
/*002*/ { pow(x, -1),       func("ln", x) },
/*003*/  { x * pow(X1, n),   1 / (pow(a,2) * (n+2)) * pow(X1, n+2) - b / (pow(a,2) * (n+1)) * pow(X1, n+1) },
/*004 is partial integration */
/*005*/ { x * pow(X1, -1),  x / a - b / pow(a,2) * func("ln", X1) },
/*006*/  { x * pow(X1, -2),  b / (pow(a,2) * X1) + 1 / pow(a,2) * func("ln", X1) },
/*007 is contained in 003 */
/*008*/ { x * pow(X1, -n),  1 / pow(a,2) * (-1 / ((n-2) * pow(X1, n-2)) + b / ((n-1) * pow(X1, n-1))) },
/*008*/ { x * pow(pow(X1, n), -1), 1 / pow(a,2) * (-1 / ((n-2) * pow(X1, n-2)) + b / ((n-1) * pow(X1, n-1))) },
/*009*/ { pow(x,2) * pow(X1, -1),  1 / pow(a,3) * (pow(X1, 2) / 2 - 2 * b * X1 + pow(b,2) * func("ln", X1)) },
/*010*/ { pow(x,2) * pow(X1, -2),  1 / pow(a,3) * (X1 - 2 * b * func("ln", X1) - pow(b,2) / X1) },
/*011*/ { pow(x,2) * pow(X1, -3),  1 / pow(a,3) * (func("ln", X1) + 2 * b / X1 - pow(b,2) / (2 * pow(X1, 2))) },
/*012*/ { pow(x,2) * pow(X1, -n),  1 / pow(a,3) * (-1 / ((n-3)*pow(X1, n-3)) + 2 * b / ((n-2) * pow(X1, n-2)) - pow(b,2) / ((n-1) * pow(X1, n-1))) },
/*012*/ { pow(x,2) * pow(pow(X1, n), -1),    1 / pow(a,3) * (-1 / ((n-3)*pow(X1, n-3)) + 2 * b / ((n-2) * pow(X1, n-2)) - pow(b,2) / ((n-1) * pow(X1, n-1))) },
/*018*/ { pow(x,-1) * pow(X1, -1), -1/b * func("ln", X1/x) },
/*031*/ { X1 * pow(f * x + g, -1), a * x / f + DELTA1 / pow(f,2) * func("ln", f * x + g) },
/*032*/ { pow(X1, -1) * pow(f * x + g, -1), 1/DELTA1 * func("ln", (f * x + g) * pow(X1, -1)) },
/*040*/ { pow(X2, -1),       2/GiNaC::sqrt(DELTA2) * func("arctan", (2 * a * x + b) / GiNaC::sqrt(DELTA2)) }, // TODO: Two results depending on DELTA2
/*041 to 056 are partial integrals depending on prior entries in the table */
/*057*/ { pow(a2pX2, -1),     1/a * Y1p }, // TODO negative branch depends on abs(x)
/*058*/ { pow(a2pX2, -2),     x / (2 * pow(a,2) * a2pX2) + 1/(2 * pow(a,3)) * Y1p }, // TODO negative branch depends on abs(x)
/*083*/ { pow(a3pX3, -1),               1 / (6 * pow(a,2)) * func("ln", pow(a + x, 2) / (pow(a,2) - a * x + pow(x,2))) + 1 / (pow(a,2) * pow(3, numeric(1,2))) * func("arctan", (2 * x - a) / (a * pow(3, numeric(1,2)))) },
/*083*/ { pow(a3mX3, -1),              -1 / (6 * pow(a,2)) * func("ln", pow(a - x, 2) / (pow(a,2) + a * x + pow(x,2))) + 1 / (pow(a,2) * pow(3, numeric(1,2))) * func("arctan", (2 * x + a) / (a * pow(3, numeric(1,2)))) },
/*083*/ { pow(pow(x,3)-pow(a,3), -1),   1 / (6 * pow(a,2)) * func("ln", pow(a - x, 2) / (pow(a,2) + a * x + pow(x,2))) - 1 / (pow(a,2) * pow(3, numeric(1,2))) * func("arctan", (2 * x + a) / (a * pow(3, numeric(1,2)))) },
/*109*/ { GiNaC::sqrt(x) / a2pb2X,  2 * GiNaC::sqrt(x) / pow(b,2) - 2 * a / pow(b,3) * Y2p },
/*109*/ { GiNaC::sqrt(x) / a2mb2X, -2 * GiNaC::sqrt(x) / pow(b,2) + 2 * a / pow(b,3) * Y2p },
/*121 is contained in 001 */
/*122*/ { x * GiNaC::sqrt(X1),     2 * (3 * a * x - 2 * b) * GiNaC::sqrt(pow(X1, 3)) / (15 * pow(a,2)) },
/*124 is contained in 001 */
/*125*/ { x / GiNaC::sqrt(X1),     2 * (a * x - 2 * b) / (3 * pow(a,2)) * GiNaC::sqrt(X1) },
/*128 is partial integral using 127 */
/*132*/ { GiNaC::sqrt(pow(X1, 3)), 2 * GiNaC::sqrt(pow(X1,5)) / (5 * a) },
/*140 is contained in 001 */
/*157*/ { GiNaC::sqrt(a2mX2), (x * GiNaC::sqrt(a2mX2) + pow(a,2) * func("arcsin", x/a)) / 2 },
/*164*/ { pow(GiNaC::sqrt(a2mX2), -1), func("arcsin", x/a) },
/*185*/ { GiNaC::sqrt(a2pX2), (x * GiNaC::sqrt(a2pX2) + pow(a,2) * func("arsinh", x/a)) / 2 },
/*186*/ { x * GiNaC::sqrt(a2pX2),   GiNaC::sqrt(pow(a2pX2, 3))/3 },
/*213*/ { GiNaC::sqrt(X2ma2), (x * GiNaC::sqrt(X2ma2) - pow(a,2) * func("arcosh", x/a))/ 2 },
/*241 has 4 different cases depending on a and DELTA2 */
/*245 is partial integral using 214 */
/*268 is contained in 001 */
/*274 is a basic function */
/*275*/ { pow(func("sin", a * x), 2), x / 2 - func("sin", 2 * a * x) / (4 * a) },
/*345*/ { pow(1 - pow(func("cos", a * x), 2), -1), -1 / (func("tan", a * x) * a) },
/*345*/ { pow(pow(func("cos", a * x), 2) - 1, -1),  1 / (func("tan", a * x) * a) },
/*354*/ { ex(func("sin", a * x)) * func("cos", a * x), pow(func("sin", a * x), 2) / (2 * a) },
/*409 is a basic function */
/*410*/ { pow(func("tan", a * x), 2),  func("tan", a * x) / a - x },
/*418 is a basic function */
/*426 is a basic function */
/*427 is a basic function */
/*436 is a basic function */
/*437 is a basic function */
/*438*/ { pow(func("tanh", a * x), 2), x - func("tanh", a * x) / a },
/*447 is a basic function */
/*448*/ { x * ex(func("exp", a * x)), func("exp", a * x) / pow(a, 2) * (a * x - 1) },
/*---*/ { pow(a, b * x + c), pow(a, b * x + c) / (b * func("ln", a)) },
/*---*/ { pow(a, x), pow(a, x) / func("ln", a) }
/*465 is a basic function */
/*488 is a basic function */
/*493 is a basic function */
/*498 is a basic function */
/*505 is a basic function */
/*512 is a basic function */
/*513 is a basic function */
/*514 is a basic function */
/*515 is a basic function */
};
} // init_table()
} // namespace integral_table

/**
   * Try to integrate
   * @param fun The function to integrate
   * @param var The integration variable
   * @param constfactor Constant factors pulled out of the integral. Must be initialized with _ex1
   * @param nonintegrable The part of the expression that was not integrable (without constant factors). Must be initialized with _ex0
   * @returns The part of the expression that was integrated
   **/
ex find_integral(const ex& fun, const ex& var, ex& constfactor, ex& nonintegrable)
{
  static bool table_initialized = false;
  if (!table_initialized) {
    MSG_INFO(1, "Initializing integral table" << endline);
    integral_table::init_table();
    table_initialized = true;
  }

  MSG_INFO(1, "Finding integral for " << fun << " in variable " << var << endline);
  // Check trivial case
  if (!fun.has(var))
    return fun * var;

  // Pull out constant factors
  ex remainder(_ex1);
  if (is_a<mul>(fun)) {
    for (size_t i=0; i<fun.nops(); ++i) {
      if (fun.op(i).has(var))
        remainder *= fun.op(i);
      else
        constfactor *= fun.op(i);
    }
  } else {
    remainder = fun;
  }

  MSG_INFO(1, "Found constfactor " << constfactor << " and remainder " << remainder << endline);

  if (is_a<extintegral>(remainder)) { // Handle multiple integrals
    ex subresult = ex_to<extintegral>(remainder).eval_integ(); // The result may or may not be integrated!
    remainder = _ex0;

    for (const auto& e : (is_a<add>(subresult) ? subresult : lst{subresult})) {
      if (is_a<extintegral>(e))
        nonintegrable += e;
      else
        remainder += e;
    }
  }

  if (remainder.is_equal(var)) {
    return _ex1_2 * pow(var, _ex2);
  } else if (is_a<add>(remainder)) {
    ex result(_ex0);

    for (const auto& part : remainder) {
      ex partconstfactor(_ex1);
      ex partnonintegrable(_ex0);
      ex partintegrated = find_integral(part, var, partconstfactor, partnonintegrable);
      MSG_INFO(1, "Part integrated: " << partintegrated << ", nonintegrable: " << partnonintegrable << ", constfactor: " << partconstfactor << endline);
      nonintegrable += partconstfactor * partnonintegrable;
      result += partconstfactor * partintegrated;
    }

    return result;
  } else if (is_a<func>(remainder)) {
    ex result;
    if (ex_to<func>(remainder).find_integral(var, result))
      return result;
  } else if (is_a<exderivative>(remainder)) {
    const exderivative& e = ex_to<exderivative>(remainder);
    if (!e.is_partial() && is_a<differential>(e.get_denom())) {
      const differential& d = ex_to<differential>(e.get_denom());
      if (d.argument().is_equal(var))
        return e.get_numer().argument(); // int df/dt dt = df
    }
  } else {
    exmap repl;

    for (const auto& i : integral_table::integrals) {
      MSG_INFO(1, "Checking match of " << remainder << " with " << i.first << " --> " << i.second << endline);
      try {
        if (remainder.match(i.first.subs(integral_table::x == var), repl)) {
          MSG_INFO(1, "Found match with " << repl << endline);
          bool nonconst = false;

          for (const auto& r : repl)
            if (r.second.has(var)) {
              nonconst = true;
              repl.clear();
              break; // non-constant factor
            }

          if (!nonconst) {
            func::replace_function_by_func replace_functions;
            return replace_functions(i.second.subs(repl, subs_options::no_pattern).subs(integral_table::x == var));
          }
        }
      } catch (std::exception& e) {
        (void)e;
        // ignore, might happen when substitution leads to division by zero
        MSG_INFO(1, "Discarding match because of exception thrown during substitution" << endline);
      }
    }
  }

  nonintegrable = remainder;
  return _ex0;
}

ex extintegral::subs(const exmap & m, unsigned options) const {
  MSG_INFO(2, "Substituting " << m << " in integral " << *this << endline);
  ex result = inherited::subs(m, options);

  // Handle substitution of the differential
  if (is_a<extintegral>(result)) {
    for (const auto& r : m) {
      if (is_a<differential>(r.first)) {
        const differential& lhs = ex_to<differential>(r.first);
        if (lhs.op(0).is_equal(result.op(0))) {
          extintegral res(op(3) * r.second, C); // Construct new extintegral from expression
          res.hasboundaries = hasboundaries;
          if (hasboundaries) {
            res.let_op(1) = op(1);
            res.let_op(2) = op(2);
          }
          return res;
        }
      }
    }
  }

  return result;
}

ex extintegral::eval_integ() const
{
  MSG_INFO(1, "eval_integ() for " << *this << endline);
  const ex& var = op(0);
  ex fun = op(3);
  if (is_a<extintegral>(fun)) // Handle multiple integrals
    fun = ex_to<extintegral>(fun).eval_integ();

  ex constfactor(_ex1);
  ex nonintegrable(_ex0);
  ex integrated = find_integral(fun, var, constfactor, nonintegrable);
  MSG_INFO(1, "Integrated: " << integrated << ", nonintegrable: " << nonintegrable << ", constfactor: " << constfactor << endline);

  if (integrated.is_zero() && constfactor.is_equal(_ex1))
      return *this;

  ex result(_ex0);
  if (!nonintegrable.is_zero()) {
    if (hasboundaries)
      result = constfactor * dynallocate<extintegral>(op(0), op(1), op(2), nonintegrable);
    else
      result = constfactor * dynallocate<extintegral>(op(0), nonintegrable, C);
  }
  if (!integrated.is_zero()) {
    if (hasboundaries)
      result += constfactor * (integrated.subs(var==op(2)) - integrated.subs(var==op(1)));
    else
      result += constfactor * integrated + C;
  }

  MSG_INFO(1, "Result of eval_integ(): " << result << endline);
  return result;
}

extintegral_unarchiver::extintegral_unarchiver() {}
extintegral_unarchiver::~extintegral_unarchiver() {}
}

