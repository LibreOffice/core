/***************************************************************************
    printing.cpp  -  Functions for pretty-printing expressions in iMath format
                             -------------------
    begin                : Sat Mar 2 2002
    copyright            : (C) 2002 by Jan Rheinlaender
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

#include <iomanip>
#include <sstream>
#include <cmath>
#include <cfloat>
#include <cln/cln.h>
#include "printing.hxx"
#include "operands.hxx"
#include "msgdriver.hxx"
#include "equation.hxx"
#include "stringex.hxx"
#include "extintegral.hxx"
#include "differential.hxx"
#include "exderivative.hxx"
#include "func.hxx"
#include "unit.hxx"
#include "eqc.hxx"

namespace GiNaC {

GINAC_IMPLEMENT_PRINT_CONTEXT(imathprint, print_dflt)

imathprint::imathprint() : print_dflt(std::cout), add_turn_around(false) {};

imathprint::imathprint(std::ostream & os, const imathprint& c) :
  print_dflt(os, c.options), poptions(c.poptions), add_turn_around(c.add_turn_around)
{
}

void imathprint::enter_fraction() const {
  unsigned fractionlevel = (*poptions)[o_fractionlevel].value.uinteger;

  if ((*poptions)[o_autofraction].value.boolean && (fractionlevel > 0)) {
    unsigned basefontheight = (*poptions)[o_basefontheight].value.uinteger;

    if ((basefontheight >= (*poptions)[o_minimumtextsize].value.uinteger + fractionlevel))
      s << "size-1";
  }

  (*poptions)[o_fractionlevel] = fractionlevel + 1;
}

void imathprint::exit_fraction() const {
  (*poptions)[o_fractionlevel] = (*poptions)[o_fractionlevel].value.uinteger - 1;
}

std::string imathprint::decimalpoint = ".";

void imathprint::init() {
    // Initialize print functions
    set_print_func<add, imathprint>(&imathprint_add);
    set_print_func<constant, imathprint>(&imathprint_constant);
    set_print_func<exprseq, imathprint>(&imathprint_exprseq);
    set_print_func<function, imathprint>(&imathprint_function);
    set_print_func<matrix, imathprint>(&imathprint_matrix);
    set_print_func<mul, imathprint>(&imathprint_mul);
    set_print_func<ncmul, imathprint>(&imathprint_ncmul);
    set_print_func<numeric, imathprint>(&imathprint_numeric);
    set_print_func<power, imathprint>(&imathprint_power);
    set_print_func<relational, imathprint>(&imathprint_relational);
    set_print_func<symbol, imathprint>(&imathprint_symbol);
    set_print_func<wildcard, imathprint>(&imathprint_wildcard);
}

/***************************************************************************
    Functions for pretty-printing expressions in starmath
 ***************************************************************************/

// Adapted from libeqc/operands.cpp
/*
void imathchecksplit(const bool toplevel, const int opnum, std::ostream &os) {
  if (toplevel) {
    if (opnum == c.poptions->get(o_eqsplit).integer) {
      MSG_INFO(1, "Splitting equation at operand " << opnum << endline);
      MSG_INFO(2, "Interjected text: " << *c.poptions->get(o_eqsplittext).str << endline);
      if (c.poptions->get(o_eqalign).align == onlyleft) {
        os << "\\\\" << endline);
        if (*c.poptions->get(o_eqsplittext).str == "") {
          os << "&\\quad\\,\\,";
        } else {
          os << *c.poptions->get(o_eqsplittext).str;
        }
      } else if (c.poptions->get(o_eqalign).align == both) {
        os << "\\nonumber\\\\" << endline);
        if (*c.poptions->get(o_eqsplittext).str == "") {
          os << "&&";
        } else {
          os << *c.poptions->get(o_eqsplittext).str;
        }
      } else {
        os << "\\\\" << std::endl << *c.poptions->get(o_eqsplittext).str;
      }
    }
  }
} // operands::()
*/

std::string ex_get_name(const ex& e);
std::string differential_get_name(const differential& d);

std::string power_get_name(const power& p) {
  ex b = get_basis(p);
  if (is_a<symbol>(b)) {
    return std::string("012") + ex_to<symbol>(b).get_name();
  } else if (is_a<constant>(b)) {
    std::ostringstream os;
    os << "012" << ex_to<constant>(b);
    return os.str();
  } else if (is_a<exderivative>(b)) {
    return std::string("055") + differential_get_name(ex_to<exderivative>(b).get_numer());
  } else if (is_a<differential>(b)) {
    return std::string("060") + differential_get_name(ex_to<differential>(b));
  } else {
    return std::string("030") + ex_get_name(b);
  }
} // power_get_name()

std::string differential_get_name(const differential& d) {
  std::string result;

  // Print smaller grades first
  if (d.get_ngrade() >= 0) {
    // Numeric grade (assumed to be less than 999)
    std::ostringstream grade;
    grade << std::setfill('0') << std::setw(3) << d.get_ngrade();
    result = grade.str();
  } else {
    result = "999";
  }

  // Print partials second
  if (d.is_partial())
    result += "p";
  else
    result += "d";

  // Order alphabetically by argument, e.g. dx befor dy
  return result + ex_get_name(d.argument());
}

// Return a code for the expression type and a symbol name, if possible
std::string ex_get_name(const ex& e) {
  MSG_INFO(3, "Finding sort name for " << e << endline);
  if (is_a<numeric>(e))
    return std::string("005");
  else if (is_a<symbol>(e)) {
    std::string name = ex_to<symbol>(e).get_name();
    if (name[0] == '%')
      return std::string("011") + name; // greek letters etc. // TODO: Sort by greek alphabet...
    else
      return std::string("010") + name;
  } else if (is_a<constant>(e)) {
    std::ostringstream name;
    name << ex_to<constant>(e);
    if (name.str()[0] == '%')
      return std::string("011") + name.str(); // greek letters etc.
    else
      return std::string("010") + name.str();
  } else if (is_a<function>(e))
    return std::string("020") + ex_to<function>(e).get_name();
  else if (is_a<func>(e))
    return std::string("020") + ex_to<func>(e).get_name();
  else if (is_a<power>(e))
    return power_get_name(ex_to<power>(e));
  else if (is_a<mul>(e)) {
    if ((e.nops() == 2) && is_a<numeric>(e.op(1)) && (is_a<symbol>(e.op(0)) || is_a<constant>(e.op(0)))) {
      // treat this like a symbol
      return ex_get_name(e.op(0));
    } else {
      std::string largest("");
      for (const auto& i : e) {
        std::string current = ex_get_name(i);
        if (current > largest) largest = current;
      }
      return std::string("040") + largest;
    }
  } else if (is_a<extintegral>(e))
    return std::string("050");
  else if (is_a<exderivative>(e))
    return std::string("055") + differential_get_name(ex_to<exderivative>(e).get_numer());
  else if (is_a<differential>(e))
    return std::string("060") + differential_get_name(ex_to<differential>(e));

  return std::string("999");
} // ex_get_name()

bool ex_compare(const ex& l, const ex& r) {
  std::string lname = ex_get_name(l);
  std::string rname = ex_get_name(r);
  MSG_INFO(2, "Comparing " << lname << " with " << rname << endline);

  if ((lname == "") || (rname == ""))
    return (l < r);
  else
    return (lname < rname);
} // ex_is_less();

std::vector<ex> order_ex(const ex& e) {
  std::vector<ex> result(e.begin(), e.end());
  std::sort(result.begin(), result.end(), ex_compare);

  return result;
} // order_ex()

// Check whether at least one add is of the form a-b so that it can be "turned around" to b-a to get rid of the negative coefficient
// Two cases:
// a) There are several adds in the ex, then we need to do the "j" iteration
// b) There is only one add, then the "i" iteration is enough
bool check_has_negop(const ex& adds) {
  if (adds.is_equal(_ex1)) return false;

  bool found = false;
  size_t negops = 0;

  if (is_a<add>(adds)) { // There is only one add
    if (adds.nops() != 2) return false; // We don't touch things like a - b + c because we want alphabetical ordering

    for (const auto& i : adds)
      if (is_negex(i)) negops++;

    found = ((negops > 0) && (negops != adds.nops()));
  } else {
    for (const auto& i : adds) { // We have several adds in 'adds'
      negops = 0;
      if (i.nops() != 2) continue;

      for (const auto& j : i)
        if (is_negex(j)) negops++;

      found = ((negops > 0) && (negops != i.nops()));
      if (found) break;
    }
  }

  return found;
} // check_has_negop()

void print_smath_ops(const ex &e, const std::string &sym, const imathprint& c,
                     const bool toplevel, size_t &opnum, const bool add_turn_around = false) {
  (void)toplevel;
  bool turn_around = add_turn_around;

  if (opnum > 0) {
    c.s << sym;
//    checksplit(toplevel, opnum, c);
  }

  if (is_a<add>(e)) {
    std::vector<ex> sorted = order_ex(e);
    for (std::vector<ex>::const_iterator i = sorted.begin(); i != sorted.end(); i++) {
      //if (i != e.begin()) checksplit(toplevel, opnum, c, 1);
      i->print(c, 1);
      std::vector<ex>::const_iterator j = i;
      j++;
      if ((j != sorted.end()) && !is_negex(*j)) c.s << sym;
      opnum++;
    }
    return; // Don't increment opnum again!
  } else if (is_a<mul>(e)) {
    std::vector<ex> sorted = order_ex(e);

    for (std::vector<ex>::const_iterator i = sorted.begin(); i != sorted.end(); i++) {
      //if (i != e.end()) checksplit(toplevel, opnum, c, 1);
      if (is_a<add>(*i)) {
        c.s << "(";
        if (turn_around && check_has_negop(*i)) {
          // This is a hassle, but there is no other way to turn around an add
          // Reason: -a * (r^4 - R^2) sometimes, but not always gives a * (R^4 - r^4)
          std::ostringstream os;
          imathprint cc(os, c);
          cc.add_turn_around = turn_around;
          imathprint_add(ex_to<add>(*i), cc, 1);
          turn_around = false;
          c.s << os.str();
        } else {
          imathprint_add(ex_to<add>(*i), c, 1);
        }
        c.s << " )";
      } else {
        i->print(c, 1);
      }
      std::vector<ex>::const_iterator j = i;
      j++;
      if (j != sorted.end()) c.s << sym;
      opnum++;
    }
    return;
  } else {
    e.print(c, 1);
  }

  opnum++;
} // print_smath_ops()

void print_smath_mul(const operands &m, const imathprint& c, const size_t ops, const bool add_turn_around = false) {
  MSG_INFO(5, "print_smath_mul for " << m << endline);
  bool turn_around = add_turn_around;

  // Print the coefficient
  size_t opnum = ops;
  if (!is_equal_int(ex_to<numeric>(m.get_coefficient()), 1, (*c.poptions)[o_precision].value.uinteger) || m.is_trivial()) {
    if (m.get_coefficient().info(info_flags::real)) {
      imathprint_numeric(ex_to<numeric>(m.get_coefficient()), c, (unsigned)opnum);
      c.s << " ";
    } else {
      c.s << "(";
      imathprint_numeric(ex_to<numeric>(m.get_coefficient()), c, (unsigned)opnum);
      c.s << ")";
    }
    opnum++;
  }

  const std::string separator = " "; // The multiplication "symbol"

  // Print powers of numerics
  ex restpowers(_ex1);
  if (!m.get_powers().is_equal(_ex1)) {
    for (const auto& i : (is_a<mul>(m.get_powers()) ? m.get_powers() : lst{m.get_powers()})) {
      if (is_a<numeric>(get_basis(ex_to<power>(i))) && is_a<numeric>(get_exp(ex_to<power>(i)))) {
        print_smath_ops(i, separator, c, false, opnum);
      } else {
        restpowers = restpowers * i;
      }
    }
  }

  // Functions must be printed last to avoid ambiguities like tan xa -> tan (x*a) or tan (x) * a ?
  if (!m.get_units().is_equal(_ex1)) {
    if (opnum == 0) c.s << "1"; // Avoid "blank" unit
    print_smath_ops(m.get_units(), separator, c, false, opnum);
  }
  if (!m.get_constants().is_equal(_ex1))
    print_smath_ops(m.get_constants(), separator, c, false, opnum);
  if (!m.check_symbols(_ex1))
    print_smath_ops(m.get_symbols(), separator, c, false, opnum);
  if (!restpowers.is_equal(_ex1))
    print_smath_ops(restpowers, separator, c, false, opnum);
  if (!m.get_adds().is_equal(_ex1)) {
    if (opnum > 0) c.s << separator;

    if (is_a<mul>(m.get_adds())) { // There are several adds in this mul
      opnum++; // suppress op_symbol here!
      print_smath_ops(m.get_adds(), separator, c, false, opnum, turn_around);
      opnum--; // correct operator count
    } else {
      if ((opnum == ops) && m.get_others().is_equal(_ex1) && m.check_functions(_ex1) &&
          m.get_muls().is_equal(_ex1) && m.check_matrices(_ex1) && m.get_integrals().is_equal(_ex1) &&
          m.get_differentials().is_equal(_ex1) && m.get_derivatives().is_equal(_ex1)) {
        // In other words, if there is only this one add in the mul, and nothing else
        std::ostringstream os;
        imathprint cc(os, c);
        cc.add_turn_around = turn_around;
        imathprint_add(ex_to<add>(m.get_adds()), cc, (unsigned)opnum);
        c.s << os.str();
      } else {
        c.s << "(";
        std::ostringstream os;
        imathprint cc(os, c);
        cc.add_turn_around = turn_around;
        imathprint_add(ex_to<add>(m.get_adds()), cc, (unsigned)opnum);
        c.s << os.str();
        c.s << ") ";
      }
      //checksplit(toplevel, opnum, c);
    }
  }
  // Warning: The preceding code depends on these statements following AFTER it!
  if (!m.get_muls().is_equal(_ex1)) {
    imathprint_mul(ex_to<mul>(m.get_muls()), c, (unsigned)opnum);
    opnum += m.get_muls().nops();
  }
  if (!m.check_matrices(_ex1))
    print_smath_ops(m.get_matrices(), separator, c, false, opnum);
  if (!m.get_others().is_equal(_ex1))
    print_smath_ops(m.get_others(), separator, c, false, opnum);
  if (!m.check_functions(_ex1))
    print_smath_ops(m.get_functions(), separator, c, false, opnum);
  if (!m.get_integrals().is_equal(_ex1))
    print_smath_ops(m.get_integrals(), separator, c, false, opnum);
  if (!m.get_derivatives().is_equal(_ex1))
    print_smath_ops(m.get_derivatives(), separator, c, false, opnum);
  if (!m.get_differentials().is_equal(_ex1))
    print_smath_ops(m.get_differentials(), separator, c, false, opnum);
  c.s << ' ';
} // print_smath_mul()

void imathprint_add(const add& a, const imathprint& c, unsigned level) {
  MSG_INFO(3, "imathprint_add " << a << endline);
  ex sym;
  bool turn_around = c.add_turn_around;
  std::ostringstream os;
  imathprint cc(os, c); // prevent propagation to other print functions
  cc.add_turn_around = false;

  // Handle special case of e.g. a - x^2, this should not print as -x^2 + a
  if ((a.nops() == 2) && (is_negex(a.op(0)) != is_negex(a.op(1)))) { // Only two operands, only one is negative
    // We don't really want to re-arrange for more than two operands, though
    std::vector<ex> ops;
    ops.emplace_back(turn_around ? (-1 * a.op(0)) : a.op(0));
    ops.emplace_back(turn_around ? (-1 * a.op(1)) : a.op(1));
    if (is_negex(ops[0])) ops[0].swap(ops[1]);

    for (std::vector<ex>::const_iterator i = ops.begin(); i != ops.end(); i++) {
      if (i != ops.begin()) {
        if (is_negex(*i)) {
          // Try to avoid unnecessary - signs
          std::ostringstream tstream;
          imathprint tprint(tstream, cc);
          tprint.add_turn_around = false;
          i->print(tprint, level+1); // This will try to get rid of negative coefficients
          std::string tstring = tstream.str();
          if (tstring[0] != '-') cc.s << " + "; // It worked!
          cc.s << tstring;
        } else {
          cc.s << " + ";
          i->print(cc, level+1);
        }
      } else {
        i->print(cc, level+1);
      }
    }

    c.s << os.str();
    return;
  }

  // Print polynomials nicely
  if (check_polynomial(a, sym)) {
    MSG_INFO(3, a << " is a polynomial in " << sym << endline);
    bool first_coeff_printed = false;

    for (int i = a.degree(sym); i >= a.ldegree(sym); --i) {
      ex coeff = turn_around ? (-1 * a.coeff(sym,i)) : a.coeff(sym,i);
      MSG_INFO(3, "coeff of degree " << i << ": " << coeff << endline);

      if (!coeff.is_zero()) {
        bool changed_sign = false;
        if (is_negex(coeff)) {
          changed_sign = true;
          cc.s << " -";
          coeff = -coeff;
        } else
          if (first_coeff_printed)
            cc.s << " + ";

        if (i >= 0) {
          if (!coeff.is_equal(_ex1)) {
            if (is_a<add>(coeff) && ((i != 0) || changed_sign)) cc.s << "(";
            coeff.print(cc, level+1);
            if (is_a<add>(coeff) && ((i != 0) || changed_sign)) cc.s << ")";
          } else if (i == 0) {
            coeff.print(cc, level+1);
          }

          if (i > 0) {
            cc.s << " ";
            imathprint_power(power(sym, i), cc, level+1);
          }
        } else {
          (coeff * power(sym,i)).print(cc, level+1);
        }

        first_coeff_printed = true;
      }
    }
  } else { // Everything else is sorted alphabetically if possible
    MSG_INFO(4, "Ordered add" << endline);
    std::vector<ex> sorted = order_ex(a);
    // TODO: Move operands around to avoid a leading - sign? Or pull out the - sign if the add is part of a mul?

    for (std::vector<ex>::const_iterator i = sorted.begin(); i != sorted.end(); i++) {
      MSG_INFO(4, "Printing add component: " << *i << endline);
      ex the_ex = turn_around ? (_ex_1 * (*i)) : *i;
      if (is_negex(the_ex) && (i == sorted.begin())) {
        cc.s << "-";
        the_ex = the_ex * _ex_1;
      }
      // Try to avoid unnecessary + signs
      std::ostringstream tstream;
      imathprint tprint(tstream, cc);
      cc.add_turn_around = false;
      the_ex.print(tprint, level);
      std::string tstring = tstream.str();
      if ((tstring[0] != '-') && (i != sorted.begin())) cc.s << " + ";
      if (is_a<Unit>(the_ex)) cc.s << "1 ";
      cc.s << tstring;
    }
  }

  c.s << os.str();
} // imathprint_add()

void imathprint_constant(const constant& cn, const imathprint& c, unsigned level) {
  (void)level;
  std::ostringstream os;

  if (cn == Pi)
    os << "%pi";
  else if (cn == Euler_number)
    os << "func e";
  else
    os << latex << cn;

  c.s << os.str();
}

void imathprint_exprseq(const exprseq& es, const imathprint& c, unsigned level) {
  MSG_INFO(4, "Inside imathprint_exprseq()" << endline);
  for (func::const_iterator i = es.begin(); i != es.end(); i++) {
    i->print(c, level+1);
    if (i != es.end() - 1) c.s << ", ";
  }
}

void imathprint_function(const function& f, const imathprint& c, unsigned level) {
  MSG_INFO(4, "Inside imathprint_function()" << endline); // Important for printing hard-coded functions
  (void)level;
  c.s << f.get_name() << "(GiNaC-Function) ";

    c.s << "({";
    for (func::const_iterator i = f.begin(); i != f.end(); i++) {
      i->print(c, level+1);
      if (i != f.end() - 1) c.s << ", ";
    }
    c.s << "})";
} // imathprint_function()

void imathprint_matrix(const matrix& m, const imathprint& c, unsigned level) {
  (void)level;

  if (m.cols() == 1) { // stack{} is only useable for matrices with one single column
    c.s << "( STACK{";
    for (unsigned r = 0; r < m.rows(); ) {
      m(r, 0).print(c, 1);
      if (++r != m.rows()) c.s << " # ";
    }
  } else {
    c.s << "( MATRIX{";
    for (unsigned r = 0; r < m.rows(); ) {
      for (unsigned col = 0; col < m.cols(); ) {
        m(r, col).print(c, 1);
        if (++col != m.cols()) c.s << " # ";
      }
      if (++r != m.rows()) c.s << " ## ";
    }
  }
  c.s << "})";
}

void imathprint_mul(const mul& m, const imathprint& c, unsigned level) {
  MSG_INFO(4, "imathprint_mul() for " << m << endline);
  operands numer(GINAC_MUL), denom(GINAC_MUL), tempn(GINAC_MUL), tempd(GINAC_MUL), temp(GINAC_MUL);
  operands::split_ex(m, numer, denom);
  size_t opnum = 0; // original was: ops;
//  checksplit(toplevel, opnum, c.s);
  int turn_around = 0;

  if (numer.get_coefficient().info(info_flags::negative)) { // The coefficient is negative
    // GiNaC likes to pull out a minus sign from adds ... so let's see if we can get rid of the minus sign of the coefficient
    if (check_has_negop(numer.get_adds()))
      turn_around = 1;
    else if (check_has_negop(denom.get_adds()))
      turn_around = 2;
    else
      c.s << '-';

    numer.include(_ex_1); // we have dealt with minus sign of the coefficient
  }

  ex n_units = numer.get_units();
  ex d_units = denom.get_units();
  ex coefficient = numer.get_coefficient(); // Note that denom.get_coefficient() is always 1
  ex nsymbols = numer.get_symbols();
  ex nconstants = numer.get_constants();

  // Check for "spurious" complex part of the coefficient (often happens when solving cubic equations)
  if (!ex_to<numeric>(coefficient).info(info_flags::real))
    if (is_equal_int(imag(ex_to<numeric>(coefficient)), 0, Digits))
      coefficient = real(ex_to<numeric>(coefficient));

  // Check if the denominator has one single symbol only
  ex coeff_nd = coefficient.numer_denom();
  bool d_one_symbol = denom.is_symbol() && (coeff_nd.op(1).is_equal(_ex1));
  // Check if the numerator has one single symbol (or constant) only
  bool n_one_symbol = ((is_a<symbol>(nsymbols) || is_a<constant>(nconstants)) &&
                       coeff_nd.op(0).is_equal(_ex1) && numer.get_adds().is_equal(_ex1) &&
                       numer.check_functions(_ex1) && numer.check_matrices(_ex1) && numer.get_muls().is_equal(_ex1) &&
                       numer.get_others().is_equal(_ex1) && numer.get_powers().is_equal(_ex1) && numer.get_units().is_equal(_ex1) &&
                       numer.get_integrals().is_equal(_ex1) && numer.get_differentials().is_equal(_ex1) && numer.get_derivatives().is_equal(_ex1));

  // Print the coefficient with its units first to get nicer output
  if (!n_one_symbol){
    // Because a simple fraction is nicer like this: x/2 than like this; 1/2 x, in my opinion
    bool has_quantity = (!coefficient.is_equal(_ex1) && (!n_units.is_equal(_ex1) || !d_units.is_equal(_ex1)));

    if (!has_quantity && (!coefficient.is_equal(_ex1) && d_one_symbol && numer.check_symbols(_ex1))) { //Prevent things like 2 1/x , make them 2/x
      c.enter_fraction();
      c.s << "{{alignc ";
      (coeff_nd.op(0) * nconstants).print(c, level+1);
      c.s << "} over {alignc ";
      if (!coeff_nd.op(1).is_equal(_ex1)) coeff_nd.op(1).print(c, level+1);
      denom.get_symbols().print(c, level+1);
      c.s << "}}";
      c.exit_fraction();
      d_one_symbol = false; // It has already been printed
      denom.clear_symbols();
      numer.clear_constants();
      opnum++;
    } else {
      // Note that testing for is_real() does not work properly!!!
      if (!imag(ex_to<numeric>(coefficient)).is_zero() && !real(ex_to<numeric>(coefficient)).is_zero()) c.s << "("; // put complex expression into brackets)
      if (!coefficient.is_equal(_ex1)) {
        imathprint_numeric(ex_to<numeric>(coefficient), c, level+1);
        c.s << " "; // if there is no space between coefficient and the following symbol, smath views them as one symbol
        opnum++;
      }
      if (!imag(ex_to<numeric>(coefficient)).is_zero() && !real(ex_to<numeric>(coefficient)).is_zero()) c.s << ")";
    }

    if (!d_units.is_equal(_ex1)) {
      c.enter_fraction();
      c.s << "{{alignc ";
    }

    if (!(n_units.is_equal(_ex1) && d_units.is_equal(_ex1))) {
      operands::split_ex(n_units, tempn, temp); // avoid infinite recursion
      print_smath_mul(tempn, c, opnum);
      if (opnum != 0) opnum++;  // The quantity counts as a single operand
    }

    if (!d_units.is_equal(_ex1)) {
      c.s << "} over {alignc ";
      operands::split_ex(d_units, tempd, temp); // *** avoid infinite recursion
      print_smath_mul(tempd, c, opnum);
      if (opnum != 0) opnum++;  // The quantity counts as a single operand
      c.s << "}}";
      c.exit_fraction();
    }

    numer.exclude(numer.get_coefficient()); // The coefficient and units have already been printed
    denom.exclude(denom.get_coefficient());
    numer.clear_units();
    denom.clear_units();
  } else {
    ex tempcoeff = numer.get_coefficient();
    numer.include(tempcoeff.denom()); // This has the effect of removing the denominator part ...
    denom.include(tempcoeff.denom()); // ... and including it here!
  }

  // If the denominator has only one single symbol, print it first
  // This looks nicer in my opinion: (abc (x - 1)) / g -> (abc)/g (x-1)

  if (d_one_symbol && !nsymbols.is_equal(_ex1)) { // There is only one symbol in the denominator
    //if (opnum != 0) checksplit(toplevel, opnum, c);
    c.enter_fraction();
    c.s << "{{alignc ";
    nsymbols.print(c, level+1);
    c.s << "} over {alignc ";
    denom.get_symbols().print(c, level+1);
    c.s << "}}";
    c.exit_fraction();
    numer.clear_symbols();
    denom.clear_symbols();
    opnum++; // Count all this as one operand
  }

  // A single differential in the numerator should not be printed d²x but dx² (but that is mostly a matter of taste)
  if (denom.get_differentials().is_equal(_ex1) && !numer.get_derivatives().is_equal(_ex1)) {
    if (is_a<differential>(numer.get_differentials())) {
      differential df = ex_to<differential>(numer.get_differentials());
      df.set_numerator(false);
      numer.set_differentials(df);
    } else if (is_a<power>(numer.get_differentials())) {
      const power& p = ex_to<power>(numer.get_differentials());
      differential df = ex_to<differential>(get_basis(p));
      df.set_numerator(false);
      numer.set_differentials(dynallocate<power>(df, get_exp(p)));
    }
  }

  // Print the rest
  if (denom.is_trivial()) {
    if (!numer.is_trivial()) {
      bool is_only_one_add = (numer.check_symbols(_ex1) && numer.get_constants().is_equal(_ex1) && is_a<add>(numer.get_adds()) &&
                              numer.check_functions(_ex1) && numer.check_matrices(_ex1) && numer.get_muls().is_equal(_ex1) &&
                              numer.get_others().is_equal(_ex1) && numer.get_powers().is_equal(_ex1) && numer.get_units().is_equal(_ex1) &&
                              numer.get_integrals().is_equal(_ex1) && numer.get_differentials().is_equal(_ex1) && numer.get_derivatives().is_equal(_ex1));
      if (is_only_one_add) c.s << "(";
      print_smath_mul(numer, c, opnum, turn_around == 1); // checksplit is taken care of here
      if (is_only_one_add) c.s << ")";
    }
  } else  {
    //if (opnum != 0) checksplit(toplevel, opnum, c);

    ex posdiffs = numer.get_differentials();
    ex negdiffs = denom.get_differentials();
    ex posderiv = numer.get_derivatives();
    ex negderiv = denom.get_derivatives();
    bool has_diffs = (!(posdiffs.is_equal(_ex1) && negdiffs.is_equal(_ex1) && posderiv.is_equal(_ex1) && negderiv.is_equal(_ex1)));
    numer.clear_diffs();
    numer.clear_derivatives();
    denom.clear_diffs();
    denom.clear_derivatives();

    // Print everything except the differentials
    if (!denom.is_trivial()) {
      c.enter_fraction();
      c.s << "{{alignc ";
      print_smath_mul(numer, c, opnum, turn_around == 1);
      c.s << "} over {alignc ";
      print_smath_mul(denom, c, opnum, turn_around == 2);
      c.s << "}}";
      c.exit_fraction();
    } else if (!numer.is_trivial()) {
      if (!numer.get_adds().is_equal(_ex1) && has_diffs)
        c.s << "(";
      print_smath_mul(numer, c, opnum, turn_around == 1);
      if (!numer.get_adds().is_equal(_ex1) && has_diffs)
        c.s << ")";
    }

    // Print derivatives
    // Move partial derivatives to the front
    lst v_posderiv;
    if (!posderiv.is_equal(_ex1)) {
      for (const auto& mm : (is_a<mul>(posderiv) ? posderiv : lst{posderiv})) {
        if (ex_to<exderivative>(mm).is_partial())
          v_posderiv.prepend(mm);
        else
          v_posderiv.append(mm);
      }
    }
    lst v_negderiv;
    if (!negderiv.is_equal(_ex1)) {
      for (const auto& mm : (is_a<mul>(negderiv) ? negderiv : lst{negderiv})) {
        if (ex_to<exderivative>(mm).is_partial())
          v_negderiv.prepend(mm);
        else
          v_negderiv.append(mm);
      }
    }

    if (v_negderiv.nops() > 0) {
      c.enter_fraction();
      c.s << "{{alignc ";
      if (v_posderiv.nops() == 0)
        c.s << "1";
      else
        for (const auto& d : v_posderiv) d.print(c, level+1);
      c.s << "} over {alignc ";
      for (const auto& d : v_negderiv) d.print(c, level+1);
      c.s << "}}";
      c.exit_fraction();
    } else if (v_posderiv.nops() > 0) {
      c.s << " ";
      for (const auto& d : v_posderiv) d.print(c, level+1);
    }

    // Print differentials (incomplete derivatives)
    MSG_INFO(3, "Remaining incomplete differentials: " << posdiffs << " / " << negdiffs << endline);
    if (!negdiffs.is_equal(_ex1)) {
      c.enter_fraction();
      c.s << "{{alignc ";
      print_smath_ops(posdiffs, " ", c, false, opnum);
      c.s << "} over {alignc ";
      print_smath_ops(negdiffs, " ", c, false, opnum);
      c.s << "}}";
      c.exit_fraction();
    } else if (!posdiffs.is_equal(_ex1)) {
      c.s << " ";
      print_smath_ops(posdiffs, " ", c, false, opnum);
    }
  }
}

void print_ncmul_fraction(const expression& n, const expression& d, const imathprint& c, unsigned level) {
  MSG_INFO(2, "Printing ncmul fraction " << n << " / " << d << endline);
  c.enter_fraction();
  c.s << "{{alignc ";
  n.print(c, level+1);
  c.s << "} over {alignc ";
  d.print(c, level+1);
  c.s << "}}";
  c.exit_fraction();
}

void imathprint_ncmul(const ncmul& m, const imathprint& c, unsigned level) {
  MSG_INFO(2, "Printing ncmul " << m << endline);
  expression n = _expr1;
  expression d = _expr1;

  // Collect consecutive numerators and denominators and put them into a fraction
  for (const auto& f: m) {
    if (is_negpower(f)) {
      d = d / expression(f);
    } else {
      if (!d.is_equal(_ex1)) {
        // Print the fraction that was accumulated
        print_ncmul_fraction(n, d, c, level);
        n = f;
        d = _expr1;
      } else {
        n = n * expression(f);
      }
    }
  }

  if (!d.is_equal(_ex1)) {
    // Print the last fraction that was accumulated
    print_ncmul_fraction(n, d, c, level);
  } else if (!n.is_equal(_ex1)) {
    if (is_a<ncmul>(n)) {
      for (size_t i = 0; i < n.nops(); ++i) {
        n.op(i).print(c, level+1);
        if (i < n.nops() - 1) c.s << " ";
      }
    } else {
      n.print(c, level+1);
    }
  }
}

std::string roundNumber(const std::string& number, const int pos, bool& overflow) {
  MSG_INFO(4, "Rounding " << number << " to " << pos << endline);
  if (pos >= (int)number.size()) return number;
  if (pos <= 0) return "";

  cln::cl_I rnumber(number.substr(0, pos).c_str());
  if ((number.at(pos) - '0') >= 5)
    rnumber++;
  std::ostringstream str;
  str << rnumber;
  std::string result = str.str();

  if ((int)result.size() > pos) {
    // Rounding added a digit, e.g. 9995 rounded at position 3 results in 1000
    overflow = true;
    result.erase(result.size()-1);
  }

  return result;
}

void imathprint_cl_R(cln::cl_R num, const imathprint& c) {
  unsigned precision = (*c.poptions)[o_precision].value.uinteger;
  bool fixeddigits = (*c.poptions)[o_fixeddigits].value.boolean;

  if (cln::instanceof(num, cln::cl_RA_ring)) { // integer or rational
    if (cln::instanceof(num, cln::cl_I_ring)) {
      c.s << cln::the<cln::cl_I>(num);
      if (fixeddigits == false && precision > 0)
        c.s << "." << std::string(precision, '0'); // Add trailing zeros
    } else { // print rational as a fraction
      if (num < 0) c.s << "-";
      c.enter_fraction();
      c.s << "{{alignc ";
      imathprint_cl_R(cln::abs(cln::numerator(cln::the<cln::cl_RA>(num))), c);
      c.s << "} over {alignc ";
      imathprint_cl_R(cln::denominator(cln::the<cln::cl_RA>(num)), c);
      c.s << "}}";
      c.exit_fraction();
    }
  } else { // print float
    std::ostringstream numstream;
    numstream << num; // Use standard printing routine of CLN
    std::string number = numstream.str();
    MSG_INFO(4, "Original number=" << number << endline);

    // Normalize the number to the form 0.<number> * 10^<exponent>
    bool negative = (number[0] == '-');
    if (negative) number.erase(0,1);
    if (number[0] == '0') number.erase(0,1); // Remove leading zero
    std::size_t epos = number.find("L"); // CLN exponent marker (is always present) TODO: Force a single marker (cl_LF), but how?
    if (epos == std::string::npos) epos = number.find("d");
    if (epos == std::string::npos) epos = number.find("f");
    if (epos == std::string::npos) epos = number.find("s");
    int exponent = std::stoi(number.substr(epos+1));
    number.erase(epos);
    std::size_t ppos = number.find(".");
    if (ppos != std::string::npos) {
      number.erase(ppos, 1);
      exponent += (int)ppos;
    }
    std::size_t bpos = number.find_first_not_of('0');
    if (bpos == std::string::npos) { c.s << "0"; return; /* Should never happen */ }
    if (bpos > 0) {
      number.erase(0, bpos);
      exponent -= (int)bpos;
    }
    MSG_INFO(4, "Precision " << (*c.poptions)[o_precision].value.uinteger);
    MSG_INFO(4, ", Fixed digits " << ((*c.poptions)[o_fixeddigits].value.boolean ? "yes" : "no"));
    MSG_INFO(4, ", Forced exponent " << (*c.poptions)[o_exponent].value.integer);
    MSG_INFO(4, ", High limit " << (*c.poptions)[o_highsclimit].value.integer);
    MSG_INFO(4, ", Low limit " << (*c.poptions)[o_lowsclimit].value.integer << endline);


    if (fixeddigits && (precision == 0))
      throw std::runtime_error("It is not possible to print a number with zero significant digits (precision=0;fixedpoint=false)");

    // Place the decimal point. Note that move=0 is equivalent to an exponent of 1
    int move = 0; // Number of places to move the decimal point: + to the right (decreasing the exponent), - to the left (increasing the exponent)
    int fixedexponent = (*c.poptions)[o_exponent].value.integer;
    bool scientific = ((exponent > (*c.poptions)[o_highsclimit].value.integer) || (exponent <= -(*c.poptions)[o_lowsclimit].value.integer));
    if (fixedexponent != 0) {
      move = exponent - fixedexponent;
    } else if (scientific) {
      move = 1; // Scientific notation d.ddddd * 10^ddd
    } else {
      move = exponent; // Eliminate the need for an exponent;
    }
    MSG_INFO(4, "Moving point by " << move << endline);

    // Rounding
    bool overflow = false;
    if (fixeddigits)
      number = roundNumber(number, precision, overflow);
    else
      number = roundNumber(number, precision + move, overflow);
    if (overflow) {
      if (!scientific) move++;
      exponent++;
    }
    if (fixeddigits) number.erase(number.find_last_not_of('0') + 1); // Remove trailing zeros
    MSG_INFO(4, "Rounded number='" << number << "', move=" << move << ", exponent=" << exponent << endline);

    if (move < 0) {
       // Note that in fixed point notation this might result in things like 0.0000
      std::string zeros = std::string(-move, '0');

      if (!fixeddigits) {
        if ((number.size() == 0) && ((int)precision < -move))
          zeros = std::string(precision, '0'); // Avoid too many trailing zeros

        // Borderline case...
        if (precision > number.size() - move)
          number += std::string(precision - (number.size() - move), '0'); // Add some trailing zeros
      }

      number = "0" + imathprint::decimalpoint + zeros + number;
    } else if (move > (int)number.size()) {
      number = number + std::string(move - number.size(), '0');
      if (!fixeddigits) number = number + imathprint::decimalpoint + std::string(precision, '0'); // Add trailing zeros
    } else {
      if (!fixeddigits && (number.size() <= precision + move))
          number = number + std::string(precision + move - number.size(), '0');

      if (move == 0)
        number = "0" + imathprint::decimalpoint + number;
      else if (move != (int)number.size()) // Avoid trailing decimal point
        number.insert(move, imathprint::decimalpoint);
    }
    MSG_INFO(4, "Number after moving: " << number << endline);

    int remainingexponent = exponent - move;
    if (remainingexponent != 0)
      number = number + " cdot 10^" + std::to_string(remainingexponent);
    if (negative)
      number = "-" + number;
    MSG_INFO(4, "Final result: " << number << endline);

    c.s << number;
  }
} // imathprint_cl_R()

void imathprint_numeric(const numeric& n, const imathprint& c, unsigned level) {
  MSG_INFO(4, "imathprint_numeric()" << endline);
  (void)level;
  const cln::cl_number value = n.to_cl_N();
  const cln::cl_R r = cln::realpart(cln::the<cln::cl_N>(value));
  const cln::cl_R i = cln::imagpart(cln::the<cln::cl_N>(value));

  if (cln::zerop(i)) { // case 1, real:  x  or  -x
    imathprint_cl_R(r, c);
  } else {
    if (cln::zerop(r)) { // case 2, imaginary:  y*I  or  -y*I
      if (i==1)
        c.s << " i ";
      else {
        if (i == -1)
          c.s << " - i ";
        else {
          imathprint_cl_R(i, c);
          c.s << " i ";
        }
      }
    } else { // case 3, complex:  x+y*I  or  x-y*I  or  -x+y*I  or  -x-y*I
      imathprint_cl_R(r, c);
      if (i < 0) {
        if (i == -1) {
          c.s << " - i ";
        } else {
          imathprint_cl_R(i, c);
          c.s << " i ";
        }
      } else {
        if (i == 1) {
          c.s << " + i ";
        } else {
          c.s << "+";
          imathprint_cl_R(i, c);
          c.s << " i ";
        }
      }
    }
  }
}

void imathprint_power(const power& p, const imathprint& c, unsigned level) {
  // Is it correct to print x^(y^2) as x^y^2 or must we use brackets?
  (void)level;
  ex basis = get_basis(p);
  ex expon = get_exp(p);

  if (is_a<numeric>(expon) && (ex_to<numeric>(expon)).is_rational()) { // exponent is a rational number
    const numeric& exponent(ex_to<numeric>(expon));

    if (exponent.is_equal(*_num1_p)) {
      basis.print(c);
      return;
    } else if (exponent.is_negative()) { // exponent is negative
      c.enter_fraction();
      c.s << "{alignc 1 over {alignc ";
      pow(basis, -exponent).print(c);
      c.s << "}}";
      c.exit_fraction();
      return;
    } else if ((exponent.is_positive()) && (!exponent.is_integer())) { // exponent is a positive fraction
      if (exponent.denom().is_equal(2))
        c.s << "sqrt{";
      else {
        c.s << "nroot{";
        exponent.denom().print(c);
        c.s << "}{";
      }
      if (!exponent.numer().is_equal(*_num1_p)) {
        if (is_a<func>(basis) && ex_to<func>(basis).is_trig()) {
          ex_to<func>(basis).print_imath(c, exponent.numer());
          c.s << "}";
        } else {
          bool bracket = is_a<expairseq>(basis) || is_a<func>(basis) || is_a<power>(basis) || is_a<exderivative>(basis) ||
            (is_a<differential>(basis) &&
              (ex_to<differential>(basis).is_numerator() || !ex_to<differential>(basis).get_grade().is_equal(_ex1)));
          if (bracket) c.s << "(";
          basis.print(c);
          if (bracket) c.s << ")";
          c.s << "^{";
          exponent.numer().print(c);
          c.s << "}}";
        }
      } else {
        basis.print(c);
        c.s << "}";
      }
      return;
    }
  }
  // exponent is a positive integer, or is not rational, or is not even a numeric
  // Should we print things like x^(-a) as \frac(1)(x^a) ?
  if (is_a<func>(basis) && (ex_to<func>(basis).is_trig() || ex_to<func>(basis).is_pure())) {
    ex_to<func>(basis).print_imath(c, expon);
  } else {
    bool bracket = is_a<expairseq>(basis) || is_a<func>(basis) || is_a<power>(basis) || is_a<exderivative>(basis) ||
      (is_a<differential>(basis) &&
        (ex_to<differential>(basis).is_numerator() || !ex_to<differential>(basis).get_grade().is_equal(_ex1))) ||
      (is_a<numeric>(basis) && basis.info(info_flags::negative));
    if (bracket) c.s << "(";
    basis.print(c);
    if (bracket) c.s << ")";

    c.s << "^{";
    expon.print(c);
    c.s << "}";
  }
} // imathprint_power()

void imathprint_relational(const relational& r, const imathprint& c, unsigned level) {
  MSG_INFO(4, "imathprint_relational()" << endline);
  unsigned prec = r.precedence();
  if (level >= prec)
    c.s << '(';
  r.lhs().print(c, level+1);

  if (r.info(info_flags::relation_equal))
    c.s << " = "; // TODO: = or ==?
  else if (r.info(info_flags::relation_not_equal))
    c.s << " <> ";
  else if (r.info(info_flags::relation_less))
    c.s << " < ";
  else if (r.info(info_flags::relation_less_or_equal))
    c.s << " leslant ";
  else if (r.info(info_flags::relation_greater))
    c.s << " > ";
  else if (r.info(info_flags::relation_greater_or_equal))
    c.s << " geslant ";
  else
    c.s << " [unknown relational operator] ";

  r.rhs().print(c, level+1);
  if (level >= prec)
    c.s << ')';
}

void imathprint_symbol(const symbol& s, const imathprint& c, unsigned level) {
  MSG_INFO(4, "imathprint_symbol()" << endline);
  (void)level;
  std::string name = s.get_name();
  size_t pos = name.find_last_of("::");
  c.s << (pos == std::string::npos ? name : name.substr(pos+1));
}

void imathprint_wildcard(const wildcard& w, const imathprint& c, unsigned level) {
  MSG_INFO(4, "imathprint_wildcard()" << endline);
  (void)level;
  c.s << "$_" << w.get_label();
}

void imathprint_generic(const basic& b, const imathprint& c, unsigned level) {
  MSG_INFO(0, "!!! imathprint_generic() for '" << ex(b) << "'" << endline);
  (void)level;
  std::ostringstream temp;
  temp << latex << b;

  c.s << temp.str();
}

}
