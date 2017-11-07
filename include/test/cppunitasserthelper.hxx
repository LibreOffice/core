/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_CPPUNITASSERTHELPER_HXX
#define INCLUDED_TEST_CPPUNITASSERTHELPER_HXX

#include <rtl/ustring.hxx>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <cppunit/TestAssert.h>

CPPUNIT_NS_BEGIN

/** @brief Trait used by CPPUNIT_ASSERT* macros to compare com::sun::star::table::CellAddress.
 *
 * This specialization from @c struct @c assertion_traits<> helps to compare
 * @see com::sun::star::table::CellAddress.
 */
template <> struct assertion_traits<css::table::CellAddress>
{
    static bool equal(const css::table::CellAddress& x, const css::table::CellAddress& y)
    {
        return x == y;
    }

    static std::string toString(const css::table::CellAddress& x)
    {
        OStringStream ost;
        ost << "Sheet: " << x.Sheet << " Column: " << x.Column << " Row: " << x.Row;
        return ost.str();
    }
};

/** @brief Trait used by CPPUNIT_ASSERT* macros to compare com::sun::star::table::CellRangeAddress.
 *
 * This specialization from @c struct @c assertion_traits<> helps to compare
 * @see com::sun::star::table::CellRangeAddress.
 */
template <> struct assertion_traits<css::table::CellRangeAddress>
{
    static bool equal(const css::table::CellRangeAddress& x, const css::table::CellRangeAddress& y)
    {
        return x == y;
    }

    static std::string toString(const css::table::CellRangeAddress& x)
    {
        OStringStream ost;
        ost << "Sheet: " << x.Sheet << " StartColumn: " << x.StartColumn
            << " StartRow: " << x.StartRow << " EndColumn: " << x.EndColumn
            << " EndRow: " << x.EndRow;
        return ost.str();
    }
};

CPPUNIT_NS_END

#endif // INCLUDED_TEST_CPPUNITASSERTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
