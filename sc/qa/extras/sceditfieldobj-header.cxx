/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/text/textcontent.hxx>
#include <test/text/xtextcontent.hxx>
#include <test/text/xtextfield.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
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
class ScEditFieldObj_Header : public CalcUnoApiTest,
                              public apitest::TextContent,
                              public apitest::XPropertySet,
                              public apitest::XTextContent,
                              public apitest::XTextField
{
public:
    ScEditFieldObj_Header();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<text::XTextContent> getTextContent() override;
    virtual uno::Reference<text::XTextRange> getTextRange() override;
    virtual bool isAttachSupported() override { return false; }

    CPPUNIT_TEST_SUITE(ScEditFieldObj_Header);

    // TextContent
    CPPUNIT_TEST(testTextContentProperties);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XTextContent
    CPPUNIT_TEST(testGetAnchor);
    CPPUNIT_TEST(testAttach);

    // XTextField
    CPPUNIT_TEST(testGetPresentationEmptyString);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
    static uno::Reference<text::XTextField> mxField;
    static uno::Reference<text::XText> mxRightText;
};

uno::Reference<text::XTextField> ScEditFieldObj_Header::mxField;
uno::Reference<text::XText> ScEditFieldObj_Header::mxRightText;

ScEditFieldObj_Header::ScEditFieldObj_Header()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , TextContent(text::TextContentAnchorType_AS_CHARACTER,
                  text::TextContentAnchorType_AS_CHARACTER, text::WrapTextMode_NONE,
                  text::WrapTextMode_NONE)
{
}

void ScEditFieldObj_Header::setUp()
{
    CalcUnoApiTest::setUp();
    // Load an empty document.
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScEditFieldObj_Header::tearDown()
{
    // Clear these before the component is destroyed.  This is important!
    mxField.clear();
    mxRightText.clear();
    closeDocument(mxComponent);

    CalcUnoApiTest::tearDown();
}

uno::Reference<uno::XInterface> ScEditFieldObj_Header::init()
{
    // Return a field that's already in the header.
    if (!mxField.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, uno::UNO_QUERY_THROW);

        // Create a new URL field object, and populate it with name and URL.
        mxField.set(xSM->createInstance("com.sun.star.text.TextField.Time"), uno::UNO_QUERY_THROW);

        uno::Reference<style::XStyleFamiliesSupplier> xSFS(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xStyleFamilies(xSFS->getStyleFamilies(),
                                                              uno::UNO_SET_THROW);
        uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                           uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xPageStyles->getByName("Default"),
                                                     uno::UNO_QUERY_THROW);

        uno::Reference<sheet::XHeaderFooterContent> xHeaderContent(
            xPropSet->getPropertyValue("RightPageHeaderContent"), uno::UNO_QUERY_THROW);

        // Use the left header text.
        uno::Reference<text::XText> xText = xHeaderContent->getLeftText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        uno::Reference<text::XTextRange> xRange(xCursor, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextContent> xContent(mxField, uno::UNO_QUERY_THROW);
        xText->insertTextContent(xRange, xContent, false);

        xPropSet->setPropertyValue("RightPageHeaderContent", uno::makeAny(xHeaderContent));

        mxRightText = xHeaderContent->getRightText();
    }

    return mxField;
}

uno::Reference<text::XTextContent> ScEditFieldObj_Header::getTextContent()
{
    // Return a field object that's not yet inserted.
    uno::Reference<lang::XMultiServiceFactory> xSM(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> xField(
        xSM->createInstance("com.sun.star.text.TextField.Date"), uno::UNO_QUERY_THROW);
    return xField;
}

uno::Reference<text::XTextRange> ScEditFieldObj_Header::getTextRange()
{
    // Use the right header text for this.
    uno::Reference<text::XTextRange> xRange(mxRightText, uno::UNO_QUERY_THROW);
    return xRange;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScEditFieldObj_Header);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
