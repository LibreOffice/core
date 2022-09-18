/***************************************************************************
    unit.hxx  -  Header file for class unit, an extension class to GiNaC
                             -------------------
    begin                : Mon Oct 22 2001
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

#ifndef UNIT_H
#define UNIT_H

/**
 * @author Jan Rheinlaender
 * @short Extends GiNaC to handle physical units, i.g., "x = 1 mm", "1 m + 1 mm = 1,001 m"
 */

#include <string>
#include <ginac/ginac.h>
#include "expression.hxx"
#include "printing.hxx"

namespace GiNaC {

extern const expression _expr0;

/// Extends GiNaC to handle physical units
class Unit : public basic {
  GINAC_DECLARE_REGISTERED_CLASS(Unit, basic)

public:
  /**
   * Construct a Unit with the given name.
   * @param n A string with the name of the unit
   * @param pn A string with the print name of the unit
   * @param cex An expression with the canonical form of the unit, expressed in the SI base units. If this is zero, the unit is assumed to be a base unit
   */
  Unit(const std::string& n, const std::string& pn, const expression& cex = _expr0);

#ifdef DEBUG_CONSTR_DESTR
  ~Unit();
  Unit(const Unit& other);
  Unit& operator=(const Unit& other);
#endif

  /**
   * Print the unit in a GiNaC print context.
   * @param c The print context (e.g., print_latex)
   * @param level Unused, for consistency with GiNaC print methods
   */
  void do_print(const print_context &c, unsigned level = 0) const;
  void do_print_imath(const imathprint &c, unsigned level) const;

  /**
   * Return the name of the unit
   * @returns A string with the name of the unit
   */
  inline std::string get_name() const { return(name); }

  /// Return printname of the unit
  inline std::string get_printname() const { return(printname); }

  /// Return the printname, and if that is not set, the unit name
  inline std::string get_unitname() const { return printname.empty() ? name : printname; }

  /// Return the canonical form of the unit
  expression get_canonical() const;

  bool is_base() const;

  // Return information
  bool info(unsigned inf) const override;

  using basic::normal; // Fixes weird linking error on LO internal build

private:
  /// The name of the unit
  std::string name;

  /// The name to be used for printing
  std::string printname;

  /// The canonical form of the unit, expressed in the base units
  expression canonical;
};

class Unit_unarchiver {
public:
  Unit_unarchiver();
  ~Unit_unarchiver();
};
static Unit_unarchiver Unit_unarchiver_instance;

}
#endif
