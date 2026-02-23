/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_poppler.h>

#include "sdmodeltestbase.hxx"
#include <tools/color.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/colritem.hxx>
#include <editeng/numitem.hxx>

#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdasitm.hxx>
#include <svx/sdmetitm.hxx>
#include <animations/animationnodehelper.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <undo/undomanager.hxx>

#include <comphelper/lok.hxx>
#include <svx/svdograf.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/vectorgraphicdata.hxx>

using namespace ::com::sun::star;

namespace com::sun::star::uno
{
template <class T>
static std::ostream& operator<<(std::ostream& rStrm, const uno::Reference<T>& xRef)
{
    rStrm << xRef.get();
    return rStrm;
}
}

class SdImportTest3 : public SdModelTestBase
{
public:
    SdImportTest3()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdImportTest3, testGradientAngle)
{
    createSdDrawDoc("odg/gradient-angle.fodg");

    uno::Reference<lang::XMultiServiceFactory> const xDoc(mxComponent, uno::UNO_QUERY);

    awt::Gradient gradient;
    uno::Reference<container::XNameAccess> const xGradients(
        xDoc->createInstance(u"com.sun.star.drawing.GradientTable"_ustr), uno::UNO_QUERY);

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 38"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), gradient.Angle); // was: 3600

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 10"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270), gradient.Angle); // 27deg

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 11"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1146), gradient.Angle); // 2rad = 114.591deg

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 12"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 100grad

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 13"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3599), gradient.Angle); // -1

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 14"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3027), gradient.Angle); // -1rad = -57.295deg -> 302.704deg

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 15"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(300), gradient.Angle); // 3900

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 16"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(105), gradient.Angle); // 10.5deg

    CPPUNIT_ASSERT(xGradients->getByName(u"Gradient 17"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // \pi rad

    uno::Reference<container::XNameAccess> const xTranspGradients(
        xDoc->createInstance(u"com.sun.star.drawing.TransparencyGradientTable"_ustr),
        uno::UNO_QUERY);

    CPPUNIT_ASSERT(xTranspGradients->getByName(u"Transparency 2"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), gradient.Angle); // 1

    CPPUNIT_ASSERT(xTranspGradients->getByName(u"Transparency 1"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 90deg

    CPPUNIT_ASSERT(xTranspGradients->getByName(u"Transparency 3"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(573), gradient.Angle); // 1.0rad = 57.295deg

    CPPUNIT_ASSERT(xTranspGradients->getByName(u"Transparency 4"_ustr) >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // 1000grad = 900deg -> 180deg
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testN778859)
{
    createSdImpressDoc("pptx/n778859.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        // Get the object
        SdrObject* pObj = pPage->GetObj(1);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT(!pTxtObj->IsAutoFit());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testFdo68594)
{
    createSdImpressDoc("ppt/fdo68594.ppt");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    SdrObject* pObj = pPage->GetObj(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const SvxColorItem* pC = &pTxtObj->GetMergedItem(EE_CHAR_COLOR);
    CPPUNIT_ASSERT_MESSAGE("no color item", pC != nullptr);
    // Color should be black
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder color mismatch", COL_BLACK, pC->GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testPlaceholderPriority)
{
    createSdImpressDoc("ppt/placeholder-priority.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Missing placeholder", sal_uInt32(2),
                                 sal_uInt32(pPage->GetObjCount()));

    tools::Rectangle pObj1Rect(9100, 3500, 29619, 4038);
    SdrObject* pObj1 = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder position is wrong, check the placeholder priority",
                                 pObj1Rect, pObj1->GetCurrentBoundRect());

    tools::Rectangle pObj2Rect(9102, 8643, 29619, 12642);
    SdrObject* pObj2 = pPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder position is wrong, check the placeholder priority",
                                 pObj2Rect, pObj2->GetCurrentBoundRect());

    // If the placeholder positions are wrong, please check placeholder priority in Placeholders class.
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testFdo72998)
{
    createSdImpressDoc("pptx/cshapes.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(2));
        CPPUNIT_ASSERT(pObj);
        const SdrCustomShapeGeometryItem& rGeometryItem
            = pObj->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
        const css::uno::Any* pViewBox = rGeometryItem.GetPropertyValueByName(u"ViewBox"_ustr);
        CPPUNIT_ASSERT_MESSAGE("Missing ViewBox", pViewBox);
        css::awt::Rectangle aViewBox;
        CPPUNIT_ASSERT((*pViewBox >>= aViewBox));
        CPPUNIT_ASSERT_MESSAGE("Width should be zero - for forcing scale to 1", !aViewBox.Width);
        CPPUNIT_ASSERT_MESSAGE("Height should be zero - for forcing scale to 1", !aViewBox.Height);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testFdo77027)
{
    createSdImpressDoc("odp/fdo77027.odp");

    const SdrPage* pPage = GetPage(1);
    {
        SdrOle2Obj* const pObj = dynamic_cast<SdrOle2Obj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObj);

        // check that the fill style/color was actually imported
        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0xff6600), rColorItem.GetColorValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf97808)
{
    createSdImpressDoc("tdf97808.fodp");

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(u"graphics"_ustr),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"objectwithoutfill"_ustr),
                                               uno::UNO_QUERY);
    OUString lineend;
    CPPUNIT_ASSERT(xStyle->getPropertyValue(u"LineEndName"_ustr) >>= lineend);
    CPPUNIT_ASSERT_EQUAL(u"Arrow"_ustr, lineend);

    // the draw:marker-end="" did not override the style
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xLine(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    //uno::Reference<style::XStyle> xParent;
    uno::Reference<beans::XPropertySet> xParent;
    CPPUNIT_ASSERT(xLine->getPropertyValue(u"Style"_ustr) >>= xParent);
    CPPUNIT_ASSERT_EQUAL(xStyle, xParent);
    CPPUNIT_ASSERT(xLine->getPropertyValue(u"LineEndName"_ustr) >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString(), lineend);
}
CPPUNIT_TEST_FIXTURE(SdImportTest3, testFillStyleNone)
{
    createSdDrawDoc("tdf123841.odg");

    const SdrPage* pPage = GetPage(1);
    const SdrObject* pObj = pPage->GetObj(0);
    auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the shape's fill was FillStyle_SOLID, making the text of the shape unreadable.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testFdo64512)
{
    createSdImpressDoc("fdo64512.odp");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                 xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly three shapes", static_cast<sal_Int32>(3),
                                 xPage->getCount());

    uno::Reference<beans::XPropertySet> xConnectorShape(xPage->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no connector shape", xConnectorShape.is());

    uno::Reference<beans::XPropertySet> xSvgShape(
        xConnectorShape->getPropertyValue(u"StartShape"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no start shape", xSvgShape.is());

    uno::Reference<beans::XPropertySet> xCustomShape(
        xConnectorShape->getPropertyValue(u"EndShape"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no end shape", xCustomShape.is());

    uno::Reference<animations::XAnimationNodeSupplier> xAnimNodeSupplier(xPage,
                                                                         uno::UNO_QUERY_THROW);
    uno::Reference<animations::XAnimationNode> xRootNode(xAnimNodeSupplier->getAnimationNode());
    std::vector<uno::Reference<animations::XAnimationNode>> aAnimVector;
    anim::create_deep_vector(xRootNode, aAnimVector);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not 8 animation nodes", static_cast<std::size_t>(8),
                                 aAnimVector.size());

    uno::Reference<animations::XAnimate> xNode(aAnimVector[7], uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xTargetShape(xNode->getTarget(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("inner node not referencing svg shape", xTargetShape != xSvgShape);
}

// Unit test for importing charts
CPPUNIT_TEST_FIXTURE(SdImportTest3, testFdo71075)
{
    double values[] = { 12.0, 13.0, 14.0 };
    css::uno::Any aAny;

    createSdImpressDoc("fdo71075.odp");

    uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
    aAny = xPropSet->getPropertyValue(u"Model"_ustr);
    CPPUNIT_ASSERT_MESSAGE("The shape doesn't have the property", aAny.hasValue());

    uno::Reference<chart::XChartDocument> xChartDoc;
    aAny >>= xChartDoc;
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    uno::Reference<chart2::XChartDocument> xChart2Doc(xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart2Doc.is());

    uno::Reference<chart2::XCoordinateSystemContainer> xBCooSysCnt(xChart2Doc->getFirstDiagram(),
                                                                   uno::UNO_QUERY);
    uno::Sequence<uno::Reference<chart2::XCoordinateSystem>> aCooSysSeq(
        xBCooSysCnt->getCoordinateSystems());
    uno::Reference<chart2::XChartTypeContainer> xCTCnt(aCooSysSeq[0], uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeriesContainer> xDSCnt(xCTCnt->getChartTypes()[0], uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load data series", xDSCnt.is());
    uno::Sequence<uno::Reference<chart2::XDataSeries>> aSeriesSeq(xDSCnt->getDataSeries());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", static_cast<sal_Int32>(1),
                                 aSeriesSeq.getLength());
    uno::Reference<chart2::data::XDataSource> xSource(aSeriesSeq[0], uno::UNO_QUERY);
    uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> aSeqCnt(
        xSource->getDataSequences());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", static_cast<sal_Int32>(1),
                                 aSeqCnt.getLength());
    uno::Reference<chart2::data::XDataSequence> xValueSeq(aSeqCnt[0]->getValues());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Data count",
                                 static_cast<sal_Int32>(SAL_N_ELEMENTS(values)),
                                 xValueSeq->getData().getLength());
    uno::Reference<chart2::data::XNumericalDataSequence> xNumSeq(xValueSeq, uno::UNO_QUERY);
    uno::Sequence<double> aValues(xNumSeq->getNumericalData());
    for (sal_Int32 i = 0; i < xValueSeq->getData().getLength(); i++)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", values[i], aValues[i]);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testStrictOOXML)
{
    createSdImpressDoc("strict_ooxml.pptx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);
    getShapeFromPage(0, 0);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc870237)
{
    createSdImpressDoc("pptx/bnc870237.pptx");
    const SdrPage* pPage = GetPage(1);

    // Simulate a:ext inside dsp:txXfrm with changing the lower distance
    const SdrObjGroup* pGroupObj = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
    const SdrObject* pObj = pGroupObj->GetSubList()->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-158), pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9760), pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-158), pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-158), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf150789)
{
    createSdImpressDoc("pptx/tdf150789.pptx");
    const SdrPage* pPage = GetPage(1);

    // Simulate a:ext inside dsp:txXfrm with changing the lower distance of prst="upArrowCallout" textbox
    const SdrObjGroup* pGroupObj = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));

    const std::array<size_t, 2> nShapes = { 4, 7 };
    for (auto i : nShapes)
    {
        const SdrObject* pObj = pGroupObj->GetSubList()->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);

        OUString sShapeType;
        const SdrCustomShapeGeometryItem& rGeometryItem(
            pObj->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
        const css::uno::Any aAny = *rGeometryItem.GetPropertyValueByName("Type");
        if (aAny.hasValue())
            aAny >>= sShapeType;
        CPPUNIT_ASSERT_EQUAL(OUString("ooxml-upArrowCallout"), sShapeType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(395),
                             pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1424),
                             pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(395),
                             pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(395), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testCreationDate)
{
    createSdImpressDoc("fdo71434.pptx");
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    util::DateTime aDate = xDocumentProperties->getCreationDate();
    OUStringBuffer aBuffer;
    sax::Converter::convertDateTime(aBuffer, aDate, nullptr);
    // Metadata wasn't imported, this was 0000-00-00.
    CPPUNIT_ASSERT_EQUAL(u"2013-11-09T10:37:56"_ustr, aBuffer.makeStringAndClear());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testMultiColTexts)
{
    createSdImpressDoc("pptx/multicol.pptx");
    const SdrPage* pPage = GetPage(1);

    auto pTextObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTextObj);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pTextObj->GetTextColumnsSpacing());

    auto pMasterTextObj = DynCastSdrTextObj(pPage->TRG_GetMasterPage().GetObj(0));
    CPPUNIT_ASSERT(pMasterTextObj);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pMasterTextObj->GetTextColumnsNumber());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pMasterTextObj->GetTextColumnsSpacing());

    uno::Reference<text::XTextRange> xText(pMasterTextObj->getUnoShape(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"mastershape1\nmastershape2"_ustr, xText->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testPredefinedTableStyle)
{
    // 073A0DAA-6AF3-43AB-8588-CEC1D06C72B9 (Medium Style 2)
    createSdImpressDoc("pptx/predefined-table-style.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_GRAY3, nColor);

    xCell.set(xTable->getCellByPosition(0, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xe7e7e7), nColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc887225)
{
    createSdImpressDoc("pptx/bnc887225.pptx");
    // In the document, lastRow and lastCol table properties are used.
    // Make sure styles are set properly for individual cells.

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(1, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xd1deef), nColor);

    xCell.set(xTable->getCellByPosition(1, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xe9eff7), nColor);

    xCell.set(xTable->getCellByPosition(1, 4), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(3, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(3, 4), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc584721_1)
{
    // Title text shape on the master page contained wrong text.

    createSdImpressDoc("pptx/bnc584721_1_2.pptx");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    SdrObject* pObj = pPage->GetObj(0);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(u"Click to edit Master title style"_ustr, aEdit.GetText(0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc584721_2)
{
    // Import created an extra/unneeded outliner shape on the master slide next to the imported title shape.

    createSdImpressDoc("pptx/bnc584721_1_2.pptx");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc591147)
{
    createSdImpressDoc("pptx/bnc591147.pptx");

    // In the document, there are two slides with media files.
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDoc->getDrawPages()->getCount());

    // First page has video file inserted
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    //uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));
    OUString sVideoURL(u"emptyURL"_ustr);
    bool bSuccess = xPropSet->getPropertyValue(u"MediaURL"_ustr) >>= sVideoURL;
    CPPUNIT_ASSERT_MESSAGE("MediaURL property is not set", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sVideoURL.isEmpty());

    // Second page has audio file inserted
    xPage.set(xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    xPropSet.set(getShape(0, xPage));
    OUString sAudioURL(u"emptyURL"_ustr);
    bSuccess = xPropSet->getPropertyValue(u"MediaURL"_ustr) >>= sAudioURL;
    CPPUNIT_ASSERT_MESSAGE("MediaURL property is not set", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sAudioURL.isEmpty());

    CPPUNIT_ASSERT_MESSAGE("sAudioURL and sVideoURL should not be equal", sAudioURL != sVideoURL);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc584721_4)
{
    // Black text was imported as white because of wrong caching mechanism

    createSdImpressDoc("pptx/bnc584721_4.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 1));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;

    // Color should be black
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc904423)
{
    // Here the problem was that different fill properties were applied in wrong order on the shape
    // Right order: 1) master slide fill style, 2) theme, 3) direct formatting

    createSdImpressDoc("pptx/bnc904423.pptx");

    const SdrPage* pPage = GetPage(1);
    // First shape's background color is defined on master slide
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x00CC99), rColorItem.GetColorValue());
    }

    // Second shape's background color is defined by theme
    {
        SdrObject* const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x3333CC), rColorItem.GetColorValue());
    }

    // Third shape's background color is defined by direct formatting
    {
        SdrObject* const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, rColorItem.GetColorValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testShapeLineStyle)
{
    // Here the problem was that different line properties were applied in wrong order on the shape
    // Right order: 1) master slide line style, 2) theme, 3) direct formatting
    createSdImpressDoc("pptx/ShapeLineProperties.pptx");

    const SdrPage* pPage = GetPage(1);
    // First shape's line style is defined on master slide
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = pObj->GetMergedItem(XATTR_LINEWIDTH);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(132), rWidthItem.GetValue());
    }

    // Second shape's line style is defined by theme
    {
        SdrObject* const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x3333CC), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = pObj->GetMergedItem(XATTR_LINEWIDTH);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), rWidthItem.GetValue());
    }

    // Third shape's line style is defined by direct formatting
    {
        SdrObject* const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x7030A0), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = pObj->GetMergedItem(XATTR_LINEWIDTH);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(176), rWidthItem.GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTableBorderLineStyle)
{
    createSdImpressDoc("pptx/tableBorderLineStyle.pptx");

    // TODO: If you are working on improvement of table border line style
    // support, then expect this unit test to fail.

    const sal_Int16 nObjBorderLineStyles[]
        = { ::table::BorderLineStyle::DASHED,   ::table::BorderLineStyle::DASH_DOT_DOT,
            ::table::BorderLineStyle::DASH_DOT, ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,   ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,   ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::DASH_DOT, ::table::BorderLineStyle::SOLID,
            ::table::BorderLineStyle::NONE };

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(nObjBorderLineStyles), pPage->GetObjCount());

    sdr::table::SdrTableObj* pTableObj;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    table::BorderLine2 aBorderLine;

    for (size_t i = 0; i < SAL_N_ELEMENTS(nObjBorderLineStyles); i++)
    {
        pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(i));
        CPPUNIT_ASSERT(pTableObj);
        xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
        xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
        xCell->getPropertyValue(u"TopBorder"_ustr) >>= aBorderLine;
        if (aBorderLine.LineWidth > 0)
        {
            CPPUNIT_ASSERT_EQUAL(nObjBorderLineStyles[i], aBorderLine.LineStyle);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTableMergedCellsBorderLineStyle)
{
    createSdImpressDoc("pptx/tdf149865.pptx");

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    table::BorderLine2 aBorderLine;

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    xCell.set(xTable->getCellByPosition(4, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"RightBorder"_ustr) >>= aBorderLine;
    table::BorderLine2 expectedRight(0x30ba78, 0, 17, 0, 0, 17);
    CPPUNIT_ASSERT_EQUAL(expectedRight.LineStyle, aBorderLine.LineStyle);
    CPPUNIT_ASSERT_EQUAL(expectedRight.Color, aBorderLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedRight.LineWidth, aBorderLine.LineWidth);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc862510_6)
{
    // Black text was imported instead of gray

    createSdImpressDoc("pptx/bnc862510_6.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;

    // Color should be gray
    CPPUNIT_ASSERT_EQUAL(Color(0x8B8B8B), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc862510_7)
{
    // Title shape's text was aligned to left instead of center.

    createSdImpressDoc("pptx/bnc862510_7.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    sal_Int16 nParaAdjust = 0;
    xPropSet->getPropertyValue(u"ParaAdjust"_ustr) >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

#if ENABLE_PDFIMPORT
// These tests use the old PDF-importing logic, which imports PDF elements as
// SD elements. This suffered many issues, and therefore wasn't ideal.
// The old PDF importer relied on an open-source project (xpdf) with an
// incompatible license (gpl), which has to be interfaced via an out-of-process
// library wrapper process. The resulting imported document was inaccurate
// and often very slow and with large memory footprint.
// Instead, PDFium offers state-of-the-art PDF importing logic,
// which is well-maintained and renders PDFs into images with high accuracy.
// So, the idea is to import PDFs as images using PDFium, which has a very
// high quality (and is much faster) than importing individual editable elements.
// So that's the "new" way of importing.
// The user then breaks the image to editable elements (which is not perfect,
// but very close to the old way), only if they need editing ability.
// PDFium should overall be better, and where it isn't, we just need to improve it.
// So these tests aren't really useful anymore. They should be modified to do
// import+break and then check the results. But that isn't straight-forward and
// currently await volunteering time to implement.

CPPUNIT_TEST_FIXTURE(SdImportTest3, testPDFImportShared)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    comphelper::LibreOfficeKit::setActive();

    createSdDrawDoc("pdf/multipage.pdf");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // This test is to verify that we share the PDF stream linked to each
    // Graphic instance in the imported document.
    // Since we import PDFs as images, we support attaching the original
    // PDF with each image to allow for advanced editing.
    // Here we iterate over all Graphic instances embedded in the pages
    // and verify that they all point to the same object in memory.
    std::vector<Graphic> aGraphics;

    for (int nPageIndex = 0; nPageIndex < pDoc->GetPageCount(); ++nPageIndex)
    {
        const SdrPage* pPage = GetPage(nPageIndex);
        if (pPage == nullptr)
            break;

        for (size_t nObjIndex = 0; nObjIndex < pPage->GetObjCount(); ++nObjIndex)
        {
            SdrObject* pObject = pPage->GetObj(nObjIndex);
            if (pObject == nullptr)
                continue;

            SdrGrafObj* pSdrGrafObj = dynamic_cast<SdrGrafObj*>(pObject);
            if (pSdrGrafObj == nullptr)
                continue;

            const GraphicObject aGraphicObject = pSdrGrafObj->GetGraphicObject().GetGraphic();
            const Graphic& rGraphic = aGraphicObject.GetGraphic();
            CPPUNIT_ASSERT_MESSAGE(
                "After loading, the PDF shouldn't have the primitive sequence created yet",
                !rGraphic.getVectorGraphicData()->isPrimitiveSequenceCreated());
            aGraphics.push_back(rGraphic);
        }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected more than one page.", size_t(9), aGraphics.size());

    Graphic const& rFirstGraphic = aGraphics[0];

    for (size_t i = 0; i < aGraphics.size(); ++i)
    {
        Graphic const& rGraphic = aGraphics[i];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Expected all PDF streams to be identical.",
            rFirstGraphic.getVectorGraphicData()->getBinaryDataContainer().getData(),
            rGraphic.getVectorGraphicData()->getBinaryDataContainer().getData());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected all GfxLinks to be identical.",
                                     rFirstGraphic.GetSharedGfxLink().get(),
                                     rGraphic.GetSharedGfxLink().get());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Page number doesn't match expected", sal_Int32(i),
                                     rGraphic.getPageNumber());
    }
}

#if defined(IMPORT_PDF_ELEMENTS)

CPPUNIT_TEST_FIXTURE(SdImportTest3, testPDFImport)
{
    createSdDrawDoc("pdf/txtpic.pdf");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("no exactly two shapes", static_cast<sal_Int32>(2),
                                 xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE("not a text shape", xText.is());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testPDFImportSkipImages)
{
    auto pParams = std::make_shared<SfxAllItemSet>(SfxGetpApp()->GetPool());
    pParams->Put(SfxStringItem(SID_FILE_FILTEROPTIONS, "SkipImages"));

    createSdDrawDoc("pdf/txtpic.pdf"), PDF, pParams);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("no exactly one shape", static_cast<sal_Int32>(1),
                                 xPage->getCount());

    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE("not a text shape", xText.is());
}

#endif
#endif

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBulletSuffix)
{
    createSdImpressDoc("pptx/n83889.pptx");

    // check suffix of the char bullet
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(1).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's suffix is wrong!",
                                 pNumFmt->GetNumRule().GetLevel(0).GetSuffix(), OUString());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testBnc910045)
{
    // Problem with table style which defines cell color with fill style

    createSdImpressDoc("pptx/bnc910045.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4f81bd), nColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testRowHeight_n80340)
{
    createSdImpressDoc("pptx/n80340.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    sal_Int32 nHeight;
    uno::Reference<css::table::XTable> xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XTableRows> xRows(xTable->getRows(), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xRefRow(xRows->getByIndex(0), uno::UNO_QUERY_THROW);
    xRefRow->getPropertyValue(u"Height"_ustr) >>= nHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(508), nHeight);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testRowHeight_tableScale)
{
    createSdImpressDoc("pptx/tablescale.pptx");
    const SdrPage* pPage2 = GetPage(1);

    sdr::table::SdrTableObj* pTableObj2 = dynamic_cast<sdr::table::SdrTableObj*>(pPage2->GetObj(0));
    CPPUNIT_ASSERT(pTableObj2);

    uno::Reference<css::table::XTable> xTable2(pTableObj2->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XTableRows> xRows2(xTable2->getRows(), uno::UNO_SET_THROW);

    sal_Int32 nHeight;
    for (sal_Int32 nRow = 0; nRow < 7; ++nRow)
    {
        uno::Reference<beans::XPropertySet> xRefRow2(xRows2->getByIndex(nRow),
                                                     uno::UNO_QUERY_THROW);
        xRefRow2->getPropertyValue(u"Height"_ustr) >>= nHeight;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(800), nHeight);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf93830)
{
    // Text shape offset was ignored

    createSdImpressDoc("pptx/tdf93830.pptx");
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    // Get the first text box from group shape
    uno::Reference<container::XIndexAccess> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xShape->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no textbox shape", xPropSet.is());

    sal_Int32 nTextLeftDistance = 0;
    xPropSet->getPropertyValue(u"TextLeftDistance"_ustr) >>= nTextLeftDistance;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4024), nTextLeftDistance);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf165732)
{
    // Text insets larger than shape dimension should be clamped symmetrically.
    createSdImpressDoc("pptx/tdf165732.pptx");
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    // Shape "2": each side reduced by 1: 200 -> 199.
    uno::Reference<beans::XPropertySet> xShape0(xPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nLeft = 0, nRight = 0;
    xShape0->getPropertyValue(u"TextLeftDistance"_ustr) >>= nLeft;
    xShape0->getPropertyValue(u"TextRightDistance"_ustr) >>= nRight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(199), nLeft);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(199), nRight);

    // Shape "1": no clamping.
    uno::Reference<beans::XPropertySet> xShape1(xPage->getByIndex(1), uno::UNO_QUERY);
    sal_Int32 nLeft1 = 0;
    xShape1->getPropertyValue(u"TextLeftDistance"_ustr) >>= nLeft1;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nLeft1);

    // Shape "5": top and bottom reduced by 17.5: 200 -> 183 (rounded).
    uno::Reference<beans::XPropertySet> xShape7(xPage->getByIndex(7), uno::UNO_QUERY);
    sal_Int32 nUpper = 0, nLower = 0;
    xShape7->getPropertyValue(u"TextUpperDistance"_ustr) >>= nUpper;
    xShape7->getPropertyValue(u"TextLowerDistance"_ustr) >>= nLower;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(183), nUpper);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(183), nLower);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf127129)
{
    createSdImpressDoc("pptx/tdf127129.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);

    // Without the accompanying fix in place, the highlight would be -1
    Color nCharBackColor;
    xPropSet->getPropertyValue(u"CharBackColor"_ustr) >>= nCharBackColor;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, nCharBackColor);
}
CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf93097)
{
    // Throwing metadata import aborted the filter, check that metadata is now imported.

    createSdImpressDoc("pptx/tdf93097.pptx");
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(u"ss"_ustr, xDocumentProperties->getTitle());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf62255)
{
    createSdImpressDoc("pptx/tdf62255.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj;
    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    css::uno::Any aAny;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    aAny = xCell->getPropertyValue(u"FillStyle"_ustr);

    if (aAny.hasValue())
    {
        drawing::FillStyle aFillStyle;
        aAny >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, aFillStyle);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf89927)
{
    createSdImpressDoc("pptx/tdf89927.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf103800)
{
    createSdImpressDoc("pptx/tdf103800.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xC00000), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf151767)
{
    createSdImpressDoc("pptx/tdf151767.pptx");

    const SdrPage* pPage = GetPage(1);
    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);

    table::BorderLine2 aLeft;
    xCell->getPropertyValue(u"LeftBorder"_ustr) >>= aLeft;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The left border is missing!", true, aLeft.LineWidth > 0);

    table::BorderLine2 aRight;
    xCell->getPropertyValue(u"RightBorder"_ustr) >>= aRight;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The right border is missing!", true, aRight.LineWidth > 0);

    table::BorderLine2 aTop;
    xCell->getPropertyValue(u"TopBorder"_ustr) >>= aTop;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The top border is missing!", true, aTop.LineWidth > 0);

    table::BorderLine2 aBottom;
    xCell->getPropertyValue(u"BottomBorder"_ustr) >>= aBottom;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The bottom border is missing!", true, aBottom.LineWidth > 0);
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf152070)
{
    createSdImpressDoc("pptx/tdf152070.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xBackgroundProps(
        xPropSet->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(50), // 50%
        xBackgroundProps->getPropertyValue(u"FillBitmapPositionOffsetX"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(83), // 83%
        xBackgroundProps->getPropertyValue(u"FillBitmapPositionOffsetY"_ustr).get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf143603)
{
    createSdImpressDoc("odp/tdf143603.fodp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    // Loading FODP used to record UNDO entries - this was 2
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetUndoManager()->GetUndoActionCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest3, testTdf111927)
{
    createSdImpressDoc("pptx/tdf163239.pptx");

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();

    // 1st slide
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"Title Slide"_ustr), uno::UNO_QUERY);
    {
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"title"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(44.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"subtitle"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(32.0f, xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
    }
    // 2nd slide
    xStyleFamily.set(xStyleFamilies->getByName(u"simple text placeholder"_ustr), uno::UNO_QUERY);
    {
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName(u"outline1"_ustr),
                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

        float nCharHeight = xPropSet->getPropertyValue(u"CharHeight"_ustr).get<float>();
        CPPUNIT_ASSERT_EQUAL(60.0f, nCharHeight);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                             xPropSet->getPropertyValue(u"ParaAdjust"_ustr).get<sal_Int16>());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
