/*******************************************************
    differential.cpp  -  Class for extending GiNaC to handle differential objects
                             -------------------
    begin                : Sat 22 Feb 2014
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

#include "differential.hxx"
#include "printing.hxx"
#include "msgdriver.hxx"
#include "utils.hxx"
#include "func.hxx"
#include <cmath>

namespace GiNaC {
  extern const ex _ex10;
  extern const ex _ex20;

GINAC_IMPLEMENT_REGISTERED_CLASS_OPT(differential, basic,
  print_func<print_context>(&differential::do_print).
  print_func<imathprint>(&differential::do_print_imath));

// Required constructors and destructors and other GiNaC-specific methods
  differential::differential() : e(_ex0), partial(false), grade(_ex1), parent(_ex0), numerator(false) {
    MSG_INFO(3, "Constructing empty differential" << endline);
  }

  differential::differential(const differential& other)
    : basic(other), e(other.e), partial(other.partial), grade(other.grade), parent(other.parent), numerator(other.numerator) {
    MSG_INFO(4, "Copying differential from " << other.e  << endline);
  }

  differential::differential(const ex& e_, const bool partial_, const ex& grade_, const ex& parent_, const bool numerator_)
    : e(e_), partial(partial_), grade(grade_), parent(parent_), numerator(numerator_)
  {
    MSG_INFO(3, "Constructing differential from " << e  << endline);
  }

  differential& differential::operator=(const differential& other) {
    MSG_INFO(3, "Assigning differential from " << other.e  << endline);
    e = other.e;
    partial = other.partial;
    grade = other.grade;
    parent = other.parent;
    numerator = other.numerator;
    return *this;
  }

#ifdef DEBUG_CONSTR_DESTR
  differential::~differential() {
    MSG_INFO(3, "Destructing differential for " << e << endline);
  }
#endif

  differential_unarchiver::differential_unarchiver() {}
  differential_unarchiver::~differential_unarchiver() {}

  int differential::compare_same_type(const basic &other) const {
    MSG_INFO(2, "compare_same_type " << *this << " with " << ex(other) << endline);
    const differential &o = static_cast<const differential &>(other);
    if (partial != o.partial)
      return partial ? -1 : 1;
    if (numerator != o.numerator)
      return numerator ? -1 : 1;
    int compval = grade.compare(o.grade);
    if (compval == 0)
      return e.compare(o.e);
    else
      return compval;
    // Note: We cannot include parent in this comparison, otherwise differentiate(f,x,1) * differential(x) will not cancel the dx's
  }

  unsigned differential::calchash() const {
    const static unsigned hash00 = _ex0.gethash();
    const static unsigned hash01 = _ex1.gethash();
    const static unsigned hash10 = _ex10.gethash();
    const static unsigned hash20 = _ex20.gethash();

    unsigned v = make_hash_seed(typeid(*this));
    v = rotate_left(v);
    v ^= (partial ? hash01 : hash00);
    v = rotate_left(v);
    v ^= (numerator ? hash20 : hash10);
    v = rotate_left(v);
    v ^= e.gethash();
    v = rotate_left(v);
    v ^= grade.gethash();
    // Note: We cannot include parent in the hash, otherwise differentiate(f,x,1) * differential(x) will not cancel the dx's

    // store calculated hash value only if object is already evaluated
    if (flags & status_flags::evaluated) {
      MSG_INFO(4, "Saved hash for " << *this << ": " << v << endline);
      setflag(status_flags::hash_calculated);
      hashvalue = v;
    } else {
      MSG_INFO(4, "Calculated hash for " << *this << ": " << v << endline);
    }

    return v;
  }

  void differential::do_print(const print_context &c, unsigned level) const {
    //MSG_INFO(3) << "differential::do_print" << endline;
    (void)level;
    c.s << (partial ? "partial(" : "d(") << e;
    if (!grade.is_equal(_ex1)) c.s << ", " << grade;
    if (!parent.is_zero()) c.s << ", '" << parent << "'";
    c.s << (numerator ? ",N" : ",D");
    c.s << ")";
  }

  void differential::do_print_imath(const imathprint &c, unsigned level) const {
    do_print_imath(c, level, true, false);
  }

  void differential::do_print_imath(const imathprint &c, unsigned level, const bool is_complete, const ex& pdiffto) const {
    (void)level;
    int gr = get_ngrade();

    std::string difftype = "dfdt";
    if ((c.poptions->find(o_difftype) != c.poptions->end()) && ((*c.poptions)[o_difftype].value.str != NULL)) difftype = *(*c.poptions)[o_difftype].value.str;
    if ((partial && (difftype == "dot")) || !is_complete) difftype = "dfdt";

    std::string lbracket = "(";
    std::string rbracket = ")";
    if (is_a<symbol>(e) || (is_a<func>(e) && ex_to<func>(e).is_pure() && !ex_to<func>(e).is_lib())) {
      lbracket = "";
      rbracket = "";
    } else if (is_a<func>(e)) {
      // ensure there is a bracket, important for proper formatting e.g. of d{abs{x}}
      lbracket = "{";
      rbracket = "}";
    }

    if (difftype == "dot") {
        if (is_a<func>(e) && ex_to<func>(e).get_numargs() != 1) MSG_WARN(0, "Warning: Diff type 'dot' makes no sense with " << e << endline);
        if (!grade.info(info_flags::posint)) throw std::logic_error("Error: Diff type 'dot' is not implemented for non-positive integer diff levels");
        if (gr > 2) throw std::logic_error("Error: Diff type 'dot' is not implemented for diff levels other than 1 or 2");

        c.s << ((gr == 1) ? " dot " : " ddot ");
        if (is_a<func>(e) && !ex_to<func>(e).is_pure()) {
          // putting a function with arguments into brackets would show the dot centered above everything
          e.print(c);
        } else {
          c.s << lbracket;
          e.print(c);
          c.s << rbracket;
        }
    } else if (difftype == "line") {
        if (is_a<func>(e) && ex_to<func>(e).get_numargs() != 1) MSG_WARN(0, "Warning: Diff type 'line' makes no sense with " << e << endline);

        c.s << lbracket;

        if (is_a<func>(e) && !ex_to<func>(e).is_nobracket()) {
          ex_to<func>(e).print_diff_line(grade, gr, c);

          if (partial && !pdiffto.is_zero()) {
            c.s << "_{";
            pdiffto.print(c);
            c.s << "}";
          }
        } else {
          e.print(c);
        }

        c.s << rbracket;

        if (!is_a<func>(e) || ex_to<func>(e).is_nobracket()) {
          if ((gr > 0) && (gr < 4)) {
            c.s << "^{";
            for (int i = 0; i < gr; i++) c.s << "%d1";
            c.s << "}";
          } else {
            c.s << "^(" << grade << ")";
          }

          if (partial && !pdiffto.is_zero()) {
            c.s << "_{";
            pdiffto.print(c);
            c.s << "}";
          }
        }
    } else {
        // Everything else is printed in 'dfdt' style
        std::string d_sign;
        if (partial)
          d_sign = "partial";
        else
          d_sign = "d"; // TODO: German standard is an upright "d" but that gives too much space behind. So we stay with American italic "d" for now

        if (lbracket == "")
          c.s << "nospace{";
        c.s << d_sign;
        if (!(gr == 1) && numerator)
          c.s << "^{" << (gr > 0 ? gr : grade) << "}";
        else if (partial)
          c.s << " ";
        c.s << lbracket;
        e.print(c);
        c.s << rbracket;
        if (!(gr == 1) && !numerator) c.s << "^{" << (gr > 0 ? gr : grade) << "}";
        if (lbracket == "")
          c.s << "}";
    }
  }

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex differential::eval() const {
    MSG_INFO(4, "Eval of differential" << endline);
    if (flags & status_flags::evaluated)
      return *this;

    if (is_a<differential>(e) && (partial == ex_to<differential>(e).is_partial())) {
      // Handle differentials of differentials by adding the grades
      const differential& ed = ex_to<differential>(e);
      return dynallocate<differential>(ed.e, partial, grade + ed.grade, ed.parent, ed.numerator).setflag(status_flags::evaluated);
    }

    return this->hold();
  }
#else
  ex differential::eval(int level) const {
    if ((level==1) && (flags & status_flags::evaluated))
      return *this;

    if (level == -max_recursion_level)
      throw(std::runtime_error("max recursion level reached"));

    ex ee = (level==1) ? e : e.eval(level-1);
    ex gg = (level==1) ? grade : grade.eval(level-1);

    if (is_a<differential>(ee) && (partial == ex_to<differential>(ee).is_partial())) {
      // Handle differentials of differentials by adding the grades
      const differential& eed = ex_to<differential>(ee);
      return (new differential(eed.e, partial, gg + eed.grade, eed.parent, eed.numerator))->setflag(status_flags::dynallocated | status_flags::evaluated);
    }

    if (are_ex_trivially_equal(ee,e) && are_ex_trivially_equal(gg, grade))
      return this->hold();

   return (new differential(ee, partial, gg, parent, numerator))->setflag(status_flags::dynallocated | status_flags::evaluated);
  }
#endif

#if (((GINACLIB_MAJOR_VERSION == 1) && (GINACLIB_MINOR_VERSION >= 7)) || (GINACLIB_MAJOR_VERSION >= 1))
  ex differential::evalf() const {
    ex ee = expression(e).evalf();
    ex gg;
    if (grade.info(info_flags::integer))
      gg = grade; // Avoid things like d^{1.0}x
    else
      gg = expression(grade).evalf();

    if (is_a<numeric>(ee))
      return _ex0;

    if (are_ex_trivially_equal(ee,e) && are_ex_trivially_equal(gg, grade))
      return *this;

    return dynallocate<differential>(ee, partial, gg, parent, numerator);
  }
#else
  ex differential::evalf(int level) const {
    ex ee;
    ex gg;

    if (level==1) {
      ee = e;
      gg = grade;
    } else if (level == -max_recursion_level) {
      throw(std::runtime_error("max recursion level reached"));
    } else {
      ee = expression(e).evalf(level-1);
      if (grade.info(info_flags::integer))
        gg = grade; // Avoid things like d^{1.0}x
      else
        gg = expression(grade).evalf(level-1);
    }

    if (is_a<numeric>(ee))
      return _ex0;

    if (are_ex_trivially_equal(ee,e) && are_ex_trivially_equal(gg, grade))
      return *this;

    return (new differential(ee, partial, gg, parent, numerator))->setflag(status_flags::dynallocated);
  }
#endif

  ex differential::evalm() const {
    ex ee = expression(e).evalm();
    ex gg = expression(grade).evalm();

    if (is_a<numeric>(ee))
      return _ex0;

    if (are_ex_trivially_equal(ee,e) && are_ex_trivially_equal(gg, grade))
      return *this;

    return dynallocate<differential>(ee, partial, gg, parent, numerator);
  }

  size_t differential::nops() const
  {
          return 2;
  }

  ex differential::op(size_t i) const
  {
    if (i == 0) return e;
    if (i == 1) return grade;
    throw (std::out_of_range("differential::op() out of range"));
  }

  ex differential::map(map_function & f) const {
    const ex &mapped_e = f(e);
    const ex &mapped_g = f(grade);

    if (are_ex_trivially_equal(e, mapped_e) && are_ex_trivially_equal(grade, mapped_g))
      return *this;

    return dynallocate<differential>(mapped_e, partial, mapped_g, parent, numerator);
  }

  ex differential::expand(unsigned options) const {
    if (options==0 && (flags & status_flags::expanded))
      return *this;

    ex ee = e.expand(options);
    ex gg = grade.expand(options);

    if (are_ex_trivially_equal(ee,e) && are_ex_trivially_equal(gg,grade)) {
      if (options==0)
        this->setflag(status_flags::expanded);
      return *this;
    }

    const basic& newdiff = (new differential(ee, partial, gg, parent, numerator))->setflag(status_flags::dynallocated);
    if (options == 0)
            newdiff.setflag(status_flags::expanded);
    return newdiff;
  }

  ex differential::subs(const exmap & m, unsigned options) const {
    MSG_INFO(2, "Substituting exmap " << m << " in " << *this << endline);
    const ex & subsed_e = e.subs(m, options);
    const ex & subsed_g = grade.subs(m, options);
    const ex & subsed_p = parent.subs(m, options);

    if (are_ex_trivially_equal(e, subsed_e) && are_ex_trivially_equal(grade, subsed_g) && are_ex_trivially_equal(parent, subsed_p))
      return subs_one_level(m, options);

   return differential(subsed_e, partial, subsed_g, subsed_p, numerator).subs_one_level(m, options);
  }

  bool differential::match_same_type(const basic & other) const {
   const differential &o = static_cast<const differential &>(other);

   return (partial == o.partial) && (numerator == o.numerator) && (grade.compare(o.grade) == 0);
  }

  bool differential::has(const ex & other, unsigned options) const {
    if (!(options & has_options::algebraic))
            return basic::has(other, options);
    if (!is_a<differential>(other))
            return basic::has(other, options);
    const differential &o = static_cast<const differential &>(other);
    if (partial != o.partial)
      return basic::has(other, options);
    if (numerator != o.numerator)
      return basic::has(other, options);
    if (grade != o.grade)
      return basic::has(other, options);
    if (e.match(o.argument()))
      return true;

    return basic::has(other, options);
}

  ex differential::derivative(const symbol & s) const {
    MSG_INFO(3, "Derivative of " << *this << " to " << s << endline);
    if (is_a<symbol>(e))
      // Derivative of a differential of a symbol is always zero
      return 0;

    // Ideally, this should never happen, since single differentials cannot be differentiated properly
    // Mainly, it is unclear what variable the denominator has for partial differentials
    // TODO: The current implementation does not take into account the chain rule
    return differential(e, partial, grade+1, parent, true) / differential(s, partial, 1, e); // Increase the grade of the differential
  }

int differential::get_ngrade() const {
  if (grade.info(info_flags::posint))
    return ex_to<numeric>(grade).to_int();

  return -1;
}
}
