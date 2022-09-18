/***************************************************************************
                          equation.cpp  -  Class for handling equations
                             -------------------
    begin                : Sun Oct 21 2001
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

#include "equation.hxx"
#include "unit.hxx"
#include "printing.hxx"
#include "expression.hxx"
#include "func.hxx"
#include "msgdriver.hxx"
#include <sstream>
#include "utils.hxx"

namespace GiNaC {

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(equation, relational,
  print_func<print_context>(&equation::do_print).
  print_func<imathprint>(&equation::do_print_imath));

  // constructors
  equation::equation() : inherited(_ex0, _ex0, relational::equal), mod(_expr0) {
    MSG_INFO(3, "Constructing empty equation" << endline);
  }

  equation::equation(const expression &le, const expression &ri, const operators op,
                     const expression& m)
    : inherited(le, ri, op), mod(m) {
    MSG_INFO(3, "Constructing equation from " << le << " = " << ri << endline);
  }

  equation::equation(const equation &eq) : inherited(eq.lh, eq.rh, eq.o), mod(eq.mod) {
      MSG_INFO(3, "Copying equation from " << eq.lh << " = " << eq.rh  << endline);
    }

  equation::equation(const relational &r, const expression& m)
    : inherited(r.lhs(), r.rhs(), relational::equal), mod(m) {
    MSG_INFO(3, "Constructing equation from relational " << r << endline);
    // Unfortunately, we cannot extract the operator from the relational directly
    if (r.info(info_flags::relation_equal))
      o = relational::equal;
    else if (r.info(info_flags::relation_not_equal))
      o = relational::not_equal;
    else if (r.info(info_flags::relation_less))
      o = relational::less;
    else if (r.info(info_flags::relation_less_or_equal))
      o = relational::less_or_equal;
    else if (r.info(info_flags::relation_greater))
      o = relational::greater;
    else if (r.info(info_flags::relation_greater_or_equal))
      o = relational::greater_or_equal;
  }

  equation &equation::operator=(const equation &other) {
    MSG_INFO(3, "Assigning equation from " << other.lh << " = " << other.rh << endline);
    lh = other.lh;
    rh = other.rh;
    o = other.o;
    mod = other.mod;
    return *this;
  }

#ifdef DEBUG_CONSTR_DESTR
  equation::~equation() {
    MSG_INFO(3, "Destructing equation " << lh << " = " << rh << endline);
  }
#endif

  unsigned equation::return_type() const {
    if ((lh.return_type() != return_types::commutative) ||
        (rh.return_type() != return_types::commutative))
      return return_types::noncommutative_composite;
    else
      return return_types::commutative;
  }

  equation_unarchiver::equation_unarchiver() {}
  equation_unarchiver::~equation_unarchiver() {}

  int equation::compare_same_type(const basic &other) const {
    return inherited::compare_same_type(other);
  }

  // methods
  bool equation::is_assignment() const {
    if (msg::info().checkprio(6)) {
      msg::info() << endline;
      std::ostringstream os;
      os << tree << rh << dflt;
      msg::info() << os.str();
      msg::info() << endline;
    }
    if (!is_a<symbol>(lh) || (o != relational::equal)) return false;
    // parse the expression tree and check that it contains only numerics and Units.
    return is_quantity(rh);
  }

  void equation::do_print(const print_context &c, unsigned level) const {
    (void)level;
    c.s << lh << get_oper(c, o, mod) << rh;
    if (!mod.is_zero()) c.s << " (mod " << mod << ")";
  }

  void equation::do_print_imath(const imathprint &c, unsigned level) const {
    MSG_INFO(4, "equation::do_print_imath()" << endline);
    (void)level;
    if (is_a<Unit>(lh))
      c.s << "1 ";
    expression(lh).print(c, level+1);
    std::string oper = get_oper(c, o, mod);
    c.s << " " << oper << " "; // Spaces are required for the case y <-x which will print an arrow instead of y < -x
    if (is_a<Unit>(rh))
      c.s << "1 ";
    expression(rh).print(c, level+1);
    if (!mod.is_zero()) {
      c.s << " (\"mod\" ";
      expression(mod).print(c, level+1);
      c.s << ")";
    }
  }

  ex equation::subs(const ex &e, unsigned options) const {
    return dynallocate<equation>(expression(lh).subs(e, options), expression(rh).subs(e, options), o, mod);
  }

  ex equation::subs(const exmap &m, unsigned options) const {
    return dynallocate<equation>(expression(lh).subs(m, options), expression(rh).subs(m, options), o, mod);
  }

  expression equation::subsv(const expression &e, unsigned options) const {
    // Note: This will return an equation with a vector on LHS and RHS
    //return dynallocate<equation>(expression(lh).subsv(e, false, options), expression(rh).subsv(e, false, options), o, mod);
    // This returns a vector of equations
    return expression(*this).subsv(e, false, options);
  }

  expression equation::csubs(const exmap &m, unsigned options) const {
    return dynallocate<equation>(expression(lh).csubs(m, options), expression(rh).csubs(m,options), o, mod);
  }

  expression equation::csubs(const ex &e, unsigned options) const {
    return dynallocate<equation>(expression(lh).csubs(e, options), expression(rh).csubs(e, options), o, mod);
  }

  expression equation::csubsv(const expression &e, unsigned options) const {
    // Note: This will return an equation with a vector on LHS and RHS
    //return dynallocate<equation>(expression(lh).subsv(e, true, options), expression(rh).subsv(e, true, options), o, mod);
    // This returns a vector of equations
    return expression(*this).subsv(e, true, options);
  }

  ex equation::expand(unsigned options) const {
    MSG_INFO(3, "Expanding " << *this << endline);
    return dynallocate<equation>(expression(lh).expand(options), expression(rh).expand(options), o, mod);
  }

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex equation::eval() const {
    if (flags & status_flags::evaluated) return *this;

    if (check_modulus(mod)) {
      MSG_INFO(2, "Applying modulus to equation " << (const equation&)this->hold() << endline);
      return dynallocate<equation>(apply_modulus(lh, mod), apply_modulus(rh, mod), o, mod).setflag(status_flags::evaluated);
    }
    return this->hold();
  }
#else
  ex equation::eval(int level) const {
    if (flags & status_flags::evaluated) return *this;

    if (check_modulus(mod)) {
      MSG_INFO(2, "Applying modulus to equation " << (const equation&)this->hold() << endline);
      return new equation(apply_modulus(lh, mod), apply_modulus(rh, mod), o, mod).setflag(status_flags::dynallocated|status_flags::evaluated);
    }
    return this->hold();
  }
#endif

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex equation::evalf() const {
    MSG_INFO(3, "Evaluating " << *this << endline);
    return dynallocate<equation>(expression(lh).evalf(), expression(rh).evalf(), o, mod);
  }
#else
  ex equation::evalf(int level) const {
    MSG_INFO(3, "Evaluating " << *this << endline);
    return equation(expression(lh).evalf(), expression(rh).evalf(), o, mod);
  }
#endif

  ex equation::evalm() const {
    MSG_INFO(3, "Evaluating matrices in " << *this << endline);
    equation result = *this;
    result.lh = expression(lh).evalm();
    result.rh = expression(rh).evalm();
    return result;
  }

  equation &equation::eqadd(const expression &add) {
    if (is_a<equation>(add)) {
      lh += ex_to<equation>(add).lh;
      rh += ex_to<equation>(add).rh;
      if (o != ex_to<equation>(add).o)
        throw std::invalid_argument("The two equations must have the same operator sign");
    } else {
      lh += add;
      rh += add;
    }
    return *this;
  }

  equation &equation::eqmul(const expression &mul) {
    if (is_a<equation>(mul)) {
      lh = lh * ex_to<equation>(mul).lh;
      rh = rh * ex_to<equation>(mul).rh;
      if (o != ex_to<equation>(mul).o)
        throw std::invalid_argument("The two equations must have the same operator sign");
    } else {
      if (o != relational::equal)
        MSG_WARN(0,  "Warning: Operator sign might have changed in multiplication" << endline);
      lh *= mul;
      rh *= mul;
    }
    return *this;
  }

  expression equation::apply_func(const expression &e) const {
    MSG_INFO(1, "Applying function " << e << " to " << *this << endline);
    if (!is_a<func>(e))
      throw std::invalid_argument("Argument must be a function name");
    return dynallocate<equation>(func(ex_to<func>(e).get_name(), lh),
                    func(ex_to<func>(e).get_name(), rh), o, mod);
  }

  expression equation::apply_func(const std::string &fname) const {
    MSG_INFO(1, "Applying function " << fname << " to " << *this << endline);
    if (!func::is_a_func(fname))
      throw std::invalid_argument("Argument must be a function name");
    return dynallocate<equation>(func(fname, lh), func(fname, rh), o, mod);
  }

  expression equation::apply_power(const expression &e) const {
    MSG_INFO(1, "Raising " << *this << " to the power of " << e << endline);
    return dynallocate<equation>(power(lh, e), power(rh, e), o, mod);
  }

  expression equation::diff(const ex &var, const expression& nth, const bool toplevel) const {
    MSG_INFO(1, "Calculating " << nth << "-th derivative of " << *this << " to " << var << endline);
    return dynallocate<equation>(expression(lh).diff(var, nth, toplevel), expression(rh).diff(var, nth, toplevel), o, mod);
  }

  ex equation::derivative(const symbol& s) const {
    return diff(s, _expr1);
  }

  expression equation::pdiff(const ex &var, const expression& nth, const bool toplevel) const {
    return dynallocate<equation>(expression(lh).pdiff(var, nth, toplevel), expression(rh).pdiff(var, nth, toplevel), o, mod);
  }

  expression equation::integrate(const ex &var, const symbol& integration_constant) const {
    return dynallocate<equation>(expression(lh).integrate(var, integration_constant), expression(rh).integrate(var, integration_constant), o, mod);
  }

  expression equation::integrate(const ex &var, const ex& lowerbound, const ex& upperbound) const {
    return dynallocate<equation>(expression(lh).integrate(var, lowerbound, upperbound), expression(rh).integrate(var, lowerbound, upperbound), o, mod);
  }

  expression equation::integrate(const ex &lvar, const symbol& l_integration_constant, const ex &rvar, const symbol& r_integration_constant) const {
    return dynallocate<equation>(expression(lh).integrate(lvar, l_integration_constant), expression(rh).integrate(rvar, r_integration_constant), o, mod);
  }

  expression equation::integrate(const ex &lvar, const ex& l_lowerbound, const ex& l_upperbound, const ex &rvar, const ex& r_lowerbound, const ex& r_upperbound) const {
    return dynallocate<equation>(expression(lh).integrate(lvar, l_lowerbound, l_upperbound), expression(rh).integrate(rvar, r_lowerbound, r_upperbound), o, mod);
  }

  expression equation::reverse() const {
    MSG_INFO(1, "Reversed equation " << *this << endline);
    operators newo = o;

    if (o == relational::greater)
      newo = relational::less;
    else if (o == relational::greater_or_equal)
      newo = relational::less_or_equal;
    else if (o == relational::less)
      newo = relational::greater;
    else if (o == relational::less_or_equal)
      newo = relational::greater_or_equal;

    return dynallocate<equation>(rh, lh, newo, mod);
  } // equation::reverse()

  expression equation::simplify(const std::vector<std::string> &s) const {
    return dynallocate<equation>(expression(lh).simplify(s), expression(rh).simplify(s), o, mod);
  }

  expression equation::collect(const expression& e) const {
    MSG_INFO(1, "Collecting equation " << *this << " to symbol " << e << endline);
    return dynallocate<equation>(expression(lh).collect(e), expression(rh).collect(e), o, mod);
  }

  expression qsolve(const exprvector &coeff, const int num) {
    // Helper function for equation::solve()
    expression help = coeff[1]/(_expr2 * coeff[2]);
    expression det = pow(help, _expr2) - coeff[0]/coeff[2];
    if (det.is_zero()) {
      if (num == 1) MSG_WARN(1, "Warning: This equation has only one solution" << endline);
      return (-help);
    } else {
      if (num > 2) MSG_WARN(1, "Warning: This equation has only two solutions" << endline);
      return ((num == 1) ? -help + pow(det, _expr1_2)
                         : -help - pow(det, _expr1_2));
    }
  }

  expression csolve(const exprvector &coeff, const int num) {
    // Handle degenerated cases
    if (coeff[0].is_zero()) {
      // One solution is zero
      switch (num) {
        case 0:
        case 1: return _expr0;
        case 2:
        case 3: {
          exprvector newcoeff;
          newcoeff.emplace_back(coeff[1]);
          newcoeff.emplace_back(coeff[2]);
          newcoeff.emplace_back(coeff[3]);
          return qsolve(newcoeff, num-1);
        }
      }
    }
    if (coeff[1].is_zero() && coeff[2].is_zero()) {
      // Extract third roots
      expression realsolution = pow(_expr_1 * (coeff[0]/coeff[3]), _expr1_3);
      switch (num) {
        case 0:
        case 1: return realsolution;
        case 2: return _expr1_2 * (_expr_1 + numeric(I) * pow(_expr3, _expr1_2)) * realsolution;
        case 3: return _expr1_2 * (_expr_1 - numeric(I) * pow(_expr3, _expr1_2)) * realsolution;
      }
    }

    // Use the formulae of Cardano to solve the cubic problem. The solution is done for the depressed case
    // y^3 + 3 p y + 2 q = 0, where y = x + b/(3a)
    // Note that sqrt(3) can not be used because it evaluates to a real number immediately
    expression q = _expr1_2 * (_expr2 * pow(coeff[2], _expr3) / (numeric(27) * pow(coeff[3], _expr3)) -
                                   coeff[2] * coeff[1] / (_expr3 * pow(coeff[3], _expr2)) +
                                   coeff[0]/coeff[3]);
    expression p = _expr1_3 * ((_expr3 * coeff[3] * coeff[1] - pow(coeff[2], _expr2)) /
                                   (_expr3 * pow(coeff[3], _expr2)));
    MSG_INFO(2, "Coefficient p : " << p << ", q: " << q << endline);
    expression u = pow(-q + pow(pow(q, _expr2) + pow(p, _expr3), _expr1_2), _expr1_3);
    expression v = pow(-q - pow(pow(q, _expr2) + pow(p, _expr3), _expr1_2), _expr1_3);
    expression eps1 = _expr_1_2 + numeric(I) * _expr1_2 * pow(_expr3, _expr1_2);
    expression eps2 = _expr_1_2 - numeric(I) * _expr1_2 * pow(_expr3, _expr1_2);
    MSG_INFO(2, "eps1 = " << eps1 << "; eps2 = " << eps2 << endline);
    expression y1 = u + v;
    expression y2 = eps1 * u + eps2 * v;
    expression y3 = eps2 * u + eps1 * v;
    switch (num) {
      case 0: // This returns the first solution, also
      case 1: return (y1 - coeff[2] / (_expr3 * coeff[3]));
      case 2: return (y2 - coeff[2] / (_expr3 * coeff[3]));
      case 3: return (y3 - coeff[2] / (_expr3 * coeff[3]));
      default:
        throw std::invalid_argument("Not more than three solutions exist");
    }
  }

  expression quarticsolve(const exprvector& coeff, const int num) {
    // The problem is in the form a x^4 + b x^3 + c x^2 + d x + e
    if (coeff[0].is_zero()) {
      // One root is zero
      switch (num) {
        case 0:
        case 1: return _expr0;
        case 2:
        case 3:
        case 4: {
          exprvector newcoeff;
          newcoeff.emplace_back(coeff[1]);
          newcoeff.emplace_back(coeff[2]);
          newcoeff.emplace_back(coeff[3]);
          newcoeff.emplace_back(coeff[4]);
          return csolve(newcoeff, num-1);
        }
      }
    }

    if (coeff[1].is_zero() && coeff[3].is_zero()) {
      if (coeff[2].is_zero()) {
        // Extract fourth roots
        expression realsolution = pow(_expr_1 * (coeff[0]/coeff[4]), _expr1_4);
        switch (num) {
          case 0:
          case 1: return realsolution;
          case 2: return -realsolution;
          case 3: return numeric(I) * realsolution;
          case 4: return numeric(-I) * realsolution;
        }
      } else {
        // The problem has the form coeff[0] + coeff[2] * e^2 + coeff[4] * e^4
        exprvector newcoeff;
        newcoeff.emplace_back(coeff[0]);
        newcoeff.emplace_back(coeff[2]);
        newcoeff.emplace_back(coeff[4]);
        switch (num) {
          case 0:
          case 1: return sqrt(qsolve(newcoeff, 1));
          case 2: return -sqrt(qsolve(newcoeff, 1));
          case 3: return sqrt(qsolve(newcoeff, 2));
          case 4: return -sqrt(qsolve(newcoeff, 2));
        }
      }
    }

    // Transform the equation into its depressed form y^4 + p y^2 + q y + r by substituting x = y - b/4
    expression x(dynallocate<symbol>("x"));
    expression y(dynallocate<symbol>("y"));
    expression problem = pow(x, _expr4) + coeff[3]/coeff[4] * pow(x, _expr3) + coeff[2]/coeff[4] * pow(x, _expr2) + coeff[1]/coeff[4] * x + coeff[0]/coeff[4];
    expression _problem = problem.subs(x == y - coeff[3] / (coeff[4] * _expr4)).expand();
    exprvector dcoeff;
    for (unsigned i = 0; i <= 4; i++) {
      dcoeff.emplace_back(_problem.coeff(y, i));
      MSG_INFO(2, "Depressed coefficient " << i << ": " << dcoeff.back() << endline);
    }

    // Get the coefficients of the depressed equation
    expression p(dcoeff[2]);
    expression q(dcoeff[1]);
    expression r(dcoeff[0]);

    // Check whether the depressed equation is biquadratic (this would lead to a division by zero in the general quartic algorithm)
    if (q.is_zero()) {
      exprvector bcoeff;
      bcoeff.emplace_back(r);
      bcoeff.emplace_back(p);
      bcoeff.emplace_back(_expr1);
      switch (num) {
        case 0:
        case 1: return pow(qsolve(bcoeff, 1), _expr1_2) - coeff[3] / (coeff[4] * _expr4);
        case 2: return pow(qsolve(bcoeff, 1), _expr1_2) * _expr_1 - coeff[3] / (coeff[4] * _expr4);
        case 3: return pow(qsolve(bcoeff, 2), _expr1_2) - coeff[3] / (coeff[4] * _expr4);
        case 4: return pow(qsolve(bcoeff, 2), _expr1_2) * _expr_1 - coeff[3] / (coeff[4] * _expr4);
        default:
          throw std::invalid_argument("Not more than four solutions exist");
      }
    }

    // Solve the resolvent cubic equation z^3 + 2p z^2 + (p^2 - 4r) z - q^2
    exprvector rcoeff;
    rcoeff.emplace_back(-pow(q, _expr2));
    rcoeff.emplace_back(pow(p, _expr2) - _expr4 * r);
    rcoeff.emplace_back(_expr2 * p);
    rcoeff.emplace_back(_expr1);
    expression s2 = csolve(rcoeff,1); // Any non-zero root will do
    expression s = pow(s2, _expr1_2);

    // Set the two quadratic factors to zero (see https://en.wikipedia.org/w/index.php?title=Quartic_function&section=16)
    exprvector f1coeff, f2coeff;
    f1coeff.emplace_back(_expr1_2 * (p + s2 - q / s));
    f1coeff.emplace_back(s);
    f1coeff.emplace_back(_expr1);
    f2coeff.emplace_back(_expr1_2 * (p + s2 + q / s));
    f2coeff.emplace_back(-s);
    f2coeff.emplace_back(_expr1);

    switch (num) {
      case 0: // This returns the first solution, also
      case 1: return qsolve(f1coeff, 1) - coeff[3] / (coeff[4] * _expr4);
      case 2: return qsolve(f1coeff, 2) - coeff[3] / (coeff[4] * _expr4);
      case 3: return qsolve(f2coeff, 1) - coeff[3] / (coeff[4] * _expr4);
      case 4: return qsolve(f2coeff, 2) - coeff[3] / (coeff[4] * _expr4);
      default:
        throw std::invalid_argument("Not more than four solutions exist");
    }
  }

  expression equation::solve(const expression &e, const expression &n) const {
    if (!is_a<numeric>(n)) throw std::invalid_argument("The solution number must be numerical!");
    if (!ex_to<numeric>(n).is_pos_integer())
      throw std::invalid_argument("The solution number must be a positive integer!");
    unsigned num = ex_to<numeric>(n).to_int();

    // Enable solving for general expressions by replacing them with temporary symbols
    exmap e_subs;
    exmap e_subs_rev;
    unsigned tempnum = 0;
    for (const auto& s : (is_a<matrix>(e) ? e : lst{e})) {
      if (is_a<symbol>(s)) continue; // Avoid unnecessary substitutions
      symbol s_sym("__temp" + std::to_string(tempnum++) + "__");
      e_subs.emplace(s, s_sym);
      e_subs_rev.emplace(s_sym, s);
    }

    if (is_a<matrix>(e)) {
      MSG_INFO(1, "Solving linear equation system " << *this << " for " << e << ", solution #" << n << " requested " << endline);
      if (!is_a<matrix>(lh)) throw std::invalid_argument("Left-hand side must be a matrix!");
      if (!is_a<matrix>(rh)) throw std::invalid_argument("Right-hand side must be a matrix!");

      matrix l = ex_to<matrix>(lh.subs(e_subs));
      matrix r = ex_to<matrix>(rh.subs(e_subs));
      matrix v = ex_to<matrix>(e.subs(e_subs));

      if ((l.cols() == 1) && (r.cols() == 1) && (v.cols() == 1)) {
        if ((l.rows() != r.rows()) || (l.rows() != v.rows())) throw std::invalid_argument("Number of rows must be equal in linear equation system");

        unsigned rows = l.rows();
        lst eqns;
        lst syms;

        for (unsigned row = 0; row < rows; ++row) {
          eqns.append(l[row] == r[row]);
          syms.append(v[row]);
        }

        MSG_INFO(2, "Solving " << eqns << " for " << syms << endline);
        ex result = lsolve(eqns, syms);
        MSG_INFO(2, "Result: " << result << endline);
        matrix result_rhs(rows, 1);

        for (unsigned row = 0; row < rows; ++row) {
          ex eq = result[row];
          MSG_INFO(2, "Sorting " << eq << endline);
          for (unsigned vrow = 0; vrow < rows; ++vrow) {
            MSG_INFO(2, "Looking at variable " << v[vrow] << endline);
            if (v[vrow].is_equal(eq.lhs()))
               result_rhs[vrow] = eq.rhs();
          }
        }

        return equation(e, result_rhs, o, mod).subs(e_subs_rev);
      } else {
        matrix s = l.solve(r, v);
        return equation(e, s, o, mod).subs(e_subs_rev);
      }
    }

    MSG_INFO(1, "Solving " << *this << " for " << e << ", solution #" << n << " requested " << endline);
    expression problem = (lh - rh).subs(e_subs).expand().normal();
    ex e_sym = e.subs(e_subs);
    MSG_INFO(2, "The problem is " << problem << endline);
    if (!denom(problem).is_equal(1)) {
      MSG_WARN(2,  "Warning: The solution may not be defined for some values because the denominator is not 1" << endline);
      problem = numer(problem);
    }

    unsigned degree;
    try {
      degree = problem.degree(e_sym);
    } catch (std::exception &) {
      throw std::invalid_argument("The equation is not a polynomial in the variable");
    }

    if (num > degree) {
      MSG_WARN(0,  "Warning: This equation can only have " << degree
                       << " solutions. Returning the last one." << endline);
      num = degree;
    }

    exprvector coeff;
    for (unsigned i = 0; i <= degree; i++) coeff.emplace_back(problem.coeff(e_sym, i));
    // The problem now has the form coeff[0] + coeff[1] * e + ... + coeff[degree] * e^degree
    // TODO: What about negative degrees?
    if (msg::info().checkprio(2)) {
      msg::info() << "Degree of the problem is " << degree << endline << "Coefficients: ";
      for (const auto& i : coeff)
        msg::info() << i << " ";
      msg::info() << endline;
    }

    switch (degree) {
      case 0: return equation (*this);
      case 1: return equation (e_sym, _expr_1 * coeff[0]/coeff[1], o, mod).subs(e_subs_rev);
      case 2: return equation (e_sym, qsolve(coeff, num), o, mod).subs(e_subs_rev);
      case 3: return equation (e_sym, csolve(coeff, num), o, mod).subs(e_subs_rev);
      case 4: return equation (e_sym, quarticsolve(coeff, num), o, mod).subs(e_subs_rev);
      default: {
        throw std::invalid_argument("Solving for higher degrees than 4 is not generally possible");
        // TODO: Check whether degree() - ldegree() > 1 instead
      }
    }
  }


const expression operator+(const expression &e, const expression &add) {
  MSG_INFO(3, "Adding " << add << " to " << e << endline);
  if (is_a<equation>(e)) {
    equation result = ex_to<equation>(e);
    return result.eqadd(add);
  } else if (is_a<equation>(add)) {
    equation result = ex_to<equation>(add);
    return result.eqadd(e);
  } else
    return operator+((ex)e, (ex)add);
}
const expression operator+(const equation &e, const equation &add) {
  return operator+(expression(e), expression(add));
}
const expression operator+(const expression &e, const equation &add) {
  return operator+(e, expression(add));
}
const expression operator+(const equation &e, const expression &add) {
  return operator+(expression(e), add);
}

const expression operator-(const expression &e, const expression &sub) {
  MSG_INFO(3, "Subtracting " << sub << " from " << e << endline);
  if (is_a<equation>(e)) {
    equation result = ex_to<equation>(e);
    return result.eqadd(sub * _expr_1);
  } else if (is_a<equation>(sub)) {
    equation result = ex_to<equation>(sub * _expr_1);
    return result.eqadd(e);
  } else
    return operator-((ex)e, (ex)sub);
}
const expression operator-(const equation &e, const equation &sub) {
  return operator-(expression(e), expression(sub));
}
const expression operator-(const expression &e, const equation &sub) {
  return operator-(e, expression(sub));
}
const expression operator-(const equation &e, const expression &sub) {
  return operator-(expression(e), sub);
}

const expression operator*(const expression &e, const expression &mul) {
  MSG_INFO(3, "Multiplying " << e << " with " << mul << endline);
  if (is_a<equation>(e)) {
    if (mul == 0) throw std::invalid_argument("Multiplication of equation with zero");
    equation result = ex_to<equation>(e);
    return result.eqmul(mul);
  } else if (is_a<equation>(mul)) {
    if (e == 0) throw std::invalid_argument("Multiplication with zero");
    equation result = ex_to<equation>(mul);
    return result.eqmul(e);
  } else {
    expression result = operator*((ex)e, (ex)mul);
    return result.evalm(); // Take care of vector multiplication that results in a scalar
  }
}
const expression operator*(const equation &e, const equation &mul) {
  return operator*(expression(e), expression(mul));
}
const expression operator*(const expression &e, const equation &mul) {
  return operator*(e, expression(mul));
}
const expression operator*(const equation &e, const expression &mul) {
  return operator*(expression(e), mul);
}

const expression operator/(const expression &e, const expression &divisor) {
  MSG_INFO(3, "Dividing " << e << " by " << divisor << endline);
  if (divisor == 0) throw std::invalid_argument("Division by zero");
  if (is_a<equation>(e) && is_a<equation>(divisor)) { // Todo: We should check that both equations are equalities a == b
    const equation& eqdividend = ex_to<equation>(e);
    const equation& eqdivisor = ex_to<equation>(divisor);
    if ((eqdividend.getop() == relational::equal) && (eqdivisor.getop() == relational::equal))
      return dynallocate<equation>(eqdividend.lhs() / eqdivisor.lhs(), eqdividend.rhs() / eqdivisor.rhs(), eqdividend.getop(), eqdividend.getmod());
    else
      throw std::invalid_argument("Only equalities can be divided by an equality");
  } else if (is_a<equation>(e)) {
    equation result = ex_to<equation>(e);
    return result.eqmul(pow(divisor, _expr_1));
  } else if (is_a<equation>(divisor)) {
    equation result = ex_to<equation>(pow(divisor, _expr_1));
    return result.eqmul(e);
  } else
    return operator/((ex)e, (ex)divisor);
}
const expression operator/(const equation &e, const equation &divisor) {
  return operator/(expression(e), expression(divisor));
}
const expression operator/(const expression &e, const equation &divisor) {
  return operator/(e, expression(divisor));
}
const expression operator/(const equation &e, const expression &divisor) {
  return operator/(expression(e), divisor);
}

expression pow(const expression &e, const expression &exponent) {
  MSG_INFO(3, "Calculating power of " << e << " to " << exponent << endline);
  if (is_a<equation>(exponent))
    throw std::invalid_argument("Cannot calculate power to exponent that is an equation");
  if (is_a<equation>(e)) {
    const equation& eq = ex_to<equation>(e);
    if (eq.getop() != relational::equal)
      MSG_WARN(0,  "Warning: Operator sign might have changed in exponentiation" << endline);
    return dynallocate<equation>(dynallocate<power>(eq.lhs(), exponent), dynallocate<power>(eq.rhs(), exponent), eq.getop(), eq.getmod());
  } else
    return power(e, exponent); // dynallocate here brings a crash on 10^{-3}
}

const expression operator-(const expression &e) {
  return e * _expr_1;
}

}
