/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/screenshot_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>
#include <svx/numinf.hxx>
#include <vcl/pngwrite.hxx>

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
#include <patattr.hxx>

#include <sc.hrc>
#include <scresid.hxx>
#include <scitems.hxx>

using namespace css;

static const char* DATA_DIRECTORY = "/sc/qa/unit/screenshots/data/";

class ScScreenshotTest : public ScreenshotTest
{
public:
    ScScreenshotTest();

    void testOpeningModalDialogs();
    //void testOpeningModelessDialogs();

    CPPUNIT_TEST_SUITE(ScScreenshotTest);
    CPPUNIT_TEST(testOpeningModalDialogs);
    //CPPUNIT_TEST(testOpeningModelessDialogs);
    CPPUNIT_TEST_SUITE_END();

private:
    void initializeWithDoc(const char* pName);

    VclAbstractDialog* createDialogByID( sal_uInt32 nID);


    uno::Reference<lang::XComponent> mxComponent;
    SfxObjectShell* pFoundShell;
    ScDocShellRef xDocSh;
    ScTabViewShell* pViewShell;
    ScAbstractDialogFactory* pFact;

    std::unique_ptr<ScImportStringStream> pStream;
    std::unique_ptr<SfxItemSet> pItemSet;
};

ScScreenshotTest::ScScreenshotTest()
{
}

void ScScreenshotTest::initializeWithDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");

    pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != nullptr);

    pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    pFact = ScAbstractDialogFactory::Create();
    CPPUNIT_ASSERT_MESSAGE("Failed to create dialog factory", pFact);

    const OUString aCsv("some, strings, here, separated, by, commas");
    pStream.reset( new ScImportStringStream( aCsv) );
}

VclAbstractDialog* ScScreenshotTest::createDialogByID( sal_uInt32 nID )
{
    VclAbstractDialog *pReturnDialog = nullptr;
    ////FIXME: translatable string here
    const OUString aDefaultSheetName("Sheet1");

    switch ( nID )
    {
        case 0:
        {
            ScViewData& rViewData = pViewShell->GetViewData();
            SCTAB nTabSelCount = rViewData.GetMarkData().GetSelectCount();

            pReturnDialog = pFact->CreateScInsertTableDlg(
                   pViewShell->GetDialogParent(), rViewData, nTabSelCount, false);

            break;
        }

        case 1:
        {
            pReturnDialog = pFact->CreateScDeleteCellDlg( pViewShell->GetDialogParent(), false );
            break;
        }

        case 2:
        {
            pReturnDialog = pFact->CreateScInsertContentsDlg( pViewShell->GetDialogParent() );
            break;
        }

        case 3:
        {
            pReturnDialog = pFact->CreateScColRowLabelDlg( pViewShell->GetDialogParent(), true, false );
            break;
        }

        case 4:
        {
            pReturnDialog = pFact->CreateScDataPilotDatabaseDlg( pViewShell->GetDialogParent() );
            break;
        }
        case 5:
        {

            pReturnDialog = pFact->CreateScDataPilotSourceTypeDlg(pViewShell->GetDialogParent(), true );
            break;
        }

        case 6:
        {
            pReturnDialog = pFact->CreateScDeleteContentsDlg( pViewShell->GetDialogParent() );
            break;
        }

        case 7:
        {
            //// just fake some flags
            sal_uInt16 nFlags = NAME_LEFT | NAME_TOP;
            pReturnDialog = pFact->CreateScNameCreateDlg( pViewShell->GetDialogParent(), nFlags );
            break;
        }

        case 8:
        {
            const OString aEmpty("");
            pReturnDialog = pFact->CreateScStringInputDlg( pViewShell->GetDialogParent(),
                                OUString(ScResId(SCSTR_APDTABLE)), OUString(ScResId(SCSTR_NAME)),
                                aDefaultSheetName, aEmpty, aEmpty );
            break;
        }

        case 9:
        {
            pReturnDialog = pFact->CreateScTabBgColorDlg( pViewShell->GetDialogParent(),
                                OUString(ScResId(SCSTR_SET_TAB_BG_COLOR)),
                                OUString(ScResId(SCSTR_NO_TAB_BG_COLOR)), Color(0xff00ff) );
            break;
        }

        case 10:
        {
            pReturnDialog = pFact->CreateScTextImportOptionsDlg();
            break;
        }

        case 11:
        {
            ////FIXME: looks butt-ugly w/ empty file, move it elsewhere, where
            ////we actually have some data
            pReturnDialog = pFact->CreateScDataFormDlg( pViewShell->GetDialogParent(), pViewShell );
            break;
        }

        case 12:
        {
            pReturnDialog = pFact->CreateScMoveTableDlg( pViewShell->GetDialogParent(), aDefaultSheetName );
            break;
        }

        case 13:
        {
            pReturnDialog = pFact->CreateScImportAsciiDlg( OUString(), pStream.get(), SC_PASTETEXT );
            break;
        }
        case 14:
        {
            ScViewData& rViewData = pViewShell->GetViewData();
            ScDocument *pDoc = rViewData.GetDocument();

            const ScPatternAttr *pAttr = pViewShell->GetSelectionPattern();
            std::unique_ptr<SvxNumberInfoItem> pNumberInfoItem;

            pItemSet.reset( new SfxItemSet( pAttr->GetItemSet() ) );
            pItemSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
                           pAttr->GetNumberFormat( pDoc->GetFormatTable() ) ) );

            pNumberInfoItem.reset( pViewShell->MakeNumberInfoItem(pDoc, &rViewData) );

            pItemSet->MergeRange( SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO );
            pItemSet->Put(*pNumberInfoItem );

            pReturnDialog = pFact->CreateScAttrDlg( pViewShell->GetDialogParent(), pItemSet.get() );
            break;
        }
           //ScopedVclPtrInstance<ScShareDocumentDlg> pDlg14( pViewShell->GetDialogParent(), &rViewData );
            //ScopedVclPtrInstance<ScTableProtectionDlg> pDlg16(pViewShell->GetDialogParent());
        default:
            break;
    }

    //CPPUNIT_ASSERT_MESSAGE( "Failed to create dialog", pReturnDialog );
    return pReturnDialog;
}

void ScScreenshotTest::testOpeningModalDialogs()
{
    initializeWithDoc("empty.ods");

    const sal_uInt32 nDialogs = 15;

    for ( sal_uInt32 i = 0; i < nDialogs; i++ )
    {
        std::unique_ptr<VclAbstractDialog> pDialog( createDialogByID( i ) );

        dumpDialogToPath( *pDialog );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScScreenshotTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
