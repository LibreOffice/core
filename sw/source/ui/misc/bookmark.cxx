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
#include <com/sun/star/text/XBookmarksSupplier.hpp>

#include "swabstdlg.hxx"
#include "swuiexp.hxx"
#include "view.hxx"
#include "basesh.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "bookmark.hxx"
#include "docsh.hxx"
#include "globals.hrc"

using namespace ::com::sun::star;

const OUString BookmarkTable::aForbiddenChars("/\\@*?\",#");
const char BookmarkTable::cSeparator(';');
const OUString BookmarkTable::sDefaultBookmarkName("Bookmark");

// callback to modify EditBox
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, ModifyHdl, Edit&, void)
{
    ValidateBookmarks();
    m_pBookmarksBox->SelectAll(false);
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    // sanitization
    OUString sTmp = m_pEditBox->GetText();
    OUString sMsg;
    const sal_Int32 nLen = sTmp.getLength();
    for (sal_Int32 i = 0; i < BookmarkTable::aForbiddenChars.getLength(); i++)
    {
        const sal_Int32 nTmpLen = sTmp.getLength();
        sTmp = comphelper::string::remove(sTmp, BookmarkTable::aForbiddenChars[i]);
        if (sTmp.getLength() != nTmpLen)
           sMsg += OUString(BookmarkTable::aForbiddenChars[i]);
    }
    if (sTmp.getLength() != nLen)
    {
        m_pEditBox->SetText(sTmp);
        ScopedVclPtrInstance<InfoBox>(this, sRemoveWarning + sMsg)->Execute();
    }

    sal_Int32 nSelectedEntries = 0;
    sal_Int32 nEntries = 0;
    sal_Int32 nTokenIndex = 0;
    while (!sTmp.isEmpty() && nTokenIndex >= 0)
    {
        OUString aToken = sTmp.getToken(0, BookmarkTable::cSeparator, nTokenIndex);
        if (m_pBookmarksBox->GetBookmarkByName(aToken))
        {
            m_pBookmarksBox->SelectByName(aToken);
            nSelectedEntries++;
        }
        nEntries++;
    }

    // allow to add new bookmark only if one name provided and its not taken
    m_pInsertBtn->Enable(nEntries == 1 && nSelectedEntries == 0);

    // allow to delete only if all bookmarks are recognized
    m_pDeleteBtn->Enable(nEntries > 0 && nSelectedEntries == nEntries);
    m_pGotoBtn->Enable(nEntries == 1 && nSelectedEntries == 1);
    m_pRenameBtn->Enable(nEntries == 1 && nSelectedEntries == 1);
}

// callback to delete a text mark
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, DeleteHdl, Button*, void)
{
    if (!ValidateBookmarks())
        return;
    if (m_pBookmarksBox->GetSelectionCount() == 0)
        return;

    SvTreeListEntry* pSelected = m_pBookmarksBox->FirstSelected();
    for (sal_Int32 i = m_pBookmarksBox->GetSelectionCount(); i; i--)
    {
        // remove from model
        sw::mark::IMark* pBookmark = static_cast<sw::mark::IMark*>(pSelected->GetUserData());
        OUString sRemoved = pBookmark->GetName();
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark(pMarkAccess->findMark(sRemoved));
        SfxRequest aReq(rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK);
        aReq.AppendItem(SfxStringItem(FN_DELETE_BOOKMARK, sRemoved));
        aReq.Done();
        aTableBookmarks.erase(std::remove(aTableBookmarks.begin(), aTableBookmarks.end(),
                              std::make_pair(pBookmark, sRemoved)), aTableBookmarks.end());
        // remove from BookmarkTable
        SvTreeListEntry* nextSelected = m_pBookmarksBox->NextSelected(pSelected);
        m_pBookmarksBox->RemoveEntry(pSelected);
        pSelected = nextSelected;
    }
    m_pBookmarksBox->SelectAll(false);
    m_pEditBox->SetText("");
    m_pDeleteBtn->Disable();
    m_pGotoBtn->Disable();
    m_pRenameBtn->Disable();
    m_pInsertBtn->Disable();
}

// callback to a goto button
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, GotoHdl, Button*, void)
{
    GotoSelectedBookmark();
}

IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, DoubleClickHdl, SvTreeListBox*, bool)
{
    GotoSelectedBookmark();
    return true;
}

IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, SelectionChangedHdl, SvTreeListBox*, void)
{
    if (!ValidateBookmarks())
        return;
    // this event should fired only if we change selection by clicking on BookmarkTable entry
    if (!m_pBookmarksBox->HasFocus())
        return;

    OUString sEditBoxText;
    SvTreeListEntry* pSelected = m_pBookmarksBox->FirstSelected();
    for (sal_Int32 i = m_pBookmarksBox->GetSelectionCount(); i; i--)
    {
        sw::mark::IMark* pBookmark = static_cast<sw::mark::IMark*>(pSelected->GetUserData());
        OUString sEntryName = pBookmark->GetName();
        sEditBoxText = sEditBoxText + sEntryName;
        if (i > 1)
            sEditBoxText += ";";
        pSelected = m_pBookmarksBox->NextSelected(pSelected);
    }
    if (m_pBookmarksBox->GetSelectionCount() > 0)
    {
        m_pInsertBtn->Disable();
        m_pGotoBtn->Enable(m_pBookmarksBox->GetSelectionCount() == 1);
        m_pRenameBtn->Enable(m_pBookmarksBox->GetSelectionCount() == 1);
        m_pDeleteBtn->Enable();
        m_pEditBox->SetText(sEditBoxText);
    }
    else
    {
        m_pInsertBtn->Enable();
        m_pGotoBtn->Disable();
        m_pRenameBtn->Disable();
        m_pDeleteBtn->Disable();
    }
}

IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, RenameHdl, Button*, void)
{
    if (!ValidateBookmarks())
        return;
    if (m_pBookmarksBox->GetSelectionCount() == 0)
        return;

    SvTreeListEntry* pSelected = m_pBookmarksBox->FirstSelected();
    sw::mark::IMark* pBookmark = static_cast<sw::mark::IMark*>(pSelected->GetUserData());

    uno::Reference<frame::XModel> xModel = rSh.GetView().GetDocShell()->GetBaseModel();
    uno::Reference<text::XBookmarksSupplier> xBkms(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess = xBkms->getBookmarks();
    uno::Any aObj = xNameAccess->getByName(pBookmark->GetName());
    uno::Reference<uno::XInterface> xTmp;
    aObj >>= xTmp;
    uno::Reference<container::XNamed> xNamed(xTmp, uno::UNO_QUERY);
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
    std::unique_ptr<AbstractSwRenameXNamedDlg> pDlg(pFact->CreateSwRenameXNamedDlg(this, xNamed, xNameAccess));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    pDlg->SetForbiddenChars(BookmarkTable::aForbiddenChars + OUStringLiteral1<BookmarkTable::cSeparator>());

    if (pDlg->Execute())
    {
        ValidateBookmarks();
        m_pDeleteBtn->Disable();
        m_pGotoBtn->Disable();
        m_pRenameBtn->Disable();
        m_pInsertBtn->Disable();
    }
}

// callback to a insert button. Inserts a new text mark to the current position.
IMPL_LINK_NOARG_TYPED(SwInsertBookmarkDlg, InsertHdl, Button*, void)
{
    OUString sBookmark = m_pEditBox->GetText();
    rSh.SetBookmark(vcl::KeyCode(), sBookmark, OUString());
    rReq.AppendItem(SfxStringItem(FN_INSERT_BOOKMARK, sBookmark));
    rReq.Done();
    if (!rReq.IsDone())
        rReq.Ignore();

    EndDialog(RET_OK);
}

void SwInsertBookmarkDlg::GotoSelectedBookmark()
{
    if (!ValidateBookmarks())
        return;
    // if no entries selected we cant jump anywhere
    // shouldn't be needed as we disable GoTo button when jump is not possible
    if (m_pBookmarksBox->GetSelectionCount() == 0)
        return;

    sw::mark::IMark* pBookmark = static_cast<sw::mark::IMark*>(m_pBookmarksBox->FirstSelected()->GetUserData());

    rSh.EnterStdMode();
    rSh.GotoMark(pBookmark);
}

bool SwInsertBookmarkDlg::ValidateBookmarks()
{
    if (HaveBookmarksChanged())
    {
        PopulateTable();
        m_pEditBox->SetText("");
        return false;
    }
    return true;
}

bool SwInsertBookmarkDlg::HaveBookmarksChanged()
{
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    if (pMarkAccess->getBookmarksCount() != static_cast<sal_Int32>(aTableBookmarks.size()))
        return true;

    IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
    for (std::pair<sw::mark::IMark*,OUString> & aTableBookmark : aTableBookmarks)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            if (aTableBookmark.first != ppBookmark->get() ||
                aTableBookmark.second != ppBookmark->get()->GetName())
                return true;
        }
        ++ppBookmark;
    }
    return false;
}

void SwInsertBookmarkDlg::PopulateTable()
{
    aTableBookmarks.clear();
    m_pBookmarksBox->Clear();
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
         ppBookmark != pMarkAccess->getBookmarksEnd(); ++ppBookmark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            m_pBookmarksBox->InsertBookmark(ppBookmark->get());
            aTableBookmarks.push_back(std::make_pair(ppBookmark->get(), ppBookmark->get()->GetName()));
        }
    }
}

void SwInsertBookmarkDlg::Apply()
{
}

SwInsertBookmarkDlg::SwInsertBookmarkDlg(vcl::Window* pParent, SwWrtShell& rS, SfxRequest& rRequest) :
    SvxStandardDialog(pParent, "InsertBookmarkDialog", "modules/swriter/ui/insertbookmark.ui"),
    rSh(rS),
    rReq(rRequest)
{
    get(m_pBookmarksContainer, "bookmarks");
    get(m_pEditBox, "name");
    get(m_pInsertBtn, "insert");
    get(m_pDeleteBtn, "delete");
    get(m_pGotoBtn, "goto");
    get(m_pRenameBtn, "rename");

    m_pBookmarksBox = VclPtr<BookmarkTable>::Create(*m_pBookmarksContainer);

    m_pBookmarksBox->SetSelectHdl(LINK(this, SwInsertBookmarkDlg, SelectionChangedHdl));
    m_pBookmarksBox->SetDeselectHdl(LINK(this, SwInsertBookmarkDlg, SelectionChangedHdl));
    m_pBookmarksBox->SetDoubleClickHdl(LINK(this, SwInsertBookmarkDlg, DoubleClickHdl));
    m_pEditBox->SetModifyHdl(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    m_pInsertBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, InsertHdl));
    m_pDeleteBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, DeleteHdl));
    m_pGotoBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, GotoHdl));
    m_pRenameBtn->SetClickHdl(LINK(this, SwInsertBookmarkDlg, RenameHdl));

    m_pDeleteBtn->Disable();
    m_pGotoBtn->Disable();
    m_pRenameBtn->Disable();

    PopulateTable();

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
    m_pBookmarksContainer.clear();
    m_pInsertBtn.clear();
    m_pDeleteBtn.clear();
    m_pGotoBtn.clear();
    m_pEditBox.clear();
    SvxStandardDialog::dispose();
}

BookmarkTable::BookmarkTable(SvSimpleTableContainer& rParent) :
    SvSimpleTable(rParent, 0)
{
    static long nTabs[] = {3, 0, 40, 150};

    SetTabs(nTabs, MAP_PIXEL);
    SetSelectionMode(MULTIPLE_SELECTION);
    InsertHeaderEntry(OUString(SW_RES(STR_PAGE)));
    InsertHeaderEntry(OUString(SW_RES(STR_BOOKMARK_NAME)));
    InsertHeaderEntry(OUString(SW_RES(STR_BOOKMARK_TEXT)));

    rParent.SetTable(this);
}

void BookmarkTable::InsertBookmark(sw::mark::IMark* pMark)
{
    OUString sBookmarkNodeText = pMark->GetMarkStart().nNode.GetNode().GetTextNode()->GetText();
    sal_Int32 nBookmarkNodeTextPos = pMark->GetMarkStart().nContent.GetIndex();
    sal_Int32 nBookmarkTextLen = 0;
    bool bPulledAll = false;
    bool bPulling = false;
    static const sal_Int32 nMaxTextLen = 50;

    if (pMark->IsExpanded())
    {
        nBookmarkTextLen = pMark->GetMarkEnd().nContent.GetIndex() - nBookmarkNodeTextPos;
    }
    else
    {
        if (nBookmarkNodeTextPos == sBookmarkNodeText.getLength()) // no text after bookmark
        {
            nBookmarkNodeTextPos = std::max<sal_Int32>(0, nBookmarkNodeTextPos - nMaxTextLen);
            bPulling = true;
            if (nBookmarkNodeTextPos == 0)
                bPulledAll = true;
        }
        nBookmarkTextLen = sBookmarkNodeText.getLength() - nBookmarkNodeTextPos;
    }
    bool bExceedsLength = nBookmarkTextLen > nMaxTextLen;
    nBookmarkTextLen = std::min<sal_Int32>(nMaxTextLen, nBookmarkTextLen);
    sBookmarkNodeText = sBookmarkNodeText.copy(nBookmarkNodeTextPos, nBookmarkTextLen).trim();
    if (bExceedsLength)
        sBookmarkNodeText += "...";
    else if (bPulling && !bPulledAll)
        sBookmarkNodeText = "..." + sBookmarkNodeText;

    OUString sPageNum = OUString::number(SwPaM(pMark->GetMarkStart()).GetPageNum());
    OUString sColumnData = sPageNum + "\t" + pMark->GetName() + "\t" + sBookmarkNodeText;
    InsertEntryToColumn(sColumnData, TREELIST_APPEND, 0xffff, pMark);
}

SvTreeListEntry* BookmarkTable::GetRowByBookmarkName(const OUString& sName)
{
    SvTreeListEntry* pEntry = First();
    while (pEntry)
    {
        sw::mark::IMark* pBookmark = static_cast<sw::mark::IMark*>(pEntry->GetUserData());
        if (pBookmark->GetName() == sName)
        {
            return pEntry;
        }
        pEntry = Next(pEntry);
    }
    return nullptr;
}

sw::mark::IMark* BookmarkTable::GetBookmarkByName(const OUString& sName)
{
    SvTreeListEntry* pEntry = GetRowByBookmarkName(sName);
    if (!pEntry)
        return nullptr;

    return static_cast<sw::mark::IMark*>(pEntry->GetUserData());
}

void BookmarkTable::SelectByName(const OUString& sName)
{
    SvTreeListEntry* pEntry = GetRowByBookmarkName(sName);
    if (!pEntry)
        return;

    Select(pEntry);
}

OUString BookmarkTable::GetNameProposal()
{
    sal_Int32 nHighestBookmarkId = 0;
    SvTreeListEntry* pEntry = First();
    while (pEntry)
    {
        sw::mark::IMark* pBookmark = static_cast<sw::mark::IMark*>(pEntry->GetUserData());
        OUString sName = pBookmark->GetName();
        sal_Int32 nIndex = 0;
        if (sName.getToken(0, ' ', nIndex) == sDefaultBookmarkName)
        {
            sal_Int32 nCurrBookmarkId = sName.getToken(0, ' ', nIndex).toInt32();
            nHighestBookmarkId = std::max<sal_Int32>(nHighestBookmarkId, nCurrBookmarkId);
        }
        pEntry = Next(pEntry);
    }
    return sDefaultBookmarkName + " " + OUString::number(nHighestBookmarkId + 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
