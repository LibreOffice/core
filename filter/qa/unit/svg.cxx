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

#if !defined MACOSX
char const DATA_DIRECTORY[] = "/filter/qa/unit/data/";
#endif

/// SVG filter tests.
class SvgFilterTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
#if !defined MACOSX
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(const OUString& rURL);
#endif
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

#if !defined MACOSX
void SvgFilterTest::load(const OUString& rFileName)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFileName;
    mxComponent = loadFromDesktop(aURL);
}
#endif

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
    xmlDocPtr pXmlDoc = parseXmlStream(&aStream);
    OUString aAttributeValue = getXPath(pXmlDoc, "//svg:image", "href");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: aAttributeValue.startsWith("data:image/jpeg")
    // i.e. the SVG export result re-compressed the image as PNG, even if the original and the
    // transformed image is the same, so there is no need for that.
    CPPUNIT_ASSERT(aAttributeValue.startsWith("data:image/jpeg"));
#endif
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
