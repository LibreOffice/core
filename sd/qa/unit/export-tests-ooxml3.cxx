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
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

    int testTdf115005_FallBack_Images(bool bAddReplacementImages);
};

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf165262)
{
    createSdImpressDoc("ppt/tdf165262.ppt");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:custGeom");

    // glue points export
    assertXPath(pXmlDoc1,
                "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:custGeom/a:gdLst/a:gd[5]", "name",
                u"GluePoint1X");
    assertXPath(pXmlDoc1,
                "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:custGeom/a:gdLst/a:gd[5]", "fmla",
                u"*/ 15 w 21600");
    assertXPath(pXmlDoc1,
                "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:custGeom/a:gdLst/a:gd[6]", "name",
                u"GluePoint1Y");
    assertXPath(pXmlDoc1,
                "/p:sld/p:cSld/p:spTree/p:grpSp/p:sp[1]/p:spPr/a:custGeom/a:gdLst/a:gd[6]", "fmla",
                u"*/ 0 h 21600");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf164775_ConnectorShape)
{
    createSdImpressDoc("odp/tdf164775.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:stCxn", "idx",
                u"3");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:cxnSp/p:nvCxnSpPr/p:cNvCxnSpPr/a:endCxn", "idx",
                u"1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf153105)
{
    createSdImpressDoc("odp/tdf153105.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "l",
                u"20000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "t",
                u"30000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "r",
                u"20000");
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:bg/p:bgPr/a:blipFill/a:stretch/a:fillRect", "b",
                u"30000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf92222)
{
    createSdImpressDoc("pptx/tdf92222.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocTheme = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[1]", "w",
                u"6350");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[2]", "w",
                u"12700");
    assertXPath(pXmlDocTheme, "/a:theme/a:themeElements/a:fmtScheme/a:lnStyleLst/a:ln[3]", "w",
                u"19050");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf129430)
{
    createSdImpressDoc("odp/tdf129430.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[2]/a:pPr/a:lnSpc/a:spcPct",
                "val", u"100000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf114848)
{
    createSdImpressDoc("pptx/tdf114848.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocTheme1 = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPath(pXmlDocTheme1, "/a:theme/a:themeElements/a:clrScheme/a:dk2/a:srgbClr", "val",
                u"1f497d");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf147586)
{
    createSdImpressDoc("pptx/tdf147586.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Without the fix in place, this test would have failed with
    // - Expected: 227fc7
    // - Actual  : 4f4f4f
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[1]/a:pPr/a:buClr/a:srgbClr", "val",
                u"227fc7");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[2]/a:pPr/a:buClr/a:srgbClr", "val",
                u"227fc7");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf68759)
{
    createSdImpressDoc("odp/tdf68759.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off", "x",
                u"1687320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:spPr/a:xfrm/a:off", "y",
                u"1615320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm", "flipH", u"1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm", "rot", u"9600000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off", "x",
                u"3847320");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:spPr/a:xfrm/a:off", "y",
                u"1614600");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm", "flipH", u"1");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off", "x",
                u"5934960");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:pic[3]/p:spPr/a:xfrm/a:off", "y",
                u"1615320");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf127901)
{
    createSdImpressDoc("odp/tdf127901.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "bright",
                u"70000");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:lum", "contrast",
                u"-70000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPath(pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:grayscl", 1);

    xmlDocUniquePtr pXmlDocContent3 = parseExport(u"ppt/slides/slide3.xml"_ustr);
    assertXPath(pXmlDocContent3, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:blip/a:biLevel",
                "thresh", u"50000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf48735)
{
    createSdImpressDoc("odp/tdf48735.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "b",
                u"23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "l",
                u"23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "r",
                u"23627");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:pic/p:blipFill/a:srcRect", "t",
                u"18842");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf90626)
{
    createSdImpressDoc("odp/tdf90626.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buSzPct",
                "val", u"100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buSzPct",
                "val", u"150142");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buSzPct",
                "val", u"100000");
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr/a:buSzPct",
                "val", u"150142");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf107608)
{
    createSdImpressDoc("pptx/tdf107608.pptx");
    save(u"Impress Office Open XML"_ustr);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, aFillStyle);

    bool bBackgroundFill = false;
    xPropSet->getPropertyValue(u"FillBackground"_ustr) >>= bBackgroundFill;
    CPPUNIT_ASSERT(bBackgroundFill);

    Color nBackgroundColor;
    xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nBackgroundColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), nBackgroundColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf111786)
{
    // Export line transparency with the color

    createSdImpressDoc("pptx/tdf111786.pptx");
    save(u"Impress Office Open XML"_ustr);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    Color nLineColor;
    xPropSet->getPropertyValue(u"LineColor"_ustr) >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x3465A4), nLineColor);

    sal_Int16 nTransparency;
    xPropSet->getPropertyValue(u"LineTransparence"_ustr) >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(33), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testFontScale)
{
    createSdImpressDoc("pptx/font-scale.pptx");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Rounding errors possible, approximate value (+/- 1%)
    OUString sScale = getXPath(
        pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:normAutofit", "fontScale");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(85000), sScale.toInt32(), 1000);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testShapeAutofitPPTX)
{
    createSdImpressDoc("pptx/testShapeAutofit.pptx");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocContent);

    // TextAutoGrowHeight --> "Resize shape to fit text" --> true
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:spAutoFit", 1);
    // TextAutoGrowHeight --> "Resize shape to fit text" --> false
    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr/a:noAutofit", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testLegacyShapeAutofitPPTX)
{
    createSdImpressDoc("odp/testLegacyShapeAutofit.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
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
    save(u"Impress Office Open XML"_ustr);

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
    save(u"Impress Office Open XML"_ustr);
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
    save(u"impress8"_ustr);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(m_xContext, maTempFile.GetURL());

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
    save(u"impress8"_ustr);

    // additional checks of the output file
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(m_xContext, maTempFile.GetURL());

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
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "//p:animMotion", "origin", u"layout");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf130058)
{
    createSdImpressDoc("pptx/tdf130058.pptx");
    save(u"Impress Office Open XML"_ustr);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    bool bHasShadow = false;
    xShape->getPropertyValue(u"Shadow"_ustr) >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    double fShadowDist = 0.0;
    xShape->getPropertyValue(u"ShadowXDistance"_ustr) >>= fShadowDist;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(0), fShadowDist);
    xShape->getPropertyValue(u"ShadowYDistance"_ustr) >>= fShadowDist;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(141), fShadowDist);
    Color nColor;
    xShape->getPropertyValue(u"ShadowColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);
    sal_Int32 nTransparency = 0;
    xShape->getPropertyValue(u"ShadowTransparence"_ustr) >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(57), nTransparency);
    double fShadowSizeX = 0.0;
    xShape->getPropertyValue(u"ShadowSizeX"_ustr) >>= fShadowSizeX;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000), fShadowSizeX);
    double fShadowSizeY = 0.0;
    xShape->getPropertyValue(u"ShadowSizeY"_ustr) >>= fShadowSizeY;
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(1000), fShadowSizeY);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf111789)
{
    // Shadow properties were not exported for text shapes.

    createSdImpressDoc("pptx/tdf111789.pptx");
    save(u"Impress Office Open XML"_ustr);

    // First text shape has some shadow
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
        bool bHasShadow = false;
        xShape->getPropertyValue(u"Shadow"_ustr) >>= bHasShadow;
        CPPUNIT_ASSERT(bHasShadow);
        double fShadowDist = 0.0;
        xShape->getPropertyValue(u"ShadowXDistance"_ustr) >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(274), fShadowDist);
        xShape->getPropertyValue(u"ShadowYDistance"_ustr) >>= fShadowDist;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(274), fShadowDist);
        Color nColor;
        xShape->getPropertyValue(u"ShadowColor"_ustr) >>= nColor;
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);
        sal_Int32 nTransparency = 0;
        xShape->getPropertyValue(u"ShadowTransparence"_ustr) >>= nTransparency;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7), nTransparency);
        double fShadowSizeX = 0.0;
        xShape->getPropertyValue(u"ShadowSizeX"_ustr) >>= fShadowSizeX;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(100000), fShadowSizeX);
        double fShadowSizeY = 0.0;
        xShape->getPropertyValue(u"ShadowSizeY"_ustr) >>= fShadowSizeY;
        CPPUNIT_ASSERT_EQUAL(static_cast<double>(100000), fShadowSizeY);
    }

    // Second text shape has no shadow
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));
        bool bHasShadow = false;
        xShape->getPropertyValue(u"Shadow"_ustr) >>= bHasShadow;
        CPPUNIT_ASSERT(!bHasShadow);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf145162)
{
    createSdImpressDoc("pptx/tdf145162.pptx");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");
    // Before the fix, that tag was missing so PP put bullet to each para.
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testZeroIndentExport)
{
    // Load the bugdoc and save to pptx then.

    createSdImpressDoc("odp/testZeroIndent.odp");
    save(u"Impress Office Open XML"_ustr);
    // There are 3 slides, get them
    xmlDocUniquePtr pSlide1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    xmlDocUniquePtr pSlide2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    xmlDocUniquePtr pSlide3 = parseExport(u"ppt/slides/slide3.xml"_ustr);

    CPPUNIT_ASSERT(pSlide1);
    CPPUNIT_ASSERT(pSlide2);
    CPPUNIT_ASSERT(pSlide3);

    // Each slide has 3 paragraphs, one full line, an empty and a normal para.
    // Check the indent and bullet. These have to match with PP. Before the fix,
    // they were different.
    assertXPath(pSlide1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");

    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr/a:buNone");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr", "indent", u"0");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr", "indent", u"0");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr/a:buNone");
    assertXPath(pSlide2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr", "indent", u"0");

    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr", "indent", u"0");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr/a:buNone");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr", "indent", u"0");
    assertXPath(pSlide3, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr", "indent", u"0");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf100348_convert_Fontwork2TextWarp)
{
    createSdImpressDoc("odp/tdf100348_Fontwork2TextWarp.odp");
    save(u"Impress Office Open XML"_ustr);

    // Resulting pptx has to contain the TextWarp shape
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart(
        "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp"_ostr);
    assertXPath(pXmlDocContent, sPathStart + "[@prst='textWave1']");
    const OString sPathAdj(sPathStart + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj1' and  @fmla='val 18750']");
    assertXPath(pXmlDocContent, sPathAdj + "[@name='adj2' and  @fmla='val -7500']");

    // Reloading has to get the Fontwork shape back
    // TextPath makes a custom shape to a Fontwork shape, so must exist
    uno::Reference<beans::XPropertySet> xShapeWavePropSet(getShapeFromPage(0, 0));
    auto aGeomPropSeq = xShapeWavePropSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);
    OUString sName = u"TextPath"_ustr;
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
    CPPUNIT_ASSERT_EQUAL(u"fontwork-wave"_ustr, sOwnName);

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
    save(u"Impress Office Open XML"_ustr);

    // Error was, that attribute 'fromWordArt' was ignored
    // ensure, resulting pptx has fromWordArt="1" on textArchDown shape
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
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
    save(u"Impress Office Open XML"_ustr);

    // slide 1 45° -> adj1 = 20493903, 270° -> adj2 = 5400000, <a:noFill/> exists
    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPathStart1("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr);
    assertXPath(pXmlDocContent1, sPathStart1 + "[@prst='arc']");
    const OString sPathAdj1(sPathStart1 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj1' and  @fmla='val 20493903']");
    assertXPath(pXmlDocContent1, sPathAdj1 + "[@name='adj2' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:noFill");

    // slide 2 270° -> adj1 = 5400000, 180° -> adj2 = 10800000
    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    static constexpr OString sPathStart2("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr);
    assertXPath(pXmlDocContent2, sPathStart2 + "[@prst='chord']");
    const OString sPathAdj2(sPathStart2 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj1' and  @fmla='val 5400000']");
    assertXPath(pXmlDocContent2, sPathAdj2 + "[@name='adj2' and  @fmla='val 10800000']");

    // slide 3 120° -> adj1 = 12600000, 30° -> adj2 = 20946396
    xmlDocUniquePtr pXmlDocContent3 = parseExport(u"ppt/slides/slide3.xml"_ustr);
    static constexpr OString sPathStart3("/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:prstGeom"_ostr);
    assertXPath(pXmlDocContent3, sPathStart3 + "[@prst='pie']");
    const OString sPathAdj3(sPathStart3 + "/a:avLst/a:gd");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj1' and  @fmla='val 12600000']");
    assertXPath(pXmlDocContent3, sPathAdj3 + "[@name='adj2' and  @fmla='val 20946396']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf104792)
{
    createSdImpressDoc("pptx/tdf104792-smart-art-animation.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst[1]/p:seq/p:cTn/p:childTnLst[1]/"
                "p:par[1]/p:cTn/p:childTnLst[1]/p:par/p:cTn/p:childTnLst[1]/p:par/p:cTn/"
                "p:childTnLst[1]/p:set/p:cBhvr/p:tgtEl/p:spTgt",
                1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf90627)
{
    createSdImpressDoc("odp/tdf90627.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
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
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide2.xml"_ustr);
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
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString sAttributeName
        = getXPathContent(pXmlDocContent, "//p:animRot/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(u"r"_ustr, sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf104789)
{
    createSdImpressDoc("pptx/tdf104789.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString sAttributeName = getXPathContent(
        pXmlDocContent, "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                        "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                        "p:childTnLst/p:set/p:cBhvr/p:attrNameLst/p:attrName");
    CPPUNIT_ASSERT_EQUAL(u"style.opacity"_ustr, sAttributeName);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testOpenDocumentAsReadOnly)
{
    createSdImpressDoc("pptx/open-as-read-only.pptx");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::DrawDocShell* pDocShell = pXImpressDocument->GetDocShell();
    CPPUNIT_ASSERT(pDocShell->IsSecurityOptOpenReadOnly());
    save(u"Impress Office Open XML"_ustr);
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDocShell = pXImpressDocument->GetDocShell();
    CPPUNIT_ASSERT(pDocShell->IsSecurityOptOpenReadOnly());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118835)
{
    createSdImpressDoc("odp/tdf118835.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "(//p:animClr)[1]", "clrSpc", u"rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[1]//p:attrName", u"style.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[1]//p:to/a:srgbClr", "val", u"ed1c24");

    assertXPath(pXmlDocContent, "(//p:animClr)[2]", "clrSpc", u"rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[2]//p:attrName", u"stroke.color");
    assertXPath(pXmlDocContent, "(//p:animClr)[2]//p:to/a:srgbClr", "val", u"333399");

    assertXPath(pXmlDocContent, "(//p:animClr)[3]", "clrSpc", u"rgb");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[3]//p:attrName", u"fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[3]//p:to/a:srgbClr", "val", u"fcd3c1");

    assertXPath(pXmlDocContent, "(//p:animClr)[5]", "clrSpc", u"hsl");
    assertXPathContent(pXmlDocContent, "(//p:animClr)[5]//p:attrName", u"fillcolor");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "h", u"10800000");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "s", u"0");
    assertXPath(pXmlDocContent, "(//p:animClr)[5]//p:by/p:hsl", "l", u"0");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118768)
{
    createSdImpressDoc("odp/tdf118768-brake.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "//p:anim[1]", "from", u"(-#ppt_w/2)");
    assertXPath(pXmlDocContent, "//p:anim[1]", "to", u"(#ppt_x)");
    assertXPath(pXmlDocContent, "//p:anim[2]", "from", u"0");

    assertXPath(pXmlDocContent, "//p:anim[2]", "to", u"-1");
    assertXPath(pXmlDocContent, "//p:anim[2]/p:cBhvr/p:cTn", "autoRev", u"1");

    assertXPath(pXmlDocContent, "//p:anim[3]", "by", u"(#ppt_h/3+#ppt_w*0.1)");
    assertXPath(pXmlDocContent, "//p:anim[3]/p:cBhvr/p:cTn", "autoRev", u"1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf118836)
{
    createSdImpressDoc("odp/tdf118836.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "x", u"250000");
    assertXPath(pXmlDocContent, "//p:animScale/p:by", "y", u"250000");
}

static double getAdjustmentValue(const uno::Reference<beans::XPropertySet>& xSet)
{
    auto aGeomPropSeq = xSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                            .get<uno::Sequence<beans::PropertyValue>>();
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

    auto aGeomPropSeq = xSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                            .get<uno::Sequence<beans::PropertyValue>>();
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
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "//p:sp[1]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", u"textArchUp");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:txBody/a:bodyPr/a:prstTxWarp", "prst", u"textCircle");
    assertXPath(pXmlDocContent, "//p:sp[14]/p:spPr/a:solidFill/a:srgbClr", 0);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf128096)
{
    createSdImpressDoc("odp/tdf128096.odp");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val",
                u"ffff00");

    // Check that underlined content is also highlighted
    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPath(pXmlDocContent2,
                "//p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:highlight/a:srgbClr", "val",
                u"ffff00");
}
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf120573)
{
    createSdImpressDoc("pptx/tdf120573.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:audioFile", 1);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:pic/p:nvPicPr/p:nvPr/a:videoFile", 0);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../media/media1.wav'])[1]", "Type",
                u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");

    xmlDocUniquePtr pXmlContentType = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/media1.wav']",
                "ContentType", u"audio/x-wav");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf119118)
{
    createSdImpressDoc("pptx/tdf119118.pptx");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent, "//p:iterate", "type", u"lt");
    assertXPath(pXmlDocContent, "//p:tmAbs", "val", u"200");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf99213)
{
    createSdImpressDoc("odp/tdf99213-target-missing.odp");
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
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
    save(u"Impress Office Open XML Template"_ustr);

    // Load and check content type
    xmlDocUniquePtr pContentTypes = parseExport(u"[Content_Types].xml"_ustr);
    CPPUNIT_ASSERT(pContentTypes);
    assertXPath(pContentTypes,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/presentation.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.presentationml.template.main+xml");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf44223)
{
    createSdImpressDoc("pptx/tdf44223.pptx");
    save(u"Impress Office Open XML"_ustr);

    std::unique_ptr<SvStream> const pStream1(
        parseExportStream(maTempFile.GetURL(), u"ppt/media/audio1.wav"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(11140), pStream1->remainingSize());

    std::unique_ptr<SvStream> const pStream2(
        parseExportStream(maTempFile.GetURL(), u"ppt/media/audio2.wav"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(28074), pStream2->remainingSize());

    xmlDocUniquePtr pXmlContentType = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio1.wav']",
                "ContentType", u"audio/x-wav");

    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/media/audio2.wav']",
                "ContentType", u"audio/x-wav");

    xmlDocUniquePtr pDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // tdf#124230 all nodes were under p:childTnLst, but event triggered nodes need
    // to be under p:subTnLst, especially for audio to work correctly.
    // Start condition: 0s after timenode id 5 begins.
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "evt",
                u"begin");
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond", "delay", u"0");
    assertXPath(pDoc1, "//p:subTnLst/p:audio/p:cMediaNode/p:cTn/p:stCondLst/p:cond/p:tn", "val",
                u"5");

    xmlDocUniquePtr pDoc2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPath(pDoc2, "//p:transition/p:sndAc/p:stSnd/p:snd[@r:embed]", 2);

    xmlDocUniquePtr pRels1 = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Type",
                u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/audio");
    assertXPath(pRels1, "//rels:Relationship[@Id='rId1']", "Target", u"../media/audio1.wav");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest3, testTdf135843)
{
    createSdImpressDoc("pptx/tdf135843_export.pptx");
    save(u"Impress Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
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
