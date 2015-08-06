/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_FORMULAUNIT_HXX
#define INCLUDED_FORMULAUNIT_HXX

#include <rtl/ustring.hxx>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <udunits2/udunits2.h>

class FormulaUnit {
public:
    FormulaUnit( ::boost::shared_ptr< ut_unit > aUnit, boost::optional< OUString > aInputString) :
        mpUnit(aUnit),
        msInputString(aInputString)
    {}

    /*
     * Default constructor returns an empty/invalid unit.
     * (Note: this is different from the dimensionless unit which is valid.)
     */
    FormulaUnit() {}

    bool isValid() const {
        // We use a null pointer/empty unit to indicate an invalid unit.
        return mpUnit.get() != 0;
    }

    ::boost::shared_ptr< ut_unit > getUnit() const     { return mpUnit; }
    boost::optional< OUString > getInputString() const { return msInputString; }

protected:
    ::boost::shared_ptr< ut_unit > mpUnit;

    /**
     * The original input string used in createUnit.
     * We can't necessarily convert a ut_unit back into the
     * original representation (e.g. cm gets formatted as 0.01m
     * by default), hence we should store the original string
     * as may need to display it to the user again.
     *
     * There is no input string for units that are created when manipulating
     * other units (i.e. multiplication/division of other UtUnits).
     */
    boost::optional< OUString > msInputString;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
