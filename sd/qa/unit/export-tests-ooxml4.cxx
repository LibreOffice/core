/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <sdpage.hxx>

class SdOOXMLExportTest4 : public SdModelTestBase
{
public:
    SdOOXMLExportTest4()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testSmartArtPreserve)
{
    createSdImpressDoc("pptx/smartart-preserve.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:cNvPr"_ostr);
    assertXPath(pXmlDoc,
                "//p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/dgm:relIds"_ostr);
    assertXPath(
        pXmlDoc,
        "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/p:extLst/p:ext"_ostr,
        "uri"_ostr, "{D42A27DB-BD31-4B8C-83A1-F6EECF244321}");
    assertXPath(pXmlDoc, "//p:sld/p:cSld/p:spTree/p:graphicFrame/p:nvGraphicFramePr/p:nvPr/"
                         "p:extLst/p:ext/p14:modId"_ostr);

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/_rels/slide1.xml.rels");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/layout1.xml'])[1]"_ostr,
        "Type"_ostr,
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramLayout");
    assertXPath(pXmlDocRels,
                "(/rels:Relationships/rels:Relationship[@Target='../diagrams/data1.xml'])[1]"_ostr,
                "Type"_ostr,
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/colors1.xml'])[1]"_ostr,
        "Type"_ostr,
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramColors");
    assertXPath(
        pXmlDocRels,
        "(/rels:Relationships/rels:Relationship[@Target='../diagrams/quickStyle1.xml'])[1]"_ostr,
        "Type"_ostr,
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramQuickStyle");

    xmlDocUniquePtr pXmlContentType = parseExport("[Content_Types].xml");
    assertXPath(
        pXmlContentType,
        "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/layout1.xml']"_ostr,
        "ContentType"_ostr,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml");
    assertXPath(pXmlContentType,
                "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/data1.xml']"_ostr,
                "ContentType"_ostr,
                "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml");
    assertXPath(
        pXmlContentType,
        "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/colors1.xml']"_ostr,
        "ContentType"_ostr,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml");
    assertXPath(
        pXmlContentType,
        "/ContentType:Types/ContentType:Override[@PartName='/ppt/diagrams/quickStyle1.xml']"_ostr,
        "ContentType"_ostr,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125346)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125346_2)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125360)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125360_1)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125360_2)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125551)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf136830)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf126234)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf126741)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf127372)
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
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), Color(aColorStops[1].getStopColor()));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf127379)
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
    CPPUNIT_ASSERT_EQUAL(Color(0xff0000), Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x2a6099), Color(aColorStops[1].getStopColor()));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf98603)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf128213)
{
    createSdImpressDoc("pptx/tdf128213.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPathNoAttribute(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp/p:spPr/a:xfrm"_ostr,
                           "rot"_ostr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf79082)
{
    createSdImpressDoc("pptx/tdf79082.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]"_ostr,
                "pos"_ostr, "360000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[1]"_ostr,
                "algn"_ostr, "l");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]"_ostr,
                "pos"_ostr, "756000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[2]"_ostr,
                "algn"_ostr, "l");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]"_ostr,
                "pos"_ostr, "1440000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[3]"_ostr,
                "algn"_ostr, "ctr");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]"_ostr,
                "pos"_ostr, "1800000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[4]"_ostr,
                "algn"_ostr, "r");

    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]"_ostr,
                "pos"_ostr, "3240000");
    assertXPath(pXmlDocContent,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:pPr/a:tabLst/a:tab[5]"_ostr,
                "algn"_ostr, "dec");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf129372)
{
    //Without the fix in place, it would crash at import time

    createSdImpressDoc("pptx/tdf129372.pptx");
    saveAndReload("Impress Office Open XML");
    const SdrPage* pPage = GetPage(1);

    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeGlowEffect)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testUnderline)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf119087)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf131554)
{
    createSdImpressDoc("pptx/tdf131554.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<drawing::XShape> xShape(getShapeFromPage(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5622), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(13251), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf132282)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf132201EffectOrder)
{
    createSdImpressDoc("pptx/effectOrder.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/slides/slide1.xml");
    assertXPathChildren(pXmlDocContent, "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst"_ostr,
                        2);
    // The relative order of effects is important: glow must be before shadow
    CPPUNIT_ASSERT_EQUAL(
        0, getXPathPosition(pXmlDocContent,
                            "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst"_ostr, "glow"));
    CPPUNIT_ASSERT_EQUAL(
        1, getXPathPosition(pXmlDocContent,
                            "/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:effectLst"_ostr, "outerShdw"));
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeSoftEdgeEffect)
{
    createSdImpressDoc("pptx/shape-soft-edges.pptx");
    saveAndReload("Impress Office Open XML");
    auto xShapeProps(getShapeFromPage(0, 0));
    sal_Int32 nRadius = -1;
    xShapeProps->getPropertyValue("SoftEdgeRadius") >>= nRadius;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRadius); // 18 pt
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testShapeShadowBlurEffect)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf119223)
{
    createSdImpressDoc("odp/tdf119223.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeCustomShape']"_ostr);

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomePicture']"_ostr);

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeFormula']"_ostr);

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeLine']"_ostr);

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTextbox']"_ostr);

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeTable']"_ostr);

    assertXPath(pXmlDocRels, "//p:cNvPr[@name='SomeGroup']"_ostr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf128213ShapeRot)
{
    createSdImpressDoc("pptx/tdf128213-shaperot.pptx");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocRels = parseExport("ppt/slides/slide1.xml");

    assertXPath(pXmlDocRels, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d"_ostr);
    assertXPath(pXmlDocRels,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot"_ostr,
                "rev"_ostr, "5400000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125560_textDeflate)
{
    createSdImpressDoc("pptx/ShapePlusImage.pptx");

    // This problem did not affect the pptx export, only the ODP so assert that
    save("impress8");

    xmlDocUniquePtr pXmlDocRels = parseExport("content.xml");
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry"_ostr,
                "type"_ostr, "mso-spt161");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf125560_textInflateTop)
{
    createSdImpressDoc("pptx/ShapeTextInflateTop.pptx");

    // This problem did not affect the pptx export, only the ODP so assert that
    save("impress8");

    xmlDocUniquePtr pXmlDocRels = parseExport("content.xml");
    assertXPath(pXmlDocRels,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry"_ostr,
                "type"_ostr, "mso-spt164");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf142235_TestPlaceholderTextAlignment)
{
    createSdImpressDoc("odp/placeholder-box-textalignment.odp");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml1 = parseExport("ppt/slides/slide2.xml");
    xmlDocUniquePtr pXml2 = parseExport("ppt/slides/slide3.xml");

    // Without the fix in place many of these asserts failed, because alignment was bad.

    assertXPath(pXml1, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr"_ostr, "anchor"_ostr, "t");
    assertXPath(pXml2, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:bodyPr"_ostr, "anchor"_ostr, "t");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf96061_textHighlight)
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

    // Check export of embedded worksheet in slide.
    saveAndReload("Impress Office Open XML");

    pPage = GetPage(1);
    pOleObj = static_cast<SdrOle2Obj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no object after the export", pOleObj != nullptr);

    pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE("no graphic after the export", pGraphic != nullptr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf143315)
{
    createSdImpressDoc("ppt/tdf143315-WordartWithoutBullet.ppt");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    // Without the fix in place, this would have failed with
    // - Expected:
    // - Actual  : 216000
    // - In <file:///tmp/lu161922zcvd.tmp>, XPath '/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr' unexpected 'marL' attribute

    assertXPathNoAttribute(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr"_ostr,
                           "marL"_ostr);
    assertXPathNoAttribute(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr"_ostr,
                           "indent"_ostr);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buClr"_ostr, 0);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buSzPct"_ostr, 0);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buFont"_ostr, 0);
    assertXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:pPr/a:buChar"_ostr, 0);
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
    const auto nFontSizeBefore = xRun->getPropertyValue("CharHeight").get<float>() * 100;

    save("Impress Office Open XML");

    // Parse the export
    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");
    const auto nFontSizeAfter
        = getXPath(pXml, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:br[1]/a:rPr"_ostr, "sz"_ostr)
              .toFloat();

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
    xShapeProps->getPropertyValue("IsEmptyPresentationObject") >>= isEmptyPresentationObject;
    CPPUNIT_ASSERT(isEmptyPresentationObject);

    // If we supported custom prompt text, here we would also test "String" property,
    // which would be equal to "Insert Image".
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testEnhancedPathViewBox)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf74670)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf109169_OctagonBevel)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf109169_DiamondBevel)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf144092_emptyShapeTextProps)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf149551_tbrl90)
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
    assertXPath(pXmlDoc, "//a:bodyPr"_ostr, "vert"_ostr, "vert");

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
        "//style:style[@style:name='gr1']/style:graphic-properties[@loext:writing-mode='tb-rl90']"_ostr);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf149551_btlr)
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
    assertXPath(pXmlDoc, "//a:bodyPr"_ostr, "vert"_ostr, "vert270");

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
        "//style:style[@style:name='gr1']/style:graphic-properties[@loext:writing-mode='bt-lr']"_ostr);
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

    save("Impress Office Open XML");

    // Without the accompanying fix in place, these tests would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In ..., XPath '/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr' number of nodes is incorrect
    // i.e. automatic color wasn't resolved & exported

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, "000000");

    xmlDocUniquePtr pXmlDocContent2 = parseExport("ppt/slides/slide2.xml");
    assertXPath(pXmlDocContent2,
                "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, "ffffff");

    xmlDocUniquePtr pXmlDocContent3 = parseExport("ppt/slides/slide3.xml");
    assertXPath(pXmlDocContent3,
                "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, "ffffff");
    assertXPath(pXmlDocContent3,
                "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, "000000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf124333)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testAutofittedTextboxIndent)
{
    createSdImpressDoc("odp/autofitted-textbox-indent.odp");

    save("Impress Office Open XML");

    // Check that the indent hasn't changed and wasn't scaled when exporting
    // (the behaviour changed).

    xmlDocUniquePtr pXmlDocContent1 = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDocContent1, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p[1]/a:pPr"_ostr,
                "marL"_ostr, "1080000");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf151622_oleIcon)
{
    createSdImpressDoc("odp/ole_icon.odp");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: prop
    // - In ..., XPath '//p:oleObj' no attribute 'showAsIcon' exist
    // i.e. show as icon option wasn't exported.
    assertXPath(pXml, "//p:oleObj"_ostr, "showAsIcon"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf152436)
{
    createSdImpressDoc("pptx/ole-emf_min.pptx");
    saveAndReload("Impress Office Open XML");

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testLinkedOLE)
{
    createSdImpressDoc("odp/linked_ole.odp");

    save("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In<>, XPath '//p:oleObj' number of nodes is incorrect
    // i.e. the linked ole object wasn't exported.
    assertXPath(pXml, "//p:oleObj"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf102261_testParaTabStopDefaultDistance)
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

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTableCellVerticalPropertyRoundtrip)
{
    createSdImpressDoc("pptx/tcPr-vert-roundtrip.pptx");
    saveAndReload("Impress Office Open XML");

    xmlDocUniquePtr pXml = parseExport("ppt/slides/slide1.xml");

    assertXPath(pXml, "(//a:tcPr)[1]"_ostr, "vert"_ostr, "vert");
    assertXPath(pXml, "(//a:tcPr)[2]"_ostr, "vert"_ostr, "vert270");
    assertXPath(pXml, "(//a:tcPr)[3]"_ostr, "vert"_ostr, "wordArtVert");
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf157740_slideMasters)
{
    createSdImpressDoc("pptx/tdf157740.pptx");
    saveAndReload("Impress Office Open XML");

    // Test how many slidemaster we have
    xmlDocUniquePtr pXmlDocContent = parseExport("ppt/presentation.xml");
    assertXPath(pXmlDocContent, "/p:presentation/p:sldMasterIdLst/p:sldMasterId"_ostr, 7);

    pXmlDocContent = parseExport("ppt/slideMasters/slideMaster1.xml");
    assertXPath(pXmlDocContent, "/p:sldMaster/p:sldLayoutIdLst/p:sldLayoutId"_ostr, 1);

    pXmlDocContent = parseExport("ppt/slideMasters/slideMaster7.xml");
    assertXPath(pXmlDocContent, "/p:sldMaster/p:sldLayoutIdLst/p:sldLayoutId"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest4, testTdf159931_slideLayouts)
{
    createSdImpressDoc("odp/repeatBitmapMode.odp");
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDocRels1 = parseExport("ppt/slides/_rels/slide1.xml.rels");
    xmlDocUniquePtr pXmlDocRels2 = parseExport("ppt/slides/_rels/slide2.xml.rels");

    assertXPath(
        pXmlDocRels1,
        "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])"_ostr);

    // the relative target e.g. "../slideLayouts/slideLayout2.xml"
    OUString sRelativeLayoutPath1 = getXPathContent(
        pXmlDocRels1,
        "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])/@Target"_ostr);

    assertXPath(
        pXmlDocRels2,
        "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])"_ostr);

    // the relative target e.g. "../slideLayouts/slideLayout1.xml"
    OUString sRelativeLayoutPath2 = getXPathContent(
        pXmlDocRels2,
        "(/rels:Relationships/rels:Relationship[@Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout'])/@Target"_ostr);

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
