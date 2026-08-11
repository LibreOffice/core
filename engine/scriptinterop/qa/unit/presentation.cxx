/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <limits>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <comphelper/processfactory.hxx>
#include <cool.hpp>
#include <rtl/ustring.hxx>
#include <scriptinterop/XPresentation.hpp>
#include <scriptinterop/XShape.hpp>
#include <scriptinterop/XSlide.hpp>
#include <scriptinterop/XSlideSelection.hpp>
#include <scriptinterop/XTextRange.hpp>
#include <scriptinterop/XTextStyle.hpp>
#include <test/unoapi_test.hxx>

namespace
{
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/scriptinterop/qa/unit/data/"_ustr)
    {
    }

protected:
    // Loads a fresh presentation and makes its frame the active one, which is what
    // getActivePresentation resolves against.
    css::uno::Reference<scriptinterop::XPresentation> loadPresentation()
    {
        mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr);
        css::uno::Reference<css::frame::XModel> const xModel(mxComponent,
                                                             css::uno::UNO_QUERY_THROW);
        auto const xDesktop
            = css::frame::Desktop::create(comphelper::getProcessComponentContext());
        xDesktop->setActiveFrame(xModel->getCurrentController()->getFrame());
        return cool::get(comphelper::getProcessComponentContext())->getActivePresentation();
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSlidesAndAppend)
{
    auto const xPresentation = loadPresentation();
    // A fresh presentation has one slide carrying the two default layout placeholders.
    auto const aSlides = xPresentation->getSlides();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSlides.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aSlides[0]->getShapes().getLength());
    // An appended slide is blank.
    auto const xNewSlide = xPresentation->appendSlide();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPresentation->getSlides().getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xNewSlide->getShapes().getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertTextBoxGeometryRoundTrip)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBox(u"Hello"_ustr, 36, 72, 288, 144);
    CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xShape->getText()->asString());
    // The chosen point values convert to whole 1/100 mm, so they round-trip exactly.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0, xShape->getLeft(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(72.0, xShape->getTop(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(288.0, xShape->getWidth(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(144.0, xShape->getHeight(), 0.05);
    xShape->setLeft(90)->setTop(18);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(90.0, xShape->getLeft(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(18.0, xShape->getTop(), 0.05);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSlide->getShapes().getLength());
    xShape->remove();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSlide->getShapes().getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testTextStyling)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBox(u"Styled"_ustr, 36, 36, 288, 72);
    auto const xRange = xShape->getText();
    xRange->getTextStyle()->setBold(true)->setFontSize(24)->setForegroundColor(u"#c9211e"_ustr);
    // The formatting lands on the text runs, so a cursor over the text reports it.
    css::uno::Reference<css::text::XText> const xText(xShape->getuno(),
                                                      css::uno::UNO_QUERY_THROW);
    auto const xCursor = xText->createTextCursor();
    xCursor->gotoStart(false);
    xCursor->gotoEnd(true);
    css::uno::Reference<css::beans::XPropertySet> const xProps(xCursor,
                                                               css::uno::UNO_QUERY_THROW);
    float fWeight = 0;
    xProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(150.0f, fWeight);
    float fHeight = 0;
    xProps->getPropertyValue(u"CharHeight"_ustr) >>= fHeight;
    CPPUNIT_ASSERT_EQUAL(24.0f, fHeight);
    sal_Int32 nColor = 0;
    xProps->getPropertyValue(u"CharColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xc9211e), nColor);
    // setText replaces the range's content.
    xRange->setText(u"Replaced"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Replaced"_ustr, xRange->asString());
}

CPPUNIT_TEST_FIXTURE(Test, testItalicAndStrikethrough)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBox(u"Styled"_ustr, 36, 36, 288, 72);
    xShape->getText()->getTextStyle()->setItalic(true)->setStrikethrough(true);
    // The formatting lands on the text runs, so a cursor over the text reports it.
    css::uno::Reference<css::text::XText> const xText(xShape->getuno(),
                                                      css::uno::UNO_QUERY_THROW);
    auto const xCursor = xText->createTextCursor();
    xCursor->gotoStart(false);
    xCursor->gotoEnd(true);
    css::uno::Reference<css::beans::XPropertySet> const xProps(xCursor,
                                                               css::uno::UNO_QUERY_THROW);
    css::awt::FontSlant eSlant = css::awt::FontSlant_NONE;
    xProps->getPropertyValue(u"CharPosture"_ustr) >>= eSlant;
    CPPUNIT_ASSERT_EQUAL(css::awt::FontSlant_ITALIC, eSlant);
    sal_Int16 nStrikeout = 0;
    xProps->getPropertyValue(u"CharStrikeout"_ustr) >>= nStrikeout;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(css::awt::FontStrikeout::SINGLE), nStrikeout);
}

CPPUNIT_TEST_FIXTURE(Test, testAppendTextRunStyling)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBox(u""_ustr, 36, 36, 288, 72);
    auto const xText = xShape->getText();
    auto const xPlain = xText->appendText(u"plain "_ustr);
    auto const xBold = xText->appendText(u"bold"_ustr);
    xBold->getTextStyle()->setBold(true);
    CPPUNIT_ASSERT_EQUAL(u"plain bold"_ustr, xText->asString());
    CPPUNIT_ASSERT_EQUAL(u"bold"_ustr, xBold->asString());
    // Styling the returned range covers only that run, so the earlier run stays regular.
    css::uno::Reference<css::beans::XPropertySet> const xPlainProps(xPlain->getuno(),
                                                                    css::uno::UNO_QUERY_THROW);
    float fWeight = 0;
    xPlainProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(100.0f, fWeight);
    css::uno::Reference<css::beans::XPropertySet> const xBoldProps(xBold->getuno(),
                                                                   css::uno::UNO_QUERY_THROW);
    xBoldProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(150.0f, fWeight);
    // A run appended after a styled run starts from regular formatting again.
    auto const xAfter = xText->appendText(u" after"_ustr);
    css::uno::Reference<css::beans::XPropertySet> const xAfterProps(xAfter->getuno(),
                                                                    css::uno::UNO_QUERY_THROW);
    xAfterProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(100.0f, fWeight);
    // Appending an empty string produces an empty range, which has no characters to style.
    CPPUNIT_ASSERT_THROW(xText->appendText(u""_ustr)->getTextStyle(),
                         cpo::uno::RuntimeException);
    // Only the shape's whole text range can append runs.
    CPPUNIT_ASSERT_THROW(xBold->appendText(u"x"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testAppendParagraphAndBulletLevels)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBox(u""_ustr, 36, 36, 288, 144);
    auto const xText = xShape->getText();
    xText->setBulletLevel(0);
    xText->appendText(u"first"_ustr);
    // appendParagraph hands back the paragraph holding the given text; its range covers that
    // text.
    auto const xPara = xText->appendParagraph(u"second"_ustr)->getRange();
    CPPUNIT_ASSERT_EQUAL(u"second"_ustr, xPara->asString());
    xPara->setBulletLevel(1);
    CPPUNIT_ASSERT_EQUAL(u"first\nsecond"_ustr, xText->asString());
    // Each paragraph carries its own bullet depth, and the bullets show because paragraphs
    // count as bulleted by default once they have a depth.
    css::uno::Reference<css::container::XEnumerationAccess> const xParagraphs(
        xText->getuno(), css::uno::UNO_QUERY_THROW);
    auto xEnum = xParagraphs->createEnumeration();
    css::uno::Reference<css::beans::XPropertySet> xParaProps(xEnum->nextElement(),
                                                             css::uno::UNO_QUERY_THROW);
    sal_Int16 nLevel = -1;
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nLevel);
    bool bIsNumber = false;
    xParaProps->getPropertyValue(u"NumberingIsNumber"_ustr) >>= bIsNumber;
    CPPUNIT_ASSERT(bIsNumber);
    xParaProps.set(xEnum->nextElement(), css::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nLevel);
    // Level -1 takes the paragraph off the bullet list again; a paragraph off the list reports
    // no numbering level at all.
    xPara->setBulletLevel(-1);
    xEnum = xParagraphs->createEnumeration();
    xEnum->nextElement();
    xParaProps.set(xEnum->nextElement(), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xParaProps->getPropertyValue(u"NumberingLevel"_ustr).hasValue());
    // Levels outside -1..9 are rejected.
    CPPUNIT_ASSERT_THROW(xText->setBulletLevel(10), cpo::uno::RuntimeException);
    // Setting the level on the whole text puts every paragraph on that depth.
    xText->setBulletLevel(2);
    xEnum = xParagraphs->createEnumeration();
    xParaProps.set(xEnum->nextElement(), css::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nLevel);
    xParaProps.set(xEnum->nextElement(), css::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nLevel);
    // Appending an empty paragraph gives back a position where later appended text lands, so
    // a depth set on the empty paragraph holds for text appended afterwards.
    auto const xEmpty = xText->appendParagraph(u""_ustr)->getRange();
    xEmpty->setBulletLevel(3);
    xText->appendText(u"third"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"first\nsecond\nthird"_ustr, xText->asString());
    xEnum = xParagraphs->createEnumeration();
    xEnum->nextElement();
    xEnum->nextElement();
    xParaProps.set(xEnum->nextElement(), css::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), nLevel);
}

CPPUNIT_TEST_FIXTURE(Test, testGeometryValidation)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBox(u"x"_ustr, 36, 36, 288, 72);
    // A geometry value must be a finite number that fits the page coordinate range.
    CPPUNIT_ASSERT_THROW(xShape->setLeft(std::numeric_limits<double>::quiet_NaN()),
                         cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xShape->setTop(std::numeric_limits<double>::infinity()),
                         cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xShape->setLeft(1e12), cpo::uno::RuntimeException);
    // A width or height must not be negative.
    CPPUNIT_ASSERT_THROW(xShape->setWidth(-1), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xShape->setHeight(-1), cpo::uno::RuntimeException);
    // A rejected insertTextBox leaves the slide without the new shape.
    CPPUNIT_ASSERT_THROW(xSlide->insertTextBox(u"x"_ustr, 0, 0, -10, 10),
                         cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSlide->getShapes().getLength());
    // A rejected setter leaves the shape's geometry untouched.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0, xShape->getLeft(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(288.0, xShape->getWidth(), 0.05);
}

CPPUNIT_TEST_FIXTURE(Test, testCurrentPageAndRemove)
{
    auto const xPresentation = loadPresentation();
    auto const xCurrent = xPresentation->getSelection()->getCurrentPage();
    CPPUNIT_ASSERT(xCurrent.is());
    CPPUNIT_ASSERT(xCurrent->asSlide().is());
    auto const xNewSlide = xPresentation->appendSlide();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPresentation->getSlides().getLength());
    xNewSlide->remove();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPresentation->getSlides().getLength());
    // The last slide cannot be removed.
    CPPUNIT_ASSERT_THROW(xPresentation->getSlides()[0]->remove(), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testPageSize)
{
    auto const xPresentation = loadPresentation();
    // A fresh presentation uses the 16:9 screen page, 28000 x 15750 in 1/100 mm, which converts
    // to 793.70 x 446.46 points.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(793.70, xPresentation->getPageWidth(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(446.46, xPresentation->getPageHeight(), 0.05);
}

CPPUNIT_TEST_FIXTURE(Test, testSlideBackgroundColor)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    xSlide->setBackgroundColor(u"#2a6099"_ustr);
    // The raw page reports the fill through its Background property set.
    css::uno::Reference<css::beans::XPropertySet> const xPageProps(xSlide->getuno(),
                                                                   css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::beans::XPropertySet> xBackground;
    xPageProps->getPropertyValue(u"Background"_ustr) >>= xBackground;
    CPPUNIT_ASSERT(xBackground.is());
    sal_Int32 nColor = 0;
    xBackground->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x2a6099), nColor);
    // A malformed color string is rejected.
    CPPUNIT_ASSERT_THROW(xSlide->setBackgroundColor(u"blue"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testCurrentPageOutsideNormalView)
{
    auto const xPresentation = loadPresentation();
    CPPUNIT_ASSERT(xPresentation->getSelection()->getCurrentPage()->asSlide().is());
    // The notes view reports the notes page as current; the notes page is a page but not a
    // slide.
    dispatchCommand(mxComponent, u".uno:NotesMode"_ustr, {});
    auto const xNotesPage = xPresentation->getSelection()->getCurrentPage();
    CPPUNIT_ASSERT(xNotesPage.is());
    CPPUNIT_ASSERT_THROW(xNotesPage->asSlide(), cpo::uno::RuntimeException);
    // Back in the normal drawing view the slide is current again.
    dispatchCommand(mxComponent, u".uno:DrawingMode"_ustr, {});
    CPPUNIT_ASSERT(xPresentation->getSelection()->getCurrentPage()->asSlide().is());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
