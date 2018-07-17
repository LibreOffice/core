/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

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


class SdActiveXControlsTest: public SdModelTestBase
{
public:
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/control_background_color.pptx"), PPTX);

    const std::vector<sal_Int32> vBackgroundColors =
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
        0x000000, // Button Text
        0xD8E4F8, // Inactive Title Bar Text
        0xFFFFFF, // Button Highlight
        0x716F64, // Button Dark Shadow
        0xF1EFE2, // Button Light Shadow
        0x000000, // Tooltip Text
        0xFFFFE1, // Tooltip
        0xFF0000, // Custom red color
    };

    for (size_t i = 0; i < vBackgroundColors.size(); ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(i, 0, xDocShRef), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xControlShape.is());

        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
        sal_Int32 nColor;
        xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
        OString sMessage = "The wrong control's index is: " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), vBackgroundColors[i], nColor);
    }

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testLabelProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_label.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Label1"), sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nBorderStyle);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    style::VerticalAlignment eAlign;
    xPropertySet->getPropertyValue("VerticalAlign") >>= eAlign;
    CPPUNIT_ASSERT_EQUAL(style::VerticalAlignment_TOP, eAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Label"), sLabel);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xE0E0E0), nColor);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000FF), nColor);

    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    sal_Int32 nBorderColor;
    xPropertySet->getPropertyValue("BorderColor") >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00), nBorderColor);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    xPropertySet->getPropertyValue("VerticalAlign") >>= eAlign;
    CPPUNIT_ASSERT_EQUAL(style::VerticalAlignment_TOP, eAlign);

    // Third control has transparent background
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("BackgroundColor") >>= nColor);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testTextBoxProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_textbox.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sText;
    xPropertySet->getPropertyValue("Text") >>= sText;
    CPPUNIT_ASSERT_EQUAL(OUString(), sText);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nBorderStyle);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    style::VerticalAlignment eAlign;
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("VerticalAlign") >>= eAlign);

    bool bHideSelection;
    xPropertySet->getPropertyValue("HideInactiveSelection") >>= bHideSelection;
    CPPUNIT_ASSERT_EQUAL(true, bHideSelection);

    sal_Int16 nMaxLength;
    xPropertySet->getPropertyValue("MaxTextLen") >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nMaxLength);

    sal_Int16 nEchoChar;
    xPropertySet->getPropertyValue("EchoChar") >>= nEchoChar;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nEchoChar);

    bool bHScroll;
    xPropertySet->getPropertyValue("HScroll") >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(false, bHScroll);

    bool bVScroll;
    xPropertySet->getPropertyValue("VScroll") >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(false, bVScroll);

    bool bReadOnly;
    xPropertySet->getPropertyValue("ReadOnly") >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Text") >>= sText;
    CPPUNIT_ASSERT_EQUAL(OUString("Some Text"), sText);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    // These textfields are not multilines in the pptx testfile
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x404040), nColor);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00C000), nColor);

    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("VerticalAlign") >>= eAlign);

    xPropertySet->getPropertyValue("HideInactiveSelection") >>= bHideSelection;
    CPPUNIT_ASSERT_EQUAL(false, bHideSelection);

    xPropertySet->getPropertyValue("MaxTextLen") >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), nMaxLength);

    xPropertySet->getPropertyValue("EchoChar") >>= nEchoChar;
    CPPUNIT_ASSERT_EQUAL(sal_Int16('x'), nEchoChar);

    xPropertySet->getPropertyValue("HScroll") >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(true, bHScroll);

    xPropertySet->getPropertyValue("VScroll") >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(false, bVScroll);

    xPropertySet->getPropertyValue("ReadOnly") >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(true, bReadOnly);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("BackgroundColor") >>= nColor);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    xPropertySet->getPropertyValue("HScroll") >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(false, bHScroll);

    xPropertySet->getPropertyValue("VScroll") >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(true, bVScroll);

    // Fourth shape has both scroll bar
    xControlShape.set(getShapeFromPage(3, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("HScroll") >>= bHScroll;
    CPPUNIT_ASSERT_EQUAL(true, bHScroll);

    xPropertySet->getPropertyValue("VScroll") >>= bVScroll;
    CPPUNIT_ASSERT_EQUAL(true, bVScroll);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testSpinButtonProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_spinbutton.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xECE9D8), nColor);

    sal_Int32 nMax;
    xPropertySet->getPropertyValue("SpinValueMax") >>= nMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nMax);

    sal_Int32 nMin;
    xPropertySet->getPropertyValue("SpinValueMin") >>= nMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nMin);

    sal_Int32 nIncrement;
    xPropertySet->getPropertyValue("SpinIncrement") >>= nIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nIncrement);

    bool bRepeat;
    xPropertySet->getPropertyValue("Repeat") >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(true, bRepeat);

    sal_Int32 nDelay;
    xPropertySet->getPropertyValue("RepeatDelay") >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), nDelay);

    sal_Int32 nArrowColor;
    xPropertySet->getPropertyValue("SymbolColor") >>= nArrowColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nArrowColor);

    sal_Int32 nOrientation;
    xPropertySet->getPropertyValue("Orientation") >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::HORIZONTAL), nOrientation);

    sal_Int32 nSpinValue;
    xPropertySet->getPropertyValue("SpinValue") >>= nSpinValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nSpinValue);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFF00), nColor);

    xPropertySet->getPropertyValue("SpinValueMax") >>= nMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(320), nMax);

    xPropertySet->getPropertyValue("SpinValueMin") >>= nMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nMin);

    xPropertySet->getPropertyValue("SpinIncrement") >>= nIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nIncrement);

    xPropertySet->getPropertyValue("Repeat") >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(true, bRepeat);

    xPropertySet->getPropertyValue("RepeatDelay") >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nDelay);

    xPropertySet->getPropertyValue("SymbolColor") >>= nArrowColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00), nArrowColor);

    xPropertySet->getPropertyValue("Orientation") >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::VERTICAL), nOrientation);

    xPropertySet->getPropertyValue("SpinValue") >>= nSpinValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nSpinValue);

    // Third control has horizontal orientation
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Orientation") >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::HORIZONTAL), nOrientation);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testCommandButtonProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_commandbutton.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("CommandButton1"), sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xECE9D8), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    bool bFocusOnClick;
    xPropertySet->getPropertyValue("FocusOnClick") >>= bFocusOnClick;
    CPPUNIT_ASSERT_EQUAL(true, bFocusOnClick);

    bool bRepeat;
    xPropertySet->getPropertyValue("Repeat") >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(false, bRepeat);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), sLabel);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000FF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFF80), nColor);

    xPropertySet->getPropertyValue("FocusOnClick") >>= bFocusOnClick;
    CPPUNIT_ASSERT_EQUAL(false, bFocusOnClick);

    xPropertySet->getPropertyValue("Repeat") >>= bRepeat;
    CPPUNIT_ASSERT_EQUAL(false, bRepeat);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    // Transparent background
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testScrollBarProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_scrollbar.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xECE9D8), nColor);

    xPropertySet->getPropertyValue("SymbolColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    sal_Int32 nDelay;
    xPropertySet->getPropertyValue("RepeatDelay") >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(50), nDelay);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nBorderStyle);

    sal_Int32 nVisibleSize;
    xPropertySet->getPropertyValue("VisibleSize") >>= nVisibleSize;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nVisibleSize);

    sal_Int32 nScrollValueMin;
    xPropertySet->getPropertyValue("ScrollValueMin") >>= nScrollValueMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nScrollValueMin);

    sal_Int32 nScrollValueMax;
    xPropertySet->getPropertyValue("ScrollValueMax") >>= nScrollValueMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(32767), nScrollValueMax);

    sal_Int32 nScrollValue;
    xPropertySet->getPropertyValue("DefaultScrollValue") >>= nScrollValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nScrollValue);

    sal_Int32 nLineIncrement;
    xPropertySet->getPropertyValue("LineIncrement") >>= nLineIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nLineIncrement);

    sal_Int32 nBlockIncrement;
    xPropertySet->getPropertyValue("BlockIncrement") >>= nBlockIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nBlockIncrement);

    sal_Int32 nOrientation;
    xPropertySet->getPropertyValue("Orientation") >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::VERTICAL), nOrientation);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FFFF), nColor);

    xPropertySet->getPropertyValue("SymbolColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), nColor);

    xPropertySet->getPropertyValue("RepeatDelay") >>= nDelay;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(230), nDelay);

    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nBorderStyle);

    xPropertySet->getPropertyValue("VisibleSize") >>= nVisibleSize;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nVisibleSize);

    xPropertySet->getPropertyValue("ScrollValueMin") >>= nScrollValueMin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), nScrollValueMin);

    xPropertySet->getPropertyValue("ScrollValueMax") >>= nScrollValueMax;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1234567), nScrollValueMax);

    xPropertySet->getPropertyValue("DefaultScrollValue") >>= nScrollValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(125), nScrollValue);

    xPropertySet->getPropertyValue("LineIncrement") >>= nLineIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), nLineIncrement);

    xPropertySet->getPropertyValue("BlockIncrement") >>= nBlockIncrement;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nBlockIncrement);

    xPropertySet->getPropertyValue("Orientation") >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::VERTICAL), nOrientation);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Orientation") >>= nOrientation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(awt::ScrollBarOrientation::HORIZONTAL), nOrientation);

    xPropertySet->getPropertyValue("VisibleSize") >>= nVisibleSize;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nVisibleSize);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testCheckBoxProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_checkbox.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("CheckBox1"), sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    bool bMultiLine;
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    sal_Int16 nVisualEffect;
    xPropertySet->getPropertyValue("VisualEffect") >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::LOOK3D), nVisualEffect);

    bool bTriState;
    xPropertySet->getPropertyValue("TriState") >>= bTriState;
    CPPUNIT_ASSERT_EQUAL(false, bTriState);

    sal_Int16 nState;
    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), sLabel);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF80FF), nColor);

    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue("VisualEffect") >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::FLAT), nVisualEffect);

    xPropertySet->getPropertyValue("TriState") >>= bTriState;
    CPPUNIT_ASSERT_EQUAL(true, bTriState);

    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("TriState") >>= bTriState;
    CPPUNIT_ASSERT_EQUAL(true, bTriState);

    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nState);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("BackgroundColor") >>= nColor);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testOptionButtonProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_optionbutton.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("OptionButton1"), sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    bool bMultiLine;
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    sal_Int16 nVisualEffect;
    xPropertySet->getPropertyValue("VisualEffect") >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::LOOK3D), nVisualEffect);

    sal_Int16 nState;
    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), sLabel);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FFFF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), nColor);

    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue("VisualEffect") >>= nVisualEffect;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::VisualEffect::FLAT), nVisualEffect);

    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState); // TriState / undefined imported as unchecked

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("BackgroundColor") >>= nColor);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testComboBoxProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_combobox.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    bool bAutocomplete;
    xPropertySet->getPropertyValue("Autocomplete") >>= bAutocomplete;
    CPPUNIT_ASSERT_EQUAL(true, bAutocomplete);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nBorderStyle);

    sal_Int32 nBorderColor;
    xPropertySet->getPropertyValue("BorderColor") >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nBorderColor);

    bool bDropdown;
    xPropertySet->getPropertyValue("Dropdown") >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(true, bDropdown);

    bool bHideInactiveSelection;
    xPropertySet->getPropertyValue("HideInactiveSelection") >>= bHideInactiveSelection;
    CPPUNIT_ASSERT_EQUAL(true, bHideInactiveSelection);

    sal_Int16 nLineCount;
    xPropertySet->getPropertyValue("LineCount") >>= nLineCount;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), nLineCount);

    sal_Int16 nMaxTextLen;
    xPropertySet->getPropertyValue("MaxTextLen") >>= nMaxTextLen;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nMaxTextLen);

    bool bReadOnly;
    xPropertySet->getPropertyValue("ReadOnly") >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x800000), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xPropertySet->getPropertyValue("Autocomplete") >>= bAutocomplete;
    CPPUNIT_ASSERT_EQUAL(true, bAutocomplete);

    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    xPropertySet->getPropertyValue("BorderColor") >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x404040), nBorderColor);

    xPropertySet->getPropertyValue("Dropdown") >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(true, bDropdown);

    xPropertySet->getPropertyValue("HideInactiveSelection") >>= bHideInactiveSelection;
    CPPUNIT_ASSERT_EQUAL(false, bHideInactiveSelection);

    xPropertySet->getPropertyValue("LineCount") >>= nLineCount;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(12), nLineCount);

    xPropertySet->getPropertyValue("MaxTextLen") >>= nMaxTextLen;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(130), nMaxTextLen);

    xPropertySet->getPropertyValue("ReadOnly") >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly); // Bogus, should be true (tdf#111417)

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Autocomplete") >>= bAutocomplete;
    CPPUNIT_ASSERT_EQUAL(false, bAutocomplete);

    xPropertySet->getPropertyValue("Dropdown") >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(false, bDropdown);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    CPPUNIT_ASSERT_EQUAL(false, xPropertySet->getPropertyValue("BackgroundColor") >>= nColor);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testListBoxProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_listbox.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    sal_Int16 nBorderStyle;
    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nBorderStyle);

    sal_Int32 nBorderColor;
    xPropertySet->getPropertyValue("BorderColor") >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nBorderColor);

    bool bDropdown;
    xPropertySet->getPropertyValue("Dropdown") >>= bDropdown;
    CPPUNIT_ASSERT_EQUAL(false, bDropdown);

    bool bMultiSelection;
    xPropertySet->getPropertyValue("MultiSelection") >>= bMultiSelection;
    CPPUNIT_ASSERT_EQUAL(false, bMultiSelection);

    sal_Int16 nLineCount;
    xPropertySet->getPropertyValue("LineCount") >>= nLineCount;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), nLineCount);

    bool bReadOnly;
    xPropertySet->getPropertyValue("ReadOnly") >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFF00), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), nColor);

    xPropertySet->getPropertyValue("Border") >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nBorderStyle);

    xPropertySet->getPropertyValue("BorderColor") >>= nBorderColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF00FF), nBorderColor);

    xPropertySet->getPropertyValue("MultiSelection") >>= bMultiSelection;
    CPPUNIT_ASSERT_EQUAL(true, bMultiSelection);

    xPropertySet->getPropertyValue("ReadOnly") >>= bReadOnly;
    CPPUNIT_ASSERT_EQUAL(false, bReadOnly); // Bogus, should be true (tdf#111417)

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("MultiSelection") >>= bMultiSelection;
    CPPUNIT_ASSERT_EQUAL(true, bMultiSelection);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testToggleButtonProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_togglebutton.pptx"), PPTX);

    // First control has default properties
    uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("ToggleButton1"), sLabel);

    bool bEnabled;
    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(true, bEnabled);

    bool bMultiLine;
    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(true, bMultiLine);

    sal_Int32 nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xECE9D8), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), nColor);

    bool bToggle;
    xPropertySet->getPropertyValue("Toggle") >>= bToggle;
    CPPUNIT_ASSERT_EQUAL(true, bToggle);

    sal_Int16 nState;
    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState);

    sal_Int16 nAlign;
    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::CENTER), nAlign);

    // Second control has custom properties
    xControlShape.set(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), sLabel);

    xPropertySet->getPropertyValue("Enabled") >>= bEnabled;
    CPPUNIT_ASSERT_EQUAL(false, bEnabled);

    xPropertySet->getPropertyValue("MultiLine") >>= bMultiLine;
    CPPUNIT_ASSERT_EQUAL(false, bMultiLine);

    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF80FF), nColor);

    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x808080), nColor);

    xPropertySet->getPropertyValue("Toggle") >>= bToggle;
    CPPUNIT_ASSERT_EQUAL(true, bToggle);

    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::LEFT), nAlign);

    // Third shape has some other custom properties
    xControlShape.set(getShapeFromPage(2, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);

    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nState); // Undefined state

    xPropertySet->getPropertyValue("Align") >>= nAlign;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::TextAlign::RIGHT), nAlign);

    // Transparent background
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFFFF), nColor);

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testPictureProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_picture.pptx"), PPTX);

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
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(i, 0, xDocShRef), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xControlShape.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

        uno::Reference<graphic::XGraphic> xGraphic;
        xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
        CPPUNIT_ASSERT(xGraphic.is());

        sal_Int16 nColor;
        xPropertySet->getPropertyValue("ImagePosition") >>= nColor;
        OString sMessage = "The wrong control's index is: " + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), vImagePositions[i], nColor);
    }

    // Picture controls with different properties
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(vImagePositions.size() + i, 0, xDocShRef), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xControlShape.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

        OString sMessage = "The wrong control's index is: " + OString::number(i);

        uno::Reference<graphic::XGraphic> xGraphic;
        xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
        if (i == 0) // First control has no image specified
            CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), !xGraphic.is());
        else
            CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), xGraphic.is());

        sal_Int16 nScaleMode;
        xPropertySet->getPropertyValue("ScaleMode") >>= nScaleMode;
        if (i == 2) // Stretch mode
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::ImageScaleMode::ANISOTROPIC, nScaleMode);
        else if (i == 3) // Zoom mode
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::ImageScaleMode::ISOTROPIC, nScaleMode);
        else // Clip mode
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::ImageScaleMode::NONE, nScaleMode);
    }

    // Note: LO picture control does not support tiled image and also image positioning
    // When there is no scaling picture positioned to center.

    xDocShRef->DoClose();
}

void SdActiveXControlsTest::testFontProperties()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/activex_fontproperties.pptx"), PPTX);

    // Different controls has different font properties

    for (size_t i = 0; i < 8; ++i)
    {
        uno::Reference< drawing::XControlShape > xControlShape(getShapeFromPage(i, 0, xDocShRef), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xControlShape.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

        OString sMessage = "The wrong control's index is: " + OString::number(i);

        OUString sFontName;
        xPropertySet->getPropertyValue("FontName") >>= sFontName;
        if (i == 4 || i == 5)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("Times New Roman"), sFontName);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), OUString("Arial"), sFontName);

        float fFontWeight;
        xPropertySet->getPropertyValue("FontWeight") >>= fFontWeight;
        if (i == 2 || i == 4)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::FontWeight::BOLD, fFontWeight);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), awt::FontWeight::NORMAL, fFontWeight);

        sal_Int16 nFontSlant;
        xPropertySet->getPropertyValue("FontSlant") >>= nFontSlant;
        if (i == 3 || i == 4)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontSlant_ITALIC), nFontSlant);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontSlant_NONE), nFontSlant);

        sal_Int16 nFontUnderline;
        xPropertySet->getPropertyValue("FontUnderline") >>= nFontUnderline;
        if (i == 7)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontUnderline::SINGLE), nFontUnderline);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontUnderline::NONE), nFontUnderline);

        sal_Int16 nFontStrikeout;
        xPropertySet->getPropertyValue("FontStrikeout") >>= nFontStrikeout;
        if (i == 6 || i == 7)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontStrikeout::SINGLE), nFontStrikeout);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), sal_Int16(awt::FontStrikeout::NONE), nFontStrikeout);

        float fFontHeight;
        xPropertySet->getPropertyValue("FontHeight") >>= fFontHeight;
        if (i == 1)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), 24.0f, fFontHeight);
        else
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), 14.0f, fFontHeight);
    }

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdActiveXControlsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
