/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>
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

namespace sc_apitest {

class ScEditFieldObj_Cell : public UnoApiTest, apitest::XTextField, apitest::XTextContent, apitest::XPropertySet
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

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);

    // XTextField
    CPPUNIT_TEST(testGetPresentation);

    // XTextContent
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testAttach);

    // Tests specific to this service implementation.
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
     : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

void ScEditFieldObj_Cell::setUp()
{
    ++nTest;
    UnoApiTest::setUp();
}

void ScEditFieldObj_Cell::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        mxField.clear();
        closeDocument(mxComponent);
    }

    UnoApiTest::tearDown();
}

namespace {

uno::Reference<text::XTextField> getNewField(const uno::Reference<lang::XMultiServiceFactory>& xSM)
{
    uno::Reference<text::XTextField> xField(
        xSM->createInstance("com.sun.star.text.TextField.URL"), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xField, UNO_QUERY_THROW);
    xPropSet->setPropertyValue("Representation", uno::makeAny(rtl::OUString("LibreOffice")));
    xPropSet->setPropertyValue("URL", uno::makeAny(rtl::OUString("http://www.libreoffice.org/")));
    return xField;
}

}

uno::Reference<uno::XInterface> ScEditFieldObj_Cell::init()
{
    // Return a field that's already in the cell.
    if (!mxField.is())
    {
        if (!mxComponent.is())
            // Load an empty document.
            mxComponent = loadFromDesktop("private:factory/scalc");

        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);

        // Create a new URL field object, and populate it with name and URL.
        mxField = getNewField(xSM);

        // Insert this field into a cell.
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);
        // Use cell A1 for this.
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
    // Return a field object that's not yet inserted.
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);
    return uno::Reference<text::XTextContent>(getNewField(xSM), UNO_QUERY_THROW);
}

uno::Reference<text::XTextRange> ScEditFieldObj_Cell::getTextRange()
{
    // Use cell A2 for this.
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
        // Test properties of date time field.
        uno::Reference<text::XTextField> xField(
            xSM->createInstance("com.sun.star.text.textfield.DateTime"), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xField, UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
        CPPUNIT_ASSERT_MESSAGE("failed to retrieve property set info.", xInfo.is());

        CPPUNIT_ASSERT_MESSAGE("Calc's date time field should have 'IsFixed' property.",
                               xInfo->hasPropertyByName("IsFixed"));
    }

    {
        // Test properties of document title field.
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
