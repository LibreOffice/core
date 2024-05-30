/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <tools/color.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>


class SdActiveXControlsTest: public SdModelTestBase
{
public:
    SdActiveXControlsTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

    void testBackgroundColor();
    void testLabelProperties();
    void testTextBoxProperties();
    void testSpinButtonProperties();
    void testCommandButtonProperties();
    void testScrollBarProperties();
    void testCheckBoxProperties();
    void testOptionButtonProperties();
    void testComboBoxProperties();
    void testListBoxProperties();
    void testToggleButtonProperties();
    void testPictureProperties();
    void testFontProperties();

    CPPUNIT_TEST_SUITE(SdActiveXControlsTest);

    CPPUNIT_TEST(testBackgroundColor);
    CPPUNIT_TEST(testLabelProperties);
    CPPUNIT_TEST(testTextBoxProperties);
    CPPUNIT_TEST(testSpinButtonProperties);
    CPPUNIT_TEST(testCommandButtonProperties);
    CPPUNIT_TEST(testScrollBarProperties);
    CPPUNIT_TEST(testCheckBoxProperties);
    CPPUNIT_TEST(testOptionButtonProperties);
    CPPUNIT_TEST(testComboBoxProperties);
    CPPUNIT_TEST(testListBoxProperties);
    CPPUNIT_TEST(testToggleButtonProperties);
    CPPUNIT_TEST(testPictureProperties);
    CPPUNIT_TEST(testFontProperties);

    CPPUNIT_TEST_SUITE_END();
};

void SdActiveXControlsTest::testBackgroundColor()
{
    // Check whether all system colors are imported correctly
    createSdImpressDoc("pptx/control_background_color.pptx");

    const std::vector<Color> vBackgroundColors =
    {
        0xD4D0C8, // Scroll Bars
        0x004E98, // Desktop
        0x0054E3, // Active Title Bar
        0x7A96DF, // Inactive Title Bar
        0xFFFFFF, // Menu Bar
        0xFFFFFF, // Window Background
        0x000000, // Window Frame
        0x000000, // Menu Text
        0x000000, // Window Text
        0xFFFFFF, // Active Title Bar Text
        0xD4D0C8, // Active Border
        0xD4D0C8, // Inactive Border
        0x808080, // Application Workspace
        0x316AC5, // Highlight
        0xFFFFFF, // Highlight Text
        0xECE9D8, // Button Face
        0xACA899, // Button Shadow
        0xACA899, // Disabled Text
        COL_BLACK, // Button Text
        0xD8E4F8, // Inactive Title Bar Text
        0xFFFFFF, // Button Highlight
        0x716F64, // Button Dark Shadow
        0xF1EFE2, // Button Light Shadow
        COL_BLACK, // Tooltip Text
        0xFFFFE1, // Tooltip
        0xFF0000, // Custom red color
    };

    for (size_t i = 0; i < vBackgroundColors.size(); ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(i, 0), uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
        Color nColor;
        xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
        OString sMessage = "The wrong control's index is: " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), vBackgroundColors[i], nColor);
    }
}

void SdActiveXControlsTest::testLabelProperties()
{
    createSdImpressDoc("pptx/activex_label.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Label1"_ustr, sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    Color nColor;
    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nBorderStyle);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    style::VerticalAlignment eAlign;
    xPropertySet->getPropertyValue(u"VerticalAlign"_ustr) >>= eAlign;
    CPPUNIT_ASSERT_EQUAL(style::VerticalAlignment_TOP, eAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Custom Label"_ustr, sLabel);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xE0E0E0), nColor);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, nColor);

    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    Color nBorderColor;
    xPropertySet->getPropertyValue(u"BorderColor"_ustr) >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, nBorderColor);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    xPropertySet->getPropertyValue(u"VerticalAlign"_ustr) >>= eAlign;
    CPPUNIT_ASSERT_EQUAL(style::VerticalAlignment_TOP, eAlign);

    // Third control has transparent background
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor);
}

void SdActiveXControlsTest::testTextBoxProperties()
{
    createSdImpressDoc("pptx/activex_textbox.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sText;
    xPropertySet->getPropertyValue(u"Text"_ustr) >>= sText;
    CPPUNIT_ASSERT_EQUAL(OUString(), sText);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    Color nColor;
    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nBorderStyle);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    style::VerticalAlignment eAlign;
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"VerticalAlign"_ustr) >>= eAlign);

    bool bHideSelection;
    xPropertySet->getPropertyValue(u"HideInactiveSelection"_ustr) >>= bHideSelection;
    CPPUNIT_ASSERT_EQUAL(true, bHideSelection);

    sal_Int16 nMaxLength;
    xPropertySet->getPropertyValue(u"MaxTextLen"_ustr) >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nMaxLength);

    sal_Int16 nEchoChar;
    xPropertySet->getPropertyValue(u"EchoChar"_ustr) >>= nEchoChar;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nEchoChar);

    bool bHScroll;
    xPropertySet->getPropertyValue(u"HScroll"_ustr) >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(false, bHScroll);

    bool bVScroll;
    xPropertySet->getPropertyValue(u"VScroll"_ustr) >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(false, bVScroll);

    bool bReadOnly;
    xPropertySet->getPropertyValue(u"ReadOnly"_ustr) >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Text"_ustr) >>= sText;
    CPPUNIT_ASSERT_EQUAL(u"Some Text"_ustr, sText);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    // These textfields are not multilines in the pptx testfile
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x404040), nColor);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x00C000), nColor);

    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"VerticalAlign"_ustr) >>= eAlign);

    xPropertySet->getPropertyValue(u"HideInactiveSelection"_ustr) >>= bHideSelection;
    CPPUNIT_ASSERT_EQUAL(false, bHideSelection);

    xPropertySet->getPropertyValue(u"MaxTextLen"_ustr) >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), nMaxLength);

    xPropertySet->getPropertyValue(u"EchoChar"_ustr) >>= nEchoChar;
    CPPUNIT_ASSERT_EQUAL(sal_Int16('x'), nEchoChar);

    xPropertySet->getPropertyValue(u"HScroll"_ustr) >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(true, bHScroll);

    xPropertySet->getPropertyValue(u"VScroll"_ustr) >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(false, bVScroll);

    xPropertySet->getPropertyValue(u"ReadOnly"_ustr) >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(true, bReadOnly);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    xPropertySet->getPropertyValue(u"HScroll"_ustr) >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(false, bHScroll);

    xPropertySet->getPropertyValue(u"VScroll"_ustr) >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(true, bVScroll);

    // Fourth shape has both scroll bar
    xControlShape.set(getShapeFromPage(3, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"HScroll"_ustr) >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(true, bHScroll);

    xPropertySet->getPropertyValue(u"VScroll"_ustr) >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(true, bVScroll);
}

void SdActiveXControlsTest::testSpinButtonProperties()
{
    createSdImpressDoc("pptx/activex_spinbutton.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xECE9D8), nColor);

    sal_Int32 nMax;
    xPropertySet->getPropertyValue(u"SpinValueMax"_ustr) >>= nMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nMax);

    sal_Int32 nMin;
    xPropertySet->getPropertyValue(u"SpinValueMin"_ustr) >>= nMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nMin);

    sal_Int32 nIncrement;
    xPropertySet->getPropertyValue(u"SpinIncrement"_ustr) >>= nIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nIncrement);

    bool bRepeat;
    xPropertySet->getPropertyValue(u"Repeat"_ustr) >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(true, bRepeat);

    sal_Int32 nDelay;
    xPropertySet->getPropertyValue(u"RepeatDelay"_ustr) >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), nDelay);

    Color nArrowColor;
    xPropertySet->getPropertyValue(u"SymbolColor"_ustr) >>= nArrowColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nArrowColor);

    sal_Int32 nOrientation;
    xPropertySet->getPropertyValue(u"Orientation"_ustr) >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::HORIZONTAL), nOrientation);

    sal_Int32 nSpinValue;
    xPropertySet->getPropertyValue(u"SpinValue"_ustr) >>= nSpinValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nSpinValue);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, nColor);

    xPropertySet->getPropertyValue(u"SpinValueMax"_ustr) >>= nMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(320), nMax);

    xPropertySet->getPropertyValue(u"SpinValueMin"_ustr) >>= nMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nMin);

    xPropertySet->getPropertyValue(u"SpinIncrement"_ustr) >>= nIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nIncrement);

    xPropertySet->getPropertyValue(u"Repeat"_ustr) >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(true, bRepeat);

    xPropertySet->getPropertyValue(u"RepeatDelay"_ustr) >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nDelay);

    xPropertySet->getPropertyValue(u"SymbolColor"_ustr) >>= nArrowColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, nArrowColor);

    xPropertySet->getPropertyValue(u"Orientation"_ustr) >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::VERTICAL), nOrientation);

    xPropertySet->getPropertyValue(u"SpinValue"_ustr) >>= nSpinValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nSpinValue);

    // Third control has horizontal orientation
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Orientation"_ustr) >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::HORIZONTAL), nOrientation);
}

void SdActiveXControlsTest::testCommandButtonProperties()
{
    createSdImpressDoc("pptx/activex_commandbutton.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"CommandButton1"_ustr, sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xECE9D8), nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    bool bFocusOnClick;
    xPropertySet->getPropertyValue(u"FocusOnClick"_ustr) >>= bFocusOnClick;
    CPPUNIT_ASSERT_EQUAL(true, bFocusOnClick);

    bool bRepeat;
    xPropertySet->getPropertyValue(u"Repeat"_ustr) >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(false, bRepeat);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Custom Caption"_ustr, sLabel);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFF80), nColor);

    xPropertySet->getPropertyValue(u"FocusOnClick"_ustr) >>= bFocusOnClick;
    CPPUNIT_ASSERT_EQUAL(false, bFocusOnClick);

    xPropertySet->getPropertyValue(u"Repeat"_ustr) >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(false, bRepeat);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    // Transparent background
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);
}

void SdActiveXControlsTest::testScrollBarProperties()
{
    createSdImpressDoc("pptx/activex_scrollbar.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xECE9D8), nColor);

    xPropertySet->getPropertyValue(u"SymbolColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    sal_Int32 nDelay;
    xPropertySet->getPropertyValue(u"RepeatDelay"_ustr) >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), nDelay);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nBorderStyle);

    sal_Int32 nVisibleSize;
    xPropertySet->getPropertyValue(u"VisibleSize"_ustr) >>= nVisibleSize;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nVisibleSize);

    sal_Int32 nScrollValueMin;
    xPropertySet->getPropertyValue(u"ScrollValueMin"_ustr) >>= nScrollValueMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nScrollValueMin);

    sal_Int32 nScrollValueMax;
    xPropertySet->getPropertyValue(u"ScrollValueMax"_ustr) >>= nScrollValueMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(32767), nScrollValueMax);

    sal_Int32 nScrollValue;
    xPropertySet->getPropertyValue(u"DefaultScrollValue"_ustr) >>= nScrollValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nScrollValue);

    sal_Int32 nLineIncrement;
    xPropertySet->getPropertyValue(u"LineIncrement"_ustr) >>= nLineIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nLineIncrement);

    sal_Int32 nBlockIncrement;
    xPropertySet->getPropertyValue(u"BlockIncrement"_ustr) >>= nBlockIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nBlockIncrement);

    sal_Int32 nOrientation;
    xPropertySet->getPropertyValue(u"Orientation"_ustr) >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::VERTICAL), nOrientation);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTCYAN, nColor);

    xPropertySet->getPropertyValue(u"SymbolColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropertySet->getPropertyValue(u"RepeatDelay"_ustr) >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(230), nDelay);

    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nBorderStyle);

    xPropertySet->getPropertyValue(u"VisibleSize"_ustr) >>= nVisibleSize;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nVisibleSize);

    xPropertySet->getPropertyValue(u"ScrollValueMin"_ustr) >>= nScrollValueMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nScrollValueMin);

    xPropertySet->getPropertyValue(u"ScrollValueMax"_ustr) >>= nScrollValueMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1234567), nScrollValueMax);

    xPropertySet->getPropertyValue(u"DefaultScrollValue"_ustr) >>= nScrollValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(125), nScrollValue);

    xPropertySet->getPropertyValue(u"LineIncrement"_ustr) >>= nLineIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), nLineIncrement);

    xPropertySet->getPropertyValue(u"BlockIncrement"_ustr) >>= nBlockIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nBlockIncrement);

    xPropertySet->getPropertyValue(u"Orientation"_ustr) >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::VERTICAL), nOrientation);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Orientation"_ustr) >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::HORIZONTAL), nOrientation);

    xPropertySet->getPropertyValue(u"VisibleSize"_ustr) >>= nVisibleSize;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nVisibleSize);
}

void SdActiveXControlsTest::testCheckBoxProperties()
{
    createSdImpressDoc("pptx/activex_checkbox.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"CheckBox1"_ustr, sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    bool bMultiLine;
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    sal_Int16 nVisualEffect;
    xPropertySet->getPropertyValue(u"VisualEffect"_ustr) >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::LOOK3D), nVisualEffect);

    bool bTriState;
    xPropertySet->getPropertyValue(u"TriState"_ustr) >>= bTriState;
    CPPUNIT_ASSERT_EQUAL(false, bTriState);

    sal_Int16 nState;
    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Custom Caption"_ustr, sLabel);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFF80FF), nColor);

    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue(u"VisualEffect"_ustr) >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::FLAT), nVisualEffect);

    xPropertySet->getPropertyValue(u"TriState"_ustr) >>= bTriState;
    CPPUNIT_ASSERT_EQUAL(true, bTriState);

    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"TriState"_ustr) >>= bTriState;
    CPPUNIT_ASSERT_EQUAL(true, bTriState);

    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nState);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor);
}

void SdActiveXControlsTest::testOptionButtonProperties()
{
    createSdImpressDoc("pptx/activex_optionbutton.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"OptionButton1"_ustr, sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    bool bMultiLine;
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    sal_Int16 nVisualEffect;
    xPropertySet->getPropertyValue(u"VisualEffect"_ustr) >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::LOOK3D), nVisualEffect);

    sal_Int16 nState;
    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Custom Caption"_ustr, sLabel);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTCYAN, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue(u"VisualEffect"_ustr) >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::FLAT), nVisualEffect);

    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState); // TriState / undefined imported as unchecked

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor);
}

void SdActiveXControlsTest::testComboBoxProperties()
{
    createSdImpressDoc("pptx/activex_combobox.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    bool bAutocomplete;
    xPropertySet->getPropertyValue(u"Autocomplete"_ustr) >>= bAutocomplete;
    CPPUNIT_ASSERT_EQUAL(true, bAutocomplete);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nBorderStyle);

    Color nBorderColor;
    xPropertySet->getPropertyValue(u"BorderColor"_ustr) >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nBorderColor);

    bool bDropdown;
    xPropertySet->getPropertyValue(u"Dropdown"_ustr) >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(true, bDropdown);

    bool bHideInactiveSelection;
    xPropertySet->getPropertyValue(u"HideInactiveSelection"_ustr) >>= bHideInactiveSelection;
    CPPUNIT_ASSERT_EQUAL(true, bHideInactiveSelection);

    sal_Int16 nLineCount;
    xPropertySet->getPropertyValue(u"LineCount"_ustr) >>= nLineCount;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), nLineCount);

    sal_Int16 nMaxTextLen;
    xPropertySet->getPropertyValue(u"MaxTextLen"_ustr) >>= nMaxTextLen;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nMaxTextLen);

    bool bReadOnly;
    xPropertySet->getPropertyValue(u"ReadOnly"_ustr) >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_RED, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    xPropertySet->getPropertyValue(u"Autocomplete"_ustr) >>= bAutocomplete;
    CPPUNIT_ASSERT_EQUAL(true, bAutocomplete);

    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    xPropertySet->getPropertyValue(u"BorderColor"_ustr) >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x404040), nBorderColor);

    xPropertySet->getPropertyValue(u"Dropdown"_ustr) >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(true, bDropdown);

    xPropertySet->getPropertyValue(u"HideInactiveSelection"_ustr) >>= bHideInactiveSelection;
    CPPUNIT_ASSERT_EQUAL(false, bHideInactiveSelection);

    xPropertySet->getPropertyValue(u"LineCount"_ustr) >>= nLineCount;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(12), nLineCount);

    xPropertySet->getPropertyValue(u"MaxTextLen"_ustr) >>= nMaxTextLen;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(130), nMaxTextLen);

    xPropertySet->getPropertyValue(u"ReadOnly"_ustr) >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly); // Bogus, should be true (tdf#111417)

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Autocomplete"_ustr) >>= bAutocomplete;
    CPPUNIT_ASSERT_EQUAL(false, bAutocomplete);

    xPropertySet->getPropertyValue(u"Dropdown"_ustr) >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(false, bDropdown);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor);
}

void SdActiveXControlsTest::testListBoxProperties()
{
    createSdImpressDoc("pptx/activex_listbox.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nBorderStyle);

    Color nBorderColor;
    xPropertySet->getPropertyValue(u"BorderColor"_ustr) >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nBorderColor);

    bool bDropdown;
    xPropertySet->getPropertyValue(u"Dropdown"_ustr) >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(false, bDropdown);

    bool bMultiSelection;
    xPropertySet->getPropertyValue(u"MultiSelection"_ustr) >>= bMultiSelection;
    CPPUNIT_ASSERT_EQUAL(false, bMultiSelection);

    sal_Int16 nLineCount;
    xPropertySet->getPropertyValue(u"LineCount"_ustr) >>= nLineCount;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), nLineCount);

    bool bReadOnly;
    xPropertySet->getPropertyValue(u"ReadOnly"_ustr) >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropertySet->getPropertyValue(u"Border"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    xPropertySet->getPropertyValue(u"BorderColor"_ustr) >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTMAGENTA, nBorderColor);

    xPropertySet->getPropertyValue(u"MultiSelection"_ustr) >>= bMultiSelection;
    CPPUNIT_ASSERT_EQUAL(true, bMultiSelection);

    xPropertySet->getPropertyValue(u"ReadOnly"_ustr) >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly); // Bogus, should be true (tdf#111417)

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"MultiSelection"_ustr) >>= bMultiSelection;
    CPPUNIT_ASSERT_EQUAL(true, bMultiSelection);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);
}

void SdActiveXControlsTest::testToggleButtonProperties()
{
    createSdImpressDoc("pptx/activex_togglebutton.pptx");

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"ToggleButton1"_ustr, sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    Color nColor;
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xECE9D8), nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    bool bToggle;
    xPropertySet->getPropertyValue(u"Toggle"_ustr) >>= bToggle;
    CPPUNIT_ASSERT_EQUAL(true, bToggle);

    sal_Int16 nState;
    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Custom Caption"_ustr, sLabel);

    xPropertySet->getPropertyValue(u"Enabled"_ustr) >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue(u"MultiLine"_ustr) >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFF80FF), nColor);

    xPropertySet->getPropertyValue(u"TextColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_GRAY, nColor);

    xPropertySet->getPropertyValue(u"Toggle"_ustr) >>= bToggle;
    CPPUNIT_ASSERT_EQUAL(true, bToggle);

    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue(u"State"_ustr) >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState); // Undefined state

    xPropertySet->getPropertyValue(u"Align"_ustr) >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nColor);
}

void SdActiveXControlsTest::testPictureProperties()
{
    createSdImpressDoc("pptx/activex_picture.pptx");

    // Different controls has different image positioning

    // Command buttons here with icons
    const std::vector<sal_Int16> vImagePositions =
    {
        awt::ImagePosition::AboveCenter,
        awt::ImagePosition::LeftTop,
        awt::ImagePosition::LeftCenter,
        awt::ImagePosition::LeftBottom,
        awt::ImagePosition::RightTop,
        awt::ImagePosition::RightCenter,
        awt::ImagePosition::RightBottom,
        awt::ImagePosition::AboveLeft,
        awt::ImagePosition::AboveRight,
        awt::ImagePosition::BelowLeft,
        awt::ImagePosition::BelowCenter,
        awt::ImagePosition::BelowRight,
        awt::ImagePosition::Centered,
    };

    for (size_t i = 0; i < vImagePositions.size(); ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(i, 0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

        uno::Reference<graphic::XGraphic> xGraphic;
        xPropertySet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT(xGraphic.is());

        sal_Int16 nColor;
        xPropertySet->getPropertyValue(u"ImagePosition"_ustr) >>= nColor;
        OString sMessage = "The wrong control's index is: " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), vImagePositions[i], nColor);
    }

    // Picture controls with different properties
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(vImagePositions.size() + i, 0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

        OString sMessage = "The wrong control's index is: " + OString::number(i);

        uno::Reference<graphic::XGraphic> xGraphic;
        xPropertySet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        if (i == 0) // First control has no image specified
            CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), !xGraphic.is());
        else
            CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), xGraphic.is());

        sal_Int16 nScaleMode;
        xPropertySet->getPropertyValue(u"ScaleMode"_ustr) >>= nScaleMode;
        if (i == 2) // Stretch mode
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::ImageScaleMode::ANISOTROPIC, nScaleMode);
        else if (i == 3) // Zoom mode
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::ImageScaleMode::ISOTROPIC, nScaleMode);
        else // Clip mode
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::ImageScaleMode::NONE, nScaleMode);
    }

    // Note: LO picture control does not support tiled image and also image positioning
    // When there is no scaling picture positioned to center.
}

void SdActiveXControlsTest::testFontProperties()
{
    createSdImpressDoc("pptx/activex_fontproperties.pptx");

    // Different controls has different font properties

    for (size_t i = 0; i < 8; ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(i, 0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

        OString sMessage = "The wrong control's index is: " + OString::number(i);

        OUString sFontName;
        xPropertySet->getPropertyValue(u"FontName"_ustr) >>= sFontName;
        if (i == 4 || i == 5)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), u"Times New Roman"_ustr, sFontName);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), u"Arial"_ustr, sFontName);

        float fFontWeight;
        xPropertySet->getPropertyValue(u"FontWeight"_ustr) >>= fFontWeight;
        if (i == 2 || i == 4)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::FontWeight::BOLD, fFontWeight);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::FontWeight::NORMAL, fFontWeight);

        sal_Int16 nFontSlant;
        xPropertySet->getPropertyValue(u"FontSlant"_ustr) >>= nFontSlant;
        if (i == 3 || i == 4)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontSlant_ITALIC), nFontSlant);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontSlant_NONE), nFontSlant);

        sal_Int16 nFontUnderline;
        xPropertySet->getPropertyValue(u"FontUnderline"_ustr) >>= nFontUnderline;
        if (i == 7)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontUnderline::SINGLE), nFontUnderline);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontUnderline::NONE), nFontUnderline);

        sal_Int16 nFontStrikeout;
        xPropertySet->getPropertyValue(u"FontStrikeout"_ustr) >>= nFontStrikeout;
        if (i == 6 || i == 7)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontStrikeout::SINGLE), nFontStrikeout);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontStrikeout::NONE), nFontStrikeout);

        float fFontHeight;
        xPropertySet->getPropertyValue(u"FontHeight"_ustr) >>= fFontHeight;
        if (i == 1)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), 24.0f, fFontHeight);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), 14.0f, fFontHeight);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdActiveXControlsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
