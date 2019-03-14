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

#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/weld.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/headbar.hxx>
#include <vcl/treelistentry.hxx>
#include <com/sun/star/text/XBookmarksSupplier.hpp>

#include <swabstdlg.hxx>
#include <swuiexp.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>
#include <bookmark.hxx>
#include <docsh.hxx>
#include <globals.hrc>
#include <strings.hrc>

using namespace ::com::sun::star;

const OUString BookmarkTable::aForbiddenChars("/\\@*?\",#");
const char BookmarkTable::cSeparator(';');

// callback to modify EditBox
IMPL_LINK_NOARG(SwInsertBookmarkDlg, ModifyHdl, weld::Entry&, void)
{
    ValidateBookmarks();
    m_xBookmarksBox->unselect_all();
    // if a string has been pasted from the clipboard then
    // there may be illegal characters in the box
    // sanitization
    OUString sTmp = m_xEditBox->get_text();
    OUString sMsg;
    const sal_Int32 nLen = sTmp.getLength();
    for (sal_Int32 i = 0; i < BookmarkTable::aForbiddenChars.getLength(); i++)
    {
        const sal_Int32 nTmpLen = sTmp.getLength();
        sTmp = sTmp.replaceAll(OUStringLiteral1(BookmarkTable::aForbiddenChars[i]), "");
        if (sTmp.getLength() != nTmpLen)
           sMsg += OUStringLiteral1(BookmarkTable::aForbiddenChars[i]);
    }
    if (sTmp.getLength() != nLen)
    {
        m_xEditBox->set_text(sTmp);
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      sRemoveWarning + sMsg));
        xInfoBox->run();
    }

    sal_Int32 nSelectedEntries = 0;
    sal_Int32 nEntries = 0;
    sal_Int32 nTokenIndex = 0;
    while (!sTmp.isEmpty() && nTokenIndex >= 0)
    {
        OUString aToken = sTmp.getToken(0, BookmarkTable::cSeparator, nTokenIndex);
        if (m_xBookmarksBox->GetBookmarkByName(aToken))
        {
            m_xBookmarksBox->SelectByName(aToken);
            nSelectedEntries++;
        }
        nEntries++;
    }

    // allow to add new bookmark only if one name provided and it's not taken
    m_xInsertBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 0);

    // allow to delete only if all bookmarks are recognized
    m_xDeleteBtn->set_sensitive(nEntries > 0 && nSelectedEntries == nEntries);
    m_xGotoBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 1);
    m_xRenameBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 1);
}

// callback to delete a text mark
IMPL_LINK_NOARG(SwInsertBookmarkDlg, DeleteHdl, weld::Button&, void)
{
    if (!ValidateBookmarks())
        return;
    std::vector<int> aSelectedRows(m_xBookmarksBox->get_selected_rows());
    if (aSelectedRows.empty())
        return;
    std::sort(aSelectedRows.begin(), aSelectedRows.end());

    for (size_t i = aSelectedRows.size(); i; --i)
    {
        int nRow = aSelectedRows[i-1];
        // remove from model
        sw::mark::IMark* pBookmark = reinterpret_cast<sw::mark::IMark*>(m_xBookmarksBox->get_id(nRow).toInt64());
        OUString sRemoved = pBookmark->GetName();
        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark(pMarkAccess->findMark(sRemoved));
        SfxRequest aReq(rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK);
        aReq.AppendItem(SfxStringItem(FN_DELETE_BOOKMARK, sRemoved));
        aReq.Done();
        aTableBookmarks.erase(std::remove(aTableBookmarks.begin(), aTableBookmarks.end(),
                              std::make_pair(pBookmark, sRemoved)), aTableBookmarks.end());
        // remove from BookmarkTable
        m_xBookmarksBox->remove(nRow);
    }
    m_xBookmarksBox->unselect_all();
    m_xEditBox->set_text("");
    m_xDeleteBtn->set_sensitive(false);
    m_xGotoBtn->set_sensitive(false);
    m_xRenameBtn->set_sensitive(false);
    m_xInsertBtn->set_sensitive(false);
}

// callback to a goto button
IMPL_LINK_NOARG(SwInsertBookmarkDlg, GotoHdl, weld::Button&, void)
{
    GotoSelectedBookmark();
}

IMPL_LINK_NOARG(SwInsertBookmarkDlg, DoubleClickHdl, weld::TreeView&, void)
{
    GotoSelectedBookmark();
}

IMPL_LINK_NOARG(SwInsertBookmarkDlg, SelectionChangedHdl, weld::TreeView&, void)
{
    if (!ValidateBookmarks())
        return;
    // this event should fired only if we change selection by clicking on BookmarkTable entry
    if (!m_xBookmarksBox->has_focus())
        return;

    OUStringBuffer sEditBoxText;

    std::vector<int> aSelectedRows(m_xBookmarksBox->get_selected_rows());
    std::sort(aSelectedRows.begin(), aSelectedRows.end());
    for (size_t i = aSelectedRows.size(); i; --i)
    {
        int nRow = aSelectedRows[i-1];
        sw::mark::IMark* pBookmark = reinterpret_cast<sw::mark::IMark*>(m_xBookmarksBox->get_id(nRow).toInt64());
        const OUString& sEntryName = pBookmark->GetName();
        sEditBoxText.append(sEntryName);
        if (nRow > 1)
            sEditBoxText.append(";");
    }
    if (!aSelectedRows.empty())
    {
        m_xInsertBtn->set_sensitive(false);
        m_xGotoBtn->set_sensitive(aSelectedRows.size() == 1);
        m_xRenameBtn->set_sensitive(aSelectedRows.size() == 1);
        m_xDeleteBtn->set_sensitive(true);
        m_xEditBox->set_text(sEditBoxText.makeStringAndClear());
    }
    else
    {
        m_xInsertBtn->set_sensitive(true);
        m_xGotoBtn->set_sensitive(false);
        m_xRenameBtn->set_sensitive(false);
        m_xDeleteBtn->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SwInsertBookmarkDlg, RenameHdl, weld::Button&, void)
{
    if (!ValidateBookmarks())
        return;
    int nSelected = m_xBookmarksBox->get_selected_index();
    if (nSelected == -1)
        return;

    sw::mark::IMark* pBookmark = reinterpret_cast<sw::mark::IMark*>(m_xBookmarksBox->get_id(nSelected).toInt64());
    uno::Reference<frame::XModel> xModel = rSh.GetView().GetDocShell()->GetBaseModel();
    uno::Reference<text::XBookmarksSupplier> xBkms(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess = xBkms->getBookmarks();
    uno::Any aObj = xNameAccess->getByName(pBookmark->GetName());
    uno::Reference<uno::XInterface> xTmp;
    aObj >>= xTmp;
    uno::Reference<container::XNamed> xNamed(xTmp, uno::UNO_QUERY);
    SwAbstractDialogFactory& rFact = swui::GetFactory();
    ScopedVclPtr<AbstractSwRenameXNamedDlg> pDlg(rFact.CreateSwRenameXNamedDlg(m_xDialog.get(), xNamed, xNameAccess));
    pDlg->SetForbiddenChars(BookmarkTable::aForbiddenChars + OUStringLiteral1(BookmarkTable::cSeparator));

    if (pDlg->Execute())
    {
        ValidateBookmarks();
        m_xDeleteBtn->set_sensitive(false);
        m_xGotoBtn->set_sensitive(false);
        m_xRenameBtn->set_sensitive(false);
        m_xInsertBtn->set_sensitive(false);
    }
}

// callback to a insert button. Inserts a new text mark to the current position.
IMPL_LINK_NOARG(SwInsertBookmarkDlg, InsertHdl, weld::Button&, void)
{
    OUString sBookmark = m_xEditBox->get_text();
    rSh.SetBookmark2(vcl::KeyCode(), sBookmark, m_xHideCB->get_active(), m_xConditionED->get_text());
    rReq.AppendItem(SfxStringItem(FN_INSERT_BOOKMARK, sBookmark));
    rReq.Done();
    if (!rReq.IsDone())
        rReq.Ignore();

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwInsertBookmarkDlg, ChangeHideHdl, weld::ToggleButton&, rBox, void)
{
    bool bHide = rBox.get_active();
    m_xConditionED->set_sensitive(bHide);
    m_xConditionFT->set_sensitive(bHide);
}

void SwInsertBookmarkDlg::GotoSelectedBookmark()
{
    if (!ValidateBookmarks())
        return;
    // if no entries selected we can't jump anywhere
    // shouldn't be needed as we disable GoTo button when jump is not possible
    int nSelected = m_xBookmarksBox->get_selected_index();
    if (nSelected == -1)
        return;

    sw::mark::IMark* pBookmark = reinterpret_cast<sw::mark::IMark*>(m_xBookmarksBox->get_id(nSelected).toInt64());

    rSh.EnterStdMode();
    rSh.GotoMark(pBookmark);
}

bool SwInsertBookmarkDlg::ValidateBookmarks()
{
    if (HaveBookmarksChanged())
    {
        PopulateTable();
        m_xEditBox->set_text("");
        return false;
    }
    return true;
}

bool SwInsertBookmarkDlg::HaveBookmarksChanged()
{
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    if (pMarkAccess->getBookmarksCount() != m_nLastBookmarksCount)
        return true;

    std::vector<std::pair<sw::mark::IMark*, OUString>>::const_iterator aListIter = aTableBookmarks.begin();
    for (IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
         ppBookmark != pMarkAccess->getBookmarksEnd(); ++ppBookmark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            // more bookmarks then expected
            if (aListIter == aTableBookmarks.end())
                return true;
            if (aListIter->first != ppBookmark->get() ||
                aListIter->second != ppBookmark->get()->GetName())
                return true;
            ++aListIter;
        }
    }
    // less bookmarks then expected
    return aListIter != aTableBookmarks.end();
}

void SwInsertBookmarkDlg::PopulateTable()
{
    aTableBookmarks.clear();
    m_xBookmarksBox->clear();

    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
         ppBookmark != pMarkAccess->getBookmarksEnd(); ++ppBookmark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            m_xBookmarksBox->InsertBookmark(ppBookmark->get());
            aTableBookmarks.emplace_back(ppBookmark->get(), ppBookmark->get()->GetName());
        }
    }
    m_nLastBookmarksCount = pMarkAccess->getBookmarksCount();
}

SwInsertBookmarkDlg::SwInsertBookmarkDlg(weld::Window* pParent, SwWrtShell& rS, SfxRequest& rRequest)
    : SfxDialogController(pParent, "modules/swriter/ui/insertbookmark.ui", "InsertBookmarkDialog")
    , rSh(rS)
    , rReq(rRequest)
    , m_nLastBookmarksCount(0)
    , m_xEditBox(m_xBuilder->weld_entry("name"))
    , m_xInsertBtn(m_xBuilder->weld_button("insert"))
    , m_xDeleteBtn(m_xBuilder->weld_button("delete"))
    , m_xGotoBtn(m_xBuilder->weld_button("goto"))
    , m_xRenameBtn(m_xBuilder->weld_button("rename"))
    , m_xHideCB(m_xBuilder->weld_check_button("hide"))
    , m_xConditionFT(m_xBuilder->weld_label("condlabel"))
    , m_xConditionED(new SwConditionEdit(m_xBuilder->weld_entry("withcond")))
    , m_xBookmarksBox(new BookmarkTable(m_xBuilder->weld_tree_view("bookmarks")))
{
    m_xBookmarksBox->connect_changed(LINK(this, SwInsertBookmarkDlg, SelectionChangedHdl));
    m_xBookmarksBox->connect_row_activated(LINK(this, SwInsertBookmarkDlg, DoubleClickHdl));
    m_xEditBox->connect_changed(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    m_xInsertBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, InsertHdl));
    m_xDeleteBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, DeleteHdl));
    m_xGotoBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, GotoHdl));
    m_xRenameBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, RenameHdl));
    m_xHideCB->connect_toggled(LINK(this, SwInsertBookmarkDlg, ChangeHideHdl));

    m_xDeleteBtn->set_sensitive(false);
    m_xGotoBtn->set_sensitive(false);
    m_xRenameBtn->set_sensitive(false);

    PopulateTable();

    m_xEditBox->set_text(m_xBookmarksBox->GetNameProposal());
    m_xEditBox->set_position(-1);

    sRemoveWarning = SwResId(STR_REMOVE_WARNING);
}

SwInsertBookmarkDlg::~SwInsertBookmarkDlg()
{
}

BookmarkTable::BookmarkTable(std::unique_ptr<weld::TreeView> xControl)
    : m_xControl(std::move(xControl))
{
    m_xControl->set_size_request(450, 250);
    std::vector<int> aWidths;
    aWidths.push_back(40);
    aWidths.push_back(110);
    aWidths.push_back(150);
    aWidths.push_back(60);
    m_xControl->set_column_fixed_widths(aWidths);
    m_xControl->set_selection_mode(SelectionMode::Multiple);
}

void BookmarkTable::InsertBookmark(sw::mark::IMark* pMark)
{
    sw::mark::IBookmark* pBookmark = dynamic_cast<sw::mark::IBookmark*>(pMark);
    assert(pBookmark);

    OUString sBookmarkNodeText = pBookmark->GetMarkStart().nNode.GetNode().GetTextNode()->GetText();
    sal_Int32 nBookmarkNodeTextPos = pBookmark->GetMarkStart().nContent.GetIndex();
    sal_Int32 nBookmarkTextLen = 0;
    bool bPulledAll = false;
    bool bPulling = false;
    static const sal_Int32 nMaxTextLen = 50;

    if (pBookmark->IsExpanded())
    {
        nBookmarkTextLen = pBookmark->GetMarkEnd().nContent.GetIndex() - nBookmarkNodeTextPos;
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

    OUString sHidden = "No";
    if (pBookmark->IsHidden())
        sHidden = "Yes";
    const OUString& sHideCondition = pBookmark->GetHideCondition();
    OUString sPageNum = OUString::number(SwPaM(pMark->GetMarkStart()).GetPageNum());
    int nRow = m_xControl->n_children();
    m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(pMark)), sPageNum);
    m_xControl->set_text(nRow, pBookmark->GetName(), 1);
    m_xControl->set_text(nRow, sBookmarkNodeText, 2);
    m_xControl->set_text(nRow, sHidden, 3);
    m_xControl->set_text(nRow, sHideCondition, 4);
}

int BookmarkTable::GetRowByBookmarkName(const OUString& sName)
{
    for (int i = 0, nCount = m_xControl->n_children(); i < nCount; ++i)
    {
        sw::mark::IMark* pBookmark = reinterpret_cast<sw::mark::IMark*>(m_xControl->get_id(i).toInt64());
        if (pBookmark->GetName() == sName)
        {
            return i;
        }
    }
    return -1;
}

sw::mark::IMark* BookmarkTable::GetBookmarkByName(const OUString& sName)
{
    int nEntry = GetRowByBookmarkName(sName);
    if (nEntry == -1)
        return nullptr;

    return reinterpret_cast<sw::mark::IMark*>(m_xControl->get_id(nEntry).toInt64());
}

void BookmarkTable::SelectByName(const OUString& sName)
{
    int nEntry = GetRowByBookmarkName(sName);
    if (nEntry == -1)
        return;
    select(nEntry);
}

OUString BookmarkTable::GetNameProposal()
{
    OUString sDefaultBookmarkName = SwResId(STR_BOOKMARK_DEF_NAME);
    sal_Int32 nHighestBookmarkId = 0;
    for (int i = 0, nCount = m_xControl->n_children(); i < nCount; ++i)
    {
        sw::mark::IMark* pBookmark = reinterpret_cast<sw::mark::IMark*>(m_xControl->get_id(i).toInt64());
        const OUString& sName = pBookmark->GetName();
        sal_Int32 nIndex = 0;
        if (sName.getToken(0, ' ', nIndex) == sDefaultBookmarkName)
        {
            sal_Int32 nCurrBookmarkId = sName.getToken(0, ' ', nIndex).toInt32();
            nHighestBookmarkId = std::max<sal_Int32>(nHighestBookmarkId, nCurrBookmarkId);
        }
    }
    return sDefaultBookmarkName + " " + OUString::number(nHighestBookmarkId + 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
