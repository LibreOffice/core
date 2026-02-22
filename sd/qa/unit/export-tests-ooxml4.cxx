/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <tools/color.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <officecfg/Office/Common.hxx>
#include <test/commontesttools.hxx>

#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <xmloff/autolayout.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <sdpage.hxx>

class SdOOXMLExportTest4 : public SdModelTestBase
{
public:
    SdOOXMLExportTest4()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf160591)
{
    createSdImpressDoc("pptx/tdf160591.pptx");
    save(TestFilter::PPTX);

    // Char scheme color
    xmlDocUniquePtr pXmlDoc1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc1,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                u"lt2");

    // Master slide ColorMap
    xmlDocUniquePtr pXmlDoc2 = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDoc2, "/p:sldMaster/p:clrMap", "bg1", u"dk1");
    assertXPath(pXmlDoc2, "/p:sldMaster/p:clrMap", "tx1", u"lt1");
    assertXPath(pXmlDoc2, "/p:sldMaster/p:clrMap", "bg2", u"dk2");
    assertXPath(pXmlDoc2, "/p:sldMaster/p:clrMap", "tx2", u"lt2");

    // Master slide background
    assertXPath(pXmlDoc2, "/p:sldMaster/p:cSld/p:bg/p:bgPr/a:solidFill/a:schemeClr", "val", u"dk1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testSmartArtPreserve)
{
    createSdImpressDoc("pptx/smartart-preserve.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr");
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/dgm:relIds");
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/p:extLst/p:ext",
                "uri", u"{D42A27DB-BD31-4B8C-83A1-F6EECF244321}");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/"
                         "p:extLst/p:ext/p14:modId");

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/layout1.xml'])[1]", "Type",
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramLayout");
    assertXPath(
        pXmlDocRels, "(/rels:Relationships/rels:Relationship[@Target='../diagrams/data1.xml'])[1]",
        "Type", u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/colors1.xml'])[1]", "Type",
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramColors");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/quickStyle1.xml'])[1]", "Type",
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramQuickStyle");

    xmlDocUniquePtr pXmlContentType = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/layout1.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/data1.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/colors1.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml");
    assertXPath(
        pXmlContentType,
        "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/quickStyle1.xml']",
        "ContentType", u"application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf168755_anim_on_SmartArt)
{
    createSdImpressDoc("pptx/tdf168755_anim_on_SmartArt.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString aDiagramId = getXPath(
        pXmlDocContent, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr", "id");
    OUString aSpTgtId
        = getXPath(pXmlDocContent,
                   "//p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                   "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                   "p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt",
                   "spid");
    // Before the fix this would fail, as the "spid" attribute of the target was -1
    CPPUNIT_ASSERT_MESSAGE("Shape id in animation target can't be -1", aSpTgtId != "-1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape id in animation target differs from original", aDiagramId,
                                 aSpTgtId);
    // Repeat the same with a slightly different path (contains "animEffect" instead of "set")
    aSpTgtId
        = getXPath(pXmlDocContent,
                   "//p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                   "p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                   "p:childTnLst/p:animEffect/p:cBhvr/p:tgtEl/p:spTgt",
                   "spid");
    CPPUNIT_ASSERT_MESSAGE("Shape id in animation target can't be -1", aSpTgtId != "-1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape id in animation target differs from original", aDiagramId,
                                 aSpTgtId);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testtdf169675_3d_object_anim)
{
    createSdImpressDoc("odp/tdf169675_3d_object_anim.odp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    OUString aSpTgtId
        = getXPath(pXmlDoc,
                   "//p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/"
                   "p:par[1]/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/"
                   "p:childTnLst/p:set/p:cBhvr/p:tgtEl/p:spTgt",
                   "spid");
    // Before the fix spid would be -1, because animations for not exported targets (3D objects)
    // would be written, now those are omitted
    CPPUNIT_ASSERT_MESSAGE("Shape id in animation target can't be -1", aSpTgtId != "-1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125346)
{
    // There are two themes in the test document, make sure we use the right theme

    createSdImpressDoc("pptx/tdf125346.pptx");
    save(TestFilter::PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

    Color nFillColor;
    xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125346_2)
{
    // There are two themes in the test document, make sure we use the right theme
    // Test more slides with different themes

    createSdImpressDoc("pptx/tdf125346_2.pptx");
    save(TestFilter::PPTX);

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
    }

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 1));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x052F61), nFillColor);
    }

    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 2));
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_SET_THROW);

        drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
        xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, aFillStyle);

        Color nFillColor;
        xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x90C226), nFillColor);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125360)
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by shape style

    createSdImpressDoc("pptx/tdf125360.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    xShape->setPropertyValue(u"FillTransparence"_ustr, uno::Any(static_cast<sal_Int32>(23)));

    save(TestFilter::PPTX);

    xShape.set(getShapeFromPage(0, 0));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue(u"FillTransparence"_ustr) >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125360_1)
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme

    createSdImpressDoc("pptx/tdf125360_1.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    xShape->setPropertyValue(u"FillTransparence"_ustr, uno::Any(static_cast<sal_Int32>(23)));

    save(TestFilter::PPTX);

    xShape.set(getShapeFromPage(0, 0));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue(u"FillTransparence"_ustr) >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125360_2)
{
    // Check whether the changed fill transparency is exported correctly.
    // Color is defined by color scheme with a transparency

    createSdImpressDoc("pptx/tdf125360_2.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    sal_Int32 nTransparence = 0;
    xShape->getPropertyValue(u"FillTransparence"_ustr) >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(82), nTransparence);

    xShape->setPropertyValue(u"FillTransparence"_ustr, uno::Any(static_cast<sal_Int32>(23)));

    save(TestFilter::PPTX);

    xShape.set(getShapeFromPage(0, 0));

    nTransparence = 0;
    xShape->getPropertyValue(u"FillTransparence"_ustr) >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(23), nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125551)
{
    createSdImpressDoc("pptx/tdf125551.pptx");
    save(TestFilter::PPTX);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeBg(xGroupShape->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1024), xShapeBg->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(576), xShapeBg->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10816), xShapeBg->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8588), xShapeBg->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf136830)
{
    createSdImpressDoc("pptx/tdf136830.pptx");
    save(TestFilter::PPTX);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf126234)
{
    createSdImpressDoc("pptx/tdf126234.pptx");
    save(TestFilter::PPTX);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf126741)
{
    createSdImpressDoc("pptx/tdf126741.pptx");
    save(TestFilter::PPTX);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf127372)
{
    createSdImpressDoc("odp/tdf127372.odp");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    awt::Gradient2 aTransparenceGradient;
    xShape->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aTransparenceGradient;

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aTransparenceGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(0.0, aColorStops.getStopOffset(0));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops.getStopColor(0)));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops.getStopOffset(1), 1.0));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops.getStopColor(1)));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf127379)
{
    createSdImpressDoc("odp/tdf127379.odp");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    uno::Any aAny = xPropSet->getPropertyValue(u"Background"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());
    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;

    drawing::FillStyle aFillStyle(drawing::FillStyle_NONE);
    aXBackgroundPropSet->getPropertyValue(u"FillStyle"_ustr) >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, aFillStyle);

    awt::Gradient2 aGradient;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue(u"FillGradient"_ustr) >>= aGradient);

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(0.0, aColorStops.getStopOffset(0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(aColorStops.getStopColor(0)));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops.getStopOffset(1), 1.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x2a6099), Color(aColorStops.getStopColor(1)));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf98603)
{
    createSdImpressDoc("pptx/tdf98603.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    css::lang::Locale aLocale;
    xPropSet->getPropertyValue(u"CharLocaleComplex"_ustr) >>= aLocale;
    CPPUNIT_ASSERT_EQUAL(u"he"_ustr, aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(u"IL"_ustr, aLocale.Country);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf128213)
{
    createSdImpressDoc("pptx/tdf128213.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm", "rot");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf79082)
{
    createSdImpressDoc("pptx/tdf79082.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]", "pos",
                u"360000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]", "algn",
                u"l");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]", "pos",
                u"756000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]", "algn",
                u"l");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]", "pos",
                u"1440000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]", "algn",
                u"ctr");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]", "pos",
                u"1800000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]", "algn",
                u"r");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]", "pos",
                u"3240000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]", "algn",
                u"dec");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf129372)
{
    //Without the fix in place, it would crash at import time

    createSdImpressDoc("pptx/tdf129372.pptx");
    saveAndReload(TestFilter::PPTX);
    const SdrPage* pPage = GetPage(1);

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::CustomShape, pObj->GetObjIdentifier());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeGlowEffect)
{
    createSdImpressDoc("pptx/shape-glow-effect.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue(u"GlowEffectRadius"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 139700 EMU = 388.0556 mm/100
    Color nColor;
    xShape->getPropertyValue(u"GlowEffectColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFFC000), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue(u"GlowEffectTransparency"_ustr) >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeTextGlowEffect)
{
    createSdImpressDoc("pptx/shape-text-glow-effect.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue(u"GlowTextEffectRadius"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(706), nRadius); // 20 pt = 706 mm/100
    Color nColor;
    xShape->getPropertyValue(u"GlowTextEffectColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4EA72E), nColor);
    sal_Int16 nTransparency;
    xShape->getPropertyValue(u"GlowTextEffectTransparency"_ustr) >>= nTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), nTransparency);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testUnderline)
{
    createSdImpressDoc("underline.fodp");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropSet->getPropertyValue(u"CharUnderlineColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nColor);

    sal_Int16 nUnderline;
    xPropSet->getPropertyValue(u"CharUnderline"_ustr) >>= nUnderline;
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::DOUBLE, nUnderline);

    saveAndReload(TestFilter::PPTX);

    xShape.set(getShapeFromPage(0, 0));
    xParagraph.set(getParagraphFromShape(0, xShape));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xPropSet.set(xRun, uno::UNO_QUERY_THROW);

    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);

    xPropSet->getPropertyValue(u"CharUnderlineColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nColor);

    xPropSet->getPropertyValue(u"CharUnderline"_ustr) >>= nUnderline;
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::DOUBLE, nUnderline);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf119087)
{
    createSdImpressDoc("pptx/tdf119087.pptx");
    saveAndReload(TestFilter::PPTX);
    // This would fail both on export validation, and reloading the saved pptx file.

    // Get first paragraph of the text
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nColor = COL_AUTO;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x00B050), nColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf131554)
{
    createSdImpressDoc("pptx/tdf131554.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5622), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(13251), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf132282)
{
    createSdImpressDoc("pptx/tdf132282.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // Without the fix in place, the position would be 0,0, height = 1 and width = 1
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1736), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(763), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(30523), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2604), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf132201EffectOrder)
{
    createSdImpressDoc("pptx/effectOrder.pptx");
    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst", 2);
    // The relative order of effects is important: glow must be before shadow
    CPPUNIT_ASSERT_EQUAL(0, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "glow"));
    CPPUNIT_ASSERT_EQUAL(1, getXPathPosition(pXmlDocContent,
                                             "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst",
                                             "outerShdw"));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeSoftEdgeEffect)
{
    createSdImpressDoc("pptx/shape-soft-edges.pptx");
    saveAndReload(TestFilter::PPTX);
    auto xShapeProps(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShapeProps->getPropertyValue(u"SoftEdgeRadius"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRadius); // 18 pt

    xShapeProps.set(getShapeFromPage(1, 0));
    xShapeProps->getPropertyValue(u"SoftEdgeRadius"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), nRadius); // 10 pt
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeShadowBlurEffect)
{
    createSdImpressDoc("pptx/shape-blur-effect.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    bool bHasShadow = false;
    xShape->getPropertyValue(u"Shadow"_ustr) >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    sal_Int32 nRadius = -1;
    xShape->getPropertyValue(u"ShadowBlur"_ustr) >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRadius); // 11 pt
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf119223)
{
    createSdImpressDoc("odp/tdf119223.odp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeCustomShape']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomePicture']");

    assertXPath(pXmlDocRels, "//mc:Choice/p:sp/p:nvSpPr/p:cNvPr[@name='SomeFormula']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeLine']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTextbox']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTable']");

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeGroup']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf128213ShapeRot)
{
    createSdImpressDoc("pptx/tdf128213-shaperot.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d");
    assertXPath(pXmlDocRels,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot", "rev",
                u"5400000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125560_textDeflate)
{
    createSdImpressDoc("pptx/ShapePlusImage.pptx");

    // This problem did not affect the pptx export, only the ODP so assert that
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry",
                "type", u"mso-spt161");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125560_textInflateTop)
{
    createSdImpressDoc("pptx/ShapeTextInflateTop.pptx");

    // This problem did not affect the pptx export, only the ODP so assert that
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry",
                "type", u"mso-spt164");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf142235_TestPlaceholderTextAlignment)
{
    createSdImpressDoc("odp/placeholder-box-textalignment.odp");

    save(TestFilter::PPTX);

    xmlDocUniquePtr pXml1 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    xmlDocUniquePtr pXml2 = parseExport(u"ppt/slides/slide3.xml"_ustr);

    // Without the fix in place many of these asserts failed, because alignment was bad.

    assertXPath(pXml1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", u"t");
    assertXPath(pXml2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", u"t");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf96061_textHighlight)
{
    createSdImpressDoc("pptx/tdf96061.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph1(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun1(getRunFromParagraph(0, xParagraph1));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun1, uno::UNO_QUERY_THROW);

    Color aColor;
    xPropSet1->getPropertyValue(u"CharBackColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aColor);

    uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(1, xShape));
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    xPropSet2->getPropertyValue(u"CharBackColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, aColor);

    saveAndReload(TestFilter::PPTX);

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph3(getParagraphFromShape(0, xShape2));
    uno::Reference<text::XTextRange> xRun3(getRunFromParagraph(0, xParagraph3));
    uno::Reference<beans::XPropertySet> xPropSet3(xRun3, uno::UNO_QUERY_THROW);
    xPropSet3->getPropertyValue(u"CharBackColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aColor);

    uno::Reference<text::XTextRange> const xParagraph4(getParagraphFromShape(1, xShape2));
    uno::Reference<text::XTextRange> xRun4(getRunFromParagraph(0, xParagraph4));
    uno::Reference<beans::XPropertySet> xPropSet4(xRun4, uno::UNO_QUERY_THROW);
    xPropSet4->getPropertyValue(u"CharBackColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, aColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf143222_embeddedWorksheet)
{
    // Check import of embedded worksheet in slide.

    createSdImpressDoc("pptx/tdf143222.pptx");

    const SdrPage* pPage = GetPage(1);
    const SdrOle2Obj* pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object", pOleObj != nullptr);

    // Without the fix we lost the graphic of ole object.
    const Graphic* pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic", pGraphic != nullptr);
    CPPUNIT_ASSERT_MESSAGE("no graphic", !pGraphic->IsNone());

    // Check export of embedded worksheet in slide.
    saveAndReload(TestFilter::PPTX);

    pPage = GetPage(1);
    pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object after the export", pOleObj != nullptr);

    pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic after the export", pGraphic != nullptr);
    CPPUNIT_ASSERT_MESSAGE("no graphic after the export", !pGraphic->IsNone());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf143315)
{
    createSdImpressDoc("ppt/tdf143315-WordartWithoutBullet.ppt");

    save(TestFilter::PPTX);

    xmlDocUniquePtr pXml = parseExport(u"ppt/slides/slide1.xml"_ustr);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf147121)
{
    // Get the bugdoc
    createSdImpressDoc("pptx/tdf147121.pptx");

    // Get the second line props of the placeholder
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xRun(
        getRunFromParagraph(2, getParagraphFromShape(0, xShape)), uno::UNO_QUERY_THROW);

    // Save the font size
    const auto nFontSizeBefore = xRun->getPropertyValue(u"CharHeight"_ustr).get<float>() * 100;

    save(TestFilter::PPTX);

    // Parse the export
    xmlDocUniquePtr pXml = parseExport(u"ppt/slides/slide1.xml"_ustr);
    const auto nFontSizeAfter
        = getXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:br[1]/a:rPr", "sz").toFloat();

    // The font size was not saved before now it must be equal with the saved one.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font size", nFontSizeBefore, nFontSizeAfter);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf140912_PicturePlaceholder)
{
    // Given a graphic placeholder with a custom prompt:
    createSdImpressDoc("pptx/tdfpictureplaceholder.pptx");

    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0));
    bool isEmptyPresentationObject = false;
    // Without the fix, it would not be imported as empty presentation object;
    // the text would be treated as its content.
    xShapeProps->getPropertyValue(u"IsEmptyPresentationObject"_ustr) >>= isEmptyPresentationObject;
    CPPUNIT_ASSERT(isEmptyPresentationObject);

    // If we supported custom prompt text, here we would also test "String" property,
    // which would be equal to "Insert Image". See first tests: testCustomPromptTexts
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testEnhancedPathViewBox)
{
    createSdImpressDoc("odp/tdf147978_enhancedPath_viewBox.odp");
    saveAndReload(TestFilter::PPTX);
    auto xShapeProps(getShapeFromPage(0, 0));
    awt::Rectangle aBoundRectangle;
    xShapeProps->getPropertyValue(u"BoundRect"_ustr) >>= aBoundRectangle;
    // The shape has a Bézier curve which does not touch the right edge. Prior to the fix the curve
    // was stretched to touch the edge, resulting in 5098 curve width instead of 2045.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2045), aBoundRectangle.Width);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf74670)
{
    createSdImpressDoc("odp/tdf74670.odp");
    save(TestFilter::PPTX);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf169911_exporting_animations_without_target)
{
    createSdImpressDoc("odp/tdf169911.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 28
    // - validation error in OOXML export: Errors: 28
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf109169_OctagonBevel)
{
    // The document has a shape 'Octagon Bevel'. It consists of an octagon with 8 points and eight
    // facets with 4 points each, total 8+8*4=40 points. Without the patch it was exported as
    // rectangle and thus had 4 points.

    createSdImpressDoc("odp/tdf109169_Octagon.odp");
    saveAndReload(TestFilter::PPTX);

    auto xPropSet(getShapeFromPage(0, 0));
    auto aGeomPropSeq = xPropSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                            .get<uno::Sequence<beans::PropertyValue>>();
    comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);
    auto aPathSeq((aCustomShapeGeometry[u"Path"_ustr]).get<uno::Sequence<beans::PropertyValue>>());
    comphelper::SequenceAsHashMap aPath(aPathSeq);
    auto aCoordinates((aPath[u"Coordinates"_ustr])
                          .get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(40), aCoordinates.getLength());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf109169_DiamondBevel)
{
    // The document has a shape 'Diamond Bevel'. It consists of a diamond with 4 points and four
    // facets with 4 points each, total 4+4*4=20 points. Without the patch it was exported as
    // rectangle and thus had 4 points.

    createSdImpressDoc("odp/tdf109169_Diamond.odp");
    saveAndReload(TestFilter::PPTX);

    auto xPropSet(getShapeFromPage(0, 0));
    auto aGeomPropSeq = xPropSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                            .get<uno::Sequence<beans::PropertyValue>>();
    comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);
    auto aPathSeq((aCustomShapeGeometry[u"Path"_ustr]).get<uno::Sequence<beans::PropertyValue>>());
    comphelper::SequenceAsHashMap aPath(aPathSeq);
    auto aCoordinates((aPath[u"Coordinates"_ustr])
                          .get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aCoordinates.getLength());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf144092_emptyShapeTextProps)
{
    // Document contains one shape and one table. Both without any text but with
    // text properties contained inside endParaRPr - The import and export
    // of endParaRPr for empty cells and shapes are tested here
    createSdImpressDoc("pptx/tdf144092-emptyShapeTextProps.pptx");
    saveAndReload(TestFilter::PPTX);

    Color aColor;
    // check text properties of empty shape
    uno::Reference<beans::XPropertySet> xRectShapeProps(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Calibri"_ustr,
                         xRectShapeProps->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    CPPUNIT_ASSERT_EQUAL(float(196),
                         xRectShapeProps->getPropertyValue(u"CharHeight"_ustr).get<float>());
    xRectShapeProps->getPropertyValue(u"CharColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x70AD47), aColor);

    const SdrPage* pPage = GetPage(1);
    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;

    // check text properties of empty cells
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"CharColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"CharColor"_ustr) >>= aColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x70AD47), aColor);
    CPPUNIT_ASSERT_EQUAL(float(96), xCell->getPropertyValue(u"CharHeight"_ustr).get<float>());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf149551_tbrl90)
{
    // The document contains a shape with writing mode TB_RL90. That is the same as vert="vert" in
    // OOXML. Without the patch it was not possible to use this writing mode at all.

    createSdImpressDoc("odp/tdf149551_tbrl90.odp");

    // Test, that the shape has writing mode TB_RL90.
    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode;
    xShapeProps->getPropertyValue(u"WritingMode"_ustr) >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL90, eWritingMode);

    // Test, that it is exported to vert="vert"
    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:bodyPr", "vert", u"vert");

    // Test, that the shape has writing mode TB_RL90 after read from pptx
    uno::Reference<beans::XPropertySet> xShapeProps2(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode2;
    xShapeProps2->getPropertyValue(u"WritingMode"_ustr) >>= eWritingMode2;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL90, eWritingMode2);

    // Test, that it is written to odp with loext:writing-mode="tb-rl90"
    save(TestFilter::ODP);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='gr1']/style:graphic-properties[@loext:writing-mode='tb-rl90']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf149551_btlr)
{
    // The document contains a shape with writing mode BT_LR. That is the same as vert="vert270" in
    // OOXML. Without the patch it was not possible to use this writing mode at all for shapes.

    createSdImpressDoc("odp/tdf149551_btlr.odp");

    // Test, that the shape has writing mode BT_LR.
    uno::Reference<beans::XPropertySet> xShapeProps(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode;
    xShapeProps->getPropertyValue(u"WritingMode"_ustr) >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, eWritingMode);

    // Test, that it is exported to vert="vert270"
    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:bodyPr", "vert", u"vert270");

    // Test, that the shape has writing mode BT_LR after read from pptx
    uno::Reference<beans::XPropertySet> xShapeProps2(getShapeFromPage(0, 0));
    sal_Int16 eWritingMode2;
    xShapeProps2->getPropertyValue(u"WritingMode"_ustr) >>= eWritingMode2;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, eWritingMode2);

    // Test, that it is written to odp with loext:writing-mode="bt-lr"
    save(TestFilter::ODP);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='gr1']/style:graphic-properties[@loext:writing-mode='bt-lr']");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf94122_autoColor)
{
    // Document contains three pages, with different scenarios for automatic
    // color export to pptx.
    // - First page: Page background light, automatic colored text on a FillType_NONE shape
    // - Second page: Page background dark, automatic colored text on a FillType_NONE shape
    // - Third page: Page background light, automatic colored text on a dark colored fill
    //   and another automatic colored text on a light colored fill

    createSdImpressDoc("odp/tdf94122_autocolor.odp");

    save(TestFilter::PPTX);

    // Without the accompanying fix in place, these tests would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In ..., XPath '/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr' number of nodes is incorrect
    // i.e. automatic color wasn't resolved & exported

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr", "val",
                u"000000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport(u"ppt/slides/slide2.xml"_ustr);
    assertXPathInsensitive(
        pXmlDocContent2, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr",
        "val", u"ffffff");

    xmlDocUniquePtr pXmlDocContent3 = parseExport(u"ppt/slides/slide3.xml"_ustr);
    assertXPathInsensitive(
        pXmlDocContent3,
        "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr", "val",
        u"ffffff");
    assertXPath(pXmlDocContent3,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr",
                "val", u"000000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf124333)
{
    // Document contains one rectangle and one embedded OLE object.

    createSdImpressDoc("pptx/ole.pptx");

    // Without the fix in place, the number of shapes was 3.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("number of shapes is incorrect", sal_Int32(2),
                                 getPage(0)->getCount());

    saveAndReload(TestFilter::PPTX);

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("number of shapes is incorrect after export", sal_Int32(2),
                                 getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testAutofittedTextboxIndent)
{
    createSdImpressDoc("odp/autofitted-textbox-indent.odp");

    save(TestFilter::PPTX);

    // Check that the indent hasn't changed and wasn't scaled when exporting
    // (the behaviour changed).

    xmlDocUniquePtr pXmlDocContent1 = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p[1]/a:pPr", "marL",
                u"1080000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf151622_oleIcon)
{
    createSdImpressDoc("odp/ole_icon.odp");

    save(TestFilter::PPTX);

    xmlDocUniquePtr pXml = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: prop
    // - In ..., XPath '//p:oleObj' no attribute 'showAsIcon' exist
    // i.e. show as icon option wasn't exported.
    assertXPath(pXml, "//p:oleObj", "showAsIcon", u"1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf152436)
{
    createSdImpressDoc("pptx/ole-emf_min.pptx");
    saveAndReload(TestFilter::PPTX);

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testLinkedOLE)
{
    createSdImpressDoc("odp/linked_ole.odp");

    save(TestFilter::PPTX);

    xmlDocUniquePtr pXml = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In<>, XPath '//p:oleObj' number of nodes is incorrect
    // i.e. the linked ole object wasn't exported.
    assertXPath(pXml, "//p:oleObj", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf102261_testParaTabStopDefaultDistance)
{
    createSdImpressDoc("pptx/tdf102261_testParaTabStopDefaultDistance.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    {
        uno::Reference<beans::XPropertySet> xPropSet(getParagraphFromShape(0, xShape),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32{ 1270 },
            xPropSet->getPropertyValue(u"ParaTabStopDefaultDistance"_ustr).get<sal_Int32>());
    }
    {
        uno::Reference<beans::XPropertySet> xPropSet(getParagraphFromShape(1, xShape),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32{ 2540 },
            xPropSet->getPropertyValue(u"ParaTabStopDefaultDistance"_ustr).get<sal_Int32>());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testNotesAuthorDate)
{
    createSdImpressDoc("pptx/pres-with-notes.pptx");

    using ScriptingCfg = officecfg::Office::Common::Security::Scripting;
    ScopedConfigValue<ScriptingCfg::RemovePersonalInfoOnSaving> aCfg1(true);
    {
        // 1. Remove all personal info, but keep note info
        ScopedConfigValue<ScriptingCfg::KeepNoteAuthorDateInfoOnSaving> aCfg2(true);

        saveAndReload(TestFilter::PPTX);

        xmlDocUniquePtr pXml = parseExport(u"ppt/commentAuthors.xml"_ustr);
        assertXPath(pXml, "/p:cmAuthorLst/p:cmAuthor[@id=0]", "name", u"Hans Wurst");
        assertXPath(pXml, "/p:cmAuthorLst/p:cmAuthor[@id=1]", "name", u"Max Muster");

        pXml = parseExport(u"ppt/comments/comment1.xml"_ustr);
        assertXPath(pXml, "/p:cmLst/p:cm", "dt", u"2024-06-13T12:03:08.000000000");
    }

    {
        // 2. Remove all personal info
        ScopedConfigValue<ScriptingCfg::KeepNoteAuthorDateInfoOnSaving> aCfg2(false);
        saveAndReload(TestFilter::PPTX);

        xmlDocUniquePtr pXml = parseExport(u"ppt/commentAuthors.xml"_ustr);
        assertXPath(pXml, "/p:cmAuthorLst/p:cmAuthor[@id=0]", "name", u"Author1");
        assertXPath(pXml, "/p:cmAuthorLst/p:cmAuthor[@id=1]", "name", u"Author2");

        pXml = parseExport(u"ppt/comments/comment1.xml"_ustr);
        assertXPathNoAttribute(pXml, "/p:cmLst/p:cm", "dt");
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTableCellVerticalPropertyRoundtrip)
{
    createSdImpressDoc("pptx/tcPr-vert-roundtrip.pptx");
    saveAndReload(TestFilter::PPTX);

    xmlDocUniquePtr pXml = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXml, "(//a:tcPr)[1]", "vert", u"vert");
    assertXPath(pXml, "(//a:tcPr)[2]", "vert", u"vert270");
    assertXPath(pXml, "(//a:tcPr)[3]", "vert", u"wordArtVert");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf157740_slideMasters)
{
    createSdImpressDoc("pptx/tdf157740.pptx");
    saveAndReload(TestFilter::PPTX);

    // The original file has 1 slide master and 7 slide layouts in that master
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/presentation.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:presentation/p:sldMasterIdLst/p:sldMasterId", 1);

    pXmlDocContent = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:sldMaster/p:sldLayoutIdLst/p:sldLayoutId", 7);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf159931_slideLayouts)
{
    createSdImpressDoc("odp/repeatBitmapMode.odp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels1 = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    xmlDocUniquePtr pXmlDocRels2 = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);

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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf163483_export_math_fallback)
{
    createSdImpressDoc("odp/formula.fodp");

    // Before export-and-reload, there is a formula on page.
    {
        auto xProps = getShapeFromPage(0, 0);
        auto xInfo = xProps->getPropertyValue(u"Model"_ustr).queryThrow<css::lang::XServiceInfo>();
        CPPUNIT_ASSERT(xInfo->supportsService(u"com.sun.star.formula.FormulaProperties"_ustr));

        // tdf#164101: check that the size is imported correctly
        css::awt::Size formulaSize = xProps.queryThrow<css::drawing::XShape>()->getSize();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1553), formulaSize.Width);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(528), formulaSize.Height);

        CPPUNIT_ASSERT_THROW(getShapeFromPage(0, 1),
                             css::lang::IndexOutOfBoundsException); // Only one shape on page
    }

    saveAndReload(TestFilter::PPTX);

    // After save-and-reload, there must still be a single shape; now it's a fallback image.
    // When we start to import formulas from PPTX, that will be formula.
    {
        // Without the fix, this threw IndexOutOfBoundsException, because there was no fallback,
        // and no shape got imported.
        auto xInfo = getShapeFromPage(0, 0).queryThrow<css::lang::XServiceInfo>();
        CPPUNIT_ASSERT(xInfo->supportsService(u"com.sun.star.drawing.CustomShape"_ustr));

        css::awt::Size formulaSize = xInfo.queryThrow<css::drawing::XShape>()->getSize();
        // The fallback image size after the roundtrip may be a bit different - allow some tolerance
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1553), formulaSize.Width, 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(528), formulaSize.Height, 1);

        CPPUNIT_ASSERT_THROW(getShapeFromPage(0, 1),
                             css::lang::IndexOutOfBoundsException); // Only one shape on page
    }

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString cNvPr_id = getXPath(
        pXmlDoc, "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:nvSpPr/p:cNvPr",
        "id");

    // Check that the alternate content is exported
    assertXPath(
        pXmlDoc,
        "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Fallback/p:sp/p:spPr/a:blipFill/a:blip",
        "embed", u"rId1");

    // Its cNvPr_id must be the same
    assertXPath(pXmlDoc,
                "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Fallback/p:sp/p:nvSpPr/p:cNvPr",
                "id", cNvPr_id);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf169979_missing_ppic)
{
    createSdImpressDoc("odp/tdf169979.odp");

    save(TestFilter::PPTX);

    xmlDocUniquePtr pXml2 = parseExport(u"ppt/slides/slide2.xml"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj/p:pic/p:nvPicPr/p:cNvPr' number of nodes is incorrect
    assertXPath(pXml2,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/"
                "a:graphicData/p:oleObj/p:pic/p:nvPicPr/p:cNvPr",
                "name", u"");

    assertXPath(pXml2,
                "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/"
                "a:graphicData/p:oleObj/p:pic/p:nvPicPr/p:cNvPr",
                "descr", u"");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testEmptyChildTnLstElement)
{
    createSdImpressDoc("odp/emptyChildTnLstElement.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // tdf#170202: Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(0, countXPathNodes(pXmlDoc, "//p:sld/p:timing"));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf169941_internal_link_to_shapes)
{
    createSdImpressDoc("odp/tdf169941.odp");

    save(TestFilter::PPTX);

    xmlDocUniquePtr pRelsDoc2 = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: slide1.xml
    // - Actual  :
    assertXPath(pRelsDoc2, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                u"slide1.xml");

    xmlDocUniquePtr pRelsDoc3 = parseExport(u"ppt/slides/_rels/slide3.xml.rels"_ustr);
    assertXPath(pRelsDoc3, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                u"slide1.xml");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testEmptyAttrNameElement)
{
    createSdImpressDoc("odp/emptyChildTnLstElement.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 2
    // - validation error in OOXML export: Errors: 2
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testPlaceHolderFitHeightToText)
{
    createSdImpressDoc("pptx/tdf160487.pptx");
    saveAndReload(TestFilter::PPTX);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));
    bool bTextAutoGrowHeight = false;
    xShape->getPropertyValue(u"TextAutoGrowHeight"_ustr) >>= bTextAutoGrowHeight;
    CPPUNIT_ASSERT_MESSAGE("PlaceHolder Fit height to text should be true.", bTextAutoGrowHeight);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testDeduplicateMasters)
{
    createSdImpressDoc("pptx/onemaster-twolayouts.pptx");
    saveAndReload(TestFilter::PPTX);

    // Check that the document still has one master and two layouts
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/presentation.xml");
    assertXPath(pXmlDocContent, "/p:presentation/p:sldMasterIdLst/p:sldMasterId"_ostr, 1);
    pXmlDocContent = parseExport("ppt/slideMasters/slideMaster1.xml");
    assertXPath(pXmlDocContent, "/p:sldMaster/p:sldLayoutIdLst/p:sldLayoutId"_ostr, 2);

    // Check that both background colors have been preserved
    uno::Reference<drawing::XMasterPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDoc->getMasterPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getMasterPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    uno::Any aAny = xPropSet->getPropertyValue("Background");
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;
    Color nColor;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue("FillColor") >>= nColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x0E2841), nColor);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getMasterPages()->getByIndex(1),
                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet1(xPage1, uno::UNO_QUERY);
    aAny = xPropSet1->getPropertyValue("Background");
    CPPUNIT_ASSERT(aAny.hasValue());
    aAny >>= aXBackgroundPropSet;
    CPPUNIT_ASSERT(aXBackgroundPropSet->getPropertyValue("FillColor") >>= nColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nColor);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testInvalidAttributeValuesInSrcRect)
{
    createSdImpressDoc("odp/invalidAttributeValuesInSrcRect.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 24
    // - validation error in OOXML export: Errors: 24
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testInvalidBuAutoNumEnumValue)
{
    createSdImpressDoc("odp/invalidBuAutoNumEnumValue.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testInvalidCxAndCyAttribute)
{
    createSdImpressDoc("odp/invalidCxAndCyAttribute.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testBadPercentageValue)
{
    createSdImpressDoc("odp/badPercentageValue.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 4
    // - validation error in OOXML export: Errors: 4
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testInvalidAudio)
{
    createSdImpressDoc("odp/invalidAudio.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    // - validation error in OOXML export: Errors: 1
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testInvalidPrstDashEnumValue)
{
    createSdImpressDoc("odp/invalidPrstDashEnumValue.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 113
    // - validation error in OOXML export: Errors: 113
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testInvalidXmlBy)
{
    createSdImpressDoc("odp/invalidXmlBy.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 15
    // - validation error in OOXML export: Errors: 15
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testConvertWithMasterDeduplication)
{
    createSdImpressDoc("odp/dupmastermultlayouts.odp");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // For each slide check that it's layout exists
    for (int i = 1; i <= 4; ++i)
    {
        xmlDocUniquePtr pXmlDocRels
            = parseExport("ppt/slides/_rels/slide" + OUString::number(i) + ".xml.rels");

        assertXPath(
            pXmlDocRels,
            "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])"_ostr);
        // the relative target e.g. "../slideLayouts/slideLayout2.xml"
        OUString sRelativeLayoutPath = getXPathContent(
            pXmlDocRels,
            "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])/@Target"_ostr);

        // Check that the referenced slideLayout files exist
        // Without the accompanying fix in place, this test would have failed with:
        // equality assertion failed
        // - Expected: 1
        // - Actual  : 0
        // i.e. the referenced slideLayout file was missing on export.
        OUString sSlideLayoutName = sRelativeLayoutPath.getToken(2, '/');
        CPPUNIT_ASSERT_EQUAL(true,
                             bool(xNameAccess->hasByName("ppt/slideLayouts/" + sSlideLayoutName)));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf162571HorzAnchor)
{
    createSdImpressDoc("pptx/tdf165261.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xProp(xShape, uno::UNO_QUERY);

    drawing::TextHorizontalAdjust eHori;
    CPPUNIT_ASSERT(xProp->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eHori);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust::TextHorizontalAdjust_LEFT, eHori);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf165261HorzAnchor)
{
    createSdImpressDoc("pptx/tdf165261.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XShapes> xGroupShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xProp(xShape, uno::UNO_QUERY);
    drawing::TextHorizontalAdjust eHori;
    CPPUNIT_ASSERT(xProp->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eHori);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust::TextHorizontalAdjust_CENTER, eHori);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testCustomPromptTexts)
{
    createSdImpressDoc("pptx/tdf163239.pptx");
    saveAndReload(TestFilter::PPTX);

    const SdrPage* pPage1 = GetPage(1);
    {
        // subtitle placeholder text
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage1->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::Text,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        OUString aText = aEdit.GetText(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text!", aText, u"Click to add Text"_ustr);
        /* TODO: handle subtitle shape: see tdf#112557 workaround
            - Expected: Click to edit customized Master Subtitle style
            - Actual : Click to add Text
            - Wrong placeholder text!
        */

        auto xShapeProps(getShapeFromPage(0, 0));
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CustomPromptText"_ustr) >>= aText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text was set!", aText, u""_ustr);
        /* TODO: handle subtitle shape: see tdf#112557 workaround
            - Expected: Click to edit customized Master Subtitle style
            - Actual :
            - Wrong placeholder text was set!
        */
    }

    {
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage1->GetObj(1));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::TitleText,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        OUString aText = aEdit.GetText(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text!", aText, u"Custom Title 1"_ustr);

        auto xShapeProps(getShapeFromPage(1, 0));
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CustomPromptText"_ustr) >>= aText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text was set!", aText,
                                     u"Custom Title 1"_ustr);
    }

    const SdrPage* pPage2 = GetPage(3);
    {
        // body placeholder text
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage2->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::OutlineText,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        OUString aText = aEdit.GetText(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text!", aText, u"Text placeholder"_ustr);

        auto xShapeProps(getShapeFromPage(0, 1));
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CustomPromptText"_ustr) >>= aText);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong placeholder text was set!", aText,
                                     u"Text placeholder"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testPresObjTextStyles)
{
    createSdImpressDoc("pptx/tdf163239_v2.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    // 1st slide
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"Title Slide"_ustr), uno::UNO_QUERY);
    {
        // Title style
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"title"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(60.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    }
    // 2nd slide
    xStyleFamily.set(xStyleFamilies->getByName(u"Title and Content"_ustr), uno::UNO_QUERY);
    {
        // Title style
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"title"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(44.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

        // outline1 style
        xStyle.set(xStyleFamily->getByName(u"outline1"_ustr), uno::UNO_QUERY);
        xPropSet.set(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(28.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

        // outline2 style
        xStyle.set(xStyleFamily->getByName(u"outline2"_ustr), uno::UNO_QUERY);
        xPropSet.set(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(24.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());

        // outline5 style
        xStyle.set(xStyleFamily->getByName(u"outline5"_ustr), uno::UNO_QUERY);
        xPropSet.set(xStyle, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(18.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
        CPPUNIT_ASSERT_EQUAL(u"Marianne"_ustr,
                             xPropSet->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTextStylesXML)
{
    createSdImpressDoc("pptx/tdf163239_v2.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slideLayouts/slideLayout1.xml"_ustr);

    assertXPath(pXmlDocRels, "/p:sldLayout/p:cSld/p:spTree/p:sp[1]/p:txBody/a:lstStyle/a:lvl1pPr",
                "algn", u"ctr");
    assertXPath(pXmlDocRels,
                "/p:sldLayout/p:cSld/p:spTree/p:sp[1]/p:txBody/a:lstStyle/a:lvl1pPr/a:defRPr", "sz",
                u"6000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTextAlignLeft)
{
    createSdImpressDoc("pptx/presLeftAlign.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/slide1.xml"_ustr);

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr", "algn", u"l");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testSubtitleNoBullets)
{
    createSdImpressDoc("odp/tdf170166.odp");
    saveAndReload(TestFilter::PPTX);

    const SdrPage* pPage1 = GetPage(1);
    {
        // subtitle placeholder object
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage1->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong object type!", SdrObjKind::Text,
                                     pTxtObj->GetObjIdentifier());
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET);
        // Without a fix, it will fail with numbering type: SVX_NUM_CHAR_SPECIAL
        CPPUNIT_ASSERT(pNumFmt);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's numbering type is wrong!", SVX_NUM_NUMBER_NONE,
                                     pNumFmt->GetNumRule().GetLevel(0).GetNumberingType());
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testOLEObjectAnimationTarget)
{
    createSdImpressDoc("pptx/tdf169088.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    const OUString sOleId = getXPath(
        pXmlDoc, "/p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr", "id");
    const OUString sPicId = getXPath(pXmlDoc,
                                     "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/"
                                     "a:graphicData/p:oleObj/p:pic/p:nvPicPr/p:cNvPr",
                                     "id");

    // Check OLE id is different from pic id
    CPPUNIT_ASSERT_MESSAGE("OLE id is same as pic id", sOleId != sPicId);

    // Check animation target spid matches OLE object id
    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
                "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:set/"
                "p:cBhvr/p:tgtEl/p:spTgt",
                "spid", sOleId);

    assertXPath(pXmlDoc,
                "/p:sld/p:timing/p:tnLst/p:par/p:cTn/p:childTnLst/p:seq/p:cTn/p:childTnLst/p:par/"
                "p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:par/p:cTn/p:childTnLst/p:animEffect/"
                "p:cBhvr/p:tgtEl/p:spTgt",
                "spid", sOleId);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testtdf169496_hidden_graphic)
{
    createSdImpressDoc("pptx/tdf169496_hidden_graphic.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Graphic 5 is hidden and Graphic 4 is visible, but their order might change in the XML
    // Without the fix the hidden attribute wasn't exported
    OUString sName1
        = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:nvPicPr/p:cNvPr", "name");
    OUString sName2
        = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:nvPicPr/p:cNvPr", "name");
    if (sName1 == "Graphic 5" && sName2 == "Graphic 4")
    {
        OUString aHidden
            = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:nvPicPr/p:cNvPr", "hidden");
        bool bHidden = aHidden == u"true"_ustr || aHidden == u"1";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Graphic 3 is supposed to be hidden", true, bHidden);

        assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:nvPicPr/p:cNvPr",
                               "hidden");
    }
    else if (sName1 == "Graphic 4" && sName2 == "Graphic 5")
    {
        assertXPathNoAttribute(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[1]/p:nvPicPr/p:cNvPr",
                               "hidden");

        OUString aHidden
            = getXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:pic[2]/p:nvPicPr/p:cNvPr", "hidden");
        bool bHidden = aHidden == u"true"_ustr || aHidden == u"1";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Graphic 4 is supposed to be hidden", true, bHidden);
    }
    else
        CPPUNIT_FAIL("Names of graphics is incorrect");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testtdf169825_vertical_layouts)
{
    createSdImpressDoc("pptx/tdf169825_vertical_layouts.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    // find layout XML for the slide from the relationship file
    OUString sLayoutRelRelative
        = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    OUString sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    xmlDocUniquePtr pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the SlidePersist::getLayoutFromValueToken() part of the patch,
    // this and the next layout types would both be exported as blank
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTitleAndTx");

    pXmlDocRels = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    sLayoutRelRelative = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTx");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testtdf169825_vertical_layouts_from_scratch)
{
    createSdImpressDoc();

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xPages = xDoc->getDrawPages();
    uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPageSet(xPage, uno::UNO_QUERY_THROW);
    xPageSet->setPropertyValue(
        "Layout",
        uno::Any(static_cast<sal_Int32>(AutoLayout::AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT)));

    uno::Reference<drawing::XDrawPage> xPage2(xPages->insertNewByIndex(1), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xPageSet2(xPage2, uno::UNO_QUERY_THROW);
    xPageSet2->setPropertyValue(
        "Layout", uno::Any(static_cast<sal_Int32>(AutoLayout::AUTOLAYOUT_VTITLE_VCONTENT)));
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    // find layout XML for the slide from the relationship file
    OUString sLayoutRelRelative
        = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    OUString sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    xmlDocUniquePtr pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the fix in place this would be exported as "objTx"
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTitleAndTxOverChart");

    pXmlDocRels = parseExport(u"ppt/slides/_rels/slide2.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    sLayoutRelRelative = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the fix in place this would be exported as "picTx"
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"vertTitleAndTx");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testtdf169825_layout_type)
{
    createSdImpressDoc("odp/tdf169825_layout_type.odp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDocRels = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlDocRels);
    // find layout XML for the slide from the relationship file
    OUString sLayoutRelRelative
        = getXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship", "Target");
    OUString sLayoutRelAbs = sLayoutRelRelative.replaceFirst("..", "ppt");
    xmlDocUniquePtr pXmlDocLayout = parseExport(sLayoutRelAbs);
    CPPUNIT_ASSERT(pXmlDocLayout);

    // without the fix in place this would be exported as "vertTitleAndTxOverChart"
    // while in the original ODP this was - still possibly bogus - notes layout
    assertXPath(pXmlDocLayout, "/p:sldLayout", "type", u"blank");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testtdf170102_layout_type)
{
    createSdImpressDoc("odp/tdf170102_layout_type.odp");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XMasterPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    sal_Int32 nMPCount = xDoc->getMasterPages()->getCount();
    // test roughly the same thing in document and in XML: no drawing object in master page/layout
    for (sal_Int32 i = 0; i < nMPCount; i++)
    {
        uno::Reference<drawing::XDrawPage> xPage(xDoc->getMasterPages()->getByIndex(i),
                                                 uno::UNO_QUERY_THROW);
        sal_Int32 nObjCount = xPage->getCount();
        for (sal_Int32 j = 0; j < nObjCount; j++)
        {
            uno::Reference<lang::XServiceInfo> xShapeInfo(xPage->getByIndex(j),
                                                          uno::UNO_QUERY_THROW);
            // without the fix in place there would be a graphic shape placeholder
            CPPUNIT_ASSERT_MESSAGE(
                "Unexpected graphic object shape in exported master page",
                !xShapeInfo->supportsService(u"com.sun.star.drawing.GraphicObjectShape"_ustr));
        }

        xmlDocUniquePtr pXmlDocLayout = parseExport(u"ppt/slideLayouts/slideLayout"_ustr
                                                    + OUString::number(i + 1) + u".xml"_ustr);
        CPPUNIT_ASSERT(pXmlDocLayout);
        // without the fix in place there would be a pic layout element for the placeholder
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Unexpected \"pic\" element in exported PPTX slide layout", 0,
            countXPathNodes(pXmlDocLayout, "/p:sldLayout/p:cSld/p:spTree/p:pic"));
    }
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testFooterIdxConsistency)
{
    createSdImpressDoc("pptx/multiplelayoutfooter.pptx");
    save(TestFilter::PPTX);

    // slide1 points to slideLayout2
    xmlDocUniquePtr pRelsDoc = parseExport(u"ppt/slides/_rels/slide1.xml.rels"_ustr);
    assertXPath(pRelsDoc,
                "/rels:Relationships/rels:Relationship[@Type='http://"
                "schemas.openxmlformats.org/officeDocument/2006/"
                "relationships/slideLayout']",
                "Target", u"../slideLayouts/slideLayout2.xml");

    // And they agree on the idx of the footer:
    xmlDocUniquePtr pSlide = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString aFtrIdx = getXPath(pSlide, "//p:sp/p:nvSpPr/p:nvPr/p:ph", "idx");
    xmlDocUniquePtr pLayout = parseExport(u"ppt/slideLayouts/slideLayout2.xml"_ustr);
    assertXPath(pLayout, "//p:sp/p:nvSpPr/p:nvPr/p:ph", "idx", aFtrIdx);
    // Without the fix it fails with:
    // - Expected: 1
    // - Actual  : 2
    // - In <>, attribute 'idx' of '//p:sp/p:nvSpPr/p:nvPr/p:ph' incorrect value.
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testOmitCanvasSlideExport)
{
    createSdImpressDoc("odp/canvas-slide.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);

    // the document has 2 pages - one canvas page, and one normal page
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDoc->GetSdPageCount(PageKind::Standard));
    CPPUNIT_ASSERT(pDoc->HasCanvasPage());

    save(TestFilter::PPTX);

    // Verify that the canvas slide was omitted from the export
    // It should have one master slide, and one slide
    xmlDocUniquePtr pXmlDocContent = parseExport(u"ppt/presentation.xml"_ustr);
    assertXPath(pXmlDocContent, "/p:presentation/p:sldMasterIdLst/p:sldMasterId", 1);
    assertXPath(pXmlDocContent, "/p:presentation/p:sldIdLst/p:sldId", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testParaAlignStartEnd)
{
    createSdImpressDoc("odp/para-align-start-end.fodp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // PresentationML doesn't support start and end paragraph alignment.
    // Paragraphs should be exported as left/right alignment, depending on para direction.
    assertXPath(pDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[1]/a:pPr[@algn='l']", 1);
    assertXPath(pDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[2]/a:pPr[@algn='r']", 1);
    assertXPath(pDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[3]/a:pPr[@algn='r']", 1);
    assertXPath(pDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p[4]/a:pPr[@algn='l']", 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf169559)
{
    createSdImpressDoc("pptx/tdf169524.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "/p:sldMaster/p:cSld/p:spTree/p:sp[2]/p:txBody/a:lstStyle/a:lvl1pPr/a:spcAft/a:spcPts",
        "val", u"1701");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
