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

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <formula/opcode.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <udunits2.h>

#include <units.hxx>
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

    UnitsImpl();
    virtual ~UnitsImpl();

    virtual bool verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) SAL_OVERRIDE;
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

private:
    UtUnit getOutputUnitsForOpCode(std::stack< UtUnit >& rUnitStack, const OpCode& rOpCode);
    OUString extractUnitStringFromFormat(const OUString& rFormatString);
    OUString extractUnitStringForCell(const ScAddress& rAddress, ScDocument* pDoc);
    bool extractUnitFromHeaderString(const OUString& rString, UtUnit& aUnit, OUString& sUnitString);
    UtUnit getUnitForRef(formula::FormulaToken* pToken,
                         const ScAddress& rFormulaAddress,
                         ScDocument* pDoc);

    /**
     * Return both the UtUnit and the String as we usually want the UtUnit
     * (which is created from the String, and has to be created to ensure
     * that there is a valid unit), but we might also need the original
     * String (which can't necessarily be regenerated from the UtUnit).
     */
    UtUnit findHeaderUnitForCell(const ScAddress& rCellAddress,
                                 ScDocument* pDoc,
                                 OUString& rsHeaderUnitString,
                                 ScAddress& rHeaderAddress);
};

}} // namespace sc::units

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_UNITSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
