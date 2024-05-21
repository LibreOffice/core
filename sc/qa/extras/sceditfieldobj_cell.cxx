/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/lang/xcomponent.hxx>
#include <test/text/textcontent.hxx>
#include <test/text/xtextfield.hxx>
#include <test/text/xtextcontent.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScEditFieldObj_Cell : public UnoApiTest,
                            public apitest::TextContent,
                            public apitest::XComponent,
                            public apitest::XPropertySet,
                            public apitest::XTextContent,
                            public apitest::XTextField
{
public:
    ScEditFieldObj_Cell();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<text::XTextContent> getTextContent() override;
    virtual uno::Reference<text::XTextRange> getTextRange() override;
    virtual bool isAttachSupported() override { return true; }
    virtual void triggerDesktopTerminate() override{};

    void testEditFieldProperties();

    CPPUNIT_TEST_SUITE(ScEditFieldObj_Cell);

    // TextContent
    CPPUNIT_TEST(testTextContentProperties);

    // XComponent
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST(testDispose);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XTextField
    CPPUNIT_TEST(testGetPresentation);

    // XTextContent
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testAttach);

    // Tests specific to this service implementation.
    CPPUNIT_TEST(testEditFieldProperties);

    CPPUNIT_TEST_SUITE_END();

private:
    static uno::Reference<text::XTextField> mxField;
};

uno::Reference<text::XTextField> ScEditFieldObj_Cell::mxField;

ScEditFieldObj_Cell::ScEditFieldObj_Cell()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , TextContent(text::TextContentAnchorType_AS_CHARACTER,
                  text::TextContentAnchorType_AS_CHARACTER, text::WrapTextMode_NONE,
                  text::WrapTextMode_NONE)
{
}

void ScEditFieldObj_Cell::setUp()
{
    UnoApiTest::setUp();
    // Load an empty document.
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

void ScEditFieldObj_Cell::tearDown()
{
    mxField.clear();
    UnoApiTest::tearDown();
}

namespace
{
uno::Reference<text::XTextField> getNewField(const uno::Reference<lang::XMultiServiceFactory>& xSM)
{
    uno::Reference<text::XTextField> xField(
        xSM->createInstance(u"com.sun.star.text.TextField.URL"_ustr), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(u"Representation"_ustr, uno::Any(u"LibreOffice"_ustr));
    xPropSet->setPropertyValue(u"URL"_ustr, uno::Any(u"http://www.libreoffice.org/"_ustr));
    return xField;
}

} // namespace

uno::Reference<uno::XInterface> ScEditFieldObj_Cell::init()
{
    // Return a field that's already in the cell.
    if (!mxField.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, uno::UNO_QUERY_THROW);

        // Create a new URL field object, and populate it with name and URL.
        mxField = getNewField(xSM);

        // Insert this field into a cell.
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
        // Use cell A1 for this.
        uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 0);
        uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY_THROW);

        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        uno::Reference<text::XTextRange> xRange(xCursor, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextContent> xContent(mxField, uno::UNO_QUERY_THROW);
        xText->insertTextContent(xRange, xContent, false);
    }
    return mxField;
}

uno::Reference<text::XTextContent> ScEditFieldObj_Cell::getTextContent()
{
    // Return a field object that's not yet inserted.
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, uno::UNO_QUERY_THROW);
    return uno::Reference<text::XTextContent>(getNewField(xSM), uno::UNO_QUERY_THROW);
}

uno::Reference<text::XTextRange> ScEditFieldObj_Cell::getTextRange()
{
    // Use cell A2 for this.
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 1);
    uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY_THROW);

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextRange> xRange(xCursor, uno::UNO_QUERY_THROW);
    return xRange;
}

void ScEditFieldObj_Cell::testEditFieldProperties()
{
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, uno::UNO_QUERY_THROW);

    {
        // Test properties of date time field.
        uno::Reference<text::XTextField> xField(
            xSM->createInstance(u"com.sun.star.text.textfield.DateTime"_ustr),
            uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
        CPPUNIT_ASSERT_MESSAGE("failed to retrieve property set info.", xInfo.is());

        CPPUNIT_ASSERT_MESSAGE("Calc's date time field should have 'IsFixed' property.",
                               xInfo->hasPropertyByName(u"IsFixed"_ustr));
    }

    {
        // Test properties of document title field.
        uno::Reference<text::XTextField> xField(
            xSM->createInstance(u"com.sun.star.text.textfield.docinfo.Title"_ustr),
            uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
        CPPUNIT_ASSERT_MESSAGE("failed to retrieve property set info.", xInfo.is());

        CPPUNIT_ASSERT_MESSAGE("Calc's title field shouldn't have 'IsFixed' property.",
                               !xInfo->hasPropertyByName(u"IsFixed"_ustr));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScEditFieldObj_Cell);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
