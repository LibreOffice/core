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

#include "rangelst.hxx"

class ScAddress;
class ScDocument;
class ScRange;
class ScTokenArray;

namespace sc {
namespace units {

class UnitsImpl;

/**
 * The units used for a range of data cells.
 */
struct RangeUnits {
    std::vector< OUString > units;
    /**
     * Whether all the units in the list are compatible (i.e. data
     * can be converted to any of the listed units).
     */
    bool compatible;
};

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

    /**
     * Convert a cell with local unit annotation to store data in the units represented by
     * its (column-level) header.
     *
     * This method should only be used with the data provided by isCellConversionRecommended.
     * It will remove the cell's local unit annotation, and not add any of it's own annotation.
     * You should use the (yet to be implemented) convertCells() method for more generic
     * unit conversion.
     *
     * Returns true for successful conversion, false if an error occured (e.g. if the data
     * has been modified in the meantime, i.e. that the units no longer correspond
     * to the expected annotation or header).
     */
    virtual bool convertCellToHeaderUnit(const ScAddress& rCellAddress,
                                         ScDocument* pDoc,
                                         const OUString& rsNewUnit,
                                         const OUString& rsOldUnit) = 0;

    /**
     * Convert cells from one unit to another.
     *
     * If possible the input unit will be determined automatically (using local
     * and header units).
     *
     * Returns false if not input units are not compatible with the desired output units,
     * however this method still converts all cells containing compatible units.
     *
     * Local and header unit annotations are modified as appropriate such that the output
     * remains unambiguous. Hence, if the header cell is included in rRange, its unit
     * annotation is also updated as appropriate. If instead the header is excluded,
     * but all other cells are selected in a column, then local annotations are added.
     *
     * rsInputUnit overrides the automatic determination of input units, i.e. disables
     * input unit detection.
     */
    virtual bool convertCellUnits(const ScRangeList& rRanges,
                                  ScDocument* pDoc,
                                  const OUString& rsOutputUnit) = 0;

    virtual bool areUnitsCompatible(const OUString& rsUnit1,
                                    const OUString& rsUnit2) = 0;

    virtual RangeUnits getUnitsForRange(const ScRangeList& rRangeList,
                                        ScDocument* pDoc) = 0;

    virtual ~Units() {}
};

}} // namespace sc::units

#endif // INCLUDED_SC_INC_UNITS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

