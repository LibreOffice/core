/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_SOURCE_CORE_UNITS_UNITSIMPL_HXX
#define INCLUDED_SC_SOURCE_CORE_UNITS_UNITSIMPL_HXX

#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <formula/opcode.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <udunits2/udunits2.h>

#include "rangelst.hxx"
#include "units.hxx"

#include "raustack.hxx"
#include "utunit.hxx"

#include <stack>

namespace formula {
    class FormulaToken;
}

struct ut_system;

namespace sc {
namespace units {

namespace test {
    class UnitsTest;
}

/**
 * The result for a given units operation.
 * If UNITS_VALID then the resulting unit is also included,
 * otherwise units is empty.
 */
struct UnitsResult {
    FormulaStatus status;
    boost::optional<UtUnit> units;
};

struct HeaderUnitDescriptor {
    bool valid;
    UtUnit unit;
    boost::optional< ScAddress > address;
    // This must be the unit string copied verbatim from the header
    // (i.e. including spaces)
    OUString unitString;
    // Position of unitString within the cell contents
    sal_Int32 unitStringPosition;
};

class UnitsImpl: public Units {
    friend class test::UnitsTest;

private:
    static ::osl::Mutex ourSingletonMutex;
    static ::boost::weak_ptr< UnitsImpl > ourUnits;

    // A scoped_ptr would be more appropriate, however
    // we require a custom deleter which scoped_ptr doesn't
    // offer.
    ::boost::shared_ptr< ut_system > mpUnitSystem;

    static void freeUtSystem(ut_system* pSystem) {
        ut_free_system(pSystem);
    }

public:
    static ::boost::shared_ptr< UnitsImpl > GetUnits();
    ::boost::shared_ptr< ut_system > GetUnitSystem() const { return mpUnitSystem; }

    UnitsImpl();
    virtual ~UnitsImpl();

    virtual FormulaStatus verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) SAL_OVERRIDE;
    virtual bool splitUnitsFromInputString(const OUString& rInput, OUString& rValue, OUString& rUnit) SAL_OVERRIDE;
    virtual bool isCellConversionRecommended(const ScAddress& rCellAddress,
                                             ScDocument* pDoc,
                                             OUString& rHeaderUnit,
                                             ScAddress& rHeaderCellAddress,
                                             OUString& rCellUnit) SAL_OVERRIDE;
    virtual bool convertCellToHeaderUnit(const ScAddress& rCellAddress,
                                         ScDocument* pDoc,
                                         const OUString& rsNewUnit,
                                         const OUString& rsOldUnit) SAL_OVERRIDE;

    virtual bool convertCellUnits(const ScRangeList& rRanges,
                                  ScDocument* pDoc,
                                  const OUString& rsOutputUnit) SAL_OVERRIDE;

    virtual bool areUnitsCompatible(const OUString& rsUnit1,
                                    const OUString& rsUnit2) SAL_OVERRIDE;

    virtual RangeUnits getUnitsForRange(const ScRangeList& rRangeList,
                                        ScDocument* pDoc) SAL_OVERRIDE;

    virtual bool isValidUnit(const OUString& rsUnit) SAL_OVERRIDE;
    /**
     * Retrieve the units for a given cell. This probes based on the usual rules
     * for cell annotation/column header.
     * Retrieving units for a formula cell is not yet supported.
     *
     * Units are undefined for any text cell (including header cells).
     */
    UtUnit getUnitForCell(const ScAddress& rCellAddress, ScDocument* pDoc);

private:
    UnitsResult getOutputUnitsForOpCode(std::stack< RAUSItem >& rStack, const formula::FormulaToken* pToken, ScDocument* pDoc);

    /**
     * Find and extract a Unit in the standard header notation,
     * i.e. a unit enclose within square brackets (e.g. "length [cm]".
     *
     * @return The HeaderUnitDescriptor, with valid set to true if a unit was found.
     */
    HeaderUnitDescriptor findUnitInStandardHeader(const OUString& rHeader);
    /**
     * Find and extract a freestanding Unit from a header string.
     * This includes strings such as "speed m/s", "speed m / s",
     * "speed (m/s)" etc.
     * This is (for now) only a fallback for the case that the user hasn't defined
     * units in the standard notation, and can only handle a limited number
     * of ways in which units could be written in a string.
     * It may turn out that in practice this method should be extended to support
     * more permutations of the same unit, but this should at least cover the most
     * obvious cases.
     *
     * @ return The HeaderUnitDescriptor, with valid set to true if a unit was found.
     */
    HeaderUnitDescriptor findFreestandingUnitInHeader(const OUString& rHeader);

    HeaderUnitDescriptor extractUnitFromHeaderString(const OUString& rHeader);

    static OUString extractUnitStringFromFormat(const OUString& rFormatString);
    static OUString extractUnitStringForCell(const ScAddress& rAddress, ScDocument* pDoc);

    UtUnit getUnitForRef(formula::FormulaToken* pToken,
                         const ScAddress& rFormulaAddress,
                         ScDocument* pDoc);

    /**
     * Convert cells within a given range. The range MUST be restricted
     * to being a group of cells within one column, in one sheet/tab.
     * rOutputUnit MUST possess an input unit string.
     */
    bool convertCellUnitsForColumnRange(const ScRange& rRange,
                                        ScDocument* pDoc,
                                        const UtUnit& rOutputUnit);

    /**
     * Return both the UtUnit and the String as we usually want the UtUnit
     * (which is created from the String, and has to be created to ensure
     * that there is a valid unit), but we might also need the original
     * String (which can't necessarily be regenerated from the UtUnit).
     */
    HeaderUnitDescriptor findHeaderUnitForCell(const ScAddress& rCellAddress,
                                               ScDocument* pDoc);
};

}} // namespace sc::units

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_UNITSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
