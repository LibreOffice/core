/***************************************************************************
    unitmgr.hxx  -  Header file for class unit manager
                             -------------------
    begin                : Wed Oct 26 2016
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

#ifndef UNITMGR_H
#define UNITMGR_H

/**
 * @author Jan Rheinlaender
 * @short Manages units for EQC
 */

#include <string>
#include "expression.hxx"
#include "unit.hxx"

/// Manage units for EQC
class Unitmanager {
public:
  /// Initialize the Unitmanager
  Unitmanager();
  /**
   * Return the unit for the given unit name. This unit is unique and the same unit is returned every time
   * @param uname A string with the name of the unit that is to be looked up
   * @returns An expression containing the unit
   */
  const GiNaC::expression& getUnit(const std::string &uname);

  /**
   * Returns the unit expressed in base units
   * Issues an error if there is no entry for this unit name in the conversion table, and returns a (base)
   * unit with the name "unknown".
   * @param uname A string with the name of the unit that is to be looked up
   * @returns An expression with the units corresponding to the name given
   */
  GiNaC::expression getCanonicalizedUnit(const std::string& uname);

  /// Checks if this is an unknown unit (as might be returned by getUnit())
  bool isUnknown(const GiNaC::expression& u) const;

  /**
   * Add a new unit. The unit with the name "uname" can be expressed
   * by the expression "other_units". This method issues a warning if a unit with this name
   * already exists, and overwrites it with the new one.
   * @param uname The name of the unit to be added
   * @param pname The printname of the unit to be added
   * @param other_units An expression with the units which are equivalent to "uname"
   */
  void addUnit(const std::string &uname, const std::string& pname, const GiNaC::expression &other_units);

  /**
   * Add a new prefix (e.g. "milli") which should be used in parsing unit names (e.g. millimetre will be automatically recognized)
   * @param prefixname The name of the prefix
   * @param pvalue The numeric value of the prefix
   */
  void addPrefix(const std::string &prefixname, const GiNaC::numeric &pvalue);

  /**
   * Returns true if the unit was previously registered using addUnit, or is a base unit
   * If prefixes exist and the unit is a combination of prefix and a known unit, a new unit
   * is created, corresponding to an addUnit() call.
   * @param uname The unit name
   * @returns True if the unit is found
   */
  bool isUnit(const std::string &uname);

  /**
   * Takes an expression and turns all units in it to base units. The expression must be a unit or a mul of
   * units and their powers. Any numerics found are left untouched. Symbols are accepted as well, to
   * allow for constants like Pi. If any other GiNaC types are found in the expression, the method issues
   * a warning and leaves them untouched.
   * @param e The expression that is to be canonicalized
   * @returns An expression containing only base units, numerics and symbols
   */
  GiNaC::expression canonicalize(const GiNaC::expression &e) const;

  /**
   * Create the conversion list from a list of preferred units
   * Auxiliary method for EQC
   * @param e A vector of the preferred units
   * @param always No conversion is is necessary for base units like "metre", but someone might want to override a global setting
   * @returns A vector of relationals containing the necessary conversions
   */
  /// This version allows expressions as the LHS of the argument
  GiNaC::unitvec* create_conversions2(const GiNaC::lst& e, const bool always = false);

  // Return a map of names of all units currently defined
  std::vector<std::string> getUnitnames() const;

  /**
   * Return the unit conversion table and the list of known unit names to a virgin state (same as after constructing a new Unitmanager)
   */
  void clear();

  /// Print unit information onto a stream
  void print(std::ostream &os) const;

private:
  /// A map of the unitnames -> Units to consistently return the same object for a unitname
  std::map<std::string, GiNaC::expression> units;

  /// A set of prefixes that could be encountered when parsing units, e.g. millimetre
  // Note: Order of prefixes is important when comparing, e.g. milli must come before m otherwise millimetre is parsed as m * illimetre not milli * metre
  std::map<std::string, GiNaC::numeric> prefixes;

  /// Map function to replace all units in an expression by their canonical form
  struct canonicalize_ex : public GiNaC::map_function {
    GiNaC::ex operator()(const GiNaC::ex &e);
  };
};
#endif
