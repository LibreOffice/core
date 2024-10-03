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

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <cppunit/TestAssert.h>

/** @brief Trait used by CPPUNIT_ASSERT* macros to compare com::sun::star:awt::Point.
 *
 * This specialization from @c struct @c assertion_traits<> helps to compare
 * @see com::sun::star::awt::Point.
 */
template <>
inline std::string CPPUNIT_NS::assertion_traits<css::awt::Point>::toString(const css::awt::Point& x)
{
    OStringStream ost;
    ost << "Point: " << x.X << "." << x.Y << " (coordinate: X.Y)";
    return ost.str();
}

/** @brief Trait used by CPPUNIT_ASSERT* macros to compare com::sun::star:awt::Size.
 *
 * This specialization from @c struct @c assertion_traits<> helps to compare
 * @see com::sun::star::awt::Size.
 */
template <>
inline std::string CPPUNIT_NS::assertion_traits<css::awt::Size>::toString(const css::awt::Size& x)
{
    OStringStream ost;
    ost << "Size: " << x.Width << " x " << x.Height << " (Width x Height)";
    return ost.str();
}

/** @brief Trait used by CPPUNIT_ASSERT* macros to compare com::sun::star::table::CellAddress.
 *
 * This specialization from @c struct @c assertion_traits<> helps to compare
 * @see com::sun::star::table::CellAddress.
 */
template <>
inline std::string
CPPUNIT_NS::assertion_traits<css::table::CellAddress>::toString(const css::table::CellAddress& x)
{
    OStringStream ost;
    ost << "Sheet: " << x.Sheet << " Column: " << x.Column << " Row: " << x.Row;
    return ost.str();
}

/** @brief Trait used by CPPUNIT_ASSERT* macros to compare com::sun::star::table::CellRangeAddress.
 *
 * This specialization from @c struct @c assertion_traits<> helps to compare
 * @see com::sun::star::table::CellRangeAddress.
 */
template <>
inline std::string CPPUNIT_NS::assertion_traits<css::table::CellRangeAddress>::toString(
    const css::table::CellRangeAddress& x)
{
    OStringStream ost;
    ost << "Sheet: " << x.Sheet << " StartColumn: " << x.StartColumn << " StartRow: " << x.StartRow
        << " EndColumn: " << x.EndColumn << " EndRow: " << x.EndRow;
    return ost.str();
}

template <>
inline std::string
CPPUNIT_NS::assertion_traits<std::u16string_view>::toString(std::u16string_view const& x)
{
    return std::string(OUStringToOString(x, RTL_TEXTENCODING_UTF8));
}

#endif // INCLUDED_TEST_CPPUNITASSERTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
