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
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svx/unopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdomedia.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/xmloff/qa/unit/data/";

/// Covers xmloff/source/draw/ fixes.
class XmloffDrawTest : public test::BootstrapFixture,
                       public unotest::MacrosTest,
                       public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void save(const OUString& rFilterName, utl::TempFile& rTempFile);

    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);

    uno::Reference<beans::XPropertySet>
    getShapeTextPortion(sal_uInt32 nIndex, uno::Reference<drawing::XShape> const& xShape)
    {
        uno::Reference<beans::XPropertySet> xPortion;

        uno::Reference<container::XEnumerationAccess> xEnumAccess(xShape, uno::UNO_QUERY);
        if (!xEnumAccess->hasElements())
            return xPortion;
        uno::Reference<container::XEnumeration> xEnum(xEnumAccess->createEnumeration());
        uno::Reference<text::XTextContent> xTextContent;
        xEnum->nextElement() >>= xTextContent;
        if (!xTextContent.is())
            return xPortion;

        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextContent, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
        sal_uInt32 nCurrent = 0;
        xPortion = uno::Reference<beans::XPropertySet>(xParaEnum->nextElement(), uno::UNO_QUERY);
        while (nIndex != nCurrent)
        {
            ++nCurrent;
            xPortion
                = uno::Reference<beans::XPropertySet>(xParaEnum->nextElement(), uno::UNO_QUERY);
        }
        return xPortion;
    }
};

void XmloffDrawTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmloffDrawTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void XmloffDrawTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
}

uno::Reference<drawing::XShape> XmloffDrawTest::getShape(sal_uInt8 nShapeIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex),
                                           uno::UNO_QUERY_THROW);
    return xShape;
}

void XmloffDrawTest::save(const OUString& rFilterName, utl::TempFile& rTempFile)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= rFilterName;
    rTempFile.EnableKillingFile();
    xStorable->storeToURL(rTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(rTempFile.GetFileName(), test::ODF);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTextBoxLoss)
{
    // Load a document that has a shape with a textbox in it. Save it to ODF and reload.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "textbox-loss.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    getComponent()->dispose();
    getComponent() = loadFromDesktop(aTempFile.GetURL());

    // Make sure that the shape is still a textbox.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    bool bTextBox = false;
    xShape->getPropertyValue("TextBox") >>= bTextBox;

    // Without the accompanying fix in place, this test would have failed, as the shape only had
    // editeng text, losing the image part of the shape text.
    CPPUNIT_ASSERT(bTextBox);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf141301_Extrusion_Angle)
{
    // Load a document that has a custom shape with extrusion direction as set by LO as its default.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141301_Extrusion_Skew.odg";
    getComponent() = loadFromDesktop(aURL, "com.sun.star.comp.drawing.DrawingDocument");

    // Prepare use of XPath
    utl::TempFile aTempFile;
    save("draw8", aTempFile);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("content.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Without fix draw:extrusion-skew="50 -135" was not written to file although "50 -135" is not
    // default in ODF, but only default inside LO.
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-skew", "50 -135");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeExport)
{
    // Create an Impress document which has a master page which has a theme associated with it.
    getComponent() = loadFromDesktop("private:factory/simpress");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap;
    aMap["Name"] <<= OUString("mytheme");
    aMap["ColorSchemeName"] <<= OUString("mycolorscheme");
    uno::Sequence<util::Color> aColorScheme
        = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb };
    aMap["ColorScheme"] <<= aColorScheme;
    uno::Any aTheme = uno::makeAny(aMap.getAsConstPropertyValueList());
    xMasterPage->setPropertyValue("Theme", aTheme);

    // Export to ODP:
    utl::TempFile aTempFile;
    save("impress8", aTempFile);

    // Check if the 12 colors are written in the XML:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "styles.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 12
    // - Actual  : 0
    // - XPath '//style:master-page/loext:theme/loext:color-table/loext:color' number of nodes is incorrect
    // i.e. the theme was lost on exporting to ODF.
    assertXPath(pXmlDoc, "//style:master-page/loext:theme/loext:color-table/loext:color", 12);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testVideoSnapshot)
{
    // Execute ODP import:
    OUString aURL = m_directories.getURLFromSrc(u"xmloff/qa/unit/data/video-snapshot.odp");
    getComponent() = loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(),
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPage.is());
    auto pUnoPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    SdrPage* pSdrPage = pUnoPage->GetSdrPage();
    auto pMedia = dynamic_cast<SdrMediaObj*>(pSdrPage->GetObj(0));

    // Check that the preview was imported:
    const avmedia::MediaItem& rItem = pMedia->getMediaProperties();
    const Graphic& rGraphic = rItem.getGraphic();
    CPPUNIT_ASSERT(!rGraphic.IsNone());

    // Check that the crop was imported:
    const text::GraphicCrop& rCrop = rItem.getCrop();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rCrop.Top);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rCrop.Bottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1356), rCrop.Left);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1356), rCrop.Right);

    // Execute ODP export:
    utl::TempFile aTempFile;
    save("impress8", aTempFile);

    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Check that the preview was exported:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//draw:frame[@draw:style-name='gr1']/draw:image' number of nodes is incorrect
    // i.e. the preview wasn't exported to ODP.
    assertXPath(pXmlDoc, "//draw:frame[@draw:style-name='gr1']/draw:image", "href",
                "Pictures/MediaPreview1.png");
    // Check that the crop was exported:
    assertXPath(pXmlDoc, "//style:style[@style:name='gr1']/style:graphic-properties", "clip",
                "rect(0cm, 1.356cm, 0cm, 1.356cm)");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeImport)
{
    // Given a document that has a master page with a theme associated:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "theme.odp";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure the doc model has a master page with a theme:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterpage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xMasterpage->getPropertyValue("Theme"));
    // Without the accompanying fix in place, this test would have failed with:
    // Cannot extract an Any(void) to string!
    // i.e. the master page had no theme.
    CPPUNIT_ASSERT_EQUAL(OUString("Office Theme"), aMap["Name"].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Office"), aMap["ColorSchemeName"].get<OUString>());
    auto aColorScheme = aMap["ColorScheme"].get<uno::Sequence<util::Color>>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12), aColorScheme.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<util::Color>(0x954F72), aColorScheme[11]);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeColorExportImport)
{
    // Given a document that refers to a theme color:
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "Reference-ThemeColors-TextAndFill.pptx";
    getComponent() = loadFromDesktop(aURL);
    utl::TempFile aTempFile;
    save("impress8", aTempFile);

    // Make sure the export result has the theme reference:
    {
        std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
        xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

        // Text color
        OString aStyle1("//style:style[@style:name='T2']/style:text-properties/"
                        "loext:char-color-theme-reference");
        assertXPath(pXmlDoc, aStyle1, "type", "accent3");
        assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "type", "lummod");
        assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "value", "2000");
        assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "type", "lumoff");
        assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "value", "8000");

        OString aStyle2("//style:style[@style:name='T3']/style:text-properties/"
                        "loext:char-color-theme-reference");
        assertXPath(pXmlDoc, aStyle2, "type", "accent3");
        assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "type", "lummod");
        assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "value", "6000");
        assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "type", "lumoff");
        assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "value", "4000");

        OString aStyle3("//style:style[@style:name='T4']/style:text-properties/"
                        "loext:char-color-theme-reference");
        assertXPath(pXmlDoc, aStyle3, "type", "accent3");
        assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "type", "lummod");
        assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "value", "5000");

        // Shapes fill color
        OString aShape1("//style:style[@style:name='gr1']/style:graphic-properties/"
                        "loext:fill-color-theme-reference");
        assertXPath(pXmlDoc, aShape1, "type", "accent2");
        assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "type", "lummod");
        assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "value", "2000");
        assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "type", "lumoff");
        assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "value", "8000");

        OString aShape2("//style:style[@style:name='gr2']/style:graphic-properties/"
                        "loext:fill-color-theme-reference");
        assertXPath(pXmlDoc, aShape2, "type", "accent2");
        assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "type", "lummod");
        assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "value", "6000");
        assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "type", "lumoff");
        assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "value", "4000");

        OString aShape3("//style:style[@style:name='gr3']/style:graphic-properties/"
                        "loext:fill-color-theme-reference");
        assertXPath(pXmlDoc, aShape3, "type", "accent2");
        assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "type", "lummod");
        assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "value", "5000");
    }

    // reload
    getComponent()->dispose();
    getComponent() = loadFromDesktop(aURL);

    // check fill color theme
    {
        uno::Reference<drawing::XShape> xShape(getShape(0));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XThemeColor> xThemeColor;
        xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(8000), rTrans2.mnValue);
    }
    {
        uno::Reference<drawing::XShape> xShape(getShape(1));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XThemeColor> xThemeColor;
        xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans2.mnValue);
    }
    {
        uno::Reference<drawing::XShape> xShape(getShape(2));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XThemeColor> xThemeColor;
        xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTrans1.mnValue);
    }

    // Char color theme
    // Shape 4
    {
        // Check the first text portion properties
        uno::Reference<drawing::XShape> xShape(getShape(3));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xPortion = getShapeTextPortion(0, xShape);
        CPPUNIT_ASSERT(xPortion.is());
        uno::Reference<util::XThemeColor> xThemeColor;
        xPortion->getPropertyValue("CharColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(8000), rTrans2.mnValue);
    }
    // Shape 5
    {
        // Check the first text portion properties
        uno::Reference<drawing::XShape> xShape(getShape(4));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xPortion = getShapeTextPortion(0, xShape);
        CPPUNIT_ASSERT(xPortion.is());
        uno::Reference<util::XThemeColor> xThemeColor;
        xPortion->getPropertyValue("CharColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans2.mnValue);
    }
    // Shape 6
    {
        // Check the first text portion properties
        uno::Reference<drawing::XShape> xShape(getShape(5));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xPortion = getShapeTextPortion(0, xShape);
        CPPUNIT_ASSERT(xPortion.is());
        uno::Reference<util::XThemeColor> xThemeColor;
        xPortion->getPropertyValue("CharColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTrans1.mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeColor_ShapeFill)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ReferenceShapeFill.pptx";
    getComponent() = loadFromDesktop(aURL);
    utl::TempFile aTempFile;
    save("impress8", aTempFile);

    // reload
    getComponent()->dispose();
    getComponent() = loadFromDesktop(aURL);

    // check fill color theme
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    uno::Reference<util::XThemeColor> xThemeColor;
    xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
    CPPUNIT_ASSERT(xThemeColor.is());
    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aThemeColor.getType());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aThemeColor.getTransformations().size());
    CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                         aThemeColor.getTransformations()[0].meType);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7500), aThemeColor.getTransformations()[0].mnValue);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
