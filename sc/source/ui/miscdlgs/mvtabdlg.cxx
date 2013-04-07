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

//==================================================================

ScMoveTableDlg::ScMoveTableDlg(Window* pParent, const OUString& rDefault)

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

        maStrTabNameUsed( SC_RESSTR(STR_TABNAME_WARN_USED) ),
        maStrTabNameEmpty( SC_RESSTR(STR_TABNAME_WARN_EMPTY) ),
        maStrTabNameInvalid( SC_RESSTR(STR_TABNAME_WARN_INVALID) ),
        //
        maDefaultName( rDefault ),
        mnCurrentDocPos( 0 ),
        nDocument   ( 0 ),
        nTable      ( 0 ),
        bCopyTable  ( false ),
        bRenameTable( false ),
        mbEverEdited( false )
{
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

void ScMoveTableDlg::GetTabNameString( OUString& rString ) const
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
    {
        // Don't reset the name when the sheet name has ever been edited.
        // But check the name, as this is also called for change of copy/move
        // buttons and document listbox selection.
        CheckNewTabName();
        return;
    }

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
            OUString aStr = maDefaultName;
            pDoc->CreateValidTabName(aStr);
            aEdTabName.SetText(aStr);
        }
        else
            aEdTabName.SetText(maDefaultName);
    }
    else
        // move
        aEdTabName.SetText(maDefaultName);

    CheckNewTabName();
}

void ScMoveTableDlg::CheckNewTabName()
{
    const OUString aNewName = aEdTabName.GetText();
    if (aNewName.isEmpty())
    {
        // New sheet name is empty.  This is not good.
        aFtWarn.SetText(maStrTabNameEmpty);
        aFtWarn.Show();
        aBtnOk.Disable();
        return;
    }

    if (!ScDocument::ValidTabName(aNewName))
    {
        // New sheet name contains invalid characters.
        aFtWarn.SetText(maStrTabNameInvalid);
        aFtWarn.Show();
        aBtnOk.Disable();
        return;
    }

    bool bMoveInCurrentDoc = (aBtnMove.IsChecked() && IsCurrentDocSelected());
    bool bFound = false;
    sal_uInt16 nLast  = aLbTable.GetEntryCount() - 1;
    for ( sal_uInt16 i=0; i<=nLast && !bFound; ++i )
    {
        if ( aNewName.equals(aLbTable.GetEntry(i)) )
        {
            // Only for move within same document the same name is allowed.
            if (!bMoveInCurrentDoc || !maDefaultName.equals( aEdTabName.GetText()))
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

bool ScMoveTableDlg::IsCurrentDocSelected() const
{
    return aLbDoc.GetSelectEntryPos() == mnCurrentDocPos;
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
                mnCurrentDocPos = nSelPos = i;
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

IMPL_LINK_NOARG(ScMoveTableDlg, OkHdl)
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
        OUString aCopyName = maDefaultName;
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
            pDoc->CreateValidTabName(aCopyName);
        if (aCopyName == OUString(aEdTabName.GetText()))
            aEdTabName.SetText( OUString() );
    }
    else
    {
        // Return an empty string, when the new name is the same as the
        // original name.
        if (maDefaultName.equals(aEdTabName.GetText()))
            aEdTabName.SetText(OUString());
    }

    EndDialog( RET_OK );

    return 0;
}

IMPL_LINK( ScMoveTableDlg, SelHdl, ListBox *, pLb )
{
    if ( pLb == &aLbDoc )
    {
        ScDocument* pDoc = GetSelectedDoc();
        OUString aName;

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
