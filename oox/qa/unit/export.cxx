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
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers ooox/source/export/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    utl::TempFile& getTempFile() { return maTempFile; }
    void loadAndSave(const OUString& rURL, const OUString& rFilterName);
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void Test::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXpathCtx);
}

void Test::loadAndSave(const OUString& rURL, const OUString& rFilterName)
{
    mxComponent = loadFromDesktop(rURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= rFilterName;
    maTempFile.EnableKillingFile();
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxComponent->dispose();
    mxComponent.clear();
    // too many DOCX validation errors right now
    if (rFilterName != "Office Open XML Text")
    {
        validate(maTempFile.GetFileName(), test::OOXML);
    }
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/oox/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(Test, testDmlGroupshapePolygon)
{
    // Given a document with a group shape, containing a single polygon child shape:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "dml-groupshape-polygon.docx";

    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure that the group shape, the group shape's child size and the child shape's size
    // match:
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, getTempFile().GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("word/document.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:ext", "cx", "5328360");
    // Without the accompanying fix in place, this test would have failed, the <a:chExt> element was
    // not written.
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:chExt", "cx", "5328360");
    assertXPath(pXmlDoc, "//wps:spPr/a:xfrm/a:ext", "cx", "5328360");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
