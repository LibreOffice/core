/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/util/xrefreshable.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScHeaderFieldsObj : public CalcUnoApiTest,
                          public apitest::XEnumerationAccess,
                          public apitest::XRefreshable
{
public:
    ScHeaderFieldsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScHeaderFieldsObj);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XRefreshable
    CPPUNIT_TEST(testRefreshListener);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScHeaderFieldsObj::ScHeaderFieldsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScHeaderFieldsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);

    uno::Reference<style::XStyleFamiliesSupplier> xSFS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA(xSFS->getStyleFamilies(), uno::UNO_SET_THROW);
    uno::Reference<container::XNameAccess> xNA1(xNA->getByName("PageStyles"), uno::UNO_QUERY_THROW);
    uno::Reference<style::XStyle> xStyle(xNA1->getByName("Default"), uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XHeaderFooterContent> xHFC(
        xPropertySet->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xText(xHFC->getLeftText(), uno::UNO_SET_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> xTC(xMSF->createInstance("com.sun.star.text.TextField.Time"),
                                           uno::UNO_QUERY_THROW);
    xText->insertTextContent(xText->createTextCursor(), xTC, false);
    xPropertySet->setPropertyValue("RightPageHeaderContent", uno::makeAny(xHFC));

    uno::Reference<text::XTextFieldsSupplier> xTFS(xText, uno::UNO_QUERY_THROW);
    return xTFS->getTextFields();
}

void ScHeaderFieldsObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScHeaderFieldsObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScHeaderFieldsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
