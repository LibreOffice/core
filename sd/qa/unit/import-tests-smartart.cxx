/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

using namespace ::com::sun::star;

namespace
{
/// Gets one child of xShape, which one is specified by nIndex.
uno::Reference<drawing::XShape> getChildShape(const uno::Reference<drawing::XShape>& xShape,
                                              sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    CPPUNIT_ASSERT(xGroup->getCount() > nIndex);

    uno::Reference<drawing::XShape> xRet(xGroup->getByIndex(nIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRet.is());

    return xRet;
}

uno::Reference<drawing::XShape> findChildShapeByText(const uno::Reference<drawing::XShape>& xShape,
                                                     const OUString& sText)
{
    uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY);
    if (xText.is() && xText->getString() == sText)
        return xShape;

    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    if (!xGroup.is())
        return uno::Reference<drawing::XShape>();

    for (sal_Int32 i = 0; i < xGroup->getCount(); i++)
    {
        uno::Reference<drawing::XShape> xChildShape(xGroup->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xReturnShape = findChildShapeByText(xChildShape, sText);
        if (xReturnShape.is())
            return xReturnShape;
    }

    return uno::Reference<drawing::XShape>();
}
}

class SdImportTestSmartArt : public SdModelTestBase
{
public:
    SdImportTestSmartArt()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBase)
{
    createSdImpressDoc("pptx/smartart1.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());
    uno::Reference<text::XText> xText3(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), xText3->getString());
    uno::Reference<text::XText> xText4(xShapeGroup->getByIndex(5), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("e"), xText4->getString());

    uno::Reference<beans::XPropertySet> xShape(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);

    Color nFillColor;
    xShape->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4F81BD), nFillColor);

    sal_Int16 nParaAdjust = 0;
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape3(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape4(xShapeGroup->getByIndex(5), uno::UNO_QUERY_THROW);

    /*
     *  Arrangement
     *     (LTR)
     *  ╭─────────╮
     *  │  0   1  │
     *  │  2   3  │
     *  │    4    │
     *  ╰─────────╯
     */
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y, xShape3->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xShape1->getPosition().X, xShape3->getPosition().X);

    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y - xShape0->getPosition().Y,
                         xShape4->getPosition().Y - xShape2->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape1->getPosition().X - xShape0->getPosition().X,
                         xShape3->getPosition().X - xShape2->getPosition().X);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(xShape2->getPosition().X + xShape3->getPosition().X,
                                 2 * xShape4->getPosition().X, 1);

    CPPUNIT_ASSERT(xShape2->getPosition().Y > xShape0->getPosition().Y);
    CPPUNIT_ASSERT(xShape4->getPosition().Y > xShape2->getPosition().Y);
    CPPUNIT_ASSERT(xShape0->getPosition().X < xShape1->getPosition().X);
    CPPUNIT_ASSERT(xShape2->getPosition().X < xShape3->getPosition().X);
    CPPUNIT_ASSERT((xShape2->getPosition().X < xShape4->getPosition().X));
    CPPUNIT_ASSERT((xShape3->getPosition().X > xShape4->getPosition().X));
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testChildren)
{
    createSdImpressDoc("pptx/smartart-children.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xShapeGroup0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup0->getCount());
    uno::Reference<text::XText> xTextA(xShapeGroup0->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xTextA->getString());

    uno::Reference<drawing::XShapes> xChildren0(xShapeGroup0->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xChildren0->getCount());
    uno::Reference<drawing::XShapes> xChildB(xChildren0->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xTextB(xChildB->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xTextB->getString());
    uno::Reference<drawing::XShapes> xChildC(xChildren0->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xTextC(xChildC->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xTextC->getString());

    uno::Reference<drawing::XShapes> xShapeGroup1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup1->getCount());
    uno::Reference<text::XText> xTextX(xShapeGroup1->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), xTextX->getString());

    uno::Reference<drawing::XShapes> xChildren1(xShapeGroup1->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xChildren1->getCount());
    uno::Reference<drawing::XShapes> xChildY(xChildren1->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xTextY(xChildY->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("y"), xTextY->getString());
    uno::Reference<drawing::XShapes> xChildZ(xChildren1->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xTextZ(xChildZ->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("z"), xTextZ->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testText)
{
    createSdImpressDoc("pptx/smartart-text.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xShapeGroup2(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);

    uno::Reference<text::XText> xText0(xShapeGroup2->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xText0->getString().isEmpty());

    uno::Reference<text::XText> xText1(xShapeGroup2->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xText1->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testCnt)
{
    createSdImpressDoc("pptx/smartart-cnt.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    sal_Int32 nCount = xShapeGroup->getCount();
    sal_Int32 nCorrect = 0;
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        uno::Reference<text::XText> xText(xShapeGroup->getByIndex(i), uno::UNO_QUERY);
        if (xText.is() && !xText->getString().isEmpty())
            nCorrect++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nCorrect);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testDir)
{
    createSdImpressDoc("pptx/smartart-dir.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xShapeGroup->getCount());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape0->getPosition().X > xShape1->getPosition().X);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf148665)
{
    // Without the fix in place, this test would have crashed at import time
    createSdImpressDoc("pptx/tdf148665.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("\nFufufu"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Susu"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Sasa Haha"), xText2->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf148921)
{
    createSdImpressDoc("pptx/tdf148921.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xShapeGroup2(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 4
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup2->getCount());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup2->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup2->getByIndex(1), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xShape0->getPosition().X < xShape1->getPosition().X);
    CPPUNIT_ASSERT(xShape0->getPosition().Y < xShape1->getPosition().Y);
    CPPUNIT_ASSERT(xShape0->getSize().Height > xShape1->getSize().Height);
    CPPUNIT_ASSERT(xShape0->getSize().Width > xShape1->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testMaxDepth)
{
    createSdImpressDoc("pptx/smartart-maxdepth.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("second"), xText1->getString());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y,
                         xShape1->getPosition().Y); // Confirms shapes are in same Y axis-level.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testRotation)
{
    createSdImpressDoc("pptx/smartart-rotation.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xShape0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape0->getPropertyValue("RotateAngle").get<sal_Int32>());

    uno::Reference<beans::XPropertySet> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24000),
                         xShape1->getPropertyValue("RotateAngle").get<sal_Int32>());

    uno::Reference<beans::XPropertySet> xShape2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12000),
                         xShape2->getPropertyValue("RotateAngle").get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTextAutoRotation)
{
    createSdImpressDoc("pptx/smartart-autoTxRot.pptx");

    auto testText = [&](int pageNo, sal_Int32 txtNo, const OUString& expTx, sal_Int32 expShRot,
                        sal_Int32 expTxRot) {
        OString msgText = "Page: " + OString::number(pageNo) + " text: " + OString::number(txtNo);
        uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, pageNo),
                                                     uno::UNO_QUERY_THROW);

        txtNo++; //skip background
        uno::Reference<text::XText> xTxt(xShapeGroup->getByIndex(txtNo), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msgText.getStr(), expTx, xTxt->getString());
        uno::Reference<beans::XPropertySet> xTxtProps(xTxt, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msgText.getStr(), expShRot,
                                     xTxtProps->getPropertyValue("RotateAngle").get<sal_Int32>());

        auto aGeomPropSeq = xTxtProps->getPropertyValue("CustomShapeGeometry")
                                .get<uno::Sequence<beans::PropertyValue>>();
        comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);

        auto it = aCustomShapeGeometry.find("TextPreRotateAngle");
        if (it == aCustomShapeGeometry.end())
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msgText.getStr(), sal_Int32(0), expTxRot);
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msgText.getStr(), expTxRot, it->second.get<sal_Int32>());
        }
    };

    // Slide 1: absent autoTxRot => defaults to "upr"
    testText(0, 0, "a", 0, 0);
    testText(0, 1, "b", 33750, 0);
    testText(0, 2, "c", 31500, 0);
    testText(0, 3, "d", 29250, 90);
    testText(0, 4, "e", 27000, 90);
    testText(0, 5, "f", 24750, 90);
    testText(0, 6, "g", 22500, 180);
    testText(0, 7, "h", 20250, 180);
    testText(0, 8, "i", 18000, 180);
    testText(0, 9, "j", 15750, 180);
    testText(0, 10, "k", 13500, 180);
    testText(0, 11, "l", 11250, 270);
    testText(0, 12, "m", 9000, 270);
    testText(0, 13, "n", 6750, 270);
    testText(0, 14, "o", 4500, 0);
    testText(0, 15, "p", 2250, 0);

    // Slide 2: autoTxRot == "none"
    testText(1, 0, "a", 0, 0);
    testText(1, 1, "b", 33750, 0);
    testText(1, 2, "c", 31500, 0);
    testText(1, 3, "d", 29250, 0);
    testText(1, 4, "e", 27000, 0);
    testText(1, 5, "f", 24750, 0);
    testText(1, 6, "g", 22500, 0);
    testText(1, 7, "h", 20250, 0);
    testText(1, 8, "i", 18000, 0);
    testText(1, 9, "j", 15750, 0);
    testText(1, 10, "k", 13500, 0);
    testText(1, 11, "l", 11250, 0);
    testText(1, 12, "m", 9000, 0);
    testText(1, 13, "n", 6750, 0);
    testText(1, 14, "o", 4500, 0);
    testText(1, 15, "p", 2250, 0);

    // Slide 3: autoTxRot == "grav"
    testText(2, 0, "a", 0, 0);
    testText(2, 1, "b", 33750, 0);
    testText(2, 2, "c", 31500, 0);
    testText(2, 3, "d", 29250, 0);
    testText(2, 4, "e", 27000, 0);
    testText(2, 5, "f", 24750, 180);
    testText(2, 6, "g", 22500, 180);
    testText(2, 7, "h", 20250, 180);
    testText(2, 8, "i", 18000, 180);
    testText(2, 9, "j", 15750, 180);
    testText(2, 10, "k", 13500, 180);
    testText(2, 11, "l", 11250, 180);
    testText(2, 12, "m", 9000, 0);
    testText(2, 13, "n", 6750, 0);
    testText(2, 14, "o", 4500, 0);
    testText(2, 15, "p", 2250, 0);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBasicProcess)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testPyramid)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testPyramidOneChild)
{
    // Load a document with a pyra algorithm in it.
    // Without the accompanying fix in place, this test would have crashed.
    createSdImpressDoc("pptx/smartart-pyramid-1child.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(getChildShape(getChildShape(xGroup, 1), 1),
                                           uno::UNO_QUERY);
    // Verify that the text of the only child is imported correctly.
    CPPUNIT_ASSERT_EQUAL(OUString("A"), xText->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testChevron)
{
    createSdImpressDoc("pptx/smartart-chevron.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xShape0->getPosition().X < xShape1->getPosition().X);
    CPPUNIT_ASSERT(xShape1->getPosition().X < xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape1->getPosition().Y, xShape2->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testCycle)
{
    createSdImpressDoc("pptx/smartart-cycle.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    // 11 children: background, 5 shapes, 5 connectors
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11), xGroup->getCount());

    uno::Reference<drawing::XShape> xShape0(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShapeConn(xGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xGroup->getByIndex(3), uno::UNO_QUERY_THROW);

    uno::Reference<text::XText> xText0(xShape0, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText2(xShape2, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText2->getString());

    // xShapeConn is connector between shapes 0 and 2
    // it should lay between them and be rotated 0 -> 2
    CPPUNIT_ASSERT(xShape0->getPosition().X < xShapeConn->getPosition().X);
    CPPUNIT_ASSERT(xShape0->getPosition().Y < xShapeConn->getPosition().Y);
    CPPUNIT_ASSERT(xShapeConn->getPosition().Y < xShape2->getPosition().Y);
    uno::Reference<beans::XPropertySet> xPropSetConn(xShapeConn, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(32400),
                         xPropSetConn->getPropertyValue("RotateAngle").get<sal_Int32>());

    // Make sure that we have an arrow shape between the two shapes
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        xPropSetConn->getPropertyValue("CustomShapeGeometry"));
    CPPUNIT_ASSERT(aCustomShapeGeometry["Type"].has<OUString>());
    OUString aType = aCustomShapeGeometry["Type"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-rightArrow"), aType);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testHierarchy)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testmatrix)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testvenndiagram)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testInvertedPyramid)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testMultidirectional)
{
    // similar document as cycle, but arrows are pointing in both directions
    createSdImpressDoc("pptx/smartart-multidirectional.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    // 7 children: background, 3 shapes, 3 connectors
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7), xGroup->getCount());

    uno::Reference<drawing::XShape> xShapeConn(xGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSetConn(xShapeConn, uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        xPropSetConn->getPropertyValue("CustomShapeGeometry"));
    CPPUNIT_ASSERT(aCustomShapeGeometry["Type"].has<OUString>());
    OUString aType = aCustomShapeGeometry["Type"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-leftRightArrow"), aType);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testHorizontalBulletList)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testEquation)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBasicRadicals)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testSegmentedCycle)
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBaseRtoL)
{
    createSdImpressDoc("pptx/smartart-rightoleftblockdiagram.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());
    uno::Reference<text::XText> xText3(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), xText3->getString());
    uno::Reference<text::XText> xText4(xShapeGroup->getByIndex(5), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("e"), xText4->getString());

    uno::Reference<beans::XPropertySet> xShape(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);

    Color nFillColor;
    xShape->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4F81BD), nFillColor);

    sal_Int16 nParaAdjust = 0;
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape3(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape4(xShapeGroup->getByIndex(5), uno::UNO_QUERY_THROW);

    /*
     *  Arrangement
     *     (RTL)
     *  ╭─────────╮
     *  │  1   0  │
     *  │  3   2  │
     *  │    4    │
     *  ╰─────────╯
     */
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y, xShape3->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xShape1->getPosition().X, xShape3->getPosition().X);

    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y - xShape0->getPosition().Y,
                         xShape4->getPosition().Y - xShape2->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape1->getPosition().X - xShape0->getPosition().X,
                         xShape3->getPosition().X - xShape2->getPosition().X);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(xShape2->getPosition().X + xShape3->getPosition().X,
                                 2 * xShape4->getPosition().X, 1);

    CPPUNIT_ASSERT(xShape2->getPosition().Y > xShape0->getPosition().Y);
    CPPUNIT_ASSERT(xShape4->getPosition().Y > xShape2->getPosition().Y);
    CPPUNIT_ASSERT(xShape0->getPosition().X > xShape1->getPosition().X);
    CPPUNIT_ASSERT(xShape2->getPosition().X > xShape3->getPosition().X);
    CPPUNIT_ASSERT((xShape2->getPosition().X > xShape4->getPosition().X));
    CPPUNIT_ASSERT((xShape3->getPosition().X < xShape4->getPosition().X));
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testVerticalBoxList)
{
    createSdImpressDoc("pptx/smartart-vertical-box-list.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 0'.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xFirstChild(xShapeGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstChild.is());
    uno::Reference<drawing::XShape> xParentText(xFirstChild->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParentText.is());
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 7361', i.e. the width was not the 70% of the parent as the
    // constraint wanted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11852), xParentText->getSize().Width);

    uno::Reference<drawing::XShape> xChildText(xShapeGroup->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChildText.is());
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 7361' (and with the fix: 'actual: 16932', i.e. the width of the
    // parent).
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(10000), xChildText->getSize().Width);

    // Assert that the right edge of the parent text is closer to the slide
    // boundary than the right edge of the parent text.
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected greater than: 25656, Actual  : 21165'.
    CPPUNIT_ASSERT_GREATER(xParentText->getPosition().X + xParentText->getSize().Width,
                           xChildText->getPosition().X + xChildText->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testVerticalBracketList)
{
    createSdImpressDoc("pptx/vertical-bracket-list.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xFirstChild(xShapeGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstChild.is());
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 2', i.e. one child shape (with its "A" text) was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xFirstChild->getCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTableList)
{
    createSdImpressDoc("pptx/table-list.pptx");
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeGroup.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xShapeGroup->getCount());

    uno::Reference<text::XText> xParentText(xShapeGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParentText.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Parent"), xParentText->getString());
    uno::Reference<drawing::XShape> xParent(xParentText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParent.is());
    int nParentRight = xParent->getPosition().X + xParent->getSize().Width;

    uno::Reference<drawing::XShapes> xChildren(xShapeGroup->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChildren.is());
    uno::Reference<text::XText> xChild2Text(xChildren->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChild2Text.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Child 2"), xChild2Text->getString());
    uno::Reference<drawing::XShape> xChild2(xChild2Text, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChild2.is());
    int nChild2Right = xChild2->getPosition().X + xChild2->getSize().Width;

    // Without the accompanying fix in place, this test would have failed with
    // 'Expected less than: 100, Actual  : 22014', i.e. the second child was
    // shifted to the right too much.
    CPPUNIT_ASSERT_LESS(100, abs(nChild2Right - nParentRight));
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testAccentProcess)
{
    createSdImpressDoc("pptx/smartart-accent-process.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    // 3 children: first pair, connector, second pair.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xGroup->getCount());
    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroupShape.is());

    // The pair is a parent (shape + text) and a child, so 3 shapes in total.
    // The order is important, first is at the back, last is at the front.
    uno::Reference<drawing::XShapes> xFirstPair(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstPair.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xFirstPair->getCount());

    uno::Reference<text::XText> xFirstParentText(xFirstPair->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstParentText.is());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xFirstParentText->getString());
    uno::Reference<drawing::XShape> xFirstParent(xFirstParentText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstParent.is());
    int nFirstParentTop = xFirstParent->getPosition().Y;

    uno::Reference<text::XText> xFirstChildText(xFirstPair->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstChildText.is());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xFirstChildText->getString());
    uno::Reference<drawing::XShape> xFirstChild(xFirstChildText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstChildText.is());

    {
        uno::Reference<container::XEnumerationAccess> xParasAccess(xFirstChildText, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParas = xParasAccess->createEnumeration();
        uno::Reference<beans::XPropertySet> xPara(xParas->nextElement(), uno::UNO_QUERY);
        // Without the accompanying fix in place, this test would have failed
        // with 'Expected: 0; Actual  : 1270', i.e. there was a large
        // unexpected left margin.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             xPara->getPropertyValue("ParaLeftMargin").get<sal_Int32>());

        uno::Reference<container::XIndexAccess> xRules(xPara->getPropertyValue("NumberingRules"),
                                                       uno::UNO_QUERY);
        comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
        CPPUNIT_ASSERT_EQUAL(u"•"_ustr, aRule["BulletChar"].get<OUString>());
    }

    int nFirstChildTop = xFirstChild->getPosition().Y;
    int nFirstChildRight = xFirstChild->getPosition().X + xFirstChild->getSize().Width;

    // First child is below the first parent.
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected less than: 3881, Actual  : 3881', i.e. xFirstChild was not
    // below xFirstParent (a good position is 9081).
    CPPUNIT_ASSERT_LESS(nFirstChildTop, nFirstParentTop);

    // Make sure that we have an arrow shape between the two pairs.
    uno::Reference<beans::XPropertySet> xArrow(xGroup->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xArrow.is());
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        xArrow->getPropertyValue("CustomShapeGeometry"));
    // Without the accompanying fix in place, this test would have failed, i.e.
    // the custom shape lacked a type -> arrow was not visible.
    CPPUNIT_ASSERT(aCustomShapeGeometry["Type"].has<OUString>());
    OUString aType = aCustomShapeGeometry["Type"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-rightArrow"), aType);

    // Make sure that height of the arrow is less than its width.
    uno::Reference<drawing::XShape> xArrowShape(xArrow, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xArrowShape.is());
    awt::Size aArrowSize = xArrowShape->getSize();
    CPPUNIT_ASSERT_LESS(aArrowSize.Width, aArrowSize.Height);

    uno::Reference<drawing::XShapes> xSecondPair(xGroup->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSecondPair.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xSecondPair->getCount());
    uno::Reference<text::XText> xSecondParentText(xSecondPair->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstParentText.is());
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: cc; Actual  : c', i.e. non-first runs on data points were ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("cc"), xSecondParentText->getString());
    uno::Reference<drawing::XShape> xSecondParent(xSecondParentText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSecondParent.is());
    int nSecondParentLeft = xSecondParent->getPosition().X;
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected less than: 12700; Actual  : 18540', i.e. the "b" and "c"
    // shapes overlapped.
    CPPUNIT_ASSERT_LESS(nSecondParentLeft, nFirstChildRight);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testContinuousBlockProcess)
{
    createSdImpressDoc("pptx/smartart-continuous-block-process.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    // 3 children: diagram background, background arrow, foreground.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup->getCount());

    uno::Reference<drawing::XShapes> xLinear(xGroup->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xLinear.is());
    // 3 children: A, B and C.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xLinear->getCount());

    uno::Reference<text::XText> xA(xLinear->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xA.is());
    CPPUNIT_ASSERT_EQUAL(OUString("A"), xA->getString());
    uno::Reference<drawing::XShape> xAShape(xA, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAShape.is());
    // Without the accompanying fix in place, this test would have failed: the
    // theoretically correct value is 5462 mm100 (16933 is the total width, and
    // need to divide that to 1, 0.5, 1, 0.5 and 1 units), while the old value
    // was 4703 and the new one is 5461.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(5000), xAShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testOrgChart)
{
    // Simple org chart with 1 manager and 1 employee only.
    createSdImpressDoc("pptx/smartart-org-chart.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<text::XText> xManager(
        getChildShape(getChildShape(getChildShape(xGroup, 1), 0), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xManager.is());
    // Without the accompanying fix in place, this test would have failed: this
    // was just "Manager", and the second paragraph was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("Manager\nSecond para"), xManager->getString());

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xManager, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xPara, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    Color nActualColor;
    xRun->getPropertyValue("CharColor") >>= nActualColor;
    // Without the accompanying fix in place, this test would have failed: the
    // "Manager" font color was black, not white.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nActualColor);

    uno::Reference<drawing::XShape> xManagerShape(xManager, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xManagerShape.is());

    awt::Point aManagerPos = xManagerShape->getPosition();
    awt::Size aManagerSize = xManagerShape->getSize();

    // Make sure that the manager has 2 employees.
    uno::Reference<drawing::XShapes> xEmployees(getChildShape(getChildShape(xGroup, 1), 2),
                                                uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployees.is());
    // 4 children: connector, 1st employee, connector, 2nd employee.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xEmployees->getCount());

    uno::Reference<text::XText> xEmployee(
        getChildShape(
            getChildShape(getChildShape(getChildShape(getChildShape(xGroup, 1), 2), 1), 0), 0),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployee.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Employee"), xEmployee->getString());

    uno::Reference<drawing::XShape> xEmployeeShape(xEmployee, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployeeShape.is());

    awt::Point aEmployeePos = xEmployeeShape->getPosition();
    awt::Size aEmployeeSize = xEmployeeShape->getSize();

    CPPUNIT_ASSERT_EQUAL(aManagerPos.X, aEmployeePos.X);

    // Without the accompanying fix in place, this test would have failed: the
    // two shapes were overlapping, i.e. "manager" was not above "employee".
    CPPUNIT_ASSERT_GREATER(aManagerPos.Y, aEmployeePos.Y);

    // Make sure that the second employee is on the right of the first one.
    // Without the accompanying fix in place, this test would have failed, as
    // the second employee was below the first one.
    uno::Reference<text::XText> xEmployee2(
        getChildShape(
            getChildShape(getChildShape(getChildShape(getChildShape(xGroup, 1), 2), 3), 0), 0),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployee2.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Employee2"), xEmployee2->getString());

    uno::Reference<drawing::XShape> xEmployee2Shape(xEmployee2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployee2Shape.is());

    awt::Point aEmployee2Pos = xEmployee2Shape->getPosition();
    //awt::Size aEmployee2Size = xEmployee2Shape->getSize();
    CPPUNIT_ASSERT_GREATER(aEmployeePos.X, aEmployee2Pos.X);

    // Make sure that assistant is above employees.
    uno::Reference<text::XText> xAssistant(
        getChildShape(
            getChildShape(getChildShape(getChildShape(getChildShape(xGroup, 1), 1), 1), 0), 0),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Assistant"), xAssistant->getString());

    uno::Reference<drawing::XShape> xAssistantShape(xAssistant, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAssistantShape.is());

    awt::Point aAssistantPos = xAssistantShape->getPosition();
    // Without the accompanying fix in place, this test would have failed: the
    // assistant shape was below the employee shape.
    CPPUNIT_ASSERT_GREATER(aAssistantPos.Y, aEmployeePos.Y);

    // Make sure the connector of the assistant is above the shape.
    uno::Reference<drawing::XShape> xAssistantConnector
        = getChildShape(getChildShape(getChildShape(xGroup, 1), 1), 0);
    CPPUNIT_ASSERT(xAssistantConnector.is());
    //awt::Point aAssistantConnectorPos = xAssistantConnector->getPosition();
    // This failed, the vertical positions of the connector and the shape of
    // the assistant were the same.
    //CPPUNIT_ASSERT_LESS(aAssistantPos.Y, aAssistantConnectorPos.Y);
    // connectors are hidden as they don't work correctly

    // Make sure the height of xManager and xManager2 is the same.
    uno::Reference<text::XText> xManager2(
        getChildShape(getChildShape(getChildShape(xGroup, 2), 0), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xManager2.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Manager2"), xManager2->getString());

    uno::Reference<drawing::XShape> xManager2Shape(xManager2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xManager2Shape.is());

    awt::Size aManager2Size = xManager2Shape->getSize();
    // Without the accompanying fix in place, this test would have failed:
    // xManager2's height was 3 times larger than xManager's height.
    CPPUNIT_ASSERT_EQUAL(aManagerSize.Height, aManager2Size.Height);

    // Make sure the employee nodes use the free space on the right, since
    // manager2 has no assistants / employees.
    //CPPUNIT_ASSERT_GREATER(aManagerSize.Width, aEmployeeSize.Width + aEmployee2Size.Width);
    // currently disabled as causes problems in complex charts

    // Without the accompanying fix in place, this test would have failed: an
    // employee was exactly the third of the total height, without any spacing.
    CPPUNIT_ASSERT_LESS(xGroup->getSize().Height / 3, aEmployeeSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testCycleMatrix)
{
    createSdImpressDoc("pptx/smartart-cycle-matrix.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    // Without the accompanying fix in place, this test would have failed: the height was 12162,
    // which is not the mm100 equivalent of the 4064000 EMU in the input file.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11288), xGroup->getSize().Height);

    uno::Reference<text::XText> xA1(getChildShape(getChildShape(xGroup, 2), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xA1.is());
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), xA1->getString());

    // Test fill color of B1, should be orange.
    uno::Reference<text::XText> xB1(getChildShape(getChildShape(xGroup, 2), 1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xB1.is());
    CPPUNIT_ASSERT_EQUAL(OUString("B1"), xB1->getString());

    uno::Reference<beans::XPropertySet> xB1Props(xB1, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xB1Props.is());
    Color nFillColor;
    xB1Props->getPropertyValue("FillColor") >>= nFillColor;
    // Without the accompanying fix in place, this test would have failed: the background color was
    // 0x4f81bd, i.e. blue, not orange.
    CPPUNIT_ASSERT_EQUAL(Color(0xf79646), nFillColor);

    // Without the accompanying fix in place, this test would have failed: the
    // content of the "A2" shape was lost.
    uno::Reference<text::XText> xA2(getChildShape(getChildShape(getChildShape(xGroup, 1), 0), 1),
                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT(xA2.is());
    CPPUNIT_ASSERT_EQUAL(OUString("A2"), xA2->getString());

    // Test that the layout of shapes is like this:
    // A2 B2
    // D2 C2

    uno::Reference<drawing::XShape> xA2Shape(xA2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xA2Shape.is());

    uno::Reference<text::XText> xB2(getChildShape(getChildShape(getChildShape(xGroup, 1), 1), 1),
                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT(xB2.is());
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), xB2->getString());
    uno::Reference<drawing::XShape> xB2Shape(xB2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xB2Shape.is());

    // Test line color of B2, should be orange.
    uno::Reference<beans::XPropertySet> xB2Props(xB2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xB2Props.is());
    Color nLineColor = 0;
    xB2Props->getPropertyValue("LineColor") >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xf79646), nLineColor);

    uno::Reference<text::XText> xC2(getChildShape(getChildShape(getChildShape(xGroup, 1), 2), 1),
                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT(xC2.is());
    // Without the accompanying fix in place, this test would have failed, i.e. the order of the
    // lines in the shape were wrong: C2-1\nC2-4\nC2-3\nC2-2.
    CPPUNIT_ASSERT_EQUAL(OUString("C2-1\nC2-2\nC2-3\nC2-4"), xC2->getString());
    uno::Reference<drawing::XShape> xC2Shape(xC2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xC2Shape.is());

    uno::Reference<text::XText> xD2(getChildShape(getChildShape(getChildShape(xGroup, 1), 3), 1),
                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT(xD2.is());
    CPPUNIT_ASSERT_EQUAL(OUString("D2"), xD2->getString());
    uno::Reference<drawing::XShape> xD2Shape(xD2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xD2Shape.is());

    // Without the accompanying fix in place, this test would have failed, i.e.
    // the A2 and B2 shapes had the same horizontal position, while B2 should
    // be on the right of A2.
    CPPUNIT_ASSERT_GREATER(xA2Shape->getPosition().X, xB2Shape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xA2Shape->getPosition().Y, xB2Shape->getPosition().Y);
    CPPUNIT_ASSERT_GREATER(xA2Shape->getPosition().X, xC2Shape->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xA2Shape->getPosition().Y, xC2Shape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xA2Shape->getPosition().X, xD2Shape->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xA2Shape->getPosition().Y, xD2Shape->getPosition().Y);

    // Without the accompanying fix in place, this test would have failed: width was expected to be
    // 4887, was actually 7331.
    uno::Reference<drawing::XShape> xA1Shape(xA1, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xA1Shape.is());
    CPPUNIT_ASSERT_EQUAL(xA1Shape->getSize().Height, xA1Shape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testPictureStrip)
{
    createSdImpressDoc("pptx/smartart-picture-strip.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<beans::XPropertySet> xFirstImage(getChildShape(getChildShape(xGroup, 1), 1),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstImage.is());
    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
    xFirstImage->getPropertyValue("FillStyle") >>= eFillStyle;
    // Without the accompanying fix in place, this test would have failed: fill style was solid, not
    // bitmap.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, eFillStyle);

    uno::Reference<graphic::XGraphic> xGraphic;
    xFirstImage->getPropertyValue("FillBitmap") >>= xGraphic;
    Graphic aFirstGraphic(xGraphic);

    uno::Reference<beans::XPropertySet> xSecondImage(getChildShape(getChildShape(xGroup, 2), 1),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSecondImage.is());
    eFillStyle = drawing::FillStyle_NONE;
    xSecondImage->getPropertyValue("FillStyle") >>= eFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, eFillStyle);

    xSecondImage->getPropertyValue("FillBitmap") >>= xGraphic;
    Graphic aSecondGraphic(xGraphic);
    // Without the accompanying fix in place, this test would have failed: both xFirstImage and
    // xSecondImage had the bitmap fill from the second shape.
    CPPUNIT_ASSERT(aFirstGraphic.GetChecksum() != aSecondGraphic.GetChecksum());

    // Test that the 3 images are in a single column, in 3 rows.
    uno::Reference<drawing::XShape> xFirstImageShape(xFirstImage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstImage.is());
    uno::Reference<drawing::XShape> xSecondImageShape(xSecondImage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSecondImage.is());
    uno::Reference<drawing::XShape> xThirdImageShape = getChildShape(getChildShape(xGroup, 3), 1);
    CPPUNIT_ASSERT(xThirdImageShape.is());
    // Without the accompanying fix in place, this test would have failed: the first and the second
    // image were in the same row.
    CPPUNIT_ASSERT_EQUAL(xFirstImageShape->getPosition().X, xSecondImageShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xSecondImageShape->getPosition().X, xThirdImageShape->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xFirstImageShape->getPosition().Y, xSecondImageShape->getPosition().Y);
    CPPUNIT_ASSERT_GREATER(xSecondImageShape->getPosition().Y, xThirdImageShape->getPosition().Y);

    // Make sure that the title shape doesn't overlap with the diagram.
    // Note that real "no overlap" is asserted here, though in fact what we want is a less strict
    // condition: that no text part of the title shape and the diagram overlaps.
    uno::Reference<drawing::XShape> xTitle(getShapeFromPage(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTitle.is());
    // Without the accompanying fix in place, this test would have failed with 'Expected greater
    // than: 2873; Actual  : 2320', i.e. the title shape and the diagram overlapped.
    uno::Reference<drawing::XShape> xFirstPair = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT_GREATER(xTitle->getPosition().Y + xTitle->getSize().Height,
                           xFirstPair->getPosition().Y);

    // Make sure that left margin is 60% of width (if you count width in points and margin in mms).
    uno::Reference<beans::XPropertySet> xFirstText(getChildShape(getChildShape(xGroup, 1), 0),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstText.is());
    sal_Int32 nTextLeftDistance = 0;
    xFirstText->getPropertyValue("TextLeftDistance") >>= nTextLeftDistance;
    uno::Reference<drawing::XShape> xFirstTextShape(xFirstText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstTextShape.is());
    sal_Int32 nWidth = xFirstTextShape->getSize().Width;
    double fFactor = oox::drawingml::convertPointToMms(0.6);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 3440,
    // Actual  : 263', i.e. the left margin was too small.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(nWidth * fFactor), nTextLeftDistance);

    // Make sure that aspect ratio is not ignored, i.e. width is not larger than height 3 times.
    awt::Size aFirstPairSize = xFirstPair->getSize();
    // Without the accompanying fix in place, this test would have failed: bad width was 16932, good
    // width is 12540, but let's accept 12541 as well.
    CPPUNIT_ASSERT_LESSEQUAL(aFirstPairSize.Height * 3 + 1, aFirstPairSize.Width);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testInteropGrabBag)
{
    createSdImpressDoc("pptx/smartart-interopgrabbag.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aGrabBagSeq;
    xPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBagSeq;
    comphelper::SequenceAsHashMap aGrabBag(aGrabBagSeq);
    CPPUNIT_ASSERT(aGrabBag.find("OOXData") != aGrabBag.end());
    CPPUNIT_ASSERT(aGrabBag.find("OOXLayout") != aGrabBag.end());
    CPPUNIT_ASSERT(aGrabBag.find("OOXStyle") != aGrabBag.end());
    CPPUNIT_ASSERT(aGrabBag.find("OOXColor") != aGrabBag.end());
    CPPUNIT_ASSERT(aGrabBag.find("OOXDrawing") != aGrabBag.end());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBackground)
{
    createSdImpressDoc("pptx/smartart-background.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    // background should fill whole diagram
    uno::Reference<drawing::XShape> xShapeGroup(xGroup, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xShape->getPosition().X, xShapeGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xShape->getPosition().Y, xShapeGroup->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape->getSize().Width, xShapeGroup->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(xShape->getSize().Height, xShapeGroup->getSize().Height);

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY_THROW);
    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
    xPropertySet->getPropertyValue("FillStyle") >>= eFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, eFillStyle);

    Color nFillColor;
    xPropertySet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x339933), nFillColor);

    bool bMoveProtect = false;
    xPropertySet->getPropertyValue("MoveProtect") >>= bMoveProtect;
    CPPUNIT_ASSERT_EQUAL(true, bMoveProtect);

    bool bSizeProtect = false;
    xPropertySet->getPropertyValue("SizeProtect") >>= bSizeProtect;
    CPPUNIT_ASSERT_EQUAL(true, bSizeProtect);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBackgroundDrawingmlFallback)
{
    // same as testBackground, but test file contains drawingML fallback

    createSdImpressDoc("pptx/smartart-background-drawingml-fallback.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    // background should fill whole diagram
    uno::Reference<drawing::XShape> xShapeGroup(xGroup, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xShape->getPosition().X, xShapeGroup->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xShape->getPosition().Y, xShapeGroup->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape->getSize().Width, xShapeGroup->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(xShape->getSize().Height, xShapeGroup->getSize().Height);

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY_THROW);
    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
    xPropertySet->getPropertyValue("FillStyle") >>= eFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, eFillStyle);

    Color nFillColor;
    xPropertySet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x339933), nFillColor);

    bool bMoveProtect = false;
    xPropertySet->getPropertyValue("MoveProtect") >>= bMoveProtect;
    CPPUNIT_ASSERT_EQUAL(true, bMoveProtect);

    bool bSizeProtect = false;
    xPropertySet->getPropertyValue("SizeProtect") >>= bSizeProtect;
    CPPUNIT_ASSERT_EQUAL(true, bSizeProtect);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testCenterCycle)
{
    createSdImpressDoc("pptx/smartart-center-cycle.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<drawing::XShapes> xGroupNested(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroupNested.is());

    uno::Reference<drawing::XShape> xShapeCenter(xGroupNested->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeA(xGroupNested->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeB(xGroupNested->getByIndex(2), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeC(xGroupNested->getByIndex(3), uno::UNO_QUERY);

    uno::Reference<text::XText> xTextCenter(xShapeCenter, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextCenter.is());
    CPPUNIT_ASSERT_EQUAL(OUString("center"), xTextCenter->getString());

    CPPUNIT_ASSERT_LESS(xShapeCenter->getPosition().Y, xShapeA->getPosition().Y);
    CPPUNIT_ASSERT_GREATER(xShapeCenter->getPosition().X, xShapeB->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xShapeCenter->getPosition().Y, xShapeB->getPosition().Y);
    CPPUNIT_ASSERT_LESS(xShapeCenter->getPosition().X, xShapeC->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xShapeCenter->getPosition().Y, xShapeC->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testFontSize)
{
    createSdImpressDoc("pptx/smartart-font-size.pptx");

    uno::Reference<drawing::XShapes> xGroup1(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape1(xGroup1->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph1(getParagraphFromShape(0, xShape1));
    uno::Reference<text::XTextRange> xRun1(getRunFromParagraph(0, xParagraph1));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun1, uno::UNO_QUERY);
    double fFontSize1 = xPropSet1->getPropertyValue("CharHeight").get<double>();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(65.0, fFontSize1, 0.01);

    uno::Reference<drawing::XShapes> xGroup2(getShapeFromPage(1, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape2(xGroup2->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph2(getParagraphFromShape(0, xShape2));
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY);
    double fFontSize2 = xPropSet2->getPropertyValue("CharHeight").get<double>();
    CPPUNIT_ASSERT_EQUAL(32.0, fFontSize2);

    uno::Reference<drawing::XShapes> xGroup3(getShapeFromPage(2, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape3(xGroup3->getByIndex(1), uno::UNO_QUERY);
    drawing::TextFitToSizeType eTextFitToSize = drawing::TextFitToSizeType_NONE;
    xShape3->getPropertyValue("TextFitToSize") >>= eTextFitToSize;
    CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, eTextFitToSize);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testVerticalBlockList)
{
    createSdImpressDoc("pptx/smartart-vertical-block-list.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<drawing::XShapes> xGroup1(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup1->getCount());
    uno::Reference<drawing::XShape> xShapeA(xGroup1->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeBC(xGroup1->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextA(xShapeA, uno::UNO_QUERY);
    uno::Reference<text::XText> xTextBC(xShapeBC, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xTextA->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("b\nc"), xTextBC->getString());

    uno::Reference<beans::XPropertySet> xPropSetBC(xShapeBC, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000),
                         xPropSetBC->getPropertyValue("RotateAngle").get<sal_Int32>());

    // BC shape is rotated 90*, so width and height is swapped
    CPPUNIT_ASSERT_GREATER(xShapeA->getSize().Width, xShapeBC->getSize().Height);
    CPPUNIT_ASSERT_LESS(xShapeA->getSize().Height, xShapeBC->getSize().Width);
    CPPUNIT_ASSERT_GREATER(xShapeA->getPosition().X, xShapeBC->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xShapeA->getPosition().Y, xShapeBC->getPosition().Y);

    uno::Reference<drawing::XShapes> xGroup3(xGroup->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xGroup3->getCount());
    uno::Reference<drawing::XShape> xShapeEmpty(xGroup3->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextEmpty(xShapeEmpty, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("empty"), xTextEmpty->getString());

    CPPUNIT_ASSERT_EQUAL(xShapeA->getSize().Width, xShapeEmpty->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(xShapeA->getSize().Height, xShapeEmpty->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(xShapeA->getPosition().X, xShapeEmpty->getPosition().X);
    CPPUNIT_ASSERT_GREATER(xShapeA->getPosition().Y + 2 * xShapeA->getSize().Height,
                           xShapeEmpty->getPosition().Y);

    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xGroupShape->getPosition().Y + xGroupShape->getSize().Height,
                         xShapeEmpty->getPosition().Y + xShapeEmpty->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testMissingBulletAndIndent)
{
    createSdImpressDoc("pptx/smartart-missing-bullet.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup1(xGroup->getByIndex(2), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup2(xGroup1->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xGroup2->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xText.is());

    uno::Reference<container::XEnumerationAccess> xParasAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParas = xParasAccess->createEnumeration();
    xParas->nextElement(); // skip parent

    uno::Reference<beans::XPropertySet> xPara1(xParas->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara1.is());

    sal_Int16 nNumberingLevel = -1;
    xPara1->getPropertyValue("NumberingLevel") >>= nNumberingLevel;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nNumberingLevel);

    uno::Reference<container::XIndexAccess> xNumRule;
    xPara1->getPropertyValue("NumberingRules") >>= xNumRule;
    uno::Sequence<beans::PropertyValue> aBulletProps;
    xNumRule->getByIndex(1) >>= aBulletProps;

    for (beans::PropertyValue const& rProp : std::as_const(aBulletProps))
    {
        if (rProp.Name == "LeftMargin")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(309), rProp.Value.get<sal_Int32>());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testBulletList)
{
    createSdImpressDoc("pptx/smartart-bullet-list.pptx");
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<text::XText> xText(xGroup->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParasAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParas = xParasAccess->createEnumeration();
    xParas->nextElement(); // skip parent

    // child levels should have bullets
    uno::Reference<beans::XPropertySet> xPara1(xParas->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xRules1(xPara1->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aRule1(xRules1->getByIndex(1));
    CPPUNIT_ASSERT_EQUAL(u"•"_ustr, aRule1["BulletChar"].get<OUString>());

    uno::Reference<beans::XPropertySet> xPara2(xParas->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xRules2(xPara2->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aRule2(xRules2->getByIndex(2));
    CPPUNIT_ASSERT_EQUAL(u"•"_ustr, aRule2["BulletChar"].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testRecursion)
{
    createSdImpressDoc("pptx/smartart-recursion.pptx");

    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup1(xGroup->getByIndex(1), uno::UNO_QUERY);

    uno::Reference<drawing::XShapes> xGroupA(xGroup1->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextA(xGroupA->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A"), xTextA->getString());

    uno::Reference<drawing::XShapes> xGroupB(xGroup1->getByIndex(1), uno::UNO_QUERY);
    // 5 connectors, B1 with children, B2 with children
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7), xGroupB->getCount());

    uno::Reference<drawing::XShapes> xGroupB1(xGroupB->getByIndex(1), uno::UNO_QUERY);

    uno::Reference<drawing::XShapes> xGroupB1a(xGroupB1->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextB1(xGroupB1a->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("B1"), xTextB1->getString());

    uno::Reference<drawing::XShape> xGroupC12(xGroupB1->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextC1(
        getChildShape(getChildShape(getChildShape(xGroupC12, 0), 0), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("C1"), xTextC1->getString());
    uno::Reference<text::XText> xTextC2(
        getChildShape(getChildShape(getChildShape(xGroupC12, 1), 0), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("C2"), xTextC2->getString());

    uno::Reference<drawing::XShapes> xGroupB2(xGroupB->getByIndex(5), uno::UNO_QUERY);

    uno::Reference<drawing::XShapes> xGroupB2a(xGroupB2->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextB2(xGroupB2a->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), xTextB2->getString());

    uno::Reference<drawing::XShape> xGroupC3(xGroupB2->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xTextC3(
        getChildShape(getChildShape(getChildShape(xGroupC3, 0), 0), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("C3"), xTextC3->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testDataFollow)
{
    // checks if data nodes are followed correctly
    // different variables are set for two presentation points with the same name
    // they should be layouted differently - one horizontally and one vertically

    createSdImpressDoc("pptx/smartart-data-follow.pptx");

    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);

    uno::Reference<drawing::XShapes> xGroupLeft(xGroup->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroupB(xGroupLeft->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeB1
        = getChildShape(getChildShape(getChildShape(xGroupB, 1), 0), 0);
    uno::Reference<text::XText> xTextB1(xShapeB1, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("B1"), xTextB1->getString());
    uno::Reference<drawing::XShape> xShapeB2
        = getChildShape(getChildShape(getChildShape(xGroupB, 3), 0), 0);
    uno::Reference<text::XText> xTextB2(xShapeB2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("B2"), xTextB2->getString());

    CPPUNIT_ASSERT_EQUAL(xShapeB1->getPosition().Y, xShapeB2->getPosition().Y);
    CPPUNIT_ASSERT_GREATEREQUAL(xShapeB1->getPosition().X + xShapeB1->getSize().Width,
                                xShapeB2->getPosition().X);

    uno::Reference<drawing::XShapes> xGroupRight(xGroup->getByIndex(2), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroupC(xGroupRight->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeC1
        = getChildShape(getChildShape(getChildShape(xGroupC, 3), 0), 0);
    uno::Reference<text::XText> xTextC1(xShapeC1, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("C1"), xTextC1->getString());
    uno::Reference<drawing::XShape> xShapeC2
        = getChildShape(getChildShape(getChildShape(xGroupC, 5), 0), 0);
    uno::Reference<text::XText> xTextC2(xShapeC2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("C2"), xTextC2->getString());

    CPPUNIT_ASSERT_EQUAL(xShapeC1->getPosition().X, xShapeC2->getPosition().X);
    CPPUNIT_ASSERT_GREATEREQUAL(xShapeC1->getPosition().Y + xShapeC1->getSize().Height,
                                xShapeC2->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testOrgChart2)
{
    createSdImpressDoc("pptx/smartart-org-chart2.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShapeC1 = findChildShapeByText(xGroup, "C1");
    uno::Reference<drawing::XShape> xShapeC2 = findChildShapeByText(xGroup, "C2");
    uno::Reference<drawing::XShape> xShapeC3 = findChildShapeByText(xGroup, "C3");
    uno::Reference<drawing::XShape> xShapeC4 = findChildShapeByText(xGroup, "C4");
    uno::Reference<drawing::XShape> xShapeD1 = findChildShapeByText(xGroup, "D1");
    uno::Reference<drawing::XShape> xShapeD2 = findChildShapeByText(xGroup, "D2");

    CPPUNIT_ASSERT(xShapeC1.is());
    CPPUNIT_ASSERT(xShapeC2.is());
    CPPUNIT_ASSERT(xShapeC3.is());
    CPPUNIT_ASSERT(xShapeC4.is());
    CPPUNIT_ASSERT(xShapeD1.is());
    CPPUNIT_ASSERT(xShapeD2.is());

    CPPUNIT_ASSERT_EQUAL(xShapeC1->getPosition().Y, xShapeC2->getPosition().Y);
    CPPUNIT_ASSERT_GREATEREQUAL(xShapeC1->getPosition().X + xShapeC1->getSize().Width,
                                xShapeC2->getPosition().X);

    CPPUNIT_ASSERT_EQUAL(xShapeC3->getPosition().X, xShapeC4->getPosition().X);
    CPPUNIT_ASSERT_GREATEREQUAL(xShapeC3->getPosition().Y + xShapeC3->getSize().Height,
                                xShapeC4->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(xShapeD1->getPosition().X, xShapeD2->getPosition().X);
    CPPUNIT_ASSERT_GREATEREQUAL(xShapeD1->getPosition().Y + xShapeD1->getSize().Height,
                                xShapeD2->getPosition().Y);

    CPPUNIT_ASSERT_GREATEREQUAL(xShapeC2->getPosition().X, xShapeD1->getPosition().X);
    CPPUNIT_ASSERT_GREATEREQUAL(xShapeC2->getPosition().Y + xShapeC2->getSize().Height,
                                xShapeD1->getPosition().Y);

    CPPUNIT_ASSERT_GREATEREQUAL(xShapeD1->getPosition().X + xShapeD1->getSize().Width,
                                xShapeC4->getPosition().X);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf131553)
{
    createSdImpressDoc("pptx/tdf131553.pptx");

    const SdrPage* pPage = GetPage(1);
    const SdrObjGroup* pObjGroup = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObjGroup);
    const SdrObject* pObj = pObjGroup->GetSubList()->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::OLE2, pObj->GetObjIdentifier());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testFillColorList)
{
    createSdImpressDoc("pptx/fill-color-list.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape = getChildShape(getChildShape(xGroup, 1), 0);
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY_THROW);
    Color nFillColor;
    xPropertySet->getPropertyValue("FillColor") >>= nFillColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 12603469 (0xc0504d)
    // - Actual  : 16225862 (0xf79646)
    // i.e. the background of the "A" shape was orange-ish, rather than red-ish.
    CPPUNIT_ASSERT_EQUAL(Color(0xC0504D), nFillColor);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2239
    // - Actual  : 5199
    // i.e. the "A" shape's height/width aspect ratio was not 0.4 but rather close to 1.0, even if
    // ppt/diagrams/layout1.xml's <dgm:constr type="h" refType="w" op="lte" fact="0.4"/> requested
    // 0.4.
    awt::Size aActualSize = xShape->getSize();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2239), aActualSize.Height);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 1738 (2766)
    // - Actual  : 1738
    // i.e. the columns were not centered vertically.
    sal_Int32 nGroupTop = xGroup->getPosition().Y;
    sal_Int32 nShapeTop = xShape->getPosition().Y;
    CPPUNIT_ASSERT_GREATER(nGroupTop, nShapeTop);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf134221)
{
    createSdImpressDoc("pptx/smartart-tdf134221.pptx");
    saveAndReload("Impress Office Open XML");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShapeB = findChildShapeByText(xGroup, "B");
    CPPUNIT_ASSERT(xShapeB.is());
    uno::Reference<beans::XPropertySet> xTxtProps(xShapeB, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-248),
                         xTxtProps->getPropertyValue("TextUpperDistance").get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testLinearRule)
{
    createSdImpressDoc("pptx/smartart-linear-rule.pptx");

    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // Last child, then first child inside that.
    // It is first as backgroundArrow is last, but chOrder="t" is set to reverse the order.
    uno::Reference<drawing::XShape> xShape = getChildShape(getChildShape(xGroup, 1), 0);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 17500 (19867)
    // - Actual  : 4966
    // i.e. the width of the background arrow was too small.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(17500), xShape->getSize().Width);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3160
    // - Actual  : 8770
    // i.e. there was unexpected spacing on the left of the arrow.
    // Then the imported version of the test document failed with:
    // - Expected: 3160
    // - Actual  : 19828
    // i.e. the spacing on the left of the arrow was so large that the shape was mostly outside the
    // slide.
    sal_Int32 nGroupLeft = xGroup->getPosition().X;
    sal_Int32 nArrowLeft = xShape->getPosition().X;
    CPPUNIT_ASSERT_EQUAL(nGroupLeft, nArrowLeft);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less or equal than: 10092
    // - Actual  : 20183
    // i.e. the arrow height was larger than the canvas given to the smartart on slide 1.
    CPPUNIT_ASSERT_LESSEQUAL(static_cast<sal_Int32>(10092), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testLinearRuleVert)
{
    createSdImpressDoc("pptx/smartart-linear-rule-vert.pptx");

    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // Get the P1 shape.
    uno::Reference<drawing::XShape> xShape = getChildShape(getChildShape(xGroup, 1), 1);
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("P1"), xShapeText->getString());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2020
    // - Actual  : 10308
    // i.e. the first item on the vertical linear layout used ~all space, the other items were not
    // visible.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2020), xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testAutofitSync)
{
    createSdImpressDoc("pptx/smartart-autofit-sync.pptx");

    uno::Reference<drawing::XShape> xDiagram(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xMiddle = getChildShape(xDiagram, 2);
    uno::Reference<beans::XPropertySet> xFirstInner(getChildShape(getChildShape(xMiddle, 0), 0),
                                                    uno::UNO_QUERY);
    double fFirstScale = 0;
    CPPUNIT_ASSERT(xFirstInner->getPropertyValue("TextFitToSizeFontScale") >>= fFirstScale);
    CPPUNIT_ASSERT_GREATER(0.0, fFirstScale);
    CPPUNIT_ASSERT_LESS(100.0, fFirstScale);

    uno::Reference<beans::XPropertySet> xSecondInner(getChildShape(getChildShape(xMiddle, 2), 0),
                                                     uno::UNO_QUERY);
    double fSecondScale = 0;
    CPPUNIT_ASSERT(xSecondInner->getPropertyValue("TextFitToSizeFontScale") >>= fSecondScale);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 56
    // - Actual  : 100
    // i.e. the left shape had no scale-down and the right shape was scaled down, even if it was
    // requested that their scaling matches.
    CPPUNIT_ASSERT_EQUAL(fSecondScale, fFirstScale);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (drawing::TextFitToSizeType_NONE)
    // - Actual  : 3 (TextFitToSizeType_AUTOFIT)
    // i.e. the 3rd shape had font size as direct formatting, but its automatic text scale was not
    // disabled.
    uno::Reference<beans::XPropertySet> xThirdInner(getChildShape(getChildShape(xMiddle, 4), 0),
                                                    uno::UNO_QUERY);
    drawing::TextFitToSizeType eType{};
    CPPUNIT_ASSERT(xThirdInner->getPropertyValue("TextFitToSize") >>= eType);
    CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, eType);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testSnakeRows)
{
    // Load a smartart which contains a snake algorithm.
    // The expected layout of the 6 children is a 3x2 grid.
    createSdImpressDoc("pptx/smartart-snake-rows.pptx");

    uno::Reference<drawing::XShapes> xDiagram(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // Collect position of the background and the real child shapes. First row and background has
    // the same top position, unless some unexpected spacing happens, since this is a
    // "left-to-right, then top-to-bottom" snake direction.
    std::set<sal_Int32> aYPositions;
    for (sal_Int32 nChild = 0; nChild < xDiagram->getCount(); ++nChild)
    {
        uno::Reference<drawing::XShape> xChild(xDiagram->getByIndex(nChild), uno::UNO_QUERY);
        aYPositions.insert(xChild->getPosition().Y);
    }

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. an unwanted row appeared.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aYPositions.size());
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testCompositeInferRight)
{
    // Load a smartart which contains a composite algorithm.
    // One constraint says that the left of the text should be the right of the image.
    createSdImpressDoc("pptx/smartart-composite-infer-right.pptx");

    uno::Reference<drawing::XShape> xDiagram(getShapeFromPage(0, 0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xMiddle = getChildShape(xDiagram, 1);
    uno::Reference<drawing::XShape> xImage = getChildShape(xMiddle, 1);
    uno::Reference<drawing::XShape> xText = getChildShape(xMiddle, 2);
    sal_Int32 nRightOfImage = xImage->getPosition().X + xImage->getSize().Width;
    sal_Int32 nLeftOfText = xText->getPosition().X;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 7925
    // - Actual  : 2430
    // i.e. the text was overlapping with the image.
    CPPUNIT_ASSERT_GREATER(nRightOfImage, nLeftOfText);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf149551Pie)
{
    // The file contains a diagram of type "Basic Pie".
    createSdImpressDoc("pptx/tdf149551_SmartArt_Pie.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // shape at index 0 is the background shape
    uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT(xShape.is());
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    CPPUNIT_ASSERT(pCustomShape);
    tools::Rectangle aTextRect;
    pCustomShape->TakeTextAnchorRect(aTextRect);
    // without fix the text area rectangle had LT [7787,3420] and RB[16677,12312]. The values from
    // txXfrm were ignored.
    // The used tolerance is estimated.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(12658), sal_Int32(aTextRect.Left()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(5073), sal_Int32(aTextRect.Top()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(15627), sal_Int32(aTextRect.Right()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(7990), sal_Int32(aTextRect.Bottom()), 4);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf149551Pyramid)
{
    // The file contains a diagram of type "Basic Pyramid".
    createSdImpressDoc("pptx/tdf149551_SmartArt_Pyramid.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // shape at index 0 is the background shape
    uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT(xShape.is());
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    CPPUNIT_ASSERT(pCustomShape);
    tools::Rectangle aTextRect;
    pCustomShape->TakeTextAnchorRect(aTextRect);
    // without fix the text area rectangle had LT [9369,2700] and RB[14632,6185]. The values from
    // txXfrm were ignored.
    // The used tolerance is estimated.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(7591), sal_Int32(aTextRect.Left()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1515), sal_Int32(aTextRect.Top()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(16410), sal_Int32(aTextRect.Right()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(7370), sal_Int32(aTextRect.Bottom()), 4);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf149551Venn)
{
    // The file contains a diagram of type "Stacked Venn".
    createSdImpressDoc("pptx/tdf149551_SmartArt_Venn.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // shape at index 0 is the background shape
    uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT(xShape.is());
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    CPPUNIT_ASSERT(pCustomShape);
    tools::Rectangle aTextRect;
    pCustomShape->TakeTextAnchorRect(aTextRect);
    // without fix the text area rectangle had LT [6865,3396] and RB[17136,4600]. The values from
    // txXfrm were ignored.
    // The used tolerance is estimated.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(8772), sal_Int32(aTextRect.Left()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1834), sal_Int32(aTextRect.Top()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(15229), sal_Int32(aTextRect.Right()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(6162), sal_Int32(aTextRect.Bottom()), 4);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf149551Gear)
{
    // The file contains a diagram of type "Gear".
    createSdImpressDoc("pptx/tdf149551_SmartArt_Gear.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // shape at index 0 is the background shape
    uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT(xShape.is());
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    CPPUNIT_ASSERT(pCustomShape);
    tools::Rectangle aTextRect;
    pCustomShape->TakeTextAnchorRect(aTextRect);
    // without fix the text area rectangle had LT [4101,1014] and RB[8019,4932]. The values from
    // txXfrm were ignored.
    // The used tolerance is estimated.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(5501), sal_Int32(aTextRect.Left()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4500), sal_Int32(aTextRect.Top()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(11000), sal_Int32(aTextRect.Right()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(9999), sal_Int32(aTextRect.Bottom()), 4);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf145528Matrix)
{
    // The file contains a diagram of type "Titled Matrix". Such is build from shapes of type
    // 'round1Rect'.
    createSdImpressDoc("pptx/tdf145528_SmartArt_Matrix.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // expected values
    sal_Int32 nLeft[]{ 4001, 12001, 12001, 18501 };
    sal_Int32 nTop[]{ 9999, 1999, 12499, 5999 };
    // nWidth = 10292, nHeight = 4499
    // Without the fix in place the values were
    // nLeft {2001, 12001, 12001, 22001}
    // nTop {7999. 1999, 13999, 7999}
    // nWidth {6000, 10000, 10000, 6000}
    // nHeight {10000, 6000, 6000, 10000}
    tools::Rectangle aTextRect;
    for (auto i : { 1, 2, 3, 4 }) // shape at index 0 is the background shape
    {
        uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, i);
        CPPUNIT_ASSERT(xShape.is());
        auto pCustomShape
            = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pCustomShape);
        pCustomShape->TakeTextAnchorRect(aTextRect);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(nLeft[i - 1], sal_Int32(aTextRect.Left()), 4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(nTop[i - 1], sal_Int32(aTextRect.Top()), 4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(10293), sal_Int32(aTextRect.GetWidth()), 4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4500), sal_Int32(aTextRect.GetHeight()), 4);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf135953TextPosition)
{
    // The file contains a diagram of type "Detailed Process". There the text area rectangle
    // is at the left edge of the shape and rotated there.
    createSdImpressDoc("pptx/tdf135953_SmartArt_textposition.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // shape at index 0 is the background shape
    uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT(xShape.is());
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    CPPUNIT_ASSERT(pCustomShape);
    tools::Rectangle aTextRect;
    pCustomShape->TakeTextAnchorRect(aTextRect);
    // without fix the text area rectangle had LT[-2213,2336] and RB[4123,4575]. It was not
    // considered that the txXfrm and the preset text area rectangle have different centers in this
    // case and thus the text was far off.
    // The used tolerance is estimated.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(3223), sal_Int32(aTextRect.Left()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(-1661), sal_Int32(aTextRect.Top()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(9559), sal_Int32(aTextRect.Right()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(578), sal_Int32(aTextRect.Bottom()), 4);
}

CPPUNIT_TEST_FIXTURE(SdImportTestSmartArt, testTdf132302RightArrow)
{
    // The file contains a diagram of type "Process Arrows". It uses shapes of type "rightArrow".
    // The text starts not at the left edge but in the middle to have space for a circle. Error was
    // that the text starts left and so was hidden by the circle.
    createSdImpressDoc("pptx/tdf132302_SmartArt_rightArrow.pptx");
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0), uno::UNO_QUERY);
    // shape at index 0 is the background shape
    uno::Reference<drawing::XShape> xShape = getChildShape(xGroup, 1);
    CPPUNIT_ASSERT(xShape.is());
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
    CPPUNIT_ASSERT(pCustomShape);
    tools::Rectangle aTextRect;
    pCustomShape->TakeTextAnchorRect(aTextRect);
    // without fix the text area rectangle had LT[2790,59] and RB[11591,7940]. Position and size
    // given in txXfrm in drawing.xml were not considered.
    // The used tolerance is estimated.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(5078), sal_Int32(aTextRect.Left()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(58), sal_Int32(aTextRect.Top()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(11989), sal_Int32(aTextRect.Right()), 4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(7940), sal_Int32(aTextRect.Bottom()), 4);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
