/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_NamedRangesEnumeration : public UnoApiTest, public apitest::XEnumeration
{
public:
    ScIndexEnumeration_NamedRangesEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_NamedRangesEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();
};

ScIndexEnumeration_NamedRangesEnumeration::ScIndexEnumeration_NamedRangesEnumeration()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_NamedRangesEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XNamedRanges> xNR;
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"NamedRanges"_ustr) >>= xNR);

    table::CellRangeAddress aCellRangeAddr(0, 0, 0, 2, 2);
    table::CellAddress aBaseAddr(aCellRangeAddr.Sheet, aCellRangeAddr.StartColumn,
                                 aCellRangeAddr.StartRow);
    xNR->addNewByName(u"ANamedRange"_ustr, u"A1:B2"_ustr, aBaseAddr, 0);
    xNR->outputList(table::CellAddress(0, 1, 1));

    uno::Reference<container::XEnumerationAccess> xEA(xNR, uno::UNO_QUERY_THROW);

    return xEA->createEnumeration();
}

void ScIndexEnumeration_NamedRangesEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_NamedRangesEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
