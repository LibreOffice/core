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

    /**
     * Check whether a cell should have it's data converted to another unit for
     * sensible unit verification.
     *
     * Returns true if the cell has a local unit annotation (e.g. contains "10cm",
     * but split into value (10) and unit format ('#"cm"')), but the column header
     * defines a different unit (e.g. "length [m]).
     * The data returned can then be used by convertCellToHeaderUnit if the user
     * desires conversion in this case.
     *
     * If returning false, the header and cell units will be empty and the address invalid.
     */
    virtual bool isCellConversionRecommended(const ScAddress& rCellAddress,
                                             ScDocument* pDoc,
                                             OUString& rHeaderUnit,
                                             ScAddress& rHeaderCellAddress,
                                             OUString& rCellUnit) = 0;

    virtual ~Units() {}
};

}} // namespace sc::units

#endif // INCLUDED_SC_INC_UNITS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

