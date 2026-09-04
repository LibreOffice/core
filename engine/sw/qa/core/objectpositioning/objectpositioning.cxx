/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <comphelper/propertysequence.hxx>
#include <editeng/ulspitem.hxx>

#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <flyfrm.hxx>
#include <frmatr.hxx>
#include <IDocumentSettingAccess.hxx>
#include <view.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>

#include <vcl/scheduler.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/// Covers sw/source/core/objectpositioning/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/objectpositioning/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testOverlapCrash)
{
    // Load a document with 2 images.
    createSwDoc("overlap-crash.odt");

    // Change their anchor type to to-char.
    uno::Reference<beans::XPropertySet> xShape1(getShape(1), uno::UNO_QUERY);
    xShape1->setPropertyValue(u"AnchorType"_ustr,
                              cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<beans::XPropertySet> xShape2(getShape(1), uno::UNO_QUERY);
    xShape2->setPropertyValue(u"AnchorType"_ustr,
                              cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));

    // Insert a new paragraph at the start.
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    // Without the accompanying fix in place, this test would have crashed.
    pWrtShell->SplitNode();
}

CPPUNIT_TEST_FIXTURE(Test, testImgMoveCrash)
{
    createSwDoc("tdf154863-img-move-crash.docx");
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Image26"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"VertOrient"_ustr, cpo::uno::Any(static_cast<sal_Int32>(0)));
    xShapeProps->setPropertyValue(u"VertOrientPosition"_ustr,
                                  cpo::uno::Any(static_cast<sal_Int32>(3000)));
    Scheduler::ProcessEventsToIdle();
    // Crash expected before assert if bug exists
    CPPUNIT_ASSERT(true);
}

CPPUNIT_TEST_FIXTURE(Test, testVertPosFromBottom)
{
    // Create a document, insert a shape and position it 1cm above the bottom of the body area.
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"AnchorType"_ustr,
                                  cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProps->setPropertyValue(u"VertOrient"_ustr, cpo::uno::Any(text::VertOrientation::NONE));
    xShapeProps->setPropertyValue(u"VertOrientRelation"_ustr,
                                  cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    xShapeProps->setPropertyValue(u"VertOrientPosition"_ustr,
                                  cpo::uno::Any(static_cast<sal_Int32>(-11000)));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // Verify that the distance between the body and anchored object bottom is indeed around 1cm.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds", "bottom").toInt32();
    sal_Int32 nAnchoredBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "bottom").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 565
    // - Actual  : 9035
    // i.e. the vertical position was from-top, not from-bottom.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(565), nBodyBottom - nAnchoredBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testVertAlignBottomMargin)
{
    // Create a document, insert three shapes and align it the bottom,center,top of page print area bottom.
    // The size of shapes are 284 ~ 0.5cm
    // The height of page print area bottom is 1134 ~ 2cm
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

    //Create first shape and align bottom of page print area bottom.
    uno::Reference<drawing::XShape> xShapeBottom(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeBottom->setSize(awt::Size(500, 500));
    uno::Reference<beans::XPropertySet> xShapePropsBottom(xShapeBottom, uno::UNO_QUERY);
    xShapePropsBottom->setPropertyValue(u"AnchorType"_ustr,
                                        cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsBottom->setPropertyValue(u"VertOrient"_ustr,
                                        cpo::uno::Any(text::VertOrientation::BOTTOM));
    xShapePropsBottom->setPropertyValue(
        u"VertOrientRelation"_ustr, cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierBottom(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierBottom->getDrawPage()->add(xShapeBottom);

    //Create second shape and align center of page print area bottom.
    uno::Reference<drawing::XShape> xShapeCenter(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeCenter->setSize(awt::Size(500, 500));
    uno::Reference<beans::XPropertySet> xShapePropsCenter(xShapeCenter, uno::UNO_QUERY);
    xShapePropsCenter->setPropertyValue(u"AnchorType"_ustr,
                                        cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsCenter->setPropertyValue(u"VertOrient"_ustr,
                                        cpo::uno::Any(text::VertOrientation::CENTER));
    xShapePropsCenter->setPropertyValue(
        u"VertOrientRelation"_ustr, cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierCenter(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierCenter->getDrawPage()->add(xShapeCenter);

    //Create third shape and align top of page print area bottom.
    uno::Reference<drawing::XShape> xShapeTop(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeTop->setSize(awt::Size(500, 500));
    uno::Reference<beans::XPropertySet> xShapePropsTop(xShapeTop, uno::UNO_QUERY);
    xShapePropsTop->setPropertyValue(u"AnchorType"_ustr,
                                     cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsTop->setPropertyValue(u"VertOrient"_ustr, cpo::uno::Any(text::VertOrientation::TOP));
    xShapePropsTop->setPropertyValue(u"VertOrientRelation"_ustr,
                                     cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierTop(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierTop->getDrawPage()->add(xShapeTop);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds", "bottom").toInt32(); //14989
    sal_Int32 nPageBottom = getXPath(pXmlDoc, "//page/infos/bounds", "bottom").toInt32(); //16123
    sal_Int32 nFirstShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds", "bottom")
              .toInt32(); //16124
    sal_Int32 nSecondShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds", "bottom")
              .toInt32(); //15699
    sal_Int32 nSecondShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds", "top").toInt32(); //15414
    sal_Int32 nThirdShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds", "top").toInt32(); //14989

    // Verify that the distance between the bottom of page and bottom of first shape is around 0cm. (align=bottom)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nFirstShapeBottom - nPageBottom);
    // Verify that the distance between the bottom of page and bottom of second shape is around 0.75cm and
    // verify that the distance between the bottom of body and top of second shape is around 0.75cm.(align=center)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(424), nPageBottom - nSecondShapeBottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(425), nSecondShapeTop - nBodyBottom);
    // Verify that the distance between the bottom of body and top of third shape is around 0cm. (align=top)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nThirdShapeTop - nBodyBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testVertAlignBottomMarginWithFooter)
{
    // Load an empty document with footer.
    createSwDoc("bottom-margin-with-footer.docx");
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

    // Insert three shapes and align it the bottom,center,top of page print area bottom.
    // The height of page print area bottom is 2268 ~ 4cm.
    // The size of shapes are 567 ~ 1cm
    // Create first shape and align bottom of page print area bottom.
    uno::Reference<drawing::XShape> xShapeBottom(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeBottom->setSize(awt::Size(1000, 1000));
    uno::Reference<beans::XPropertySet> xShapePropsBottom(xShapeBottom, uno::UNO_QUERY);
    xShapePropsBottom->setPropertyValue(u"AnchorType"_ustr,
                                        cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsBottom->setPropertyValue(u"VertOrient"_ustr,
                                        cpo::uno::Any(text::VertOrientation::BOTTOM));
    xShapePropsBottom->setPropertyValue(
        u"VertOrientRelation"_ustr, cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierBottom(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierBottom->getDrawPage()->add(xShapeBottom);

    // Create second shape and align center of page print area bottom.
    uno::Reference<drawing::XShape> xShapeCenter(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeCenter->setSize(awt::Size(1000, 1000));
    uno::Reference<beans::XPropertySet> xShapePropsCenter(xShapeCenter, uno::UNO_QUERY);
    xShapePropsCenter->setPropertyValue(u"AnchorType"_ustr,
                                        cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsCenter->setPropertyValue(u"VertOrient"_ustr,
                                        cpo::uno::Any(text::VertOrientation::CENTER));
    xShapePropsCenter->setPropertyValue(
        u"VertOrientRelation"_ustr, cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierCenter(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierCenter->getDrawPage()->add(xShapeCenter);

    // Create third shape and align top of page print area bottom.
    uno::Reference<drawing::XShape> xShapeTop(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeTop->setSize(awt::Size(1000, 1000));
    uno::Reference<beans::XPropertySet> xShapePropsTop(xShapeTop, uno::UNO_QUERY);
    xShapePropsTop->setPropertyValue(u"AnchorType"_ustr,
                                     cpo::uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsTop->setPropertyValue(u"VertOrient"_ustr, cpo::uno::Any(text::VertOrientation::TOP));
    xShapePropsTop->setPropertyValue(u"VertOrientRelation"_ustr,
                                     cpo::uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierTop(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierTop->getDrawPage()->add(xShapeTop);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds", "bottom").toInt32(); //14853
    sal_Int32 nPageBottom = getXPath(pXmlDoc, "//page/infos/bounds", "bottom").toInt32(); //17121
    sal_Int32 nFirstShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds", "bottom")
              .toInt32(); //17122
    sal_Int32 nSecondShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds", "top").toInt32(); //15703
    sal_Int32 nSecondShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds", "bottom")
              .toInt32(); //16272
    sal_Int32 nThirdShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds", "top").toInt32(); //14853

    // Verify that the distance between the bottom of page and bottom of first shape is around 0cm. (align=bottom)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nFirstShapeBottom - nPageBottom);
    // Verify that the distance between the bottom of page and bottom of second shape is around 1.5cm and
    // verify that the distance between the bottom of body and top of second shape is around 1.5cm.(align=center)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(849), nPageBottom - nSecondShapeBottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(850), nSecondShapeTop - nBodyBottom);
    // Verify that the distance between the bottom of body and top of third shape is around 0cm. (align=top)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nThirdShapeTop - nBodyBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testInsideOutsideVertAlignBottomMargin)
{
    // Load a document, with two shapes.
    // The shapes align the outside and inside of page print area bottom.
    createSwDoc("inside-outside-vert-align.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds", "bottom").toInt32(); //15704
    sal_Int32 nPageBottom = getXPath(pXmlDoc, "//page/infos/bounds", "bottom").toInt32(); //17121
    sal_Int32 nFirstShapeOutside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds", "bottom")
              .toInt32(); //17098
    sal_Int32 nSecondShapeInside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds", "top").toInt32(); //15694

    // Verify that the distance between the bottom of page and bottom of first shape is around 0cm. (align=outside)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), nPageBottom - nFirstShapeOutside);
    // Verify that the distance between the bottom of body and top of second shape is around 0cm. (align=inside)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nBodyBottom - nSecondShapeInside);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLVertAlignBottomMargin)
{
    // Load a document, with five shapes.
    // The shapes align the top,center,bottom,outside and inside of page print area bottom.
    // The height of page print area bottom is 4320 ~ 7.62cm.
    // The size of shapes are 442 ~ 0.78cm
    createSwDoc("vml-vertical-alignment.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds", "bottom").toInt32(); //11803
    sal_Int32 nPageBottom = getXPath(pXmlDoc, "//page/infos/bounds", "bottom").toInt32(); //16123

    sal_Int32 nFirstVMLShapeInside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds", "top").toInt32(); //11802
    sal_Int32 nSecondVMLShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds", "bottom")
              .toInt32(); //16124
    sal_Int32 nThirdVMLShapeCenterBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds", "bottom")
              .toInt32(); //14185
    sal_Int32 nThirdVMLShapeCenterTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds", "top").toInt32(); //13741
    sal_Int32 nFourthVMLShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[4]/bounds", "top").toInt32(); //11802
    sal_Int32 nFifthVMLShapeOutside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[5]/bounds", "bottom")
              .toInt32(); //16124

    // Verify that the distance between the bottom of body and top of first VMLshape is around 0cm. (align=inside)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nBodyBottom - nFirstVMLShapeInside);
    // Verify that the distance between the bottom of page and bottom of second VMLshape is around 0cm. (align=bottom)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nSecondVMLShapeBottom - nPageBottom);
    // Verify that the distance between the bottom of page and bottom of third VMLshape is around 3.4cm and
    // verify that the distance between the bottom of body and top of third shape is around 3.4cm.(align=center)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1938), nPageBottom - nThirdVMLShapeCenterBottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1938), nThirdVMLShapeCenterTop - nBodyBottom);
    // Verify that the distance between the bottom of body and top of fourth VMLshape is around 0cm. (align=top)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nBodyBottom - nFourthVMLShapeTop);
    // Verify that the distance between the bottom of page and bottom of fifth shape is around 0cm. (align=outside)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nFifthVMLShapeOutside - nPageBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableOverlapNever)
{
    // Given a document with two floating tables, positioned in a way that normally these would
    // overlap, but SwFormatWrapInfluenceOnObjPos::mbAllowOverlap == false explicitly asks to avoid
    // overlaps:
    createSwDoc("floattable-tbl-overlap.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure no overlap happens:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPage1Objs.size());
    auto pFlyFrame1 = rPage1Objs[0]->DynCastFlyFrame();
    auto pFlyFrame2 = rPage1Objs[1]->DynCastFlyFrame();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 2291
    // - Actual  : 2175
    // i.e. the 2nd floating table overlapped with the first one.
    CPPUNIT_ASSERT_GREATER(pFlyFrame1->getFrameArea().Bottom(), pFlyFrame2->getFrameArea().Top());
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableVertOrientTop)
{
    // Given a document with a vert-orient=from-top anchored floating table:
    createSwDoc("floattable-vert-orient-top.odt");

    // When laying out that document:
    calcLayout();

    // Then make sure we correctly split the table to two pages:
    // Without the accompanying fix in place, this test would have produced a layout loop.
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableFollowWrongPage)
{
    // Given a document with text on 2 pages, the first page has a fly frame that can split:
    createSwDoc("floattable-follow-on-wrong-page.odt");

    // When increasing the top and bottom margins from 0.5cm to 2.5cm:
    SwDoc* pDoc = getSwDoc();
    SwPageDesc aStandard(pDoc->GetPageDesc(0));
    SvxULSpaceItem aPageMargin(aStandard.GetMaster().GetULSpace());
    aPageMargin.SetUpper(1417);
    aPageMargin.SetLower(1417);
    aStandard.GetMaster().SetFormatAttr(aPageMargin);
    pDoc->ChgPageDesc(0, aStandard);

    // Then make sure the first and second page has fly frames:
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    // Without the accompanying fix in place, this test would have failed, page 2 had no fly frame
    // (page 3 had one).
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableOverlapCell)
{
    // Given a document with floating tables, overlapping, but anchored to different table cells:
    createSwDoc("floattable-overlap-cell.docx");

    // When laying out the document:
    // Without the accompanying fix in place, this resulted in a layout loop.
    calcLayout();

    // Then make sure the layout doesn't loop and results in a single page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(!pPage1->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testDoNotMirrorRtlDrawObjsLayout)
{
    // Given a document with an RTL paragraph, Word-style compat flag is enabled:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    auto& rIDSA = pDoc->getIDocumentSettingAccess();
    rIDSA.set(DocumentSettingId::DO_NOT_MIRROR_RTL_DRAW_OBJS, true);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();
    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END> aSet(rView.GetPool());
    SvxFrameDirectionItem aDirection(SvxFrameDirection::Horizontal_RL_TB, RES_FRAMEDIR);
    aSet.Put(aDirection);
    pWrtShell->SetAttrSet(aSet, SetAttrMode::DEFAULT, nullptr, /*bParagraphSetting=*/true);
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPageFrame = pLayout->Lower()->DynCastPageFrame();
    SwFrame* pBodyFrame = pPageFrame->GetLower();

    // When inserting a graphic on the middle of the right margin:
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_CHAR);
    aFrameSet.Put(aAnchor);
    // Default margin is 1440, this is 1440/2.
    SwFormatFrameSize aSize(SwFrameSize::Fixed, 720, 720);
    aFrameSet.Put(aSize);
    // This is 1440/4.
    SwFormatHoriOrient aOrient(pBodyFrame->getFrameArea().Right() + 360);
    aFrameSet.Put(aOrient);
    Graphic aGrf;
    pWrtShell->SwFEShell::Insert(OUString(), OUString(), &aGrf, &aFrameSet);

    // Then make sure that the image is on the right margin:
    SwTwips nBodyRight = pBodyFrame->getFrameArea().Right();
    CPPUNIT_ASSERT(pPageFrame->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPageFrame->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    const SwAnchoredObject* pAnchored = rPageObjs[0];
    Point aAnchoredCenter = pAnchored->GetDrawObj()->GetLastBoundRect().Center();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 11389
    // - Actual  : 643
    // i.e. the graphic was on the left margin, not on the right margin.
    CPPUNIT_ASSERT_GREATER(nBodyRight, aAnchoredCenter.getX());
}

CPPUNIT_TEST_FIXTURE(Test, testDoNotCaptureDrawObjsOnPageDrawWrapNone)
{
    // Given a document with a draw object on page 2, wrap type is set to none (not through):
    createSwDoc("do-not-capture-draw-objs-on-page-draw-wrap-none.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure the draw object is captured on page 2:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rPage2Objs.size());
    SwAnchoredObject* pDrawObj = rPage2Objs[0];
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_DRAWFRMFMT),
                         pDrawObj->GetFrameFormat()->Which());
    SwTwips nDrawObjTop = pDrawObj->GetObjRect().Top();
    SwTwips nPage2Top = pPage2->getFrameArea().Top();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 17383
    // - Actual  : 13518
    // i.e. the draw object was way above the page 2 rectangle, instead of inside it (apart from
    // some <1px difference).
    CPPUNIT_ASSERT_GREATER(nPage2Top - MINFLY, nDrawObjTop);
}

CPPUNIT_TEST_FIXTURE(Test, testInsertShapeOnAsCharImg_tdf16890)
{
    // Given a document with an image anchored as character
    createSwDoc("as_char_image.docx");
    SwDoc* pDoc = getSwDoc();

    // Insert a shape over it
    cpo::uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { u"KeyModifier"_ustr, cpo::uno::Any(KEY_MOD1) } }));
    dispatchCommand(mxComponent, u".uno:BasicShapes.rectangle"_ustr, aArgs);

    // Check that the new shape is anchored at para (i.e. has an anchor node)
    const auto& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(size_t(o3tl::make_unsigned(2)), rFrmFormats.size());
    auto pShape = rFrmFormats[1];
    CPPUNIT_ASSERT(pShape);
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: pShape->GetAnchor().GetAnchorNode()
    CPPUNIT_ASSERT(pShape->GetAnchor().GetAnchorNode());
}

CPPUNIT_TEST_FIXTURE(Test, testDoNotCaptureDrawObjsDrawObjNoCapture)
{
    // Given a document with a draw object, which is positioned outside the page frame:
    createSwDoc("do-not-capture-draw-objs-draw-obj-no-capture.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure the draw object is not captured on page 1:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->GetSortedObjs());
    const SwSortedObjs& rPageObjs = *pPage->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPageObjs.size());
    SwAnchoredObject* pDrawObj = rPageObjs[0];
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_DRAWFRMFMT),
                         pDrawObj->GetFrameFormat()->Which());
    SwTwips nDrawObjLeft = pDrawObj->GetObjRect().Left();
    SwTwips nPageLeft = pPage->getFrameArea().Left();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 284
    // - Actual  : 284
    // i.e. the draw object was captured inside the page frame, but it was not in Word.
    CPPUNIT_ASSERT_GREATER(nDrawObjLeft, nPageLeft);
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableNestedOverlap)
{
    // Given a document with an outer inline table, an inner floating table, the floating table's
    // overlap is set to 'never':
    createSwDoc("floattable-nested-overlap.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure the content fits two pages and the floating table is split:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwPageFrame* pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    SwPageFrame* pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    // Without the accompanying fix in place, this test would have failed, the content was laid out
    // on 3 pages in Writer.
    CPPUNIT_ASSERT(!pPage2->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testAlignedVmlShapesFlushAtCellEdges)
{
    // Given a DOCX with a table cell that holds two VML shapes with the default wrap distance
    // of the format: a wrap-through shape aligned to the left edge of the cell text area and a
    // tight-wrapped text frame aligned to its right edge:
    createSwDoc("aligned-objects-in-cell.docx");

    // Then make sure the shape sits flush at the left edge of the cell text area, which
    // starts at 1824:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1823
    // - Actual  : 2003
    // The wrap distance of the shape pushed it 180 twips inside the edge it is aligned to,
    // while the wrap distance of the format only keeps text away from the shape.
    assertXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "left", u"1823");

    // And make sure the text frame sits flush at the right edge of the cell text area, which
    // ends at 6809:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6809
    // - Actual  : 6629
    assertXPath(pXmlDoc, "//anchored/fly[1]/infos/bounds", "right", u"6809");

    // And make sure both objects keep their wrap distance in the model, so a later save
    // writes the distance back unchanged:
    uno::Reference<text::XTextFramesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xFrames->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, u"LeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xFrame, u"RightMargin"_ustr));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPageSupplier->getDrawPage()->getByIndex(0),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xShape, u"LeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xShape, u"RightMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFlyAutoWidthKeptInsideCell)
{
    // A table cell holds a grow-with-content frame that follows the text flow and is aligned
    // to the right edge of the cell text area. The content of the frame is a table that is
    // wider than the cell.
    createSwDoc("aligned-objects-in-cell.docx");
    uno::Reference<text::XTextFramesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xFrames->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(getProperty<bool>(xFrame, u"IsFollowingTextFlow"_ustr));

    // The frame stays inside the cell text area, which runs from 1824 to 6810, keeps a margin
    // of its own border and padding width (262 twips) on the left, and stays aligned to the
    // right edge of the cell text area.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4724
    // - Actual  : 5248
    // Only the content area of the frame was limited to the cell text area, so the frame
    // overshot the cell by its own border and padding width and was clipped and moved off
    // the edge it is aligned to.
    assertXPath(pXmlDoc, "(//fly[infos])[1]/infos/bounds", "width", u"4724");
    assertXPath(pXmlDoc, "(//fly[infos])[1]/infos/bounds", "right", u"6809");
}

CPPUNIT_TEST_FIXTURE(Test, testRightAlignedShapeStaysInParagraphArea)
{
    // A paragraph holds two shapes that are aligned to the paragraph area: one at the left edge
    // with the text wrapped on its right, and a wrap-through one at the right edge. The document
    // adds the fly offsets of the format, so the left shape pushes the text start of the
    // paragraph to the right.
    createSwDoc("right-aligned-shape-beside-fly-paragraph-area.fodt");
    uno::Reference<beans::XPropertySet> xShape(getShapeByName(u"Right shape"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));

    // The right shape ends at the right edge of the paragraph.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nParagraphRight
        = getXPath(pXmlDoc, "//txt[anchored/SwAnchoredDrawObject]/infos/bounds", "right")
              .toInt32();
    sal_Int32 nShapeRight
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject[SdrObject/@name='Right shape']/bounds",
                   "right")
              .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11055
    // - Actual  : 12756
    // The text start pushed by the left shape was added to the alignment area, but the area
    // kept its full width, so the shape landed one left shape width past the paragraph edge,
    // outside the page.
    CPPUNIT_ASSERT_EQUAL(nParagraphRight, nShapeRight);
}

CPPUNIT_TEST_FIXTURE(Test, testRightAlignedShapeBesideFly)
{
    // A table cell holds an empty paragraph with two objects anchored to it: a tight-wrapped
    // text frame aligned to the left edge of the cell text area, and a wrap-through shape
    // aligned to the right edge of it. The text frame pushes the text start of the paragraph
    // to the right.
    createSwDoc("right-aligned-shape-beside-fly.docx");

    // The shape is 1002 twips wide and ends at the right edge of the cell text area, which is
    // also the right edge of the anchor paragraph.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nParagraphRight
        = getXPath(pXmlDoc, "//txt[anchored/SwAnchoredDrawObject[@page-frame]]/infos/bounds",
                   "right")
              .toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "//SwAnchoredDrawObject[@page-frame]/bounds", "left").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10155
    // - Actual  : 15060
    // The text start pushed by the text frame was added to the alignment area, but the area
    // kept its full width, so the shape landed one text frame width too far right, outside
    // the page.
    CPPUNIT_ASSERT_EQUAL(nParagraphRight - 1001, nShapeLeft);

    // And the shape keeps its alignment. Nothing turned it into an absolute position.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    bool bFound = false;
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<lang::XServiceInfo> xInfo(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        if (!xInfo->supportsService(u"com.sun.star.drawing.CustomShape"_ustr))
            continue;
        uno::Reference<beans::XPropertySet> xShape(xInfo, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                             getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));
        bFound = true;
    }
    CPPUNIT_ASSERT(bFound);
}

CPPUNIT_TEST_FIXTURE(Test, testFlyInSplitRowMovesWithRow)
{
    // A table row starts near the page bottom. Its cell holds an empty paragraph, a paragraph
    // that anchors a text frame following the text flow and aligned to the top of the cell, and
    // a nested table below the frame. The frame is taller than the space left on the page.
    createSwDoc("fly-in-split-row.docx");
    uno::Reference<text::XTextFramesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xFrames->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(getProperty<bool>(xFrame, u"IsFollowingTextFlow"_ustr));

    // The row moves to the next page as a whole and the frame keeps its full height there.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // The row was split, the anchor paragraph stayed in the part on the first page and the
    // frame was cut at the page bottom, showing 4385 of its 6000 twips.
    assertXPath(pXmlDoc, "//page[1]//anchored/fly", 0);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row/cell/txt/anchored/fly/infos/bounds", "height",
                u"6000");
    sal_Int32 nFlyBottom
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/txt/anchored/fly/infos/bounds",
                   "bottom")
              .toInt32();
    sal_Int32 nBodyBottom
        = getXPath(pXmlDoc, "//page[2]/body/infos/bounds", "bottom").toInt32();
    CPPUNIT_ASSERT_LESS(nBodyBottom, nFlyBottom);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
