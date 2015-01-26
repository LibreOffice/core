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

namespace formula {
    class FormulaToken;
}

struct ut_system;

namespace sc {
namespace units {

class UnitsImpl: public Units {
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
    static ::boost::shared_ptr< Units > GetUnits();

    UnitsImpl();
    virtual ~UnitsImpl();

    virtual bool verifyFormula(ScTokenArray* pArray, const ScAddress& rFormulaAddress, ScDocument* pDoc) SAL_OVERRIDE;

private:
    UtUnit getOutputUnitsForOpCode(const UtUnit& pFirstUnit, const UtUnit& pSecondUnit, const OpCode& rOpCode);
    OUString extractUnitStringFromFormat(const OUString& rFormatString);
    OUString extractUnitStringForCell(ScAddress& rAddress, ScDocument* pDoc);
    UtUnit getUnitForRef(formula::FormulaToken* pToken,
                        const ScAddress& rFormulaAddress,
                        ScDocument* pDoc,
                        ::boost::shared_ptr< ut_system > pUnitSystem);

};

}} // namespace sc::units

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_UNITSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
