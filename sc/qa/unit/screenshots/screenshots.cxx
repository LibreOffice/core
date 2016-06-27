/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>

#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <sharedocdlg.hxx>
#include <protectiondlg.hxx>
#include <docuno.hxx>
#include <scabstdlg.hxx>
#include <reffact.hxx>
#include <scui_def.hxx>
#include <impex.hxx>

#include <sc.hrc>
#include <scresid.hxx>
#include <scitems.hxx>

using namespace css;

#if !defined(WNT) && !defined(MACOSX)
static const char* DATA_DIRECTORY = "/sc/qa/unit/screenshots/data/";
#endif

class ScScreenshotTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    ScScreenshotTest();
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

#if !defined(WNT) && !defined(MACOSX)
    void testOpeningSomeDialog();
    //void testOpeningModelessDialogs();
#endif

    CPPUNIT_TEST_SUITE(ScScreenshotTest);
#if !defined(WNT) && !defined(MACOSX)
    CPPUNIT_TEST(testOpeningSomeDialog);
    //CPPUNIT_TEST(testOpeningModelessDialogs);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
#if !defined(WNT) && !defined(MACOSX)
    ScModelObj* createDoc(const char* pName);

#endif

    uno::Reference<lang::XComponent> mxComponent;
};

ScScreenshotTest::ScScreenshotTest()
{
}

void ScScreenshotTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void ScScreenshotTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

#if !defined(WNT) && !defined(MACOSX)
ScModelObj* ScScreenshotTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj;
}

void ScScreenshotTest::testOpeningSomeDialog()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    // display insert sheet dialog
    //uno::Sequence<beans::PropertyValue> aArgs;
    //comphelper::dispatchCommand(".uno:Insert", aArgs);

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != nullptr);

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    CPPUNIT_ASSERT( pFact != nullptr );

    ScViewData& rViewData = pViewShell->GetViewData();
    SCTAB nTabSelCount = rViewData.GetMarkData().GetSelectCount();
    std::unique_ptr<AbstractScInsertTableDlg> pDlg( pFact->CreateScInsertTableDlg(
           pViewShell->GetDialogParent(), rViewData, nTabSelCount, false));
    CPPUNIT_ASSERT( pDlg != nullptr );

    pDlg->Execute();

    std::unique_ptr<AbstractScDeleteCellDlg> pDlg2( pFact->CreateScDeleteCellDlg(
           pViewShell->GetDialogParent(), false));
    CPPUNIT_ASSERT( pDlg2 != nullptr );

    pDlg2->Execute();

    std::unique_ptr<AbstractScInsertContentsDlg> pDlg3( pFact->CreateScInsertContentsDlg(
           pViewShell->GetDialogParent()));
    CPPUNIT_ASSERT( pDlg3 != nullptr );

    pDlg3->Execute();

    std::unique_ptr<AbstractScColRowLabelDlg> pDlg4( pFact->CreateScColRowLabelDlg(
           pViewShell->GetDialogParent(), true, false));
    CPPUNIT_ASSERT( pDlg4 != nullptr );

    pDlg4->Execute();

    std::unique_ptr<AbstractScDataPilotDatabaseDlg> pDlg5( pFact->CreateScDataPilotDatabaseDlg(
           pViewShell->GetDialogParent()));
    CPPUNIT_ASSERT( pDlg5 != nullptr );

    pDlg5->Execute();

    std::unique_ptr<AbstractScDataPilotSourceTypeDlg> pDlg6( pFact->CreateScDataPilotSourceTypeDlg(
           pViewShell->GetDialogParent(), true));
    CPPUNIT_ASSERT( pDlg6 != nullptr );

    pDlg6->Execute();

    std::unique_ptr<AbstractScDeleteContentsDlg> pDlg7( pFact->CreateScDeleteContentsDlg(
           pViewShell->GetDialogParent()));
    CPPUNIT_ASSERT( pDlg7 != nullptr );

    pDlg7->Execute();

    // just fake some flags
    sal_uInt16 nFlags = NAME_LEFT | NAME_TOP;
    std::unique_ptr<AbstractScNameCreateDlg> pDlg8( pFact->CreateScNameCreateDlg(
           pViewShell->GetDialogParent(), nFlags));
    CPPUNIT_ASSERT( pDlg8 != nullptr );

    pDlg8->Execute();

    //FIXME: translatable string here
    const OUString aDefaultSheetName("Sheet1");
    const OString aEmpty("");
    std::unique_ptr<AbstractScStringInputDlg> pDlg9( pFact->CreateScStringInputDlg(
           pViewShell->GetDialogParent(), OUString(ScResId(SCSTR_APDTABLE)), OUString(ScResId(SCSTR_NAME)),
           aDefaultSheetName, aEmpty, aEmpty));
    CPPUNIT_ASSERT( pDlg9 != nullptr );

    pDlg9->Execute();

    std::unique_ptr<AbstractScTabBgColorDlg> pDlg10( pFact->CreateScTabBgColorDlg(
           pViewShell->GetDialogParent(), OUString(ScResId(SCSTR_SET_TAB_BG_COLOR)),
           OUString(ScResId(SCSTR_NO_TAB_BG_COLOR)), Color(0xff00ff), ".uno:TabBgColor"));
    CPPUNIT_ASSERT( pDlg10 != nullptr );

    pDlg10->Execute();

    std::unique_ptr<AbstractScTextImportOptionsDlg> pDlg11( pFact->CreateScTextImportOptionsDlg());
    CPPUNIT_ASSERT( pDlg11 != nullptr );

    pDlg11->Execute();

    //FIXME: looks butt-ugly w/ empty file, move it elsewhere, where
    //we actually have some data
    std::unique_ptr<AbstractScDataFormDlg> pDlg12( pFact->CreateScDataFormDlg(
           pViewShell->GetDialogParent(), pViewShell));
    CPPUNIT_ASSERT( pDlg12 != nullptr );

    pDlg12->Execute();

    const OUString aCsv("some, strings, here, separated, by, commas");
    ScImportStringStream aStream( aCsv );
    std::unique_ptr<AbstractScImportAsciiDlg> pDlg13( pFact->CreateScImportAsciiDlg(
           OUString(), &aStream, SC_PASTETEXT ));
    CPPUNIT_ASSERT( pDlg13 != nullptr );

    pDlg13->Execute();

    ScopedVclPtrInstance<ScShareDocumentDlg> pDlg14( pViewShell->GetDialogParent(), &rViewData );
    CPPUNIT_ASSERT( pDlg14 != nullptr );

    pDlg14->Execute();

    std::unique_ptr<AbstractScMoveTableDlg> pDlg15( pFact->CreateScMoveTableDlg(
           pViewShell->GetDialogParent(), aDefaultSheetName));
    CPPUNIT_ASSERT( pDlg15 != nullptr );

    pDlg15->Execute();

    ScopedVclPtrInstance<ScTableProtectionDlg> pDlg16(pViewShell->GetDialogParent());
    CPPUNIT_ASSERT( pDlg16 != nullptr );

    pDlg16->Execute();
}

#endif

CPPUNIT_TEST_SUITE_REGISTRATION(ScScreenshotTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
