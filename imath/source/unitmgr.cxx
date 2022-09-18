/*******************************************************
    unitmgr.cpp  -  Class for handling units for EQC
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

#ifdef INSIDE_SM
#include <imath/unitmgr.hxx>
#include <imath/msgdriver.hxx>
#include <imath/utils.hxx>
#else
#include "unitmgr.hxx"
#include "msgdriver.hxx"
#include "utils.hxx"
#endif

using namespace GiNaC;

Unitmanager::Unitmanager() {
  MSG_INFO(2, "Unitmanager: Initializing units" << endline);
  units = std::map <std::string, expression>();
  prefixes = std::map<std::string, numeric>();

  // base SI units
  // Note that °C must be a base unit because otherwise %theta = 20 %celsius will print as %theta = 20 %K !
  // The "unity" exists so that units like %radian can be printed by defining a custom unit %ra = %unity
  units.emplace(std::piecewise_construct, std::forward_as_tuple("metre"), std::forward_as_tuple(dynallocate<Unit>("metre", "m")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("kilogram"), std::forward_as_tuple(dynallocate<Unit>("kilogram", "kg")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("second"), std::forward_as_tuple(dynallocate<Unit>("second", "s")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("ampere"), std::forward_as_tuple(dynallocate<Unit>("ampere", "A")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("kelvin"), std::forward_as_tuple(dynallocate<Unit>("kelvin", "K")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("celsius"), std::forward_as_tuple(dynallocate<Unit>("celsius", "°C")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("mole"), std::forward_as_tuple(dynallocate<Unit>("mole", "mol")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("candela"), std::forward_as_tuple(dynallocate<Unit>("candela", "cd")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("unity"), std::forward_as_tuple(dynallocate<Unit>("unity", "unity")));
  units.emplace(std::piecewise_construct, std::forward_as_tuple("unknown"), std::forward_as_tuple(dynallocate<Unit>("unknown", "???")));
  MSG_INFO(2, "Unitmanager: Finished initializing units" << endline);
}

const expression& Unitmanager::getUnit(const std::string &uname) {
  std::map<std::string, expression>::iterator u = units.find(uname);
  if (u == units.end()) {
    MSG_ERROR(0, "Unitmanager: Unknown unit " << uname << endline);
    return units.at("unknown");
  } else {
    MSG_INFO(3, "Unitmanager: Found unit " << uname << endline);
    return u->second;
  }
}

expression Unitmanager::getCanonicalizedUnit(const std::string& uname) {
  std::map<std::string, expression>::iterator u = units.find(uname);
  if (u == units.end()) {
    MSG_ERROR(0, "Unitmanager: Cannot get canonicalized unit " << uname << endline);
    return units.at("unknown");
  } else {
    MSG_INFO(3, "Unitmanager: Found canonicalized unit " << uname << " = " << ex_to<Unit>(u->second).get_canonical() << endline);
    return ex_to<Unit>(u->second).get_canonical();
  }
}

bool Unitmanager::isUnknown(const expression& u) const {
  if (is_a<Unit>(u) && (ex_to<Unit>(u).get_name() == "unknown")) return true;
  return false;
}

void Unitmanager::addUnit(const std::string& uname, const std::string& pname, const expression &other_units) {
  if (units.find(uname) != units.end())
    MSG_WARN(0, "Warning: Overwriting unit definition of " << uname << endline);

  expression cex = canonicalize(other_units);
  units[uname] = expression(Unit(uname, pname, cex));

  if (!pname.empty() && (pname != uname))
    units[pname] = units[uname];

  MSG_INFO(2, "Added unit [" << uname << "]: '" << pname << "' = " << cex << endline);
}

void Unitmanager::addPrefix(const std::string &prefixname, const numeric &pvalue) {
  if (prefixes.find(prefixname) != prefixes.end())
    MSG_WARN(0, "Warning: Overwriting prefix definition of " << prefixname << endline);

  prefixes[prefixname] = pvalue;
  MSG_INFO(2, "Added prefix '" << prefixname << "' = " << pvalue << endline);
}

bool Unitmanager::isUnit(const std::string &uname) {
  MSG_INFO(2, "Checking whether " << uname << " is a unit: ");

  if (units.find(uname) != units.end()) {
    MSG_INFO(2, "Yes." << endline);
    return true;
  }
  if (prefixes.empty()) {
    MSG_INFO(2, "No." << endline);
    return false;
  }

  // Check for prefix+unit
  for (auto& p : prefixes) {
    if (uname.compare(0, p.first.length(), p.first) == 0) {
      if (units.find(uname.substr(p.first.length())) != units.end()) {
        MSG_INFO(2, "Yes (with prefix " << p.first << ")" << endline);
        const Unit& u = ex_to<Unit>(units.at(uname.substr(p.first.length())));
        addUnit(uname, p.first + u.get_printname(), p.second * u.get_canonical());
        MSG_INFO(2, "Added unit [" << uname << "]: '" << p.first + u.get_printname() << "' = " << p.second * u.get_canonical() << endline);
        return true;
      }
    }
  }

  MSG_INFO(2, "No." << endline);
  return false;
}

std::vector<std::string> Unitmanager::getUnitnames() const {
  std::vector<std::string> result;

  for (auto &u : units)
    result.emplace_back(u.first);

  std::sort(result.begin(), result.end());
  return result;
}

// Replace all units in an expression by their canonical representation
ex Unitmanager::canonicalize_ex::operator()(const ex &e) {
  if (is_a<Unit>(e)) {
    return ex_to<Unit>(e).get_canonical();
  }

  // The exception for constants exists to allow for things like Pi. Of course, it is
  // impossible to avoid allowing other constants that should not occur in a unit!
  if (!(is_a<numeric>(e) || is_a<constant>(e) || is_a<mul>(e) || is_a<power>(e)))
    MSG_WARN(0, "Warning: Cannot canonicalize " << e << "!" << endline);

  return e.map(*this);
}

expression Unitmanager::canonicalize(const expression &e) const {
  // This method only works on expressions which are units or a multiplication of units
  MSG_INFO(3, "Canonicalizing " << e << endline);
  expression u = e.expand(); // added in 0.5 to expand functions in unit definitions

  canonicalize_ex c_ex;
  expression result = c_ex(u);
  MSG_INFO(2, "Result of canonicalization: " << result << endline);

  return result;
}

unitvec *Unitmanager::create_conversions2(const lst &e, const bool always) {
  unitvec *result = new unitvec;
  unitvec convs;

  for (const auto& u : e) {
    MSG_INFO(2, "Creating conversion for " << u << endline);
    expression pref_unit = u;

    // Replace all units in the expression by their canonical representation
    canonicalize_ex c_ex;
    expression conversion = c_ex(u);
    MSG_INFO(2, "Result of canonicalization: " << conversion << endline);

    if (conversion.is_equal(_expr1)) { // subst_units would crash with this conversion
      MSG_WARN(0, "Warning: No conversion exists for " << u << ". Ignoring" << endline);
      continue;
    }

    if (!result->empty()) conversion = conversion.csubs(convs);

    // Any numerics on the left hand side of the relation need to be moved to the
    // right hand side because 3\mm will not match "1000\mm == \m"
    if (is_a<mul>(conversion)) {
      // Other cases are either impossible or need not be handled
      // Note: GiNaC stores the overall numeric multiplier in the last sub-expression of the mul
      pref_unit = pref_unit / expression(*(conversion.end() - 1));
      ex res(_ex1);
      for (const_iterator o = conversion.begin(); o != conversion.end() - 1; ++o)
        res = res * *o;
      conversion = res; // This removes the numeric multiplier
    }

    // May happen if the user specifies the same preferred unit twice
    if (conversion.is_equal(pref_unit))
      continue;

    if (always || (conversion != pref_unit)) {
      MSG_INFO(2, "Adding " << conversion << "==" << pref_unit << " to list of preferred units." << endline);
      result->emplace_back(conversion == pref_unit);
      convs.emplace_back(conversion == pref_unit);
    }
  }

  return result;
}

void Unitmanager::clear() {
  MSG_INFO(3, "Clearing all units except base units" << endline);
  for (auto u = units.begin(); u != units.end();) {
    if (!ex_to<Unit>(u->second).is_base())
      u = units.erase(u);
    else
      ++u;
  }
  prefixes.clear();
}

void Unitmanager::print(std::ostream &os) const {
  os << "List of unit conversions:" << std::endl;
  for (auto &u : units)
    os << u.first << ": " << u.second << std::endl;
  os << std::endl;
}
