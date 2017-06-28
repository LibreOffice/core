/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <test/screenshot_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <osl/endian.h>
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
#include <strings.hrc>
#include <scresid.hxx>
#include <scitems.hxx>

using namespace css;

class ScScreenshotTest : public ScreenshotTest
{
private:
    /// members
    uno::Reference<lang::XComponent>        mxComponent;
    SfxObjectShell*                         mpFoundShell;
    ScDocShellRef                           mxDocSh;
    ScTabViewShell*                         mpViewShell;
    ScAbstractDialogFactory*                mpFact;
    OUString                                mCsv; // needs to outlive mpStream
    std::unique_ptr<SvMemoryStream>         mpStream;
    std::unique_ptr<SfxItemSet>             mpItemSet;

    /// helper
    void initialize();

    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    ScScreenshotTest();

    void testOpeningModalDialogs();

    CPPUNIT_TEST_SUITE(ScScreenshotTest);
    CPPUNIT_TEST(testOpeningModalDialogs);
    CPPUNIT_TEST_SUITE_END();
};

ScScreenshotTest::ScScreenshotTest()
:   mxComponent(),
    mpFoundShell(nullptr),
    mxDocSh(),
    mpViewShell(nullptr),
    mpFact(nullptr),
    mCsv("some, strings, here, separated, by, commas"),
    mpStream(),
    mpItemSet()
{
}

void ScScreenshotTest::initialize()
{
    if (mxComponent.is())
        mxComponent->dispose();
    // use new, empty doc to avoid file locking
    mxComponent = loadFromDesktop("private:factory/scalc", "com.sun.star.sheet.SpreadsheetDocument");

    mpFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", mpFoundShell);

    mxDocSh = dynamic_cast<ScDocShell*>(mpFoundShell);
    CPPUNIT_ASSERT(mxDocSh != nullptr);

    mpViewShell = mxDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(mpViewShell != nullptr);

    mpFact = ScAbstractDialogFactory::Create();
    CPPUNIT_ASSERT_MESSAGE("Failed to create dialog factory", mpFact);

    SvMemoryStream* pNewMemStream = new SvMemoryStream(const_cast<sal_Unicode *>(mCsv.getStr()), mCsv.getLength() * sizeof(sal_Unicode), StreamMode::READ);
    pNewMemStream->SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
    #ifdef OSL_BIGENDIAN
        pNewMemStream->SetEndian(SvStreamEndian::BIG);
    #else
        pNewMemStream->SetEndian(SvStreamEndian::LITTLE);
    #endif
    mpStream.reset(pNewMemStream);
}

void ScScreenshotTest::registerKnownDialogsByID(mapType& rKnownDialogs)
{
    // fill map of known dialogs
    rKnownDialogs["modules/scalc/ui/insertsheet.ui"] = 0;
    rKnownDialogs["modules/scalc/ui/deletecells.ui"] = 1;
    rKnownDialogs["modules/scalc/ui/pastespecial.ui"] = 2;
    rKnownDialogs["modules/scalc/ui/changesourcedialog.ui"] = 3;
    rKnownDialogs["modules/scalc/ui/selectdatasource.ui"] = 4;
    rKnownDialogs["modules/scalc/ui/selectsource.ui"] = 5;
    rKnownDialogs["modules/scalc/ui/deletecontents.ui"] = 6;
    rKnownDialogs["modules/scalc/ui/createnamesdialog.ui"] = 7;
    rKnownDialogs["modules/scalc/ui/inputstringdialog.ui"] = 8;
    rKnownDialogs["modules/scalc/ui/tabcolordialog.ui"] = 9;
    rKnownDialogs["modules/scalc/ui/textimportoptions.ui"] = 10;
    rKnownDialogs["modules/scalc/ui/dataform.ui"] = 11;
    rKnownDialogs["modules/scalc/ui/movecopysheet.ui"] = 12;
    rKnownDialogs["modules/scalc/ui/textimportcsv.ui"] = 13;
    rKnownDialogs["modules/scalc/ui/formatcellsdialog.ui"] = 14;
}

VclPtr<VclAbstractDialog> ScScreenshotTest::createDialogByID(sal_uInt32 nID)
{
    VclPtr<VclAbstractDialog> pReturnDialog;
    ////FIXME: translatable string here
    const OUString aDefaultSheetName("Sheet1");

    switch ( nID )
    {
        case 0: // "modules/scalc/ui/insertsheet.ui"
        {
            ScViewData& rViewData = mpViewShell->GetViewData();
            SCTAB nTabSelCount = rViewData.GetMarkData().GetSelectCount();

            pReturnDialog = mpFact->CreateScInsertTableDlg(
                mpViewShell->GetDialogParent(), rViewData, nTabSelCount, false);

            break;
        }

        case 1: // "modules/scalc/ui/deletecells.ui"
        {
            pReturnDialog = mpFact->CreateScDeleteCellDlg(mpViewShell->GetDialogParent(), false);
            break;
        }

        case 2: // "modules/scalc/ui/pastespecial.ui"
        {
            pReturnDialog = mpFact->CreateScInsertContentsDlg(mpViewShell->GetDialogParent());
            break;
        }

        case 3: // "modules/scalc/ui/changesourcedialog.ui"
        {
            pReturnDialog = mpFact->CreateScColRowLabelDlg(mpViewShell->GetDialogParent(), true, false);
            break;
        }

        case 4: // "modules/scalc/ui/selectdatasource.ui"
        {
            pReturnDialog = mpFact->CreateScDataPilotDatabaseDlg(mpViewShell->GetDialogParent());
            break;
        }
        case 5: // "modules/scalc/ui/selectsource.ui"
        {
            pReturnDialog = mpFact->CreateScDataPilotSourceTypeDlg(mpViewShell->GetDialogParent(), true);
            break;
        }

        case 6: // "modules/scalc/ui/deletecontents.ui"
        {
            pReturnDialog = mpFact->CreateScDeleteContentsDlg(mpViewShell->GetDialogParent());
            break;
        }

        case 7: // "modules/scalc/ui/createnamesdialog.ui"
        {
            pReturnDialog = mpFact->CreateScNameCreateDlg(
                mpViewShell->GetDialogParent(),
                CreateNameFlags::Left | CreateNameFlags::Top);
                    // just fake some flags
            break;
        }

        case 8: // "modules/scalc/ui/inputstringdialog.ui"
        {
            const OString aEmpty("");
            pReturnDialog = mpFact->CreateScStringInputDlg(mpViewShell->GetDialogParent(),
                                ScResId(SCSTR_APDTABLE), ScResId(SCSTR_NAME),
                                aDefaultSheetName, aEmpty, aEmpty );
            break;
        }

        case 9: // "modules/scalc/ui/tabcolordialog.ui"
        {
            pReturnDialog = mpFact->CreateScTabBgColorDlg(mpViewShell->GetDialogParent(),
                                ScResId(SCSTR_SET_TAB_BG_COLOR),
                                ScResId(SCSTR_NO_TAB_BG_COLOR), Color(0xff00ff) );
            break;
        }

        case 10: // "modules/scalc/ui/textimportoptions.ui"
        {
            pReturnDialog = mpFact->CreateScTextImportOptionsDlg();
            break;
        }

        case 11: // "modules/scalc/ui/dataform.ui"
        {
            ////FIXME: looks butt-ugly w/ empty file, move it elsewhere, where
            ////we actually have some data
            pReturnDialog = mpFact->CreateScDataFormDlg(mpViewShell->GetDialogParent(), mpViewShell);
            break;
        }

        case 12: // "modules/scalc/ui/movecopysheet.ui"
        {
            pReturnDialog = mpFact->CreateScMoveTableDlg(mpViewShell->GetDialogParent(), aDefaultSheetName);
            break;
        }

        case 13: // "modules/scalc/ui/textimportcsv.ui"
        {
            pReturnDialog = mpFact->CreateScImportAsciiDlg(OUString(), mpStream.get(), SC_PASTETEXT);
            break;
        }
        case 14: // "modules/scalc/ui/formatcellsdialog.ui"
        {
            ScViewData& rViewData = mpViewShell->GetViewData();
            ScDocument *pDoc = rViewData.GetDocument();

            const ScPatternAttr *pAttr = mpViewShell->GetSelectionPattern();
            std::unique_ptr<SvxNumberInfoItem> pNumberInfoItem;

            mpItemSet.reset(new SfxItemSet(pAttr->GetItemSet()));
            mpItemSet->Put(SfxUInt32Item(ATTR_VALUE_FORMAT,
                           pAttr->GetNumberFormat( pDoc->GetFormatTable() ) ) );

            pNumberInfoItem.reset(ScTabViewShell::MakeNumberInfoItem(pDoc, &rViewData));

            mpItemSet->MergeRange(SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO);
            mpItemSet->Put(*pNumberInfoItem);

            pReturnDialog = mpFact->CreateScAttrDlg(mpViewShell->GetDialogParent(), mpItemSet.get());
            break;
        }
           //ScopedVclPtrInstance<ScShareDocumentDlg> pDlg14( mpViewShell->GetDialogParent(), &rViewData );
            //ScopedVclPtrInstance<ScTableProtectionDlg> pDlg16(mpViewShell->GetDialogParent());
        default:
            break;
    }

    //CPPUNIT_ASSERT_MESSAGE( "Failed to create dialog", pReturnDialog );
    return pReturnDialog;
}

void ScScreenshotTest::testOpeningModalDialogs()
{
    initialize();

    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile("sc/qa/unit/screenshots/data/screenshots.txt");

    mxComponent->dispose();
    mxComponent.clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScScreenshotTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
