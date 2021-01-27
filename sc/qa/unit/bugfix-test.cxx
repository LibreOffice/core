/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstdlib>
#include <string_view>

#include <postit.hxx>
#include <validat.hxx>
#include <tabvwsh.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include "helper/qahelper.hxx"
#include <comphelper/processfactory.hxx>
#include <svx/svdocapt.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <drwlayer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdomeas.hxx>
#include <userdat.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScFiltersTest : public ScBootstrapFixture
{
public:
    ScFiltersTest();

    virtual void setUp() override;

    void testTdf137576_Measureline();
    void testTdf137216_HideCol();
    void testTdf137044_CoverHiddenRows();
    void testTdf137020_FlipVertical();
    void testTdf64229();
    void testTdf36933();
    void testTdf43700();
    void testTdf43534();
    void testTdf91979();
    // void testTdf40110();
    void testTdf98657();
    void testTdf88821();
    void testTdf88821_2();
    void testTdf103960();
    void testRhbz1390776();
    void testTdf104310();
    void testTdf31231();
    void testTdf128951();
    void testTdf129789();
    void testTdf130725();
    void testTdf104502_hiddenColsCountedInPageCount();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testTdf137576_Measureline);
    CPPUNIT_TEST(testTdf137216_HideCol);
    CPPUNIT_TEST(testTdf137044_CoverHiddenRows);
    CPPUNIT_TEST(testTdf137020_FlipVertical);
    CPPUNIT_TEST(testTdf64229);
    CPPUNIT_TEST(testTdf36933);
    CPPUNIT_TEST(testTdf43700);
    CPPUNIT_TEST(testTdf43534);
    CPPUNIT_TEST(testTdf91979);
    // CPPUNIT_TEST(testTdf40110);
    CPPUNIT_TEST(testTdf98657);
    CPPUNIT_TEST(testTdf88821);
    CPPUNIT_TEST(testTdf88821_2);
    CPPUNIT_TEST(testTdf103960);
    CPPUNIT_TEST(testRhbz1390776);
    CPPUNIT_TEST(testTdf104310);
    CPPUNIT_TEST(testTdf31231);
    CPPUNIT_TEST(testTdf128951);
    CPPUNIT_TEST(testTdf129789);
    CPPUNIT_TEST(testTdf130725);
    CPPUNIT_TEST(testTdf104502_hiddenColsCountedInPageCount);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

static void lcl_AssertRectEqualWithTolerance(std::string_view sInfo,
                                             const tools::Rectangle& rExpected,
                                             const tools::Rectangle& rActual,
                                             const sal_Int32 nTolerance)
{
    // Left
    OString sMsg = OString::Concat(sInfo) + " Left expected " + OString::number(rExpected.Left())
                   + " actual "
                   + OString::number(rActual.Left()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(
        sMsg.getStr(), std::abs(rExpected.Left() - rActual.Left()) <= nTolerance);

    // Top
    sMsg = OString::Concat(sInfo) + " Top expected " + OString::number(rExpected.Top()) + " actual "
           + OString::number(rActual.Top()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.Top() - rActual.Top()) <= nTolerance);

    // Width
    sMsg = OString::Concat(sInfo) + " Width expected " + OString::number(rExpected.GetWidth())
           + " actual "
           + OString::number(rActual.GetWidth()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.GetWidth() - rActual.GetWidth()) <= nTolerance);

    // Height
    sMsg = OString::Concat(sInfo) + " Height expected " + OString::number(rExpected.GetHeight())
           + " actual "
           + OString::number(rActual.GetHeight()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.GetHeight() - rActual.GetHeight()) <= nTolerance);
}

static void lcl_AssertPointEqualWithTolerance(std::string_view sInfo, const Point rExpected,
                                              const Point rActual, const sal_Int32 nTolerance)
{
    // X
    OString sMsg = OString::Concat(sInfo) + " X expected " + OString::number(rExpected.X())
                   + " actual "
                   + OString::number(rActual.X()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.X() - rActual.X()) <= nTolerance);
    // Y
    sMsg = OString::Concat(sInfo) + " Y expected " + OString::number(rExpected.Y()) + " actual "
           + OString::number(rActual.Y()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.Y() - rActual.Y()) <= nTolerance);
}

void ScFiltersTest::testTdf137576_Measureline()
{
    // The document contains a vertical measure line, anchored "To Cell (resize with cell)" with
    // length 37mm. Save and reload had resulted in a line of 0mm length.

    // Get document
    ScDocShellRef xDocSh = loadDoc(u"tdf137576_Measureline.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf137576_Measureline.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrMeasureObj* pObj = static_cast<SdrMeasureObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("Load: No measure object", pObj);

    // Check start and end point of measureline
    const Point aStart = pObj->GetPoint(0);
    lcl_AssertPointEqualWithTolerance("Load, start point: ", Point(4800, 1500), aStart, 1);
    const Point aEnd = pObj->GetPoint(1);
    lcl_AssertPointEqualWithTolerance("Load, end point: ", Point(4800, 5200), aEnd, 1);

    // Save and reload
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_ODS);
    ScDocument& rDoc2 = xDocSh->GetDocument();

    // Get shape
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = static_cast<SdrMeasureObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("Reload: No measure object", pObj);

    // Check start and end point of measureline, should be unchanged
    const Point aStart2 = pObj->GetPoint(0);
    lcl_AssertPointEqualWithTolerance("Reload start point: ", Point(4800, 1500), aStart2, 1);
    const Point aEnd2 = pObj->GetPoint(1);
    lcl_AssertPointEqualWithTolerance("Reload end point: ", Point(4800, 5200), aEnd2, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf137216_HideCol()
{
    // The document contains a shape anchored "To Cell (resize with cell)" with start in C3.
    // Error was, that hiding column C did not make the shape invisible.

    // Get document
    ScDocShellRef xDocSh = loadDoc(u"tdf137216_HideCol.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf137216_HideCol.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object found", pObj);

    // Assert object is visible before and invisible after hiding column.
    CPPUNIT_ASSERT_MESSAGE("before column hide: Object should be visible", pObj->IsVisible());
    rDoc.SetColHidden(2, 2, 0, true); // col C in UI = col index 2 to 2.
    CPPUNIT_ASSERT_MESSAGE("after column hide: Object should be invisible", !pObj->IsVisible());
}

void ScFiltersTest::testTdf137044_CoverHiddenRows()
{
    // The document contains a shape anchored "To Cell (resize with cell)" with start in cell A4 and
    // end in cell A7. Row height is 30mm. Hiding rows 5 and 6, then saving and reload had resulted
    // in a wrong end cell offset and thus a wrong height of the shape.

    // Get document
    ScDocShellRef xDocSh = loadDoc(u"tdf137044_CoverHiddenRows.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf137044_CoverHiddenRows.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object", pObj);

    // Get original object values
    tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();
    Point aOriginalEndOffset = ScDrawLayer::GetObjData(pObj)->maEndOffset;
    lcl_AssertRectEqualWithTolerance("Load:", tools::Rectangle(Point(500, 3500), Size(1501, 11001)),
                                     aSnapRectOrig, 1);
    lcl_AssertPointEqualWithTolerance("Load: end offset", Point(2000, 2499), aOriginalEndOffset, 1);

    // Hide rows 5 and 6 in UI = row index 4 to 5.
    rDoc.SetRowHidden(4, 5, 0, true);

    // Save and reload
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_ODS);
    ScDocument& rDoc2 = xDocSh->GetDocument();

    // Get shape
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No object", pObj);

    // Get new values and compare. End offset should be the same, height should be 6000 smaller.
    tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    Point aReloadEndOffset = ScDrawLayer::GetObjData(pObj)->maEndOffset;
    lcl_AssertRectEqualWithTolerance(
        "Reload:", tools::Rectangle(Point(500, 3500), Size(1501, 5001)), aSnapRectReload, 1);
    lcl_AssertPointEqualWithTolerance("Reload: end offset", Point(2000, 2499), aReloadEndOffset, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf137020_FlipVertical()
{
    // Get document
    ScDocShellRef xDocSh = loadDoc(u"tdf137020_FlipVertical.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf137020_FlipVertical.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object", pObj);

    const tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();

    // Vertical mirror on center should not change the snap rect.
    pObj->Mirror(aSnapRectOrig.LeftCenter(), aSnapRectOrig.RightCenter());
    const tools::Rectangle aSnapRectFlip = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Mirror:", aSnapRectOrig, aSnapRectFlip, 1);

    // Save and reload
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_ODS);
    ScDocument& rDoc2 = xDocSh->GetDocument();

    // Get shape
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No object", pObj);

    // Check pos and size of shape again, should be unchanged
    const tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload:", aSnapRectOrig, aSnapRectReload, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf64229()
{
    ScDocShellRef xDocSh = loadDoc(u"fdo64229b.", FORMAT_ODS);

    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo64229b.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo64229b.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf36933()
{
    ScDocShellRef xDocSh = loadDoc(u"fdo36933test.", FORMAT_ODS);

    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo36933test.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo36933test.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf43700()
{
    ScDocShellRef xDocSh = loadDoc(u"fdo43700test.", FORMAT_ODS);

    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo43700test.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo43700test.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf43534()
{
    ScDocShellRef xDocSh = loadDoc(u"fdo43534test.", FORMAT_ODS);

    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo43534test.*", xDocSh.is());
    // ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("fdo43534test.", aCSVFileName);
    // testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf91979()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    Sequence < beans::PropertyValue > args(1);
    args[0].Name = "Hidden";
    args[0].Value <<= true;

    uno::Reference< lang::XComponent > xComponent = xDesktop->loadComponentFromURL(
        "private:factory/scalc",
        "_blank",
        0,
        args);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);
    auto& aViewData = pViewShell->GetViewData();
    auto& rDoc = aViewData.GetDocument();

    // Check coordinates of a distant cell
    Point aPos = aViewData.GetScrPos(MAXCOL - 1, 10000, SC_SPLIT_TOPLEFT, true);
    int nColWidth = ScViewData::ToPixel(rDoc.GetColWidth(0, 0), aViewData.GetPPTX());
    int nRowHeight = ScViewData::ToPixel(rDoc.GetRowHeight(0, 0), aViewData.GetPPTY());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>((MAXCOL - 1) * nColWidth), aPos.getX());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(10000 * nRowHeight), aPos.getY());
}

/*
void ScFiltersTest::testTdf40110()
{
    ScDocShellRef xDocSh = loadDoc("fdo40110test.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo40110test.*", xDocSh.Is());
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath(OUString("fdo40110test."), aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}
*/

void ScFiltersTest::testTdf98657()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf98657.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    xDocSh->DoHardRecalc();

    // this was a NaN before the fix
    CPPUNIT_ASSERT_EQUAL(285.0, rDoc.GetValue(ScAddress(1, 1, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf88821()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf88821.", FORMAT_HTML);
    ScDocument& rDoc = xDocSh->GetDocument();

    // B2 should be 'Périmètre', not 'PÃ©rimÃ¨tre'
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("P\xC3\xA9rim\xC3\xA8tre", RTL_TEXTENCODING_UTF8), rDoc.GetString(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf88821_2()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf88821-2.", FORMAT_HTML);
    ScDocument& rDoc = xDocSh->GetDocument();

    // A2 should be 'ABCabcČŠŽčšž', not 'ABCabcÄŒÅ Å½ÄﾍÅ¡Å¾'
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("ABCabc\xC4\x8C\xC5\xA0\xC5\xBD\xC4\x8D\xC5\xA1\xC5\xBE", RTL_TEXTENCODING_UTF8), rDoc.GetString(0, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf103960()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf103960.", FORMAT_HTML);
    ScDocument& rDoc = xDocSh->GetDocument();

    // A1 should be 'Data', not the entire content of the file
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("Data", RTL_TEXTENCODING_UTF8), rDoc.GetString(0, 0, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testRhbz1390776()
{
    ScDocShellRef xDocSh = loadDoc(u"rhbz1390776.", FORMAT_XLS_XML);
    ScDocument& rDoc = xDocSh->GetDocument();

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0, 27, 0), "SUM(A18:A23)", "Wrong range");

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf104310()
{
    // 1. Test x14 extension
    {
        ScDocShellRef xDocSh = loadDoc(u"tdf104310.", FORMAT_XLSX);
        ScDocument& rDoc = xDocSh->GetDocument();

        const ScValidationData* pData = rDoc.GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        // Make sure the list is correct.
        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));
        CPPUNIT_ASSERT_EQUAL(size_t(5), aList.size());
        for (size_t i = 0; i < 5; ++i)
            CPPUNIT_ASSERT_DOUBLES_EQUAL(double(i + 1), aList[i].GetValue(), 1e-8);

        xDocSh->DoClose();
    }

    // 2. Test x12ac extension
    {
        ScDocShellRef xDocSh = loadDoc(u"tdf104310-2.", FORMAT_XLSX);
        ScDocument& rDoc = xDocSh->GetDocument();

        const ScValidationData* pData = rDoc.GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        // Make sure the list is correct.
        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));
        CPPUNIT_ASSERT_EQUAL(size_t(3), aList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("1"),   aList[0].GetString());
        CPPUNIT_ASSERT_EQUAL(OUString("2,3"), aList[1].GetString());
        CPPUNIT_ASSERT_EQUAL(OUString("4"),   aList[2].GetString());

        xDocSh->DoClose();
    }
}

void ScFiltersTest::testTdf31231()
{
    // We must open it read-write to allow setting modified flag
    ScDocShellRef xDocSh = loadDoc(u"tdf31231.", FORMAT_ODS, true);
    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must be allowed to set modified state", xDocSh->IsEnableSetModified());
    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must not be modified on open", !xDocSh->IsModified());

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf128951()
{
    css::uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // 1. Create spreadsheet
    css::uno::Sequence<css::beans::PropertyValue> aHiddenArgList(1);
    aHiddenArgList[0].Name = "Hidden";
    aHiddenArgList[0].Value <<= true;

    css::uno::Reference<css::lang::XComponent> xComponent
        = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, aHiddenArgList);
    CPPUNIT_ASSERT(xComponent.is());

    // 2. Create a new sheet instance
    css::uno::Reference<css::lang::XMultiServiceFactory> xFac(xComponent,
                                                              css::uno::UNO_QUERY_THROW);
    auto xSheet = xFac->createInstance("com.sun.star.sheet.Spreadsheet");

    // 3. Insert sheet into the spreadsheet (was throwing IllegalArgumentException)
    css::uno::Reference<css::sheet::XSpreadsheetDocument> xDoc(xComponent,
                                                               css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_NO_THROW(xDoc->getSheets()->insertByName("mustNotThrow", css::uno::Any(xSheet)));
}

namespace {

SdrCaptionObj* checkCaption( ScDocument& rDoc, const ScAddress& rAddress, bool bIsShown)
{
    ScPostIt *pNote = rDoc.GetNote(rAddress);

    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(pNote->IsCaptionShown(), bIsShown );

    if (!bIsShown)
        pNote->ShowCaption(rAddress, true);

    SdrCaptionObj* pCaption = pNote->GetCaption();
    CPPUNIT_ASSERT(pCaption);

    return pCaption;
}
}

void ScFiltersTest::testTdf129789()
{

    ScDocShellRef xDocSh = loadDoc(u"tdf129789.", FORMAT_ODS, true);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    {
        // Fill: None
        SdrCaptionObj *const pCaptionB2 = checkCaption(rDoc, ScAddress(1, 1, 0), true);

        const XFillStyleItem& rStyleItemB2 = dynamic_cast<const XFillStyleItem&>(
            pCaptionB2->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rStyleItemB2.GetValue());

        SdrCaptionObj *const pCaptionB9 = checkCaption(rDoc, ScAddress(1, 8, 0), false);

        const XFillStyleItem& rStyleItemB9 = dynamic_cast<const XFillStyleItem&>(
            pCaptionB9->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rStyleItemB9.GetValue());
    }

    {
        // Fill: Solid
        SdrCaptionObj *const pCaptionE2 = checkCaption(rDoc, ScAddress(4, 1, 0), true);

        const XFillStyleItem& rStyleItemE2 = dynamic_cast<const XFillStyleItem&>(
            pCaptionE2->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItemE2.GetValue());

        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
            pCaptionE2->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xffffc0), rColorItem.GetColorValue());

        SdrCaptionObj *const pCaptionE9 = checkCaption(rDoc, ScAddress(4, 8, 0), false);

        const XFillStyleItem& rStyleItemE9 = dynamic_cast<const XFillStyleItem&>(
            pCaptionE9->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItemE9.GetValue());

        const XFillColorItem& rColorItem2 = dynamic_cast<const XFillColorItem&>(
            pCaptionE9->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xffffc0), rColorItem2.GetColorValue());
    }

    {
        // Fill: Gradient
        SdrCaptionObj *const pCaptionH2 = checkCaption(rDoc, ScAddress(7, 1, 0), true);

        const XFillStyleItem& rStyleItemH2 = dynamic_cast<const XFillStyleItem&>(
            pCaptionH2->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, rStyleItemH2.GetValue());
        const XFillGradientItem& rGradientItem = dynamic_cast<const XFillGradientItem&>(
            pCaptionH2->GetMergedItem(XATTR_FILLGRADIENT));
        CPPUNIT_ASSERT_EQUAL(Color(0xdde8cb), rGradientItem.GetGradientValue().GetStartColor());
        CPPUNIT_ASSERT_EQUAL(Color(0xffd7d7), rGradientItem.GetGradientValue().GetEndColor());

        SdrCaptionObj *const pCaptionH9 = checkCaption(rDoc, ScAddress(7, 8, 0), false);

        const XFillStyleItem& rStyleItemH9 = dynamic_cast<const XFillStyleItem&>(
            pCaptionH9->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, rStyleItemH9.GetValue());
        const XFillGradientItem& rGradientItem2 = dynamic_cast<const XFillGradientItem&>(
            pCaptionH2->GetMergedItem(XATTR_FILLGRADIENT));
        CPPUNIT_ASSERT_EQUAL(Color(0xdde8cb), rGradientItem2.GetGradientValue().GetStartColor());
        CPPUNIT_ASSERT_EQUAL(Color(0xffd7d7), rGradientItem2.GetGradientValue().GetEndColor());
    }

    {
        // Fill: Hatch
        SdrCaptionObj *const pCaptionK2 = checkCaption(rDoc, ScAddress(10, 1, 0), true);

        const XFillStyleItem& rStyleItemK2 = dynamic_cast<const XFillStyleItem&>(
            pCaptionK2->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, rStyleItemK2.GetValue());
        const XFillHatchItem& rHatchItem = dynamic_cast<const XFillHatchItem&>(
            pCaptionK2->GetMergedItem(XATTR_FILLHATCH));
        CPPUNIT_ASSERT_EQUAL(Color(0x000080), rHatchItem.GetHatchValue().GetColor());

        SdrCaptionObj *const pCaptionK9 = checkCaption(rDoc, ScAddress(10, 8, 0), false);

        const XFillStyleItem& rStyleItemK9 = dynamic_cast<const XFillStyleItem&>(
            pCaptionK9->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, rStyleItemK9.GetValue());
        const XFillHatchItem& rHatchItem2 = dynamic_cast<const XFillHatchItem&>(
            pCaptionK9->GetMergedItem(XATTR_FILLHATCH));
        CPPUNIT_ASSERT_EQUAL(Color(0x000080), rHatchItem2.GetHatchValue().GetColor());
    }

    {
        // Fill: Bitmap
        SdrCaptionObj *const pCaptionN2 = checkCaption(rDoc, ScAddress(13, 1, 0), true);

        const XFillStyleItem& rStyleItemN2 = dynamic_cast<const XFillStyleItem&>(
            pCaptionN2->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, rStyleItemN2.GetValue());

        SdrCaptionObj *const pCaptionN9 = checkCaption(rDoc, ScAddress(13, 8, 0), false);

        const XFillStyleItem& rStyleItemN9 = dynamic_cast<const XFillStyleItem&>(
            pCaptionN9->GetMergedItem(XATTR_FILLSTYLE));

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, rStyleItemN9.GetValue());
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf130725()
{
    css::uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // 1. Create spreadsheet
    css::uno::Sequence<css::beans::PropertyValue> aHiddenArgList(1);
    aHiddenArgList[0].Name = "Hidden";
    aHiddenArgList[0].Value <<= true;

    css::uno::Reference<css::lang::XComponent> xComponent
        = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, aHiddenArgList);
    css::uno::Reference<css::sheet::XSpreadsheetDocument> xDoc(xComponent,
        css::uno::UNO_QUERY_THROW);

    // 2. Insert 0.0042 into a cell as a formula, to force the conversion from string to double
    css::uno::Reference<css::sheet::XCellRangesAccess> xSheets(xDoc->getSheets(),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XCell> xCell = xSheets->getCellByPosition(0, 0, 0);
    xCell->setFormula("0.0042"); // this assumes en-US locale

    // 3. Check that the value is the nearest double-precision representation of the decimal 0.0042
    //    (it was 0.0042000000000000006 instead of 0.0041999999999999997).
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value must be the nearest representation of decimal 0.0042",
        0.0042, xCell->getValue()); // strict equality
}

void ScFiltersTest::testTdf104502_hiddenColsCountedInPageCount()
{
    ScDocShellRef xShell = loadDoc(u"tdf104502_hiddenColsCountedInPageCount.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocument& rDoc = xShell->GetDocument();

    //Check that hidden columns are not calculated into Print Area
    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    CPPUNIT_ASSERT(rDoc.GetPrintArea(0, nEndCol, nEndRow, false));
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), nEndCol);
    CPPUNIT_ASSERT_EQUAL(SCROW(55), nEndRow);

    xShell->DoClose();
}

ScFiltersTest::ScFiltersTest()
      : ScBootstrapFixture( "sc/qa/unit/data" )
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
