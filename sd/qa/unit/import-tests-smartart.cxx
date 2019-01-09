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
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XText.hpp>

#include <comphelper/sequenceashashmap.hxx>

using namespace ::com::sun::star;

namespace
{
/// Gets one child of xShape, which one is specified by nIndex.
uno::Reference<drawing::XShape> getChildShape(const uno::Reference<drawing::XShape>& xShape, sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    CPPUNIT_ASSERT(xGroup->getCount() > nIndex);

    uno::Reference<drawing::XShape> xRet(xGroup->getByIndex(nIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRet.is());

    return xRet;
}
}

class SdImportTestSmartArt : public SdModelTestBase
{
public:
    void testBase();
    void testChildren();
    void testText();
    void testCnt();
    void testDir();
    void testMaxDepth();
    void testRotation();
    void testTextAutoRotation();
    void testPyramid();
    void testChevron();
    void testCycle();
    void testvenndiagram();
    void testmatrix();
    void testHierarchy();
    void testInvertedPyramid();
    void testBasicProcess();
    void testMultidirectional();
    void testHorizontalBulletList();
    void testBasicRadicals();
    void testEquation();
    void testSegmentedCycle();
    void testBaseRtoL();
    void testVerticalBoxList();
    void testVerticalBracketList();
    void testTableList();
    void testAccentProcess();
    void testContinuousBlockProcess();
    void testOrgChart();

    CPPUNIT_TEST_SUITE(SdImportTestSmartArt);

    CPPUNIT_TEST(testBase);
    CPPUNIT_TEST(testChildren);
    CPPUNIT_TEST(testText);
    CPPUNIT_TEST(testCnt);
    CPPUNIT_TEST(testDir);
    CPPUNIT_TEST(testMaxDepth);
    CPPUNIT_TEST(testRotation);
    CPPUNIT_TEST(testTextAutoRotation);
    CPPUNIT_TEST(testPyramid);
    CPPUNIT_TEST(testChevron);
    CPPUNIT_TEST(testCycle);
    CPPUNIT_TEST(testHierarchy);
    CPPUNIT_TEST(testmatrix);
    CPPUNIT_TEST(testvenndiagram);
    CPPUNIT_TEST(testInvertedPyramid);
    CPPUNIT_TEST(testBasicProcess);
    CPPUNIT_TEST(testMultidirectional);
    CPPUNIT_TEST(testHorizontalBulletList);
    CPPUNIT_TEST(testBasicRadicals);
    CPPUNIT_TEST(testEquation);
    CPPUNIT_TEST(testSegmentedCycle);
    CPPUNIT_TEST(testBaseRtoL);
    CPPUNIT_TEST(testVerticalBoxList);
    CPPUNIT_TEST(testVerticalBracketList);
    CPPUNIT_TEST(testTableList);
    CPPUNIT_TEST(testAccentProcess);
    CPPUNIT_TEST(testContinuousBlockProcess);
    CPPUNIT_TEST(testOrgChart);

    CPPUNIT_TEST_SUITE_END();
};

void SdImportTestSmartArt::testBase()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart1.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());
    uno::Reference<text::XText> xText3(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), xText3->getString());
    uno::Reference<text::XText> xText4(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("e"), xText4->getString());

    uno::Reference<beans::XPropertySet> xShape(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);

    sal_Int32 nFillColor = 0;
    xShape->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4F81BD), nFillColor);

    sal_Int16 nParaAdjust = 0;
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(nParaAdjust));

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape3(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape4(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y,xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y,xShape3->getPosition().Y);
    CPPUNIT_ASSERT(xShape2->getPosition().Y > xShape0->getPosition().Y);
    CPPUNIT_ASSERT(xShape4->getPosition().Y > xShape2->getPosition().Y);
    CPPUNIT_ASSERT(xShape0->getPosition().X < xShape1->getPosition().X);
    CPPUNIT_ASSERT(xShape2->getPosition().X < xShape3->getPosition().X);
    CPPUNIT_ASSERT((xShape2->getPosition().X <  xShape4->getPosition().X) && (xShape3->getPosition().X > xShape4->getPosition().X));

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testChildren()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-children.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xShapeGroup0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
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

    uno::Reference<drawing::XShapes> xShapeGroup1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
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

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testText()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-text.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xShapeGroup2(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<text::XText> xText0(xShapeGroup2->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xText0->getString().isEmpty());

    uno::Reference<text::XText> xText1(xShapeGroup2->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xText1->getString());

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testCnt()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-cnt.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    sal_Int32 nCount = xShapeGroup->getCount();
    sal_Int32 nCorrect = 0;
    for (sal_Int32 i=0; i<nCount; i++)
    {
        uno::Reference<text::XText> xText(xShapeGroup->getByIndex(i), uno::UNO_QUERY);
        if (xText.is() && !xText->getString().isEmpty())
            nCorrect++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nCorrect);

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testDir()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-dir.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup->getCount());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape0->getPosition().X > xShape1->getPosition().X);

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testMaxDepth()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-maxdepth.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("first"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("second"), xText1->getString());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y , xShape1->getPosition().Y); // Confirms shapes are in same Y axis-level.

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testRotation()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-rotation.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape0->getPropertyValue("RotateAngle").get<sal_Int32>());

    uno::Reference<beans::XPropertySet> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24000), xShape1->getPropertyValue("RotateAngle").get<sal_Int32>());

    uno::Reference<beans::XPropertySet> xShape2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12000), xShape2->getPropertyValue("RotateAngle").get<sal_Int32>());

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testTextAutoRotation()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-autoTxRot.pptx"), PPTX);

    auto testText = [&](int pageNo, sal_Int32 txtNo, const OUString& expTx, sal_Int32 expShRot,
                        sal_Int32 expTxRot) {
        OString msgText = "Page: " + OString::number(pageNo) + " text: " + OString::number(txtNo);
        uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, pageNo, xDocShRef),
                                                     uno::UNO_QUERY_THROW);

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
    testText(0, 0,  "a", 0,     0);
    testText(0, 1,  "b", 33750, 0);
    testText(0, 2,  "c", 31500, 0);
    testText(0, 3,  "d", 29250, 90);
    testText(0, 4,  "e", 27000, 90);
    testText(0, 5,  "f", 24750, 90);
    testText(0, 6,  "g", 22500, 180);
    testText(0, 7,  "h", 20250, 180);
    testText(0, 8,  "i", 18000, 180);
    testText(0, 9,  "j", 15750, 180);
    testText(0, 10, "k", 13500, 180);
    testText(0, 11, "l", 11250, 270);
    testText(0, 12, "m", 9000,  270);
    testText(0, 13, "n", 6750,  270);
    testText(0, 14, "o", 4500,  0);
    testText(0, 15, "p", 2250,  0);

    // Slide 2: autoTxRot == "none"
    testText(1, 0,  "a", 0,     0);
    testText(1, 1,  "b", 33750, 0);
    testText(1, 2,  "c", 31500, 0);
    testText(1, 3,  "d", 29250, 0);
    testText(1, 4,  "e", 27000, 0);
    testText(1, 5,  "f", 24750, 0);
    testText(1, 6,  "g", 22500, 0);
    testText(1, 7,  "h", 20250, 0);
    testText(1, 8,  "i", 18000, 0);
    testText(1, 9,  "j", 15750, 0);
    testText(1, 10, "k", 13500, 0);
    testText(1, 11, "l", 11250, 0);
    testText(1, 12, "m", 9000,  0);
    testText(1, 13, "n", 6750,  0);
    testText(1, 14, "o", 4500,  0);
    testText(1, 15, "p", 2250,  0);

    // Slide 3: autoTxRot == "grav"
    testText(2, 0,  "a", 0,     0);
    testText(2, 1,  "b", 33750, 0);
    testText(2, 2,  "c", 31500, 0);
    testText(2, 3,  "d", 29250, 0);
    testText(2, 4,  "e", 27000, 0);
    testText(2, 5,  "f", 24750, 180);
    testText(2, 6,  "g", 22500, 180);
    testText(2, 7,  "h", 20250, 180);
    testText(2, 8,  "i", 18000, 180);
    testText(2, 9,  "j", 15750, 180);
    testText(2, 10, "k", 13500, 180);
    testText(2, 11, "l", 11250, 180);
    testText(2, 12, "m", 9000,  0);
    testText(2, 13, "n", 6750,  0);
    testText(2, 14, "o", 4500,  0);
    testText(2, 15, "p", 2250,  0);

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testBasicProcess()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testPyramid()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testChevron()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-chevron.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xShape0->getPosition().X < xShape1->getPosition().X && xShape1->getPosition().X < xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape1->getPosition().Y, xShape2->getPosition().Y);

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testCycle()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testHierarchy()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testmatrix()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testvenndiagram()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testInvertedPyramid()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testMultidirectional()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testHorizontalBulletList()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testEquation()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testBasicRadicals()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testSegmentedCycle()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testBaseRtoL()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-rightoleftblockdiagram.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xShapeGroup->getCount());

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());
    uno::Reference<text::XText> xText3(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), xText3->getString());
    uno::Reference<text::XText> xText4(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("e"), xText4->getString());

    uno::Reference<beans::XPropertySet> xShape(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);

    sal_Int32 nFillColor = 0;
    xShape->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4F81BD), nFillColor);

    sal_Int16 nParaAdjust = 0;
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(nParaAdjust));

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape3(xShapeGroup->getByIndex(3), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape4(xShapeGroup->getByIndex(4), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y,xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y,xShape3->getPosition().Y);
    CPPUNIT_ASSERT(xShape2->getPosition().Y > xShape0->getPosition().Y);
    CPPUNIT_ASSERT(xShape4->getPosition().Y > xShape2->getPosition().Y);
    CPPUNIT_ASSERT(xShape0->getPosition().X > xShape1->getPosition().X);
    CPPUNIT_ASSERT(xShape2->getPosition().X > xShape3->getPosition().X);
    CPPUNIT_ASSERT((xShape2->getPosition().X >  xShape4->getPosition().X) && (xShape3->getPosition().X < xShape4->getPosition().X));

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testVerticalBoxList()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/smartart-vertical-box-list.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef),
                                                 uno::UNO_QUERY_THROW);
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 0'.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xFirstChild(xShapeGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstChild.is());
    uno::Reference<drawing::XShape> xParentText(xFirstChild->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParentText.is());
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 7361', i.e. the width was not the 70% of the parent as the
    // constraint wanted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11852), xParentText->getSize().Width);

    uno::Reference<drawing::XShape> xChildText(xShapeGroup->getByIndex(1), uno::UNO_QUERY);
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

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testVerticalBracketList()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/vertical-bracket-list.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef),
                                                 uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xShapeGroup->getCount());

    uno::Reference<drawing::XShapes> xFirstChild(xShapeGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFirstChild.is());
    // Without the accompanying fix in place, this test would have failed with
    // 'actual: 2', i.e. one child shape (with its "A" text) was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xFirstChild->getCount());

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testTableList()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/table-list.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeGroup.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xShapeGroup->getCount());

    uno::Reference<text::XText> xParentText(xShapeGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParentText.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Parent"), xParentText->getString());
    uno::Reference<drawing::XShape> xParent(xParentText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParent.is());
    int nParentRight = xParent->getPosition().X + xParent->getSize().Width;

    uno::Reference<drawing::XShapes> xChildren(xShapeGroup->getByIndex(1), uno::UNO_QUERY);
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

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testAccentProcess()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/smartart-accent-process.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    // 3 children: first pair, connector, second pair.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup->getCount());
    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroupShape.is());

    // The pair is a parent (shape + text) and a child, so 3 shapes in total.
    // The order is important, first is at the back, last is at the front.
    uno::Reference<drawing::XShapes> xFirstPair(xGroup->getByIndex(0), uno::UNO_QUERY);
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
        comphelper::SequenceAsHashMap aRule(xRules->getByIndex(1));
        CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(u8"•"), aRule["BulletChar"].get<OUString>());
    }

    int nFirstChildTop = xFirstChild->getPosition().Y;
    int nFirstChildRight = xFirstChild->getPosition().X + xFirstChild->getSize().Width;

    // First child is below the first parent.
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected less than: 3881, Actual  : 3881', i.e. xFirstChild was not
    // below xFirstParent (a good position is 9081).
    CPPUNIT_ASSERT_LESS(nFirstChildTop, nFirstParentTop);

    // Make sure that we have an arrow shape between the two pairs.
    uno::Reference<beans::XPropertySet> xArrow(xGroup->getByIndex(1), uno::UNO_QUERY);
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

    uno::Reference<drawing::XShapes> xSecondPair(xGroup->getByIndex(2), uno::UNO_QUERY);
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

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testContinuousBlockProcess()
{
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/smartart-continuous-block-process.pptx"),
        PPTX);
    uno::Reference<drawing::XShapes> xGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    // 2 children: background, foreground.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup->getCount());

    uno::Reference<drawing::XShapes> xLinear(xGroup->getByIndex(1), uno::UNO_QUERY);
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

    xDocShRef->DoClose();
}

void SdImportTestSmartArt::testOrgChart()
{
    // Simple org chart with 1 manager and 1 employee only.
    sd::DrawDocShellRef xDocShRef = loadURL(
        m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/smartart-org-chart.pptx"),
        PPTX);
    uno::Reference<drawing::XShape> xGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    uno::Reference<text::XText> xManager(
        getChildShape(getChildShape(getChildShape(xGroup, 0), 0), 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xManager.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Manager"), xManager->getString());

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xManager, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xPara, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nActualColor = xRun->getPropertyValue("CharColor").get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed: the
    // "Manager" font color was black, not white.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffff), nActualColor);

    uno::Reference<drawing::XShape> xManagerShape(xManager, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xManagerShape.is());

    awt::Point aManagerPos = xManagerShape->getPosition();

    // Make sure that the manager has 2 employees.
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 1'.
    uno::Reference<drawing::XShapes> xEmployees(getChildShape(getChildShape(xGroup, 0), 2),
                                                uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployees.is());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xEmployees->getCount());

    uno::Reference<text::XText> xEmployee(
        getChildShape(
            getChildShape(getChildShape(getChildShape(getChildShape(xGroup, 0), 2), 0), 0), 0),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployee.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Employee"), xEmployee->getString());

    uno::Reference<drawing::XShape> xEmployeeShape(xEmployee, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployeeShape.is());

    awt::Point aEmployeePos = xEmployeeShape->getPosition();

    CPPUNIT_ASSERT_EQUAL(aManagerPos.X, aEmployeePos.X);

    // Without the accompanying fix in place, this test would have failed: the
    // two shapes were overlapping, i.e. "manager" was not above "employee".
    CPPUNIT_ASSERT_GREATER(aManagerPos.Y, aEmployeePos.Y);

    // Make sure that the second employee is on the right of the first one.
    // Without the accompanying fix in place, this test would have failed, as
    // the second employee was below the first one.
    uno::Reference<text::XText> xEmployee2(
        getChildShape(
            getChildShape(getChildShape(getChildShape(getChildShape(xGroup, 0), 2), 1), 0), 0),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployee2.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Employee2"), xEmployee2->getString());

    uno::Reference<drawing::XShape> xEmployee2Shape(xEmployee2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmployee2Shape.is());

    awt::Point aEmployee2Pos = xEmployee2Shape->getPosition();
    CPPUNIT_ASSERT_GREATER(aEmployeePos.X, aEmployee2Pos.X);

    // Make sure that assistant is above employees.
    uno::Reference<text::XText> xAssistant(
        getChildShape(
            getChildShape(getChildShape(getChildShape(getChildShape(xGroup, 0), 1), 1), 0), 0),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Assistant"), xAssistant->getString());

    uno::Reference<drawing::XShape> xAssistantShape(xAssistant, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAssistantShape.is());

    awt::Point aAssistantPos = xAssistantShape->getPosition();
    // Without the accompanying fix in place, this test would have failed: the
    // assistant shape was below the employee shape.
    CPPUNIT_ASSERT_GREATER(aAssistantPos.Y, aEmployeePos.Y);

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdImportTestSmartArt);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
