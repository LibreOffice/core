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
#include "globals.hrc"

const String BookmarkCombo::aForbiddenChars = OUString("/\\@:*?\";,.#");

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

    m_pOkBtn->Enable(!bSelEntries);    // new text mark
    m_pDeleteBtn->Enable(bSelEntries); // deletable?

    return 0;
}

/*------------------------------------------------------------------------
     Description: callback to delete a text mark
 -----------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwInsertBookmarkDlg, DeleteHdl)
{
    // remove text marks from the ComboBox

    for (sal_uInt16 i = m_pBookmarkBox->GetSelectEntryCount(); i; i-- )
        m_pBookmarkBox->RemoveEntryAt(m_pBookmarkBox->GetSelectEntryPos(i - 1));

    m_pBookmarkBox->SetText(aEmptyStr);
    m_pDeleteBtn->Enable(sal_False);   // no further entries there

    m_pOkBtn->Enable();            // the OK handler deletes
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
    for (sal_uInt16 nCount = m_pBookmarkBox->GetRemovedCount(); nCount > 0; nCount--)
    {
        String sRemoved = m_pBookmarkBox->GetRemovedEntry( nCount -1 ).GetName();
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark( pMarkAccess->findMark(sRemoved) );
        SfxRequest aReq( rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK );
        aReq.AppendItem( SfxStringItem( FN_DELETE_BOOKMARK, sRemoved ) );
        aReq.Done();
    }

    // insert text mark
    SwBoxEntry  aTmpEntry(m_pBookmarkBox->GetText(), 0 );

    if (!m_pBookmarkBox->GetText().isEmpty() &&
        (m_pBookmarkBox->GetSwEntryPos(aTmpEntry) == COMBOBOX_ENTRY_NOTFOUND))
    {
        String sEntry(comphelper::string::remove(m_pBookmarkBox->GetText(),
            m_pBookmarkBox->GetMultiSelectionSeparator()));

        rSh.SetBookmark( KeyCode(), sEntry, aEmptyStr );
        rReq.AppendItem( SfxStringItem( FN_INSERT_BOOKMARK, sEntry ) );
        rReq.Done();
    }

    if ( !rReq.IsDone() )
        rReq.Ignore();

}

SwInsertBookmarkDlg::SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rS, SfxRequest& rRequest ) :
    SvxStandardDialog(pParent, "InsertBookmarkDialog", "modules/swriter/ui/insertbookmark.ui"),
    rSh( rS ),
    rReq( rRequest )
{
    get(m_pBookmarkBox, "bookmarks");
    get(m_pOkBtn, "ok");
    get(m_pDeleteBtn, "delete");

    m_pBookmarkBox->SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    m_pBookmarkBox->EnableMultiSelection(sal_True);
    m_pBookmarkBox->EnableAutocomplete( sal_True, sal_True );

    m_pDeleteBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));

    // fill Combobox with existing bookmarks
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    sal_uInt16 nId = 0;
    for( IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ++ppBookmark)
    {
        if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            m_pBookmarkBox->InsertSwEntry(
                    SwBoxEntry(ppBookmark->get()->GetName(), nId++));
        }
    }

    sRemoveWarning = String(SW_RES(STR_REMOVE_WARNING));
}

SwInsertBookmarkDlg::~SwInsertBookmarkDlg()
{
}

BookmarkCombo::BookmarkCombo(Window* pWin)
    : SwComboBox(pWin)
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
        OUString sEntry(comphelper::string::strip(GetText().getToken(nPos, cSep), ' '));
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
            OUString sEntry(comphelper::string::strip(GetText().getToken(nPos, cSep), ' '));
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
        OUString sKey( rNEvt.GetKeyEvent()->GetCharCode() );
        if(STRING_NOTFOUND != aForbiddenChars.Search(sKey))
            nHandled = 1;
    }
    if(!nHandled)
        nHandled = SwComboBox::PreNotify( rNEvt );
    return nHandled;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeBookmarkCombo(Window* pParent, VclBuilder::stringmap &)
{
    return new BookmarkCombo(pParent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
