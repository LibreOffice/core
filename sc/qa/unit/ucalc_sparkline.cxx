/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include "helper/qahelper.hxx"
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <cliputil.hxx>
#include <docfunc.hxx>
#include <undomanager.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <SparklineList.hxx>

using namespace css;

/** Test operation for sparklines, sparkline groups and attributes */
class SparklineTest : public UnoApiTest
{
public:
    SparklineTest()
        : UnoApiTest("sc/qa/unit/data")
    {
    }

    void testAddSparkline();
    void testClearContentSprkline();
    void testCopyPasteSparkline();
    void testCutPasteSparkline();
    void testUndoRedoInsertSparkline();
    void testUndoRedoDeleteSparkline();
    void testUndoRedoDeleteSparklineGroup();
    void testUndoRedoClearContentForSparkline();
    void testUndoRedoEditSparklineGroup();
    void testUndoRedoUngroupSparklines();
    void testUndoRedoGroupSparklines();
    void testUndoRedoEditSparkline();
    void testSparklineList();

    CPPUNIT_TEST_SUITE(SparklineTest);
    CPPUNIT_TEST(testAddSparkline);
    CPPUNIT_TEST(testClearContentSprkline);
    CPPUNIT_TEST(testCopyPasteSparkline);
    CPPUNIT_TEST(testCutPasteSparkline);
    CPPUNIT_TEST(testUndoRedoInsertSparkline);
    CPPUNIT_TEST(testUndoRedoDeleteSparkline);
    CPPUNIT_TEST(testUndoRedoDeleteSparklineGroup);
    CPPUNIT_TEST(testUndoRedoClearContentForSparkline);
    CPPUNIT_TEST(testUndoRedoEditSparklineGroup);
    CPPUNIT_TEST(testUndoRedoUngroupSparklines);
    CPPUNIT_TEST(testUndoRedoGroupSparklines);
    CPPUNIT_TEST(testUndoRedoEditSparkline);
    CPPUNIT_TEST(testSparklineList);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
void insertTestData(ScDocument& rDoc)
{
    rDoc.SetValue(0, 0, 0, 4);
    rDoc.SetValue(0, 1, 0, -2);
    rDoc.SetValue(0, 2, 0, 1);
    rDoc.SetValue(0, 3, 0, -3);
    rDoc.SetValue(0, 4, 0, 5);
    rDoc.SetValue(0, 5, 0, 3);
}

sc::Sparkline* createTestSparkline(ScDocument& rDoc)
{
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    sc::Sparkline* pSparkline = rDoc.CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);
    if (!pSparkline)
        return nullptr;

    insertTestData(rDoc);

    ScRangeList aList;
    aList.push_back(ScRange(0, 0, 0, 0, 5, 0));
    pSparkline->setInputRange(aList);

    return pSparkline;
}

} // end anonymous namespace

void SparklineTest::testAddSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    auto pSparkline = createTestSparkline(*pDoc);
    CPPUNIT_ASSERT(pSparkline);

    auto pGetSparkline = pDoc->GetSparkline(ScAddress(0, 6, 0));
    CPPUNIT_ASSERT(pGetSparkline);

    CPPUNIT_ASSERT_EQUAL(pGetSparkline.get(), pSparkline);

    sc::SparklineList* pList = pDoc->GetSparklineList(SCTAB(0));
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->getSparklineGroups().size());

    auto const& aSparklineVector = pList->getSparklinesFor(pGetSparkline->getSparklineGroup());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aSparklineVector.size());
    CPPUNIT_ASSERT_EQUAL(aSparklineVector[0].get(), pSparkline);
}

void SparklineTest::testClearContentSprkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    auto pSparkline = createTestSparkline(*pDoc);
    CPPUNIT_ASSERT(pSparkline);

    clearRange(pDoc, ScRange(0, 6, 0, 0, 6, 0));

    auto pGetSparkline = pDoc->GetSparkline(ScAddress(0, 6, 0));
    CPPUNIT_ASSERT(!pGetSparkline);
}

void SparklineTest::testCopyPasteSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    auto* pCreatedSparkline = createTestSparkline(*pDoc);
    CPPUNIT_ASSERT(pCreatedSparkline);

    ScRange aSourceRange(0, 6, 0, 0, 6, 0);
    auto pSparkline = pDoc->GetSparkline(aSourceRange.aStart);
    auto const& pOriginalGroup = pSparkline->getSparklineGroup();

    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // CopyToClip / CopyFromClip with a aClipDoc
    {
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        copyToClip(pDoc, aSourceRange, &aClipDoc);

        auto pClipSparkline = aClipDoc.GetSparkline(aSourceRange.aStart);
        CPPUNIT_ASSERT(pClipSparkline);

        ScRange aPasteRange(0, 7, 0, 0, 7, 0);

        ScMarkData aMark(pDoc->GetSheetLimits());
        aMark.SetMarkArea(aPasteRange);
        pDoc->CopyFromClip(aPasteRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

        auto pSparklineCopy = pDoc->GetSparkline(aPasteRange.aStart);
        CPPUNIT_ASSERT(pSparklineCopy);

        CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(7), pSparklineCopy->getRow());

        auto const& pCopyGroup = pSparklineCopy->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pOriginalGroup->getID(), pCopyGroup->getID());
        CPPUNIT_ASSERT_EQUAL(pOriginalGroup.get(), pCopyGroup.get());
    }

    // Copy / Paste with a ClipDoc
    {
        pViewShell->GetViewData().GetMarkData().SetMarkArea(aSourceRange);

        // Copy
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

        // Paste
        ScRange aPasteRange(0, 8, 0, 0, 8, 0);

        pViewShell->GetViewData().GetMarkData().SetMarkArea(aPasteRange);
        pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

        auto pSparklineCopy = pDoc->GetSparkline(aPasteRange.aStart);
        CPPUNIT_ASSERT(pSparklineCopy);

        CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(8), pSparklineCopy->getRow());

        auto const& pCopyGroup = pSparklineCopy->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pOriginalGroup->getID(), pCopyGroup->getID());
        CPPUNIT_ASSERT_EQUAL(pOriginalGroup.get(), pCopyGroup.get());
    }
}

void SparklineTest::testCutPasteSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();

    auto* pCreatedSparkline = createTestSparkline(*pDoc);
    CPPUNIT_ASSERT(pCreatedSparkline);

    ScRange aSourceRange(0, 6, 0, 0, 6, 0);
    auto pSparkline = pDoc->GetSparkline(aSourceRange.aStart);

    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Mark source range
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSourceRange);

    // Cut
    pViewShell->GetViewData().GetView()->CopyToClip(nullptr, true /*bCut*/, false, false, true);

    // Paste
    ScRange aPasteRange(0, 7, 0, 0, 7, 0);
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aPasteRange);
    ScClipUtil::PasteFromClipboard(pViewShell->GetViewData(), pViewShell, false);

    // Check
    auto pSparklineCopy = pDoc->GetSparkline(aPasteRange.aStart);
    CPPUNIT_ASSERT(pSparklineCopy);

    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(7), pSparklineCopy->getRow());
}

void SparklineTest::testUndoRedoInsertSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScRange aRange(0, 6, 0, 0, 6, 0);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup);

    // Check Sparkline at cell A7 exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Undo
    pDoc->GetUndoManager()->Undo();

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    // Redo
    pDoc->GetUndoManager()->Redo();

    // Check Sparkline at cell A7 exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());
}

void SparklineTest::testUndoRedoDeleteSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScRange aRange(0, 6, 0, 0, 6, 0);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup));

    // Check Sparkline at cell A7 exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Delete Sparkline
    CPPUNIT_ASSERT(rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    // Undo
    pDoc->GetUndoManager()->Undo();

    // Check Sparkline at cell A7 exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Redo
    pDoc->GetUndoManager()->Redo();

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    CPPUNIT_ASSERT(!pDoc->HasSparkline(aRange.aStart));
}

void SparklineTest::testUndoRedoDeleteSparklineGroup()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    // Delete Sparkline
    CPPUNIT_ASSERT(rDocFunc.DeleteSparklineGroup(pSparklineGroup, SCTAB(0)));

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    // Undo
    pDoc->GetUndoManager()->Undo();

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    // Check if the sparkline has the input range set
    auto const& pSparkline = pDoc->GetSparkline(ScAddress(3, 6, 0));
    ScRangeList rRangeList = pSparkline->getInputRange();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rRangeList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(3, 0, 0, 3, 5, 0), rRangeList[0]);

    // Redo
    pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7
}

void SparklineTest::testUndoRedoClearContentForSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    // Check Sparklines - none should exist
    {
        CPPUNIT_ASSERT(!pDoc->HasSparkline(ScAddress(0, 6, 0)));
        CPPUNIT_ASSERT(!pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(!pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT(!pDoc->HasSparkline(ScAddress(3, 6, 0)));
    }

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    {
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(0, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 exists
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check D7
        auto pSparkline = pDoc->GetSparkline(ScAddress(3, 6, 0));
        CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSparkline->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

        // Check collections
        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(4), pSparklines.size());
    }

    // Clear content of cell D7 - including sparkline
    {
        ScMarkData aMark(pDoc->GetSheetLimits());
        aMark.SetMarkArea(ScAddress(3, 6, 0));
        rDocFunc.DeleteContents(aMark, InsertDeleteFlags::CONTENTS, true, true);
    }

    // Check Sparklines
    {
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 is gone
        CPPUNIT_ASSERT(!pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check collections
        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }

    // Undo
    pDoc->GetUndoManager()->Undo();

    // Check Sparkline
    {
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(0, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 exists - again
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check D7
        auto pSparkline = pDoc->GetSparkline(ScAddress(3, 6, 0));
        CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSparkline->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(4), pSparklines.size());
    }

    // Redo
    pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 is gone - again
        CPPUNIT_ASSERT(!pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check collections
        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }
}

void SparklineTest::testUndoRedoEditSparklineGroup()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    {
        sc::SparklineAttributes& rAttibutes = pSparklineGroup->getAttributes();
        rAttibutes.setType(sc::SparklineType::Column);
        rAttibutes.setColorSeries(COL_YELLOW);
        rAttibutes.setColorAxis(COL_GREEN);
    }

    pDoc->CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);

    sc::SparklineAttributes aNewAttributes;
    aNewAttributes.setType(sc::SparklineType::Stacked);
    aNewAttributes.setColorSeries(COL_BLACK);
    aNewAttributes.setColorAxis(COL_BLUE);

    sc::SparklineAttributes aInitialAttibutes(pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT(aNewAttributes != aInitialAttibutes);

    CPPUNIT_ASSERT_EQUAL(true, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(false, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, pSparklineGroup->getAttributes().getColorSeries());
    CPPUNIT_ASSERT_EQUAL(COL_GREEN, pSparklineGroup->getAttributes().getColorAxis());

    rDocFunc.ChangeSparklineGroupAttributes(pSparklineGroup, aNewAttributes);

    CPPUNIT_ASSERT_EQUAL(false, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(true, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Stacked, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pSparklineGroup->getAttributes().getColorSeries());
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pSparklineGroup->getAttributes().getColorAxis());

    pDoc->GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL(true, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(false, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, pSparklineGroup->getAttributes().getColorSeries());
    CPPUNIT_ASSERT_EQUAL(COL_GREEN, pSparklineGroup->getAttributes().getColorAxis());

    pDoc->GetUndoManager()->Redo();

    CPPUNIT_ASSERT_EQUAL(false, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(true, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Stacked, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pSparklineGroup->getAttributes().getColorSeries());
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pSparklineGroup->getAttributes().getColorAxis());
}

void SparklineTest::testUndoRedoUngroupSparklines()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    CPPUNIT_ASSERT(rDocFunc.UngroupSparklines(ScRange(2, 6, 0, 3, 6, 0)));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }

    // Undo
    pDoc->GetUndoManager()->Undo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    // Redo
    pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }
}

void SparklineTest::testUndoRedoGroupSparklines()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    CPPUNIT_ASSERT(rDocFunc.UngroupSparklines(ScRange(2, 6, 0, 3, 6, 0)));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }

    {
        auto pGroup = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();
        CPPUNIT_ASSERT(rDocFunc.GroupSparklines(ScRange(1, 6, 0, 3, 6, 0), pGroup));
    }

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT(pGroup1 != pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    // Undo
    pDoc->GetUndoManager()->Undo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }

    // Redo
    pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT(pGroup1 != pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }
}

void SparklineTest::testUndoRedoEditSparkline()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    auto& rDocFunc = pDocSh->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(*pDoc);

    // Sparkline range
    ScAddress aAddress(0, 6, 0);
    ScRange aRange(aAddress);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup));

    // Check Sparkline at cell A7
    pSparkline = pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 5, 0), pSparkline->getInputRange()[0]);

    // Change Sparkline
    CPPUNIT_ASSERT(
        rDocFunc.ChangeSparkline(pSparkline, SCTAB(0), ScRangeList(ScRange(0, 1, 0, 0, 4, 0))));

    pSparkline = pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 0, 4, 0), pSparkline->getInputRange()[0]);

    // Undo
    pDoc->GetUndoManager()->Undo();

    pSparkline = pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 5, 0), pSparkline->getInputRange()[0]);

    // Redo
    pDoc->GetUndoManager()->Redo();

    pSparkline = pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 0, 4, 0), pSparkline->getInputRange()[0]);
}

void SparklineTest::testSparklineList()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    pDoc->CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);

    {
        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklines.size());
    }
    pDoc->CreateSparkline(ScAddress(1, 6, 0), pSparklineGroup);
    pDoc->CreateSparkline(ScAddress(2, 6, 0), pSparklineGroup);

    {
        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }

    {
        auto pSparklineGroup2 = std::make_shared<sc::SparklineGroup>();
        pDoc->CreateSparkline(ScAddress(3, 6, 0), pSparklineGroup2);

        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));

        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(2), pSparklineGroups.size());

        auto pSparklines2 = pSparklineList->getSparklinesFor(pSparklineGroup2);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklines2.size());
    }

    pDoc->DeleteSparkline(ScAddress(3, 6, 0));

    {
        auto* pSparklineList = pDoc->GetSparklineList(SCTAB(0));

        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
