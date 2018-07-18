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

using namespace ::com::sun::star;

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

    CPPUNIT_TEST_SUITE(SdImportTestSmartArt);

    CPPUNIT_TEST(testBase);
    CPPUNIT_TEST(testChildren);
    CPPUNIT_TEST(testText);
    CPPUNIT_TEST(testCnt);
    CPPUNIT_TEST(testDir);
    CPPUNIT_TEST(testMaxDepth);
    CPPUNIT_TEST(testRotation);
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

void SdImportTestSmartArt::testBasicProcess()
{
    //FIXME : so far this only introduce the test document, but the actual importer was not fixed yet.
}

void SdImportTestSmartArt::testPyramid()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-pyramid.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xShapeGroup->getCount());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);

    uno::Reference<text::XText> xText0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xText0->getString());
    uno::Reference<text::XText> xText1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xText1->getString());
    uno::Reference<text::XText> xText2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xText2->getString());

    CPPUNIT_ASSERT(xShape1->getPosition().Y > xShape0->getPosition().Y);
    CPPUNIT_ASSERT(xShape2->getPosition().Y > xShape1->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(xShape1->getSize().Height, xShape2->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(xShape2->getSize().Height, xShape3->getSize().Height);

    xDocShRef->DoClose();
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
    CPPUNIT_ASSERT_EQUAL(xShape0->getPosition().Y , xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(xShape2->getPosition().Y , xShape1->getPosition().Y); // Confirms shapes are in same Y axis-level.

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/smartart-invertedPyramid.pptx"), PPTX);
    uno::Reference<drawing::XShapes> xShapeGroup(getShapeFromPage(0, 0, xDocShRef), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xShapeGroup->getCount());

    uno::Reference<drawing::XShape> xShape0(xShapeGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape1(xShapeGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xShapeGroup->getByIndex(2), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xShape1->getPosition().Y < xShape0->getPosition().Y);
    CPPUNIT_ASSERT(xShape2->getPosition().Y < xShape1->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(xShape0->getSize().Height, xShape1->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(xShape1->getSize().Height, xShape2->getSize().Height);

    xDocShRef->DoClose();
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

CPPUNIT_TEST_SUITE_REGISTRATION(SdImportTestSmartArt);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
