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
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

using namespace css;

class SparklineTest : public ScBootstrapFixture
{
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

    sc::Sparkline* createTestSparkline(ScDocument& rDocument);

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
    void testDeleteSprkline();
    void testCopyPasteSparkline();
    void testCutPasteSparkline();

    CPPUNIT_TEST_SUITE(SparklineTest);
    CPPUNIT_TEST(testAddSparkline);
    CPPUNIT_TEST(testDeleteSprkline);
    CPPUNIT_TEST(testCopyPasteSparkline);
    CPPUNIT_TEST(testCutPasteSparkline);
    CPPUNIT_TEST_SUITE_END();
};

sc::Sparkline* SparklineTest::createTestSparkline(ScDocument& rDocument)
{
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    sc::Sparkline* pSparkline = rDocument.CreateSparkline(ScAddress(0, 6, 0), pSparklineGroup);
    if (!pSparkline)
        return nullptr;

    rDocument.SetValue(0, 0, 0, 4);
    rDocument.SetValue(0, 1, 0, -2);
    rDocument.SetValue(0, 2, 0, 1);
    rDocument.SetValue(0, 3, 0, -3);
    rDocument.SetValue(0, 4, 0, 5);
    rDocument.SetValue(0, 5, 0, 3);

    ScRangeList aList;
    aList.push_back(ScRange(0, 0, 0, 0, 5, 0));
    pSparkline->setInputRange(aList);

    return pSparkline;
}

void SparklineTest::testAddSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();

    auto pSparkline = createTestSparkline(rDocument);
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

void SparklineTest::testDeleteSprkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();

    auto pSparkline = createTestSparkline(rDocument);
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

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
