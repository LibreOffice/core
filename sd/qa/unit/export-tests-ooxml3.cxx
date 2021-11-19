
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <officecfg/Office/Common.hxx>
#include "sdmodeltestbase.hxx"
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdotable.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdoole2.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <sdpage.hxx>
#include <cfloat>
#include <cstdlib>

#include <rtl/character.hxx>

class SdOOXMLExportTest3 : public SdModelTestBaseXML
{
public:
    void testTdf129430();
    void testTdf114848();
    void testTdf68759();
    void testTdf127901();
    void testTdf48735();
    void testTdf90626();
    void testTdf107608();
    void testTdf111786();
    void testFontScale();
    void testShapeAutofitPPTX();
    void testLegacyShapeAutofitPPTX();
    void testTdf115394();
    void testTdf115394Zero();
    void testTdf115005();
    int testTdf115005_FallBack_Images(bool bAddReplacementImages);
    void testTdf115005_FallBack_Images_On();
    void testTdf115005_FallBack_Images_Off();
    void testTdf118806();
    void testTdf130058();
    void testTdf111789();
    void testTdf100348_convert_Fontwork2TextWarp();
    void testTdf1225573_FontWorkScaleX();
    void testTdf99497_keepAppearanceOfCircleKind();
    /// SmartArt animated elements
    void testTdf104792();
    void testTdf90627();
    void testTdf104786();
    void testTdf118783();
    void testTdf104789();
    void testOpenDocumentAsReadOnly();
    void testTdf118835();
    void testTdf118768();
    void testTdf118836();
    void testTdf116350TextEffects();
    void testTdf128096();
    void testTdf120573();
    void testTdf119118();
    void testTdf99213();
    void testPotxExport();
    void testTdf44223();
    void testSmartArtPreserve();
    void testTdf125346();
    void testTdf125346_2();
    void testTdf125360();
    void testTdf125360_1();
    void testTdf125360_2();
    void testTdf125551();
    void testTdf136830();
    void testTdf126234();
    void testTdf126741();
    void testTdf127372();
    void testTdf127379();
    void testTdf98603();
    void testTdf79082();
    void testTdf128213();
    void testTdf129372();
    void testShapeGlowEffect();
    void testTdf119087();
    void testTdf131554();
    void testTdf132282();
    void testTdf132201EffectOrder();
    void testShapeSoftEdgeEffect();
    void testShapeShadowBlurEffect();
    void testTdf119223();
    void testTdf128213ShapeRot();
    void testTdf125560_textDeflate();
    void testTdf125560_textInflateTop();
    void testTdf96061_textHighlight();
    void testTdf143222_embeddedWorksheet();
    void testTdf142235_TestPlaceholderTextAlignment();
    void testTdf143315();
    void testTdf140912_PicturePlaceholder();

    CPPUNIT_TEST_SUITE(SdOOXMLExportTest3);

    CPPUNIT_TEST(testTdf129430);
    CPPUNIT_TEST(testTdf114848);
    CPPUNIT_TEST(testTdf68759);
    CPPUNIT_TEST(testTdf127901);
    CPPUNIT_TEST(testTdf48735);
    CPPUNIT_TEST(testTdf90626);
    CPPUNIT_TEST(testTdf107608);
    CPPUNIT_TEST(testTdf111786);
    CPPUNIT_TEST(testFontScale);
    CPPUNIT_TEST(testShapeAutofitPPTX);
    CPPUNIT_TEST(testLegacyShapeAutofitPPTX);
    CPPUNIT_TEST(testTdf115394);
    CPPUNIT_TEST(testTdf115394Zero);
    CPPUNIT_TEST(testTdf115005);
    CPPUNIT_TEST(testTdf115005_FallBack_Images_On);
    CPPUNIT_TEST(testTdf115005_FallBack_Images_Off);
    CPPUNIT_TEST(testTdf118806);
    CPPUNIT_TEST(testTdf130058);
    CPPUNIT_TEST(testTdf111789);
    CPPUNIT_TEST(testTdf100348_convert_Fontwork2TextWarp);
    CPPUNIT_TEST(testTdf1225573_FontWorkScaleX);
    CPPUNIT_TEST(testTdf99497_keepAppearanceOfCircleKind);
    CPPUNIT_TEST(testTdf104792);
    CPPUNIT_TEST(testTdf90627);
    CPPUNIT_TEST(testTdf104786);
    CPPUNIT_TEST(testTdf118783);
    CPPUNIT_TEST(testTdf104789);
    CPPUNIT_TEST(testOpenDocumentAsReadOnly);
    CPPUNIT_TEST(testTdf118835);
    CPPUNIT_TEST(testTdf118768);
    CPPUNIT_TEST(testTdf118836);
    CPPUNIT_TEST(testTdf116350TextEffects);
    CPPUNIT_TEST(testTdf128096);
    CPPUNIT_TEST(testTdf120573);
    CPPUNIT_TEST(testTdf119118);
    CPPUNIT_TEST(testTdf99213);
    CPPUNIT_TEST(testPotxExport);
    CPPUNIT_TEST(testTdf44223);
    CPPUNIT_TEST(testSmartArtPreserve);
    CPPUNIT_TEST(testTdf125346);
    CPPUNIT_TEST(testTdf125346_2);
    CPPUNIT_TEST(testTdf125360);
    CPPUNIT_TEST(testTdf125360_1);
    CPPUNIT_TEST(testTdf125360_2);
    CPPUNIT_TEST(testTdf125551);
    CPPUNIT_TEST(testTdf136830);
    CPPUNIT_TEST(testTdf126234);
    CPPUNIT_TEST(testTdf126741);
    CPPUNIT_TEST(testTdf127372);
    CPPUNIT_TEST(testTdf127379);
    CPPUNIT_TEST(testTdf98603);
    CPPUNIT_TEST(testTdf79082);
    CPPUNIT_TEST(testTdf128213);
    CPPUNIT_TEST(testTdf129372);
    CPPUNIT_TEST(testShapeGlowEffect);
    CPPUNIT_TEST(testTdf119087);
    CPPUNIT_TEST(testTdf131554);
    CPPUNIT_TEST(testTdf132282);
    CPPUNIT_TEST(testTdf132201EffectOrder);
    CPPUNIT_TEST(testShapeSoftEdgeEffect);
    CPPUNIT_TEST(testShapeShadowBlurEffect);
    CPPUNIT_TEST(testTdf119223);
    CPPUNIT_TEST(testTdf128213ShapeRot);
    CPPUNIT_TEST(testTdf125560_textDeflate);
    CPPUNIT_TEST(testTdf125560_textInflateTop);
    CPPUNIT_TEST(testTdf96061_textHighlight);
    CPPUNIT_TEST(testTdf143222_embeddedWorksheet);
    CPPUNIT_TEST(testTdf142235_TestPlaceholderTextAlignment);
    CPPUNIT_TEST(testTdf143315);
    CPPUNIT_TEST(testTdf140912_PicturePlaceholder);
    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
        XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    }
};

void SdOOXMLExportTest3::testTdf129430()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf129430.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:pPr/a:lnSpc/a:spcPct",
                "val", "100000");
}

void SdOOXMLExportTest3::testTdf114848()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf114848.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocTheme1 = parseExport(tempFile, "ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val",
                "1f497d");
    xmlDocUniquePtr pXmlDocTheme2 = parseExport(tempFile, "ppt/theme/theme2.xml");
    assertXPath(pXmlDocTheme2, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val",
                "1f497d");
}

void SdOOXMLExportTest3::testTdf68759()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf68759.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off", "x",
                "1687320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off", "y",
                "1615320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm", "rot", "9600000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off", "x",
                "3847320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off", "y",
                "1614600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm", "flipH", "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off", "x",
                "5934960");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off", "y",
                "1615320");
}

void SdOOXMLExportTest3::testTdf127901()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf127901.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "bright",
                "70000");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "contrast",
                "-70000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:grayscl", 1);

    xmlDocUniquePtr pXmlDocContent3 = parseExport(tempFile, "ppt/slides/slide3.xml");
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:biLevel",
                "thresh", "50000");
}

void SdOOXMLExportTest3::testTdf48735()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf48735.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "b", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "l", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "r", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "t", "18842");
}

void SdOOXMLExportTest3::testTdf90626()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf90626.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buSzPct",
                "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buSzPct",
                "val", "150142");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buSzPct",
                "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr/a:buSzPct",
                "val", "150142");
}

void SdOOXMLExportTest3::testTdf107608()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf107608.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, aFillStyle);

    bool bBackgroundFill = false;
    xPropSet->getPropertyValue("FillBackground") >>= bBackgroundFill;
    CPPUNIT_ASSERT(bBackgroundFill);

    Color nBackgroundColor;
    xPropSet->getPropertyValue("FillColor") >>= nBackgroundColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), nBackgroundColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf111786()
{
    // Export line transparency with the color
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111786.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    Color nLineColor;
    xPropSet->getPropertyValue("LineColor") >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x3465A4), nLineColor);

    sal_Int16 nTransparency;
    xPropSet->getPropertyValue("LineTransparence") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(33), nTransparency);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testFontScale()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/font-scale.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");

    // Rounding errors possible, approximate value (+/- 1%)
    OUString sScale = getXPath(
        pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:normAutofit", "fontScale");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(76000), sScale.toInt32(), 1000);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testShapeAutofitPPTX()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/testShapeAutofit.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // TextAutoGrowHeight --> "Resize shape to fit text" --> true
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:spAutoFit", 1);
    // TextAutoGrowHeight --> "Resize shape to fit text" --> false
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

void SdOOXMLExportTest3::testLegacyShapeAutofitPPTX()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/testLegacyShapeAutofit.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // Text in a legacy rectangle
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:noAutofit", 1);
    // Text in (closed) Polygon
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
    // Text in a legacy ellipse
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

void SdOOXMLExportTest3::testTdf115394()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf115394.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    double fTransitionDuration;

    // Slow in MS formats
    SdPage* pPage1 = xDocShRef->GetDoc()->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);

    // Medium in MS formats
    SdPage* pPage2 = xDocShRef->GetDoc()->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Fast in MS formats
    SdPage* pPage3 = xDocShRef->GetDoc()->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Custom values
    SdPage* pPage4 = xDocShRef->GetDoc()->GetSdPage(3, PageKind::Standard);
    fTransitionDuration = pPage4->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.25, fTransitionDuration);

    SdPage* pPage5 = xDocShRef->GetDoc()->GetSdPage(4, PageKind::Standard);
    fTransitionDuration = pPage5->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(4.25, fTransitionDuration);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf115394Zero()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf115394-zero.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    double fTransitionDuration;

    SdPage* pPage = xDocShRef->GetDoc()->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.01, fTransitionDuration);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf115005()
{
    sd::DrawDocShellRef xDocShRefOriginal
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf115005.odp"), ODP);
    utl::TempFile tempFile;
    sd::DrawDocShellRef xDocShRefResaved = saveAndReload(xDocShRefOriginal.get(), ODP, &tempFile);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      tempFile.GetURL());

    // check that the document contains original vector images
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSVMFiles = 0;
    for (OUString const& s : names)
    {
        if (s.endsWith(".svm"))
            nSVMFiles++;
    }
    CPPUNIT_ASSERT_EQUAL(3, nSVMFiles);
}

int SdOOXMLExportTest3::testTdf115005_FallBack_Images(bool bAddReplacementImages)
{
    sd::DrawDocShellRef xDocShRefOriginal = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf115005_no_fallback_images.odp"), ODP);

    // check if fallback images were not created if AddReplacementImages=true/false
    // set AddReplacementImages
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create());
        if (!officecfg::Office::Common::Save::Graphic::AddReplacementImages::isReadOnly())
            officecfg::Office::Common::Save::Graphic::AddReplacementImages::set(
                bAddReplacementImages, batch);
        batch->commit();
    }

    // save the file with already set options
    utl::TempFile tempFile;
    sd::DrawDocShellRef xDocShRefResaved = saveAndReload(xDocShRefOriginal.get(), ODP, &tempFile);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      tempFile.GetURL());

    // check that the document contains original vector images
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSVMFiles = 0;
    int nPNGFiles = 0;
    for (OUString const& n : names)
    {
        if (n.endsWith(".svm"))
            nSVMFiles++;
        if (n.endsWith(".png"))
            nPNGFiles++;
    }

    // check results
    CPPUNIT_ASSERT_EQUAL(1, nSVMFiles);
    return nPNGFiles;
}

void SdOOXMLExportTest3::testTdf115005_FallBack_Images_On()
{
    const int nPNGFiles = testTdf115005_FallBack_Images(true);
    CPPUNIT_ASSERT_EQUAL(1, nPNGFiles);
}

void SdOOXMLExportTest3::testTdf115005_FallBack_Images_Off()
{
    const int nPNGFiles = testTdf115005_FallBack_Images(false);
    CPPUNIT_ASSERT_EQUAL(0, nPNGFiles);
}

void SdOOXMLExportTest3::testTdf118806()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118806.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animMotion", "origin", "layout");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf130058()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf130058.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    double fShadowDist = 0.0;
    xShape->getPropertyValue("ShadowXDistance") >>= fShadowDist;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(0), fShadowDist);
    xShape->getPropertyValue("ShadowYDistance") >>= fShadowDist;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(141), fShadowDist);
    Color nColor;
    xShape->getPropertyValue("ShadowColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);
    sal_Int32 nTransparency = 0;
    xShape->getPropertyValue("ShadowTransparence") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(57), nTransparency);
    double fShadowSizeX = 0.0;
    xShape->getPropertyValue("ShadowSizeX") >>= fShadowSizeX;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000), fShadowSizeX);
    double fShadowSizeY = 0.0;
    xShape->getPropertyValue("ShadowSizeY") >>= fShadowSizeY;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000), fShadowSizeY);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf111789()
{
    // Shadow properties were not exported for text shapes.
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf111789.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // First text shape has some shadow
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
        bool bHasShadow = false;
        xShape->getPropertyValue("Shadow") >>= bHasShadow;
        CPPUNIT_ASSERT(bHasShadow);
        double fShadowDist = 0.0;
        xShape->getPropertyValue("ShadowXDistance") >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(273), fShadowDist);
        xShape->getPropertyValue("ShadowYDistance") >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(273), fShadowDist);
        Color nColor;
        xShape->getPropertyValue("ShadowColor") >>= nColor;
        CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), nColor);
        sal_Int32 nTransparency = 0;
        xShape->getPropertyValue("ShadowTransparence") >>= nTransparency;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7), nTransparency);
        double fShadowSizeX = 0.0;
        xShape->getPropertyValue("ShadowSizeX") >>= fShadowSizeX;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(100000), fShadowSizeX);
        double fShadowSizeY = 0.0;
        xShape->getPropertyValue("ShadowSizeY") >>= fShadowSizeY;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(100000), fShadowSizeY);
    }

    // Second text shape has no shadow
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0, xDocShRef));
        bool bHasShadow = false;
        xShape->getPropertyValue("Shadow") >>= bHasShadow;
        CPPUNIT_ASSERT(!bHasShadow);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf100348_convert_Fontwork2TextWarp()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf100348_Fontwork2TextWarp.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // Resulting pptx has to contain the TextWarp shape
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart("/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp");
    assertXPath(pXmlDocContent, sPathStart + "[@prst='textWave1']");
    const OString sPathAdj(sPathStart + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj1' and  @fmla='val 18750']");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj2' and  @fmla='val -7500']");

    // Reloading has to get the Fontwork shape back
    // TextPath makes a custom shape to a Fontwork shape, so must exist
    uno::Reference<beans::XPropertySet> xShapeWavePropSet(getShapeFromPage(0, 0, xDocShRef));
    auto aGeomPropSeq = xShapeWavePropSet->getPropertyValue("CustomShapeGeometry")
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);
    OUString sName = "TextPath";
    auto aIterator = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [sName](const beans::PropertyValue& rValue) { return rValue.Name == sName; });
    CPPUNIT_ASSERT_MESSAGE("No TextPath", aIterator != aGeomPropVec.end());

    // Type has to be same as in original document on roundtrip.
    sName = "Type";
    auto aIterator2 = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [sName](const beans::PropertyValue& rValue) { return rValue.Name == sName; });
    CPPUNIT_ASSERT_MESSAGE("No Type", aIterator2 != aGeomPropVec.end());
    OUString sOwnName;
    aIterator2->Value >>= sOwnName;
    CPPUNIT_ASSERT_EQUAL(OUString("fontwork-wave"), sOwnName);

    // Adjustmentvalues need to be the same.
    sName = "AdjustmentValues";
    auto aIterator3 = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [sName](const beans::PropertyValue& rValue) { return rValue.Name == sName; });
    CPPUNIT_ASSERT_MESSAGE("No AdjustmentValues", aIterator3 != aGeomPropVec.end());
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjValueSeq;
    aIterator3->Value >>= aAdjValueSeq;
    double fAdj1;
    aAdjValueSeq[0].Value >>= fAdj1;
    double fAdj2;
    aAdjValueSeq[1].Value >>= fAdj2;
    CPPUNIT_ASSERT_EQUAL(4050.0, fAdj1); // odp values, not pptx values
    CPPUNIT_ASSERT_EQUAL(9180.0, fAdj2);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf1225573_FontWorkScaleX()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf125573_FontWorkScaleX.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // Error was, that attribute 'fromWordArt' was ignored
    // ensure, resulting pptx has fromWordArt="1" on textArchDown shape
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr[@fromWordArt='1']");

    // Error was, that text in legacy shapes of category "Follow Path" was not scaled to the path.
    uno::Reference<beans::XPropertySet> xShapeArchProps(getShapeFromPage(0, 0, xDocShRef));
    awt::Rectangle aBoundRectArch;
    xShapeArchProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectArch;
    // difference should be zero, but allow some range for stroke thickness
    CPPUNIT_ASSERT_LESS(sal_Int32(50), std::abs(aBoundRectArch.Width - 13081));

    // Error was, that text in shapes of category "Warp" was not scaled to the path.
    uno::Reference<beans::XPropertySet> xShapeWaveProps(getShapeFromPage(0, 1, xDocShRef));
    awt::Rectangle aBoundRectWave;
    xShapeWaveProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectWave;
    // difference should be zero, but allow some range for stroke thickness
    CPPUNIT_ASSERT_LESS(sal_Int32(50), std::abs(aBoundRectWave.Width - 11514));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf99497_keepAppearanceOfCircleKind()
{
    // Error was, that all CircleKind were exported to 'ellipse'.
    // Resulting pptx has to contain the customshapes of the corresponding kind
    // slide 1 ARC -> arc, slide 2 CUT -> chord, slide 3 SECTION -> pie
    // Adjustment values need to exist and their values need to correspond to the
    // original angles. Shape 'arc' needs to be unfilled.
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf99497_CircleKind.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // slide 1 45° -> adj1 = 20493903, 270° -> adj2 = 5400000, <a:noFill/> exists
    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    const OString sPathStart1("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent1, sPathStart1 + "[@prst='arc']");
    const OString sPathAdj1(sPathStart1 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj1' and  @fmla='val 20493903']");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj2' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill");

    // slide 2 270° -> adj1 = 5400000, 180° -> adj2 = 10800000
    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    const OString sPathStart2("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent2, sPathStart2 + "[@prst='chord']");
    const OString sPathAdj2(sPathStart2 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj1' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj2' and  @fmla='val 10800000']");

    // slide 3 120° -> adj1 = 12600000, 30° -> adj2 = 20946396
    xmlDocUniquePtr pXmlDocContent3 = parseExport(tempFile, "ppt/slides/slide3.xml");
    const OString sPathStart3("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent3, sPathStart3 + "[@prst='pie']");
    const OString sPathAdj3(sPathStart3 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj1' and  @fmla='val 12600000']");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj2' and  @fmla='val 20946396']");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf104792()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104792-smart-art-animation.pptx"),
        PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[1]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:childTnLst[1]/p:set/p:cBhvr/p:tgtEl/p:spTgt",
                1);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf90627()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf90627.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    // Don't export empty endCondLst without cond.
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:endCondLst[not(*)]",
                0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf104786()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104786.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide2.xml");
    // Don't export empty 'to'
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:childTnLst/p:set[2]/p:to",
                0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf118783()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118783.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sAttributeName
        = getXPathContent(pXmlDocContent, "//p:animRot/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("r"), sAttributeName);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf104789()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf104789.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:set/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("style.opacity"), sAttributeName);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testOpenDocumentAsReadOnly()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/open-as-read-only.pptx"), PPTX);
    CPPUNIT_ASSERT(xDocShRef->IsSecurityOptOpenReadOnly());
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    CPPUNIT_ASSERT(xDocShRef->IsSecurityOptOpenReadOnly());
    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf118835()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118835.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "(//p:animClr)[1]", "clrSpc", "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[1]//p:attrName", "style.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[1]//p:to/a:srgbClr", "val", "ed1c24");

    assertXPath(pXmlDocContent, "(//p:animClr)[2]", "clrSpc", "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[2]//p:attrName", "stroke.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[2]//p:to/a:srgbClr", "val", "333399");

    assertXPath(pXmlDocContent, "(//p:animClr)[3]", "clrSpc", "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[3]//p:attrName", "fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[3]//p:to/a:srgbClr", "val", "fcd3c1");

    assertXPath(pXmlDocContent, "(//p:animClr)[5]", "clrSpc", "hsl");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[5]//p:attrName", "fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "h", "10800000");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "s", "0");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "l", "0");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf118768()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118768-brake.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:anim[1]", "from", "(-#ppt_w/2)");
    assertXPath(pXmlDocContent, "//p:anim[1]", "to", "(#ppt_x)");
    assertXPath(pXmlDocContent, "//p:anim[2]", "from", "0");

    assertXPath(pXmlDocContent, "//p:anim[2]", "to", "-1");
    assertXPath(pXmlDocContent, "//p:anim[2]/p:cBhvr/p:cTn", "autoRev", "1");

    assertXPath(pXmlDocContent, "//p:anim[3]", "by", "(#ppt_h/3+#ppt_w*0.1)");
    assertXPath(pXmlDocContent, "//p:anim[3]/p:cBhvr/p:cTn", "autoRev", "1");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf118836()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf118836.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "x", "250000");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "y", "250000");
    xDocShRef->DoClose();
}

static double getAdjustmentValue(const uno::Reference<beans::XPropertySet>& xSet)
{
    auto aGeomPropSeq
        = xSet->getPropertyValue("CustomShapeGeometry").get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);

    auto aIterator = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [](const beans::PropertyValue& rValue) { return rValue.Name == "AdjustmentValues"; });

    if (aIterator != aGeomPropVec.end())
    {
        uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustment;
        double fResult = 0.0;
        aIterator->Value >>= aAdjustment;
        aAdjustment[0].Value >>= fResult;
        return fResult;
    }

    return -1.0;
}

static bool getScaleXValue(const uno::Reference<beans::XPropertySet>& xSet)
{
    bool bScaleX = false;

    auto aGeomPropSeq
        = xSet->getPropertyValue("CustomShapeGeometry").get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);

    auto aIterator = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [](const beans::PropertyValue& rValue) { return rValue.Name == "TextPath"; });

    if (aIterator != aGeomPropVec.end())
    {
        uno::Sequence<beans::PropertyValue> aTextPathProperties;
        aIterator->Value >>= aTextPathProperties;
        auto aIterator2 = std::find_if(
            std::cbegin(aTextPathProperties), std::cend(aTextPathProperties),
            [](const beans::PropertyValue& rValue) { return rValue.Name == "ScaleX"; });

        if (aIterator2 != std::cend(aTextPathProperties))
        {
            aIterator2->Value >>= bScaleX;
        }
    }

    return bScaleX;
}

void SdOOXMLExportTest3::testTdf116350TextEffects()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf116350-texteffects.pptx"), PPTX);

    // Default angle for ArchUp
    uno::Reference<beans::XPropertySet> xShape0(getShapeFromPage(0, 0, xDocShRef));
    double fAdjust = getAdjustmentValue(xShape0);
    CPPUNIT_ASSERT_EQUAL(180.0, fAdjust);

    bool bScaleX = getScaleXValue(xShape0);
    CPPUNIT_ASSERT_EQUAL(true, bScaleX);

    // Default angle for ArchDown
    uno::Reference<beans::XPropertySet> xShape14(getShapeFromPage(14, 0, xDocShRef));
    fAdjust = getAdjustmentValue(xShape14);
    CPPUNIT_ASSERT_EQUAL(0.0, fAdjust);

    bScaleX = getScaleXValue(xShape14);
    CPPUNIT_ASSERT_EQUAL(true, bScaleX);

    // Angle directly set
    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(1, 0, xDocShRef));
    fAdjust = getAdjustmentValue(xShape1);
    CPPUNIT_ASSERT_EQUAL(213.25, fAdjust);

    bScaleX = getScaleXValue(xShape1);
    CPPUNIT_ASSERT_EQUAL(true, bScaleX);

    // Export
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", "textArchUp");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", "textCircle");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:spPr/a:solidFill/a:srgbClr", 0);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf128096()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf128096.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocContent1 = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val",
                "ffff00");

    // Check that underlined content is also highlighted
    xmlDocUniquePtr pXmlDocContent2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val",
                "ffff00");
}
void SdOOXMLExportTest3::testTdf120573()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf120573.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:audioFile", 1);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile", 0);

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../media/media1.wav'])[1]", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");

    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.wav']",
                "ContentType", "audio/x-wav");
}

void SdOOXMLExportTest3::testTdf119118()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf119118.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:iterate", "type", "lt");
    assertXPath(pXmlDocContent, "//p:tmAbs", "val", "200");
    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf99213()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf99213-target-missing.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    // Number of nodes with p:attrNameLst was 3, including one that missed tgtEl
    assertXPath(pXmlDocContent, "//p:attrNameLst", 2);
    // Timenode that miss its target element should be filtered.
    assertXPath(pXmlDocContent, "//p:attrNameLst/preceding-sibling::p:tgtEl", 2);
    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testPotxExport()
{
    // Create new document
    sd::DrawDocShellRef xDocShRef
        = new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Draw);
    uno::Reference<frame::XLoadable> xLoadable(xDocShRef->GetModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xLoadable.is());
    xLoadable->initNew();

    // Export as a POTM template
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), POTX, &tempFile);
    xDocShRef->DoClose();

    // Load and check content type
    xmlDocUniquePtr pContentTypes = parseExport(tempFile, "[Content_Types].xml");
    CPPUNIT_ASSERT(pContentTypes);
    assertXPath(pContentTypes,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml");
}

void SdOOXMLExportTest3::testTdf44223()
{
    utl::TempFile tempFile;
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf44223.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    std::unique_ptr<SvStream> const pStream1(parseExportStream(tempFile, "ppt/media/audio1.wav"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(11140), pStream1->remainingSize());

    std::unique_ptr<SvStream> const pStream2(parseExportStream(tempFile, "ppt/media/audio2.wav"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(28074), pStream2->remainingSize());

    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio1.wav']",
                "ContentType", "audio/x-wav");

    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio2.wav']",
                "ContentType", "audio/x-wav");

    xmlDocUniquePtr pDoc1 = parseExport(tempFile, "ppt/slides/slide1.xml");

    // Start condition: 0s after timenode id 5 begins.
    assertXPath(pDoc1, "//p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "evt", "begin");
    assertXPath(pDoc1, "//p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "delay", "0");
    assertXPath(pDoc1, "//p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond/p:tn", "val", "5");

    xmlDocUniquePtr pDoc2 = parseExport(tempFile, "ppt/slides/slide2.xml");
    assertXPath(pDoc2, "//p:transition/p:sndAc/p:stSnd/p:snd[@r:embed]", 2);

    xmlDocUniquePtr pRels1 = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Target", "../media/audio1.wav");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testSmartArtPreserve()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/smartart-preserve.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDoc = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr");
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/dgm:relIds");
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/p:extLst/p:ext",
                "uri", "{D42A27DB-BD31-4B8C-83A1-F6EECF244321}");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/"
                         "p:extLst/p:ext/p14:modId");

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/_rels/slide1.xml.rels");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/layout1.xml'])[1]", "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramLayout");
    assertXPath(
        pXmlDocRels, "(/rels:Relationships/rels:Relationship[@Target='../diagrams/data1.xml'])[1]",
        "Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/colors1.xml'])[1]", "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramColors");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/quickStyle1.xml'])[1]", "Type",
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramQuickStyle");

    xmlDocUniquePtr pXmlContentType = parseExport(tempFile, "[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/layout1.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/data1.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/colors1.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml");
    assertXPath(
        pXmlContentType,
        "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/quickStyle1.xml']",
        "ContentType", "application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf125346()
{
    // There are two themes in the test document, make sure we use the right theme
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125346.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

    Color nFillColor;
    xPropSet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf125346_2()
{
    // There are two themes in the test document, make sure we use the right theme
    // Test more slides with different themes
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125346_2.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
    }

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 1, xDocShRef));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x052F61), nFillColor);
    }

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 2, xDocShRef));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
    }

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf125360()
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by shape style
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125360.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));

    xShape->setPropertyValue("FillTransparence", uno::makeAny(static_cast<sal_Int32>(23)));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xShape.set(getShapeFromPage(0, 0, xDocShRef));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf125360_1()
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125360_1.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));

    xShape->setPropertyValue("FillTransparence", uno::makeAny(static_cast<sal_Int32>(23)));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xShape.set(getShapeFromPage(0, 0, xDocShRef));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf125360_2()
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme with a transparency
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125360_2.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(82), nTransparence);

    xShape->setPropertyValue("FillTransparence", uno::makeAny(static_cast<sal_Int32>(23)));

    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xShape.set(getShapeFromPage(0, 0, xDocShRef));

    nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf125551()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf125551.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeBg(xGroupShape->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1024), xShapeBg->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(576), xShapeBg->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10815), xShapeBg->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8587), xShapeBg->getSize().Height);
}

void SdOOXMLExportTest3::testTdf136830()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf136830.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // Without the fix in place, the X position of the shapes would have been 0
    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape1(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14134), xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-5321), xShape1->getPosition().Y);

    uno::Reference<drawing::XShape> xShape2(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8085), xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8085), xShape2->getPosition().Y);

    uno::Reference<drawing::XShape> xShape3(xGroupShape->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8283), xShape3->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4620), xShape3->getPosition().Y);
}

void SdOOXMLExportTest3::testTdf126234()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf126234.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // check relative size of the bullet, 400% is a legitimate value for MS Office document
    // Without a fix, it will fail to set the size correctly
    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(400), pNumFmt->GetNumRule().GetLevel(0).GetBulletRelSize());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf126741()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf126741.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    // dash dot dot line style import fix
    // The original fixed values are replaced with the percent values, because
    // with fix for tdf#127166 the MS Office preset styles are correctly detected.
    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrObject* const pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);

    const XLineStyleItem& rStyleItem
        = dynamic_cast<const XLineStyleItem&>(pObj->GetMergedItem(XATTR_LINESTYLE));
    const XLineDashItem& rDashItem
        = dynamic_cast<const XLineDashItem&>(pObj->GetMergedItem(XATTR_LINEDASH));

    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rDashItem.GetDashValue().GetDots());
    CPPUNIT_ASSERT_EQUAL(800.0, rDashItem.GetDashValue().GetDotLen());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rDashItem.GetDashValue().GetDashes());
    CPPUNIT_ASSERT_EQUAL(100.0, rDashItem.GetDashValue().GetDashLen());
    CPPUNIT_ASSERT_EQUAL(300.0, rDashItem.GetDashValue().GetDistance());

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf127372()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf127372.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    awt::Gradient aTransparenceGradient;
    xShape->getPropertyValue("FillTransparenceGradient") >>= aTransparenceGradient;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aTransparenceGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aTransparenceGradient.EndColor);
}

void SdOOXMLExportTest3::testTdf127379()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf127379.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(getPage(0, xDocShRef));
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    uno::Any aAny = xPropSet->getPropertyValue("Background");
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());
    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    aXBackgroundPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, aFillStyle);

    awt::Gradient aGradient;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue("FillGradient") >>= aGradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x2A6099), aGradient.EndColor);
}

void SdOOXMLExportTest3::testTdf98603()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf98603.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    css::lang::Locale aLocale;
    xPropSet->getPropertyValue("CharLocaleComplex") >>= aLocale;
    CPPUNIT_ASSERT_EQUAL(OUString("he"), aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("IL"), aLocale.Country);
}

void SdOOXMLExportTest3::testTdf128213()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf128213.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm", "rot");
}

void SdOOXMLExportTest3::testTdf79082()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf79082.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]", "pos",
                "360000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]", "algn", "l");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]", "pos",
                "756000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]", "algn", "l");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]", "pos",
                "1440000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]", "algn",
                "ctr");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]", "pos",
                "1800000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]", "algn", "r");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]", "pos",
                "3240000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]", "algn",
                "dec");

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf129372()
{
    //Without the fix in place, it would crash at import time
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf129372.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef.get());

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(OBJ_OLE2, pObj->GetObjIdentifier());
}

void SdOOXMLExportTest3::testShapeGlowEffect()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-glow-effect.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("GlowEffectRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 139700 EMU = 388.0556 mm/100
    Color nColor;
    xShape->getPropertyValue("GlowEffectColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue("GlowEffectTransparency") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nTransparency);
}

void SdOOXMLExportTest3::testTdf119087()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf119087.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    // This would fail both on export validation, and reloading the saved pptx file.

    // Get first paragraph of the text
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nColor = COL_AUTO;
    xPropSet->getPropertyValue("CharColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x00B050), nColor);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf131554()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf131554.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0, xDocShRef), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5622), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(13251), xShape->getPosition().Y);
}

void SdOOXMLExportTest3::testTdf132282()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf132282.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    // Without the fix in place, the position would be 0,0, height = 1 and width = 1
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1736), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(763), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(30523), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2604), xShape->getSize().Height);
}

void SdOOXMLExportTest3::testTdf132201EffectOrder()
{
    auto xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/effectOrder.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xmlDocUniquePtr pXmlDocContent = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst", 2);
    // The relative order of effects is important: glow must be before shadow
    CPPUNIT_ASSERT_EQUAL(0, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "glow"));
    CPPUNIT_ASSERT_EQUAL(1, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "outerShdw"));

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testShapeSoftEdgeEffect()
{
    auto xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-soft-edges.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    auto xShapeProps(getShapeFromPage(0, 0, xDocShRef));
    sal_Int32 nRadius = -1;
    xShapeProps->getPropertyValue("SoftEdgeRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRadius); // 18 pt
}

void SdOOXMLExportTest3::testShapeShadowBlurEffect()
{
    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/shape-blur-effect.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("ShadowBlur") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 11 pt
}

void SdOOXMLExportTest3::testTdf119223()
{
    auto xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/tdf119223.odp"), ODP);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);

    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeCustomShape']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomePicture']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeFormula']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeLine']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTextbox']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTable']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeGroup']");
}

void SdOOXMLExportTest3::testTdf128213ShapeRot()
{
    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf128213-shaperot.pptx"), PPTX);
    utl::TempFile tempFile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "ppt/slides/slide1.xml");

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d");
    assertXPath(pXmlDocRels,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot", "rev",
                "5400000");
}

void SdOOXMLExportTest3::testTdf125560_textDeflate()
{
    auto xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/ShapePlusImage.pptx"), PPTX);
    utl::TempFile tempFile;

    // This problem did not affect the pptx export, only the ODP so assert that
    xDocShRef = saveAndReload(xDocShRef.get(), ODP, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "content.xml");
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry",
                "type", "mso-spt161");
}

void SdOOXMLExportTest3::testTdf125560_textInflateTop()
{
    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/ShapeTextInflateTop.pptx"), PPTX);
    utl::TempFile tempFile;

    // This problem did not affect the pptx export, only the ODP so assert that
    xDocShRef = saveAndReload(xDocShRef.get(), ODP, &tempFile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXmlDocRels = parseExport(tempFile, "content.xml");
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry",
                "type", "mso-spt164");
}

void SdOOXMLExportTest3::testTdf142235_TestPlaceholderTextAlignment()
{
    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/odp/placeholder-box-textalignment.odp"), ODP);

    utl::TempFile tmpfile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tmpfile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXml1 = parseExport(tmpfile, "ppt/slides/slide2.xml");
    xmlDocUniquePtr pXml2 = parseExport(tmpfile, "ppt/slides/slide3.xml");

    // Without the fix in place many of these asserts failed, because alignment was bad.

    assertXPath(pXml1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", "t");
    assertXPath(pXml2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", "t");
}

void SdOOXMLExportTest3::testTdf96061_textHighlight()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf96061.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<text::XTextRange> const xParagraph1(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun1(getRunFromParagraph(0, xParagraph1));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun1, uno::UNO_QUERY_THROW);
    sal_Int32 aColor;
    xPropSet1->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16776960), aColor);

    uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(1, xShape));
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    xPropSet2->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aColor);

    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(0, 0, xDocShRef));
    uno::Reference<text::XTextRange> const xParagraph3(getParagraphFromShape(0, xShape2));
    uno::Reference<text::XTextRange> xRun3(getRunFromParagraph(0, xParagraph3));
    uno::Reference<beans::XPropertySet> xPropSet3(xRun3, uno::UNO_QUERY_THROW);
    xPropSet3->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16776960), aColor);

    uno::Reference<text::XTextRange> const xParagraph4(getParagraphFromShape(1, xShape2));
    uno::Reference<text::XTextRange> xRun4(getRunFromParagraph(0, xParagraph4));
    uno::Reference<beans::XPropertySet> xPropSet4(xRun4, uno::UNO_QUERY_THROW);
    xPropSet4->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aColor);
}

void SdOOXMLExportTest3::testTdf143222_embeddedWorksheet()
{
    // Check import of embedded worksheet in slide.
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf143222.pptx"), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef.get());
    const SdrOle2Obj* pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object", pOleObj != nullptr);

    // Without the fix we lost the graphic of ole object.
    const Graphic* pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic", pGraphic != nullptr);

    // Check export of embedded worksheet in slide.
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX);

    pPage = GetPage(1, xDocShRef.get());
    pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object after the export", pOleObj != nullptr);

    pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic after the export", pGraphic != nullptr);

    xDocShRef->DoClose();
}

void SdOOXMLExportTest3::testTdf143315()
{
    auto xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/ppt/tdf143315-WordartWithoutBullet.ppt"),
        PPT);

    utl::TempFile tmpfile;
    xDocShRef = saveAndReload(xDocShRef.get(), PPTX, &tmpfile);
    xDocShRef->DoClose();

    xmlDocUniquePtr pXml = parseExport(tmpfile, "ppt/slides/slide1.xml");

    // Without the fix in place, this would have failed with
    // - Expected:
    // - Actual  : 216000
    // - In <file:///tmp/lu161922zcvd.tmp>, XPath '/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr' unexpected 'marL' attribute

    assertXPathNoAttribute(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr", "marL");
    assertXPathNoAttribute(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr", "indent");
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buClr", 0);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buSzPct", 0);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buFont", 0);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buChar", 0);
}

void SdOOXMLExportTest3::testTdf140912_PicturePlaceholder()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdfpictureplaceholder.pptx"), PPTX);

    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0, xDocShRef));
    bool bTextContourFrame = true;
    xShapeProps->getPropertyValue("TextContourFrame") >>= bTextContourFrame;
    CPPUNIT_ASSERT_EQUAL(false, bTextContourFrame);

    text::GraphicCrop aGraphicCrop;
    xShapeProps->getPropertyValue("GraphicCrop") >>= aGraphicCrop;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-8490), aGraphicCrop.Top);

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdOOXMLExportTest3);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
