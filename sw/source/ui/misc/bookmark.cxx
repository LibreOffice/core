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

#include <rtl/ustrbuf.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/weld.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <officecfg/Office/Common.hxx>

#include <swabstdlg.hxx>
#include <swuiexp.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>
#include <bookmark.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <strings.hrc>
#include <IDocumentSettingAccess.hxx>

using namespace ::com::sun::star;

const char BookmarkTable::s_cSeparator(';');

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
        sTmp = sTmp.replaceAll(OUStringChar(BookmarkTable::aForbiddenChars.getStr()[i]), "");
        if (sTmp.getLength() != nTmpLen)
            sMsg += OUStringChar(BookmarkTable::aForbiddenChars.getStr()[i]);
    }
    const bool bHasForbiddenChars = sTmp.getLength() != nLen;
    m_xForbiddenChars->set_visible(bHasForbiddenChars);
    if (bHasForbiddenChars)
        m_xEditBox->set_message_type(weld::EntryMessageType::Error);
    else
        m_xEditBox->set_message_type(weld::EntryMessageType::Normal);

    sal_Int32 nSelectedEntries = 0;
    sal_Int32 nEntries = 0;
    sal_Int32 nTokenIndex = 0;
    while (!sTmp.isEmpty() && nTokenIndex >= 0)
    {
        OUString aToken = sTmp.getToken(0, BookmarkTable::s_cSeparator, nTokenIndex);
        if (m_xBookmarksBox->GetBookmarkByName(aToken))
        {
            m_xBookmarksBox->SelectByName(aToken);
            nSelectedEntries++;
        }
        nEntries++;
    }

    // allow to add new bookmark only if one name provided and it's not taken
    m_xInsertBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 0 && !bHasForbiddenChars
                                && !m_bAreProtected);

    // allow to delete only if all bookmarks are recognized
    m_xDeleteBtn->set_sensitive(nEntries > 0 && nSelectedEntries == nEntries && !m_bAreProtected);
    m_xGotoBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 1);
    m_xEditTextBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 1);
    m_xRenameBtn->set_sensitive(nEntries == 1 && nSelectedEntries == 1 && !m_bAreProtected);
}

// callback to delete a text mark
IMPL_LINK_NOARG(SwInsertBookmarkDlg, DeleteHdl, weld::Button&, void)
{
    if (!ValidateBookmarks())
        return;

    int nSelectedRows(0);

    m_xBookmarksBox->selected_foreach([this, &nSelectedRows](weld::TreeIter& rEntry) {
        // remove from model
        sw::mark::IMark* pBookmark
            = weld::fromId<sw::mark::IMark*>(m_xBookmarksBox->get_id(rEntry));
        OUString sRemoved = pBookmark->GetName();
        IDocumentMarkAccess* const pMarkAccess = m_rSh.getIDocumentMarkAccess();
        pMarkAccess->deleteMark(pMarkAccess->findMark(sRemoved), false);
        SfxRequest aReq(m_rSh.GetView().GetViewFrame(), FN_DELETE_BOOKMARK);
        aReq.AppendItem(SfxStringItem(FN_DELETE_BOOKMARK, sRemoved));
        aReq.Done();
        std::erase(m_aTableBookmarks, std::make_pair(pBookmark, sRemoved));

        ++nSelectedRows;

        return false;
    });

    if (!nSelectedRows)
        return;

    // remove from BookmarkTable
    m_xBookmarksBox->remove_selection();

    ValidateBookmarks();

    m_xDeleteBtn->set_sensitive(false);
    m_xGotoBtn->set_sensitive(false);
    m_xEditTextBtn->set_sensitive(false);
    m_xRenameBtn->set_sensitive(false);
    m_xInsertBtn->set_sensitive(false);
}

// callback to a goto button
IMPL_LINK_NOARG(SwInsertBookmarkDlg, GotoHdl, weld::Button&, void) { GotoSelectedBookmark(); }

IMPL_LINK_NOARG(SwInsertBookmarkDlg, DoubleClickHdl, weld::TreeView&, bool)
{
    GotoSelectedBookmark();
    return true;
}

IMPL_LINK_NOARG(SwInsertBookmarkDlg, SelectionChangedHdl, weld::TreeView&, void)
{
    if (!ValidateBookmarks())
        return;
    // this event should fired only if we change selection by clicking on BookmarkTable entry
    if (!m_xBookmarksBox->has_focus())
        return;

    SelectionChanged();
}

void SwInsertBookmarkDlg::SelectionChanged()
{
    OUStringBuffer sEditBoxText;
    int nSelectedRows = 0;
    m_xBookmarksBox->selected_foreach(
        [this, &sEditBoxText, &nSelectedRows](weld::TreeIter& rEntry) {
            sw::mark::IMark* pBookmark
                = weld::fromId<sw::mark::IMark*>(m_xBookmarksBox->get_id(rEntry));
            const OUString& sEntryName = pBookmark->GetName();
            if (!sEditBoxText.isEmpty())
                sEditBoxText.append(";");
            sEditBoxText.append(sEntryName);
            ++nSelectedRows;
            return false;
        });
    if (nSelectedRows)
    {
        m_xInsertBtn->set_sensitive(false);
        m_xGotoBtn->set_sensitive(nSelectedRows == 1);
        m_xEditTextBtn->set_sensitive(nSelectedRows == 1);
        m_xRenameBtn->set_sensitive(nSelectedRows == 1 && !m_bAreProtected);
        m_xDeleteBtn->set_sensitive(!m_bAreProtected);
        m_xEditBox->set_text(sEditBoxText.makeStringAndClear());
    }
    else
    {
        m_xInsertBtn->set_sensitive(!m_bAreProtected);
        m_xGotoBtn->set_sensitive(false);
        m_xEditTextBtn->set_sensitive(false);
        m_xRenameBtn->set_sensitive(false);
        m_xDeleteBtn->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SwInsertBookmarkDlg, EditTextHdl, weld::Button&, void)
{
    if (!ValidateBookmarks())
        return;
    auto pSelected = m_xBookmarksBox->get_selected();
    if (!pSelected)
        return;

    m_xBookmarksBox->start_editing(*pSelected);
}

IMPL_LINK_NOARG(SwInsertBookmarkDlg, RenameHdl, weld::Button&, void)
{
    if (!ValidateBookmarks())
        return;
    auto xSelected = m_xBookmarksBox->get_selected();
    if (!xSelected)
        return;

    sw::mark::IMark* pBookmark
        = weld::fromId<sw::mark::IMark*>(m_xBookmarksBox->get_id(*xSelected));
    uno::Reference<frame::XModel> xModel = m_rSh.GetView().GetDocShell()->GetBaseModel();
    uno::Reference<text::XBookmarksSupplier> xBkms(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess = xBkms->getBookmarks();
    uno::Any aObj = xNameAccess->getByName(pBookmark->GetName());
    uno::Reference<uno::XInterface> xTmp;
    aObj >>= xTmp;
    uno::Reference<container::XNamed> xNamed(xTmp, uno::UNO_QUERY);
    SwAbstractDialogFactory& rFact = swui::GetFactory();
    VclPtr<AbstractSwRenameXNamedDlg> pDlg(
        rFact.CreateSwRenameXNamedDlg(m_xDialog.get(), xNamed, xNameAccess));
    pDlg->SetForbiddenChars(BookmarkTable::aForbiddenChars
                            + OUStringChar(BookmarkTable::s_cSeparator));

    pDlg->StartExecuteAsync([pDlg, this](sal_Int32 nResult) {
        if (nResult == RET_OK)
        {
            ValidateBookmarks();
            m_xDeleteBtn->set_sensitive(false);
            m_xGotoBtn->set_sensitive(false);
            m_xEditTextBtn->set_sensitive(false);
            m_xRenameBtn->set_sensitive(false);
            m_xInsertBtn->set_sensitive(false);
        }
        pDlg->disposeOnce();
    });
}

// callback to an insert button. Inserts a new text mark to the current position.
IMPL_LINK_NOARG(SwInsertBookmarkDlg, InsertHdl, weld::Button&, void)
{
    OUString sBookmark = m_xEditBox->get_text();
    m_rSh.SetBookmark2(vcl::KeyCode(), sBookmark, m_xHideCB->get_active(),
                       m_xConditionED->get_text());

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwInsertBookmarkDlg, ChangeHideHdl, weld::Toggleable&, rBox, void)
{
    bool bHide = rBox.get_active();
    m_xConditionED->set_sensitive(bHide);
    m_xConditionFT->set_sensitive(bHide);
}

IMPL_LINK(SwInsertBookmarkDlg, EditingHdl, weld::TreeIter const&, rIter, bool)
{
    sw::mark::IMark const* const pBookmark(
        weld::fromId<sw::mark::IMark*>(m_xBookmarksBox->get_id(rIter)));
    assert(pBookmark);
    return pBookmark->IsExpanded()
           && pBookmark->GetMarkPos().GetNode() == pBookmark->GetOtherMarkPos().GetNode()
           && !m_xBookmarksBox->get_text(rIter).endsWith(u"…");
}

IMPL_LINK(SwInsertBookmarkDlg, EditedHdl, weld::TreeView::iter_string const&, rIterString, bool)
{
    sw::mark::IMark const* const pBookmark(
        weld::fromId<sw::mark::IMark*>(m_xBookmarksBox->get_id(rIterString.first)));
    assert(pBookmark);
    bool bRet(false);
    if (pBookmark->GetMarkPos() != pBookmark->GetOtherMarkPos())
    {
        if (pBookmark->GetMarkPos().GetNode() != pBookmark->GetOtherMarkPos().GetNode())
        {
            return false; // don't allow editing if it spans multiple nodes
        }
        m_rSh.Push();
        m_rSh.GotoMark(pBookmark);
        // GetSelText only works for 1 paragraph, but it's checked above
        if (m_rSh.GetSelText() != rIterString.second)
        {
            bRet = m_rSh.Replace(rIterString.second, false);
        }
        m_rSh.Pop(SwEditShell::PopMode::DeleteCurrent);
    }
    else if (pBookmark->IsExpanded() && !rIterString.second.isEmpty())
    { // SwEditShell::Replace does nothing for empty selection
        m_rSh.Insert(rIterString.second);
        bRet = true;
    }
    return bRet;
}

void SwInsertBookmarkDlg::GotoSelectedBookmark()
{
    if (!ValidateBookmarks())
        return;
    // if no entries selected we can't jump anywhere
    // shouldn't be needed as we disable GoTo button when jump is not possible
    auto xSelected = m_xBookmarksBox->get_selected();
    if (!xSelected)
        return;

    sw::mark::IMark* pBookmark
        = weld::fromId<sw::mark::IMark*>(m_xBookmarksBox->get_id(*xSelected));

    m_rSh.EnterStdMode();
    m_rSh.GotoMark(pBookmark);
}

bool SwInsertBookmarkDlg::ValidateBookmarks()
{
    if (HaveBookmarksChanged())
    {
        PopulateTable();
        m_xEditBox->set_text(u""_ustr);
        return false;
    }
    return true;
}

bool SwInsertBookmarkDlg::HaveBookmarksChanged()
{
    IDocumentMarkAccess* const pMarkAccess = m_rSh.getIDocumentMarkAccess();
    if (pMarkAccess->getBookmarksCount() != m_nLastBookmarksCount)
        return true;

    std::vector<std::pair<sw::mark::IMark*, OUString>>::const_iterator aListIter
        = m_aTableBookmarks.begin();
    for (IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
         ppBookmark != pMarkAccess->getBookmarksEnd(); ++ppBookmark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            // more bookmarks then expected
            if (aListIter == m_aTableBookmarks.end())
                return true;
            if (aListIter->first != *ppBookmark || aListIter->second != (*ppBookmark)->GetName())
                return true;
            ++aListIter;
        }
    }
    // less bookmarks then expected
    return aListIter != m_aTableBookmarks.end();
}

void SwInsertBookmarkDlg::PopulateTable()
{
    m_aTableBookmarks.clear();
    m_xBookmarksBox->clear();

    IDocumentMarkAccess* const pMarkAccess = m_rSh.getIDocumentMarkAccess();
    for (IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
         ppBookmark != pMarkAccess->getBookmarksEnd(); ++ppBookmark)
    {
        if (IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            m_xBookmarksBox->InsertBookmark(m_rSh, *ppBookmark);
            m_aTableBookmarks.emplace_back(*ppBookmark, (*ppBookmark)->GetName());
        }
    }
    m_nLastBookmarksCount = pMarkAccess->getBookmarksCount();
}

SwInsertBookmarkDlg::SwInsertBookmarkDlg(weld::Window* pParent, SwWrtShell& rS,
                                         OUString const* const pSelected)
    : SfxDialogController(pParent, u"modules/swriter/ui/insertbookmark.ui"_ustr,
                          u"InsertBookmarkDialog"_ustr)
    , m_rSh(rS)
    , m_nLastBookmarksCount(0)
    , m_bSorted(false)
    , m_xEditBox(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xInsertBtn(m_xBuilder->weld_button(u"insert"_ustr))
    , m_xDeleteBtn(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xGotoBtn(m_xBuilder->weld_button(u"goto"_ustr))
    , m_xEditTextBtn(m_xBuilder->weld_button(u"edittext"_ustr))
    , m_xRenameBtn(m_xBuilder->weld_button(u"rename"_ustr))
    , m_xHideCB(m_xBuilder->weld_check_button(u"hide"_ustr))
    , m_xConditionFT(m_xBuilder->weld_label(u"condlabel"_ustr))
    , m_xConditionED(new ConditionEdit(m_xBuilder->weld_entry(u"withcond"_ustr)))
    , m_xBookmarksBox(new BookmarkTable(m_xBuilder->weld_tree_view(u"bookmarks"_ustr)))
    , m_xForbiddenChars(m_xBuilder->weld_label(u"lbForbiddenChars"_ustr))
{
    m_xBookmarksBox->connect_changed(LINK(this, SwInsertBookmarkDlg, SelectionChangedHdl));
    m_xBookmarksBox->connect_row_activated(LINK(this, SwInsertBookmarkDlg, DoubleClickHdl));
    m_xBookmarksBox->connect_column_clicked(LINK(this, SwInsertBookmarkDlg, HeaderBarClick));
    m_xBookmarksBox->connect_editing(LINK(this, SwInsertBookmarkDlg, EditingHdl),
                                     LINK(this, SwInsertBookmarkDlg, EditedHdl));
    m_xEditBox->connect_changed(LINK(this, SwInsertBookmarkDlg, ModifyHdl));
    m_xInsertBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, InsertHdl));
    m_xDeleteBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, DeleteHdl));
    m_xGotoBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, GotoHdl));
    m_xEditTextBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, EditTextHdl));
    m_xRenameBtn->connect_clicked(LINK(this, SwInsertBookmarkDlg, RenameHdl));
    m_xHideCB->connect_toggled(LINK(this, SwInsertBookmarkDlg, ChangeHideHdl));

    m_xDeleteBtn->set_sensitive(false);
    m_xGotoBtn->set_sensitive(false);
    m_xEditTextBtn->set_sensitive(false);
    m_xRenameBtn->set_sensitive(false);

    // select 3rd column, otherwise it'll pick 1st one
    m_xBookmarksBox->set_column_editables({ false, false, true, false, false });

    PopulateTable();

    m_xEditBox->set_text(m_xBookmarksBox->GetNameProposal());
    m_xEditBox->set_position(-1);

    m_xForbiddenChars->set_label(SwResId(STR_BOOKMARK_FORBIDDENCHARS) + " "
                                 + BookmarkTable::aForbiddenChars);
    m_xForbiddenChars->set_visible(false);

    if (!officecfg::Office::Common::Misc::ExperimentalMode::get())
    {
        m_xHideCB->set_visible(false);
        m_xConditionFT->set_visible(false);
        m_xConditionED->set_visible(false);
    }

    m_bAreProtected = m_rSh.getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS);

    // disabled until "Hide" flag is not checked
    m_xConditionED->set_sensitive(false);
    m_xConditionFT->set_sensitive(false);

    // restore dialog size
    SvtViewOptions aDlgOpt(EViewType::Dialog, u"BookmarkDialog"_ustr);
    if (aDlgOpt.Exists())
        m_xDialog->set_window_state(aDlgOpt.GetWindowState());

    if (pSelected)
    {
        if (m_xBookmarksBox->SelectByName(*pSelected))
        {
            SelectionChanged();
            // which is better, focus on a button or focus on the table row?
            // as long as editing doesn't work via the TreeView with VCL
            // widgets, better on button.
            m_xEditTextBtn->grab_focus();
        }
    }
}

SwInsertBookmarkDlg::~SwInsertBookmarkDlg()
{
    // tdf#146261 - Remember size of bookmark dialog
    SvtViewOptions aDlgOpt(EViewType::Dialog, u"BookmarkDialog"_ustr);
    OUString sWindowState = m_xDialog->get_window_state(vcl::WindowDataMask::PosSize);
    aDlgOpt.SetWindowState(sWindowState);
}

IMPL_LINK(SwInsertBookmarkDlg, HeaderBarClick, int, nColumn, void)
{
    if (!m_bSorted)
    {
        m_xBookmarksBox->make_sorted();
        m_bSorted = true;
    }

    bool bSortAtoZ = m_xBookmarksBox->get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn == m_xBookmarksBox->get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        m_xBookmarksBox->set_sort_order(bSortAtoZ);
    }
    else
    {
        int nOldSortColumn = m_xBookmarksBox->get_sort_column();
        if (nOldSortColumn != -1)
            m_xBookmarksBox->set_sort_indicator(TRISTATE_INDET, nOldSortColumn);
        m_xBookmarksBox->set_sort_column(nColumn);
    }

    if (nColumn != -1)
    {
        //sort lists
        m_xBookmarksBox->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
    }
}

BookmarkTable::BookmarkTable(std::unique_ptr<weld::TreeView> xControl)
    : m_xControl(std::move(xControl))
{
    m_xControl->set_size_request(-1, m_xControl->get_height_rows(8));
    m_xControl->set_column_fixed_widths({ 40, 110, 150, 160 });
    m_xControl->set_selection_mode(SelectionMode::Multiple);
}

std::unique_ptr<weld::TreeIter> BookmarkTable::get_selected() const
{
    std::unique_ptr<weld::TreeIter> xIter(m_xControl->make_iterator());
    if (!m_xControl->get_selected(xIter.get()))
        xIter.reset();
    return xIter;
}

void BookmarkTable::InsertBookmark(SwWrtShell& rSh, sw::mark::IMark* const pMark)
{
    sw::mark::IBookmark* pBookmark = dynamic_cast<sw::mark::IBookmark*>(pMark);
    assert(pBookmark);

    OUString sBookmarkNodeText;
    static const sal_Int32 nMaxTextLen = 50;

    if (pBookmark->IsExpanded())
    {
        rSh.Push();
        rSh.GotoMark(pBookmark);
        rSh.GetSelectedText(sBookmarkNodeText, ParaBreakType::ToBlank);
        rSh.Pop(SwEditShell::PopMode::DeleteCurrent);
    }
    if (nMaxTextLen < sBookmarkNodeText.getLength())
    {
        sBookmarkNodeText = sBookmarkNodeText.subView(0, nMaxTextLen);
        ;
        sBookmarkNodeText += u"…";
    }

    const OUString& sHideCondition = pBookmark->GetHideCondition();
    const OUString& sName = pBookmark->GetName();
    OUString sHidden
        = (pBookmark->IsHidden() || !sHideCondition.isEmpty() ||
           // tdf#150955 add "hidden" status to the imported OOXML _Toc and _Ref bookmarks
           // to allow separating custom bookmarks by sorting based on their Hidden status.
           // Note: this "hidden" means here only that these bookmarks haven't got
           // visible bookmark formatting aids (gray I-shape or brackets), otherwise
           // their anchor are still visible.
           sName.startsWith("_Toc") || sName.startsWith("_Ref"))
              ? SwResId(STR_BOOKMARK_YES)
              : SwResId(STR_BOOKMARK_NO);

    OUString sPageNum = OUString::number(SwPaM(pMark->GetMarkStart()).GetPageNum());
    int nRow = m_xControl->n_children();
    m_xControl->append(weld::toId(pMark), sPageNum);
    m_xControl->set_text(nRow, sName, 1);
    m_xControl->set_text(nRow, sBookmarkNodeText, 2);
    m_xControl->set_text(nRow, sHidden, 3);
    m_xControl->set_text(nRow, sHideCondition, 4);
}

std::unique_ptr<weld::TreeIter> BookmarkTable::GetRowByBookmarkName(const OUString& sName)
{
    std::unique_ptr<weld::TreeIter> xRet;
    m_xControl->all_foreach([this, &sName, &xRet](weld::TreeIter& rEntry) {
        sw::mark::IMark* pBookmark = weld::fromId<sw::mark::IMark*>(m_xControl->get_id(rEntry));
        if (pBookmark->GetName() == sName)
        {
            xRet = m_xControl->make_iterator(&rEntry);
            return true;
        }
        return false;
    });
    return xRet;
}

sw::mark::IMark* BookmarkTable::GetBookmarkByName(const OUString& sName)
{
    auto xEntry = GetRowByBookmarkName(sName);
    if (!xEntry)
        return nullptr;

    return weld::fromId<sw::mark::IMark*>(m_xControl->get_id(*xEntry));
}

bool BookmarkTable::SelectByName(const OUString& sName)
{
    auto xEntry = GetRowByBookmarkName(sName);
    if (!xEntry)
        return false;
    select(*xEntry);
    return true;
}

OUString BookmarkTable::GetNameProposal() const
{
    OUString sDefaultBookmarkName = SwResId(STR_BOOKMARK_DEF_NAME);
    sal_Int32 nHighestBookmarkId = 0;
    for (int i = 0, nCount = m_xControl->n_children(); i < nCount; ++i)
    {
        sw::mark::IMark* pBookmark = weld::fromId<sw::mark::IMark*>(m_xControl->get_id(i));
        const OUString& sName = pBookmark->GetName();
        sal_Int32 nIndex = 0;
        if (o3tl::getToken(sName, 0, ' ', nIndex) == sDefaultBookmarkName)
        {
            sal_Int32 nCurrBookmarkId = o3tl::toInt32(o3tl::getToken(sName, 0, ' ', nIndex));
            nHighestBookmarkId = std::max<sal_Int32>(nHighestBookmarkId, nCurrBookmarkId);
        }
    }
    return sDefaultBookmarkName + " " + OUString::number(nHighestBookmarkId + 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
