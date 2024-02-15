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
#include <editeng/unoprnms.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

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
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect"_ostr,
                "l"_ostr, "20000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect"_ostr,
                "t"_ostr, "30000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect"_ostr,
                "r"_ostr, "20000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect"_ostr,
                "b"_ostr, "30000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf92222)
{
    createSdImpressDoc("pptx/tdf92222.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocTheme = parseExport("ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[1]"_ostr,
                "w"_ostr, "6350");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[2]"_ostr,
                "w"_ostr, "12700");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[3]"_ostr,
                "w"_ostr, "19050");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf129430)
{
    createSdImpressDoc("odp/tdf129430.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:pPr/a:lnSpc/a:spcPct"_ostr,
                "val"_ostr, "100000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf114848)
{
    createSdImpressDoc("pptx/tdf114848.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocTheme1 = parseExport("ppt/theme/theme1.xml");
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr"_ostr,
                "val"_ostr, "1f497d");
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
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[1]/a:pPr/a:buClr/a:srgbClr"_ostr,
                "val"_ostr, "227fc7");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[2]/a:pPr/a:buClr/a:srgbClr"_ostr,
                "val"_ostr, "227fc7");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf68759)
{
    createSdImpressDoc("odp/tdf68759.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off"_ostr,
                "x"_ostr, "1687320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off"_ostr,
                "y"_ostr, "1615320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm"_ostr, "rot"_ostr,
                "9600000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off"_ostr,
                "x"_ostr, "3847320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off"_ostr,
                "y"_ostr, "1614600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm"_ostr, "flipH"_ostr,
                "1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off"_ostr,
                "x"_ostr, "5934960");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off"_ostr,
                "y"_ostr, "1615320");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf127901)
{
    createSdImpressDoc("odp/tdf127901.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum"_ostr,
                "bright"_ostr, "70000");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum"_ostr,
                "contrast"_ostr, "-70000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:grayscl"_ostr,
                1);

    xmlDocUniquePtr pXmlDocContent3 = parseExport("ppt/slides/slide3.xml");
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:biLevel"_ostr,
                "thresh"_ostr, "50000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf48735)
{
    createSdImpressDoc("odp/tdf48735.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect"_ostr, "b"_ostr,
                "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect"_ostr, "l"_ostr,
                "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect"_ostr, "r"_ostr,
                "23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect"_ostr, "t"_ostr,
                "18842");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf90626)
{
    createSdImpressDoc("odp/tdf90626.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buSzPct"_ostr, "val"_ostr,
                "100000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buSzPct"_ostr, "val"_ostr,
                "150142");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buSzPct"_ostr, "val"_ostr,
                "100000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr/a:buSzPct"_ostr, "val"_ostr,
                "150142");
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
    OUString sScale = getXPath(pXmlDocContent,
                               "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:normAutofit"_ostr,
                               "fontScale"_ostr);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(81111), sScale.toInt32(), 1000);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testShapeAutofitPPTX)
{
    createSdImpressDoc("pptx/testShapeAutofit.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // TextAutoGrowHeight --> "Resize shape to fit text" --> true
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:spAutoFit"_ostr,
                1);
    // TextAutoGrowHeight --> "Resize shape to fit text" --> false
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testLegacyShapeAutofitPPTX)
{
    createSdImpressDoc("odp/testLegacyShapeAutofit.odp");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    CPPUNIT_ASSERT(pXmlDocContent);

    // Text in a legacy rectangle
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:noAutofit"_ostr,
                1);
    // Text in (closed) Polygon
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit"_ostr,
                1);
    // Text in a legacy ellipse
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:txBody/a:bodyPr/a:noAutofit"_ostr,
                1);
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
    assertXPath(pXmlDocContent, "//p:animMotion"_ostr, "origin"_ostr, "layout");
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
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);
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

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone"_ostr);
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
    assertXPath(pSlide1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone"_ostr);

    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buNone"_ostr);
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr"_ostr, "indent"_ostr,
                "0");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone"_ostr);
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr"_ostr, "indent"_ostr,
                "0");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buNone"_ostr);
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr"_ostr, "indent"_ostr,
                "0");

    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr"_ostr, "indent"_ostr,
                "0");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone"_ostr);
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr"_ostr, "indent"_ostr,
                "0");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr"_ostr, "indent"_ostr,
                "0");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf100348_convert_Fontwork2TextWarp)
{
    createSdImpressDoc("odp/tdf100348_Fontwork2TextWarp.odp");
    save("Impress Office Open XML");

    // Resulting pptx has to contain the TextWarp shape
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    static constexpr OString sPathStart(
        "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp"_ostr);
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
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr[@fromWordArt='1']"_ostr);

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
    static constexpr OString sPathStart1("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr);
    assertXPath(pXmlDocContent1, sPathStart1 + "[@prst='arc']");
    const OString sPathAdj1(sPathStart1 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj1' and  @fmla='val 20493903']");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj2' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill"_ostr);

    // slide 2 270° -> adj1 = 5400000, 180° -> adj2 = 10800000
    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    static constexpr OString sPathStart2("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr);
    assertXPath(pXmlDocContent2, sPathStart2 + "[@prst='chord']");
    const OString sPathAdj2(sPathStart2 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj1' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj2' and  @fmla='val 10800000']");

    // slide 3 120° -> adj1 = 12600000, 30° -> adj2 = 20946396
    xmlDocUniquePtr pXmlDocContent3 = parseExport("ppt/slides/slide3.xml");
    static constexpr OString sPathStart3("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr);
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
                "p:childTnLst[1]/p:set/p:cBhvr/p:tgtEl/p:spTgt"_ostr,
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
                "p:endCondLst[not(*)]"_ostr,
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
                "p:childTnLst/p:set[2]/p:to"_ostr,
                0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118783)
{
    createSdImpressDoc("odp/tdf118783.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    OUString sAttributeName
        = getXPathContent(pXmlDocContent, "//p:animRot/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
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
                        "p:childTnLst/p:set/p:cBhvr/p:attrNameLst/p:attrName"_ostr);
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
    assertXPath(pXmlDocContent, "(//p:animClr)[1]"_ostr, "clrSpc"_ostr, "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[1]//p:attrName"_ostr, "style.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[1]//p:to/a:srgbClr"_ostr, "val"_ostr, "ed1c24");

    assertXPath(pXmlDocContent, "(//p:animClr)[2]"_ostr, "clrSpc"_ostr, "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[2]//p:attrName"_ostr, "stroke.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[2]//p:to/a:srgbClr"_ostr, "val"_ostr, "333399");

    assertXPath(pXmlDocContent, "(//p:animClr)[3]"_ostr, "clrSpc"_ostr, "rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[3]//p:attrName"_ostr, "fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[3]//p:to/a:srgbClr"_ostr, "val"_ostr, "fcd3c1");

    assertXPath(pXmlDocContent, "(//p:animClr)[5]"_ostr, "clrSpc"_ostr, "hsl");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[5]//p:attrName"_ostr, "fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl"_ostr, "h"_ostr, "10800000");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl"_ostr, "s"_ostr, "0");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl"_ostr, "l"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118768)
{
    createSdImpressDoc("odp/tdf118768-brake.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:anim[1]"_ostr, "from"_ostr, "(-#ppt_w/2)");
    assertXPath(pXmlDocContent, "//p:anim[1]"_ostr, "to"_ostr, "(#ppt_x)");
    assertXPath(pXmlDocContent, "//p:anim[2]"_ostr, "from"_ostr, "0");

    assertXPath(pXmlDocContent, "//p:anim[2]"_ostr, "to"_ostr, "-1");
    assertXPath(pXmlDocContent, "//p:anim[2]/p:cBhvr/p:cTn"_ostr, "autoRev"_ostr, "1");

    assertXPath(pXmlDocContent, "//p:anim[3]"_ostr, "by"_ostr, "(#ppt_h/3+#ppt_w*0.1)");
    assertXPath(pXmlDocContent, "//p:anim[3]/p:cBhvr/p:cTn"_ostr, "autoRev"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118836)
{
    createSdImpressDoc("odp/tdf118836.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:animScale/p:by"_ostr, "x"_ostr, "250000");
    assertXPath(pXmlDocContent, "//p:animScale/p:by"_ostr, "y"_ostr, "250000");
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
    assertXPath(pXmlDocContent, "//p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp"_ostr, "prst"_ostr,
                "textArchUp");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:txBody/a:bodyPr/a:prstTxWarp"_ostr, "prst"_ostr,
                "textCircle");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:spPr/a:solidFill/a:srgbClr"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf128096)
{
    createSdImpressDoc("odp/tdf128096.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr"_ostr,
                "val"_ostr, "ffff00");

    // Check that underlined content is also highlighted
    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr"_ostr,
                "val"_ostr, "ffff00");
}
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf120573)
{
    createSdImpressDoc("pptx/tdf120573.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:audioFile"_ostr, 1);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile"_ostr, 0);

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../media/media1.wav'])[1]"_ostr,
                "Type"_ostr,
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");

    xmlDocUniquePtr pXmlContentType = parseExport("[Content_Types].xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.wav']"_ostr,
                "ContentType"_ostr, "audio/x-wav");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf119118)
{
    createSdImpressDoc("pptx/tdf119118.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent, "//p:iterate"_ostr, "type"_ostr, "lt");
    assertXPath(pXmlDocContent, "//p:tmAbs"_ostr, "val"_ostr, "200");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf99213)
{
    createSdImpressDoc("odp/tdf99213-target-missing.odp");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    // Number of nodes with p:attrNameLst was 3, including one that missed tgtEl
    assertXPath(pXmlDocContent, "//p:attrNameLst"_ostr, 2);
    // Timenode that miss its target element should be filtered.
    assertXPath(pXmlDocContent, "//p:attrNameLst/preceding-sibling::p:tgtEl"_ostr, 2);
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
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']"_ostr,
                "ContentType"_ostr,
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
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio1.wav']"_ostr,
                "ContentType"_ostr, "audio/x-wav");

    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio2.wav']"_ostr,
                "ContentType"_ostr, "audio/x-wav");

    xmlDocUniquePtr pDoc1 = parseExport("ppt/slides/slide1.xml");

    // tdf#124230 all nodes were under p:childTnLst, but event triggered nodes need
    // to be under p:subTnLst, especially for audio to work correctly.
    // Start condition: 0s after timenode id 5 begins.
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond"_ostr,
                "evt"_ostr, "begin");
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond"_ostr,
                "delay"_ostr, "0");
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond/p:tn"_ostr,
                "val"_ostr, "5");

    xmlDocUniquePtr pDoc2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pDoc2, "//p:transition/p:sndAc/p:stSnd/p:snd[@r:embed]"_ostr, 2);

    xmlDocUniquePtr pRels1 = parseExport("ppt/slides/_rels/slide1.xml.rels");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']"_ostr, "Type"_ostr,
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                "../media/audio1.wav");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf135843)
{
    createSdImpressDoc("pptx/tdf135843_export.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    static constexpr OString sPathStart(
        "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl"_ostr);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
