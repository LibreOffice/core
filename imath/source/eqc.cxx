/***************************************************************************
    begin                : Sun Oct 21 2001
    copyright            : (C) 2016 by Jan Rheinlaender
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

#include <stdio.h>
#include <sstream>
#include <cmath>
#ifdef INSIDE_SM
#include <imath/eqc.hxx>
#include <imath/printing.hxx>
#include <imath/func.hxx>
#include <imath/msgdriver.hxx>
#else
#include "eqc.hxx"
#include "printing.hxx"
#include "func.hxx"
#include "msgdriver.hxx"
#endif
#include "operands.hxx"

using namespace GiNaC;

extern std::map<unsigned, exrec> remember_split;

// constructors
  eqc::eqc() : unitmgr() {
    MSG_INFO(3, "Constructing empty eqc"  << endline);
    previous_it = equations.end();
    nextlabel = 0;
    current_namespace = "";
  } // eqc::eqc()

  std::shared_ptr<eqc> eqc::clone() {
    MSG_INFO(3, "Taking deep copy of eqc"  << endline);
    auto result = std::make_shared<eqc>();

    // Ensure that previous_it points to the correct item in the clone
    result->previous_it = result->equations.end();
    for (auto e = equations.begin(); e != equations.end(); ++e) {
        eqrec_it newrec = result->equations.emplace(*e).first;
        if (e == previous_it)
            result->previous_it = newrec;
    }

    // Take copy of pointer data, preserving relationships
    for (const auto& o_eq : other_equations) {
        for (auto& e : result->equations) {
            if (o_eq->label == e.second.label) {
                result->other_equations.emplace_back(&e.second);
                break;
            }
        }
    }

    // The rest is plain data
    result->unitmgr = unitmgr;
    result->expressions = expressions;
    result->assignments = assignments;
    result->recent_assgn = recent_assgn;
    result->vars = vars;
    result->current_namespace = current_namespace;
    result->nextlabel = nextlabel;

    return result;
  }

namespace GiNaC {
ex EulernumEvalf() {
  // Note: numeric(cln::exp1()) will return precision as in cln::default_float_format
  return exp(*_num1_p); // Suggestion by Vladimir Kisil on ginac-devel
}
const constant Euler_number("Eulernum", EulernumEvalf, "e", domain::positive);
}

/// Exclude variables used in init.imath etc.
bool is_internal(const std::string& varname) {
  std::string s = varname.substr(0,1);
  return (varname == s + "_{." + s + ".}");
}

// methods

  bool eqc::check_eq(eqrec* eqr, std::vector<std::string>& names, expression &rhs, expression &arhs) {
    // check if the equation contained in the eqrec is an assignment and if so, assure that the symbol
    // is on the left hand side
    // The result of the last substitution is saved, and only newly added assignments
    // (contained in recent_assgn) are substituted.
    // If the equation relates two column vectors, then the check is done for each row.
    // TODO: Currently all rows must be assignments for the check to succeed

    // Only equalities can be used for finding values
    if (!(eqr->eq.info(info_flags::relation_equal)))
      return false;

    names.clear();
    expression lhs;
    expression alhs;
    bool error = false;
    bool lhs_is_quantity;
    bool rhs_is_quantity;

    if (eqr->subsed_lhs.is_empty()) {
      alhs = eqr->eq.lhs();
      lhs = alhs;
      arhs = eqr->eq.rhs();
      rhs = arhs;

      MSG_INFO(2, "Checking for quantities" << endline);
      lhs_is_quantity = is_quantity(alhs);
      rhs_is_quantity = is_quantity(arhs);

      MSG_INFO(2, "Doing full substitution of " << eqr->eq << endline);
      // The following steps might throw exceptions:
      // subs(): division by 0 because we are just blindly substituting assignments into equations
      //         Example: There is an equation dh/dt = 0 and we substitute t=200 into it -> division by zero
      // evalm(): Incompatible matrices, invalid mix of commutative/non-commutative objects
      // evalf(): floating point overflow/underflow if you have your precision value set too low
      // We ignore these errors so that we can take advantage of partial results
      if (!lhs_is_quantity) {
        try { alhs = alhs.expand(expand_options::expand_function_args);  } catch (std::exception&) {}
        try { alhs = alhs.subs(assignments).evalm(); } catch (std::exception&) { error = true; }
        try { lhs = alhs.evalf(); } catch (std::exception&) {}
      }
      if (!rhs_is_quantity) {
        try { arhs = arhs.expand(expand_options::expand_function_args); } catch (std::exception&) {}
        try { arhs = arhs.subs(assignments).evalm();  } catch (std::exception&) { error = true; }
        try { rhs = arhs.evalf();  } catch (std::exception&) {}
      }
    } else {
      MSG_INFO(3, "Using cached substituted value" << endline);
      alhs = eqr->subsed_lhs;
      arhs = eqr->subsed_rhs;
      try { lhs = alhs.evalf(); } catch (std::exception&) {}
      try { rhs = arhs.evalf(); } catch (std::exception&) {}
      lhs_is_quantity = is_quantity(lhs);
      rhs_is_quantity = is_quantity(rhs);
    }

    if (!recent_assgn.empty()) {
      MSG_INFO(2,  "Substituting recent assignments in " << eqr->eq << endline);
      if (!lhs_is_quantity) {
        try { alhs = alhs.expand(expand_options::expand_function_args); } catch (std::exception &e) { (void)e; }
        try { alhs = alhs.subs(recent_assgn).evalm(); } catch (std::exception &e) {(void)e; }
        try { lhs = alhs.evalf(); } catch (std::exception &e) { (void)e; }
      }
      if (!rhs_is_quantity) {
        try { arhs = arhs.expand(expand_options::expand_function_args); } catch (std::exception &e) { (void)e; }
        try { arhs = arhs.subs(recent_assgn).evalm(); } catch (std::exception &e) { (void)e; }
        try { rhs = arhs.evalf(); } catch (std::exception &e) { (void)e; }
      }
    }

    if (!error) {
      // Otherwise, if in the full substitution subs() failed, try again next time
      eqr->subsed_lhs = alhs;
      eqr->subsed_rhs = arhs;
    }

    // Convert lhs and rhs to a matrix, so we can handle all cases in one algorithm
    // Note: Don't do this earlier because expression::evalm() converts 1x1-matrices to scalars
    if (!(is_a<matrix>(lhs) && (ex_to<matrix>(lhs).cols() == 1) &&
          is_a<matrix>(rhs) && (ex_to<matrix>(rhs).cols() == 1) &&
          (ex_to<matrix>(lhs).rows() == ex_to<matrix>(rhs).rows()))) {
      // If lhs and rhs are not both column vectors with matching number of rows
      alhs = dynallocate<matrix>(1, 1, lst{alhs});
      arhs = dynallocate<matrix>(1, 1, lst{arhs});
      lhs = dynallocate<matrix>(1, 1, lst{lhs});
      rhs = dynallocate<matrix>(1, 1, lst{rhs});
    }

    // Swap lhs and rhs so that the lhs always has the symbol(s)
    if (!is_symbolic(ex_to<matrix>(lhs))) {
      if (is_symbolic(ex_to<matrix>(rhs))) {
        arhs.swap(alhs);
        rhs.swap(lhs);
      } else
        return false; // No assignments available
    }

    const matrix& l = ex_to<matrix>(lhs);
    for (unsigned r = 0; r < l.rows(); ++r) {
      names.push_back(ex_to<symbol>(l(r,0)).get_name());
      MSG_INFO(3, "Found assignment for variable " << names.back() << endline);
    }

    return true;
  } // check_eq()

  // TODO: Currently we only return true if all rows of rhs are quantities
  bool eqc::store_assgn(const std::vector<std::string> &names, const expression &rhs, const expression &arhs, eqrec* eqr) {
    bool result = true;
    const matrix& rh = ex_to<matrix>(rhs);
    const matrix& arh = ex_to<matrix>(arhs);

    for (unsigned r = 0; r < names.size(); ++r) {
      // If the symbol is a constant, then this is in fact just a coincidence
      // (or an error). The assignment is not stored.
      MSG_INFO(3, "store_assgn() for " << names[r] << endline);
      symrec& varr = vars.at(names[r]);
      if (varr.getsymtype() == t_constant) {
        throw (std::logic_error("Warning: equation " + eqr->label + " redefines constant " + names[r] +
                           "! Not registering."));
      }

      // Store the equation with the variable whose value it defines
      // If the rhs is a quantity, store the value for later
      if (is_quantity(rh(r,0))) {
        if (varr.has_value()) {
          MSG_ERROR(0,  "Warning: New value " << ex(arh(r,0)) << " given for " << names[r] << " in "
            << eqr->label << ". Previous value was " << varr.val << " in "
            << varr.assignments.front()->label << ". Overwriting old value. " << endline);
          varr.assignments.emplace_front(eqr); // Put the equation at the front
        } else {
          varr.val = rh(r,0);
          varr.aval = arh(r,0);
          varr.assignments.emplace_front(eqr); // The first equation gives the value
          MSG_INFO(2,  "Stored assignment " << names[r] << " == " << ex(arh(r,0)) << endline);
        }
      } else {
        varr.assignments.emplace_back(eqr);
        MSG_INFO(1,  "Stored assignment " << names[r] << " == " << ex(arh(r,0)) << endline);
        result = false;
      }
    }

    return result;
  } // store_assgn()

  void eqc::check_and_register(const expression &eq, const std::string& l) {
    // 1. Give the equation a label if it has none
    std::string label;
    if (l == "") {
      // Note: Currently this only happens when someone uses VALWITH etc.
      label = "{" + std::to_string(nextlabel++) + "}";
    } else {
      label = l;
    }
    MSG_INFO(2,  "eqc::check_and_register: Label '" << label << "'" << endline);

    // 2. Check for special case VALLABEL and clear this equation if it is there
    if (label == VALLABEL) {
      eqrec_it it_valeq = equations.find(VALLABEL);
      eqreclist_it it_othereqr = std::find(other_equations.begin(), other_equations.end(), &(it_valeq->second));
      if (it_othereqr != other_equations.end()) other_equations.erase(it_othereqr);
      if (it_valeq != equations.end()) equations.erase(it_valeq);
      vars.at(VALSYM).assignments.clear();
      vars.at(VALSYM).make_unknown(); // Clear old value
    }

    if (equations.find(label) != equations.end()) { // An equation with this label already exists
      MSG_INFO(0,  "Equation " << label << ": " << equations.at(label).eq << endline);
      throw std::invalid_argument("Label " + label + " already exists");
    }

    // 2.5 Get the label of the PREVIOUS equation (if it exists)
    std::string prevlabel("");
    if (previous_it != equations.end()) prevlabel = previous_it->second.label;

    // 3. Remove the PREVIOUS equation if it is temporary and no assignment
    // Note: Currently this only happens when someone uses VALWITH etc.
    if ((label != VALLABEL) && (previous_it != equations.end()) && previous_it->second.is_automatic()) {
      expression lhs = previous_it->second.eq.lhs();
      expression rhs = previous_it->second.eq.rhs();
      if (!(is_a<symbol>(lhs) || is_a<symbol>(rhs))) {
        deleq(previous_it);
        prevlabel = "";
      }
    }

    // 4. Register the equation and make it the PREVIOUS equation
    eqrec_it it_eqr_new = equations.emplace(std::piecewise_construct, std::forward_as_tuple(label), std::forward_as_tuple(eq, label, prevlabel)).first;
    if (label != VALLABEL)
      previous_it = it_eqr_new;

    // 5. Check if this equation could be useful to find values of variables (i.e, it is an
    // assignment for a variable)
    if (is_lib(label)) {
      // This is a generic library equation which can not be used for finding values
      MSG_INFO(3,  "Found library equation " << label << ". Not storing in other_equations." << endline);
    } else {
      other_equations.emplace_front(&(it_eqr_new->second));
      MSG_INFO(2,  "Added " << label << " to other_equations" << endline);
    }

    // 6. Print messages
    if (!is_lib(label)) {
      if (it_eqr_new->second.is_automatic() || (label == VALLABEL)) {
        MSG_INFO(2, "Registered equation " << label << ": " << it_eqr_new->second.eq << endline);
      } else {
        MSG_INFO(1, "Registered equation " << label << ": " << it_eqr_new->second.eq << endline);
      }
    } else {
      MSG_INFO(3, "Registered library equation " << label << ": " << it_eqr_new->second.eq << endline);
    }
  } // eqc::check_and_register

  void eqc::remove_assignment(const ex sym, std::list<ex>& syms) {
    syms.emplace_back(sym);
    for (auto& v : vars) {
      if (!v.second.assignments.empty() && v.second.has_value()) {
        if (v.second.assignments.front()->eq.has(sym)) {
          // The equation that is affected defines the value of the variable
          v.second.make_unknown();
          MSG_INFO(1,  "Therefore erased value of " << v.first << endline);
          remove_assignment(v.second.getsym(), syms);
        }
      }
    } // for (v ...)
  } // eqc::remove_assignment()

  void eqc::remove_subsed(const ex& sym) {
    for (auto& e : equations) {
      if (!is_lib(e.first)) {
        MSG_INFO(3,  "Checking " << e.first << ": " << e.second.eq << endline);

        if (!e.second.subsed_lhs.is_empty() && e.second.eq.has(sym)) {
          e.second.subsed_lhs.clear();
          e.second.subsed_rhs.clear();
          MSG_INFO(3,  "Removed stored substitution result of " << e.second.eq
              << " because value of " << sym << " was removed" << endline);
        }
      }
    }
  } // eqc::removed_subsed()

  void eqc::deleq (const std::string &which) {
    eqrec_it it_eqr = equations.find(which);
    if (it_eqr == equations.end())
      // An equation with this label does not exist
      // This is frequently the case if an equation was never registered, therefore no error message
      return;

    deleq(it_eqr);
  }

  void eqc::deleq(eqrec_it which) {
    std::string label = which->first;
    if (is_lib(label))
      MSG_WARN(0,  "Warning: Requesting deletion of " << label
        << ", which is a library equation!" << endline);

    MSG_INFO(1,  "Deleting equation with label: " << label << endline);

    // 1. Search the equation in other_equations
    // Note: Using _cit here breaks the debian build on trusty
    eqreclist_it it_othereqr = std::find(other_equations.begin(), other_equations.end(), &(which->second));

    if (it_othereqr != other_equations.end()) {
      // 1.1 This equation is stored (only!) in other_equations
      MSG_INFO(2,  "Removed " << label << " from other_equations." << endline);
      other_equations.erase(it_othereqr);
      // Beware: it_othereqr is invalidated after this!
    } else {
    // 2. Search the equation in the assignments for variables
    // Note: This does not cover equations like x = VAL(y), even if y looses its value, x will still retain it
    // At this point this is considered a FEATURE, see Feature Tracker #3526107
      for (auto& v : vars) {
        // Note: Using _cit here breaks the debian build on trusty
        eqreclist_it it_assignmenteqr = std::find(v.second.assignments.begin(), v.second.assignments.end(), &(which->second));
        if (it_assignmenteqr != v.second.assignments.end()) { // The label is contained in the list
          if (it_assignmenteqr == v.second.assignments.begin()) {// The variable's value is defined by the equation
            // 2.1 Remove the equation label from assignments and the value of the variable
            v.second.assignments.erase(it_assignmenteqr);
            // Beware: it_assignmenteqr is invalidated after this!
            v.second.make_unknown();
            MSG_INFO(1, "Erased value of " << v.first << endline);

            // 2.2 Check if any other variables depend on the value of this variable
            std::list<ex> dependentvars; // This list collects all symbols that have no value any more
            remove_assignment(v.second.getsym(), dependentvars);

            // 2.3 remove the assignment(s) from the list of (global) assignments
            for (const auto& dependentvar : dependentvars) {
              exmap::iterator a = assignments.find(dependentvar);
              if (a != assignments.end()) {
                MSG_INFO(2,  "Removing " << dependentvar << " = " << a->second << " from list of global assignments." << endline);
                assignments.erase(a);
              }
              a = recent_assgn.find(dependentvar);
              if (a != recent_assgn.end()) recent_assgn.erase(a);
              remove_subsed(dependentvar);
            }
          } else { // The value of the variable is not affected by deleting this equation
            v.second.assignments.erase(it_assignmenteqr);
            // Beware: it_assignmenteqr is invalidated after this!
          }
          MSG_INFO(1,  "Removed " << label << " from list of assignments for "
                             << v.first << endline);
          break; // Stop searching, since an equation can only be an assignment for one variable
        }
      } // for (v ...)
    }

    // Remove the equation from the list of registered equations
    if (which == previous_it) previous_it = equations.end(); // The previous equation was deleted
    equations.erase(which);

    MSG_INFO(1, "Unregistered equation " << label << endline);
  } // eqc::deleq()

  void eqc::register_constant (const expression &eq) {
    if (! is_a<symbol>(eq.lhs()) )
      throw std::invalid_argument("Warning: Left hand side for 'constant' is no symbol! Ignoring.");

    std::string varname = ex_to<symbol>(eq.lhs()).get_name();
    const auto& varr = vars.find(varname);
    if ((varr != vars.end()) && (varr->second.getsymtype() == t_constant))
      throw std::invalid_argument("Error: Constant already exists! Ignoring.");

    if (!is_quantity(eq.rhs())) {
      // Automatically evaluate the RHS to a quantity (if possible)
      const extsymbol& valsym = ex_to<extsymbol>(getsym(VALSYM));
      check_and_register(equation(valsym, eq.rhs(), relational::equal, _expr0), VALLABEL);

      varr->second.val = find_value_of(valsym);
      deleq(VALLABEL);
    } else {
      // This avoids find_value_of() which always evaluates numbers to floats
      varr->second.val = eq.rhs();
    }

    varr->second.setsymtype(t_constant);
    if (!is_quantity(varr->second.val))
      throw std::invalid_argument("Warning: Constant " + varname + " is no quantity.");
    recent_assgn.emplace(varr->second.getsym(), varr->second.val);
    MSG_INFO(1, "Registered constant: " << varname << " = " << varr->second.val << endline);
  } // eqc::register_constant()

  void eqc::register_function (const std::string &n, exvector &args, const unsigned hints, const std::string& printname) {
    // Note: A symbol "n" always exists before this call because the \function{} statement gets parsed first!
    func::registr(n, args, hints, printname);
    vars.at(n).setsymtype(t_function);
  } // eqc::register_function()

  void eqc::register_expression (const expression &ex, const std::string& l) {
    if (expressions.find(l) != expressions.end()) { // An expressions with this label already exists
      MSG_INFO(0,  "Expression " << l << ": " << expressions.at(l) << endline);
      throw std::invalid_argument("Label " + l + " already exists");
    }
    expressions.emplace(std::piecewise_construct, std::forward_as_tuple(l), std::forward_as_tuple(ex));
  }

  void eqc::begin_namespace(const std::string& ns) {
    current_namespace += (current_namespace.size() > 0 ? "::" : "") + ns;
    MSG_INFO(0, "Current namespace: " << current_namespace << endline);
  }

  void eqc::end_namespace(const std::string& ns) {
    if (ns.size() == 0) {
      current_namespace = "";
      return;
    }

    // TODO: Check parameter ns for consistency
    size_t cpos = current_namespace.find_last_of("::");

    if (cpos == std::string::npos)
      current_namespace = "";
    else
      current_namespace = current_namespace.substr(0, cpos);

    MSG_INFO(0, "Current namespace: " << current_namespace << endline);
  }

  std::string eqc::varname_ns(const std::string& varname) const {
    if (varname.find("::") == 0)
      return varname.substr(2); // Access top-level namespace from inside other namespace

    if (current_namespace.size() == 0 || varname.find("::") != std::string::npos || func::is_lib(varname))
      return varname;
    else
      return current_namespace + "::" + varname;
  }

  std::string eqc::label_ns(const std::string& label, const bool check) const {
    if (label.find("::") == 0)
      return label.substr(2); // Access top-level namespace from inside other namespace

    if (current_namespace.size() == 0 || label.find("::") != std::string::npos) {
      return label;
    } else if (check) {
      if (is_label(current_namespace + "::" + label))
        return current_namespace + "::" + label;
      else
        return label;
    } else {
      return current_namespace + "::" + label;
    }
  }

  std::string eqc::exlabel_ns(const std::string& label, const bool check) const {
    if (label.find("::") == 0)
      return label.substr(2); // Access top-level namespace from inside other namespace

    if (current_namespace.size() == 0 || label.find("::") != std::string::npos) {
      return label;
    } else if (check) {
      if (is_expression_label(current_namespace + "::" + label))
        return current_namespace + "::" + label;
      else
        return label;
    } else {
      return current_namespace + "::" + label;
    }
  }

  symrec::symrec(const symtype t, const std::string& varname, const symprop p) : sym(dynallocate<extsymbol>(varname)), type(t) {
    MSG_INFO(3, "Constructing symrec from " << varname  << endline);
    sym.add_reference(); // otherwise the symbol will be deallocated after we pass it out...
    setsymprop(p);
    make_unknown();
  } // symrec::symrec()

  symrec& symrec::operator=(const symrec& other) {
    MSG_INFO(3, "Assigning symrec from " << other.sym  << endline);
    sym = other.sym;
    type = other.type;
    prop = other.prop;
    val = other.val;
    assignments = std::list<eqrec*>(other.assignments.begin(), other.assignments.end());
    return *this;
  }

#ifdef DEBUG_CONSTR_DESTR
  symrec::symrec() : sym(dynallocate<extsymbol>("empty symbol")) {
    MSG_INFO(3, "Constructing empty symrec" << endline);
    sym.add_reference(); // otherwise the symbol will be deallocated after we pass it out...
  }
  symrec::symrec(const symrec& other) : sym(other.sym), type(other.type), prop(other.prop), val(other.val) {
    MSG_INFO(3, "Copying symrec from " << other.sym  << endline);
    assignments = std::list<eqrec*>(other.assignments.begin(), other.assignments.end());
  }
  symrec::~symrec() {
    MSG_INFO(3, "Destructing symrec for " << sym  << endline);
  }
#endif

  void symrec::setsymprop(const symprop p) {
    prop = p;
    if (p == p_real) {
      sym.make_c();
      sym.make_real();
    } else if (p == p_pos) {
      sym.make_c();
      sym.make_pos();
    } else if ((p == p_vector) || (p == p_matrix)) {
      sym.make_nc();
      sym.make_complex();
    } else {
      sym.make_complex();
      sym.make_c();
    }
  }

  void symrec::make_unknown() {
    MSG_INFO(3, "symrec::make_unknown()" << endline);
    val = sym;
  }

  bool symrec::has_value() const {
    return !val.is_equal(sym);
  }

  expression eqc::getsym (const std::string& varname, const symprop p) {
    MSG_INFO(3,  "getsym() for " << varname << endline);
    if (varname == "%pi") {
      return Pi;
    } else if (varname == "%e") {
          return Euler_number;
    } else if (varname == "i") {
      return I;
    } else if (vars.find(varname) == vars.end()) { // create a new variable
      MSG_INFO(3,  "Creating new variable " << varname << endline);
      const symrec_it& v = (vars.emplace(std::piecewise_construct, std::forward_as_tuple(varname), std::forward_as_tuple(t_variable, varname, p))).first;
      return v->second.getsym();
    } else {
      MSG_INFO(3, "Returning existing variable " << varname << endline);
      return (vars.at(varname).getsym());
    }
  } // eqc::getsym()

  expression* eqc::getsymp(const std::string& varname, const symprop p) {
    MSG_INFO(3,  "getsymp() for " << varname << endline);
    return new expression(getsym(varname, p));
  } // eqc::getsymp()

  void eqc::setsymprop(const std::string& varname, const symprop p) {
    symrec_it v = vars.find(varname);

    if (v != vars.end()) {
      v->second.setsymprop(p);
      v->second.make_unknown(); // The value certainly gets lost here
    }
  }

  symtype eqc::getsymtype(const std::string& varname) {
    symrec_cit v = vars.find(varname);
    if (v != vars.end())
      return v->second.getsymtype();
    else
      return t_none; // symbol does not exist
  } // eqc::getsymtype()

  symprop eqc::getsymprop(const std::string& varname) {
    symrec_cit v = vars.find(varname);
    if (v != vars.end())
      return v->second.getsymprop();
    else
      return p_complex;
  }

  bool eqc::is_label(const std::string &s) const {
    MSG_INFO(3, "Checking if label exists: " << s << endline);
    return (equations.find(s) != equations.end());
  } // eqc::is_label()

  bool eqc::is_expression_label(const std::string &s) const {
    MSG_INFO(3, "Checking if expression label exists: " << s << endline);
    return (expressions.find(s) != expressions.end());
  }

  bool eqc::is_lib(const std::string &s) const {
    return (std::string(s, 0, 4) == "lib:");
  } // eqc::is_lib()

  bool eqc::has_value(const symbol& s) const {
    symrec_cit v = vars.find(s.get_name());
    if (v == vars.end())
      throw(std::range_error("has_value: Symbol '" + s.get_name() + "' is not registered with the compiler"));
    return (v->second.has_value());
  } // eqc::has_value()

  expression eqc::get_assignment(const symbol& s) const {
    symrec_cit v = vars.find(s.get_name());
    if (v == vars.end())
      throw(std::range_error("has_value: Symbol '" + s.get_name() + "' is not registered with the compiler"));
    if (v->second.assignments.empty())
      return equation();

    return (v->second.assignments.front()->eq);
  }

  expression eqc::get_value(const symbol &s) const {
    symrec_cit v = vars.find(s.get_name());
    if (v == vars.end())
      throw(std::range_error("get_value: Symbol '" + s.get_name() + "' is not registered with the compiler"));
    return (v->second.val);
  } // eqc::get_value()

  bool eqc::find_values(const symbol &var, numeric &val, expression &unit, expression &value, const lst &assgn, const bool tofloat) {
    bool found_value = false;
    MSG_INFO(1,  "Searching value of " << var << endline);

    // 1.0 Prepare the equation list if optional parameters are being used
    std::list<eqrec_it> tempeqs;
    eqrec_it preveq = previous_it; // Needs to be restored later

    if (assgn.nops() != 0) {
      for (const auto& a : assgn) {
        if (is_a<equation>(a)) {
          MSG_INFO(2,  "Creating temporary equation " << a << endline);
          try {
            check_and_register(ex_to<equation>(a)); // previous_it is set to the newly registered equation
            tempeqs.emplace_back(previous_it);
          } catch (std::exception &e) {
            MSG_ERROR(0,  "Failed to register temporary equation. Reason: " << e.what() << endline);
            for (auto it_eqr : tempeqs) deleq(it_eqr);
            throw (std::invalid_argument("Error: Could not find quantity of " + var.get_name()));
          }
        } else if (is_a<relational>(a)) { // TODO: WHY WHY WHY???
          MSG_INFO(2,  "Creating temporary equation from relational " << a << endline);
          try {
            const relational& r = ex_to<relational>(a);
            equation eqr(r.lhs(), r.rhs(), relational::equal, _expr0);
            check_and_register(eqr);
            tempeqs.emplace_back(previous_it);
          } catch (std::exception &e) {
            MSG_ERROR(0,  "Failed to register temporary equation. Reason: " << e.what() << endline);
            for (auto it_eqr : tempeqs) deleq(it_eqr);
            throw (std::invalid_argument("Error: Could not find quantity of " + var.get_name()));
          }
        } else {
          MSG_INFO(2,  "Not an equation: " << a << endline);
          for (auto it_eqr : tempeqs) deleq(it_eqr);
          throw (std::invalid_argument("Error: Equation expected"));
        }
      }
    }

    // 1.1 Check if the variable is a constant or already has a value, then we can skip
    // all the iterations
    std::string varname = var.get_name();
    if (has_value(var)) {
      found_value = true;
      MSG_INFO(1,  "Variable/constant already has a value. Not iterating." << endline);
    }

    symrec_it it_varname = vars.find(varname);
    symrec_it v;
    eqrec_it e;

    if (msg::info().checkprio(1)) {
      msg::info() << "Available other equations: " << endline;
      for (const auto& eqr : other_equations) {
        msg::info() << eqr->label << ": " << eqr->eq << endline;
      }
    }
    if (msg::info().checkprio(2)) {
      msg::info() << "Available equations: " << endline;
      for (const auto& eqrp : equations)
        if (!is_lib(eqrp.first))
          msg::info() << eqrp.first << ": " << eqrp.second.eq << endline;
    }
    if (msg::info().checkprio(2)) {
      msg::info() << "Available assignments: " << assignments << endline
                  << "Recent assignments: " << recent_assgn  << endline;
    }

    exmap new_assgn; // Collect new assignments found during an iteration loop
    unsigned num_it = 0; // just for the statistics...
    expression result;
    expression aresult;
    std::vector<std::string> names;

    // 2. Find all possible assignments and further equations for the variable
    do {
      if (msg::info().checkprio(1) && !found_value)
        msg::info() << "Starting iteration #" << ++num_it << endline;

      // 2.1 Iterate over other_equations and check each equation whether it evaluates
      // to an assignment or even a quantity
      // For the special case of searching for the value of VALSYM (which means that the user
      // asked for the value of an expression, not of a symbol, e.g. \val{\tan\alpha}),
      // there are two possible solutions:
      // a) The rhs of the equation VALLABEL: VALSYM = ... evaluates to the result
      // b) The lhs or rhs of another equation correspond to this expression. If yes, we
      // use this as a new assignment for VALSYM. Example:
      // User: \val{\tan\alpha} -> eqc creates equation: VALSYM = \tan\alpha. Another equation exists
      // with \tan\alpha = 3 \tan\beta, then this amounts to an assignment VALSYM = 3 \tan\beta
      // TODO: This is not always usefull, e.g. we have y = a x^2 + b x + c and then ask for VAL(a x^2 + b x + c)
      // By the above logic, this would be equivalent to VALSYM = y...
      //
      if (!found_value) {
        for (auto it_othereqr = other_equations.begin(); it_othereqr != other_equations.end(); ++it_othereqr) {
          MSG_INFO(2,  "Investigating other equation " << (*it_othereqr)->eq << endline);

          if (check_eq(*it_othereqr, names, result, aresult)) {
            // The equation is an assignment. This also handles case a) for varname = VALSYM
            try {
              if (store_assgn(names, result, aresult, *it_othereqr)) {
                const matrix& res = ex_to<matrix>(aresult);
                for (unsigned r = 0; r < names.size(); ++r) {
                  if ((*it_othereqr)->label != VALLABEL) {
                    new_assgn.emplace(vars.at(names[r]).getsym(), res(r,0));
                    MSG_INFO(1,  "Found value from other_equations: " << names[r] << " == " << res(r,0) << endline);
                  }
                  if (names[r] == varname)
                    found_value = true;
                }
                if (found_value) {
                  MSG_INFO(1,  "Found value of " << varname << ". Stopping iteration." << endline);
                  other_equations.erase(it_othereqr); // The equation is stored with the variable now
                  break; // jump out of the for... loop
                }
              }
              it_othereqr = other_equations.erase(it_othereqr); // The equation is stored with the variable now
              --it_othereqr;
            } catch (std::exception &except) {
              MSG_ERROR(0,  except.what() << endline);
            }
          }
        } // for(it_othereqr ...)
      }

      // 2.1.5 For the special case of varname being VALSYM, we also need to look at all the equations
      // because they might have been moved out of other_equations by now. The reason is that the
      // symrec for VALSYM gets deleted every time
      if (!found_value && (varname == VALSYM)) {
        for (auto& eqrp : equations) {
          if ((previous_it != equations.end()) && (eqrp.first == previous_it->first)) continue; // Avoid "duplicate value" warning
          if (is_lib(eqrp.first)) continue; // Generic library equation
          if (eqrp.first == VALLABEL) continue;
          MSG_INFO(2,  "VALSYM: Investigating equation '" << eqrp.first << "': "  << eqrp.second.eq << endline);

          if (eqrp.second.eq.lhs().is_equal(equations.at(VALLABEL).eq.rhs())) {
            matrix r(1,1);
            r(0,0) = eqrp.second.eq.rhs();
            std::vector<std::string> vec;
            vec.push_back(VALSYM);
            store_assgn(vec, r, r, &(eqrp.second));
          } else if (eqrp.second.eq.rhs().is_equal(equations.at(VALLABEL).eq.rhs())) {
            matrix l(1,1);
            l(0,0) = eqrp.second.eq.lhs();
            std::vector<std::string> vec;
            vec.push_back(VALSYM);
            store_assgn(vec, l, l, &(eqrp.second));
          }
        }
      }

      // 2.2 Iterate over the equations stored in vars[...].assignments and check
      // each equation whether it evaluates to an assignment
      if (!found_value) {
        v = it_varname;
        do {
          if (!v->second.has_value() && (v->second.getsymtype() != t_function)) {
            // For every variable that has no value yet
            if (msg::info().checkprio(2))
              if (!is_internal(v->first)) // Avoid unnecessary debug output. Note: is_internal() is probably more expensive than checking begin() against end()
                MSG_INFO(2,  "Investigating assignments for " << v->first << endline);

            for (auto it_assignmenteqr = v->second.assignments.begin(); it_assignmenteqr != v->second.assignments.end(); ++it_assignmenteqr) {
              // For all the assignments stored with this variable
              MSG_INFO(2,  "Investigating " << (*it_assignmenteqr)->eq << endline);

              if (check_eq(*it_assignmenteqr, names, result, aresult)) {
                // Note: Using store_assgn() here is not useful
                const matrix& res = ex_to<matrix>(result);
                const matrix& ares = ex_to<matrix>(aresult);
                unsigned r = 0;
                for (; r < names.size(); ++r)
                  if (v->first == names[r]) break; // Find this variable in the matrix of assignments
                if (r == names.size()) continue; // Variable not found: How can this be?
                MSG_INFO(2,  "Found assignment " << names[r] << " == " << ares(r,0) << endline);

                // If the result is a quantity, store the value for later
                // If this is the symbol we are searching a value for, the search is finished
                if (is_quantity(res(r,0)) || is_a<matrix>(res(r,0))) {
                  v->second.val = res(r,0); // A value was found for the variable
                  v->second.aval = ares(r,0);
                  if (names[r] != VALSYM)
                    new_assgn.emplace(vars.at(names[r]).getsym(), ares(r,0));
                  MSG_INFO(1,  "Found value from equations: " << names[r] << " == " << ares(r,0) << endline);
                  // Move this equation label to the front of vars[...].assignments
                  v->second.assignments.emplace_front(*it_assignmenteqr);
                  it_assignmenteqr = v->second.assignments.erase(it_assignmenteqr);
                  --it_assignmenteqr;

                  if (v->first == varname) {
                    found_value = true;
                    MSG_INFO(1,  "Found value of " << varname << ". Stopping iteration." << endline);
                  }

                  break; // jump out of the for(it_assignmenteqr ...) loop, this variable has a value
                }
              } // if (check_eq ...)
            } // for (it_assignmenteqr ...)
          } // if (has_value ...)
         if (found_value) break; // jump out of the do {} while(v ...) loop
         v++;
         if (v == vars.end()) v = vars.begin();
        } while (v != it_varname); // do {} while()
      } // if (!found_value)

      // 2.3 All the assignments stored in recent_assgn have been applied to the equations in
      // this iteration. Merge them with the global assignments and move the contents of
      // new_assgn to recent_assgn
      if (!found_value) { // If a value was found, the iteration was incomplete!
        MSG_INFO(1, "Completed iteration. Recent assignments that have been substituted: "
                    << recent_assgn << endline << "New assignments for next iteration: " << new_assgn << endline);

        assignments.insert(recent_assgn.begin(), recent_assgn.end());
        recent_assgn.swap(new_assgn);
        new_assgn.clear();
      }
    } while (!recent_assgn.empty() && !found_value);

    // 2.4 Add the new assignments found to the recent assignments, for the next call of this function
    if (!new_assgn.empty()) {
      recent_assgn.insert(new_assgn.begin(), new_assgn.end());
      new_assgn.clear();
    }

    // 3. Check whether we found too few or too many assignments for the variable
    symrec& varsr = vars.at(varname);
    if ((varname == VALSYM) && (varsr.assignments.size() > 1))
      varsr.assignments.remove(&(equations.at(VALLABEL)));

    if (!varsr.has_value() && varsr.assignments.empty()) {
      for (auto it_eqr : tempeqs) deleq(it_eqr);
      throw (std::invalid_argument("Variable " + varname + " does not have a value"));
    }

    // The variable does not have a value, but multiple assignments try to define one
    if (!has_value(var) && varsr.assignments.size() > 1) {
      MSG_WARN(0,  "Warning: Variable " << varname << " has " << (long)varsr.assignments.size()
                          << " possible values." << endline);
      MSG_INFO(1,  "Possible equations for " << varname << " after search: " << endline);
      for (auto it_assignmenteqr = varsr.assignments.begin(); it_assignmenteqr != varsr.assignments.end(); ) {
        if ((varname != VALSYM) && ((*it_assignmenteqr)->subsed_lhs != varsr.getsym())) {
          // This also catches the case that subsed_lhs is empty because there was an error in check_eq subs/evalf
          MSG_INFO(1,  (*it_assignmenteqr)->label << ": " << (*it_assignmenteqr)->eq
            << ", removing because symbol is on right-hand side" << endline);
          it_assignmenteqr = varsr.assignments.erase(it_assignmenteqr);
        } else {
          MSG_INFO(1,  (*it_assignmenteqr)->label << ": " << (*it_assignmenteqr)->eq << endline);
          ++it_assignmenteqr;
        }
      }
    }

    // 4. Find the best value for the variable
    std::list<expression> values;
    if (has_value(var)) {
      // 4.1 The variable has a value which is a quantity
      ::operands n(GINAC_MUL), d(GINAC_MUL);
      ::operands::split_ex(varsr.val, n, d);
      val = ex_to<numeric>(n.get_coefficient());
      unit = n.get_units()/d.get_units();
      value = tofloat ? varsr.val : varsr.aval;

      // If optional parameters were used, clean up
      if (assgn.nops() != 0) {
        for (auto it_eqr : tempeqs) deleq(it_eqr);
        previous_it = preveq;
      }

      return true;
    } else {
      // 4.2 Collect all the possible values, detect if assignment is the wrong way around
      //     (symbol on the left-hand side).
      // TODO: This doesn't work if we are searching for VALSYM
      for (const auto& assignmenteqr : varsr.assignments) {
        if (assignmenteqr->subsed_rhs.is_empty())
          values.emplace_back((assignmenteqr->eq.rhs() == varsr.getsym()) ?
            assignmenteqr->eq.lhs() : assignmenteqr->eq.rhs());
        else
          values.emplace_back((assignmenteqr->subsed_rhs == varsr.getsym()) ?
            assignmenteqr->subsed_lhs : assignmenteqr->subsed_rhs);
      }
    }

    // 5. The variable does not have a quantity. Return the best of the values found
    // TODO: Choose the equation with the least
    // unknown variables in it or the last one defined by the user?
    if (msg::info().checkprio(0)) {
      msg::info() << "Possible values for " << varname << " after simplification: ";
      std::ostringstream os;
      copy(values.begin(), values.end(), std::ostream_iterator<expression>(os, "; "));
      msg::info() << os.str();
      msg::info() << endline;
    }
    value = values.front(); // Returns the last equation defined by the user

    if (assgn.nops() != 0) {
      for (auto it_eqr : tempeqs) deleq(it_eqr);
      previous_it = preveq;
    }

    return false;
  } // eqc::find_values()

  expression eqc::find_value_of(const symbol &var, const lst &assgn, const bool tofloat) {
    numeric v;
    expression u, value;
    find_values(var, v, u, value, assgn, tofloat);
    return (value);
  } // eqc::find_value_of()

  expression eqc::find_quantity_of(const symbol &var, const lst &assgn) {
    numeric v;
    expression u, value;
    if (!find_values(var, v, u, value, assgn, true)) {
      MSG_ERROR(0,  "Value found: " << value << endline);
      throw (std::invalid_argument("Variable '" + var.get_name() + "' does not have a quantity"));
    }
    return (value.evalf());
  } // eqc::find_quantity_of()

  numeric eqc::find_numval_of(const symbol &var, const lst &assgn) {
    // Note: This function discards any units the result might contain!
    numeric v;
    expression u, value;
    if (!find_values(var, v, u, value, assgn, true)) {
      MSG_ERROR(0,  "Value found: " << value << endline);
      throw (std::invalid_argument("Variable '" + var.get_name() + "' does not have a numeric value"));
    }
    if (!u.is_equal(_ex1))
      MSG_ERROR(0,  "Warning: Numerical value requested, but units were found for "
        << var << endline);
    return (v);
  } //eqc::find_numval_of()

  expression eqc::find_units_of(const symbol &var, const lst &assgn) {
    numeric v;
    expression u, value;
    if (!find_values(var, v, u, value, assgn, true)) {
      MSG_ERROR(0,  "Value found: " << value << endline);
      throw (std::invalid_argument("Error: Variable '" + var.get_name() + "' does not have a quantity"));
    }
    if (v != 1)
      MSG_ERROR(0,  "Warning: Units requested, but numerical value was found for "
        << var << endline);
    return (u);
  } //eqc::find_units_of()

  exhashmap<ex> eqc::find_variable_values(const expression& e) const {
    MSG_INFO(1, "Searching variable values for " << e << endline);
    exhashmap<ex> result;

    for (const_preorder_iterator i = e.preorder_begin(); i != e.preorder_end(); ++i) {
      if (is_a<symbol>(*i) && (result.find(ex_to<symbol>(*i)) == result.end())) {
        MSG_INFO(3, "Found contained symbol " << *i << " with value " << get_value(ex_to<symbol>(*i)) << endline);
        result[ex_to<symbol>(*i)] = get_value(ex_to<symbol>(*i)); // exhashmap has no emplace()
      }
    }

    return result;
  }

  matrix eqc::iterate(const matrix& syms, const matrix& exprs, const matrix& start, const matrix& conv, const unsigned maxiter) {
    unsigned rows = syms.rows();

    // Convert convergence criteria to numerics
    matrix criteria(rows,1);
    bool ignore_convergence = true; // If all criteria are exactly 0, then we want to run all the specified iterations
    for (unsigned row = 0; row < rows; ++row) {
      criteria(row,0) = conv(row,0);
      if (!criteria(row,0).is_zero())
        ignore_convergence = false;
    }

    // Iteration zero
    unsigned iter = 0;
    matrix var(rows, 1);
    for (unsigned row = 0; row < rows; ++row) {
      var(row,0) = expression(start(row,0)).evalf();
      MSG_INFO(2,  "s0 = " << var(row,0) << endline);
    }

    bool converged;
    iter++;

    do {
      matrix nextvar(rows, 1);
      // Collect substitutions
      exmap substvars; // Cannot use exhashmap because subs() doesn't accept it
      for (unsigned row = 0; row < rows; ++row)
        substvars[syms(row,0)] = var(row,0);

      for (unsigned row = 0; row < rows; ++row) {
        nextvar(row,0) = expression(exprs(row,0).subs(substvars)).evalf();
        MSG_INFO(2,  "s" << iter << " = " << nextvar(row,0) << endline);
      }

      if (!ignore_convergence) {
        converged = true;
        for (unsigned row = 0; row < rows; ++row) {
          ex diff = nextvar(row,0) - var(row,0);
          if (diff.info(info_flags::real)) {
            if (abs(ex_to<numeric>(diff)) > criteria(row,0)) {
              converged = false;
              break;
            }
          } else {
            converged = false;
            break;
          }
        }
      } else {
        converged = false;
      }

      var = std::move(nextvar);
      iter++;
    } while (!converged && (iter < maxiter));

    return var;
  }

  void eqc::clear() {
    if (msg::info().checkprio(1))
      for (const auto& i : remember_split)
        msg::info() << i.second.hits << " hits for " << i.second.e << endline;
    remember_split.clear();

    other_equations.clear();
    previous_it = equations.end();
    for (eqrec_it it_eqr = equations.begin(); it_eqr != equations.end(); ) {
      if (!is_lib(it_eqr->first)) {
        MSG_INFO(3,  "Deleting equation " << it_eqr->first
            << ": " << it_eqr->second.eq << endline);
        it_eqr= equations.erase(it_eqr);
      } else {
        ++it_eqr;
      }
    }
    expressions.clear();

    // Clear only variables and non-library functions
    assignments.clear();
    recent_assgn.clear();
    for (auto& v : vars) {
      if (v.second.getsymtype() == t_variable) {
        MSG_INFO(3,  "Deleting variable " << v.first << endline);
        v.second.make_unknown();
        v.second.setsymprop(p_complex);
        v.second.assignments.clear();
      } else if (v.second.getsymtype() == t_function) {
        if (!func::is_lib(v.first)) {
          MSG_INFO(3,  "Deleting function " << v.first << endline);
          func::remove(v.first);
          v.second.setsymtype(t_variable); // Keep this variable because it might have been shadowed by a function
          v.second.setsymprop(p_complex);
          v.second.make_unknown();
          v.second.assignments.clear();
        } else {
          MSG_INFO(3,  "Keeping " << v.first << endline);
          // A library function might have obtained a value through a normal equation (instead of through FUNCDEF)
          v.second.make_unknown();
          v.second.assignments.clear();
        }
      } else {
        MSG_INFO(3,  "Keeping " << v.first << endline);
        assignments.emplace(v.second.getsym(), v.second.val);
      }
    }
    func::clear();
  } //eqc::clear()

  void eqc::clearall (const bool persist_symbols) {
    MSG_INFO(3, "eqc::clearall, persisting symbols: " << (persist_symbols ? "yes" : "no") << endline);
    if (msg::info().checkprio(1))
      for (const auto& i : remember_split)
        msg::info() << i.second.hits << " hits for " << i.second.e << endline;
    remember_split.clear();

    other_equations.clear();
    equations.clear();
    expressions.clear();
    previous_it = equations.end();
    assignments.clear();
    recent_assgn.clear();
    if (persist_symbols) {
      for (auto& v : vars) {
        v.second.setsymtype(t_variable);
        v.second.setsymprop(p_complex);
        v.second.make_unknown();
        v.second.assignments.clear();
        MSG_INFO(3,  "Persisting symbol " << v.first << endline);
      }
    } else {
      vars.clear();
    }
    unitmgr.clear();
    func::clearall();
    nextlabel = 0;
  } //eqc::clearall()

  void eqc::print(std::ostream &os) const {
    os << "List of variables:" << std::endl;
    for (auto& variable : vars)
      os << variable.first << std::endl;

    os << "List of equations:" << std::endl;
    for (auto& eq : equations)
      os << eq.first << ": " << ex(eq.second.eq) << std::endl;

    os << "List of expressions:" << std::endl;
    for (auto& ex : expressions)
      os << ex.first << ": " << ex.second << std::endl;

    unitmgr.print(os);
  }

  void eqc::dumpvars(std::ostream & os) {
    std::vector<symrec> v_values;
    std::vector<symrec> v_novalues;

    // Collect variables, distinguishing whether they have a value or not
    for (const auto& v : vars) {
      if (is_internal(v.first)) continue;
      if ((v.first != VALSYM) && (v.second.getsymtype() != t_function) && (v.second.getsymtype() != t_none) && !is_internal(v.first)) {
        if (v.second.has_value())
          v_values.emplace_back(v.second);
        else
          v_novalues.emplace_back(v.second);
      }
    }

    // Dump variables with values first
    for (const auto& v : v_values) {
      os << v.get_name() << " = " << v.val;

      if (!v.assignments.empty()) {
        os << " (";
        for (const auto& l : v.assignments)
          os << "@" << l->label << "@ ";
        os << ")";
      }

      os << std::endl;
    }

    for (const auto& v : v_novalues)
      os << v.get_name() << " = (?)" << std::endl;
  }

  const expression &eqc::at(const std::string &label) const {
    if (label == "prev") {
      if (previous_it != equations.end())
        return previous_it->second.eq;
      else
        throw std::runtime_error("Currently no previous equation exists.");
    }

    eqrec_cit result = equations.find(label);
    if (result != equations.end()) return result->second.eq;
    throw std::range_error("equation label " + label + " does not exist.");
  }

  const expression &eqc::expression_at(const std::string &label) const {
    const auto& result = expressions.find(label);
    if (result != expressions.end()) return result->second;
    throw std::range_error("expression label " + label + " does not exist.");
  }

  // TODO: Implement this without using previous_it, and get rid of previous_it completely
  std::string eqc::getPreviousEquationLabel(const unsigned n) const {
    if (previous_it == equations.end()) return "";

    std::string label = previous_it->second.label; // This is already @prev1@
    for (unsigned i = 1; i < n; ++i) {
      eqrec_cit next = equations.find(label);
      if (next == equations.end()) return "";
      label = next->second.prevlabel;
    }
    return label;
  }

  bool eqrec::is_automatic() const {
    return ((label == "") || ((label[0] == '{') && (label[label.size() - 1] == '}')));
  }

  eqrec::eqrec(const expression &e, const std::string& l, const std::string& pl) : eq(e), label(l), prevlabel(pl) {
    MSG_INFO(3, "Constructing eqrec from " << e  << endline);
  }

#ifdef DEBUG_CONSTR_DESTR
  eqrec::eqrec() : eq(dynallocate<equation>()) {
    MSG_INFO(3, "Constructing empty eqrec"  << endline);
  }
  eqrec::eqrec(const eqrec& other) : eq(other.eq), subsed_lhs(other.subsed_lhs), subsed_rhs(other.subsed_rhs), label(other.label), prevlabel(other.prevlabel) {
    MSG_INFO(3, "Copying eqrec from " << other.label  << endline);
  }
  eqrec& eqrec::operator=(const eqrec& other) {
    MSG_INFO(3, "Assigning eqrec from " << other.label  << endline);
    eq = other.eq;
    subsed_lhs = other.subsed_lhs;
    subsed_rhs = other.subsed_rhs;
    label = other.label;
    prevlabel = other.prevlabel;
    return *this;
  }
  eqrec::~eqrec() {
    MSG_INFO(3, "Destructing eqrec for " << label  << endline);
  }
#endif
