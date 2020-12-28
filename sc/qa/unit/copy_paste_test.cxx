/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <comphelper/processfactory.hxx>

#include <docsh.hxx>
#include <docfunc.hxx>
#include <cellmergeoption.hxx>
#include <tabvwsh.hxx>
#include <impex.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <userlist.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#include "helper/qahelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScCopyPasteTest : public ScBootstrapFixture
{
public:
    ScCopyPasteTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCopyPasteXLS();
    void testTdf84411();
    void testTdf124565();
    void testTdf126421();
    void testTdf107394();
    void testTdf53431_fillOnAutofilter();
    void testTdf40993_fillMergedCells();
    void testTdf43958_clickSelectOnMergedCells();
    void testTdf88782_autofillLinearNumbersInMergedCells();
    void tdf137621_autofillMergedBool();
    void tdf137205_autofillDatesInMergedCells();
    void tdf137653_137654_autofillUserlist();
    void tdf113500_autofillMixed();
    void tdf137625_autofillMergedUserlist();
    void tdf137624_autofillMergedMixed();

    CPPUNIT_TEST_SUITE(ScCopyPasteTest);
    CPPUNIT_TEST(testCopyPasteXLS);
    CPPUNIT_TEST(testTdf84411);
    CPPUNIT_TEST(testTdf124565);
    CPPUNIT_TEST(testTdf126421);
    CPPUNIT_TEST(testTdf107394);
    CPPUNIT_TEST(testTdf53431_fillOnAutofilter);
    CPPUNIT_TEST(testTdf40993_fillMergedCells);
    CPPUNIT_TEST(testTdf43958_clickSelectOnMergedCells);
    CPPUNIT_TEST(testTdf88782_autofillLinearNumbersInMergedCells);
    CPPUNIT_TEST(tdf137621_autofillMergedBool);
    CPPUNIT_TEST(tdf137205_autofillDatesInMergedCells);
    CPPUNIT_TEST(tdf137653_137654_autofillUserlist);
    CPPUNIT_TEST(tdf113500_autofillMixed);
    CPPUNIT_TEST(tdf137625_autofillMergedUserlist);
    CPPUNIT_TEST(tdf137624_autofillMergedMixed);
    CPPUNIT_TEST_SUITE_END();

private:

    ScDocShellRef loadDocAndSetupModelViewController(std::u16string_view rFileName, sal_Int32 nFormat, bool bReadWrite);
    void addToUserList(const OUString& rStr);
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

// tdf#83366
void ScCopyPasteTest::testCopyPasteXLS()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT( xDesktop.is() );

    // create a frame
    Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame( "_blank", 0 );
    CPPUNIT_ASSERT( xTargetFrame.is() );

    // 1. Open the document
    ScDocShellRef xDocSh = loadDoc(u"chartx2.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chartx2.xls.", xDocSh.is());

    uno::Reference< frame::XModel2 > xModel2 ( xDocSh->GetModel(), UNO_QUERY );
    CPPUNIT_ASSERT( xModel2.is() );

    Reference< frame::XController2 > xController = xModel2->createDefaultViewController( xTargetFrame );
    CPPUNIT_ASSERT( xController.is() );

    // introduce model/view/controller to each other
    xController->attachModel( xModel2.get() );
    xModel2->connectController( xController.get() );
    xTargetFrame->setComponent( xController->getComponentWindow(), xController.get() );
    xController->attachFrame( xTargetFrame );
    xModel2->setCurrentController( xController.get() );

    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // 2. Highlight B2:C5
    ScRange aSrcRange;
    ScRefFlags nRes = aSrcRange.Parse("B2:C5", rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));

    ScMarkData aMark(rDoc.GetSheetLimits());
    aMark.SetMarkArea(aSrcRange);

    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);

    // 3. Copy
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // 4. Close the document (Ctrl-W)
    xDocSh->DoClose();

    // 5. Create a new Spreadsheet
    Sequence < beans::PropertyValue > args(1);
    args[0].Name = "Hidden";
    args[0].Value <<= true;

    uno::Reference< lang::XComponent > xComponent = xDesktop->loadComponentFromURL(
            "private:factory/scalc",
            "_blank",
            0,
            args );
    CPPUNIT_ASSERT( xComponent.is() );

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    // Get the document controller
    pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // 6. Paste
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

    xComponent->dispose();
}

namespace {

ScMarkData::MarkedTabsType TabsInRange(const ScRange& r)
{
    ScMarkData::MarkedTabsType aResult;
    for (SCTAB i = r.aStart.Tab(); i <= r.aEnd.Tab(); ++i)
        aResult.insert(i);
    return aResult;
}

void lcl_copy( const OUString& rSrcRange, const OUString& rDstRange, ScDocument& rDoc, ScTabViewShell* pViewShell )
{
    ScDocument aClipDoc(SCDOCMODE_CLIP);

    // 1. Copy
    ScRange aSrcRange;
    ScRefFlags nRes = aSrcRange.Parse(rSrcRange, rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
    pViewShell->GetViewData().GetMarkData().SetSelectedTabs(TabsInRange(aSrcRange));
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // 2. Paste
    ScRange aDstRange;
    nRes = aDstRange.Parse(rDstRange, rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aDstRange);
    pViewShell->GetViewData().GetMarkData().SetSelectedTabs(TabsInRange(aDstRange));
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);
}

} // anonymous namespace

void ScCopyPasteTest::testTdf84411()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT( xDesktop.is() );

    // create a frame
    Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame( "_blank", 0 );
    CPPUNIT_ASSERT( xTargetFrame.is() );

    // 1. Create spreadsheet
    uno::Sequence< beans::PropertyValue > aEmptyArgList;
    uno::Reference< lang::XComponent > xComponent = xDesktop->loadComponentFromURL(
            "private:factory/scalc",
            "_blank",
            0,
            aEmptyArgList );
    CPPUNIT_ASSERT( xComponent.is() );

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    uno::Reference< frame::XModel2 > xModel2 ( xDocSh->GetModel(), UNO_QUERY );
    CPPUNIT_ASSERT( xModel2.is() );

    Reference< frame::XController2 > xController = xModel2->createDefaultViewController( xTargetFrame );
    CPPUNIT_ASSERT( xController.is() );

    // introduce model/view/controller to each other
    xController->attachModel( xModel2.get() );
    xModel2->connectController( xController.get() );
    xTargetFrame->setComponent( xController->getComponentWindow(), xController.get() );
    xController->attachFrame( xTargetFrame );
    xModel2->setCurrentController( xController.get() );

    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);


    // 2. Setup data and formulas
    for (unsigned int r = 0; r <= 4991; ++r)
        for (unsigned int c = 0; c <= 14; ++c)
            rDoc.SetValue( ScAddress(c,r,0), (r+1)*(c+1) );

    rDoc.SetString(ScAddress(15,10000,0), "=AVERAGE(A10001:O10001)");
    rDoc.SetString(ScAddress(16,10000,0), "=MIN(A10001:O10001)");
    rDoc.SetString(ScAddress(17,10000,0), "=MAX(A10001:O10001)");

    lcl_copy("P10001:R10001", "P10002:R12500", rDoc, pViewShell);


    // 3. Disable OpenCL
    ScModelObj* pModel = comphelper::getUnoTunnelImplementation<ScModelObj>(pFoundShell->GetModel());
    CPPUNIT_ASSERT(pModel != nullptr);
    bool bOpenCLState = ScCalcConfig::isOpenCLEnabled();
    pModel->enableOpenCL(false);
    CPPUNIT_ASSERT(!ScCalcConfig::isOpenCLEnabled() || ScCalcConfig::getForceCalculationType() == ForceCalculationOpenCL);
    pModel->enableAutomaticCalculation(true);


    // 4. Copy and Paste
    lcl_copy("A1:O2500", "A10001:O12500", rDoc, pViewShell);

    lcl_copy("A2501:O5000", "A12501:O15000", rDoc, pViewShell);

    lcl_copy("P10001:R10001", "P12501:R15000", rDoc, pViewShell);


    // 5. Close the document (Ctrl-W)
    pModel->enableOpenCL(bOpenCLState);
    xComponent->dispose();
}

void ScCopyPasteTest::testTdf124565()
{
    // Create new document
    ScDocShell* xDocSh = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    xDocSh->DoInitNew();

    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT( xDesktop.is() );

    Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame( "_blank", 0 );
    CPPUNIT_ASSERT( xTargetFrame.is() );

    uno::Reference< frame::XModel2 > xModel2 ( xDocSh->GetModel(), UNO_QUERY );
    CPPUNIT_ASSERT( xModel2.is() );

    Reference< frame::XController2 > xController = xModel2->createDefaultViewController( xTargetFrame );
    CPPUNIT_ASSERT( xController.is() );

    // introduce model/view/controller to each other
    xController->attachModel( xModel2.get() );
    xModel2->connectController( xController.get() );
    xTargetFrame->setComponent( xController->getComponentWindow(), xController.get() );
    xController->attachFrame( xTargetFrame );
    xModel2->setCurrentController( xController.get() );

    ScDocument& rDoc = xDocSh->GetDocument();
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // Set content and height of first row
    rDoc.SetString(ScAddress(0, 0, 0), "Test");
    rDoc.SetRowHeight(0, 0, 500);
    rDoc.SetManualHeight(0, 0, 0, true);

    // Copy first row
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aCopyRange(0, 0, 0, aClipDoc.MaxCol(), 0, 0);
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aCopyRange);
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // Paste to second row
    SCTAB nTab = 0;
    SCCOL nCol = 0;
    SCROW nRow = 1;

    ScRange aPasteRange(nCol, nRow, nTab, aClipDoc.MaxCol(), nRow, nTab);
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aPasteRange);
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

    // Copy-pasted?
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String was not pasted!", OUString("Test"), rDoc.GetString(nCol, nRow, nTab));

    // And height same as in source?
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row#2 height is invalid!", sal_uInt16(500), rDoc.GetRowHeight(nRow, nTab));

    CPPUNIT_ASSERT_MESSAGE("Row#2 must be manual height!", rDoc.IsManualRowHeight(nRow, nTab));

    xDocSh->DoClose();
}

void ScCopyPasteTest::testTdf126421()
{
    uno::Reference<frame::XDesktop2> xDesktop
        = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // create a frame
    Reference<frame::XFrame> xTargetFrame = xDesktop->findFrame("_blank", 0);
    CPPUNIT_ASSERT(xTargetFrame.is());

    // 1. Create spreadsheet
    uno::Sequence<beans::PropertyValue> aEmptyArgList;
    uno::Reference<lang::XComponent> xComponent
        = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, aEmptyArgList);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    uno::Reference<frame::XModel2> xModel2(xDocSh->GetModel(), UNO_QUERY);
    CPPUNIT_ASSERT(xModel2.is());

    Reference<frame::XController2> xController = xModel2->createDefaultViewController(xTargetFrame);
    CPPUNIT_ASSERT(xController.is());

    // introduce model/view/controller to each other
    xController->attachModel(xModel2.get());
    xModel2->connectController(xController.get());
    xTargetFrame->setComponent(xController->getComponentWindow(), xController.get());
    xController->attachFrame(xTargetFrame);
    xModel2->setCurrentController(xController.get());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // 2. Setup data
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 1024; ++c)
            rDoc.SetValue(c, r, 0, (c + 1) * 100 + (r + 1));

    const SCTAB n2ndTab = rDoc.GetMaxTableNumber() + 1;
    rDoc.MakeTable(n2ndTab);
    const auto aTabNames = rDoc.GetAllTableNames();

    lcl_copy(aTabNames[0] + ".A1:AMJ2", aTabNames[n2ndTab] + ".A1:AMJ2", rDoc, pViewShell);

    // 3. Check all cells in destination table
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 1024; ++c)
            CPPUNIT_ASSERT_EQUAL(double((c + 1) * 100 + (r + 1)), rDoc.GetValue(c, r, n2ndTab));

    xDocSh->DoClose();
}

void ScCopyPasteTest::testTdf107394()
{
    uno::Reference<frame::XDesktop2> xDesktop
        = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    uno::Reference<lang::XComponent> xComponent
        = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, {});
    CPPUNIT_ASSERT(xComponent.is());

    auto pModelObj = dynamic_cast<ScModelObj*>(xComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    CPPUNIT_ASSERT(pModelObj->GetDocument());

    ScDocument& rDoc = *pModelObj->GetDocument();

    sal_uInt16 nFirstRowHeight = rDoc.GetRowHeight(0, 0);
    sal_uInt16 nSecondRowHeight = rDoc.GetRowHeight(1, 0);
    CPPUNIT_ASSERT_EQUAL(nFirstRowHeight, nSecondRowHeight);

    // Import values to A1:A2.
    ScImportExport aObj(rDoc, ScAddress(0,0,0));
    aObj.SetImportBroadcast(true);

    SvMemoryStream aStream;
    aStream.WriteOString("<pre>First\nVery long sentence.</pre>");
    aStream.Seek(0);
    CPPUNIT_ASSERT(aObj.ImportStream(aStream, OUString(), SotClipboardFormatId::HTML));

    CPPUNIT_ASSERT_EQUAL(OUString("First"), rDoc.GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Very long sentence."), rDoc.GetString(ScAddress(0,1,0)));

    nFirstRowHeight = rDoc.GetRowHeight(0, 0);
    nSecondRowHeight = rDoc.GetRowHeight(1, 0);
    CPPUNIT_ASSERT_GREATER(nFirstRowHeight, nSecondRowHeight);

    // Undo, and check the result.
    SfxUndoManager* pUndoMgr = rDoc.GetUndoManager();
    CPPUNIT_ASSERT_MESSAGE("Failed to get the undo manager.", pUndoMgr);
    pUndoMgr->Undo();

    CPPUNIT_ASSERT(rDoc.GetString(ScAddress(0,0,0)).isEmpty());
    CPPUNIT_ASSERT(rDoc.GetString(ScAddress(0,1,0)).isEmpty());

    nFirstRowHeight = rDoc.GetRowHeight(0, 0);
    nSecondRowHeight = rDoc.GetRowHeight(1, 0);
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: 256
    // - Actual  : 477
    // i.e. the increased height of the second row remained after undo.
    CPPUNIT_ASSERT_EQUAL(nFirstRowHeight, nSecondRowHeight);

    xComponent->dispose();
}

static ScMF lcl_getMergeFlagOfCell(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const SfxPoolItem& rPoolItem = rDoc.GetPattern(nCol, nRow, nTab)->GetItem(ATTR_MERGE_FLAG);
    const ScMergeFlagAttr& rMergeFlag = static_cast<const ScMergeFlagAttr&>(rPoolItem);
    return rMergeFlag.GetValue();
}

static ScAddress lcl_getMergeSizeOfCell(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    const SfxPoolItem& rPoolItem = rDoc.GetPattern(nCol, nRow, nTab)->GetItem(ATTR_MERGE);
    const ScMergeAttr& rMerge = static_cast<const ScMergeAttr&>(rPoolItem);
    return ScAddress(rMerge.GetColMerge(), rMerge.GetRowMerge(), nTab);
}

ScDocShellRef ScCopyPasteTest::loadDocAndSetupModelViewController(std::u16string_view rFileName, sal_Int32 nFormat, bool bReadWrite)
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // create a frame
    Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame("_blank", 0);
    CPPUNIT_ASSERT(xTargetFrame.is());

    // 1. Open the document
    ScDocShellRef xDocSh = loadDoc(rFileName, nFormat, bReadWrite);
    CPPUNIT_ASSERT_MESSAGE(OString("Failed to load " + OUStringToOString(rFileName, RTL_TEXTENCODING_UTF8)).getStr(), xDocSh.is());

    uno::Reference< frame::XModel2 > xModel2(xDocSh->GetModel(), UNO_QUERY);
    CPPUNIT_ASSERT(xModel2.is());

    Reference< frame::XController2 > xController = xModel2->createDefaultViewController(xTargetFrame);
    CPPUNIT_ASSERT(xController.is());

    // introduce model/view/controller to each other
    xController->attachModel(xModel2.get());
    xModel2->connectController(xController.get());
    xTargetFrame->setComponent(xController->getComponentWindow(), xController.get());
    xController->attachFrame(xTargetFrame);
    xModel2->setCurrentController(xController.get());

    return xDocSh;
}

void ScCopyPasteTest::testTdf53431_fillOnAutofilter()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf53431_autofilterFilldown.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    //Fill should not clone Autofilter button
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(1, 1, 0, 2, 4, 0));
    pView->FillSimple(FILL_TO_BOTTOM);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(rDoc, 1, 4, 0) & ScMF::Auto));

    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(1, 1, 0, 4, 4, 0));
    pView->FillSimple(FILL_TO_RIGHT);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(rDoc, 4, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(rDoc, 1, 4, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(rDoc, 4, 4, 0) & ScMF::Auto));

    //Fill should not delete Autofilter buttons
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 0, 0, 2, 4, 0));
    pView->FillSimple(FILL_TO_TOP);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(rDoc, 1, 0, 0) & ScMF::Auto));

    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 0, 0, 4, 4, 0));
    pView->FillSimple(FILL_TO_LEFT);
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 1, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT((lcl_getMergeFlagOfCell(rDoc, 2, 1, 0) & ScMF::Auto));
    CPPUNIT_ASSERT(!(lcl_getMergeFlagOfCell(rDoc, 0, 1, 0) & ScMF::Auto));
}

void ScCopyPasteTest::testTdf40993_fillMergedCells()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf40993_fillMergedCells.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // check content of the merged cell H11:I11
    CPPUNIT_ASSERT_EQUAL(OUString("1.5"), rDoc.GetString(ScAddress(7, 10, 0)));

    // fill operation on the merged cell should clone ATTR_MERGE and ATTR_MERGE_FLAG
    // (as long as ATTR_MERGE_FLAG has only ScMF::Hor or ScMF::Ver)
    //
    // select merged cell
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(7, 10, 0, 8, 10, 0));
    // copy its content in the next ten rows
    pView->FillAuto(FILL_TO_BOTTOM, 7, 10, 8, 10, 10);
    for (int i = 7; i < 9; i++)
    {
        ScMF nOriginFlag = lcl_getMergeFlagOfCell(rDoc, i, 10, 0);
        ScAddress aOriginMerge = lcl_getMergeSizeOfCell(rDoc, i, 10, 0);

        // ATTR_MERGE_FLAG: top left cell is NONE, the other cell shows horizontal overlapping
        CPPUNIT_ASSERT_EQUAL(i == 7 ? ScMF::NONE : ScMF::Hor, nOriginFlag);

        // ATTR_MERGE: top left cell contains the size of the
        // merged area (2:1), the other cell doesn't
        CPPUNIT_ASSERT_EQUAL(i == 7 ? ScAddress(2, 1, 0): ScAddress(0, 0, 0), aOriginMerge);

        for (int j = 11; j < 21; j++)
        {
            // check copying of ATTR_MERGE and ATTR_MERGE_FLAG
            CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(rDoc, i, j, 0), nOriginFlag);
            CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, i, j, 0), aOriginMerge);
        }
    }

    CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(rDoc, 7, 21, 0),
                    lcl_getMergeFlagOfCell(rDoc, 7, 10, 0));
    CPPUNIT_ASSERT(lcl_getMergeSizeOfCell(rDoc, 7, 21, 0) !=
                    lcl_getMergeSizeOfCell(rDoc, 7, 10, 0));
    CPPUNIT_ASSERT(lcl_getMergeFlagOfCell(rDoc, 8, 21, 0) !=
                    lcl_getMergeFlagOfCell(rDoc, 8, 10, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, 8, 21, 0),
                    lcl_getMergeSizeOfCell(rDoc, 8, 10, 0));

    // area A6:E9 with various merged cells copied vertically and horizontally
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 5, 0, 4, 8, 0));
    pView->FillAuto(FILL_TO_BOTTOM, 0, 5, 4, 8, 12);
    ScDocShell::GetViewData()->GetMarkData().SetMarkArea(ScRange(0, 5, 0, 4, 8, 0));
    pView->FillAuto(FILL_TO_RIGHT, 0, 5, 4, 8, 10);
    for (int i = 0; i < 5; i++)
    {
        for (int j = 5; j < 9; j++)
        {
            ScMF nOriginFlag = lcl_getMergeFlagOfCell(rDoc, i, j, 0);
            ScAddress aOriginMerge = lcl_getMergeSizeOfCell(rDoc, i, j, 0);
            // copies contain the same ATTR_MERGE and ATTR_MERGE_FLAG
            for (int k = 0; k < 12; k += 4)
            {
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(rDoc, i, j + k, 0), nOriginFlag);
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, i, j + k, 0), aOriginMerge);
            }
            for (int k = 0; k < 10; k += 5)
            {
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeFlagOfCell(rDoc, i + k, j, 0), nOriginFlag);
                CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, i + k, j, 0), aOriginMerge);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, 1, 5, 0), ScAddress(2, 4, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, 0, 5, 0), ScAddress(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, 4, 6, 0), ScAddress(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(lcl_getMergeSizeOfCell(rDoc, 3, 5, 0), ScAddress(2, 1, 0));
}

static void lcl_clickAndCheckCurrentArea(SCCOL nCol, SCROW nRow, SCCOL nCol2, SCROW nRow2)
{
    ScRange aRange;
    ScDocShell::GetViewData()->SetCurX(nCol);
    ScDocShell::GetViewData()->SetCurY(nRow);
    ScDocShell::GetViewData()->GetSimpleArea(aRange);
    CPPUNIT_ASSERT_EQUAL(aRange, ScRange(nCol, nRow, 0, nCol2, nRow2, 0));
}

void ScCopyPasteTest::testTdf43958_clickSelectOnMergedCells()
{
    loadDocAndSetupModelViewController(u"tdf40993_fillMergedCells.", FORMAT_ODS, true);

    // select cell (e.g. by clicking on it) and check what is selected [but not marked]:
    // if it is the top left cell of a merged area, the selection is enlarged to the area
    lcl_clickAndCheckCurrentArea(1, 5, 2, 8);    // B6 -> B6:C9
    lcl_clickAndCheckCurrentArea(0, 5, 0, 6);    // A6 -> A6:A7
    lcl_clickAndCheckCurrentArea(3, 5, 4, 5);    // D6 -> D6:E6
    lcl_clickAndCheckCurrentArea(4, 6, 4, 7);    // D7 -> D6:D7
    lcl_clickAndCheckCurrentArea(7, 10, 8, 10);  // H11 -> H11:I11
    lcl_clickAndCheckCurrentArea(7, 13, 8, 13);  // H14 -> H14:I14

    // otherwise it remains the same
    lcl_clickAndCheckCurrentArea(0, 7, 0, 7);    // A8
    lcl_clickAndCheckCurrentArea(0, 8, 0, 8);    // A9
    lcl_clickAndCheckCurrentArea(2, 6, 2, 6);    // C7
    lcl_clickAndCheckCurrentArea(2, 7, 2, 7);    // C8
    lcl_clickAndCheckCurrentArea(2, 8, 2, 8);    // C9
}

void ScCopyPasteTest::testTdf88782_autofillLinearNumbersInMergedCells()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf88782_AutofillLinearNumbersInMergedCells.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // merge the yellow cells
    ScCellMergeOption aMergeOptions(9, 11, 10, 13);     //J12:K14
    aMergeOptions.maTabs.insert(0);
    xDocSh->GetDocFunc().MergeCells(aMergeOptions, false, true, true, false);

    // fillauto numbers, these areas contain mostly merged cells
    pView->FillAuto(FILL_TO_BOTTOM, 1, 8, 3, 14, 7);    // B9:D15 ->  B9:D22
    pView->FillAuto(FILL_TO_BOTTOM, 5, 8, 7, 17, 10);   // F9:H18 ->  F9:H28
    pView->FillAuto(FILL_TO_BOTTOM, 9, 8, 10, 13, 6);   // J9:K14 ->  J9:K20
    pView->FillAuto(FILL_TO_RIGHT, 9, 30, 16, 35, 8);   //J31:Q36 -> J31:Y36
    pView->FillAuto(FILL_TO_LEFT, 9, 30, 16, 35, 8);    //J31:Q36 -> B31:Q36

    // compare the results of fill-down with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    // the test file have instructions / explanations, so that is easy to understand
    for (int nCol = 1; nCol <= 10; nCol++)
    {
        for (int nRow = 8; nRow <= 27; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol + 22, nRow, 0));
            double* pValue1 = rDoc.GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = rDoc.GetValueCell(ScAddress(nCol + 22, nRow, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);   //cells with number value
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);     //empty cells
        }
    }

    // compare the results of fill-right and left with the reference stored in the test file
    for (int nCol = 1; nCol <= 24; nCol++)
    {
        for (int nRow = 30; nRow <= 35; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 16, 0));
            double* pValue1 = rDoc.GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = rDoc.GetValueCell(ScAddress(nCol, nRow + 16, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);
        }
    }
}

void ScCopyPasteTest::tdf137621_autofillMergedBool()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf137621_autofillMergedBool.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // fillauto booleans, these areas contain only merged cells
    pView->FillAuto(FILL_TO_RIGHT, 0, 4, 3, 5, 8);   //A5:D6

    // compare the results of fill-right with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 4; nCol <= 11; nCol++)
    {
        for (int nRow = 4; nRow <= 5; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 3, 0));
            double* pValue1 = rDoc.GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = rDoc.GetValueCell(ScAddress(nCol, nRow + 3, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);   //cells with boolean value
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);     //empty cells
        }
    }
}

void ScCopyPasteTest::tdf137205_autofillDatesInMergedCells()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf137205_AutofillDatesInMergedCells.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // fillauto dates, this areas contain only merged cells
    pView->FillAuto(FILL_TO_RIGHT, 1, 5, 4, 7, 8);   //B6:E8

    // compare the results of fill-right with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 5; nCol <= 12; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 5, 0));
            double* pValue1 = rDoc.GetValueCell(ScAddress(nCol, nRow, 0));
            double* pValue2 = rDoc.GetValueCell(ScAddress(nCol, nRow + 5, 0));

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            if (pValue2 != nullptr)
                CPPUNIT_ASSERT_EQUAL(*pValue1, *pValue2);   //cells with number value
            else
                CPPUNIT_ASSERT_EQUAL(pValue1, pValue2);     //empty cells
        }
    }
}

void ScCopyPasteTest::addToUserList(const OUString& rStr)
{
    ScUserListData* aListData = new ScUserListData(rStr);
    ScGlobal::GetUserList()->push_back(aListData);
}

void ScCopyPasteTest::tdf137653_137654_autofillUserlist()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf137653_137654_autofillUserlist.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // delete every userlist to make sure there won't be any string that is in 2 different userlist
    ScGlobal::GetUserList()->clear();
    addToUserList({ "January,February,March,April,May,June,July,August,September,October,November,December" });
    const ScUserListData* pListData = ScGlobal::GetUserList()->GetData("January");
    sal_uInt16 nIdx1 = 0, nIdx2 = 0;
    bool bHasIdx1, bHasIdx2;
    bool bMatchCase = false;

    // fillauto userlist, these areas contain only merged cells
    pView->FillAuto(FILL_TO_RIGHT, 4, 5, 6, 7, 3);   //E6:G8
    pView->FillAuto(FILL_TO_LEFT, 4, 5, 6, 7, 3);    //E6:G8
    pView->FillAuto(FILL_TO_BOTTOM, 1, 18, 3, 19, 2); //B19:D20
    pView->FillAuto(FILL_TO_TOP, 1, 18, 3, 19, 2);    //B19:D20

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 1; nCol <= 9; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 4, 0));
            bHasIdx1 = pListData->GetSubIndex(rDoc.GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(rDoc.GetString(nCol, nRow + 4, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT(bHasIdx1 && bHasIdx2);
            CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   // userlist index value of cells
        }
    }

    // compare the results of fill-up / -down
    for (int nCol = 1; nCol <= 3; nCol++)
    {
        for (int nRow = 16; nRow <= 21; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol + 4, nRow, 0));
            bHasIdx1 = pListData->GetSubIndex(rDoc.GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(rDoc.GetString(nCol + 4, nRow, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT(bHasIdx1 && bHasIdx2);
            CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   // userlist index value of cells
        }
    }
}

void ScCopyPasteTest::tdf113500_autofillMixed()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf113500_autofillMixed.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // fillauto userlist, these areas contain only merged cells
    pView->FillAuto(FILL_TO_RIGHT, 4, 5, 6, 7, 3);   //E6:G8
    pView->FillAuto(FILL_TO_LEFT, 4, 5, 6, 7, 3);    //E6:G8
    pView->FillAuto(FILL_TO_BOTTOM, 1, 18, 3, 19, 2); //B19:D20
    pView->FillAuto(FILL_TO_TOP, 1, 18, 3, 19, 2);    //B19:D20

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    // do not check the 3. row: a1,b2,a3. It is another bug to fix
    for (int nCol = 1; nCol <= 9; nCol++)
    {
        for (int nRow = 5; nRow <= 6; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 4, 0));
            OUString aStr1 = rDoc.GetString(nCol, nRow, 0);
            OUString aStr2 = rDoc.GetString(nCol, nRow + 4, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }

    // compare the results of fill-up / -down
    // do not check the 2. column: 1st,3st. It is another bug to fix
    for (int nCol = 1; nCol <= 3; nCol+=2)
    {
        for (int nRow = 16; nRow <= 21; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol + 4, nRow, 0));
            OUString aStr1 = rDoc.GetString(nCol, nRow, 0);
            OUString aStr2 = rDoc.GetString(nCol + 4, nRow, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }
}

void ScCopyPasteTest::tdf137625_autofillMergedUserlist()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf137625_autofillMergedUserlist.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // delete every userlist to make sure there won't be any string that is in 2 different userlist
    ScGlobal::GetUserList()->clear();
    addToUserList({ "January,February,March,April,May,June,July,August,September,October,November,December" });
    const ScUserListData* pListData = ScGlobal::GetUserList()->GetData("January");
    sal_uInt16 nIdx1 = 0, nIdx2 = 0;
    bool bHasIdx1, bHasIdx2;
    bool bMatchCase = false;

    // fillauto userlist, these areas contain only merged cells
    pView->FillAuto(FILL_TO_RIGHT, 7, 5, 12, 7, 6);   //H6:M8
    pView->FillAuto(FILL_TO_LEFT, 7, 5, 12, 7, 6);    //H6:M8
    pView->FillAuto(FILL_TO_BOTTOM, 1, 20, 3, 23, 4); //B21:D24
    pView->FillAuto(FILL_TO_TOP, 1, 20, 3, 23, 4);    //B21:D24

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 1; nCol <= 18; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 4, 0));
            bHasIdx1 = pListData->GetSubIndex(rDoc.GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(rDoc.GetString(nCol, nRow + 4, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(bHasIdx1, bHasIdx2);
            if (bHasIdx1)
                CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   //cells with userlist value
        }
    }

    // compare the results of fill-up / -down
    for (int nCol = 1; nCol <= 3; nCol++)
    {
        for (int nRow = 16; nRow <= 27; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol + 4, nRow, 0));
            bHasIdx1 = pListData->GetSubIndex(rDoc.GetString(nCol, nRow, 0), nIdx1, bMatchCase);
            bHasIdx2 = pListData->GetSubIndex(rDoc.GetString(nCol + 4, nRow, 0), nIdx2, bMatchCase);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(bHasIdx1, bHasIdx2);
            if (bHasIdx1)
                CPPUNIT_ASSERT_EQUAL(nIdx1, nIdx2);   //cells with userlist value
        }
    }
}

void ScCopyPasteTest::tdf137624_autofillMergedMixed()
{
    ScDocShellRef xDocSh = loadDocAndSetupModelViewController(u"tdf137624_autofillMergedMixed.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pView = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pView != nullptr);

    // add 1aa,2bb,3cc,4dd,5ee,6ff to userlist, to test that autofill won't confuse it with 1aa,3aa
    // delete every userlist to make sure there won't be any string that is in 2 different userlist
    ScGlobal::GetUserList()->clear();
    addToUserList({ "1aa,2bb,3cc,4dd,5ee,6ff" });

    // fillauto mixed (string + number), these areas contain only merged cells
    pView->FillAuto(FILL_TO_RIGHT, 7, 5, 12, 7, 6);   //H6:M8
    pView->FillAuto(FILL_TO_LEFT, 7, 5, 12, 7, 6);    //H6:M8
    pView->FillAuto(FILL_TO_BOTTOM, 1, 20, 3, 23, 4); //B21:D24
    pView->FillAuto(FILL_TO_TOP, 1, 20, 3, 23, 4);    //B21:D24

    // compare the results of fill-right / -left with the reference stored in the test file
    // this compares the whole area blindly, for specific test cases, check the test file
    for (int nCol = 1; nCol <= 18; nCol++)
    {
        for (int nRow = 5; nRow <= 7; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol, nRow + 4, 0));
            OUString aStr1 = rDoc.GetString(nCol, nRow, 0);
            OUString aStr2 = rDoc.GetString(nCol, nRow + 4, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }

    // compare the results of fill-up / -down
    for (int nCol = 1; nCol <= 3; nCol++)
    {
        for (int nRow = 16; nRow <= 27; nRow++)
        {
            CellType nType1 = rDoc.GetCellType(ScAddress(nCol, nRow, 0));
            CellType nType2 = rDoc.GetCellType(ScAddress(nCol + 4, nRow, 0));
            OUString aStr1 = rDoc.GetString(nCol, nRow, 0);
            OUString aStr2 = rDoc.GetString(nCol + 4, nRow, 0);

            CPPUNIT_ASSERT_EQUAL(nType1, nType2);
            CPPUNIT_ASSERT_EQUAL(aStr1, aStr2);
        }
    }
}

ScCopyPasteTest::ScCopyPasteTest()
      : ScBootstrapFixture( "sc/qa/unit/data" )
{
}

void ScCopyPasteTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScCopyPasteTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCopyPasteTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
