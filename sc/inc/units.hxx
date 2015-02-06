/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_INC_UNITS_HXX
#define INCLUDED_SC_INC_UNITS_HXX

#include <rtl/ustring.hxx>

#include <boost/shared_ptr.hpp>

class ScAddress;
class ScDocument;
class ScTokenArray;

namespace sc {
namespace units {

class UnitsImpl;

class Units {
public:
    static ::boost::shared_ptr< Units > GetUnits();

    virtual bool verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) = 0;

    /*
     * Split the input into value and unit, where rInput == rValue + rUnit.
     * (We assume that the unit is always the last part of the input string.)
     *
     * Returns whether or not the string has been split.
     * rValue and rUnit are always set to valid values, irrespective of string
     * splitting having actually taken place.
     */
    virtual bool splitUnitsFromInputString(const OUString& rInput, OUString& rValue, OUString& rUnit) = 0;

    virtual ~Units() {}
};

}} // namespace sc::units

#endif // INCLUDED_SC_INC_UNITS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

