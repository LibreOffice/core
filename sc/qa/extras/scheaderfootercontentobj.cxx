/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xheaderfootercontent.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
#define NUMBER_OF_TESTS 3

class ScHeaderFooterContentObj : public CalcUnoApiTest, public apitest::XHeaderFooterContent
{
public:
    ScHeaderFooterContentObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScHeaderFooterContentObj);

    // XHeaderFooterContent
    CPPUNIT_TEST(testGetCenterText);
    CPPUNIT_TEST(testGetLeftText);
    CPPUNIT_TEST(testGetRightText);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;
};

sal_Int32 ScHeaderFooterContentObj::nTest = 0;
uno::Reference<lang::XComponent> ScHeaderFooterContentObj::mxComponent;

ScHeaderFooterContentObj::ScHeaderFooterContentObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScHeaderFooterContentObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamSupp(xDoc, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xStyleFamiliesNames(xStyleFamSupp->getStyleFamilies(),
                                                               UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamiliesNames->getByName("PageStyles"),
                                                       UNO_QUERY_THROW);
    uno::Any aDefaultStyle = xPageStyles->getByName("Default");
    uno::Reference<beans::XPropertySet> xProp(aDefaultStyle, UNO_QUERY_THROW);

    uno::Any aHFC = xProp->getPropertyValue("RightPageHeaderContent");
    uno::Reference<sheet::XHeaderFooterContent> xHFC(aHFC, UNO_QUERY_THROW);

    uno::Reference<text::XText> xTxtCenter = xHFC->getCenterText();
    uno::Reference<text::XText> xTxtLeft = xHFC->getLeftText();
    uno::Reference<text::XText> xTxtRight = xHFC->getRightText();

    xTxtCenter->setString("CENTER");
    xTxtLeft->setString("LEFT");
    xTxtRight->setString("RIGHT");

    xProp->setPropertyValue("RightPageHeaderContent", aHFC);

    return xHFC;
}

void ScHeaderFooterContentObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScHeaderFooterContentObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScHeaderFooterContentObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
