/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>

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
                              uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<beans::XPropertySet> xShape2(getShape(1), uno::UNO_QUERY);
    xShape2->setPropertyValue(u"AnchorType"_ustr,
                              uno::Any(text::TextContentAnchorType_AT_CHARACTER));

    // Insert a new paragraph at the start.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    // Without the accompanying fix in place, this test would have crashed.
    pWrtShell->SplitNode();
}

CPPUNIT_TEST_FIXTURE(Test, testImgMoveCrash)
{
    createSwDoc("tdf154863-img-move-crash.docx");
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Image26"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"VertOrient"_ustr, uno::Any(static_cast<sal_Int32>(0)));
    xShapeProps->setPropertyValue(u"VertOrientPosition"_ustr,
                                  uno::Any(static_cast<sal_Int32>(3000)));
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
                                  uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProps->setPropertyValue(u"VertOrient"_ustr, uno::Any(text::VertOrientation::NONE));
    xShapeProps->setPropertyValue(u"VertOrientRelation"_ustr,
                                  uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    xShapeProps->setPropertyValue(u"VertOrientPosition"_ustr,
                                  uno::Any(static_cast<sal_Int32>(-11000)));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // Verify that the distance between the body and anchored object bottom is indeed around 1cm.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom = getXPath(pXmlDoc, "//body/infos/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 nAnchoredBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "bottom"_ostr).toInt32();
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
                                        uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsBottom->setPropertyValue(u"VertOrient"_ustr,
                                        uno::Any(text::VertOrientation::BOTTOM));
    xShapePropsBottom->setPropertyValue(u"VertOrientRelation"_ustr,
                                        uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierBottom(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierBottom->getDrawPage()->add(xShapeBottom);

    //Create second shape and align center of page print area bottom.
    uno::Reference<drawing::XShape> xShapeCenter(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeCenter->setSize(awt::Size(500, 500));
    uno::Reference<beans::XPropertySet> xShapePropsCenter(xShapeCenter, uno::UNO_QUERY);
    xShapePropsCenter->setPropertyValue(u"AnchorType"_ustr,
                                        uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsCenter->setPropertyValue(u"VertOrient"_ustr,
                                        uno::Any(text::VertOrientation::CENTER));
    xShapePropsCenter->setPropertyValue(u"VertOrientRelation"_ustr,
                                        uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierCenter(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierCenter->getDrawPage()->add(xShapeCenter);

    //Create third shape and align top of page print area bottom.
    uno::Reference<drawing::XShape> xShapeTop(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeTop->setSize(awt::Size(500, 500));
    uno::Reference<beans::XPropertySet> xShapePropsTop(xShapeTop, uno::UNO_QUERY);
    xShapePropsTop->setPropertyValue(u"AnchorType"_ustr,
                                     uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsTop->setPropertyValue(u"VertOrient"_ustr, uno::Any(text::VertOrientation::TOP));
    xShapePropsTop->setPropertyValue(u"VertOrientRelation"_ustr,
                                     uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierTop(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierTop->getDrawPage()->add(xShapeTop);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom
        = getXPath(pXmlDoc, "//body/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //14989
    sal_Int32 nPageBottom
        = getXPath(pXmlDoc, "//page/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //16123
    sal_Int32 nFirstShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //16124
    sal_Int32 nSecondShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //15699
    sal_Int32 nSecondShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds"_ostr, "top"_ostr)
              .toInt32(); //15414
    sal_Int32 nThirdShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds"_ostr, "top"_ostr)
              .toInt32(); //14989

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
                                        uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsBottom->setPropertyValue(u"VertOrient"_ustr,
                                        uno::Any(text::VertOrientation::BOTTOM));
    xShapePropsBottom->setPropertyValue(u"VertOrientRelation"_ustr,
                                        uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierBottom(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierBottom->getDrawPage()->add(xShapeBottom);

    // Create second shape and align center of page print area bottom.
    uno::Reference<drawing::XShape> xShapeCenter(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeCenter->setSize(awt::Size(1000, 1000));
    uno::Reference<beans::XPropertySet> xShapePropsCenter(xShapeCenter, uno::UNO_QUERY);
    xShapePropsCenter->setPropertyValue(u"AnchorType"_ustr,
                                        uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsCenter->setPropertyValue(u"VertOrient"_ustr,
                                        uno::Any(text::VertOrientation::CENTER));
    xShapePropsCenter->setPropertyValue(u"VertOrientRelation"_ustr,
                                        uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierCenter(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierCenter->getDrawPage()->add(xShapeCenter);

    // Create third shape and align top of page print area bottom.
    uno::Reference<drawing::XShape> xShapeTop(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShapeTop->setSize(awt::Size(1000, 1000));
    uno::Reference<beans::XPropertySet> xShapePropsTop(xShapeTop, uno::UNO_QUERY);
    xShapePropsTop->setPropertyValue(u"AnchorType"_ustr,
                                     uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapePropsTop->setPropertyValue(u"VertOrient"_ustr, uno::Any(text::VertOrientation::TOP));
    xShapePropsTop->setPropertyValue(u"VertOrientRelation"_ustr,
                                     uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplierTop(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplierTop->getDrawPage()->add(xShapeTop);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nBodyBottom
        = getXPath(pXmlDoc, "//body/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //14853
    sal_Int32 nPageBottom
        = getXPath(pXmlDoc, "//page/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //17121
    sal_Int32 nFirstShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //17122
    sal_Int32 nSecondShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds"_ostr, "top"_ostr)
              .toInt32(); //15703
    sal_Int32 nSecondShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //16272
    sal_Int32 nThirdShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds"_ostr, "top"_ostr)
              .toInt32(); //14853

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
    sal_Int32 nBodyBottom
        = getXPath(pXmlDoc, "//body/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //15704
    sal_Int32 nPageBottom
        = getXPath(pXmlDoc, "//page/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //17121
    sal_Int32 nFirstShapeOutside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //17098
    sal_Int32 nSecondShapeInside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds"_ostr, "top"_ostr)
              .toInt32(); //15694

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
    sal_Int32 nBodyBottom
        = getXPath(pXmlDoc, "//body/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //11803
    sal_Int32 nPageBottom
        = getXPath(pXmlDoc, "//page/infos/bounds"_ostr, "bottom"_ostr).toInt32(); //16123

    sal_Int32 nFirstVMLShapeInside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[1]/bounds"_ostr, "top"_ostr)
              .toInt32(); //11802
    sal_Int32 nSecondVMLShapeBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[2]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //16124
    sal_Int32 nThirdVMLShapeCenterBottom
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds"_ostr, "bottom"_ostr)
              .toInt32(); //14185
    sal_Int32 nThirdVMLShapeCenterTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[3]/bounds"_ostr, "top"_ostr)
              .toInt32(); //13741
    sal_Int32 nFourthVMLShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[4]/bounds"_ostr, "top"_ostr)
              .toInt32(); //11802
    sal_Int32 nFifthVMLShapeOutside
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject[5]/bounds"_ostr, "bottom"_ostr)
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
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
