
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
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <sdpage.hxx>

class SdOOXMLExportTest3 : public SdModelTestBase
{
public:
    SdOOXMLExportTest3()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }

    int testTdf115005_FallBack_Images(bool bAddReplacementImages);
};

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf153105)
{
    createSdImpressDoc("odp/tdf153105.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "l",
                "20000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "t",
                "30000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "r",
                "20000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "b",
                "30000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf92222)
{
    createSdImpressDoc("pptx/tdf92222.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocTheme = parseExport("ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[1]", "w",
                "6350");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[2]", "w",
                "12700");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[3]", "w",
                "19050");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf129430)
{
    createSdImpressDoc("odp/tdf129430.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:pPr/a:lnSpc/a:spcPct",
                "val", "100000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf114848)
{
    createSdImpressDoc("pptx/tdf114848.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocTheme1 = parseExport("ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val",
                "1f497d");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf147586)
{
    createSdImpressDoc("pptx/tdf147586.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    // Without the fix in place, this test would have failed with
    // - Expected: 227fc7
    // - Actual  : 4f4f4f
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[1]/a:pPr/a:buClr/a:srgbClr", "val",
                "227fc7");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[2]/a:pPr/a:buClr/a:srgbClr", "val",
                "227fc7");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf68759)
{
    createSdImpressDoc("odp/tdf68759.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf127901)
{
    createSdImpressDoc("odp/tdf127901.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "bright",
                "70000");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "contrast",
                "-70000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:grayscl", 1);

    xmlDocUniquePtr pXmlDocContent3 = parseExport("ppt/slides/slide3.xml");
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:biLevel",
                "thresh", "50000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf48735)
{
    createSdImpressDoc("odp/tdf48735.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "b", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "l", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "r", "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "t", "18842");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf90626)
{
    createSdImpressDoc("odp/tdf90626.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buSzPct",
                "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buSzPct",
                "val", "150142");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buSzPct",
                "val", "100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr/a:buSzPct",
                "val", "150142");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf107608)
{
    createSdImpressDoc("pptx/tdf107608.pptx");
    save("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf111786)
{
    // Export line transparency with the color

    createSdImpressDoc("pptx/tdf111786.pptx");
    save("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    Color nLineColor;
    xPropSet->getPropertyValue("LineColor") >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x3465A4), nLineColor);

    sal_Int16 nTransparency;
    xPropSet->getPropertyValue("LineTransparence") >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(33), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testFontScale)
{
    createSdImpressDoc("pptx/font-scale.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");

    // Rounding errors possible, approximate value (+/- 1%)
    OUString sScale = getXPath(
        pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:normAutofit", "fontScale");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(81111), sScale.toInt32(), 1000);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testShapeAutofitPPTX)
{
    createSdImpressDoc("pptx/testShapeAutofit.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // TextAutoGrowHeight --> "Resize shape to fit text" --> true
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:spAutoFit", 1);
    // TextAutoGrowHeight --> "Resize shape to fit text" --> false
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testLegacyShapeAutofitPPTX)
{
    createSdImpressDoc("odp/testLegacyShapeAutofit.odp");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // Text in a legacy rectangle
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:noAutofit", 1);
    // Text in (closed) Polygon
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
    // Text in a legacy ellipse
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf115394)
{
    createSdImpressDoc("pptx/tdf115394.pptx");
    save("Impress Office Open XML");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    double fTransitionDuration;

    // Slow in MS formats
    SdPage* pPage1 = pDoc->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);

    // Medium in MS formats
    SdPage* pPage2 = pDoc->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Fast in MS formats
    SdPage* pPage3 = pDoc->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Custom values
    SdPage* pPage4 = pDoc->GetSdPage(3, PageKind::Standard);
    fTransitionDuration = pPage4->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.25, fTransitionDuration);

    SdPage* pPage5 = pDoc->GetSdPage(4, PageKind::Standard);
    fTransitionDuration = pPage5->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(4.25, fTransitionDuration);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf115394Zero)
{
    createSdImpressDoc("pptx/tdf115394-zero.pptx");
    save("Impress Office Open XML");
    double fTransitionDuration;

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdPage* pPage = pDoc->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.01, fTransitionDuration);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf115005)
{
    createSdImpressDoc("odp/tdf115005.odp");
    save("impress8");

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());

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
    createSdImpressDoc("odp/tdf115005_no_fallback_images.odp");

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
    save("impress8");

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());

    // check that the document contains original vector images
    const uno::Sequence<OUString> names = xNameAccess->getElementNames();
    int nSVMFiles = 0;
    int nPNGFiles = 0;
    for (OUString const& n : names)
    {
        if (n.endsWith(".svm"))
            nSVMFiles++;
        if (n.endsWith(".png") && n != "Thumbnails/thumbnail.png") // Ignore the thumbnail
            nPNGFiles++;
    }

    // check results
    CPPUNIT_ASSERT_EQUAL(1, nSVMFiles);
    return nPNGFiles;
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf115005_FallBack_Images_On)
{
    const int nPNGFiles = testTdf115005_FallBack_Images(true);
    CPPUNIT_ASSERT_EQUAL(1, nPNGFiles);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf115005_FallBack_Images_Off)
{
    const int nPNGFiles = testTdf115005_FallBack_Images(false);
    CPPUNIT_ASSERT_EQUAL(0, nPNGFiles);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118806)
{
    createSdImpressDoc("odp/tdf118806.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animMotion", "origin", "layout");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf130058)
{
    createSdImpressDoc("pptx/tdf130058.pptx");
    save("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf111789)
{
    // Shadow properties were not exported for text shapes.

    createSdImpressDoc("pptx/tdf111789.pptx");
    save("Impress Office Open XML");

    // First text shape has some shadow
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
        bool bHasShadow = false;
        xShape->getPropertyValue("Shadow") >>= bHasShadow;
        CPPUNIT_ASSERT(bHasShadow);
        double fShadowDist = 0.0;
        xShape->getPropertyValue("ShadowXDistance") >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(274), fShadowDist);
        xShape->getPropertyValue("ShadowYDistance") >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(274), fShadowDist);
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
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));
        bool bHasShadow = false;
        xShape->getPropertyValue("Shadow") >>= bHasShadow;
        CPPUNIT_ASSERT(!bHasShadow);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf145162)
{
    createSdImpressDoc("pptx/tdf145162.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");
    // Before the fix, that tag was missing so PP put bullet to each para.
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testZeroIndentExport)
{
    // Load the bugdoc and save to pptx then.

    createSdImpressDoc("odp/testZeroIndent.odp");
    save("Impress Office Open XML");
    // There are 3 slides, get them
    xmlDocUniquePtr pSlide1 = parseExport("ppt/slides/slide1.xml");
    xmlDocUniquePtr pSlide2 = parseExport("ppt/slides/slide2.xml");
    xmlDocUniquePtr pSlide3 = parseExport("ppt/slides/slide3.xml");

    CPPUNIT_ASSERT(pSlide1);
    CPPUNIT_ASSERT(pSlide2);
    CPPUNIT_ASSERT(pSlide3);

    // Each slide has 3 paragraphs, one full line, an empty and a normal para.
    // Check the indent and bullet. These have to match with PP. Before the fix,
    // they were different.
    assertXPath(pSlide1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");

    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buNone");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr", "indent", "0");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr", "indent", "0");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buNone");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr", "indent", "0");

    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr", "indent", "0");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr", "indent", "0");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr", "indent", "0");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf100348_convert_Fontwork2TextWarp)
{
    createSdImpressDoc("odp/tdf100348_Fontwork2TextWarp.odp");
    save("Impress Office Open XML");

    // Resulting pptx has to contain the TextWarp shape
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    const OString sPathStart("/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp");
    assertXPath(pXmlDocContent, sPathStart + "[@prst='textWave1']");
    const OString sPathAdj(sPathStart + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj1' and  @fmla='val 18750']");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj2' and  @fmla='val -7500']");

    // Reloading has to get the Fontwork shape back
    // TextPath makes a custom shape to a Fontwork shape, so must exist
    uno::Reference<beans::XPropertySet> xShapeWavePropSet(getShapeFromPage(0, 0));
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
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125573_FontWorkScaleX)
{
    createSdImpressDoc("pptx/tdf125573_FontWorkScaleX.pptx");
    save("Impress Office Open XML");

    // Error was, that attribute 'fromWordArt' was ignored
    // ensure, resulting pptx has fromWordArt="1" on textArchDown shape
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr[@fromWordArt='1']");

    // Error was, that text in legacy shapes of category "Follow Path" was not scaled to the path.
    uno::Reference<beans::XPropertySet> xShapeArchProps(getShapeFromPage(0, 0));
    awt::Rectangle aBoundRectArch;
    xShapeArchProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectArch;
    // BoundRect is DPI dependent, thus allow some range.
    // Expected width is 13139 in 96dpi and is 13106 in 120 dpi, for example
    // (Without fix Expected less than: 85 Actual  : 10432)
    CPPUNIT_ASSERT_LESS(sal_Int32(85), std::abs(aBoundRectArch.Width - 13145));

    // Error was, that text in shapes of category "Warp" was not scaled to the path.
    uno::Reference<beans::XPropertySet> xShapeWaveProps(getShapeFromPage(0, 1));
    awt::Rectangle aBoundRectWave;
    xShapeWaveProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectWave;
    // BoundRect is DPI dependent, thus allow some range.
    // Expected with is 11576 in 96dpt and is 11578 in 120dpi, for example
    CPPUNIT_ASSERT_LESS(sal_Int32(85), std::abs(aBoundRectWave.Width - 11576));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf99497_keepAppearanceOfCircleKind)
{
    // Error was, that all CircleKind were exported to 'ellipse'.
    // Resulting pptx has to contain the customshapes of the corresponding kind
    // slide 1 ARC -> arc, slide 2 CUT -> chord, slide 3 SECTION -> pie
    // Adjustment values need to exist and their values need to correspond to the
    // original angles. Shape 'arc' needs to be unfilled.
    createSdImpressDoc("odp/tdf99497_CircleKind.odp");
    save("Impress Office Open XML");

    // slide 1 45° -> adj1 = 20493903, 270° -> adj2 = 5400000, <a:noFill/> exists
    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    const OString sPathStart1("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent1, sPathStart1 + "[@prst='arc']");
    const OString sPathAdj1(sPathStart1 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj1' and  @fmla='val 20493903']");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj2' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill");

    // slide 2 270° -> adj1 = 5400000, 180° -> adj2 = 10800000
    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    const OString sPathStart2("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent2, sPathStart2 + "[@prst='chord']");
    const OString sPathAdj2(sPathStart2 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj1' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj2' and  @fmla='val 10800000']");

    // slide 3 120° -> adj1 = 12600000, 30° -> adj2 = 20946396
    xmlDocUniquePtr pXmlDocContent3 = parseExport("ppt/slides/slide3.xml");
    const OString sPathStart3("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom");
    assertXPath(pXmlDocContent3, sPathStart3 + "[@prst='pie']");
    const OString sPathAdj3(sPathStart3 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj1' and  @fmla='val 12600000']");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj2' and  @fmla='val 20946396']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf104792)
{
    createSdImpressDoc("pptx/tdf104792-smart-art-animation.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[1]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:childTnLst[1]/p:set/p:cBhvr/p:tgtEl/p:spTgt",
                1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf90627)
{
    createSdImpressDoc("odp/tdf90627.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    // Don't export empty endCondLst without cond.
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:endCondLst[not(*)]",
                0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf104786)
{
    createSdImpressDoc("pptx/tdf104786.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide2.xml");
    // Don't export empty 'to'
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[2]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:childTnLst/p:set[2]/p:to",
                0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118783)
{
    createSdImpressDoc("odp/tdf118783.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    OUString sAttributeName
        = getXPathContent(pXmlDocContent, "//p:animRot/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("r"), sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf104789)
{
    createSdImpressDoc("pptx/tdf104789.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:set/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(OUString("style.opacity"), sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testOpenDocumentAsReadOnly)
{
    createSdImpressDoc("pptx/open-as-read-only.pptx");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::DrawDocShell* pDocShell = pXImpressDocument->GetDocShell();
    CPPUNIT_ASSERT(pDocShell->IsSecurityOptOpenReadOnly());
    save("Impress Office Open XML");
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDocShell = pXImpressDocument->GetDocShell();
    CPPUNIT_ASSERT(pDocShell->IsSecurityOptOpenReadOnly());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118835)
{
    createSdImpressDoc("odp/tdf118835.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
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
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118768)
{
    createSdImpressDoc("odp/tdf118768-brake.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:anim[1]", "from", "(-#ppt_w/2)");
    assertXPath(pXmlDocContent, "//p:anim[1]", "to", "(#ppt_x)");
    assertXPath(pXmlDocContent, "//p:anim[2]", "from", "0");

    assertXPath(pXmlDocContent, "//p:anim[2]", "to", "-1");
    assertXPath(pXmlDocContent, "//p:anim[2]/p:cBhvr/p:cTn", "autoRev", "1");

    assertXPath(pXmlDocContent, "//p:anim[3]", "by", "(#ppt_h/3+#ppt_w*0.1)");
    assertXPath(pXmlDocContent, "//p:anim[3]/p:cBhvr/p:cTn", "autoRev", "1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118836)
{
    createSdImpressDoc("odp/tdf118836.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "x", "250000");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "y", "250000");
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf116350TextEffects)
{
    createSdImpressDoc("pptx/tdf116350-texteffects.pptx");

    // Default angle for ArchUp
    uno::Reference<beans::XPropertySet> xShape0(getShapeFromPage(0, 0));
    double fAdjust = getAdjustmentValue(xShape0);
    CPPUNIT_ASSERT_EQUAL(180.0, fAdjust);

    bool bScaleX = getScaleXValue(xShape0);
    CPPUNIT_ASSERT_EQUAL(true, bScaleX);

    // Default angle for ArchDown
    uno::Reference<beans::XPropertySet> xShape14(getShapeFromPage(14, 0));
    fAdjust = getAdjustmentValue(xShape14);
    CPPUNIT_ASSERT_EQUAL(0.0, fAdjust);

    bScaleX = getScaleXValue(xShape14);
    CPPUNIT_ASSERT_EQUAL(true, bScaleX);

    // Angle directly set
    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(1, 0));
    fAdjust = getAdjustmentValue(xShape1);
    CPPUNIT_ASSERT_EQUAL(213.25, fAdjust);

    bScaleX = getScaleXValue(xShape1);
    CPPUNIT_ASSERT_EQUAL(true, bScaleX);

    // Export
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", "textArchUp");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", "textCircle");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:spPr/a:solidFill/a:srgbClr", 0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf128096)
{
    createSdImpressDoc("odp/tdf128096.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val",
                "ffff00");

    // Check that underlined content is also highlighted
    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val",
                "ffff00");
}
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf120573)
{
    createSdImpressDoc("pptx/tdf120573.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:audioFile", 1);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile", 0);

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../media/media1.wav'])[1]", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");

    xmlDocUniquePtr pXmlContentType = parseExport("[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.wav']",
                "ContentType", "audio/x-wav");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf119118)
{
    createSdImpressDoc("pptx/tdf119118.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:iterate", "type", "lt");
    assertXPath(pXmlDocContent, "//p:tmAbs", "val", "200");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf99213)
{
    createSdImpressDoc("odp/tdf99213-target-missing.odp");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    // Number of nodes with p:attrNameLst was 3, including one that missed tgtEl
    assertXPath(pXmlDocContent, "//p:attrNameLst", 2);
    // Timenode that miss its target element should be filtered.
    assertXPath(pXmlDocContent, "//p:attrNameLst/preceding-sibling::p:tgtEl", 2);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testPotxExport)
{
    // Create new document
    createSdImpressDoc();

    // Export as a POTM template
    save("Impress Office Open XML Template");

    // Load and check content type
    xmlDocUniquePtr pContentTypes = parseExport("[Content_Types].xml");
    CPPUNIT_ASSERT(pContentTypes);
    assertXPath(pContentTypes,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf44223)
{
    createSdImpressDoc("pptx/tdf44223.pptx");
    save("Impress Office Open XML");

    std::unique_ptr<SvStream> const pStream1(
        parseExportStream(maTempFile.GetURL(), "ppt/media/audio1.wav"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(11140), pStream1->remainingSize());

    std::unique_ptr<SvStream> const pStream2(
        parseExportStream(maTempFile.GetURL(), "ppt/media/audio2.wav"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(28074), pStream2->remainingSize());

    xmlDocUniquePtr pXmlContentType = parseExport("[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio1.wav']",
                "ContentType", "audio/x-wav");

    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio2.wav']",
                "ContentType", "audio/x-wav");

    xmlDocUniquePtr pDoc1 = parseExport("ppt/slides/slide1.xml");

    // tdf#124230 all nodes were under p:childTnLst, but event triggered nodes need
    // to be under p:subTnLst, especially for audio to work correctly.
    // Start condition: 0s after timenode id 5 begins.
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "evt",
                "begin");
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "delay", "0");
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond/p:tn", "val",
                "5");

    xmlDocUniquePtr pDoc2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pDoc2, "//p:transition/p:sndAc/p:stSnd/p:snd[@r:embed]", 2);

    xmlDocUniquePtr pRels1 = parseExport("ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Type",
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Target", "../media/audio1.wav");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf135843)
{
    createSdImpressDoc("pptx/tdf135843_export.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    const OString sPathStart("/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[1]/a:tc[1]/a:tcPr/a:lnL/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[1]/a:tc[1]/a:tcPr/a:lnR/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[1]/a:tc[1]/a:tcPr/a:lnT/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[1]/a:tc[1]/a:tcPr/a:lnB/a:solidFill");

    assertXPath(pXmlDoc, sPathStart + "/a:tr[2]/a:tc[1]/a:tcPr/a:lnL/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[2]/a:tc[1]/a:tcPr/a:lnR/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[2]/a:tc[1]/a:tcPr/a:lnT/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[2]/a:tc[1]/a:tcPr/a:lnB/a:solidFill");

    assertXPath(pXmlDoc, sPathStart + "/a:tr[3]/a:tc[1]/a:tcPr/a:lnL/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[3]/a:tc[1]/a:tcPr/a:lnR/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[3]/a:tc[1]/a:tcPr/a:lnT/a:solidFill");
    assertXPath(pXmlDoc, sPathStart + "/a:tr[3]/a:tc[1]/a:tcPr/a:lnB/a:solidFill");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testSmartArtPreserve)
{
    createSdImpressDoc("pptx/smartart-preserve.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr");
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/dgm:relIds");
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/p:extLst/p:ext",
                "uri", "{D42A27DB-BD31-4B8C-83A1-F6EECF244321}");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/"
                         "p:extLst/p:ext/p14:modId");

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/_rels/slide1.xml.rels");
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

    xmlDocUniquePtr pXmlContentType = parseExport("[Content_Types].xml");
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
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125346)
{
    // There are two themes in the test document, make sure we use the right theme

    createSdImpressDoc("pptx/tdf125346.pptx");
    save("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

    Color nFillColor;
    xPropSet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125346_2)
{
    // There are two themes in the test document, make sure we use the right theme
    // Test more slides with different themes

    createSdImpressDoc("pptx/tdf125346_2.pptx");
    save("Impress Office Open XML");

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
    }

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 1));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x052F61), nFillColor);
    }

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 2));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125360)
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by shape style

    createSdImpressDoc("pptx/tdf125360.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    xShape->setPropertyValue("FillTransparence", uno::Any(static_cast<sal_Int32>(23)));

    save("Impress Office Open XML");

    xShape.set(getShapeFromPage(0, 0));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125360_1)
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme

    createSdImpressDoc("pptx/tdf125360_1.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    xShape->setPropertyValue("FillTransparence", uno::Any(static_cast<sal_Int32>(23)));

    save("Impress Office Open XML");

    xShape.set(getShapeFromPage(0, 0));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125360_2)
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme with a transparency

    createSdImpressDoc("pptx/tdf125360_2.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(82), nTransparence);

    xShape->setPropertyValue("FillTransparence", uno::Any(static_cast<sal_Int32>(23)));

    save("Impress Office Open XML");

    xShape.set(getShapeFromPage(0, 0));

    nTransparence = 0;
    xShape->getPropertyValue("FillTransparence") >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125551)
{
    createSdImpressDoc("pptx/tdf125551.pptx");
    save("Impress Office Open XML");

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeBg(xGroupShape->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1024), xShapeBg->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(576), xShapeBg->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10816), xShapeBg->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8588), xShapeBg->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf136830)
{
    createSdImpressDoc("pptx/tdf136830.pptx");
    save("Impress Office Open XML");

    // Without the fix in place, the X position of the shapes would have been 0
    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf126234)
{
    createSdImpressDoc("pptx/tdf126234.pptx");
    save("Impress Office Open XML");

    // check relative size of the bullet, 400% is a legitimate value for MS Office document
    // Without a fix, it will fail to set the size correctly
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(400), pNumFmt->GetNumRule().GetLevel(0).GetBulletRelSize());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf126741)
{
    createSdImpressDoc("pptx/tdf126741.pptx");
    save("Impress Office Open XML");

    // dash dot dot line style import fix
    // The original fixed values are replaced with the percent values, because
    // with fix for tdf#127166 the MS Office preset styles are correctly detected.
    const SdrPage* pPage = GetPage(1);
    SdrObject* const pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);

    const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
    const XLineDashItem& rDashItem = pObj->GetMergedItem(XATTR_LINEDASH);

    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), rDashItem.GetDashValue().GetDots());
    CPPUNIT_ASSERT_EQUAL(800.0, rDashItem.GetDashValue().GetDotLen());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), rDashItem.GetDashValue().GetDashes());
    CPPUNIT_ASSERT_EQUAL(100.0, rDashItem.GetDashValue().GetDashLen());
    CPPUNIT_ASSERT_EQUAL(300.0, rDashItem.GetDashValue().GetDistance());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf127372)
{
    createSdImpressDoc("odp/tdf127372.odp");
    saveAndReload("Impress Office Open XML");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    awt::Gradient2 aTransparenceGradient;
    xShape->getPropertyValue("FillTransparenceGradient") >>= aTransparenceGradient;

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aTransparenceGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(aColorStops[0].getStopColor(), basegfx::BColor(0.0, 0.0, 0.0));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(aColorStops[1].getStopColor(), basegfx::BColor(0.0, 0.0, 0.0));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf127379)
{
    createSdImpressDoc("odp/tdf127379.odp");
    saveAndReload("Impress Office Open XML");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    uno::Any aAny = xPropSet->getPropertyValue("Background");
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());
    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    aXBackgroundPropSet->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, aFillStyle);

    awt::Gradient2 aGradient;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue("FillGradient") >>= aGradient);

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(aColorStops[0].getStopColor(), basegfx::BColor(1.0, 0.0, 0.0));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(
        aColorStops[1].getStopColor(),
        basegfx::BColor(0.16470588235294117, 0.37647058823529411, 0.59999999999999998));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf98603)
{
    createSdImpressDoc("pptx/tdf98603.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    css::lang::Locale aLocale;
    xPropSet->getPropertyValue("CharLocaleComplex") >>= aLocale;
    CPPUNIT_ASSERT_EQUAL(OUString("he"), aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("IL"), aLocale.Country);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf128213)
{
    createSdImpressDoc("pptx/tdf128213.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm", "rot");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf79082)
{
    createSdImpressDoc("pptx/tdf79082.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
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
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf129372)
{
    //Without the fix in place, it would crash at import time

    createSdImpressDoc("pptx/tdf129372.pptx");
    saveAndReload("Impress Office Open XML");
    const SdrPage* pPage = GetPage(1);

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testShapeGlowEffect)
{
    createSdImpressDoc("pptx/shape-glow-effect.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testUnderline)
{
    createSdImpressDoc("underline.fodp");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nColor;
    xPropSet->getPropertyValue("CharColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropSet->getPropertyValue("CharUnderlineColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nColor);

    sal_Int16 nUnderline;
    xPropSet->getPropertyValue("CharUnderline") >>= nUnderline;
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::DOUBLE, nUnderline);

    saveAndReload("Impress Office Open XML");

    xShape.set(getShapeFromPage(0, 0));
    xParagraph.set(getParagraphFromShape(0, xShape));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue("CharColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropSet->getPropertyValue("CharUnderlineColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nColor);

    xPropSet->getPropertyValue("CharUnderline") >>= nUnderline;
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::DOUBLE, nUnderline);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf119087)
{
    createSdImpressDoc("pptx/tdf119087.pptx");
    saveAndReload("Impress Office Open XML");
    // This would fail both on export validation, and reloading the saved pptx file.

    // Get first paragraph of the text
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nColor = COL_AUTO;
    xPropSet->getPropertyValue("CharColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x00B050), nColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf131554)
{
    createSdImpressDoc("pptx/tdf131554.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5622), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(13251), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf132282)
{
    createSdImpressDoc("pptx/tdf132282.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // Without the fix in place, the position would be 0,0, height = 1 and width = 1
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1736), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(763), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(30523), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2604), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf132201EffectOrder)
{
    createSdImpressDoc("pptx/effectOrder.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst", 2);
    // The relative order of effects is important: glow must be before shadow
    CPPUNIT_ASSERT_EQUAL(0, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "glow"));
    CPPUNIT_ASSERT_EQUAL(1, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "outerShdw"));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testShapeSoftEdgeEffect)
{
    createSdImpressDoc("pptx/shape-soft-edges.pptx");
    saveAndReload("Impress Office Open XML");
    auto xShapeProps(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShapeProps->getPropertyValue("SoftEdgeRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRadius); // 18 pt
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testShapeShadowBlurEffect)
{
    createSdImpressDoc("pptx/shape-blur-effect.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    bool bHasShadow = false;
    xShape->getPropertyValue("Shadow") >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue("ShadowBlur") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 11 pt
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf119223)
{
    createSdImpressDoc("odp/tdf119223.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeCustomShape']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomePicture']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeFormula']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeLine']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTextbox']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTable']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeGroup']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf128213ShapeRot)
{
    createSdImpressDoc("pptx/tdf128213-shaperot.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/slide1.xml");

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d");
    assertXPath(pXmlDocRels,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot", "rev",
                "5400000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125560_textDeflate)
{
    createSdImpressDoc("pptx/ShapePlusImage.pptx");

    // This problem did not affect the pptx export, only the ODP so assert that
    save("impress8");

    xmlDocUniquePtr pXmlDocRels = parseExport("content.xml");
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry",
                "type", "mso-spt161");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf125560_textInflateTop)
{
    createSdImpressDoc("pptx/ShapeTextInflateTop.pptx");

    // This problem did not affect the pptx export, only the ODP so assert that
    save("impress8");

    xmlDocUniquePtr pXmlDocRels = parseExport("content.xml");
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry",
                "type", "mso-spt164");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf142235_TestPlaceholderTextAlignment)
{
    createSdImpressDoc("odp/placeholder-box-textalignment.odp");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml1 = parseExport("ppt/slides/slide2.xml");
    xmlDocUniquePtr pXml2 = parseExport("ppt/slides/slide3.xml");

    // Without the fix in place many of these asserts failed, because alignment was bad.

    assertXPath(pXml1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", "t");
    assertXPath(pXml2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", "t");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf96061_textHighlight)
{
    createSdImpressDoc("pptx/tdf96061.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph1(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun1(getRunFromParagraph(0, xParagraph1));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun1, uno::UNO_QUERY_THROW);

    Color aColor;
    xPropSet1->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aColor);

    uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(1, xShape));
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    xPropSet2->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, aColor);

    saveAndReload("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph3(getParagraphFromShape(0, xShape2));
    uno::Reference<text::XTextRange> xRun3(getRunFromParagraph(0, xParagraph3));
    uno::Reference<beans::XPropertySet> xPropSet3(xRun3, uno::UNO_QUERY_THROW);
    xPropSet3->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aColor);

    uno::Reference<text::XTextRange> const xParagraph4(getParagraphFromShape(1, xShape2));
    uno::Reference<text::XTextRange> xRun4(getRunFromParagraph(0, xParagraph4));
    uno::Reference<beans::XPropertySet> xPropSet4(xRun4, uno::UNO_QUERY_THROW);
    xPropSet4->getPropertyValue("CharBackColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, aColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf143222_embeddedWorksheet)
{
    // Check import of embedded worksheet in slide.

    createSdImpressDoc("pptx/tdf143222.pptx");

    const SdrPage* pPage = GetPage(1);
    const SdrOle2Obj* pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object", pOleObj != nullptr);

    // Without the fix we lost the graphic of ole object.
    const Graphic* pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic", pGraphic != nullptr);

    // Check export of embedded worksheet in slide.
    saveAndReload("Impress Office Open XML");

    pPage = GetPage(1);
    pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object after the export", pOleObj != nullptr);

    pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic after the export", pGraphic != nullptr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf143315)
{
    createSdImpressDoc("ppt/tdf143315-WordartWithoutBullet.ppt");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf147121)
{
    // Get the bugdoc
    createSdImpressDoc("pptx/tdf147121.pptx");

    // Get the second line props of the placeholder
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xRun(
        getRunFromParagraph(2, getParagraphFromShape(0, xShape)), uno::UNO_QUERY_THROW);

    // Save the font size
    const auto nFontSizeBefore = xRun->getPropertyValue("CharHeight").get<float>() * 100;

    save("Impress Office Open XML");

    // Parse the export
    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");
    const auto nFontSizeAfter
        = getXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:br[1]/a:rPr", "sz").toFloat();

    // The font size was not saved before now it must be equal with the saved one.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font size", nFontSizeBefore, nFontSizeAfter);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf140912_PicturePlaceholder)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    // -8490 in the test below turns into -8014 on my Windows box with 150% scaling.
    if (!IsDefaultDPI())
        return;

    createSdImpressDoc("pptx/tdfpictureplaceholder.pptx");

    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0));
    bool bTextContourFrame = true;
    xShapeProps->getPropertyValue("TextContourFrame") >>= bTextContourFrame;
    CPPUNIT_ASSERT_EQUAL(false, bTextContourFrame);

    text::GraphicCrop aGraphicCrop;
    xShapeProps->getPropertyValue("GraphicCrop") >>= aGraphicCrop;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-8490), aGraphicCrop.Top);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testEnhancedPathViewBox)
{
    createSdImpressDoc("odp/tdf147978_enhancedPath_viewBox.odp");
    saveAndReload("Impress Office Open XML");
    auto xShapeProps(getShapeFromPage(0, 0));
    awt::Rectangle aBoundRectangle;
    xShapeProps->getPropertyValue("BoundRect") >>= aBoundRectangle;
    // The shape has a Bézier curve which does not touch the right edge. Prior to the fix the curve
    // was stretched to touch the edge, resulting in 5098 curve width instead of 2045.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2045), aBoundRectangle.Width);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf74670)
{
    createSdImpressDoc("odp/tdf74670.odp");
    save("Impress Office Open XML");

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    const uno::Sequence<OUString> aNames(xNameAccess->getElementNames());
    int nImageFiles = 0;
    for (const auto& rElementName : aNames)
        if (rElementName.startsWith("ppt/media/image"))
            nImageFiles++;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the embedded picture would have been saved twice.
    CPPUNIT_ASSERT_EQUAL(1, nImageFiles);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf109169_OctagonBevel)
{
    // The document has a shape 'Octagon Bevel'. It consists of an octagon with 8 points and eight
    // facets with 4 points each, total 8+8*4=40 points. Without the patch it was exported as
    // rectangle and thus had 4 points.

    createSdImpressDoc("odp/tdf109169_Octagon.odp");
    saveAndReload("Impress Office Open XML");

    auto xPropSet(getShapeFromPage(0, 0));
    auto aGeomPropSeq = xPropSet->getPropertyValue("CustomShapeGeometry")
                            .get<uno::Sequence<beans::PropertyValue>>();
    comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);
    auto aPathSeq((aCustomShapeGeometry["Path"]).get<uno::Sequence<beans::PropertyValue>>());
    comphelper::SequenceAsHashMap aPath(aPathSeq);
    auto aCoordinates(
        (aPath["Coordinates"]).get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(40), aCoordinates.getLength());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf109169_DiamondBevel)
{
    // The document has a shape 'Diamond Bevel'. It consists of a diamond with 4 points and four
    // facets with 4 points each, total 4+4*4=20 points. Without the patch it was exported as
    // rectangle and thus had 4 points.

    createSdImpressDoc("odp/tdf109169_Diamond.odp");
    saveAndReload("Impress Office Open XML");

    auto xPropSet(getShapeFromPage(0, 0));
    auto aGeomPropSeq = xPropSet->getPropertyValue("CustomShapeGeometry")
                            .get<uno::Sequence<beans::PropertyValue>>();
    comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);
    auto aPathSeq((aCustomShapeGeometry["Path"]).get<uno::Sequence<beans::PropertyValue>>());
    comphelper::SequenceAsHashMap aPath(aPathSeq);
    auto aCoordinates(
        (aPath["Coordinates"]).get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aCoordinates.getLength());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf144092_emptyShapeTextProps)
{
    // Document contains one shape and one table. Both without any text but with
    // text properties contained inside endParaRPr - The import and export
    // of endParaRPr for empty cells and shapes are tested here
    createSdImpressDoc("pptx/tdf144092-emptyShapeTextProps.pptx");
    saveAndReload("Impress Office Open XML");

    Color aColor;
    // check text properties of empty shape
    uno::Reference<beans::XPropertySet> xRectShapeProps(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Calibri"),
                         xRectShapeProps->getPropertyValue("CharFontName").get<OUString>());
    CPPUNIT_ASSERT_EQUAL(float(196), xRectShapeProps->getPropertyValue("CharHeight").get<float>());
    xRectShapeProps->getPropertyValue("CharColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x70AD47), aColor);

    const SdrPage* pPage = GetPage(1);
    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;

    // check text properties of empty cells
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("CharColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFFFF), aColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("CharColor") >>= aColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x70AD47), aColor);
    CPPUNIT_ASSERT_EQUAL(float(96), xCell->getPropertyValue("CharHeight").get<float>());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf149551_tbrl90)
{
    // The document contains a shape with writing mode TB_RL90. That is the same as vert="vert" in
    // OOXML. Without the patch it was not possible to use this writing mode at all.

    createSdImpressDoc("odp/tdf149551_tbrl90.odp");

    // Test, that the shape has writing mode TB_RL90.
    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode;
    xShapeProps->getPropertyValue("WritingMode") >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL90, eWritingMode);

    // Test, that it is exported to vert="vert"
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//a:bodyPr", "vert", "vert");

    // Test, that the shape has writing mode TB_RL90 after read from pptx
    uno::Reference<beans::XPropertySet> xShapeProps2(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode2;
    xShapeProps2->getPropertyValue("WritingMode") >>= eWritingMode2;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL90, eWritingMode2);

    // Test, that it is written to odp with loext:writing-mode="tb-rl90"
    save("impress8");
    pXmlDoc = parseExport("content.xml");
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='gr1']/style:graphic-properties[@loext:writing-mode='tb-rl90']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf149551_btlr)
{
    // The document contains a shape with writing mode BT_LR. That is the same as vert="vert270" in
    // OOXML. Without the patch it was not possible to use this writing mode at all for shapes.

    createSdImpressDoc("odp/tdf149551_btlr.odp");

    // Test, that the shape has writing mode BT_LR.
    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode;
    xShapeProps->getPropertyValue("WritingMode") >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, eWritingMode);

    // Test, that it is exported to vert="vert270"
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//a:bodyPr", "vert", "vert270");

    // Test, that the shape has writing mode BT_LR after read from pptx
    uno::Reference<beans::XPropertySet> xShapeProps2(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode2;
    xShapeProps2->getPropertyValue("WritingMode") >>= eWritingMode2;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, eWritingMode2);

    // Test, that it is written to odp with loext:writing-mode="bt-lr"
    save("impress8");
    pXmlDoc = parseExport("content.xml");
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='gr1']/style:graphic-properties[@loext:writing-mode='bt-lr']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf94122_autoColor)
{
    // Document contains three pages, with different scenarios for automatic
    // color export to pptx.
    // - First page: Page background light, automatic colored text on a FillType_NONE shape
    // - Second page: Page background dark, automatic colored text on a FillType_NONE shape
    // - Third page: Page background light, automatic colored text on a dark colored fill
    //   and another automatic colored text on a light colored fill

    createSdImpressDoc("odp/tdf94122_autocolor.odp");

    save("Impress Office Open XML");

    // Without the accompanying fix in place, these tests would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In ..., XPath '/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr' number of nodes is incorrect
    // i.e. automatic color wasn't resolved & exported

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr", "val",
                "000000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr", "val",
                "ffffff");

    xmlDocUniquePtr pXmlDocContent3 = parseExport("ppt/slides/slide3.xml");
    assertXPath(pXmlDocContent3,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr",
                "val", "ffffff");
    assertXPath(pXmlDocContent3,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr",
                "val", "000000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf124333)
{
    // Document contains one rectangle and one embedded OLE object.

    createSdImpressDoc("pptx/ole.pptx");

    // Without the fix in place, the number of shapes was 3.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("number of shapes is incorrect", sal_Int32(2),
                                 getPage(0)->getCount());

    saveAndReload("Impress Office Open XML");

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("number of shapes is incorrect after export", sal_Int32(2),
                                 getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testAutofittedTextboxIndent)
{
    createSdImpressDoc("odp/autofitted-textbox-indent.odp");

    save("Impress Office Open XML");

    // Check that the indent hasn't changed and wasn't scaled when exporting
    // (the behaviour changed).

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:pPr", "marL",
                "1080000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf151622_oleIcon)
{
    createSdImpressDoc("odp/ole_icon.odp");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: prop
    // - In ..., XPath '//p:oleObj' no attribute 'showAsIcon' exist
    // i.e. show as icon option wasn't exported.
    assertXPath(pXml, "//p:oleObj", "showAsIcon", "1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf152436)
{
    createSdImpressDoc("pptx/ole-emf_min.pptx");
    saveAndReload("Impress Office Open XML");

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testLinkedOLE)
{
    createSdImpressDoc("odp/linked_ole.odp");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In<>, XPath '//p:oleObj' number of nodes is incorrect
    // i.e. the linked ole object wasn't exported.
    assertXPath(pXml, "//p:oleObj", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf102261_testParaTabStopDefaultDistance)
{
    createSdImpressDoc("pptx/tdf102261_testParaTabStopDefaultDistance.pptx");
    saveAndReload("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    {
        uno::Reference<beans::XPropertySet> xPropSet(getParagraphFromShape(0, xShape),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32{ 1270 },
            xPropSet->getPropertyValue("ParaTabStopDefaultDistance").get<sal_Int32>());
    }
    {
        uno::Reference<beans::XPropertySet> xPropSet(getParagraphFromShape(1, xShape),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32{ 2540 },
            xPropSet->getPropertyValue("ParaTabStopDefaultDistance").get<sal_Int32>());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTableCellVerticalPropertyRoundtrip)
{
    createSdImpressDoc("pptx/tcPr-vert-roundtrip.pptx");
    saveAndReload("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    assertXPath(pXml, "(//a:tcPr)[1]", "vert", "vert");
    assertXPath(pXml, "(//a:tcPr)[2]", "vert", "vert270");
    assertXPath(pXml, "(//a:tcPr)[3]", "vert", "wordArtVert");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf159931_slideLayouts)
{
    createSdImpressDoc("odp/repeatBitmapMode.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocRels1 = parseExport("ppt/slides/_rels/slide1.xml.rels");
    xmlDocUniquePtr pXmlDocRels2 = parseExport("ppt/slides/_rels/slide2.xml.rels");

    assertXPath(pXmlDocRels1, "(/rels:Relationships/rels:Relationship[@Type='http://"
                              "schemas.openxmlformats.org/officeDocument/2006/relationships/"
                              "slideLayout'])");

    // the relative target e.g. "../slideLayouts/slideLayout2.xml"
    OUString sRelativeLayoutPath1
        = getXPathContent(pXmlDocRels1, "(/rels:Relationships/rels:Relationship[@Type='http://"
                                        "schemas.openxmlformats.org/officeDocument/2006/"
                                        "relationships/slideLayout'])/@Target");

    assertXPath(pXmlDocRels2, "(/rels:Relationships/rels:Relationship[@Type='http://"
                              "schemas.openxmlformats.org/officeDocument/2006/relationships/"
                              "slideLayout'])");

    // the relative target e.g. "../slideLayouts/slideLayout1.xml"
    OUString sRelativeLayoutPath2
        = getXPathContent(pXmlDocRels2, "(/rels:Relationships/rels:Relationship[@Type='http://"
                                        "schemas.openxmlformats.org/officeDocument/2006/"
                                        "relationships/slideLayout'])/@Target");

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // Check that the referenced slideLayout files exist
    // Without the accompanying fix in place, this test would have failed with:
    // equality assertion failed
    // - Expected: 1
    // - Actual  : 0
    // i.e. the referenced slideLayout file was missing on export.
    OUString sSlideLayoutName1 = sRelativeLayoutPath1.getToken(2, '/');
    OUString sSlideLayoutName2 = sRelativeLayoutPath2.getToken(2, '/');

    CPPUNIT_ASSERT_EQUAL(true,
                         bool(xNameAccess->hasByName("ppt/slideLayouts/" + sSlideLayoutName1)));
    CPPUNIT_ASSERT_EQUAL(true,
                         bool(xNameAccess->hasByName("ppt/slideLayouts/" + sSlideLayoutName2)));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
