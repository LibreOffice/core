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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>


#include "view.hxx"
#include "basesh.hxx"
#include "wrtsh.hxx"        //
#include "cmdid.h"
#include "bookmark.hxx"     // SwInsertBookmarkDlg
#include "IMark.hxx"
#include "bookmark.hrc"
#include "misc.hrc"

const String BookmarkCombo::aForbiddenChars = String::CreateFromAscii("/\\@:*?\";,.#");

IMPL_LINK( SwInsertBookmarkDlg, ModifyHdl, BookmarkCombo *, pBox )
{
    BOOL bSelEntries = pBox->GetSelectEntryCount() != 0;
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    if(!bSelEntries)
    {
        String sTmp = pBox->GetText();
        USHORT nLen = sTmp.Len();
        String sMsg;
        for(USHORT i = 0; i < BookmarkCombo::aForbiddenChars.Len(); i++)
        {
            USHORT nTmpLen = sTmp.Len();
            sTmp.EraseAllChars(BookmarkCombo::aForbiddenChars.GetChar(i));
            if(sTmp.Len() != nTmpLen)
                sMsg += BookmarkCombo::aForbiddenChars.GetChar(i);
        }
        if(sTmp.Len() != nLen)
        {
            pBox->SetText(sTmp);
            String sWarning(sRemoveWarning);
            sWarning += sMsg;
            InfoBox(this, sWarning).Execute();
        }


    }

    aOkBtn.Enable(!bSelEntries);    // neue Textmarke
    aDeleteBtn.Enable(bSelEntries); // loeschbar?

    return 0;
}

/*------------------------------------------------------------------------
     Beschreibung: Callback zum Loeschen einer Textmarke
 -----------------------------------------------------------------------*/
IMPL_LINK( SwInsertBookmarkDlg, DeleteHdl, Button *, EMPTYARG )
{
    // Textmarken aus der ComboBox entfernen

    for (USHORT i = aBookmarkBox.GetSelectEntryCount(); i; i-- )
        aBookmarkBox.RemoveEntry(aBookmarkBox.GetSelectEntryPos(i - 1));

    aBookmarkBox.SetText(aEmptyStr);
    aDeleteBtn.Enable(FALSE);   // keine weiteren Eintraege vorhanden

    aOkBtn.Enable();            // Im OK Handler wird geloescht
    return 0;
}

/*------------------------------------------------------------------------
     Beschreibung: Callback fuer OKButton. Fuegt eine neue Textmarke
     an die akt. Position ein. Geloeschte Textmarken werden auch am Modell
     entfernt.
 -----------------------------------------------------------------------*/
void SwInsertBookmarkDlg::Apply()
{
    //at first remove deleted bookmarks to prevent multiple bookmarks with the same
    //name
    for (USHORT nCount = aBookmarkBox.GetRemovedCount(); nCount > 0; nCount--)
    {
        String sRemoved = aBookmarkBox.GetRemovedEntry( nCount -1 ).GetName();
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark( pMarkAccess->findMark(sRemoved) );
        SfxRequest aReq( rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK );
        aReq.AppendItem( SfxStringItem( FN_DELETE_BOOKMARK, sRemoved ) );
        aReq.Done();
    }

    // Textmarke einfuegen
    USHORT      nLen = aBookmarkBox.GetText().Len();
    SwBoxEntry  aTmpEntry(aBookmarkBox.GetText(), 0 );

    if ( nLen && (aBookmarkBox.GetEntryPos(aTmpEntry) == COMBOBOX_ENTRY_NOTFOUND) )
    {
        String sEntry(aBookmarkBox.GetText());
        sEntry.EraseAllChars(aBookmarkBox.GetMultiSelectionSeparator());

        rSh.SetBookmark( KeyCode(), sEntry, aEmptyStr );
        rReq.AppendItem( SfxStringItem( FN_INSERT_BOOKMARK, sEntry ) );
        rReq.Done();
    }

    if ( !rReq.IsDone() )
        rReq.Ignore();

}

/*------------------------------------------------------------------------
     Beschreibung: CTOR
 -----------------------------------------------------------------------*/
SwInsertBookmarkDlg::SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rS, SfxRequest& rRequest ) :

    SvxStandardDialog(pParent,SW_RES(DLG_INSERT_BOOKMARK)),

    aBookmarkBox(this,SW_RES(CB_BOOKMARK)),
    aBookmarkFl(this,SW_RES(FL_BOOKMARK)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aDeleteBtn(this,SW_RES(BT_DELETE)),
    rSh( rS ),
    rReq( rRequest )
{
    aBookmarkBox.SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    aBookmarkBox.EnableMultiSelection(TRUE);
    aBookmarkBox.EnableAutocomplete( TRUE, TRUE );

    aDeleteBtn.SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));

    // Combobox mit vorhandenen Bookmarks fuellen
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    USHORT nId = 0;
    for( IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ppBookmark++)
    {
        if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
            aBookmarkBox.InsertEntry( SwBoxEntry( ppBookmark->get()->GetName(), nId++ ) );
    }
    FreeResource();
    sRemoveWarning = String(SW_RES(STR_REMOVE_WARNING));
}

SwInsertBookmarkDlg::~SwInsertBookmarkDlg()
{
}

BookmarkCombo::BookmarkCombo( Window* pWin, const ResId& rResId ) :
    SwComboBox(pWin, rResId)
{
}

USHORT BookmarkCombo::GetFirstSelEntryPos() const
{
    return GetSelEntryPos(0);
}

USHORT BookmarkCombo::GetNextSelEntryPos(USHORT nPos) const
{
    return GetSelEntryPos(nPos + 1);
}

USHORT BookmarkCombo::GetSelEntryPos(USHORT nPos) const
{
    sal_Unicode cSep = GetMultiSelectionSeparator();

    USHORT nCnt = GetText().GetTokenCount(cSep);

    for (; nPos < nCnt; nPos++)
    {
        String sEntry(GetText().GetToken(nPos, cSep));
        sEntry.EraseLeadingChars();
        sEntry.EraseTrailingChars();
        if (GetEntryPos(sEntry) != COMBOBOX_ENTRY_NOTFOUND)
            return nPos;
    }

    return COMBOBOX_ENTRY_NOTFOUND;
}

USHORT BookmarkCombo::GetSelectEntryCount() const
{
    USHORT nCnt = 0;

    USHORT nPos = GetFirstSelEntryPos();
    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return nCnt;
}

/*------------------------------------------------------------------------
     Beschreibung: Position in der Listbox (der ComboBox)
 -----------------------------------------------------------------------*/
USHORT BookmarkCombo::GetSelectEntryPos( USHORT nSelIndex ) const
{
    USHORT nCnt = 0;
    USHORT nPos = GetFirstSelEntryPos();

    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        if (nSelIndex == nCnt)
        {
            sal_Unicode cSep = GetMultiSelectionSeparator();
            String sEntry(GetText().GetToken(nPos, cSep));
            sEntry.EraseLeadingChars();
            sEntry.EraseTrailingChars();

            return GetEntryPos(sEntry);
        }
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return COMBOBOX_ENTRY_NOTFOUND;
}

long BookmarkCombo::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    if( EVENT_KEYINPUT == rNEvt.GetType() &&
         rNEvt.GetKeyEvent()->GetCharCode() )
    {
        String sKey( rNEvt.GetKeyEvent()->GetCharCode() );
        if(STRING_NOTFOUND != aForbiddenChars.Search(sKey))
            nHandled = 1;
    }
    if(!nHandled)
        nHandled = SwComboBox::PreNotify( rNEvt );
    return nHandled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
