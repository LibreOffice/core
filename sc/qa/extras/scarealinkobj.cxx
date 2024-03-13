/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/sheet/cellarealink.hxx>
#include <test/sheet/xarealink.hxx>
#include <test/util/xrefreshable.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScAreaLinkObj : public UnoApiTest,
                      public apitest::CellAreaLink,
                      public apitest::XAreaLink,
                      public apitest::XPropertySet,
                      public apitest::XRefreshable
{
public:
    ScAreaLinkObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScAreaLinkObj);

    // CellAreaLink
    CPPUNIT_TEST(testUrl);
    CPPUNIT_TEST(testFilter);
    CPPUNIT_TEST(testFilterOptions);
    CPPUNIT_TEST(testRefreshDelay);
    CPPUNIT_TEST(testRefreshPeriod);

    // XAreaLink
    CPPUNIT_TEST(testGetDestArea);
    CPPUNIT_TEST(testGetSourceArea);
    CPPUNIT_TEST(testSetSourceArea);
    CPPUNIT_TEST(testSetDestArea);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XRefreshable
    CPPUNIT_TEST(testRefreshListener);

    CPPUNIT_TEST_SUITE_END();
};

ScAreaLinkObj::ScAreaLinkObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , CellAreaLink(m_directories.getURLFromSrc(u"/sc/qa/extras/testdocuments/scarealinkobj.ods"))
{
}

uno::Reference<uno::XInterface> ScAreaLinkObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XAreaLinks> xLinks(xPropSet->getPropertyValue("AreaLinks"),
                                             uno::UNO_QUERY_THROW);

    table::CellAddress aCellAddress(1, 2, 3);
    xLinks->insertAtPosition(
        aCellAddress, m_directories.getURLFromSrc(u"/sc/qa/extras/testdocuments/scarealinkobj.ods"),
        "a2:b5", "", "");

    uno::Reference<sheet::XAreaLink> xLink(xLinks->getByIndex(0), uno::UNO_QUERY_THROW);
    return xLink;
}

void ScAreaLinkObj::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAreaLinkObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
