/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <docsh.hxx>
#include <docfunc.hxx>
#include <undomanager.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <SparklineList.hxx>
#include <SparklineAttributes.hxx>
#include <ThemeColorChanger.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace css;

/** Test operation for sparklines, sparkline groups and attributes */
class SparklineTest : public ScUcalcTestBase
{
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

CPPUNIT_TEST_FIXTURE(SparklineTest, testAddSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pSparkline = createTestSparkline(*m_pDoc);
    CPPUNIT_ASSERT(pSparkline);

    auto pGetSparkline = m_pDoc->GetSparkline(ScAddress(0, 6, 0));
    CPPUNIT_ASSERT(pGetSparkline);

    CPPUNIT_ASSERT_EQUAL(pGetSparkline.get(), pSparkline);

    sc::SparklineList* pList = m_pDoc->GetSparklineList(SCTAB(0));
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->getSparklineGroups().size());

    auto const& aSparklineVector = pList->getSparklinesFor(pGetSparkline->getSparklineGroup());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aSparklineVector.size());
    CPPUNIT_ASSERT_EQUAL(aSparklineVector[0].get(), pSparkline);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testClearContentSprkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pSparkline = createTestSparkline(*m_pDoc);
    CPPUNIT_ASSERT(pSparkline);

    clearRange(m_pDoc, ScRange(0, 6, 0, 0, 6, 0));

    auto pGetSparkline = m_pDoc->GetSparkline(ScAddress(0, 6, 0));
    CPPUNIT_ASSERT(!pGetSparkline);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testCopyPasteSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto* pCreatedSparkline = createTestSparkline(*m_pDoc);
    CPPUNIT_ASSERT(pCreatedSparkline);

    ScRange aSourceRange(0, 6, 0, 0, 6, 0);
    auto pSparkline = m_pDoc->GetSparkline(aSourceRange.aStart);
    auto const& pOriginalGroup = pSparkline->getSparklineGroup();

    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // CopyToClip / CopyFromClip with a aClipDoc
    {
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        copyToClip(m_pDoc, aSourceRange, &aClipDoc);

        auto pClipSparkline = aClipDoc.GetSparkline(aSourceRange.aStart);
        CPPUNIT_ASSERT(pClipSparkline);

        ScRange aPasteRange(0, 7, 0, 0, 7, 0);

        ScMarkData aMark(m_pDoc->GetSheetLimits());
        aMark.SetMarkArea(aPasteRange);
        m_pDoc->CopyFromClip(aPasteRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

        auto pSparklineCopy = m_pDoc->GetSparkline(aPasteRange.aStart);
        CPPUNIT_ASSERT(pSparklineCopy);

        CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(7), pSparklineCopy->getRow());

        auto const& pCopyGroup = pSparklineCopy->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pOriginalGroup->getID(), pCopyGroup->getID());
        CPPUNIT_ASSERT_EQUAL(pOriginalGroup.get(), pCopyGroup.get());
    }

    // Copy / Paste with a Clim_pDoc
    {
        // Copy
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        copyToClip(m_pDoc, aSourceRange, &aClipDoc);

        // Paste
        ScRange aPasteRange(0, 8, 0, 0, 8, 0);

        pasteFromClip(m_pDoc, aPasteRange, &aClipDoc);

        auto pSparklineCopy = m_pDoc->GetSparkline(aPasteRange.aStart);
        CPPUNIT_ASSERT(pSparklineCopy);

        CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(8), pSparklineCopy->getRow());

        auto const& pCopyGroup = pSparklineCopy->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pOriginalGroup->getID(), pCopyGroup->getID());
        CPPUNIT_ASSERT_EQUAL(pOriginalGroup.get(), pCopyGroup.get());
    }

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testCutPasteSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto* pCreatedSparkline = createTestSparkline(*m_pDoc);
    CPPUNIT_ASSERT(pCreatedSparkline);

    ScRange aSourceRange(0, 6, 0, 0, 6, 0);
    auto pSparkline = m_pDoc->GetSparkline(aSourceRange.aStart);

    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Cut
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    cutToClip(*m_xDocShell, aSourceRange, &aClipDoc, false);

    // Paste
    ScRange aPasteRange(0, 7, 0, 0, 7, 0);
    pasteFromClip(m_pDoc, aPasteRange, &aClipDoc);

    // Check
    auto pSparklineCopy = m_pDoc->GetSparkline(aPasteRange.aStart);
    CPPUNIT_ASSERT(pSparklineCopy);

    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(7), pSparklineCopy->getRow());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoInsertSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScRange aRange(0, 6, 0, 0, 6, 0);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup);

    // Check Sparkline at cell A7 exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    // Check Sparkline at cell A7 exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoDeleteSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScRange aRange(0, 6, 0, 0, 6, 0);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup));

    // Check Sparkline at cell A7 exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Delete Sparkline
    CPPUNIT_ASSERT(rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    // Check Sparkline at cell A7 exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = m_pDoc->GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    CPPUNIT_ASSERT(!m_pDoc->HasSparkline(aRange.aStart));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoDeleteSparklineGroup)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    // Delete Sparkline
    CPPUNIT_ASSERT(rDocFunc.DeleteSparklineGroup(pSparklineGroup, SCTAB(0)));

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    // Check if the sparkline has the input range set
    auto const& pSparkline = m_pDoc->GetSparkline(ScAddress(3, 6, 0));
    ScRangeList rRangeList = pSparkline->getInputRange();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rRangeList.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(3, 0, 0, 3, 5, 0), rRangeList[0]);

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(false, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoClearContentForSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    // Check Sparklines - none should exist
    {
        CPPUNIT_ASSERT(!m_pDoc->HasSparkline(ScAddress(0, 6, 0)));
        CPPUNIT_ASSERT(!m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(!m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT(!m_pDoc->HasSparkline(ScAddress(3, 6, 0)));
    }

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    {
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(0, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 exists
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check D7
        auto pSparkline = m_pDoc->GetSparkline(ScAddress(3, 6, 0));
        CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSparkline->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

        // Check collections
        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(4), pSparklines.size());
    }

    // Clear content of cell D7 - including sparkline
    {
        ScMarkData aMark(m_pDoc->GetSheetLimits());
        aMark.SetMarkArea(ScAddress(3, 6, 0));
        rDocFunc.DeleteContents(aMark, InsertDeleteFlags::CONTENTS, true, true);
    }

    // Check Sparklines
    {
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 is gone
        CPPUNIT_ASSERT(!m_pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check collections
        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    // Check Sparkline
    {
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(0, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 exists - again
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check D7
        auto pSparkline = m_pDoc->GetSparkline(ScAddress(3, 6, 0));
        CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSparkline->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(4), pSparklines.size());
    }

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT(m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        // D7 is gone - again
        CPPUNIT_ASSERT(!m_pDoc->HasSparkline(ScAddress(3, 6, 0)));

        // Check collections
        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoEditSparklineGroup)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    {
        sc::SparklineAttributes& rAttibutes = pSparklineGroup->getAttributes();
        rAttibutes.setType(sc::SparklineType::Column);
        rAttibutes.setColorSeries(model::ComplexColor::createRGB(COL_YELLOW));
        rAttibutes.setColorAxis(model::ComplexColor::createRGB(COL_GREEN));
    }

    m_pDoc->CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);

    sc::SparklineAttributes aNewAttributes;
    aNewAttributes.setType(sc::SparklineType::Stacked);
    aNewAttributes.setColorSeries(model::ComplexColor::createRGB(COL_BLACK));
    aNewAttributes.setColorAxis(model::ComplexColor::createRGB(COL_BLUE));

    sc::SparklineAttributes aInitialAttibutes(pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT(aNewAttributes != aInitialAttibutes);

    CPPUNIT_ASSERT_EQUAL(true, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(false, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW,
                         pSparklineGroup->getAttributes().getColorSeries().getFinalColor());
    CPPUNIT_ASSERT_EQUAL(COL_GREEN,
                         pSparklineGroup->getAttributes().getColorAxis().getFinalColor());

    rDocFunc.ChangeSparklineGroupAttributes(pSparklineGroup, aNewAttributes);

    CPPUNIT_ASSERT_EQUAL(false, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(true, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Stacked, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
                         pSparklineGroup->getAttributes().getColorSeries().getFinalColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pSparklineGroup->getAttributes().getColorAxis().getFinalColor());

    m_pDoc->GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL(true, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(false, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW,
                         pSparklineGroup->getAttributes().getColorSeries().getFinalColor());
    CPPUNIT_ASSERT_EQUAL(COL_GREEN,
                         pSparklineGroup->getAttributes().getColorAxis().getFinalColor());

    m_pDoc->GetUndoManager()->Redo();

    CPPUNIT_ASSERT_EQUAL(false, aInitialAttibutes == pSparklineGroup->getAttributes());
    CPPUNIT_ASSERT_EQUAL(true, aNewAttributes == pSparklineGroup->getAttributes());

    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Stacked, pSparklineGroup->getAttributes().getType());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK,
                         pSparklineGroup->getAttributes().getColorSeries().getFinalColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pSparklineGroup->getAttributes().getColorAxis().getFinalColor());

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoUngroupSparklines)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    CPPUNIT_ASSERT(rDocFunc.UngroupSparklines(ScRange(2, 6, 0, 3, 6, 0)));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoGroupSparklines)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    CPPUNIT_ASSERT(rDocFunc.UngroupSparklines(ScRange(2, 6, 0, 3, 6, 0)));

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }

    {
        auto pGroup = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();
        CPPUNIT_ASSERT(rDocFunc.GroupSparklines(ScRange(1, 6, 0, 3, 6, 0), pGroup));
    }

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT(pGroup1 != pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT_EQUAL(pGroup1, pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT(pGroup3 != pGroup4);
    }

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    // Check Sparklines
    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(1, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(2, 6, 0)));
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(3, 6, 0))); // D7

        auto pGroup1 = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        auto pGroup2 = m_pDoc->GetSparkline(ScAddress(1, 6, 0))->getSparklineGroup();
        auto pGroup3 = m_pDoc->GetSparkline(ScAddress(2, 6, 0))->getSparklineGroup();
        auto pGroup4 = m_pDoc->GetSparkline(ScAddress(3, 6, 0))->getSparklineGroup();

        CPPUNIT_ASSERT(pGroup1 != pGroup2);
        CPPUNIT_ASSERT(pGroup1 != pGroup3);
        CPPUNIT_ASSERT(pGroup1 != pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup3);
        CPPUNIT_ASSERT_EQUAL(pGroup2, pGroup4);
        CPPUNIT_ASSERT_EQUAL(pGroup3, pGroup4);
    }
    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testUndoRedoEditSparkline)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    // Sparkline range
    ScAddress aAddress(0, 6, 0);
    ScRange aRange(aAddress);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = m_pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup));

    // Check Sparkline at cell A7
    pSparkline = m_pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 5, 0), pSparkline->getInputRange()[0]);

    // Change Sparkline
    CPPUNIT_ASSERT(
        rDocFunc.ChangeSparkline(pSparkline, SCTAB(0), ScRangeList(ScRange(0, 1, 0, 0, 4, 0))));

    pSparkline = m_pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 0, 4, 0), pSparkline->getInputRange()[0]);

    // Undo
    m_pDoc->GetUndoManager()->Undo();

    pSparkline = m_pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 5, 0), pSparkline->getInputRange()[0]);

    // Redo
    m_pDoc->GetUndoManager()->Redo();

    pSparkline = m_pDoc->GetSparkline(aAddress);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSparkline->getInputRange().size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 0, 4, 0), pSparkline->getInputRange()[0]);

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testSparklineList)
{
    m_pDoc->InsertTab(0, u"Test"_ustr);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    m_pDoc->CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);

    {
        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklines.size());
    }
    m_pDoc->CreateSparkline(ScAddress(1, 6, 0), pSparklineGroup);
    m_pDoc->CreateSparkline(ScAddress(2, 6, 0), pSparklineGroup);

    {
        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));
        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }

    {
        auto pSparklineGroup2 = std::make_shared<sc::SparklineGroup>();
        m_pDoc->CreateSparkline(ScAddress(3, 6, 0), pSparklineGroup2);

        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));

        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(2), pSparklineGroups.size());

        auto pSparklines2 = pSparklineList->getSparklinesFor(pSparklineGroup2);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklines2.size());
    }

    m_pDoc->DeleteSparkline(ScAddress(3, 6, 0));

    {
        auto* pSparklineList = m_pDoc->GetSparklineList(SCTAB(0));

        auto pSparklineGroups = pSparklineList->getSparklineGroups();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSparklineGroups.size());

        auto pSparklines = pSparklineList->getSparklinesFor(pSparklineGroups[0]);
        CPPUNIT_ASSERT_EQUAL(size_t(3), pSparklines.size());
    }
}

CPPUNIT_TEST_FIXTURE(SparklineTest, testSparklineThemeColorChange)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // insert test data - A1:A6
    insertTestData(*m_pDoc);

    auto& rDocFunc = m_xDocShell->GetDocFunc();

    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);

    // Sparkline range
    ScRange aDataRange(0, 0, 0, 3, 5, 0); //A1:D6
    ScRange aRange(0, 6, 0, 3, 6, 0); // A7:D7

    {
        auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
        sc::SparklineAttributes& rAttibutes = pSparklineGroup->getAttributes();

        model::ComplexColor aSeriesComplexColor;
        aSeriesComplexColor.setThemeColor(model::ThemeColorType::Accent3);
        aSeriesComplexColor.setFinalColor(pTheme->getColorSet()->resolveColor(aSeriesComplexColor));
        rAttibutes.setColorSeries(aSeriesComplexColor);

        model::ComplexColor aAxisComplexColor;
        aAxisComplexColor.setThemeColor(model::ThemeColorType::Accent1);
        aAxisComplexColor.setFinalColor(pTheme->getColorSet()->resolveColor(aAxisComplexColor));
        rAttibutes.setColorAxis(aAxisComplexColor);

        CPPUNIT_ASSERT(rDocFunc.InsertSparklines(aDataRange, aRange, pSparklineGroup));
    }

    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        auto pGroup = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        CPPUNIT_ASSERT(pGroup);
        sc::SparklineAttributes& rAttibutes = pGroup->getAttributes();
        CPPUNIT_ASSERT_EQUAL(Color(0xa33e03), rAttibutes.getColorSeries().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x18a303), rAttibutes.getColorAxis().getFinalColor());
    }

    {
        auto pColorSet = std::make_shared<model::ColorSet>("TestColorScheme");
        pColorSet->add(model::ThemeColorType::Dark1, 0x000000);
        pColorSet->add(model::ThemeColorType::Light1, 0x111111);
        pColorSet->add(model::ThemeColorType::Dark2, 0x222222);
        pColorSet->add(model::ThemeColorType::Light2, 0x333333);
        pColorSet->add(model::ThemeColorType::Accent1, 0x444444);
        pColorSet->add(model::ThemeColorType::Accent2, 0x555555);
        pColorSet->add(model::ThemeColorType::Accent3, 0x666666);
        pColorSet->add(model::ThemeColorType::Accent4, 0x777777);
        pColorSet->add(model::ThemeColorType::Accent5, 0x888888);
        pColorSet->add(model::ThemeColorType::Accent6, 0x999999);
        pColorSet->add(model::ThemeColorType::Hyperlink, 0xaaaaaa);
        pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xbbbbbb);

        sc::ThemeColorChanger aChanger(*m_xDocShell);
        aChanger.apply(pColorSet);
    }

    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        auto pGroup = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        CPPUNIT_ASSERT(pGroup);
        sc::SparklineAttributes& rAttibutes = pGroup->getAttributes();
        CPPUNIT_ASSERT_EQUAL(Color(0x666666), rAttibutes.getColorSeries().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x444444), rAttibutes.getColorAxis().getFinalColor());
    }

    m_pDoc->GetUndoManager()->Undo();

    {
        CPPUNIT_ASSERT_EQUAL(true, m_pDoc->HasSparkline(ScAddress(0, 6, 0))); // A7
        auto pGroup = m_pDoc->GetSparkline(ScAddress(0, 6, 0))->getSparklineGroup();
        CPPUNIT_ASSERT(pGroup);
        sc::SparklineAttributes& rAttibutes = pGroup->getAttributes();
        CPPUNIT_ASSERT_EQUAL(Color(0xa33e03), rAttibutes.getColorSeries().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x18a303), rAttibutes.getColorAxis().getFinalColor());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
