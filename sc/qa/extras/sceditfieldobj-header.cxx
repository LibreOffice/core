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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>

#define NUMBER_OF_TESTS 5

namespace sc_apitest {

class ScEditFieldObj_Header : public UnoApiTest, apitest::XTextContent, apitest::XPropertySet
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

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);

    // XTextContent
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
     : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

void ScEditFieldObj_Header::setUp()
{
    ++nTest;
    UnoApiTest::setUp();
}

void ScEditFieldObj_Header::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        // Clear these before the component is destroyed.  This is important!
        mxField.clear();
        mxRightText.clear();
        closeDocument(mxComponent);
    }

    UnoApiTest::tearDown();
}

uno::Reference<uno::XInterface> ScEditFieldObj_Header::init()
{
    // Return a field that's already in the header.
    if (!mxField.is())
    {
        if (!mxComponent.is())
            // Load an empty document.
            mxComponent = loadFromDesktop("private:factory/scalc");

        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);

        // Create a new URL field object, and populate it with name and URL.
        mxField.set(xSM->createInstance("com.sun.star.text.TextField.Time"), UNO_QUERY_THROW);

        uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xStyleFamilies(xSFS->getStyleFamilies(), UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xPageStyles->getByName("Default"), UNO_QUERY_THROW);

        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent(
            xPropSet->getPropertyValue("RightPageHeaderContent"), UNO_QUERY_THROW);

        // Use the left header text.
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
    // Return a field object that's not yet inserted.
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> xField(
        xSM->createInstance("com.sun.star.text.TextField.Date"), UNO_QUERY_THROW);
    return xField;
}

uno::Reference<text::XTextRange> ScEditFieldObj_Header::getTextRange()
{
    // Use the right header text for this.
    uno::Reference<text::XTextRange> xRange(mxRightText, UNO_QUERY_THROW);
    return xRange;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScEditFieldObj_Header);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
