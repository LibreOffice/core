/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapixml_test.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <unotools/streamwrap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/stream.hxx>

using namespace ::com::sun::star;

/// SVG filter tests.
class SvgFilterTest : public UnoApiXmlTest
{
public:
    SvgFilterTest();
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
};

SvgFilterTest::SvgFilterTest()
    : UnoApiXmlTest("/filter/qa/unit/data/")
{
}

void SvgFilterTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("svg"), BAD_CAST("http://www.w3.org/2000/svg"));
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testPreserveJpg)
{
#if !defined(MACOSX)
    // Load a document with a jpeg image in it.
    loadFromURL(u"preserve-jpg.odt");

    // Select the image.
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});

    // Export the selection to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_svg_Export");
    aMediaDescriptor["SelectionOnly"] <<= true;
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Make sure that the original JPG data is reused and we don't perform a PNG re-compress.
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    OUString aAttributeValue = getXPath(pXmlDoc, "//svg:image", "href");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: aAttributeValue.startsWith("data:image/jpeg")
    // i.e. the SVG export result re-compressed the image as PNG, even if the original and the
    // transformed image is the same, so there is no need for that.
    CPPUNIT_ASSERT(aAttributeValue.startsWith("data:image/jpeg"));
#endif
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testSemiTransparentLine)
{
    // Load a document with a semi-transparent line shape.
    loadFromURL(u"semi-transparent-line.odg");

    // Export it to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Get the style of the group around the actual <path> element.
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    OUString aStyle = getXPath(
        pXmlDoc, "//svg:g[@class='com.sun.star.drawing.LineShape']/svg:g/svg:g", "style");
    // Without the accompanying fix in place, this test would have failed, as the style was
    // "mask:url(#mask1)", not "opacity: <value>".
    CPPUNIT_ASSERT(aStyle.startsWith("opacity: ", &aStyle));
    int nPercent = std::round(aStyle.toDouble() * 100);
    // Make sure that the line is still 30% opaque, rather than completely invisible.
    CPPUNIT_ASSERT_EQUAL(30, nPercent);
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testSemiTransparentFillWithTransparentLine)
{
    // Load a document with a shape with semi-transparent fill and line
    loadFromURL(u"semi-transparent-fill.odg");

    // Export it to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Get the style of the group around the actual <path> element.
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    OUString aStyle = getXPath(
        pXmlDoc, "//svg:g[@class='com.sun.star.drawing.EllipseShape']/svg:g/svg:g", "style");
    CPPUNIT_ASSERT(aStyle.startsWith("opacity: ", &aStyle));
    int nPercent = std::round(aStyle.toDouble() * 100);
    // Make sure that the line is still 50% opaque
    CPPUNIT_ASSERT_EQUAL(50, nPercent);

    // Get the stroke of the fill of the EllipseShape (it must be "none")
    OUString aStroke = getXPath(
        pXmlDoc, "//svg:g[@class='com.sun.star.drawing.EllipseShape']/svg:g/svg:path", "stroke");
    // Without the accompanying fix in place, this test would have failed, as the stroke was
    // "rgb(255,255,255)", not "none".
    CPPUNIT_ASSERT_EQUAL(OUString("none"), aStroke);
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testSemiTransparentText)
{
    // Two shapes, one with transparent text and the other one with
    // opaque text. We expect both to be exported to the SVG with the
    // correct transparency factor applied for the first shape.

    // Load draw document with transparent text in one box
    loadFromURL(u"TransparentText.odg");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);

    // We expect 2 groups of class "TextShape" that
    // have some svg:text node inside.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the 2nd shape lots its text.

    assertXPath(pXmlDoc, "//svg:g[@class='TextShape']//svg:text", 2);

    // First shape has semi-transparent text.
    assertXPath(pXmlDoc, "//svg:text[1]/svg:tspan/svg:tspan/svg:tspan[@fill-opacity='0.8']");

    // Second shape has normal text.
    assertXPath(pXmlDoc, "//svg:text[2]/svg:tspan/svg:tspan/svg:tspan[@fill-opacity]", 0);
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testSemiTransparentMultiParaText)
{
    // Given a shape with semi-transparent, multi-paragraph text:
    mxComponent
        = loadFromDesktop("private:factory/simpress", "com.sun.star.drawing.DrawingDocument");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.TextShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                               uno::UNO_QUERY);
    xDrawPage->add(xShape);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<text::XSimpleText> xShapeText(xShape, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xCursor = xShapeText->createTextCursor();
    xShapeText->insertString(xCursor, "foo", /*bAbsorb=*/false);
    xShapeText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                       /*bAbsorb=*/false);
    xShapeText->insertString(xCursor, "bar", /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("CharColor", uno::Any(static_cast<sal_Int32>(0xff0000)));
    xShapeProps->setPropertyValue("CharTransparence", uno::Any(static_cast<sal_Int16>(20)));

    // When exporting to SVG:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Then make sure that the two semi-transparent paragraphs have the same X position:
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    assertXPath(pXmlDoc, "(//svg:g[@class='TextShape']//svg:tspan[@class='TextPosition'])[1]", "x",
                "250");
    assertXPath(pXmlDoc,
                "(//svg:g[@class='TextShape']//svg:tspan[@class='TextPosition'])[1]/svg:tspan",
                "fill-opacity", "0.8");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 250
    // - Actual  : 8819
    // i.e. the X position of the second paragraph was wrong.
    assertXPath(pXmlDoc, "(//svg:g[@class='TextShape']//svg:tspan[@class='TextPosition'])[2]", "x",
                "250");
    assertXPath(pXmlDoc,
                "(//svg:g[@class='TextShape']//svg:tspan[@class='TextPosition'])[2]/svg:tspan",
                "fill-opacity", "0.8");
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testShapeNographic)
{
    // Load a document containing a 3D shape.
    loadFromURL(u"shape-nographic.odp");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;

    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.io.IOException
    // - SfxBaseModel::impl_store <private:stream> failed: 0xc10(Error Area:Io Class:Write Code:16)
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testCustomBullet)
{
    // Given a presentation with a custom bullet:
    loadFromURL(u"custom-bullet.fodp");

    // When exporting that to SVG:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure the bullet glyph is not lost:
    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//svg:g[@class='BulletChars']//svg:path' number of nodes is incorrect
    // i.e. the custom bullet used '<use transform="scale(285,285)"
    // xlink:href="#bullet-char-template-45"/>', but nobody produced a bullet-char-template-45,
    // instead we need the path of the glyph inline.
    CPPUNIT_ASSERT(!getXPath(pXmlDoc, "//svg:g[@class='BulletChars']//svg:path", "d").isEmpty());
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, attributeRedefinedTest)
{
    // Load document containing empty paragraphs with ids.
    loadFromURL(u"attributeRedefinedTest.odp");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);

    // We expect four paragraph
    // 2 empty paragraphs with ids
    // 2 paragraphs with text
    // Without the accompanying fix the test would have failed with
    // Expected : 4
    // Actual : 2
    // i.e. 2 of the empty paragraph do not get generated even if there
    // is id imported for the paragraphs
    // If we don't create the empty paragraphs the id attribute gets redefined like this:
    // <tspan id="id14" id="id15" id="id17" class="TextParagraph" font-family="Bahnschrift Light" font-size="1129px" font-weight="400">

    OString xPath = "//svg:g[@class='TextShape']//svg:text[@class='SVGTextShape']//"
                    "svg:tspan[@class='TextParagraph']";
    assertXPath(pXmlDoc, xPath, 4);

    //assert that each tspan element with TextParagraph class has id and the tspan element of
    //each empty paragraph does not contain tspan element with class TextPosition
    assertXPath(pXmlDoc, xPath + "[1]", "id", "id4");
    assertXPath(pXmlDoc, xPath + "[2]", "id", "id5");
    assertXPath(pXmlDoc, xPath + "[2]//svg:tspan[@class='TextPosition']", 0);
    assertXPath(pXmlDoc, xPath + "[3]", "id", "id6");
    assertXPath(pXmlDoc, xPath + "[3]//svg:tspan[@class='TextPosition']", 0);
    assertXPath(pXmlDoc, xPath + "[4]", "id", "id7");
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testTab)
{
    // Given a shape with "A\tB" text:
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.TextShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                               uno::UNO_QUERY);
    xDrawPage->add(xShape);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    xShapeText->setString("A\tB");

    // When exporting that document to SVG:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure the tab is not lost:
    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the 2nd text portion was not positioned, which looked as if the tab is lost.
    assertXPath(pXmlDoc, "//svg:g[@class='TextShape']//svg:tspan[@class='TextPosition']", 2);
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, textInImage)
{
    // Load document containing empty paragraphs with ids.
    loadFromURL(u"text-in-image.odp");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);

    // We expect the Graphic to have an image and a text
    assertXPath(pXmlDoc, "//svg:g[@class='Graphic']//svg:image", 1);
    assertXPath(pXmlDoc, "//svg:g[@class='Graphic']//svg:text", 1);
    // Without the accomanying fix, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
