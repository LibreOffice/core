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
#include <tabvwsh.hxx>
#include <cliputil.hxx>
#include <docfunc.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

using namespace css;

class SparklineTest : public ScBootstrapFixture
{
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

public:
    SparklineTest()
        : ScBootstrapFixture("sc/qa/unit/data")
    {
    }

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
        // which is a private symbol to us, gets called
        m_xCalcComponent = getMultiServiceFactory()->createInstance(
            "com.sun.star.comp.Calc.SpreadsheetDocument");
        CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    }

    virtual void tearDown() override
    {
        uno::Reference<lang::XComponent>(m_xCalcComponent, uno::UNO_QUERY_THROW)->dispose();
        test::BootstrapFixture::tearDown();
    }

    void testAddSparkline();
    void testClearContentSprkline();
    void testCopyPasteSparkline();
    void testCutPasteSparkline();
    void testUndoRedoInsertSparkline();
    void testUndoRedoDeleteSparkline();

    CPPUNIT_TEST_SUITE(SparklineTest);
    CPPUNIT_TEST(testAddSparkline);
    CPPUNIT_TEST(testClearContentSprkline);
    CPPUNIT_TEST(testCopyPasteSparkline);
    CPPUNIT_TEST(testCutPasteSparkline);
    CPPUNIT_TEST(testUndoRedoInsertSparkline);
    CPPUNIT_TEST(testUndoRedoDeleteSparkline);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
void insertTestData(ScDocument& rDocument)
{
    rDocument.SetValue(0, 0, 0, 4);
    rDocument.SetValue(0, 1, 0, -2);
    rDocument.SetValue(0, 2, 0, 1);
    rDocument.SetValue(0, 3, 0, -3);
    rDocument.SetValue(0, 4, 0, 5);
    rDocument.SetValue(0, 5, 0, 3);
}

sc::Sparkline* createTestSparkline(ScDocument& rDocument)
{
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    sc::Sparkline* pSparkline = rDocument.CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);
    if (!pSparkline)
        return nullptr;

    insertTestData(rDocument);

    ScRangeList aList;
    aList.push_back(ScRange(0, 0, 0, 0, 5, 0));
    pSparkline->setInputRange(aList);

    return pSparkline;
}

} // end anonymous namespace

void SparklineTest::testAddSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();

    sc::Sparkline* pSparkline = createTestSparkline(rDocument);
    CPPUNIT_ASSERT(pSparkline);

    auto pGetSparkline = rDocument.GetSparkline(ScAddress(0, 6, 0));
    CPPUNIT_ASSERT(pGetSparkline);

    CPPUNIT_ASSERT_EQUAL(pGetSparkline.get(), pSparkline);

    sc::SparklineList* pList = rDocument.GetSparklineList(0);
    CPPUNIT_ASSERT(pList);

    std::vector<std::shared_ptr<sc::Sparkline>> aSparklineVector = pList->getSparklines();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aSparklineVector.size());
    CPPUNIT_ASSERT_EQUAL(aSparklineVector[0].get(), pSparkline);

    xDocSh->DoClose();
}

void SparklineTest::testClearContentSprkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();

    sc::Sparkline* pSparkline = createTestSparkline(rDocument);
    CPPUNIT_ASSERT(pSparkline);

    clearRange(&rDocument, ScRange(0, 6, 0, 0, 6, 0));

    auto pGetSparkline = rDocument.GetSparkline(ScAddress(0, 6, 0));
    CPPUNIT_ASSERT(!pGetSparkline);

    xDocSh->DoClose();
}

void SparklineTest::testCopyPasteSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    auto* pCreatedSparkline = createTestSparkline(rDocument);
    CPPUNIT_ASSERT(pCreatedSparkline);

    ScRange aSourceRange(0, 6, 0, 0, 6, 0);
    auto pSparkline = rDocument.GetSparkline(aSourceRange.aStart);

    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // CopyToClip / CopyFromClip with a aClipDoc
    {
        ScDocument aClipDoc(SCDOCMODE_CLIP);
        copyToClip(&rDocument, aSourceRange, &aClipDoc);

        auto pClipSparkline = aClipDoc.GetSparkline(aSourceRange.aStart);
        CPPUNIT_ASSERT(pClipSparkline);

        ScRange aPasteRange(0, 7, 0, 0, 7, 0);

        ScMarkData aMark(rDocument.GetSheetLimits());
        aMark.SetMarkArea(aPasteRange);
        rDocument.CopyFromClip(aPasteRange, aMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

        auto pSparklineCopy = rDocument.GetSparkline(aPasteRange.aStart);
        CPPUNIT_ASSERT(pSparklineCopy);

        CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(7), pSparklineCopy->getRow());
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

        auto pSparklineCopy = rDocument.GetSparkline(aPasteRange.aStart);
        CPPUNIT_ASSERT(pSparklineCopy);

        CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
        CPPUNIT_ASSERT_EQUAL(SCROW(8), pSparklineCopy->getRow());
    }

    xDocSh->DoClose();
}

void SparklineTest::testCutPasteSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    auto* pCreatedSparkline = createTestSparkline(rDocument);
    CPPUNIT_ASSERT(pCreatedSparkline);

    ScRange aSourceRange(0, 6, 0, 0, 6, 0);
    auto pSparkline = rDocument.GetSparkline(aSourceRange.aStart);

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
    auto pSparklineCopy = rDocument.GetSparkline(aPasteRange.aStart);
    CPPUNIT_ASSERT(pSparklineCopy);

    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparklineCopy->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(7), pSparklineCopy->getRow());

    xDocSh->DoClose();
}

void SparklineTest::testUndoRedoInsertSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    auto& rDocFunc = xDocSh->GetDocFunc();

    // insert test data - A1:A6
    insertTestData(rDocument);

    // Sparkline range
    ScRange aRange(0, 6, 0, 0, 6, 0);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup);

    // Check Sparkline at cell A7 exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Undo
    rDocument.GetUndoManager()->Undo();

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    // Redo
    rDocument.GetUndoManager()->Redo();

    // Check Sparkline at cell A7 exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    xDocSh->DoClose();
}

void SparklineTest::testUndoRedoDeleteSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    auto& rDocFunc = xDocSh->GetDocFunc();

    // Try to delete sparkline that doesn't exist - returns false
    CPPUNIT_ASSERT(!rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // insert test data - A1:A6
    insertTestData(rDocument);

    // Sparkline range
    ScRange aRange(0, 6, 0, 0, 6, 0);

    // Check Sparkline at cell A7 doesn't exists
    auto pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 0, 0, 0, 5, 0), aRange, pSparklineGroup));

    // Check Sparkline at cell A7 exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Delete Sparkline
    CPPUNIT_ASSERT(rDocFunc.DeleteSparkline(ScAddress(0, 6, 0)));

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    // Undo
    rDocument.GetUndoManager()->Undo();

    // Check Sparkline at cell A7 exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSparkline->getColumn());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), pSparkline->getRow());

    // Redo
    rDocument.GetUndoManager()->Redo();

    // Check Sparkline at cell A7 doesn't exists
    pSparkline = rDocument.GetSparkline(aRange.aStart);
    CPPUNIT_ASSERT(!pSparkline);

    CPPUNIT_ASSERT(!rDocument.HasSparkline(aRange.aStart));

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
