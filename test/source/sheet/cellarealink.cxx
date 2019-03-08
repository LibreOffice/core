/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/cellarealink.hxx>
#include <sfx2/app.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void CellAreaLink::testUrl()
{
    uno::Reference<beans::XPropertySet> xCellAreaLink(init(), uno::UNO_QUERY_THROW);

    const OUString propName("Url");

    OUString aUrl;
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aUrl);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default Url already changed", m_aFileURL, aUrl);

    uno::Any aNewUrl;
    aNewUrl <<= OUString("file:///tmp");
    xCellAreaLink->setPropertyValue(propName, aNewUrl);
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aUrl);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of Url wasn't changed", OUString("file:///tmp"), aUrl);
}

void CellAreaLink::testFilter()
{
    uno::Reference<beans::XPropertySet> xCellAreaLink(init(), uno::UNO_QUERY_THROW);

    const OUString propName("Filter");

    OUString aFilter;
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aFilter);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default Filter already changed", OUString("calc8"), aFilter);

    uno::Any aNewFilter;
    aNewFilter <<= OUString("UnitTest");
    xCellAreaLink->setPropertyValue(propName, aNewFilter);
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aFilter);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of Filter wasn't changed", OUString("UnitTest"), aFilter);
}

void CellAreaLink::testFilterOptions()
{
    uno::Reference<beans::XPropertySet> xCellAreaLink(init(), uno::UNO_QUERY_THROW);

    const OUString propName("FilterOptions");

    OUString aFilterOptions;
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aFilterOptions);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default FilterOptions already changed", OUString(""),
                                 aFilterOptions);

    uno::Any aNewFilterOptions;
    aNewFilterOptions <<= OUString("UnitTest");
    xCellAreaLink->setPropertyValue(propName, aNewFilterOptions);
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aFilterOptions);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of FilterOptions wasn't changed", OUString("UnitTest"),
                                 aFilterOptions);
}

void CellAreaLink::testRefreshDelay()
{
    uno::Reference<beans::XPropertySet> xCellAreaLink(init(), uno::UNO_QUERY_THROW);

    const OUString propName("RefreshDelay");

    sal_Int32 aRefreshDelay = 0;
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aRefreshDelay);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default RefreshDelay already changed", sal_Int32(0),
                                 aRefreshDelay);

    uno::Any aNewRefreshDelay;
    aNewRefreshDelay <<= static_cast<sal_Int32>(42);
    xCellAreaLink->setPropertyValue(propName, aNewRefreshDelay);
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aRefreshDelay);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of RefreshDelay wasn't changed", sal_Int32(42),
                                 aRefreshDelay);
}

void CellAreaLink::testRefreshPeriod()
{
    uno::Reference<beans::XPropertySet> xCellAreaLink(init(), uno::UNO_QUERY_THROW);

    const OUString propName("RefreshPeriod");

    sal_Int32 aRefreshPeriod = 0;
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aRefreshPeriod);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default RefreshPeriod already changed", sal_Int32(0),
                                 aRefreshPeriod);

    uno::Any aNewRefreshPeriod;
    aNewRefreshPeriod <<= static_cast<sal_Int32>(42);
    xCellAreaLink->setPropertyValue(propName, aNewRefreshPeriod);
    CPPUNIT_ASSERT(xCellAreaLink->getPropertyValue(propName) >>= aRefreshPeriod);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of RefreshPeriod wasn't changed", sal_Int32(42),
                                 aRefreshPeriod);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
