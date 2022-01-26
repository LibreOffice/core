/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <unotools/streamwrap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/stream.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/filter/qa/unit/data/";

/// SVG filter tests.
class SvgFilterTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(std::u16string_view rURL);
};

void SvgFilterTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SvgFilterTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void SvgFilterTest::load(std::u16string_view rFileName)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFileName;
    mxComponent = loadFromDesktop(aURL);
}

void SvgFilterTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("svg"), BAD_CAST("http://www.w3.org/2000/svg"));
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testPreserveJpg)
{
#if !defined(MACOSX)
    // Load a document with a jpeg image in it.
    load(u"preserve-jpg.odt");

    // Select the image.
    dispatchCommand(getComponent(), ".uno:JumpToNextFrame", {});

    // Export the selection to SVG.
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);
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
    load(u"semi-transparent-line.odg");

    // Export it to SVG.
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);
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

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testSemiTransparentText)
{
    // Two shapes, one with transparent text and the other one with
    // opaque text. We expect both to be exported to the SVG with the
    // correct transparency factor applied for the first shape.

    // Load draw document with transparent text in one box
    load(u"TransparentText.odg");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);

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

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testShapeNographic)
{
    // Load a document containing a 3D shape.
    load(u"shape-nographic.odp");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);
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
    load(u"custom-bullet.fodp");

    // When exporting that to SVG:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);
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
    load(u"attributeRedefinedTest.odp");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
