/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SC_SOURCE_CORE_UNITS_UTUNIT_HXX
#define INCLUDED_SC_SOURCE_CORE_UNITS_UTUNIT_HXX

#include <rtl/ustring.hxx>

#include <boost/shared_ptr.hpp>

#include <udunits2.h>

namespace sc {
namespace units {

/*
 * Convenience shared_ptr wrapper for ut_unit, which takes
 * care of dealing with the necessary custom deleter.
 *
 * We could just use shared_ptr directly, however to avoid
 * repeatedly referring to the custom deleter we can instead
 * simply use it in the 3 necessary places within our UtUnit
 * wrapper.
 */
class UtUnit {
private:
    ::boost::shared_ptr< ut_unit > mpUnit;

    static void freeUt(ut_unit* pUnit) {
        ut_free(pUnit);
    }

public:
    UtUnit(ut_unit* pUnit = 0):
        mpUnit(pUnit, &freeUt)
    {}

    UtUnit(const UtUnit& rUnit):
        mpUnit(rUnit.mpUnit)
    {}

    void reset(ut_unit* pUnit) {
        mpUnit.reset(pUnit, &freeUt);
    }

    OUString getString() const;

    ut_unit* get() const {
        return mpUnit.get();
    }

    explicit operator bool() const {
        return mpUnit.operator bool();
    }
};

}} // namespace sc::units

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_UTUNIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
