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

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <udunits2/udunits2.h>

namespace sc {
namespace units {

namespace test {
    class UnitsTest;
}

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
    friend class test::UnitsTest;

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

    static void freeUt(ut_unit* pUnit) {
        ut_free(pUnit);
    }

    UtUnit(ut_unit* pUnit,
           const boost::optional< OUString > rInputString = boost::optional< OUString >())
        : mpUnit(pUnit, &freeUt)
        , msInputString(rInputString)
    {}

    void reset(ut_unit* pUnit) {
        mpUnit.reset(pUnit, &freeUt);
    }

    ut_unit* get() const {
        return mpUnit.get();
    }

public:
    /**
     * return false if we try to create in invalid unit.
     */
    static bool createUnit(const OUString& rUnitString, UtUnit& rUnitOut, const boost::shared_ptr< ut_system >& pUTSystem);

    /*
     * Default constructor returns an empty/invalid unit.
     * (Note: this is different from the dimensionless unit which is valid.)
     */
    UtUnit() {};

    UtUnit(const UtUnit& rUnit)
        : mpUnit(rUnit.mpUnit)
        , msInputString(rUnit.msInputString)
    {}

    boost::optional< OUString > getInputString() const {
        return msInputString;
    }

    OUString getString() const;

    ::boost::shared_ptr< ut_unit > getUnit() const {
        return mpUnit;
    }

    bool isValid() const {
        // We use a null pointer/empty unit to indicate an invalid unit.
        return mpUnit.get() != 0;
    }

    bool isDimensionless() const {
        return ut_is_dimensionless(this->get());
    }

    UtUnit& operator=(const UtUnit& rUnit) {
        //mpUnit = rUnit.getUnit();
        msInputString = rUnit.msInputString;
        return *this;
    }

    bool operator==(const UtUnit& rUnit) const {
        return ut_compare(this->get(), rUnit.get()) == 0;
    }

    bool operator!=(const UtUnit& rUnit) const {
        return !operator==(rUnit);
    }

    UtUnit operator*(const UtUnit& rUnit) {
        return UtUnit(ut_multiply(this->get(), rUnit.get()));
    }

    UtUnit& operator*=(const UtUnit& rUnit) {
        reset(ut_multiply(this->get(), rUnit.get()));
        return *this;
    }

    /**
     * Multiply a unit by a given constant.
     * This scales the unit by the inverse of that constant,
     * e.g. 100*m is equivalent to cm, hence
     * the unit needs to be scaled by 1/100.
     * (This is implemented in this way so that we can simply
     * multiply units by any constants present in a formula
     * in a natural way.)
     */
    UtUnit operator*(const double nMultiplier) {
        return UtUnit(ut_scale(1/nMultiplier, this->get()));
    }

    UtUnit operator/(const UtUnit& rUnit) {
        // the parameter is the right hand side value in the operation,
        // i.e. we are working with this / rUnit.
        return UtUnit(ut_divide(this->get(), rUnit.get()));
    }

    bool areConvertibleTo(const UtUnit& rUnit) const {
        return ut_are_convertible(this->get(), rUnit.get());
    }

    double convertValueTo(double nOriginalValue, const UtUnit& rUnit) const {
        assert(isValid());
        assert(rUnit.isValid());

        // We could write our own cv_converter wrapper too, but that
        // seems unnecessary given the limited selection of
        // operations (convert float/double, either individually
        // or as an array) -- of which we only need a subset anyway
        // (ie. only for doubles).
        cv_converter* pConverter = ut_get_converter(this->get(), rUnit.get());
        assert(pConverter);

        float nConvertedValue = cv_convert_double(pConverter, nOriginalValue);

        cv_free(pConverter);
        return nConvertedValue;
    }
};

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const UtUnit& rUnit )
{
    return stream << "[" << rUnit.getString() << "]";
}

}} // namespace sc::units

#endif // INCLUDED_SC_SOURCE_CORE_UNITS_UTUNIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
