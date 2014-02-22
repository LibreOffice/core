/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <test/calc_unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/text/xtextfield.hxx>
#include <test/text/xtextcontent.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>

#define NUMBER_OF_TESTS 5

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScEditFieldObj_Header : public CalcUnoApiTest, apitest::XTextContent, apitest::XPropertySet
{
public:
    ScEditFieldObj_Header();

    virtual void setUp();
    virtual void tearDown();
    virtual uno::Reference<uno::XInterface> init();
    virtual uno::Reference<text::XTextContent> getTextContent();
    virtual uno::Reference<text::XTextRange> getTextRange();
    virtual bool isAttachSupported() { return false; }

    CPPUNIT_TEST_SUITE(ScEditFieldObj_Header);

    
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);

    
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testAttach);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;
    static uno::Reference<text::XTextField> mxField;
    static uno::Reference<text::XText> mxRightText;
};

sal_Int32 ScEditFieldObj_Header::nTest = 0;
uno::Reference<lang::XComponent> ScEditFieldObj_Header::mxComponent;
uno::Reference<text::XTextField> ScEditFieldObj_Header::mxField;
uno::Reference<text::XText> ScEditFieldObj_Header::mxRightText;

ScEditFieldObj_Header::ScEditFieldObj_Header()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

void ScEditFieldObj_Header::setUp()
{
    ++nTest;
    CalcUnoApiTest::setUp();
}

void ScEditFieldObj_Header::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        
        mxField.clear();
        mxRightText.clear();
        closeDocument(mxComponent);
    }

    CalcUnoApiTest::tearDown();
}

uno::Reference<uno::XInterface> ScEditFieldObj_Header::init()
{
    
    if (!mxField.is())
    {
        if (!mxComponent.is())
            
            mxComponent = loadFromDesktop("private:factory/scalc");

        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);

        
        mxField.set(xSM->createInstance("com.sun.star.text.TextField.Time"), UNO_QUERY_THROW);

        uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xStyleFamilies(xSFS->getStyleFamilies(), UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xPageStyles->getByName("Default"), UNO_QUERY_THROW);

        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent(
            xPropSet->getPropertyValue("RightPageHeaderContent"), UNO_QUERY_THROW);

        
        uno::Reference<text::XText> xText = xHeaderContent->getLeftText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        uno::Reference<text::XTextRange> xRange(xCursor, UNO_QUERY_THROW);
        uno::Reference<text::XTextContent> xContent(mxField, UNO_QUERY_THROW);
        xText->insertTextContent(xRange, xContent, sal_False);

        xPropSet->setPropertyValue("RightPageHeaderContent", uno::makeAny(xHeaderContent));

        mxRightText = xHeaderContent->getRightText();
    }

    return mxField;
}

uno::Reference<text::XTextContent> ScEditFieldObj_Header::getTextContent()
{
    
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> xField(
        xSM->createInstance("com.sun.star.text.TextField.Date"), UNO_QUERY_THROW);
    return xField;
}

uno::Reference<text::XTextRange> ScEditFieldObj_Header::getTextRange()
{
    
    uno::Reference<text::XTextRange> xRange(mxRightText, UNO_QUERY_THROW);
    return xRange;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScEditFieldObj_Header);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
