/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "sdmodeltestbase.hxx"
#include <sdpage.hxx>
#include <sdtiledrenderingtest.hxx>

#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <osl/process.h>
#include <unotools/saveopt.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomedia.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

#include <svx/svdotable.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace css;
using namespace css::animations;

class SdExportTest3 : public SdModelTestBase
{
public:
    SdExportTest3()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdExportTest3, testEmbeddedText)
{
    createSdDrawDoc("objectwithtext.fodg");
    saveAndReload(TestFilter::ODG);

    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xText.is());

    uno::Reference<container::XEnumerationAccess> xEA(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEA->hasElements());
    uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration());
    uno::Reference<text::XTextContent> xTC;
    xEnum->nextElement() >>= xTC;
    CPPUNIT_ASSERT(xTC.is());

    uno::Reference<container::XEnumerationAccess> xParaEA(xTC, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEA->createEnumeration());
    uno::Reference<beans::XPropertySet> xPortion(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortion.is());
    uno::Reference<text::XTextRange> xRange(xPortion, uno::UNO_QUERY);
    OUString type;
    xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= type;
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, type);
    CPPUNIT_ASSERT_EQUAL(u"foobar"_ustr, xRange->getString()); //tdf#112547
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTransparenText)
{
    createSdDrawDoc("transparent-text.fodg");
    saveAndReload(TestFilter::ODG);

    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nCharTransparence = 0;
    xShape->getPropertyValue(u"CharTransparence"_ustr) >>= nCharTransparence;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 75
    // - Actual  : 0
    // i.e. the 75% transparent text was turned into a "not transparent at all" text.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(75), nCharTransparence);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testDefaultSubscripts)
{
    createSdDrawDoc("tdf80194_defaultSubscripts.fodg");
    saveAndReload(TestFilter::ODG);

    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(1), uno::UNO_QUERY);
    // Default subscripts were too large, enlarging the gap between the next line.
    // The exact size isn't important. Was 18975, now 16604.
    CPPUNIT_ASSERT(17000 > xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf98477)
{
    createSdImpressDoc("pptx/tdf98477grow.pptx");
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//anim:animateTransform", "by", u"0.5,0.5");
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testAuthorField)
{
    createSdImpressDoc("odp/author_fixed.odp");

    saveAndReload(TestFilter::ODP);

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());

    uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);
    bool bFixed = false;
    xPropSet->getPropertyValue(u"IsFixed"_ustr) >>= bFixed;
    CPPUNIT_ASSERT_MESSAGE("Author field is not fixed", bFixed);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf50499)
{
    createSdImpressDoc("pptx/tdf50499.pptx");

    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//anim:animate[1]", "from", u"(-width/2)");
    assertXPath(pXmlDoc, "//anim:animate[1]", "to", u"(x)");
    assertXPath(pXmlDoc, "//anim:animate[3]", "by", u"(height/3+width*0.1)");
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf100926)
{
    createSdImpressDoc("pptx/tdf100926_ODP.pptx");

    saveAndReload(TestFilter::ODP);

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    sal_Int32 nRotation = 0;
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"RotateAngle"_ustr) >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000), nRotation);

    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"RotateAngle"_ustr) >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9000), nRotation);

    xCell.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"RotateAngle"_ustr) >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRotation);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testPageWithTransparentBackground)
{
    createSdImpressDoc("odp/page_transparent_background.odp");

    saveAndReload(TestFilter::ODP);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly one page", static_cast<sal_Int32>(1),
                                 xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    cpo::uno::Any aAny = xPropSet->getPropertyValue(u"Background"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());

    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;
    sal_Int32 nTransparence;
    aAny = aXBackgroundPropSet->getPropertyValue(u"FillTransparence"_ustr);
    aAny >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide background transparency is wrong", sal_Int32(42),
                                 nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTextRotation)
{
    // Save behavior depends on whether ODF strict or extended is used.
    comphelper::ScopeGuard g([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // The contained shape has a text rotation vert="vert" which corresponds to
    // loext:writing-mode="tb-rl90" in the graphic-properties of the style of the shape in ODF 1.3
    // extended.
    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "tb-rl90" is included in ODF strict.
    {
        createSdImpressDoc("pptx/shape-text-rotate.pptx");
        saveAndReload(TestFilter::ODP);

        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));
        CPPUNIT_ASSERT(xPropSet.is());

        auto aWritingMode = xPropSet->getPropertyValue(u"WritingMode"_ustr).get<sal_Int16>();
        CPPUNIT_ASSERT_EQUAL(sal_Int16(text::WritingMode2::TB_RL90), aWritingMode);
    }
    // In ODF 1.3 strict the workaround to use the TextRotateAngle is used instead.
    {
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);

        createSdImpressDoc("pptx/shape-text-rotate.pptx");
        saveAndReload(TestFilter::ODP);

        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));

        CPPUNIT_ASSERT(xPropSet.is());
        auto aGeomPropSeq = xPropSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                                .get<uno::Sequence<beans::PropertyValue>>();
        comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);

        auto it = aCustomShapeGeometry.find(u"TextRotateAngle"_ustr);
        CPPUNIT_ASSERT(it != aCustomShapeGeometry.end());

        CPPUNIT_ASSERT_EQUAL(double(-90),
                             aCustomShapeGeometry[u"TextRotateAngle"_ustr].get<double>());
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf115394PPT)
{
    createSdImpressDoc("ppt/tdf115394.ppt");

    // Export the document and import again for a check
    saveAndReload(TestFilter::PPT);

    double fTransitionDuration;

    // Fast
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdPage* pPage1 = pDoc->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Medium
    SdPage* pPage2 = pDoc->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Slow
    SdPage* pPage3 = pDoc->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testBulletsAsImageImpress8)
{
    createSdImpressDoc("odp/BulletsAsImage.odp");
    saveAndReload(TestFilter::ODP);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(
        xPropSet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProperties;
    xLevels->getByIndex(0) >>= aProperties; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    awt::Size aSize;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProperty : aProperties)
    {
        if (rProperty.Name == "NumberingType")
        {
            nNumberingType = rProperty.Value.get<sal_Int16>();
        }
        else if (rProperty.Name == "GraphicBitmap")
        {
            xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rProperty.Name == "GraphicSize")
        {
            aSize = rProperty.Value.get<awt::Size>();
        }
    }

    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);

    // Graphic Bitmap
    CPPUNIT_ASSERT_MESSAGE("No bitmap for the bullets", xBitmap.is());
    Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));

    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Height());

    // Graphic Size
    CPPUNIT_ASSERT_EQUAL(sal_Int32(500), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(500), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testBulletsAsImageImpressOfficeOpenXml)
{
    createSdImpressDoc("odp/BulletsAsImage.odp");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(
        xPropSet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProperties;
    xLevels->getByIndex(0) >>= aProperties; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    awt::Size aSize;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProperty : aProperties)
    {
        if (rProperty.Name == "NumberingType")
        {
            nNumberingType = rProperty.Value.get<sal_Int16>();
        }
        else if (rProperty.Name == "GraphicBitmap")
        {
            xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rProperty.Name == "GraphicSize")
        {
            aSize = rProperty.Value.get<awt::Size>();
        }
    }

    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);

    // Graphic Bitmap
    CPPUNIT_ASSERT_MESSAGE("No bitmap for the bullets", xBitmap.is());
    Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));

    // FIXME: what happened here
    CPPUNIT_ASSERT_EQUAL(tools::Long(64), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(64), aGraphic.GetSizePixel().Height());

    // Graphic Size
    // FIXME: totally wrong
    CPPUNIT_ASSERT_EQUAL(sal_Int32(790), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(790), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testBulletsAsImageMsPowerpoint97)
{
    createSdImpressDoc("odp/BulletsAsImage.odp");
    saveAndReload(TestFilter::PPT);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(
        xPropSet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProperties;
    xLevels->getByIndex(0) >>= aProperties; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    awt::Size aSize;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProperty : aProperties)
    {
        if (rProperty.Name == "NumberingType")
        {
            nNumberingType = rProperty.Value.get<sal_Int16>();
        }
        else if (rProperty.Name == "GraphicBitmap")
        {
            xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rProperty.Name == "GraphicSize")
        {
            aSize = rProperty.Value.get<awt::Size>();
        }
    }

    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);

    // Graphic Bitmap
    CPPUNIT_ASSERT_MESSAGE("No bitmap for the bullets", xBitmap.is());
    Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));

    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Height());

    // Graphic Size
    // seems like a conversion error
    CPPUNIT_ASSERT_EQUAL(sal_Int32(504), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(504), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf113822)
{
    createSdImpressDoc("pptx/tdf113822underline.pptx");

    // Was unable to export iterate container (tdf#99213).
    saveAndReload(TestFilter::PPTX);
    // Was unable to import iterate container (tdf#113822).
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // IterateContainer was created as ParallelTimeContainer before, so
    // the iterate type is not set too.
    assertXPath(pXmlDoc, "//anim:iterate", "iterate-type", u"by-letter");
    // The target of the child animation nodes need to be in the iterate container.
    assertXPath(pXmlDoc, "//anim:iterate", "targetElement", u"id1");
    assertXPath(pXmlDoc, "//anim:iterate/anim:set", "attributeName", u"text-underline");
    assertXPath(pXmlDoc, "//anim:iterate/anim:set", "to", u"solid");
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf113818)
{
    createSdImpressDoc("pptx/tdf113818-swivel.pptx");
    saveAndReload(TestFilter::PPT);
    saveAndReload(TestFilter::PPTX);
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//anim:animate[1]", "formula", u"width*sin(2.5*pi*$)");
    assertXPath(pXmlDoc, "//anim:animate[1]", "values", u"0;1");
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf119629)
{
    createSdImpressDoc("ppt/tdf119629.ppt");
    saveAndReload(TestFilter::PPT);
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // MSO's effect node type Click parallel node, with group node, after group node
    // were missing.
    assertXPath(pXmlDoc,
                "//draw:page"
                "/anim:par[@presentation:node-type='timing-root']"
                "/anim:seq[@presentation:node-type='main-sequence']"
                "/anim:par[@presentation:node-type='on-click']"
                "/anim:par[@presentation:node-type='with-previous']"
                "/anim:par[@presentation:node-type='on-click']"
                "/anim:animate[@anim:formula='width*sin(2.5*pi*$)']",
                1);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf141269)
{
    createSdImpressDoc("odp/tdf141269.odp");
    saveAndReload(TestFilter::PPT);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    Bitmap aBitmap(aGraphic.GetBitmap());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1920), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1080), aBitmap.GetSizePixel().Height());

    // Without the fix in place, this test would have failed with
    // - Expected: 240
    // - Actual  : 15
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(0x0f), aBitmap.GetPixelColor(960, 540).GetAlpha());
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf123557)
{
    createSdImpressDoc("pptx/trigger.pptx");
    saveAndReload(TestFilter::PPTX);
    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // Contains 2 interactive sequences and 3 triggered effects.
    assertXPath(pXmlDoc, "//draw:page", 1);
    assertXPath(pXmlDoc, "//draw:page/anim:par", 1);
    assertXPath(pXmlDoc,
                "//draw:page"
                "/anim:par[@presentation:node-type='timing-root']"
                "/anim:seq[@presentation:node-type='interactive-sequence']",
                2);
    assertXPath(pXmlDoc,
                "//draw:page"
                "/anim:par[@presentation:node-type='timing-root']"
                "/anim:seq[@presentation:node-type='interactive-sequence']"
                "/anim:par[@smil:begin]",
                3);
}

CPPUNIT_TEST_FIXTURE(SdExportTest3, testTdf126761)
{
    createSdImpressDoc("ppt/tdf126761.ppt");
    saveAndReload(TestFilter::ODP);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    // Check character underline, to make sure it has been set correctly
    sal_uInt32 nCharUnderline;
    xPropSet->getPropertyValue(u"CharUnderline"_ustr) >>= nCharUnderline;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), nCharUnderline);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
