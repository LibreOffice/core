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

    m_pDeleteBtn->Disable();
    m_pOkBtn->Enable();
}

// callback to goto a text mark
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, GotoHdl, Button*, void)
{
    // if no entries selected we cant jump anywhere
    if (m_pBookmarksBox->GetSelectionCount() == 0)
        return;

    // todo c++ cast
    sw::mark::IMark* pBookmark = (sw::mark::IMark*)m_pBookmarksBox->FirstSelected()->GetUserData();

    // Maybe check if Bookmark is still valid. To prevent crash
    rSh.EnterStdMode();
    rSh.GotoMark( pBookmark );
    Close();
}

IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, SelectionChangedHdl, SvTreeListBox*, void)
{
    // this event is fired only if we change selection by selecting BookmarkTable entry
    if (!m_pBookmarksBox->HasFocus())
    {
        return;
    }

    OUString sEditBoxText;
    SvTreeListEntry* pSelected = m_pBookmarksBox->FirstSelected();
    for (sal_Int32 i = m_pBookmarksBox->GetSelectionCount(); i; i-- )
    {
        OUString sEntryName = ((sw::mark::IMark*)pSelected->GetUserData())->GetName(); // todo: c++ cast
        if (i!=1)
        {
            sEditBoxText = sEditBoxText + sEntryName + "; ";
        }
        else
        {
             sEditBoxText = sEditBoxText + sEntryName;
        }
        pSelected = m_pBookmarksBox->NextSelected(pSelected);;
    }

    if (m_pBookmarksBox->GetSelectionCount() > 0)
    {
        m_pOkBtn->Disable();
        m_pDeleteBtn->Enable();
        m_pEditBox->SetText(sEditBoxText);
    }
    else
    {
        m_pOkBtn->Enable();
        m_pDeleteBtn->Disable();
    }
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
    m_pEditBox->SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    m_pDeleteBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));
    m_pGotoBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, GotoHdl));

    m_pDeleteBtn->Disable();

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

    // bookmark name proposal eg. "Bookmark 1"
    // complexity could be better if we would do pattern reading (like scanf("Bookmark %d", &nBId)) and find min nBId
    static sal_Int32 nBookmarkId = 1;
    OUString sBookmarkName = "Bookmark " + OUString::number(nBookmarkId);
    nBookmarkId++;
    m_pEditBox->SetText(sBookmarkName);
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

void BookmarkTable::InsertBookmark(sw::mark::IMark* mark)
{
    SwPaM* pSwPaM = new SwPaM( mark->GetMarkPos() );
    const SwPosition* pPos = pSwPaM->GetPoint();
    sal_Int32 nBookmarkPos = pPos->nContent.GetIndex();
    OUString sBookmarkNodeText = pPos->nNode.GetNode().GetTextNode()->GetText();
    sBookmarkNodeText = sBookmarkNodeText.copy(nBookmarkPos,
                                               std::min<sal_Int32>((sBookmarkNodeText.getLength() - nBookmarkPos), 20));

    OUString sPageNum = OUString::number( pSwPaM->GetPageNum() );
    OUString sData = sPageNum + "\t"+ mark->GetName() + "\t" + sBookmarkNodeText;

    InsertEntryToColumn(sData, TREELIST_APPEND, 0xffff, mark);
    delete pSwPaM;
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

sw::mark::IMark* BookmarkTable::GetBookmarkByName(OUString name)
{
    SvTreeListEntry* pEntry = GetRowByBookmarkName(name);
    if (!pEntry)
    {
        return NULL;
    }
    return (sw::mark::IMark*)pEntry->GetUserData(); // todo c++ cast
}

void BookmarkTable::SelectByName(OUString sName)
{
    SvTreeListEntry* pEntry = GetRowByBookmarkName(sName);
    if (pEntry)
    {
        Select(pEntry);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
