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
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

char const DATA_DIRECTORY[] = "/filter/qa/unit/data/";

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

    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());
    mxDesktop.set(frame::Desktop::create(xComponentContext));
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
    xmlDocPtr pXmlDoc = parseXmlStream(&aStream);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
