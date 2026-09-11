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
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <comphelper/processfactory.hxx>
#include <cool.hpp>
#include <cpo/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <scriptinterop/PageElementType.hpp>
#include <scriptinterop/PageType.hpp>
#include <scriptinterop/PlaceholderType.hpp>
#include <scriptinterop/PredefinedLayout.hpp>
#include <scriptinterop/SlideLinkingMode.hpp>
#include <scriptinterop/XAffineTransform.hpp>
#include <scriptinterop/XAffineTransformBuilder.hpp>
#include <scriptinterop/XLayout.hpp>
#include <scriptinterop/XMaster.hpp>
#include <scriptinterop/XNotesMaster.hpp>
#include <scriptinterop/XNotesPage.hpp>
#include <scriptinterop/XPageElement.hpp>
#include <scriptinterop/XPresentation.hpp>
#include <scriptinterop/XShape.hpp>
#include <scriptinterop/XSlide.hpp>
#include <scriptinterop/XSlideSelection.hpp>
#include <scriptinterop/XTextRange.hpp>
#include <scriptinterop/XTextStyle.hpp>
#include <test/unoapi_test.hxx>

namespace
{
template<typename T> T getValue(css::beans::Optional<T> const & optional) {
    CPPUNIT_ASSERT(optional.IsPresent);
    return optional.Value;
}

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
    cpo::uno::Reference<scriptinterop::XPresentation> loadPresentation()
    {
        mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr);
        cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent,
                                                             cpo::uno::UNO_QUERY_THROW);
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

CPPUNIT_TEST_FIXTURE(Test, testAppendSlideWithPredefinedLayout)
{
    auto const xPresentation = loadPresentation();
    // A predefined layout brings its placeholder shapes with it: a title-and-body slide starts
    // out with the title placeholder and the body placeholder.
    auto const xTitleBody = xPresentation->appendSlideFrom(
        cpo::uno::Any(scriptinterop::PredefinedLayout_TITLE_AND_BODY));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTitleBody->getShapes().getLength());
    auto const xTitleOnly
        = xPresentation->appendSlideFrom(cpo::uno::Any(scriptinterop::PredefinedLayout_TITLE_ONLY));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTitleOnly->getShapes().getLength());
    // The blank predefined layout gives the same empty slide as the no-argument call.
    auto const xBlank
        = xPresentation->appendSlideFrom(cpo::uno::Any(scriptinterop::PredefinedLayout_BLANK));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xBlank->getShapes().getLength());
    // A predefined layout with no matching page layout is rejected.
    CPPUNIT_ASSERT_THROW(
        xPresentation->appendSlideFrom(cpo::uno::Any(scriptinterop::PredefinedLayout_BIG_NUMBER)),
        cpo::uno::RuntimeException);
    // A slide linking mode has no implementation yet, so it is rejected.
    CPPUNIT_ASSERT_THROW(xPresentation->appendSlideLinked(xPresentation->getSlides()[0],
                                                          scriptinterop::SlideLinkingMode_LINKED),
                         cpo::uno::RuntimeException);
    // A layout argument that is not a predefined layout is rejected.
    CPPUNIT_ASSERT_THROW(xPresentation->appendSlideFrom(cpo::uno::Any(u"BLANK"_ustr)),
                         cpo::uno::RuntimeException);
    // The rejected calls left no slide behind.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xPresentation->getSlides().getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertTextBoxDefaultGeometry)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    // A text box inserted without geometry lands at the page's top left corner with the default
    // square size of 236.22 points.
    auto const xShape = xSlide->insertTextBox(u"Hello"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xShape->getText()->asString());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, xShape->getLeft(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, xShape->getTop(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(236.22, getValue(xShape->getWidth()), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(236.22, getValue(xShape->getHeight()), 0.05);
}

CPPUNIT_TEST_FIXTURE(Test, testInsertTextBoxGeometryRoundTrip)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBoxAt(u"Hello"_ustr, 36, 72, 288, 144);
    CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xShape->getText()->asString());
    // The chosen point values convert to whole 1/100 mm, so they round-trip exactly.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0, xShape->getLeft(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(72.0, xShape->getTop(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(288.0, getValue(xShape->getWidth()), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(144.0, getValue(xShape->getHeight()), 0.05);
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
    auto const xShape = xSlide->insertTextBoxAt(u"Styled"_ustr, 36, 36, 288, 72);
    auto const xRange = xShape->getText();
    getValue(xRange->getTextStyle())
        ->setBold(true)
        ->setFontSize(24)
        ->setForegroundColor(cpo::uno::Any(u"#c9211e"_ustr));
    // The formatting lands on the text runs, so a cursor over the text reports it.
    cpo::uno::Reference<css::text::XText> const xText(xShape->getuno(),
                                                      cpo::uno::UNO_QUERY_THROW);
    auto const xCursor = xText->createTextCursor();
    xCursor->gotoStart(false);
    xCursor->gotoEnd(true);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xCursor,
                                                               cpo::uno::UNO_QUERY_THROW);
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
    auto const xShape = xSlide->insertTextBoxAt(u"Styled"_ustr, 36, 36, 288, 72);
    getValue(xShape->getText()->getTextStyle())->setItalic(true)->setStrikethrough(true);
    // The formatting lands on the text runs, so a cursor over the text reports it.
    cpo::uno::Reference<css::text::XText> const xText(xShape->getuno(),
                                                      cpo::uno::UNO_QUERY_THROW);
    auto const xCursor = xText->createTextCursor();
    xCursor->gotoStart(false);
    xCursor->gotoEnd(true);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xCursor,
                                                               cpo::uno::UNO_QUERY_THROW);
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
    auto const xShape = xSlide->insertTextBoxAt(u""_ustr, 36, 36, 288, 72);
    auto const xText = xShape->getText();
    auto const xPlain = xText->appendText(u"plain "_ustr);
    auto const xBold = xText->appendText(u"bold"_ustr);
    getValue(xBold->getTextStyle())->setBold(true);
    CPPUNIT_ASSERT_EQUAL(u"plain bold"_ustr, xText->asString());
    CPPUNIT_ASSERT_EQUAL(u"bold"_ustr, xBold->asString());
    // Styling the returned range covers only that run, so the earlier run stays regular.
    cpo::uno::Reference<css::beans::XPropertySet> const xPlainProps(xPlain->getuno(),
                                                                    cpo::uno::UNO_QUERY_THROW);
    float fWeight = 0;
    xPlainProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(100.0f, fWeight);
    cpo::uno::Reference<css::beans::XPropertySet> const xBoldProps(xBold->getuno(),
                                                                   cpo::uno::UNO_QUERY_THROW);
    xBoldProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(150.0f, fWeight);
    // A run appended after a styled run starts from regular formatting again.
    auto const xAfter = xText->appendText(u" after"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xAfterProps(xAfter->getuno(),
                                                                    cpo::uno::UNO_QUERY_THROW);
    xAfterProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(100.0f, fWeight);
    // Appending an empty string produces an empty range, which has no characters to style.
    CPPUNIT_ASSERT(!xText->appendText(u""_ustr)->getTextStyle().IsPresent);
    // Only the shape's whole text range can append runs.
    CPPUNIT_ASSERT_THROW(xBold->appendText(u"x"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testAppendParagraphAndBulletLevels)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBoxAt(u""_ustr, 36, 36, 288, 144);
    auto const xText = xShape->getText();
    xText->setBulletLevel(0);
    xText->appendText(u"first"_ustr);
    // appendParagraph hands back the paragraph holding the given text; its range covers that
    // text.
    auto const xPara = getValue(xText->appendParagraph(u"second"_ustr)->getRange());
    CPPUNIT_ASSERT_EQUAL(u"second"_ustr, xPara->asString());
    xPara->setBulletLevel(1);
    CPPUNIT_ASSERT_EQUAL(u"first\nsecond"_ustr, xText->asString());
    // Each paragraph carries its own bullet depth, and the bullets show because paragraphs
    // count as bulleted by default once they have a depth.
    cpo::uno::Reference<css::container::XEnumerationAccess> const xParagraphs(
        xText->getuno(), cpo::uno::UNO_QUERY_THROW);
    auto xEnum = xParagraphs->createEnumeration();
    cpo::uno::Reference<css::beans::XPropertySet> xParaProps(xEnum->nextElement(),
                                                             cpo::uno::UNO_QUERY_THROW);
    sal_Int16 nLevel = -1;
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nLevel);
    bool bIsNumber = false;
    xParaProps->getPropertyValue(u"NumberingIsNumber"_ustr) >>= bIsNumber;
    CPPUNIT_ASSERT(bIsNumber);
    xParaProps.set(xEnum->nextElement(), cpo::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nLevel);
    // Level -1 takes the paragraph off the bullet list again; a paragraph off the list reports
    // no numbering level at all.
    xPara->setBulletLevel(-1);
    xEnum = xParagraphs->createEnumeration();
    xEnum->nextElement();
    xParaProps.set(xEnum->nextElement(), cpo::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xParaProps->getPropertyValue(u"NumberingLevel"_ustr).hasValue());
    // Levels outside -1..9 are rejected.
    CPPUNIT_ASSERT_THROW(xText->setBulletLevel(10), cpo::uno::RuntimeException);
    // Setting the level on the whole text puts every paragraph on that depth.
    xText->setBulletLevel(2);
    xEnum = xParagraphs->createEnumeration();
    xParaProps.set(xEnum->nextElement(), cpo::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nLevel);
    xParaProps.set(xEnum->nextElement(), cpo::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nLevel);
    // Appending an empty paragraph gives back a position where later appended text lands, so
    // a depth set on the empty paragraph holds for text appended afterwards.
    auto const xEmpty = getValue(xText->appendParagraph(u""_ustr)->getRange());
    xEmpty->setBulletLevel(3);
    xText->appendText(u"third"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"first\nsecond\nthird"_ustr, xText->asString());
    xEnum = xParagraphs->createEnumeration();
    xEnum->nextElement();
    xEnum->nextElement();
    xParaProps.set(xEnum->nextElement(), cpo::uno::UNO_QUERY_THROW);
    xParaProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), nLevel);
}

CPPUNIT_TEST_FIXTURE(Test, testGeometryValidation)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    auto const xShape = xSlide->insertTextBoxAt(u"x"_ustr, 36, 36, 288, 72);
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
    CPPUNIT_ASSERT_THROW(xSlide->insertTextBoxAt(u"x"_ustr, 0, 0, -10, 10),
                         cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSlide->getShapes().getLength());
    // A rejected setter leaves the shape's geometry untouched.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(36.0, xShape->getLeft(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(288.0, getValue(xShape->getWidth()), 0.05);
}

CPPUNIT_TEST_FIXTURE(Test, testCurrentPageAndRemove)
{
    auto const xPresentation = loadPresentation();
    auto const xCurrent = getValue(getValue(xPresentation->getSelection())->getCurrentPage());
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
    cpo::uno::Reference<css::beans::XPropertySet> const xPageProps(xSlide->getuno(),
                                                                   cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySet> xBackground;
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
    CPPUNIT_ASSERT(
        getValue(getValue(xPresentation->getSelection())->getCurrentPage())->asSlide().is());
    // The notes view reports the notes page as current; the notes page is a page but not a
    // slide.
    dispatchCommand(mxComponent, u".uno:NotesMode"_ustr, {});
    auto const xNotesPage = getValue(getValue(xPresentation->getSelection())->getCurrentPage());
    CPPUNIT_ASSERT(xNotesPage.is());
    CPPUNIT_ASSERT_THROW(xNotesPage->asSlide(), cpo::uno::RuntimeException);
    // Back in the normal drawing view the slide is current again.
    dispatchCommand(mxComponent, u".uno:DrawingMode"_ustr, {});
    CPPUNIT_ASSERT(
        getValue(getValue(xPresentation->getSelection())->getCurrentPage())->asSlide().is());
}

CPPUNIT_TEST_FIXTURE(Test, testUnimplementedMethodReportsItInTheExceptionMessage)
{
    auto const xPresentation = loadPresentation();
    // The facade reaches all the way from the presentation down to a slide obtained through the
    // selection.
    auto const xSlide
        = getValue(getValue(xPresentation->getSelection())->getCurrentPage())->asSlide();
    // A method still awaiting an implementation says so plainly in its exception message.
    try
    {
        xSlide->getBackground();
        CPPUNIT_FAIL("getBackground: expected an exception");
    }
    catch (cpo::uno::RuntimeException const& e)
    {
        // A debug build appends the throw site to the message, so the check looks for the
        // wording rather than the exact end of the string.
        CPPUNIT_ASSERT(e.Message.indexOf(u"not implemented") >= 0);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testAffineTransformBuilder)
{
    auto const xFactory = cool::get(comphelper::getProcessComponentContext());
    // A fresh builder produces the identity transform.
    auto const xIdentity = xFactory->newAffineTransformBuilder()->build();
    CPPUNIT_ASSERT_EQUAL(1.0, xIdentity->getScaleX());
    CPPUNIT_ASSERT_EQUAL(1.0, xIdentity->getScaleY());
    CPPUNIT_ASSERT_EQUAL(0.0, xIdentity->getShearX());
    CPPUNIT_ASSERT_EQUAL(0.0, xIdentity->getShearY());
    CPPUNIT_ASSERT_EQUAL(0.0, xIdentity->getTranslateX());
    CPPUNIT_ASSERT_EQUAL(0.0, xIdentity->getTranslateY());
    // The setters chain, and the built transform carries every coefficient.
    auto const xTransform = xFactory->newAffineTransformBuilder()
                                ->setScaleX(2)
                                ->setScaleY(3)
                                ->setShearX(0.5)
                                ->setShearY(-0.5)
                                ->setTranslateX(10)
                                ->setTranslateY(20)
                                ->build();
    CPPUNIT_ASSERT_EQUAL(2.0, xTransform->getScaleX());
    CPPUNIT_ASSERT_EQUAL(3.0, xTransform->getScaleY());
    CPPUNIT_ASSERT_EQUAL(0.5, xTransform->getShearX());
    CPPUNIT_ASSERT_EQUAL(-0.5, xTransform->getShearY());
    CPPUNIT_ASSERT_EQUAL(10.0, xTransform->getTranslateX());
    CPPUNIT_ASSERT_EQUAL(20.0, xTransform->getTranslateY());
    // toBuilder starts from the transform's coefficients; changing the copy leaves the original
    // transform as it was.
    auto const xCopy = xTransform->toBuilder()->setTranslateX(99)->build();
    CPPUNIT_ASSERT_EQUAL(99.0, xCopy->getTranslateX());
    CPPUNIT_ASSERT_EQUAL(3.0, xCopy->getScaleY());
    CPPUNIT_ASSERT_EQUAL(10.0, xTransform->getTranslateX());
}

CPPUNIT_TEST_FIXTURE(Test, testPageElementsAndTheirTypes)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->getSlides()[0];
    // The fresh slide's two layout placeholders are shapes, so both listings hold them.
    auto const aElements = xSlide->getPageElements();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aElements.getLength());
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageElementType_SHAPE,
                         aElements[0]->getPageElementType());
    auto const xAsShape = aElements[0]->asShape();
    CPPUNIT_ASSERT(xAsShape.is());
    CPPUNIT_ASSERT_EQUAL(aElements[0]->getuno(), xAsShape->getuno());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(xAsShape->getLeft(), aElements[0]->getLeft(), 0.05);
    // A shape is not an image.
    CPPUNIT_ASSERT_THROW(aElements[0]->asImage(), cpo::uno::RuntimeException);
    // An image added through UNO is a page element but not a shape.
    cpo::uno::Reference<css::lang::XMultiServiceFactory> const xFactory(xPresentation->getuno(),
                                                                        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::drawing::XShape> const xImage(
        xFactory->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::drawing::XShapes> const xShapes(xSlide->getuno(),
                                                             cpo::uno::UNO_QUERY_THROW);
    xShapes->add(xImage);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSlide->getPageElements().getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->getShapes().getLength());
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageElementType_IMAGE,
                         xSlide->getPageElements()[2]->getPageElementType());
    // Removing through the element takes the shape off the page.
    xSlide->getPageElements()[2]->remove();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->getPageElements().getLength());
    // An organisation chart placeholder is an embedded object, which the API has no type for.
    cpo::uno::Reference<css::drawing::XShape> const xOrgChart(
        xFactory->createInstance(u"com.sun.star.presentation.OrgChartShape"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    xShapes->add(xOrgChart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSlide->getPageElements().getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->getShapes().getLength());
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageElementType_UNSUPPORTED,
                         xSlide->getPageElements()[2]->getPageElementType());
    xSlide->getPageElements()[2]->remove();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->getPageElements().getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testPlaceholders)
{
    auto const xPresentation = loadPresentation();
    // A title-and-body slide carries a title placeholder and a body placeholder.
    auto const xSlide = xPresentation->appendSlideFrom(
        cpo::uno::Any(scriptinterop::PredefinedLayout_TITLE_AND_BODY));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->getPlaceholders().getLength());
    auto const xTitle = getValue(xSlide->getPlaceholder(scriptinterop::PlaceholderType_TITLE));
    CPPUNIT_ASSERT(xTitle.is());
    cpo::uno::Reference<css::drawing::XShape> const xTitleShape(xTitle->getuno(),
                                                                cpo::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.presentation.TitleTextShape"_ustr,
                         xTitleShape->getShapeType());
    CPPUNIT_ASSERT(xSlide->getPlaceholder(scriptinterop::PlaceholderType_BODY).IsPresent);
    // A placeholder kind the slide does not carry gives back null rather than an error.
    CPPUNIT_ASSERT(!xSlide->getPlaceholder(scriptinterop::PlaceholderType_SUBTITLE).IsPresent);
    // The indexed lookup counts placeholders of one kind from zero.
    CPPUNIT_ASSERT(
        xSlide->getPlaceholderByIndex(scriptinterop::PlaceholderType_TITLE, 0).IsPresent);
    CPPUNIT_ASSERT(
        !xSlide->getPlaceholderByIndex(scriptinterop::PlaceholderType_TITLE, 1).IsPresent);
    CPPUNIT_ASSERT_THROW(xSlide->getPlaceholderByIndex(scriptinterop::PlaceholderType_TITLE, -1),
                         cpo::uno::RuntimeException);
    // A plain text box is not a placeholder.
    xSlide->insertTextBox(u"x"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->getPlaceholders().getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSlide->getShapes().getLength());
    // On the title layout the title is the centered title, and the second placeholder is the
    // subtitle.
    auto const xTitleSlide
        = xPresentation->appendSlideFrom(cpo::uno::Any(scriptinterop::PredefinedLayout_TITLE));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTitleSlide->getPlaceholders().getLength());
    CPPUNIT_ASSERT(
        xTitleSlide->getPlaceholder(scriptinterop::PlaceholderType_CENTERED_TITLE).IsPresent);
    CPPUNIT_ASSERT(!xTitleSlide->getPlaceholder(scriptinterop::PlaceholderType_TITLE).IsPresent);
    CPPUNIT_ASSERT(xTitleSlide->getPlaceholder(scriptinterop::PlaceholderType_SUBTITLE).IsPresent);
}

CPPUNIT_TEST_FIXTURE(Test, testLayoutsAndMasters)
{
    auto const xPresentation = loadPresentation();
    // A fresh presentation has one master page.  It serves as both the layout and the master.
    auto const aLayouts = xPresentation->getLayouts();
    auto const aMasters = xPresentation->getMasters();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aLayouts.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aMasters.getLength());
    CPPUNIT_ASSERT_EQUAL(aMasters[0]->getuno(), aLayouts[0]->getuno());
    auto const xSlide = xPresentation->getSlides()[0];
    auto const xLayout = getValue(xSlide->getLayout());
    CPPUNIT_ASSERT_EQUAL(aLayouts[0]->getuno(), xLayout->getuno());
    CPPUNIT_ASSERT_EQUAL(aMasters[0]->getuno(), xLayout->getMaster()->getuno());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aMasters[0]->getLayouts().getLength());
    // The layout name is the master page's name.
    cpo::uno::Reference<css::container::XNamed> const xNamed(xLayout->getuno(),
                                                             cpo::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xLayout->getLayoutName().isEmpty());
    CPPUNIT_ASSERT_EQUAL(xNamed->getName(), xLayout->getLayoutName());
    // Each page kind reports its own type.
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageType_SLIDE, xSlide->getPageType());
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageType_LAYOUT, xLayout->getPageType());
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageType_MASTER, aMasters[0]->getPageType());
    // The master page lists its own placeholder shapes.
    CPPUNIT_ASSERT(aMasters[0]->getShapes().getLength() > 0);
    // A master page that a slide still uses cannot be removed, an unused one can.
    cpo::uno::Reference<css::drawing::XMasterPagesSupplier> const xSupplier(
        xPresentation->getuno(), cpo::uno::UNO_QUERY_THROW);
    xSupplier->getMasterPages()->insertNewByIndex(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPresentation->getMasters().getLength());
    CPPUNIT_ASSERT_THROW(aMasters[0]->remove(), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPresentation->getMasters().getLength());
    xPresentation->getMasters()[1]->remove();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPresentation->getMasters().getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testCurrentPageInMasterView)
{
    auto const xPresentation = loadPresentation();
    // In the master view the current page is a master page, which is also a layout but not a
    // slide.
    dispatchCommand(mxComponent, u".uno:SlideMasterPage"_ustr, {});
    auto const xPage = getValue(getValue(xPresentation->getSelection())->getCurrentPage());
    CPPUNIT_ASSERT(xPage.is());
    CPPUNIT_ASSERT(xPage->asMaster().is());
    CPPUNIT_ASSERT(xPage->asLayout().is());
    CPPUNIT_ASSERT_THROW(xPage->asSlide(), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_EQUAL(scriptinterop::PageType_MASTER, xPage->getPageType());
    // Back in the normal view the slide is current, and a slide is not a master.
    dispatchCommand(mxComponent, u".uno:CloseMasterView"_ustr, {});
    auto const xSlidePage = getValue(getValue(xPresentation->getSelection())->getCurrentPage());
    CPPUNIT_ASSERT(xSlidePage->asSlide().is());
    CPPUNIT_ASSERT_THROW(xSlidePage->asMaster(), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testNotesPage)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->getSlides()[0];
    auto const xNotes = xSlide->getNotesPage();
    CPPUNIT_ASSERT(xNotes.is());
    // The speaker notes shape holds the notes text.
    auto const xNotesShape = xNotes->getSpeakerNotesShape();
    xNotesShape->getText()->setText(u"Speaker notes here"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Speaker notes here"_ustr, xNotesShape->getText()->asString());
    // Replacing counts each replaced occurrence; the default match ignores case.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xNotes->replaceAllText(u"here"_ustr, u"there"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Speaker notes there"_ustr, xNotesShape->getText()->asString());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         xNotes->replaceAllTextMatchCase(u"SPEAKER"_ustr, u"x"_ustr, true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xNotes->replaceAllTextMatchCase(u"SPEAKER"_ustr,
                                                                       u"Presenter"_ustr, false));
    CPPUNIT_ASSERT_EQUAL(u"Presenter notes there"_ustr, xNotesShape->getText()->asString());
    // An empty search text is rejected.
    CPPUNIT_ASSERT_THROW(xNotes->replaceAllText(u""_ustr, u"x"_ustr), cpo::uno::RuntimeException);
    // The notes master exists, and the notes page is taller than it is wide.
    CPPUNIT_ASSERT(xPresentation->getNotesMaster().is());
    CPPUNIT_ASSERT(xPresentation->getNotesPageWidth() > 0);
    CPPUNIT_ASSERT(xPresentation->getNotesPageHeight() > xPresentation->getNotesPageWidth());
}

CPPUNIT_TEST_FIXTURE(Test, testReplaceAllText)
{
    auto const xPresentation = loadPresentation();
    auto const xSlide = xPresentation->appendSlide();
    xSlide->insertTextBoxAt(u"Hello World"_ustr, 36, 36, 288, 72);
    xSlide->insertTextBoxAt(u"hello again"_ustr, 36, 144, 288, 72);
    xSlide->getNotesPage()->getSpeakerNotesShape()->getText()->setText(u"hello notes"_ustr);
    // The default match ignores case and counts every replaced occurrence on the page.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSlide->replaceAllText(u"hello"_ustr, u"Bye"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Bye World"_ustr, xSlide->getShapes()[0]->getText()->asString());
    CPPUNIT_ASSERT_EQUAL(u"Bye again"_ustr, xSlide->getShapes()[1]->getText()->asString());
    // A case-sensitive match leaves a differently cased word alone.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         xSlide->replaceAllTextMatchCase(u"bye"_ustr, u"x"_ustr, true));
    // The presentation-wide replacement reaches the notes pages too.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         xPresentation->replaceAllText(u"hello"_ustr, u"Farewell"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Farewell notes"_ustr,
                         xSlide->getNotesPage()->getSpeakerNotesShape()->getText()->asString());
}

CPPUNIT_TEST_FIXTURE(Test, testSelectAsCurrentPage)
{
    auto const xPresentation = loadPresentation();
    auto const xSecond = xPresentation->appendSlide();
    xSecond->selectAsCurrentPage();
    CPPUNIT_ASSERT_EQUAL(
        xSecond->getuno(),
        getValue(getValue(xPresentation->getSelection())->getCurrentPage())->getuno());
    // A master page can become current too; the view then shows master pages.
    xPresentation->getMasters()[0]->selectAsCurrentPage();
    CPPUNIT_ASSERT(
        getValue(getValue(xPresentation->getSelection())->getCurrentPage())->asMaster().is());
    // Selecting a slide brings the view back to the slides.
    xPresentation->getSlides()[0]->selectAsCurrentPage();
    CPPUNIT_ASSERT(
        getValue(getValue(xPresentation->getSelection())->getCurrentPage())->asSlide().is());
}

CPPUNIT_TEST_FIXTURE(Test, testDuplicateSkipAndInsertSlide)
{
    auto const xPresentation = loadPresentation();
    auto const xFirst = xPresentation->getSlides()[0];
    xFirst->insertTextBoxAt(u"copy me"_ustr, 36, 36, 288, 72);
    // The duplicate lands right after the original and carries the same shapes.
    auto const xCopy = xFirst->duplicate();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPresentation->getSlides().getLength());
    CPPUNIT_ASSERT_EQUAL(xCopy->getuno(), xPresentation->getSlides()[1]->getuno());
    CPPUNIT_ASSERT_EQUAL(xFirst->getShapes().getLength(), xCopy->getShapes().getLength());
    // A slide shows by default; skipping hides it from the show.
    CPPUNIT_ASSERT(!xCopy->isSkipped());
    xCopy->setSkipped(true);
    CPPUNIT_ASSERT(xCopy->isSkipped());
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xCopy->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    bool bVisible = true;
    xProps->getPropertyValue(u"Visible"_ustr) >>= bVisible;
    CPPUNIT_ASSERT(!bVisible);
    xCopy->setSkipped(false);
    CPPUNIT_ASSERT(!xCopy->isSkipped());
    // A slide that was not created from another presentation is not linked.
    CPPUNIT_ASSERT_EQUAL(scriptinterop::SlideLinkingMode_NOT_LINKED,
                         xCopy->getSlideLinkingMode());
    // Inserting puts a blank slide at the index; inserting with a layout brings its placeholders.
    auto const xInserted = xPresentation->insertSlide(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xPresentation->getSlides().getLength());
    CPPUNIT_ASSERT_EQUAL(xInserted->getuno(), xPresentation->getSlides()[1]->getuno());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xInserted->getShapes().getLength());
    auto const xTitled = xPresentation->insertSlideFrom(
        3, cpo::uno::Any(scriptinterop::PredefinedLayout_TITLE_ONLY));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTitled->getShapes().getLength());
    CPPUNIT_ASSERT_EQUAL(xTitled->getuno(), xPresentation->getSlides()[3]->getuno());
    // An index outside 0..count is rejected and leaves the slides alone.
    CPPUNIT_ASSERT_THROW(xPresentation->insertSlide(-1), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xPresentation->insertSlide(5), cpo::uno::RuntimeException);
    // The drawing layer only inserts a page after an existing one, so a slide before the first
    // one is not implemented yet.
    CPPUNIT_ASSERT_THROW(xPresentation->insertSlide(0), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xPresentation->getSlides().getLength());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
