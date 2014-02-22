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
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#define NUMBER_OF_TESTS 7

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScEditFieldObj_Cell : public CalcUnoApiTest, apitest::XTextField, apitest::XTextContent, apitest::XPropertySet
{
public:
    ScEditFieldObj_Cell();

    virtual void setUp();
    virtual void tearDown();
    virtual uno::Reference<uno::XInterface> init();
    virtual uno::Reference<text::XTextContent> getTextContent();
    virtual uno::Reference<text::XTextRange> getTextRange();
    virtual bool isAttachSupported() { return true; }

    void testEditFieldProperties();

    CPPUNIT_TEST_SUITE(ScEditFieldObj_Cell);

    
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);

    
    CPPUNIT_TEST(testGetPresentation);

    
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testAttach);

    
    CPPUNIT_TEST(testEditFieldProperties);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;
    static uno::Reference<text::XTextField> mxField;
};

sal_Int32 ScEditFieldObj_Cell::nTest = 0;
uno::Reference<lang::XComponent> ScEditFieldObj_Cell::mxComponent;
uno::Reference<text::XTextField> ScEditFieldObj_Cell::mxField;

ScEditFieldObj_Cell::ScEditFieldObj_Cell()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

void ScEditFieldObj_Cell::setUp()
{
    ++nTest;
    CalcUnoApiTest::setUp();
}

void ScEditFieldObj_Cell::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        mxField.clear();
        closeDocument(mxComponent);
    }

    CalcUnoApiTest::tearDown();
}

namespace {

uno::Reference<text::XTextField> getNewField(const uno::Reference<lang::XMultiServiceFactory>& xSM)
{
    uno::Reference<text::XTextField> xField(
        xSM->createInstance("com.sun.star.text.TextField.URL"), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xField, UNO_QUERY_THROW);
    xPropSet->setPropertyValue("Representation", uno::makeAny(OUString("LibreOffice")));
    xPropSet->setPropertyValue("URL", uno::makeAny(OUString("http:
    return xField;
}

}

uno::Reference<uno::XInterface> ScEditFieldObj_Cell::init()
{
    
    if (!mxField.is())
    {
        if (!mxComponent.is())
            
            mxComponent = loadFromDesktop("private:factory/scalc");

        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);

        
        mxField = getNewField(xSM);

        
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);
        
        uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 0);
        uno::Reference<text::XText> xText(xCell, UNO_QUERY_THROW);

        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        uno::Reference<text::XTextRange> xRange(xCursor, UNO_QUERY_THROW);
        uno::Reference<text::XTextContent> xContent(mxField, UNO_QUERY_THROW);
        xText->insertTextContent(xRange, xContent, sal_False);
    }
    return mxField;
}

uno::Reference<text::XTextContent> ScEditFieldObj_Cell::getTextContent()
{
    
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);
    return uno::Reference<text::XTextContent>(getNewField(xSM), UNO_QUERY_THROW);
}

uno::Reference<text::XTextRange> ScEditFieldObj_Cell::getTextRange()
{
    
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 1);
    uno::Reference<text::XText> xText(xCell, UNO_QUERY_THROW);

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextRange> xRange(xCursor, UNO_QUERY_THROW);
    return xRange;
}

void ScEditFieldObj_Cell::testEditFieldProperties()
{
    CPPUNIT_ASSERT_MESSAGE("component doesn't exist.", mxComponent.is());
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);

    {
        
        uno::Reference<text::XTextField> xField(
            xSM->createInstance("com.sun.star.text.textfield.DateTime"), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xField, UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
        CPPUNIT_ASSERT_MESSAGE("failed to retrieve property set info.", xInfo.is());

        CPPUNIT_ASSERT_MESSAGE("Calc's date time field should have 'IsFixed' property.",
                               xInfo->hasPropertyByName("IsFixed"));
    }

    {
        
        uno::Reference<text::XTextField> xField(
            xSM->createInstance("com.sun.star.text.textfield.docinfo.Title"), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xField, UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
        CPPUNIT_ASSERT_MESSAGE("failed to retrieve property set info.", xInfo.is());

        CPPUNIT_ASSERT_MESSAGE("Calc's title field shouldn't have 'IsFixed' property.",
                               !xInfo->hasPropertyByName("IsFixed"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScEditFieldObj_Cell);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
