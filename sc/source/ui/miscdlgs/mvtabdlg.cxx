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

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "mvtabdlg.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "miscdlgs.hrc"
#include "global.hxx"
#include "scresid.hxx"
#include "globstr.hrc"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef ScResId
#define ScResId(x) #x
#undef ModalDialog
#define ModalDialog( parent, id ) Dialog( parent, "move-copy-sheet.xml", id )
#endif /* ENABLE_LAYOUT */

//==================================================================

ScMoveTableDlg::ScMoveTableDlg( Window*       pParent,
                                const String& rDefault )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_MOVETAB ) ),
        //
        aFlAction   ( this, ScResId( FL_ACTION ) ),
        aBtnMove    ( this, ScResId( BTN_MOVE ) ),
        aBtnCopy    ( this, ScResId( BTN_COPY ) ),
        aFlLocation ( this, ScResId( FL_LOCATION ) ),
        aFtDoc      ( this, ScResId( FT_DEST ) ),
        aLbDoc      ( this, ScResId( LB_DEST ) ),
        aFtTable    ( this, ScResId( FT_INSERT ) ),
        aLbTable    ( this, ScResId( LB_INSERT ) ),
        aFlName     ( this, ScResId( FL_NAME ) ),
        aFtTabName  ( this, ScResId( FT_TABNAME ) ),
        aEdTabName  ( this, ScResId( ED_INPUT ) ),
        aFtWarn     ( this, ScResId( FT_TABNAME_WARN ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),

        maStrTabNameUsed( ScResId(STR_TABNAME_WARN_USED) ),
        maStrTabNameEmpty( ScResId(STR_TABNAME_WARN_EMPTY) ),
        maStrTabNameInvalid( ScResId(STR_TABNAME_WARN_INVALID) ),
        //
        mrDefaultName( rDefault ),
        nDocument   ( 0 ),
        nTable      ( 0 ),
        bCopyTable  ( false ),
        bRenameTable( false ),
        mbEverEdited( false )
{
#if ENABLE_LAYOUT
#undef ScResId
    SetHelpId (FID_TAB_MOVE);
#endif /* ENABLE_LAYOUT */
    Init();
    FreeResource();
}

//------------------------------------------------------------------------

ScMoveTableDlg::~ScMoveTableDlg()
{
}

//------------------------------------------------------------------------

sal_uInt16 ScMoveTableDlg::GetSelectedDocument () const { return nDocument;  }

SCTAB ScMoveTableDlg::GetSelectedTable    () const { return nTable;     }

bool   ScMoveTableDlg::GetCopyTable        () const { return bCopyTable; }

bool   ScMoveTableDlg::GetRenameTable        () const { return bRenameTable; }

void ScMoveTableDlg::GetTabNameString( String& rString ) const
{
    rString = aEdTabName.GetText();
}

void ScMoveTableDlg::SetForceCopyTable()
{
    aBtnCopy.Check(true);
    aBtnMove.Disable();
    aBtnCopy.Disable();
}

void ScMoveTableDlg::EnableCopyTable(sal_Bool bFlag)
{
    if(bFlag)
        aBtnCopy.Enable();
    else
        aBtnCopy.Disable();
}

void ScMoveTableDlg::EnableRenameTable(sal_Bool bFlag)
{
    bRenameTable = bFlag;
    aEdTabName.Enable(bFlag);
    aFtTabName.Enable(bFlag);
    ResetRenameInput();
}

void ScMoveTableDlg::ResetRenameInput()
{
    if (mbEverEdited)
        // Don't reset the name when the sheet name has ever been edited.
        return;

    if (!aEdTabName.IsEnabled())
    {
        aEdTabName.SetText(String());
        return;
    }

    bool bVal = aBtnCopy.IsChecked();
    if (bVal)
    {
        // copy
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
        {
            String aStr = mrDefaultName;
            pDoc->CreateValidTabName(aStr);
            aEdTabName.SetText(aStr);
        }
        else
            aEdTabName.SetText(mrDefaultName);
    }
    else
        // move
        aEdTabName.SetText(mrDefaultName);

    CheckNewTabName();
}

void ScMoveTableDlg::CheckNewTabName()
{
    const String& rNewName = aEdTabName.GetText();
    if (!rNewName.Len())
    {
        // New sheet name is empty.  This is not good.
        aFtWarn.SetText(maStrTabNameEmpty);
        aFtWarn.Show();
        aBtnOk.Disable();
        return;
    }

    if (!ScDocument::ValidTabName(rNewName))
    {
        // New sheet name contains invalid characters.
        aFtWarn.SetText(maStrTabNameInvalid);
        aFtWarn.Show();
        aBtnOk.Disable();
        return;
    }

    bool   bFound = false;
    sal_uInt16 nLast  = aLbTable.GetEntryCount() - 1;
    for ( sal_uInt16 i=0; i<=nLast; ++i )
    {
        if ( rNewName == aLbTable.GetEntry( i ) )
        {
            if( ( aBtnMove.IsChecked() ) &&
                ( aLbDoc.GetSelectEntryPos() == 0 ) &&
                ( aEdTabName.GetText() == mrDefaultName) )

                // Move inside same document, thus same name is allowed.
                bFound = false;
            else
                bFound = true;
        }
    }

    if ( bFound )
    {
        aFtWarn.SetText(maStrTabNameUsed);
        aFtWarn.Show();
        aBtnOk.Disable();
    }
    else
    {
        aFtWarn.Hide();
        aBtnOk.Enable();
    }
}

ScDocument* ScMoveTableDlg::GetSelectedDoc()
{
    sal_uInt16 nPos = aLbDoc.GetSelectEntryPos();
    return static_cast<ScDocument*>(aLbDoc.GetEntryData(nPos));
}

//------------------------------------------------------------------------

void ScMoveTableDlg::Init()
{
    aBtnOk.SetClickHdl   ( LINK( this, ScMoveTableDlg, OkHdl ) );
    aLbDoc.SetSelectHdl  ( LINK( this, ScMoveTableDlg, SelHdl ) );
    aBtnCopy.SetToggleHdl( LINK( this, ScMoveTableDlg, CheckBtnHdl ) );
    aEdTabName.SetModifyHdl( LINK( this, ScMoveTableDlg, CheckNameHdl ) );
    aBtnMove.Check( true );
    aBtnCopy.Check( false );
    aEdTabName.Enable(false);
    aFtWarn.SetControlBackground( Color( COL_YELLOW ) );
    aFtWarn.Hide();
    InitDocListBox();
    SelHdl( &aLbDoc );
}

//------------------------------------------------------------------------

void ScMoveTableDlg::InitDocListBox()
{
    SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
    ScDocShell*     pScSh   = NULL;
    sal_uInt16          nSelPos = 0;
    sal_uInt16          i       = 0;
    String          aEntryName;

    aLbDoc.Clear();
    aLbDoc.SetUpdateMode( false );

    while ( pSh )
    {
        pScSh = PTR_CAST( ScDocShell, pSh );

        if ( pScSh )
        {
            aEntryName = pScSh->GetTitle();

            if ( pScSh == SfxObjectShell::Current() )
            {
                nSelPos = i;
                aEntryName += sal_Unicode( ' ' );
                aEntryName += String( ScResId( STR_CURRENTDOC ) );
            }

            aLbDoc.InsertEntry( aEntryName, i );
            aLbDoc.SetEntryData( i, (void*)pScSh->GetDocument() );

            i++;
        }
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    aLbDoc.SetUpdateMode( sal_True );
    aLbDoc.InsertEntry( String( ScResId( STR_NEWDOC ) ) );
    aLbDoc.SelectEntryPos( nSelPos );
}

//------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScMoveTableDlg, CheckBtnHdl, void *, pBtn )
{
    if (pBtn == &aBtnCopy)
        ResetRenameInput();

    return 0;
}

IMPL_LINK( ScMoveTableDlg, OkHdl, void *, EMPTYARG )
{
    sal_uInt16  nDocSel     = aLbDoc.GetSelectEntryPos();
    sal_uInt16  nDocLast    = aLbDoc.GetEntryCount()-1;
    sal_uInt16  nTabSel     = aLbTable.GetSelectEntryPos();
    sal_uInt16  nTabLast    = aLbTable.GetEntryCount()-1;

    nDocument   = (nDocSel != nDocLast) ? nDocSel : SC_DOC_NEW;
    nTable      = (nTabSel != nTabLast) ? static_cast<SCTAB>(nTabSel) : SC_TAB_APPEND;
    bCopyTable  = aBtnCopy.IsChecked();

    if (bCopyTable)
    {
        // Return an empty string when the new name is the same as the
        // automatic name assigned by the document.
        String aCopyName = mrDefaultName;
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
            pDoc->CreateValidTabName(aCopyName);
        if (aCopyName == aEdTabName.GetText())
            aEdTabName.SetText( String() );
    }
    else
    {
        // Return an empty string, when the new name is the same as the
        // original name.
        if( mrDefaultName == aEdTabName.GetText() )
            aEdTabName.SetText( String() );
    }

    EndDialog( RET_OK );

    return 0;
}

IMPL_LINK( ScMoveTableDlg, SelHdl, ListBox *, pLb )
{
    if ( pLb == &aLbDoc )
    {
        ScDocument* pDoc = GetSelectedDoc();
        String      aName;

        aLbTable.Clear();
        aLbTable.SetUpdateMode( false );
        if ( pDoc )
        {
            SCTAB nLast = pDoc->GetTableCount()-1;
            for ( SCTAB i=0; i<=nLast; i++ )
            {
                pDoc->GetName( i, aName );
                aLbTable.InsertEntry( aName, static_cast<sal_uInt16>(i) );
            }
        }
        aLbTable.InsertEntry( ScGlobal::GetRscString(STR_MOVE_TO_END) );
        aLbTable.SetUpdateMode( sal_True );
        aLbTable.SelectEntryPos( 0 );
        ResetRenameInput();
    }

    return 0;
}

IMPL_LINK( ScMoveTableDlg, CheckNameHdl, Edit *, pEdt )
{
    if ( pEdt == &aEdTabName )
    {
        mbEverEdited = true;
        CheckNewTabName();
    }

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
