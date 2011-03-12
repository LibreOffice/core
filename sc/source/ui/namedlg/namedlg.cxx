/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes ---------------------------------------------------------



// INCLUDE -------------------------------------------------------------------

#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "namedlg.hrc"
#include "namedlg.hxx"

#include <vcl/msgbox.hxx>



// defines -------------------------------------------------------------------

#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

struct ScNameDlgImpl
{
    ScNameDlgImpl() :
        bCriteria(false), bPrintArea(false),
        bColHeader(false), bRowHeader(false),
        bDirty(false) {}

    void Clear()
    {
        aStrSymbol = ::rtl::OUString();
        bCriteria  = bPrintArea = bColHeader = bRowHeader = false;
        bDirty = true;
    }

    ::rtl::OUString aStrSymbol;
    bool bCriteria:1;
    bool bPrintArea:1;
    bool bColHeader:1;
    bool bRowHeader:1;
    bool bDirty:1;
};

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute();

ScNameDlg::ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                      ScViewData*       ptrViewData,
                      const ScAddress&  aCursorPos )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_NAMES ),
        //
        maFtScope( this, ScResId(FT_SCOPE) ),
        maLbScope( this, ScResId(LB_SCOPE) ),
        aFlName         ( this, ScResId( FL_NAME ) ),
        aEdName         ( this, ScResId( ED_NAME ) ),
        //
        aFlAssign       ( this, ScResId( FL_ASSIGN ) ),
        aEdAssign       ( this, this, ScResId( ED_ASSIGN ) ),
        aRbAssign       ( this, ScResId( RB_ASSIGN ), &aEdAssign, this ),
        //
        aFlType         ( this, ScResId( FL_TYPE ) ),
        aBtnPrintArea   ( this, ScResId( BTN_PRINTAREA ) ),
        aBtnColHeader   ( this, ScResId( BTN_COLHEADER ) ),
        aBtnCriteria    ( this, ScResId( BTN_CRITERIA ) ),
        aBtnRowHeader   ( this, ScResId( BTN_ROWHEADER ) ),
        //
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),
        //
        bSaved          (FALSE),
        aStrAdd         ( ScResId( STR_ADD ) ),
        aStrModify      ( ScResId( STR_MODIFY ) ),
        errMsgInvalidSym( ScResId( STR_INVALIDSYMBOL ) ),
        //
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        aLocalRangeName ( *(pDoc->GetRangeName()) ),
        theCursorPos    ( aCursorPos ),
        mpImpl(new ScNameDlgImpl)
{
    Init();
    FreeResource();
}

ScNameDlg::~ScNameDlg()
{
    delete mpImpl;
}

void ScNameDlg::Init()
{
    String  aAreaStr;
    ScRange aRange;

    DBG_ASSERT( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

    aBtnOk.SetClickHdl      ( LINK( this, ScNameDlg, OkBtnHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScNameDlg, CancelBtnHdl ) );
    aBtnAdd.SetClickHdl     ( LINK( this, ScNameDlg, AddBtnHdl ) );
    aBtnRemove.SetClickHdl  ( LINK( this, ScNameDlg, RemoveBtnHdl ) );
    aEdAssign.SetGetFocusHdl( LINK( this, ScNameDlg, AssignGetFocusHdl ) );
    aEdAssign.SetModifyHdl  ( LINK( this, ScNameDlg, EdModifyHdl ) );
    aEdName.SetModifyHdl    ( LINK( this, ScNameDlg, EdModifyHdl ) );
    aEdName.SetSelectHdl    ( LINK( this, ScNameDlg, NameSelectHdl ) );

    aBtnCriteria .Hide();
    aBtnPrintArea.Hide();
    aBtnColHeader.Hide();
    aBtnRowHeader.Hide();

    aBtnMore.AddWindow( &aFlType );
    aBtnMore.AddWindow( &aBtnCriteria );
    aBtnMore.AddWindow( &aBtnPrintArea );
    aBtnMore.AddWindow( &aBtnColHeader );
    aBtnMore.AddWindow( &aBtnRowHeader );

    UpdateNames();

    pViewData->GetSimpleArea( aRange );
    aRange.Format( aAreaStr, ABS_DREF3D, pDoc,
                   ScAddress::Details(pDoc->GetAddressConvention(), 0, 0) );

    theCurSel = Selection( 0, SELECTION_MAX );
    aEdAssign.GrabFocus();
    aEdAssign.SetText( aAreaStr );
    aEdAssign.SetSelection( theCurSel );
    aEdName.GrabFocus();

    aBtnAdd.Disable();
    aBtnRemove.Disable();
    if ( aEdName.GetEntryCount() > 0 )
        aBtnAdd.SetText( aStrAdd );
    UpdateChecks();
    EdModifyHdl( 0 );

    bSaved=TRUE;
    SaveData();
}

BOOL ScNameDlg::IsRefInputMode() const
{
    return aEdAssign.IsEnabled();
}

void ScNameDlg::RefInputDone( BOOL bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    EdModifyHdl(&aEdAssign);
}

void ScNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( aEdAssign.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(&aEdAssign);
        String aRefStr;
        rRef.Format( aRefStr, ABS_DREF3D, pDocP,
                     ScAddress::Details(pDocP->GetAddressConvention(), 0, 0) );
        aEdAssign.SetRefString( aRefStr );
    }
}

BOOL ScNameDlg::Close()
{
    return DoClose( ScNameDlgWrapper::GetChildWindowId() );
}

void ScNameDlg::SetActive()
{
    aEdAssign.GrabFocus();
    RefInputDone();
}

void ScNameDlg::UpdateChecks()
{
    const ScRangeData* pData = aLocalRangeName.findByName(aEdName.GetText());
    if (pData)
    {
        aBtnCriteria .Check( pData->HasType( RT_CRITERIA ) );
        aBtnPrintArea.Check( pData->HasType( RT_PRINTAREA ) );
        aBtnColHeader.Check( pData->HasType( RT_COLHEADER ) );
        aBtnRowHeader.Check( pData->HasType( RT_ROWHEADER ) );
    }

    if ( aEdName.GetText().Len() != 0 )
    {
        if ( !aFlType.IsEnabled() )
        {
            aFlType      .Enable();
            aBtnCriteria .Enable();
            aBtnPrintArea.Enable();
            aBtnColHeader.Enable();
            aBtnRowHeader.Enable();
            aFlAssign    .Enable();
            aEdAssign    .Enable();
            aRbAssign    .Enable();
        }
    }
    else if ( aFlType.IsEnabled() )
    {
        aFlType      .Disable();
        aBtnCriteria .Disable();
        aBtnPrintArea.Disable();
        aBtnColHeader.Disable();
        aBtnRowHeader.Disable();
        aFlAssign    .Disable();
        aEdAssign    .Disable();
        aRbAssign    .Disable();
    }
}

void ScNameDlg::UpdateNames()
{
    aEdName.SetUpdateMode( FALSE );
    USHORT  nNamePos = aEdName.GetTopEntry();
    aEdName.Clear();

    aEdAssign.SetText( EMPTY_STRING );

    if (aLocalRangeName.empty())
    {
        aBtnAdd.SetText( aStrAdd );
        aBtnAdd.Disable();
        aBtnRemove.Disable();
    }
    else
    {
        ScRangeName::const_iterator itr = aLocalRangeName.begin(), itrEnd = aLocalRangeName.end();
        for (; itr != itrEnd; ++itr)
        {
            if (!itr->HasType(RT_DATABASE) && !itr->HasType(RT_SHARED))
                aEdName.InsertEntry(itr->GetName());
        }
    }

    aEdName.SetUpdateMode( TRUE );
    aEdName.SetTopEntry(nNamePos);
    aEdName.Invalidate();
}

void ScNameDlg::CalcCurTableAssign( String& aAssign, ScRangeData* pRangeData )
{
    if ( pRangeData )
    {
        rtl::OUStringBuffer sBuffer;
        pRangeData->UpdateSymbol( sBuffer, theCursorPos );
        aAssign = sBuffer;
    }
    else
    {
        aAssign.Erase();
    }
}

void ScNameDlg::SaveData()
{
    mpImpl->aStrSymbol = aEdAssign.GetText();
    mpImpl->bCriteria  = aBtnCriteria.IsChecked();
    mpImpl->bPrintArea = aBtnPrintArea.IsChecked();
    mpImpl->bColHeader = aBtnColHeader.IsChecked();
    mpImpl->bRowHeader = aBtnRowHeader.IsChecked();
    mpImpl->bDirty = true;
}

void ScNameDlg::RestoreData()
{
    if ( mpImpl->bDirty )
    {
        aEdAssign.SetText( mpImpl->aStrSymbol );
        aBtnCriteria.Check( mpImpl->bCriteria );
        aBtnPrintArea.Check( mpImpl->bPrintArea );
        aBtnColHeader.Check( mpImpl->bColHeader );
        aBtnRowHeader.Check( mpImpl->bRowHeader );
        mpImpl->bDirty = false;
    }
}

IMPL_LINK( ScNameDlg, OkBtnHdl, void *, EMPTYARG )
{
    if ( aBtnAdd.IsEnabled() )
        AddBtnHdl( 0 );

    if ( !aBtnAdd.IsEnabled() && !aBtnRemove.IsEnabled() )
    {
        ScDocShell* pDocSh = pViewData->GetDocShell();
        ScDocFunc aFunc(*pDocSh);
        aFunc.ModifyRangeNames( aLocalRangeName );
        Close();
    }
    return 0;
}

IMPL_LINK( ScNameDlg, CancelBtnHdl, void *, EMPTYARG )
{
    Close();
    return 0;
}

IMPL_LINK( ScNameDlg, AddBtnHdl, void *, EMPTYARG )
{
    BOOL    bAdded    = FALSE;
    String  aNewEntry = aEdName.GetText();
    USHORT  nNamePos = aEdName.GetTopEntry();
    aNewEntry.EraseLeadingChars( ' ' );
    aNewEntry.EraseTrailingChars( ' ' );

    if ( aNewEntry.Len() > 0 )
    {
        if ( ScRangeData::IsNameValid( aNewEntry, pDoc ) )
        {
            if ( pDoc )
            {
                ScRangeData*    pNewEntry   = NULL;
                RangeType       nType       = RT_NAME;
                String          theSymbol   = aEdAssign.GetText();
                String          aStrPos;
                String          aStrArea;

                pNewEntry = new ScRangeData( pDoc,
                                             aNewEntry,
                                             theSymbol,
                                             theCursorPos,
                                             nType );
                if (pNewEntry)
                {
                    nType = nType
                    | (aBtnRowHeader .IsChecked() ? RT_ROWHEADER  : RangeType(0))
                    | (aBtnColHeader .IsChecked() ? RT_COLHEADER  : RangeType(0))
                    | (aBtnPrintArea .IsChecked() ? RT_PRINTAREA  : RangeType(0))
                    | (aBtnCriteria  .IsChecked() ? RT_CRITERIA   : RangeType(0));
                    pNewEntry->AddType(nType);
                }

                // theSymbol gueltig?
                // (= konnte theSymbol im ScRangeData-Ctor
                //    in ein Token-Array uebersetzt werden?)
                if ( 0 == pNewEntry->GetErrCode() )
                {
                    ScRangeData* pData = aLocalRangeName.findByName(aNewEntry);
                    if (pData)
                    {
                        pNewEntry->SetIndex(pData->GetIndex());
                        aLocalRangeName.erase(*pData);
                    }
                    else
                        mpImpl->Clear();

                    if ( !aLocalRangeName.insert( pNewEntry ) )
                        delete pNewEntry;

                    UpdateNames();
                    bSaved=FALSE;
                    RestoreData();
                    aEdName.SetText(EMPTY_STRING);
                    aEdName.GrabFocus();
                    UpdateChecks();
                    aBtnAdd.SetText( aStrAdd );
                    aBtnAdd.Disable();
                    aBtnRemove.Disable();

                    bAdded = TRUE;
                }
                else
                {
                    delete pNewEntry;
                    ERRORBOX( errMsgInvalidSym );
                    theCurSel = Selection( 0, SELECTION_MAX );
                    aEdAssign.GrabFocus();
                }
            }
        }
        else
        {
            ERRORBOX( ScGlobal::GetRscString(STR_INVALIDNAME) );
            aEdName.SetSelection( Selection( 0, SELECTION_MAX ) );
            aEdName.GrabFocus();
        }
    }

    aEdName.SetTopEntry(nNamePos);
    return bAdded;
}

IMPL_LINK( ScNameDlg, RemoveBtnHdl, void *, EMPTYARG )
{
    const String aStrEntry = aEdName.GetText();
    ScRangeData* pData = aLocalRangeName.findByName(aStrEntry);
    if (pData)
    {
        String aStrDelMsg = ScGlobal::GetRscString( STR_QUERY_DELENTRY );
        String aMsg       = aStrDelMsg.GetToken( 0, '#' );

        aMsg += aStrEntry;
        aMsg += aStrDelMsg.GetToken( 1, '#' );

        if ( RET_YES ==
             QueryBox( this, WinBits( WB_YES_NO | WB_DEF_YES ), aMsg ).Execute() )
        {
            aLocalRangeName.erase(*pData);
            UpdateNames();
            UpdateChecks();
            bSaved=FALSE;
            RestoreData();
            theCurSel = Selection( 0, SELECTION_MAX );
            aBtnAdd.SetText( aStrAdd );
            aBtnAdd.Disable();
            aBtnRemove.Disable();
        }
    }
    return 0;
}

IMPL_LINK( ScNameDlg, NameSelectHdl, void *, EMPTYARG )
{
    ScRangeData* pData = aLocalRangeName.findByName(aEdName.GetText());
    if (pData)
    {
        String aSymbol;
        pData->GetSymbol( aSymbol );
        CalcCurTableAssign( aSymbol, pData );
        aEdAssign.SetText( aSymbol );
        aBtnAdd.SetText( aStrModify );
        theCurSel = Selection( 0, SELECTION_MAX );
    }
    UpdateChecks();
    return 0;
}

IMPL_LINK( ScNameDlg, EdModifyHdl, Edit *, pEd )
{
    String  theName     = aEdName.GetText();
    String  theSymbol   = aEdAssign.GetText();
    BOOL    bNameFound  = (COMBOBOX_ENTRY_NOTFOUND
                           != aEdName.GetEntryPos( theName ));

    if ( pEd == &aEdName )
    {
        if ( theName.Len() == 0 )
        {
            if ( aBtnAdd.GetText() != aStrAdd )
                aBtnAdd.SetText( aStrAdd );
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            aFlAssign.Disable();
            aEdAssign.Disable();
            aRbAssign.Disable();
        }
        else
        {
            if ( bNameFound )
            {
                if ( aBtnAdd.GetText() != aStrModify )
                    aBtnAdd.SetText( aStrModify );

                aBtnRemove.Enable();

                if(!bSaved)
                {
                    bSaved=TRUE;
                    SaveData();
                }
                NameSelectHdl( 0 );
            }
            else
            {
                if ( aBtnAdd.GetText() != aStrAdd )
                    aBtnAdd.SetText( aStrAdd );
                aBtnRemove.Disable();

                bSaved=FALSE;
                RestoreData();
            }
            theSymbol = aEdAssign.GetText();

            if ( theSymbol.Len() > 0 )
                aBtnAdd.Enable();
            else
                aBtnAdd.Disable();

            aFlAssign.Enable();
            aEdAssign.Enable();
            aRbAssign.Enable();
        }
        UpdateChecks();
        theCurSel = Selection( 0, SELECTION_MAX );
    }
    else if ( pEd == &aEdAssign )
    {
        if ( (theName.Len()>0) && (theSymbol.Len()>0) )
        {
            aBtnAdd.Enable();
            if ( bNameFound )
                aBtnRemove.Enable();
        }
        else
        {
            aBtnAdd.Disable();
            aBtnRemove.Disable();
        }
    }
    return 0;
}

IMPL_LINK( ScNameDlg, AssignGetFocusHdl, void *, EMPTYARG )
{
    EdModifyHdl( &aEdAssign );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
