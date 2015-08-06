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

class FormulaUnit
{
public:
    /*FormulaUnit(ut_unit* pUnit,
           const boost::optional< OUString > pInputString = boost::optional< OUString >())
        : mpUnit(pUnit, &freeUt)
        , msInputString(pInputString)
        , bIsValid(true)
    {}*/
    FormulaUnit(::boost::shared_ptr< ut_unit > pUnit,
           const boost::optional< OUString > pInputString = boost::optional< OUString >())
        : mpUnit(pUnit)
        , msInputString(pInputString)
        , bIsValid( true )
    {}
    FormulaUnit( const FormulaUnit& aUnit )
        : mpUnit( aUnit.mpUnit )
        , msInputString( aUnit.getInputString() )
        , bIsValid( aUnit.bIsValid )
    {}

    /*
     * Default constructor returns an empty/invalid unit.
     * (Note: this is different from the dimensionless unit which is valid.)
     */
    FormulaUnit() { bIsValid = false; }

    FormulaUnit& operator=(const FormulaUnit& rUnit) {
        //this->setNewUnit( rUnit.mpUnit );
        this->msInputString = rUnit.msInputString;
        this->bIsValid = rUnit.bIsValid;
        return *this;
    }

    bool isValid() const {
        return bIsValid;
    }

    //ut_unit* get() const                                { return mpUnit.get(); }
    ::boost::shared_ptr< ut_unit > getUnit() const      { return mpUnit; }
    boost::optional< OUString > getInputString() const  { return msInputString; }

    void setNewUnit( ::boost::shared_ptr< ut_unit > pNewUnit )
    {
        //mpUnit = pNewUnit;
        //bIsValid = true;
    }

private:
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
    bool bIsValid;

    static void freeUt(ut_unit* pUnit) {
        ut_free(pUnit);
    }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
