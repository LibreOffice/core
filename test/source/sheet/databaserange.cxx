/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/databaserange.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void DatabaseRange::testMoveCells()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"MoveCells");

    bool bMoveCells = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bMoveCells);
    CPPUNIT_ASSERT_MESSAGE("Default MoveCells already changed", !bMoveCells);

    uno::Any aNewMoveCells;
    aNewMoveCells <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewMoveCells);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bMoveCells);
    CPPUNIT_ASSERT_MESSAGE("Value of MoveCells wasn't changed", bMoveCells);
}

void DatabaseRange::testKeepFormats()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"KeepFormats");

    bool bKeepFormats = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bKeepFormats);
    CPPUNIT_ASSERT_MESSAGE("Default KeepFormats already changed", !bKeepFormats);

    uno::Any aNewKeepFormats;
    aNewKeepFormats <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewKeepFormats);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bKeepFormats);
    CPPUNIT_ASSERT_MESSAGE("Value of KeepFormats wasn't changed", bKeepFormats);
}

void DatabaseRange::testStripData()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"StripData");

    bool bStripData = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bStripData);
    CPPUNIT_ASSERT_MESSAGE("Default StripData already changed", !bStripData);

    uno::Any aNewStripData;
    aNewStripData <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewStripData);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bStripData);
    CPPUNIT_ASSERT_MESSAGE("Value of StripData wasn't changed", bStripData);
}

void DatabaseRange::testAutoFilter()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"AutoFilter");

    bool bAutoFilter = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bAutoFilter);
    CPPUNIT_ASSERT_MESSAGE("Default AutoFilter already changed", !bAutoFilter);

    uno::Any aNewAutoFilter;
    aNewAutoFilter <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewAutoFilter);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bAutoFilter);
    CPPUNIT_ASSERT_MESSAGE("Value of AutoFilter wasn't changed", bAutoFilter);
}

void DatabaseRange::testUseFilterCriteriaSource()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"UseFilterCriteriaSource");

    bool bUseFilterCriteriaSource = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bUseFilterCriteriaSource);
    CPPUNIT_ASSERT_MESSAGE("Default UseFilterCriteriaSource already changed",
                           !bUseFilterCriteriaSource);

    uno::Any aNewUseFilterCriteriaSource;
    aNewUseFilterCriteriaSource <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewUseFilterCriteriaSource);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bUseFilterCriteriaSource);
    CPPUNIT_ASSERT_MESSAGE("Value of UseFilterCriteriaSource wasn't changed",
                           bUseFilterCriteriaSource);
}

void DatabaseRange::testFilterCriteriaSource()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"FilterCriteriaSource");

    table::CellRangeAddress cellRangeAddress;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= cellRangeAddress);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default RefreshPeriod already changed",
                                 table::CellRangeAddress(0, 0, 0, 0, 0), cellRangeAddress);

    uno::Any aFilterCriteriaSource;
    aFilterCriteriaSource <<= table::CellRangeAddress(1, 1, 1, 1, 1);
    xDatabaseRange->setPropertyValue(propName, aFilterCriteriaSource);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= cellRangeAddress);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of FilterCriteriaSource wasn't changed",
                                 table::CellRangeAddress(1, 1, 1, 1, 1), cellRangeAddress);
}

void DatabaseRange::testRefreshPeriod()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"RefreshPeriod");

    sal_Int32 aRefreshPeriod = 1;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= aRefreshPeriod);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default RefreshPeriod already changed", sal_Int32(0),
                                 aRefreshPeriod);

    uno::Any aNewRefreshPeriod;
    aNewRefreshPeriod <<= static_cast<sal_Int32>(42);
    xDatabaseRange->setPropertyValue(propName, aNewRefreshPeriod);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= aRefreshPeriod);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of RefreshPeriod wasn't changed", sal_Int32(42),
                                 aRefreshPeriod);
}

void DatabaseRange::testFromSelection()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"FromSelection");

    bool bFromSelection = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bFromSelection);
    CPPUNIT_ASSERT_MESSAGE("Default FromSelection already changed", !bFromSelection);

    uno::Any aNewFromSelection;
    aNewFromSelection <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewFromSelection);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bFromSelection);
    CPPUNIT_ASSERT_MESSAGE("Value of FromSelection wasn't changed", bFromSelection);
}

void DatabaseRange::testTokenIndex()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"TokenIndex");

    sal_Int32 aTokenIndex = 0;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= aTokenIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default TotalIndex already changed", sal_Int32(1), aTokenIndex);

    uno::Any aNewTokenIndex;
    aNewTokenIndex <<= static_cast<sal_Int32>(42);
    xDatabaseRange->setPropertyValue(propName, aNewTokenIndex);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= aTokenIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of TotalIndex was changed", sal_Int32(1), aTokenIndex);
}

void DatabaseRange::testTotalsRow()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"TotalsRow");

    bool bTotalsRow = true;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bTotalsRow);
    CPPUNIT_ASSERT_MESSAGE("Default TotalsRow already changed", !bTotalsRow);

    uno::Any aNewTotalsRow;
    aNewTotalsRow <<= true;
    xDatabaseRange->setPropertyValue(propName, aNewTotalsRow);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bTotalsRow);
    CPPUNIT_ASSERT_MESSAGE("Value of TotalsRow wasn't changed", bTotalsRow);
}

void DatabaseRange::testContainsHeader()
{
    uno::Reference<beans::XPropertySet> xDatabaseRange(init("DataArea"), UNO_QUERY_THROW);

    static const OUStringLiteral propName(u"ContainsHeader");

    bool bContainsHeader = false;
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bContainsHeader);
    CPPUNIT_ASSERT_MESSAGE("Default ContainsHeader already changed", bContainsHeader);

    uno::Any aNewContainsHeader;
    aNewContainsHeader <<= false;
    xDatabaseRange->setPropertyValue(propName, aNewContainsHeader);
    CPPUNIT_ASSERT(xDatabaseRange->getPropertyValue(propName) >>= bContainsHeader);
    CPPUNIT_ASSERT_MESSAGE("Value of ContainsHeader wasn't changed", !bContainsHeader);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
