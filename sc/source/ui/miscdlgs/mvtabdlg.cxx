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

#include <vcl/msgbox.hxx>

#include "mvtabdlg.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "miscdlgs.hrc"
#include "global.hxx"
#include "scresid.hxx"
#include "globstr.hrc"

ScMoveTableDlg::ScMoveTableDlg(vcl::Window* pParent, const OUString& rDefault)

    :   ModalDialog ( pParent, "MoveCopySheetDialog", "modules/scalc/ui/movecopysheet.ui" ),
        maDefaultName( rDefault ),
        mnCurrentDocPos( 0 ),
        nDocument   ( 0 ),
        nTable      ( 0 ),
        bCopyTable  ( false ),
        bRenameTable( false ),
        mbEverEdited( false )
{
    get(pBtnMove, "move");
    get(pBtnCopy, "copy");
    get(pLbDoc, "toDocument");

    assert(pLbDoc->GetEntryCount() == 2);
    msCurrentDoc = pLbDoc->GetEntry(0);
    msNewDoc = pLbDoc->GetEntry(1);
    pLbDoc->Clear();
    assert(pLbDoc->GetEntryCount() == 0);

    get(pLbTable, "insertBefore");
    pLbTable->set_height_request(pLbTable->GetTextHeight() * 8);
    get(pEdTabName, "newName");
    get(pFtWarn, "newNameWarn");
    get(pBtnOk, "ok");

    msStrTabNameUsed = get<FixedText>("warnunused")->GetText();
    msStrTabNameEmpty = get<FixedText>("warnempty")->GetText();
    msStrTabNameInvalid = get<FixedText>("warninvalid")->GetText();

    Init();
}

ScMoveTableDlg::~ScMoveTableDlg()
{
    disposeOnce();
}

void ScMoveTableDlg::dispose()
{
    pBtnMove.clear();
    pBtnCopy.clear();
    pLbDoc.clear();
    pLbTable.clear();
    pEdTabName.clear();
    pFtWarn.clear();
    pBtnOk.clear();
    ModalDialog::dispose();
}

void ScMoveTableDlg::GetTabNameString( OUString& rString ) const
{
    rString = pEdTabName->GetText();
}

void ScMoveTableDlg::SetForceCopyTable()
{
    pBtnCopy->Check();
    pBtnMove->Disable();
    pBtnCopy->Disable();
}

void ScMoveTableDlg::EnableRenameTable(bool bFlag)
{
    bRenameTable = bFlag;
    pEdTabName->Enable(bFlag);
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

    if (!pEdTabName->IsEnabled())
    {
        pEdTabName->SetText(OUString());
        return;
    }

    bool bVal = pBtnCopy->IsChecked();
    if (bVal)
    {
        // copy
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
        {
            OUString aStr = maDefaultName;
            pDoc->CreateValidTabName(aStr);
            pEdTabName->SetText(aStr);
        }
        else
            pEdTabName->SetText(maDefaultName);
    }
    else
        // move
        pEdTabName->SetText(maDefaultName);

    CheckNewTabName();
}

void ScMoveTableDlg::CheckNewTabName()
{
    const OUString aNewName = pEdTabName->GetText();
    if (aNewName.isEmpty())
    {
        // New sheet name is empty.  This is not good.
        pFtWarn->Show(true);
        pFtWarn->SetControlBackground(Color(COL_YELLOW));
        pFtWarn->SetText(msStrTabNameEmpty);
        pBtnOk->Disable();
        return;
    }

    if (!ScDocument::ValidTabName(aNewName))
    {
        // New sheet name contains invalid characters.
        pFtWarn->Show(true);
        pFtWarn->SetControlBackground(Color(COL_YELLOW));
        pFtWarn->SetText(msStrTabNameInvalid);
        pBtnOk->Disable();
        return;
    }

    bool bMoveInCurrentDoc = (pBtnMove->IsChecked() && IsCurrentDocSelected());
    bool bFound = false;
    const sal_Int32 nLast = pLbTable->GetEntryCount();
    for ( sal_uInt16 i=0; i<nLast && !bFound; ++i )
    {
        if ( aNewName.equals(pLbTable->GetEntry(i)) )
        {
            // Only for move within same document the same name is allowed.
            if (!bMoveInCurrentDoc || !maDefaultName.equals( pEdTabName->GetText()))
                bFound = true;
        }
    }

    if ( bFound )
    {
        pFtWarn->Show(true);
        pFtWarn->SetControlBackground(Color(COL_YELLOW));
        pFtWarn->SetText(msStrTabNameUsed);
        pBtnOk->Disable();
    }
    else
    {
        pFtWarn->Hide();
        pFtWarn->SetControlBackground();
        pFtWarn->SetText(OUString());
        pBtnOk->Enable();
    }
}

ScDocument* ScMoveTableDlg::GetSelectedDoc()
{
    sal_Int32 nPos = pLbDoc->GetSelectEntryPos();
    return static_cast<ScDocument*>(pLbDoc->GetEntryData(nPos));
}

bool ScMoveTableDlg::IsCurrentDocSelected() const
{
    return pLbDoc->GetSelectEntryPos() == mnCurrentDocPos;
}

void ScMoveTableDlg::Init()
{
    pBtnOk->SetClickHdl   ( LINK( this, ScMoveTableDlg, OkHdl ) );
    pLbDoc->SetSelectHdl  ( LINK( this, ScMoveTableDlg, SelHdl ) );
    pBtnCopy->SetToggleHdl( LINK( this, ScMoveTableDlg, CheckBtnHdl ) );
    pEdTabName->SetModifyHdl( LINK( this, ScMoveTableDlg, CheckNameHdl ) );
    pBtnMove->Check();
    pBtnCopy->Check( false );
    pEdTabName->Enable(false);
    pFtWarn->Hide();
    InitDocListBox();
    SelHdl( *pLbDoc.get() );
}

void ScMoveTableDlg::InitDocListBox()
{
    SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
    ScDocShell*     pScSh   = NULL;
    sal_uInt16          nSelPos = 0;
    sal_uInt16          i       = 0;
    OUString          aEntryName;

    pLbDoc->Clear();
    pLbDoc->SetUpdateMode( false );

    while ( pSh )
    {
        pScSh = dynamic_cast<ScDocShell*>( pSh  );

        if ( pScSh )
        {
            aEntryName = pScSh->GetTitle();

            if ( pScSh == SfxObjectShell::Current() )
            {
                mnCurrentDocPos = nSelPos = i;
                aEntryName += " ";
                aEntryName += msCurrentDoc;
            }

            pLbDoc->InsertEntry( aEntryName, i );
            pLbDoc->SetEntryData( i, static_cast<void*>(&pScSh->GetDocument()) );

            i++;
        }
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    pLbDoc->SetUpdateMode( true );
    pLbDoc->InsertEntry(msNewDoc);
    pLbDoc->SelectEntryPos( nSelPos );
}

// Handler:

IMPL_LINK_TYPED( ScMoveTableDlg, CheckBtnHdl, RadioButton&, rBtn, void )
{
    if (&rBtn == pBtnCopy)
        ResetRenameInput();
}

IMPL_LINK_NOARG_TYPED(ScMoveTableDlg, OkHdl, Button*, void)
{
    const sal_Int32 nDocSel  = pLbDoc->GetSelectEntryPos();
    const sal_Int32 nDocLast = pLbDoc->GetEntryCount()-1;
    const sal_Int32 nTabSel  = pLbTable->GetSelectEntryPos();
    const sal_Int32 nTabLast = pLbTable->GetEntryCount()-1;

    nDocument   = (nDocSel != nDocLast) ? nDocSel : SC_DOC_NEW;
    nTable      = (nTabSel != nTabLast) ? static_cast<SCTAB>(nTabSel) : SC_TAB_APPEND;
    bCopyTable  = pBtnCopy->IsChecked();

    if (bCopyTable)
    {
        // Return an empty string when the new name is the same as the
        // automatic name assigned by the document.
        OUString aCopyName = maDefaultName;
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
            pDoc->CreateValidTabName(aCopyName);
        if (aCopyName == pEdTabName->GetText())
            pEdTabName->SetText( OUString() );
    }
    else
    {
        // Return an empty string, when the new name is the same as the
        // original name.
        if (maDefaultName.equals(pEdTabName->GetText()))
            pEdTabName->SetText(OUString());
    }

    EndDialog( RET_OK );
}

IMPL_LINK_TYPED( ScMoveTableDlg, SelHdl, ListBox&, rLb, void )
{
    if ( &rLb == pLbDoc )
    {
        ScDocument* pDoc = GetSelectedDoc();
        OUString aName;

        pLbTable->Clear();
        pLbTable->SetUpdateMode( false );
        if ( pDoc )
        {
            SCTAB nLast = pDoc->GetTableCount()-1;
            for ( SCTAB i=0; i<=nLast; i++ )
            {
                pDoc->GetName( i, aName );
                pLbTable->InsertEntry( aName, static_cast<sal_uInt16>(i) );
            }
        }
        pLbTable->InsertEntry( ScGlobal::GetRscString(STR_MOVE_TO_END) );
        pLbTable->SetUpdateMode( true );
        pLbTable->SelectEntryPos( 0 );
        ResetRenameInput();
    }
}

IMPL_LINK( ScMoveTableDlg, CheckNameHdl, Edit *, pEdt )
{
    if ( pEdt == pEdTabName )
    {
        mbEverEdited = true;
        CheckNewTabName();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
