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
#include <vcl/builderfactory.hxx>
#include <svtools/headbar.hxx>
#include <svtools/treelistentry.hxx>

#include "view.hxx"
#include "basesh.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "bookmark.hxx"
#include "globals.hrc"

const OUString BookmarkTable::aForbiddenChars("/\\@*?\",#");
const OUString BookmarkTable::sDefaultBookmarkName("Bookmark");

// gtkEdit ModifyHandler
IMPL_LINK_NOARG_TYPED( SwInsertBookmarkDlg, ModifyHdl, Edit&, void )
{
    m_pBookmarksBox->SelectAll(false);
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    // sanitization
    OUString sTmp = m_pEditBox->GetText();
    const sal_Int32 nLen = sTmp.getLength();
    OUString sMsg;
    for(sal_Int32 i = 0; i < BookmarkTable::aForbiddenChars.getLength(); i++)
    {
        const sal_Int32 nTmpLen = sTmp.getLength();
        sTmp = comphelper::string::remove(sTmp, BookmarkTable::aForbiddenChars[i]);
        if(sTmp.getLength() != nTmpLen)
           sMsg += OUString(BookmarkTable::aForbiddenChars[i]);
    }
    if(sTmp.getLength() != nLen)
    {
        m_pEditBox->SetText(sTmp);
        ScopedVclPtr<InfoBox>::Create(this, sRemoveWarning+sMsg)->Execute();
    }

    sal_Int32 nSelectedEntries = 0;
    sal_Int32 nEntries = 0;
    sal_Int32 nTokenIndex = 0;
    do
    {
        OUString aToken = sTmp.getToken(0, ';', nTokenIndex).trim();
        if (m_pBookmarksBox->GetBookmarkByName(aToken))
        {
            m_pBookmarksBox->SelectByName(aToken);
            nSelectedEntries++;
        }
        nEntries++;
    }
    while ( nTokenIndex >= 0 );

    // allow to add new bookmark only if one name provided and its not taken
    m_pOkBtn->Enable(nEntries == 1 && nSelectedEntries == 0);

    // allow to delete only if all bookmarks are recognized;
    m_pDeleteBtn->Enable(nEntries == nSelectedEntries);
    m_pGotoBtn->Enable(nEntries == 1 && nSelectedEntries == 1);
}

// callback to delete a text mark
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, DeleteHdl, Button*, void)
{
    if (m_pBookmarksBox->GetSelectionCount() == 0)
        return;

    SvTreeListEntry* pSelected = m_pBookmarksBox->FirstSelected();
    for (sal_Int32 i = m_pBookmarksBox->GetSelectionCount(); i; i-- )
    {
        // remove from internal container
        OUString sRemoved = ((sw::mark::IMark*)pSelected->GetUserData())->GetName(); // todo: c++ cast
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark( pMarkAccess->findMark(sRemoved) );
        SfxRequest aReq( rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK );
        aReq.AppendItem( SfxStringItem( FN_DELETE_BOOKMARK, sRemoved ) );
        aReq.Done();
        // remove from ComboBox
        SvTreeListEntry* nextSelected = m_pBookmarksBox->NextSelected(pSelected);
        m_pBookmarksBox->RemoveEntry(pSelected);
        pSelected = nextSelected;
    }
    m_pBookmarksBox->SelectAll(false);
    m_pEditBox->SetText("");

    m_pDeleteBtn->Disable();
    m_pGotoBtn->Disable();
    m_pOkBtn->Enable();
}

// callback to goto a text mark
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, GotoHdl, Button*, void)
{
    GoToSelectedBookmark();
}

IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, SelectionChangedHdl, SvTreeListBox*, void)
{
    // this event is fired only if we change selection by selecting BookmarkTable entry
    if (!m_pBookmarksBox->HasFocus())
        return;

    OUString sEditBoxText;
    SvTreeListEntry* pSelected = m_pBookmarksBox->FirstSelected();
    for (sal_Int32 i = m_pBookmarksBox->GetSelectionCount(); i; i-- )
    {
        OUString sEntryName = ((sw::mark::IMark*)pSelected->GetUserData())->GetName(); // todo: c++ cast
        sEditBoxText = sEditBoxText + sEntryName;
        if (i!=1)
            sEditBoxText += "; ";

        pSelected = m_pBookmarksBox->NextSelected(pSelected);
    }

    if (m_pBookmarksBox->GetSelectionCount() > 0)
    {
        m_pOkBtn->Disable();
        m_pGotoBtn->Enable(m_pBookmarksBox->GetSelectionCount() == 1);
        m_pDeleteBtn->Enable();
        m_pEditBox->SetText(sEditBoxText);
    }
    else
    {
        m_pOkBtn->Enable();
        m_pGotoBtn->Disable();
        m_pDeleteBtn->Disable();
    }
}

IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, DoubleClickHdl, SvTreeListBox*, bool)
{
   GoToSelectedBookmark();
   return true;
}

void SwInsertBookmarkDlg::GoToSelectedBookmark()
{
    // if no entries selected we cant jump anywhere
    if (m_pBookmarksBox->GetSelectionCount() == 0)
        return;

    // todo c++ cast
    sw::mark::IMark* pBookmark = (sw::mark::IMark*)m_pBookmarksBox->FirstSelected()->GetUserData();

    // maybe should we check if Bookmark is still valid
    rSh.EnterStdMode();
    rSh.GotoMark( pBookmark );
    Close();
}

// callback for OKButton. Inserts a new text mark to the current position.
// Deleted text marks are also deleted in the model.
void SwInsertBookmarkDlg::Apply()
{
    // don't allow spaces at beginnig or end of bookmark name
    OUString sBookmark = m_pEditBox->GetText().trim();
    rSh.SetBookmark( vcl::KeyCode(), sBookmark, OUString() );
    rReq.AppendItem( SfxStringItem( FN_INSERT_BOOKMARK, sBookmark ) );
    rReq.Done();
    if ( !rReq.IsDone() )
        rReq.Ignore();
}

SwInsertBookmarkDlg::SwInsertBookmarkDlg( vcl::Window *pParent, SwWrtShell &rS, SfxRequest& rRequest ) :
    SvxStandardDialog(pParent, "InsertBookmarkDialog", "modules/swriter/ui/insertbookmark.ui"),
    rSh( rS ),
    rReq( rRequest )
{
    get(m_BookmarksContainer, "bookmarks");
    get(m_pOkBtn, "ok");
    get(m_pDeleteBtn, "delete");
    get(m_pGotoBtn, "goto");
    get(m_pEditBox, "bookmark_name");

    m_pBookmarksBox = VclPtr<BookmarkTable>::Create( *m_BookmarksContainer, 0);

    m_pBookmarksBox->SetSelectHdl(LINK(this, SwInsertBookmarkDlg, SelectionChangedHdl));
    m_pBookmarksBox->SetDeselectHdl(LINK(this, SwInsertBookmarkDlg, SelectionChangedHdl));
    m_pBookmarksBox->SetDoubleClickHdl(LINK(this, SwInsertBookmarkDlg, DoubleClickHdl));
    m_pEditBox->SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    m_pDeleteBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));
    m_pGotoBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, GotoHdl));

    m_pDeleteBtn->Disable();
    m_pGotoBtn->Disable();

    // fill Combobox with existing bookmarks
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for( IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ++ppBookmark)
    {
        if(IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            m_pBookmarksBox->InsertBookmark(ppBookmark->get());
        }
    }

    m_pEditBox->SetText(m_pBookmarksBox->GetNameProposal());
    m_pEditBox->SetCursorAtLast();

    sRemoveWarning = OUString(SW_RES(STR_REMOVE_WARNING));
}

SwInsertBookmarkDlg::~SwInsertBookmarkDlg()
{
    disposeOnce();
}

void SwInsertBookmarkDlg::dispose()
{
    m_pBookmarksBox.disposeAndClear();
    m_BookmarksContainer.clear();
    m_pOkBtn.clear();
    m_pDeleteBtn.clear();
    m_pGotoBtn.clear();
    m_pEditBox.clear();
    SvxStandardDialog::dispose();
}

BookmarkTable::BookmarkTable(SvSimpleTableContainer& rParent, WinBits nStyle)
    : SvSimpleTable(rParent, nStyle)
{
    static long nTabs[] = { 3, 0, 50, 150 };

    SetTabs( nTabs, MAP_PIXEL );
    InsertHeaderEntry("Page");
    InsertHeaderEntry("Name");
    InsertHeaderEntry("Text");

    SetSelectionMode( MULTIPLE_SELECTION );

    rParent.SetTable(this);
}

void BookmarkTable::InsertBookmark(sw::mark::IMark* pMark)
{
    OUString sBookmarkNodeText = pMark->GetMarkStart().nNode.GetNode().GetTextNode()->GetText();
    sal_Int32 nBookmarkNodeTextPos = pMark->GetMarkStart().nContent.GetIndex();
    sal_Int32 nBookmarkTextLen = 0;
    static const sal_Int32 nMaxTextLen = 50;

    if (pMark->IsExpanded())
    {
        nBookmarkTextLen = pMark->GetMarkEnd().nContent.GetIndex() - nBookmarkNodeTextPos;
    }
    else
    {
        if (nBookmarkNodeTextPos == sBookmarkNodeText.getLength()) // no text after bookmark
            nBookmarkNodeTextPos = std::max<int>(0, nBookmarkNodeTextPos - nMaxTextLen); // pulling text
        nBookmarkTextLen = sBookmarkNodeText.getLength() - nBookmarkNodeTextPos;
    }

    nBookmarkTextLen = std::min<int>(nMaxTextLen, nBookmarkTextLen);
    sBookmarkNodeText = sBookmarkNodeText.copy(nBookmarkNodeTextPos, nBookmarkTextLen).trim();

    OUString sPageNum = OUString::number( SwPaM(pMark->GetMarkStart()).GetPageNum() );
    OUString sColumnData = sPageNum + "\t"+ pMark->GetName() + "\t" + sBookmarkNodeText;

    InsertEntryToColumn(sColumnData, TREELIST_APPEND, 0xffff, pMark);
}

SvTreeListEntry* BookmarkTable::GetRowByBookmarkName(OUString name)
{
    SvTreeListEntry* pEntry = First();
    for (sal_Int32 i = GetRowCount(); i; i-- )
    {
        OUString sName = ((sw::mark::IMark*)pEntry->GetUserData())->GetName(); // todo: c++ cast
        if (sName == name)
        {
            return pEntry;
        }
        pEntry = Next(pEntry);
    }
    return NULL;
}

sw::mark::IMark* BookmarkTable::GetBookmarkByName(OUString sName)
{
    SvTreeListEntry* pEntry = GetRowByBookmarkName(sName);
    if (!pEntry)
        return NULL;

    return (sw::mark::IMark*)pEntry->GetUserData(); // todo c++ cast
}

void BookmarkTable::SelectByName(OUString sName)
{
    SvTreeListEntry* pEntry = GetRowByBookmarkName(sName);
    if (!pEntry)
        return;

    Select(pEntry);
}

OUString BookmarkTable::GetNameProposal()
{
    SvTreeListEntry* pEntry = First();
    sal_Int32 nHighestBookmarkId = 0;
    for (sal_Int32 i = GetRowCount(); i; i-- )
    {
        OUString sName = ((sw::mark::IMark*)pEntry->GetUserData())->GetName(); // todo: c++ cast
        sal_Int32 nIndex = 0;
        if (sName.getToken(0, ' ', nIndex) == sDefaultBookmarkName)
        {
            sal_Int32 nCurrBookmarkId = sName.getToken(0, ' ', nIndex).toInt32();
            nHighestBookmarkId = std::max<int>(nHighestBookmarkId, nCurrBookmarkId);
        }
        pEntry = Next(pEntry);
    }
    return sDefaultBookmarkName + " " + OUString::number(nHighestBookmarkId + 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
