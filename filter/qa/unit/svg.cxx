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

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
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
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(const OUString& rURL);
    void dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                         const OUString& rCommand,
                         const uno::Sequence<beans::PropertyValue>& rPropertyValues);
};

void SvgFilterTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
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

void SvgFilterTest::dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                                    const OUString& rCommand,
                                    const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
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

    // Make sure the the original JPG data is reused and we don't perform a PNG re-compress.
    xmlDocPtr pXmlDoc = parseXmlStream(&aStream);
    OUString aAttributeValue = getXPath(pXmlDoc, "//svg:image", "href");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: aAttributeValue.startsWith("data:image/jpeg")
    // i.e. the SVG export result re-compressed the image as PNG, even if the original and the
    // transformed image is the same, so there is no need for that.
    CPPUNIT_ASSERT(aAttributeValue.startsWith("data:image/jpeg"));
#endif
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

CPPUNIT_TEST_FIXTURE(SvgFilterTest, attributeRedefinedTest)
{
    // Load document containing empty paragraphs with ids.
    load("attributeRedefinedTest.odp");

    // Export to SVG.
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
    aMediaDescriptor["OutputStream"] <<= xOut;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocPtr pXmlDoc = parseXmlStream(&aStream);

    // We expect four paragraph
    // 2 empty paragraphs with ids
    // 2 paragraphs with text
    // Without the accompanying fix the test would have failed with
    // Expected : 4
    // Actual : 2
    // i.e. 2 of the empty paragraph do not get generated even if there
    // is id imported for the paragraphs
    // If we don't create the empty paragraphs the id attribute attribute gets redefined like this:
    // <tspan id="id14" id="id15" id="id17" class="TextParagraph" font-family="Bahnschrift Light" font-size="1129px" font-weight="400">

    OString xPath = "//svg:g[@class='TextShape']//svg:text[@class='SVGTextShape']//"
                    "svg:tspan[@class='TextParagraph']";
    assertXPath(pXmlDoc, xPath, 4);

    //assert that each tspan element with TextParagraph class has id and the tspan element of
    //each empty paragraph doesnot contain tspan element with class TextPosition
    assertXPath(pXmlDoc, xPath + "[1]", "id", "id4");
    assertXPath(pXmlDoc, xPath + "[2]", "id", "id5");
    assertXPath(pXmlDoc, xPath + "[2]//svg:tspan[@class='TextPosition']", 0);
    assertXPath(pXmlDoc, xPath + "[3]", "id", "id6");
    assertXPath(pXmlDoc, xPath + "[3]//svg:tspan[@class='TextPosition']", 0);
    assertXPath(pXmlDoc, xPath + "[4]", "id", "id7");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
