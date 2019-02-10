/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <sal/config.h>

#include <cassert>

#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <reffact.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <rangenam.hxx>
#include <globalnames.hxx>
#include <dbnamdlg.hxx>
#include <dbdocfun.hxx>

class DBSaveData;

static DBSaveData* pSaveObj = nullptr;

namespace
{
    void ERRORBOX(weld::Window* pParent, const OUString& rString)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  rString));
        xBox->run();
    }
}

//  class DBSaveData

class DBSaveData
{
public:
    DBSaveData( Edit& rEd, CheckBox& rHdr, CheckBox& rTot, CheckBox& rSize, CheckBox& rFmt,
                            CheckBox& rStrip, ScRange& rArea )
        : rEdAssign(rEd)
        , rBtnHeader(rHdr)
        , rBtnTotals(rTot)
        , rBtnSize(rSize)
        , rBtnFormat(rFmt)
        , rBtnStrip(rStrip)
        , rCurArea(rArea)
        , bHeader(false)
        , bTotals(false)
        , bSize(false)
        , bFormat(false)
        , bStrip(false)
        , bDirty(false)
    {
    }
    void Save();
    void Restore();

private:
    Edit&       rEdAssign;
    CheckBox&   rBtnHeader;
    CheckBox&   rBtnTotals;
    CheckBox&   rBtnSize;
    CheckBox&   rBtnFormat;
    CheckBox&   rBtnStrip;
    ScRange&    rCurArea;
    OUString    aStr;
    ScRange     aArea;
    bool        bHeader:1;
    bool        bTotals:1;
    bool        bSize:1;
    bool        bFormat:1;
    bool        bStrip:1;
    bool        bDirty:1;
};

void DBSaveData::Save()
{
    aArea   = rCurArea;
    aStr    = rEdAssign.GetText();
    bHeader = rBtnHeader.IsChecked();
    bTotals = rBtnTotals.IsChecked();
    bSize   = rBtnSize.IsChecked();
    bFormat = rBtnFormat.IsChecked();
    bStrip  = rBtnStrip.IsChecked();
    bDirty  = true;
}

void DBSaveData::Restore()
{
    if ( bDirty )
    {
        rCurArea = aArea;
        rEdAssign.SetText( aStr );
        rBtnHeader.Check ( bHeader );
        rBtnTotals.Check ( bTotals );
        rBtnSize.Check   ( bSize );
        rBtnFormat.Check ( bFormat );
        rBtnStrip.Check  ( bStrip );
        bDirty = false;
    }
}

//  class ScDbNameDlg

ScDbNameDlg::ScDbNameDlg(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
    ScViewData* ptrViewData)
    : ScAnyRefDlg(pB, pCW, pParent,
        "DefineDatabaseRangeDialog",
            "modules/scalc/ui/definedatabaserangedialog.ui")
    , pViewData(ptrViewData)
    , pDoc(ptrViewData->GetDocument())
    , bRefInputMode(false)
    , aAddrDetails(pDoc->GetAddressConvention(), 0, 0)
    , aLocalDbCol(*(pDoc->GetDBCollection()))
{
    get(m_pEdName, "entry");
    m_pEdName->set_height_request(m_pEdName->GetOptimalSize().Height() + m_pEdName->GetTextHeight() * 8);
    get(m_pEdAssign, "assign");
    get(m_pAssignFrame, "RangeFrame");
    m_pEdAssign->SetReferences(this, m_pAssignFrame->get_label_widget());
    get(m_pRbAssign, "assignrb");
    m_pRbAssign->SetReferences(this, m_pEdAssign);
    get(m_pOptions, "Options");
    get(m_pBtnHeader, "ContainsColumnLabels");
    get(m_pBtnTotals, "ContainsTotalsRow");
    get(m_pBtnDoSize, "InsertOrDeleteCells");
    get(m_pBtnKeepFmt, "KeepFormatting");
    get(m_pBtnStripData, "DontSaveImportedData");
    get(m_pFTSource, "Source");
    get(m_pFTOperations, "Operations");
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pBtnAdd, "add");
    aStrAdd = m_pBtnAdd->GetText();
    aStrModify = get<vcl::Window>("modify")->GetText();
    get(m_pBtnRemove, "delete");
    aStrInvalid = get<vcl::Window>("invalid")->GetText();

    m_pFTSource->SetStyle(m_pFTSource->GetStyle() | WB_NOLABEL);
    m_pFTOperations->SetStyle(m_pFTOperations->GetStyle() | WB_NOLABEL);

    //  so that the strings in the resource can stay with fixed texts:
    aStrSource      = m_pFTSource->GetText();
    aStrOperations  = m_pFTOperations->GetText();

    pSaveObj = new DBSaveData( *m_pEdAssign, *m_pBtnHeader, *m_pBtnTotals,
                        *m_pBtnDoSize, *m_pBtnKeepFmt, *m_pBtnStripData, theCurArea );
    Init();
}

ScDbNameDlg::~ScDbNameDlg()
{
    disposeOnce();
}

void ScDbNameDlg::dispose()
{
    DELETEZ( pSaveObj );
    m_pEdName.clear();
    m_pAssignFrame.clear();
    m_pEdAssign.clear();
    m_pRbAssign.clear();
    m_pOptions.clear();
    m_pBtnHeader.clear();
    m_pBtnTotals.clear();
    m_pBtnDoSize.clear();
    m_pBtnKeepFmt.clear();
    m_pBtnStripData.clear();
    m_pFTSource.clear();
    m_pFTOperations.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    ScAnyRefDlg::dispose();
}

void ScDbNameDlg::Init()
{
    m_pBtnHeader->Check();          // Default: with column headers
    m_pBtnTotals->Check( false );   // Default: without totals row
    m_pBtnDoSize->Check();
    m_pBtnKeepFmt->Check();

    m_pBtnOk->SetClickHdl      ( LINK( this, ScDbNameDlg, OkBtnHdl ) );
    m_pBtnCancel->SetClickHdl  ( LINK( this, ScDbNameDlg, CancelBtnHdl ) );
    m_pBtnAdd->SetClickHdl     ( LINK( this, ScDbNameDlg, AddBtnHdl ) );
    m_pBtnRemove->SetClickHdl  ( LINK( this, ScDbNameDlg, RemoveBtnHdl ) );
    m_pEdName->SetModifyHdl    ( LINK( this, ScDbNameDlg, NameModifyHdl ) );
    m_pEdAssign->SetModifyHdl  ( LINK( this, ScDbNameDlg, AssModifyHdl ) );
    UpdateNames();

    OUString  theAreaStr;

    if ( pViewData && pDoc )
    {
        SCCOL   nStartCol   = 0;
        SCROW   nStartRow   = 0;
        SCTAB   nStartTab   = 0;
        SCCOL   nEndCol     = 0;
        SCROW   nEndRow     = 0;
        SCTAB   nEndTab     = 0;

        ScDBCollection* pDBColl = pDoc->GetDBCollection();
        ScDBData*       pDBData = nullptr;

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        theCurArea = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);

        theAreaStr = theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, aAddrDetails);

        if ( pDBColl )
        {
            // determine if the defined DB area has been marked:
            pDBData = pDBColl->GetDBAtCursor( nStartCol, nStartRow, nStartTab, ScDBDataPortion::TOP_LEFT );
            if ( pDBData )
            {
                ScAddress&  rStart = theCurArea.aStart;
                ScAddress&  rEnd   = theCurArea.aEnd;
                SCCOL nCol1;
                SCCOL  nCol2;
                SCROW  nRow1;
                SCROW  nRow2;
                SCTAB  nTab;

                pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

                if (   (rStart.Tab() == nTab)
                    && (rStart.Col() == nCol1) && (rStart.Row() == nRow1)
                    && (rEnd.Col()   == nCol2) && (rEnd.Row()   == nRow2 ) )
                {
                    OUString aDBName = pDBData->GetName();
                    if ( aDBName != STR_DB_LOCAL_NONAME )
                        m_pEdName->SetText(aDBName);

                    m_pBtnHeader->Check( pDBData->HasHeader() );
                    m_pBtnTotals->Check( pDBData->HasTotals() );
                    m_pBtnDoSize->Check( pDBData->IsDoSize() );
                    m_pBtnKeepFmt->Check( pDBData->IsKeepFmt() );
                    m_pBtnStripData->Check( pDBData->IsStripData() );
                    SetInfoStrings( pDBData );
                }
            }
        }
    }

    m_pEdAssign->SetText( theAreaStr );
    m_pEdName->GrabFocus();
    bSaved = true;
    pSaveObj->Save();
    NameModifyHdl( *m_pEdName );
}

void ScDbNameDlg::SetInfoStrings( const ScDBData* pDBData )
{
    OUStringBuffer aBuf;
    aBuf.append(aStrSource);
    if (pDBData)
    {
        aBuf.append(' ');
        aBuf.append(pDBData->GetSourceString());
    }
    m_pFTSource->SetText(aBuf.makeStringAndClear());

    aBuf.append(aStrOperations);
    if (pDBData)
    {
        aBuf.append(' ');
        aBuf.append(pDBData->GetOperations());
    }
    m_pFTOperations->SetText(aBuf.makeStringAndClear());
}

// Transfer of a table area selected with the mouse, which is then displayed
// as a new selection in the reference window.

void ScDbNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);

        theCurArea = rRef;

        OUString aRefStr(theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDocP, aAddrDetails));
        m_pEdAssign->SetRefString( aRefStr );
        m_pOptions->Enable();
        m_pBtnAdd->Enable();
        bSaved = true;
        pSaveObj->Save();
    }
}

bool ScDbNameDlg::Close()
{
    return DoClose( ScDbNameDlgWrapper::GetChildWindowId() );
}

void ScDbNameDlg::SetActive()
{
    m_pEdAssign->GrabFocus();

    //  No NameModifyHdl, because otherwise areas can not be changed
    //  (the old content would be displayed again after the reference selection is pulled)
    //  (the selected DB name has not changed either)

    RefInputDone();
}

void ScDbNameDlg::UpdateNames()
{
    typedef ScDBCollection::NamedDBs DBsType;

    const DBsType& rDBs = aLocalDbCol.getNamedDBs();

    m_pEdName->SetUpdateMode( false );

    m_pEdName->Clear();
    m_pEdAssign->SetText( EMPTY_OUSTRING );

    if (!rDBs.empty())
    {
        for (const auto& rxDB : rDBs)
            m_pEdName->InsertEntry(rxDB->GetName());
    }
    else
    {
        m_pBtnAdd->SetText( aStrAdd );
        m_pBtnAdd->Disable();
        m_pBtnRemove->Disable();
    }

    m_pEdName->SetUpdateMode( true );
    m_pEdName->Invalidate();
}

void ScDbNameDlg::UpdateDBData( const OUString& rStrName )
{

    const ScDBData* pData = aLocalDbCol.getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rStrName));

    if ( pData )
    {
        SCCOL nColStart = 0;
        SCROW nRowStart = 0;
        SCCOL nColEnd    = 0;
        SCROW nRowEnd    = 0;
        SCTAB nTab       = 0;

        pData->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
        theCurArea = ScRange( ScAddress( nColStart, nRowStart, nTab ),
                              ScAddress( nColEnd,   nRowEnd,   nTab ) );
        OUString theArea(theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, aAddrDetails));
        m_pEdAssign->SetText( theArea );
        m_pBtnAdd->SetText( aStrModify );
        m_pBtnHeader->Check( pData->HasHeader() );
        m_pBtnTotals->Check( pData->HasTotals() );
        m_pBtnDoSize->Check( pData->IsDoSize() );
        m_pBtnKeepFmt->Check( pData->IsKeepFmt() );
        m_pBtnStripData->Check( pData->IsStripData() );
        SetInfoStrings( pData );
    }

    m_pBtnAdd->SetText( aStrModify );
    m_pBtnAdd->Enable();
    m_pBtnRemove->Enable();
    m_pOptions->Enable();
}

bool ScDbNameDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

// Handler:

IMPL_LINK_NOARG(ScDbNameDlg, OkBtnHdl, Button*, void)
{
    AddBtnHdl( nullptr );

    // Pass the changes and the remove list to the view: both are
    // transferred as a reference only, so that no dead memory can
    // be created at this point:
    if ( pViewData )
    {
        ScDBDocFunc aFunc(*pViewData->GetDocShell());
        aFunc.ModifyAllDBData(aLocalDbCol, aRemoveList);
    }

    Close();
}

IMPL_LINK_NOARG(ScDbNameDlg, CancelBtnHdl, Button*, void)
{
    Close();
}

IMPL_LINK_NOARG(ScDbNameDlg, AddBtnHdl, Button*, void)
{
    OUString  aNewName = comphelper::string::strip(m_pEdName->GetText(), ' ');
    OUString  aNewArea = m_pEdAssign->GetText();

    if ( !aNewName.isEmpty() && !aNewArea.isEmpty() )
    {
        if ( ScRangeData::IsNameValid( aNewName, pDoc ) == ScRangeData::NAME_VALID && aNewName != STR_DB_LOCAL_NONAME )
        {
            //  because editing can be done now, parsing is needed first
            ScRange aTmpRange;
            OUString aText = m_pEdAssign->GetText();
            if ( aTmpRange.ParseAny( aText, pDoc, aAddrDetails ) & ScRefFlags::VALID )
            {
                theCurArea = aTmpRange;
                ScAddress aStart = theCurArea.aStart;
                ScAddress aEnd   = theCurArea.aEnd;

                ScDBData* pOldEntry = aLocalDbCol.getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(aNewName));
                if (pOldEntry)
                {
                    //  modify area

                    pOldEntry->MoveTo( aStart.Tab(), aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row() );
                    pOldEntry->SetByRow( true );
                    pOldEntry->SetHeader( m_pBtnHeader->IsChecked() );
                    pOldEntry->SetTotals( m_pBtnTotals->IsChecked() );
                    pOldEntry->SetDoSize( m_pBtnDoSize->IsChecked() );
                    pOldEntry->SetKeepFmt( m_pBtnKeepFmt->IsChecked() );
                    pOldEntry->SetStripData( m_pBtnStripData->IsChecked() );
                }
                else
                {
                    //  insert new area

                    std::unique_ptr<ScDBData> pNewEntry(new ScDBData( aNewName, aStart.Tab(),
                                                        aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row(),
                                                        true, m_pBtnHeader->IsChecked(),
                                                        m_pBtnTotals->IsChecked() ));
                    pNewEntry->SetDoSize( m_pBtnDoSize->IsChecked() );
                    pNewEntry->SetKeepFmt( m_pBtnKeepFmt->IsChecked() );
                    pNewEntry->SetStripData( m_pBtnStripData->IsChecked() );

                    bool ins = aLocalDbCol.getNamedDBs().insert(std::move(pNewEntry));
                    assert(ins); (void)ins;
                }

                UpdateNames();

                m_pEdName->SetText( EMPTY_OUSTRING );
                m_pEdName->GrabFocus();
                m_pBtnAdd->SetText( aStrAdd );
                m_pBtnAdd->Disable();
                m_pBtnRemove->Disable();
                m_pEdAssign->SetText( EMPTY_OUSTRING );
                m_pBtnHeader->Check();             // Default: with column headers
                m_pBtnTotals->Check( false );      // Default: without totals row
                m_pBtnDoSize->Check( false );
                m_pBtnKeepFmt->Check( false );
                m_pBtnStripData->Check( false );
                SetInfoStrings( nullptr );     // empty
                theCurArea = ScRange();
                bSaved = true;
                pSaveObj->Save();
                NameModifyHdl( *m_pEdName );
            }
            else
            {
                ERRORBOX(GetFrameWeld(), aStrInvalid);
                m_pEdAssign->SetSelection( Selection( 0, SELECTION_MAX ) );
                m_pEdAssign->GrabFocus();
            }
        }
        else
        {
            ERRORBOX(GetFrameWeld(), ScResId(STR_INVALIDNAME));
            m_pEdName->SetSelection( Selection( 0, SELECTION_MAX ) );
            m_pEdName->GrabFocus();
        }
    }
}

namespace {

class FindByName
{
    const OUString& mrName;
public:
    explicit FindByName(const OUString& rName) : mrName(rName) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetName() == mrName;
    }
};

}

IMPL_LINK_NOARG(ScDbNameDlg, RemoveBtnHdl, Button*, void)
{
    OUString aStrEntry = m_pEdName->GetText();
    ScDBCollection::NamedDBs& rDBs = aLocalDbCol.getNamedDBs();
    ScDBCollection::NamedDBs::iterator itr =
        ::std::find_if(rDBs.begin(), rDBs.end(), FindByName(aStrEntry));

    if (itr != rDBs.end())
    {
        OUString aStrDelMsg = ScResId( STR_QUERY_DELENTRY );
        OUString sMsg{ aStrDelMsg.getToken(0, '#') + aStrEntry + aStrDelMsg.getToken(1, '#') };
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       sMsg));
        xQueryBox->set_default_response(RET_YES);
        if (RET_YES == xQueryBox->run())
        {
            SCTAB nTab;
            SCCOL nColStart, nColEnd;
            SCROW nRowStart, nRowEnd;
            (*itr)->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
            aRemoveList.emplace_back( ScAddress( nColStart, nRowStart, nTab ),
                         ScAddress( nColEnd,   nRowEnd,   nTab ) );

            rDBs.erase(itr);

            UpdateNames();

            m_pEdName->SetText( EMPTY_OUSTRING );
            m_pEdName->GrabFocus();
            m_pBtnAdd->SetText( aStrAdd );
            m_pBtnAdd->Disable();
            m_pBtnRemove->Disable();
            m_pEdAssign->SetText( EMPTY_OUSTRING );
            theCurArea = ScRange();
            m_pBtnHeader->Check();             // Default: with column headers
            m_pBtnTotals->Check( false );      // Default: without totals row
            m_pBtnDoSize->Check( false );
            m_pBtnKeepFmt->Check( false );
            m_pBtnStripData->Check( false );
            SetInfoStrings( nullptr );     // empty
            bSaved=false;
            pSaveObj->Restore();
            NameModifyHdl( *m_pEdName );
        }
    }
}

IMPL_LINK_NOARG(ScDbNameDlg, NameModifyHdl, Edit&, void)
{
    OUString  theName     = m_pEdName->GetText();
    bool    bNameFound  = (COMBOBOX_ENTRY_NOTFOUND
                           != m_pEdName->GetEntryPos( theName ));

    if ( theName.isEmpty() )
    {
        if (m_pBtnAdd->GetText() != aStrAdd)
            m_pBtnAdd->SetText( aStrAdd );
        m_pBtnAdd->Disable();
        m_pBtnRemove->Disable();
        m_pAssignFrame->Disable();
        m_pOptions->Disable();
        //bSaved=sal_False;
        //pSaveObj->Restore();
        //@BugID 54702 enable/disable in the base class only
        //SFX_APPWINDOW->Disable(sal_False);        //! general method in ScAnyRefDlg
        bRefInputMode = false;
    }
    else
    {
        if ( bNameFound )
        {
            if (m_pBtnAdd->GetText() != aStrModify)
                m_pBtnAdd->SetText( aStrModify );

            if(!bSaved)
            {
                bSaved = true;
                pSaveObj->Save();
            }
            UpdateDBData( theName );
        }
        else
        {
            if (m_pBtnAdd->GetText() != aStrAdd)
                m_pBtnAdd->SetText( aStrAdd );

            bSaved=false;
            pSaveObj->Restore();

            if ( !m_pEdAssign->GetText().isEmpty() )
            {
                m_pBtnAdd->Enable();
                m_pOptions->Enable();
            }
            else
            {
                m_pBtnAdd->Disable();
                m_pOptions->Disable();
            }
            m_pBtnRemove->Disable();
        }

        m_pAssignFrame->Enable();

        //@BugID 54702 enable/disable in the base class only
        //SFX_APPWINDOW->Enable();
        bRefInputMode = true;
    }
}

IMPL_LINK_NOARG(ScDbNameDlg, AssModifyHdl, Edit&, void)
{
    //  parse here for Save(), etc.

    ScRange aTmpRange;
    OUString aText = m_pEdAssign->GetText();
    if ( aTmpRange.ParseAny( aText, pDoc, aAddrDetails ) & ScRefFlags::VALID )
        theCurArea = aTmpRange;

    if (!aText.isEmpty() && !m_pEdName->GetText().isEmpty())
    {
        m_pBtnAdd->Enable();
        m_pBtnHeader->Enable();
        m_pBtnTotals->Enable();
        m_pBtnDoSize->Enable();
        m_pBtnKeepFmt->Enable();
        m_pBtnStripData->Enable();
        m_pFTSource->Enable();
        m_pFTOperations->Enable();
    }
    else
    {
        m_pBtnAdd->Disable();
        m_pBtnHeader->Disable();
        m_pBtnTotals->Disable();
        m_pBtnDoSize->Disable();
        m_pBtnKeepFmt->Disable();
        m_pBtnStripData->Disable();
        m_pFTSource->Disable();
        m_pFTOperations->Disable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
