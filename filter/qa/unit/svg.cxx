/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

OUStringLiteral const DATA_DIRECTORY = u"/filter/qa/unit/data/";

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
    void load(const OUString& rURL);
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

void SvgFilterTest::load(const OUString& rFileName)
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
    load("preserve-jpg.odt");

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
    load("semi-transparent-line.odg");

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
    OUString aPrefix("opacity: ");
    // Without the accompanying fix in place, this test would have failed, as the style was
    // "mask:url(#mask1)", not "opacity: <value>".
    CPPUNIT_ASSERT(aStyle.startsWith(aPrefix));
    int nPercent = std::round(aStyle.copy(aPrefix.getLength()).toDouble() * 100);
    // Make sure that the line is still 30% opaque, rather than completely invisible.
    CPPUNIT_ASSERT_EQUAL(30, nPercent);
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testSemiTransparentText)
{
    // Two shapes, one with transparent text and the other one with
    // opaque text. We expect both to be exported to the SVG with the
    // correct transparency factor applied for the first shape.

    // Load draw document with transparent text in one box
    load("TransparentText.odg");

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

    // We expect 2 groups of class "com.sun.star.drawing.TextShape" that
    // have some svg:text node inside.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the 2nd shape lots its text.

    assertXPath(pXmlDoc, "//svg:g[@class='com.sun.star.drawing.TextShape']//svg:text", 2);

    // First shape has semi-transparent text.
    assertXPath(pXmlDoc, "//svg:text[1]/svg:tspan/svg:tspan/svg:tspan[@fill-opacity='0.8']");

    // Second shape has normal text.
    assertXPath(pXmlDoc, "//svg:text[2]/svg:tspan/svg:tspan/svg:tspan[@fill-opacity]", 0);
}

CPPUNIT_TEST_FIXTURE(SvgFilterTest, testShapeNographic)
{
    // Load a document containing a 3D shape.
    load("shape-nographic.odp");

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
