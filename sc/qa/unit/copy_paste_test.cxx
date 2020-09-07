/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <comphelper/processfactory.hxx>

#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <impex.hxx>
#include <viewfunc.hxx>
#include <scitems.hxx>
#include <attrib.hxx>

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

    CPPUNIT_TEST_SUITE(ScCopyPasteTest);
    CPPUNIT_TEST(testCopyPasteXLS);
    CPPUNIT_TEST(testTdf84411);
    CPPUNIT_TEST(testTdf124565);
    CPPUNIT_TEST(testTdf126421);
    CPPUNIT_TEST(testTdf107394);
    CPPUNIT_TEST(testTdf53431_fillOnAutofilter);
    CPPUNIT_TEST_SUITE_END();

private:

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
    ScDocShellRef xDocSh = loadDoc("chartx2.", FORMAT_XLS);
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
    ScRefFlags nRes = aSrcRange.Parse("B2:C5", &rDoc, rDoc.GetAddressConvention());
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
    ScRefFlags nRes = aSrcRange.Parse(rSrcRange, &rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & ScRefFlags::VALID));
    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);
    pViewShell->GetViewData().GetMarkData().SetSelectedTabs(TabsInRange(aSrcRange));
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // 2. Paste
    ScRange aDstRange;
    nRes = aDstRange.Parse(rDstRange, &rDoc, rDoc.GetAddressConvention());
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
    ScImportExport aObj(&rDoc, ScAddress(0,0,0));
    aObj.SetImportBroadcast(true);

    OString aHTML("<pre>First\nVery long sentence.</pre>");
    SvMemoryStream aStream;
    aStream.WriteOString(aHTML);
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

void ScCopyPasteTest::testTdf53431_fillOnAutofilter()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // create a frame
    Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame("_blank", 0);
    CPPUNIT_ASSERT(xTargetFrame.is());

    // 1. Open the document
    ScDocShellRef xDocSh = loadDoc("tdf53431_autofilterFilldown.", FORMAT_ODS, true);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf53431_autofilterFilldown.ods.", xDocSh.is());

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
