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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/util/Color.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <test/xmldocptr.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers sd/source/filter/eppt/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
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

constexpr OUStringLiteral DATA_DIRECTORY = u"/sd/qa/filter/eppt/data/";

CPPUNIT_TEST_FIXTURE(Test, testOOXMLCustomShapeBitmapFill)
{
    // Save the bugdoc to PPT.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "custom-shape-bitmap-fill.pptx";
    getComponent() = loadFromDesktop(aURL);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("MS PowerPoint 97");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    getComponent()->dispose();
    getComponent() = loadFromDesktop(aTempFile.GetURL());

    // Check if the bitmap shape was lost.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: com.sun.star.drawing.GraphicObjectShape
    // - Actual  : com.sun.star.drawing.CustomShape
    // i.e. the custom shape geometry was kept, but the actual bitmap was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GraphicObjectShape"),
                         xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testThemeExport)
{
    // Given a document with a master slide and a theme, lt1 is set to 0x000002:
    uno::Reference<lang::XComponent> xComponent = loadFromDesktop("private:factory/simpress");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap;
    aMap["Name"] <<= OUString("mytheme");
    aMap["ColorSchemeName"] <<= OUString("mycolorscheme");
    uno::Sequence<util::Color> aColorScheme
        = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc };
    aMap["ColorScheme"] <<= aColorScheme;
    uno::Any aTheme(aMap.getAsConstPropertyValueList());
    xMasterPage->setPropertyValue("Theme", aTheme);

    // When exporting to PPTX:
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Impress Office Open XML");
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(aTempFile.GetFileName(), test::OOXML);

    // Then verify that this color is not lost:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "ppt/theme/theme1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//a:clrScheme/a:lt1/a:srgbClr", "val", "000002");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//a:clrScheme/a:lt1/a:srgbClr' number of nodes is incorrect
    // i.e. the RGB color was lost on export.
    xComponent->dispose();
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
