/***************************************************************************
                          operands.cpp  -  description
                             -------------------
    begin                : Wed Jun 26 2002
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

#include <sstream>
#ifdef INSIDE_SM
#include <imath/printing.hxx>
#include <imath/msgdriver.hxx>
#include <imath/equation.hxx>
#include <imath/extintegral.hxx>
#include <imath/differential.hxx>
#include <imath/func.hxx>
#include <imath/unit.hxx>
#include <imath/utils.hxx>
#else
#include "printing.hxx"
#include "msgdriver.hxx"
#include "equation.hxx"
#include "extintegral.hxx"
#include "differential.hxx"
#include "func.hxx"
#include "unit.hxx"
#include "utils.hxx"
#endif
#include "operands.hxx"
#include "exderivative.hxx"

using namespace GiNaC;

/// Stores the result of previous split_ex() calls for faster execution
std::map<unsigned, exrec> remember_split;
typedef std::map<unsigned, exrec>::iterator remember_split_it;

exrec::exrec (const expression &exp, const operands &op1, const operands &op2)
    : e(exp), o1(op1), o2(op2), hits(0) {
  MSG_INFO(3, "Constructing exrec from " << e << endline);
}

exrec::~exrec() {
  #ifndef _MSC_VER
  // MSG_INFO(0,  "Destructing exrec with " << hits << " hits for " << e << endline); Beware, this may crash on exiting the Office!
  #endif
} // ~exrec()

#ifdef DEBUG_CONSTR_DESTR
exrec::exrec (const exrec& other) : e(other.e), o1(other.o1), o2(other.o2), hits(other.hits) {
  MSG_INFO(3, "Copying exrec from " << other.e << endline);
}

exrec& exrec::operator=(const exrec& other) {
  MSG_INFO(3, "Assigning exrec from " << e << " =" << other.e << endline);
  e = other.e;
  o1 = other.o1;
  o2 = other.o2;
  hits = other.hits;
  return *this;
}
#endif

ex mul_ops(const ex &op1, const ex &op2) {
  return (op1 * op2);
} //mul_ops()

ex add_ops(const ex &op1, const ex &op2) {
  return (op1 + op2);
} //add_ops()

operands::operands(const ex& ops_type) {
  MSG_INFO(3, "Constructing operands" << endline);
  if (!ops_type.is_equal(GINAC_ADD) && !ops_type.is_equal(GINAC_MUL))
    throw std::invalid_argument("operands::operands(): Type must be ADD or MUL");
  symbols = constants = units = functions = integrals = differentials = derivatives = adds = muls = powers = matrices = others = coefficient = ops_type;
  oper = (ops_type.is_equal(GINAC_ADD) ? &add_ops : &mul_ops);
  type = ops_type;
} //operands()

#ifdef DEBUG_CONSTR_DESTR
operands::~operands() {
  // MSG_INFO(3, "Destructing operands " << *this << endline); Beware, this may crash on exiting the Office
}

operands::operands(const operands& other) : type(other.type), symbols(other.symbols), constants(other.constants), units(other.units), functions(other.functions),
  integrals(other.integrals), differentials(other.differentials), derivatives(other.derivatives), adds(other.adds), muls(other.muls), powers(other.powers), matrices(other.matrices),
  others(other.others), coefficient(other.coefficient), oper(other.oper)
{
  MSG_INFO(3, "Copying operands from " << other << endline);
}

operands& operands::operator=(const operands& other) {
  MSG_INFO(3, "Assigning operands from " << other << endline);
  symbols = other.symbols;
  constants = other.constants;
  units = other.units;
  functions = other.functions;
  integrals = other.integrals;
  differentials = other.differentials;
  derivatives = other.derivatives;
  adds = other.adds;
  muls = other.muls;
  powers = other.powers;
  matrices = other.matrices;
  others = other.others;
  coefficient = other.coefficient;
  oper = other.oper;
  type = other.type;
  return *this;
}
#endif

void operands::split_ex(const expression &e, operands &o1, operands &o2) {
  // Split the ex into different types of operands (symbols, functions etc.)
  MSG_INFO(3, "Splitting expression " << e <<
    ", number of operands: " << (long)e.nops() << endline);

  unsigned hash = e.gethash();
  remember_split_it it = remember_split.find(hash);
  if ((it != remember_split.end()) && (it->second.e.is_equal(e))) {
    o1 = it->second.o1;
    o2 = it->second.o2;
    it->second.hits++;
    MSG_INFO(3, "split_ex: Cache hit for " << e << endline);
    return;
  }

  if (is_a<mul>(e))
    for (const auto& i : e) {
      if (is_negpower(i) && is_a<numeric>(get_exp(ex_to<power>(i))))
        // If the exponent is not a numeric, the double power will not be resolved by GiNaC
        // A work-around would be to call reduce_double_powers here
        o2.include(pow(i, -1));
      else
        o1.include(i);
    }
  else if (is_a<add>(e))
    for (const auto& i : e) {
      if (is_negex(i))
        o2.include(-i);
      else
        o1.include(i);
    }
  else if (is_negpower(e))
    o2.include(pow(e, -1));
  else if (is_a<power>(e))
    o1.include(e);
  else if (is_a<numeric>(e))
    o1.include(e);
  else if (is_a<constant>(e)) // Pi etc.
    o1.include(e);
  else if (is_a<function>(e))
    o1.include(e);
  else if (is_a<func>(e))
    o1.include(e);
  else if (is_a<extintegral>(e))
    o1.include(e);
  else if (is_a<matrix>(e))
    o1.include(e);
  else if (e.nops() == 0)
    o1.include(e);
  else
    MSG_ERROR(0, "Internal error: Unexpected expression type in split_ex()" << endline);

  if (msg::info().checkprio(4)) {
    std::ostringstream os;
    o1.print(os);
    o2.print(os);
    msg::info() << os.str() << endline;
  }

  if (it == remember_split.end())
    remember_split.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(e, o1, o2));
  else {
    it->second.e = e;
    it->second.o1 = o1;
    it->second.o2 = o2;
  }
  MSG_INFO(3, "split_ex: Cached result for " << e << endline);
}

void operands::include(const ex &what) {
  // Find the type of what and include it in the receiver
       if (is_a<symbol>(what)) symbols = oper(symbols, what);
  else if (is_a<constant>(what)) constants = oper(constants, what);
  else if (is_a<Unit>(what)) units = oper(units, what);
  else if (is_a<func>(what)) functions = oper(functions, what);
  else if (is_a<function>(what)) functions = oper(functions, what);
  else if (is_a<extintegral>(what)) integrals = oper(integrals, what);
  else if (is_a<differential>(what)) differentials = oper(differentials, what);
  else if (is_a<exderivative>(what)) derivatives = oper(derivatives, what);
  else if (is_a<add>(what)) adds = oper(adds, what);
  else if (is_a<mul>(what)) muls = oper(muls, what);
  else if (is_a<matrix>(what)) matrices = oper(matrices, what);
  else if (is_a<power>(what)) {
    const power& pow = ex_to<power>(what);
    ex base = get_basis(pow);
    ex exponent = get_exp(pow);
    if (is_a<numeric>(exponent) && (is_a<Unit>(base))) {
      units = oper(units, what);
    } else if (is_a<numeric>(exponent) && is_a<symbol>(base)) {
      symbols = oper(symbols, what);
    } else if (is_a<numeric>(exponent) && (is_a<constant>(base))) {
      constants = oper(constants, what);
    } else if (is_a<differential>(base)) {
      differentials = oper(differentials, what);
    } else if (is_a<exderivative>(base)) {
      derivatives = oper(derivatives, what);
    } else {
      powers = oper(powers, what);
    }
  } else if (is_a<numeric>(what)) {
      if (what == I)
        others = oper(others, I);
      else if (what == -I) {
        coefficient = ex_to<numeric>(oper(coefficient, -1));
        others = oper(others, I);
      } else
        coefficient = ex_to<numeric>(oper(coefficient, what));
  } else others = oper(others, what);
  MSG_INFO(6,  "Included expression " << what << endline);
}

void operands::exclude(const ex &what) {
  // Find the type of what and include it in the receiver
       if (is_a<symbol>(what)) symbols = oper(symbols, 1 / what);
  else if (is_a<constant>(what)) constants = oper(constants, 1 / what);
  else if (is_a<Unit>(what)) units = oper(units, 1 / what);
  else if (is_a<func>(what)) functions = oper(functions, 1 / what);
  else if (is_a<function>(what)) functions = oper(functions, 1 / what);
  else if (is_a<extintegral>(what)) integrals = oper(integrals, 1 / what);
  else if (is_a<differential>(what)) differentials = oper(differentials, 1 / what);
  else if (is_a<exderivative>(what)) derivatives = oper(derivatives, 1 / what);
  else if (is_a<add>(what)) adds = oper(adds, 1 / what);
  else if (is_a<mul>(what)) muls = oper(muls, 1 / what);
  else if (is_a<matrix>(what)) matrices = oper(matrices, 1 / what);
  else if (is_a<power>(what)) {
    const power& pow = ex_to<power>(what);
    ex base = get_basis(pow);
    ex exponent = get_exp(pow);
    if (is_a<numeric>(exponent) && (is_a<Unit>(base))) {
      units = oper(units, 1 / what);
    } else if (is_a<numeric>(exponent) && is_a<symbol>(base)) {
      symbols = oper(symbols, 1 / what);
    } else if (is_a<numeric>(exponent) && (is_a<constant>(base))) {
      constants = oper(constants, 1 / what);
    } else if (is_a<differential>(base)) {
      differentials = oper(differentials, 1 / what);
    } else if (is_a<exderivative>(base)) {
      derivatives = oper(derivatives, 1 / what);
    } else {
      powers = oper(powers, 1 / what);
    }
  } else if (is_a<numeric>(what)) {
    if (coefficient.is_equal(ex_to<numeric>(what)))
      coefficient = *_num1_p; // Avoid rounding errors for floating point coefficients
    else
      coefficient = ex_to<numeric>(oper(coefficient, 1 / what));
  } else {
    others = oper(others, 1 / what);
  }
}

bool checkmatrix(const matrix &m) {
  for (unsigned i = 0; i < m.nops(); i++) {
    if (is_a<matrix>(m.op(i))) {
      if (!checkmatrix(ex_to<matrix>(m.op(i))))
        return false;
    } else {
      if (!m.op(i).info(info_flags::numeric))
        return false;
    }
  }
  return true;
}

// TODO: Store the result of these queries for the next time!

bool operands::is_quantity() const {
  return (check_symbols(type) && constants.is_equal(type) && check_functions(type) && integrals.is_equal(type) &&
          differentials.is_equal(type) && derivatives.is_equal(type) && adds.is_equal(type) &&
          muls.is_equal(type) && powers.is_equal(type) && others.is_equal(type) && check_matrices(type));
}

bool operands::is_number() const {
  return (this->is_quantity() && units.is_equal(type));
  }

bool operands::is_trivial() const {
  return (this->is_number() && abs(ex_to<numeric>(coefficient)).is_equal(ex_to<numeric>(type)));
  }

bool operands::is_symbol() const {
  return (coefficient.is_equal(type) && !check_symbols(type) && constants.is_equal(type) && check_functions(type) &&
          adds.is_equal(type) &&  muls.is_equal(type) && powers.is_equal(type) && check_matrices(type) &&
          others.is_equal(type) && integrals.is_equal(type) && differentials.is_equal(type) && derivatives.is_equal(type));
}

bool operands::is_add() const {
  return type.is_equal(GINAC_ADD);
}

bool operands::is_mul() const {
  return type.is_equal(GINAC_MUL);
}
/*
void checksplit(const bool toplevel, const int opnum, std::ostream &os) {
  if (toplevel) {
    if (opnum == optstack::options->get(o_eqsplit).integer) {
      MSG_INFO(1, "Splitting equation at operand " << opnum << endline);
      MSG_INFO(2, "Interjected text: " << *optstack::options->get(o_eqsplittext).str << endline);
      if (optstack::options->get(o_eqalign).align == onlyleft) {
        os << "\\\\" << std::endl;
        if (*optstack::options->get(o_eqsplittext).str == "") {
          os << "&\\quad\\,\\,";
        } else {
          os << *optstack::options->get(o_eqsplittext).str;
        }
      } else if (optstack::options->get(o_eqalign).align == both) {
        os << "\\nonumber\\\\" << std::endl;
        if (*optstack::options->get(o_eqsplittext).str == "") {
          os << "&&";
        } else {
          os << *optstack::options->get(o_eqsplittext).str;
        }
      } else {
        os << "\\\\" << std::endl << *optstack::options->get(o_eqsplittext).str;
      }
    }
  }
}
*/
// These three functions exist because a non-commutative expression cannot be checked against '1' without throwing an exception
bool operands::check_functions(const ex& t) const {
  if (functions.info(info_flags::numeric)) {
    return (functions.is_equal(t));
  } else {
    return (false);
  }
}

bool operands::check_matrices(const ex& t) const {
  if (matrices.info(info_flags::numeric)) {
    return (matrices.is_equal(t));
  } else {
    return (false);
  }
}

bool operands::check_symbols(const ex& t) const {
  if (symbols.info(info_flags::numeric)) {
    return (symbols.is_equal(t));
  } else {
    return (false);
  }
}

void operands::print(std::ostream &os) const {
  os << "Symbols: " << symbols << std::endl;
  os << "Constants: " << constants << std::endl;
  os << "Units: " << units << std::endl;
  os << "Functions: " << functions << std::endl;
  os << "Adds: " << adds << std::endl;
  os << "Muls: " << muls << std::endl;
  os << "Powers: " << powers << std::endl;
  os << "Numerics: " << ex(coefficient) << std::endl;
  os << "Matrices: " << matrices << std::endl;
  os << "Integrals: " << integrals << std::endl;
  os << "Differentials: " << differentials << std::endl;
  os << "Derivatives:" << derivatives << std::endl;
  os << "Others: " << others << std::endl;
  os << "Quantity: " << (is_quantity() ? "true" : "false") << std::endl;
  os << "Number: " << (is_number() ? "true" : "false")  << std::endl;
  os << "Trivial: " << (is_trivial() ? "true" : "false")  << std::endl;
}
