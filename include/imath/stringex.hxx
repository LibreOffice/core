/***************************************************************************
    stringex.hxx  -  Header file for class stringex, an extension class to GiNaC
                             -------------------
    begin                : Fri Apr 06 2012
    copyright            : (C) 2012 by Jan Rheinlaender
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

#ifndef STRINGEX_H
#define STRINGEX_H

/**
 * @author Jan Rheinlaender
 * @short Extends GiNaC to handle string "expressions"
 */

#include <string>
#include <stdexcept>
#include "printing.hxx"

namespace GiNaC {

/// Extends GiNaC to handle string "expressions"
class stringex : public basic {
  GINAC_DECLARE_REGISTERED_CLASS(stringex, basic)

public:
  /**
   * Construct a stringex with the given value
   * @param s A string with the value of stringex
   */
  stringex(const std::string &s);

#ifdef DEBUG_CONSTR_DESTR
  ~stringex();
  stringex(const stringex& other);
  stringex& operator=(const stringex& other);
#endif

  /// Return the content of the stringex object
  inline const std::string& get_string() const { return value; }

  /**
   * Print the stringex in a GiNaC print context.
   * @param c The print context (e.g., print_latex)
   * @param level Unused, for consistency with GiNaC print methods
   */
  void do_print(const print_context &c, unsigned level = 0) const;
  void do_print_imath(const imathprint &c, unsigned level = 0) const;

  using basic::normal; // Fixes weird linking error on LO internal build

private:
  /// The value of the stringex
  std::string value;
};

class stringex_unarchiver {
public:
  stringex_unarchiver();
  ~stringex_unarchiver();
};
static stringex_unarchiver stringex_unarchiver_instance;

}
#endif
