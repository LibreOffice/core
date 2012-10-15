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

#include <comphelper/string.hxx>
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

const String BookmarkCombo::aForbiddenChars = rtl::OUString("/\\@:*?\";,.#");

IMPL_LINK( SwInsertBookmarkDlg, ModifyHdl, BookmarkCombo *, pBox )
{
    sal_Bool bSelEntries = pBox->GetSelectEntryCount() != 0;
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    if(!bSelEntries)
    {
        String sTmp = pBox->GetText();
        sal_uInt16 nLen = sTmp.Len();
        String sMsg;
        for(sal_uInt16 i = 0; i < BookmarkCombo::aForbiddenChars.Len(); i++)
        {
            sal_uInt16 nTmpLen = sTmp.Len();
            sTmp = comphelper::string::remove(sTmp, BookmarkCombo::aForbiddenChars.GetChar(i));
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

    aOkBtn.Enable(!bSelEntries);    // new text mark
    aDeleteBtn.Enable(bSelEntries); // deletable?

    return 0;
}

/*------------------------------------------------------------------------
     Description: callback to delete a text mark
 -----------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwInsertBookmarkDlg, DeleteHdl)
{
    // remove text marks from the ComboBox

    for (sal_uInt16 i = aBookmarkBox.GetSelectEntryCount(); i; i-- )
        aBookmarkBox.RemoveEntry(aBookmarkBox.GetSelectEntryPos(i - 1));

    aBookmarkBox.SetText(aEmptyStr);
    aDeleteBtn.Enable(sal_False);   // no further entries there

    aOkBtn.Enable();            // the OK handler deletes
    return 0;
}

/*------------------------------------------------------------------------
     Description: callback for OKButton. Inserts a new text mark to the
     current position. Deleted text marks are also deleted in the model.
 -----------------------------------------------------------------------*/
void SwInsertBookmarkDlg::Apply()
{
    //at first remove deleted bookmarks to prevent multiple bookmarks with the same
    //name
    for (sal_uInt16 nCount = aBookmarkBox.GetRemovedCount(); nCount > 0; nCount--)
    {
        String sRemoved = aBookmarkBox.GetRemovedEntry( nCount -1 ).GetName();
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark( pMarkAccess->findMark(sRemoved) );
        SfxRequest aReq( rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK );
        aReq.AppendItem( SfxStringItem( FN_DELETE_BOOKMARK, sRemoved ) );
        aReq.Done();
    }

    // insert text mark
    sal_uInt16      nLen = aBookmarkBox.GetText().Len();
    SwBoxEntry  aTmpEntry(aBookmarkBox.GetText(), 0 );

    if ( nLen && (aBookmarkBox.GetEntryPos(aTmpEntry) == COMBOBOX_ENTRY_NOTFOUND) )
    {
        String sEntry(comphelper::string::remove(aBookmarkBox.GetText(),
            aBookmarkBox.GetMultiSelectionSeparator()));

        rSh.SetBookmark( KeyCode(), sEntry, aEmptyStr );
        rReq.AppendItem( SfxStringItem( FN_INSERT_BOOKMARK, sEntry ) );
        rReq.Done();
    }

    if ( !rReq.IsDone() )
        rReq.Ignore();

}

/*------------------------------------------------------------------------
     Description: CTOR
 -----------------------------------------------------------------------*/
SwInsertBookmarkDlg::SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rS, SfxRequest& rRequest ) :

    SvxStandardDialog(pParent,SW_RES(DLG_INSERT_BOOKMARK)),
    aBookmarkFl(this,SW_RES(FL_BOOKMARK)),
    aBookmarkBox(this,SW_RES(CB_BOOKMARK)),
    aOkBtn(this,SW_RES(BT_OK)),
    aCancelBtn(this,SW_RES(BT_CANCEL)),
    aDeleteBtn(this,SW_RES(BT_DELETE)),
    rSh( rS ),
    rReq( rRequest )
{
    aBookmarkBox.SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    aBookmarkBox.EnableMultiSelection(sal_True);
    aBookmarkBox.EnableAutocomplete( sal_True, sal_True );

    aDeleteBtn.SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));

    // fill Combobox with existing bookmarks
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    sal_uInt16 nId = 0;
    for( IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ++ppBookmark)
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

sal_uInt16 BookmarkCombo::GetFirstSelEntryPos() const
{
    return GetSelEntryPos(0);
}

sal_uInt16 BookmarkCombo::GetNextSelEntryPos(sal_uInt16 nPos) const
{
    return GetSelEntryPos(nPos + 1);
}

sal_uInt16 BookmarkCombo::GetSelEntryPos(sal_uInt16 nPos) const
{
    sal_Unicode cSep = GetMultiSelectionSeparator();

    sal_uInt16 nCnt = comphelper::string::getTokenCount(GetText(), cSep);

    for (; nPos < nCnt; nPos++)
    {
        String sEntry(comphelper::string::strip(GetText().GetToken(nPos, cSep), ' '));
        if (GetEntryPos(sEntry) != COMBOBOX_ENTRY_NOTFOUND)
            return nPos;
    }

    return COMBOBOX_ENTRY_NOTFOUND;
}

sal_uInt16 BookmarkCombo::GetSelectEntryCount() const
{
    sal_uInt16 nCnt = 0;

    sal_uInt16 nPos = GetFirstSelEntryPos();
    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        nPos = GetNextSelEntryPos(nPos);
        nCnt++;
    }

    return nCnt;
}

/*------------------------------------------------------------------------
     Description: position inside of the listbox (the ComboBox)
 -----------------------------------------------------------------------*/
sal_uInt16 BookmarkCombo::GetSelectEntryPos( sal_uInt16 nSelIndex ) const
{
    sal_uInt16 nCnt = 0;
    sal_uInt16 nPos = GetFirstSelEntryPos();

    while (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        if (nSelIndex == nCnt)
        {
            sal_Unicode cSep = GetMultiSelectionSeparator();
            String sEntry(comphelper::string::strip(GetText().GetToken(nPos, cSep), ' '));
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
        rtl::OUString sKey( rNEvt.GetKeyEvent()->GetCharCode() );
        if(STRING_NOTFOUND != aForbiddenChars.Search(sKey))
            nHandled = 1;
    }
    if(!nHandled)
        nHandled = SwComboBox::PreNotify( rNEvt );
    return nHandled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
