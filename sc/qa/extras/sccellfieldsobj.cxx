/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/util/xrefreshable.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScCellFieldsObj : public CalcUnoApiTest,
                        public apitest::XElementAccess,
                        public apitest::XEnumerationAccess,
                        public apitest::XRefreshable
{
public:
    ScCellFieldsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScCellFieldsObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XRefreshable
    CPPUNIT_TEST(testRefreshListener);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScCellFieldsObj::ScCellFieldsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<text::XTextField>::get())
{
}

uno::Reference<uno::XInterface> ScCellFieldsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> xTC(xMSF->createInstance("com.sun.star.text.TextField.URL"),
                                           uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell(xSheet0->getCellByPosition(2, 3), uno::UNO_SET_THROW);
    uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY_THROW);
    xText->insertTextContent(xText->createTextCursor(), xTC, true);

    uno::Reference<text::XTextFieldsSupplier> xTFS(xCell, uno::UNO_QUERY_THROW);
    return xTFS->getTextFields();
}

void ScCellFieldsObj::setUp()
{
    CalcUnoApiTest::setUp();
    m_xComponent = loadFromDesktop("private:factory/scalc");
    CPPUNIT_ASSERT_MESSAGE("no component", m_xComponent.is());
}

void ScCellFieldsObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellFieldsObj);
} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
