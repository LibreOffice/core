/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatedlglocalview.hxx>

#include <comphelper/string.hxx>
#include <sfx2/inputdlg.hxx>
#include <templateviewitem.hxx>
#include <sfx2/sfxresid.hxx>
#include <templatecontaineritem.hxx>
#include <sfx2/strings.hrc>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <sfx2/doctempl.hxx>
#include <helpids.h>
#include <bitmaps.hlst>

TemplateDlgLocalView::TemplateDlgLocalView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                                           std::unique_ptr<weld::Menu> xMenu,
                                           std::unique_ptr<weld::TreeView> xTreeView)
    : TemplateLocalView(std::move(xWindow), std::move(xMenu))
    , ListView(std::move(xTreeView))
    , mViewMode(TemplateViewMode::eThumbnailView)
{
    mxTreeView->connect_row_activated(LINK(this, TemplateDlgLocalView, RowActivatedHdl));
    mxTreeView->connect_column_clicked(LINK(this, ListView, ColumnClickedHdl));
    mxTreeView->connect_changed(LINK(this, TemplateDlgLocalView, ListViewChangedHdl));
    mxTreeView->connect_popup_menu(LINK(this, TemplateDlgLocalView, PopupMenuHdl));
    mxTreeView->connect_key_press(LINK(this, TemplateDlgLocalView, KeyPressHdl));
}

void TemplateDlgLocalView::showAllTemplates()
{
    mnCurRegionId = 0;

    insertItems(maAllTemplates, false, true);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateDlgLocalView::showRegion(TemplateContainerItem const* pItem)
{
    mnCurRegionId = pItem->mnRegionId + 1;

    insertItems(pItem->maTemplates, true, false);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateDlgLocalView::showRegion(std::u16string_view rName)
{
    for (auto const& pRegion : maRegions)
    {
        if (pRegion->maTitle == rName)
        {
            showRegion(pRegion.get());
            break;
        }
    }
}

void TemplateDlgLocalView::reload()
{
    mpDocTemplates->Update();
    OUString sCurRegionName = getRegionItemName(mnCurRegionId);
    Populate();
    mnCurRegionId = getRegionId(sCurRegionName);

    // Check if we are currently browsing a region or root folder
    if (mnCurRegionId)
    {
        sal_uInt16 nRegionId = mnCurRegionId - 1; //Is offset by 1

        for (auto const& pRegion : maRegions)
        {
            if (pRegion->mnRegionId == nRegionId)
            {
                showRegion(pRegion.get());
                break;
            }
        }
    }
    else
        showAllTemplates();

    //No items should be selected by default
    ThumbnailView::deselectItems();
    ListView::unselect_all();
}

void TemplateDlgLocalView::createContextMenu(const bool bIsDefault, const bool bIsBuiltIn,
                                             const bool bIsSingleSel, const OUString& rDefaultImg)
{
    mxContextMenu->clear();
    mxContextMenu->append(u"open"_ustr, SfxResId(STR_OPEN), BMP_MENU_OPEN);
    mxContextMenu->append(u"edit"_ustr, SfxResId(STR_EDIT_TEMPLATE), BMP_MENU_EDIT);

    if (!bIsDefault)
        mxContextMenu->append(u"default"_ustr, SfxResId(STR_DEFAULT_TEMPLATE), rDefaultImg);
    else
        mxContextMenu->append(u"default"_ustr, SfxResId(STR_RESET_DEFAULT), rDefaultImg);

    mxContextMenu->append_separator(u"separator1"_ustr);
    mxContextMenu->append(u"rename"_ustr, SfxResId(STR_SFX_RENAME), BMP_MENU_RENAME);
    mxContextMenu->append(u"delete"_ustr, SfxResId(STR_DELETE_TEMPLATE), BMP_MENU_DELETE);
    mxContextMenu->append_separator(u"separator2"_ustr);
    mxContextMenu->append(u"move"_ustr, SfxResId(STR_MOVE), BMP_MENU_MOVE);
    mxContextMenu->append(u"export"_ustr, SfxResId(STR_EXPORT), BMP_MENU_EXPORT);

    // Set the help IDs of all context menu entries
    mxContextMenu->set_item_help_id(u"open"_ustr, HID_TEMPLATEDLG_OPEN);
    mxContextMenu->set_item_help_id(u"edit"_ustr, HID_TEMPLATEDLG_EDIT);
    mxContextMenu->set_item_help_id(u"default"_ustr, HID_TEMPLATEDLG_DEFAULT);
    mxContextMenu->set_item_help_id(u"rename"_ustr, HID_TEMPLATEDLG_RENAME);
    mxContextMenu->set_item_help_id(u"delete"_ustr, HID_TEMPLATEDLG_DELETE);
    mxContextMenu->set_item_help_id(u"move"_ustr, HID_TEMPLATEDLG_MOVE);
    mxContextMenu->set_item_help_id(u"export"_ustr, HID_TEMPLATEDLG_EXPORT);

    if (!bIsSingleSel)
    {
        mxContextMenu->set_sensitive(u"open"_ustr, false);
        mxContextMenu->set_sensitive(u"edit"_ustr, false);
        mxContextMenu->set_sensitive(u"default"_ustr, false);
        mxContextMenu->set_sensitive(u"rename"_ustr, false);
    }
    if (bIsBuiltIn)
    {
        mxContextMenu->set_sensitive(u"rename"_ustr, false);
        mxContextMenu->set_sensitive(u"delete"_ustr, false);
    }
    if (mViewMode == TemplateViewMode::eThumbnailView)
    {
        ContextMenuSelectHdl(mxContextMenu->popup_at_rect(
            GetDrawingArea(), tools::Rectangle(maPosition, Size(1, 1))));
        Invalidate();
    }
    else if (mViewMode == TemplateViewMode::eListView)
        ContextMenuSelectHdl(mxContextMenu->popup_at_rect(
            mxTreeView.get(), tools::Rectangle(maPosition, Size(1, 1))));
}

void TemplateDlgLocalView::ContextMenuSelectHdl(std::u16string_view rIdent)
{
    if (rIdent == u"open")
        maOpenTemplateHdl.Call(maSelectedItem);
    else if (rIdent == u"edit")
        maEditTemplateHdl.Call(maSelectedItem);
    else if (rIdent == u"rename")
    {
        InputDialog aTitleEditDlg(GetDrawingArea(), SfxResId(STR_RENAME_TEMPLATE));
        aTitleEditDlg.set_title(SfxResId(STR_WINDOW_TITLE_RENAME_TEMPLATE));
        OUString sOldTitle = maSelectedItem->getTitle();
        aTitleEditDlg.SetEntryText(sOldTitle);
        aTitleEditDlg.HideHelpBtn();

        auto aCurRegionItems = getFilteredItems([&](const TemplateItemProperties& rItem) {
            return rItem.aRegionName == getRegionName(maSelectedItem->mnRegionId);
        });
        OUString sTooltip(SfxResId(STR_TOOLTIP_ERROR_RENAME_TEMPLATE));
        sTooltip = sTooltip.replaceFirst("$2", getRegionName(maSelectedItem->mnRegionId));
        aTitleEditDlg.setCheckEntry([&](OUString sNewTitle) {
            if (sNewTitle.isEmpty() || sNewTitle == sOldTitle)
                return true;
            for (const auto& rItem : aCurRegionItems)
            {
                if (rItem.aName == sNewTitle)
                {
                    aTitleEditDlg.SetTooltip(sTooltip.replaceFirst("$1", sNewTitle));
                    return false;
                }
            }
            return true;
        });
        if (!aTitleEditDlg.run())
            return;
        OUString sNewTitle = comphelper::string::strip(aTitleEditDlg.GetEntryText(), ' ');

        if (!sNewTitle.isEmpty() && sNewTitle != sOldTitle)
        {
            maSelectedItem->setTitle(sNewTitle);
            ListView::rename(OUString::number(maSelectedItem->mnId), maSelectedItem->maTitle);
        }
    }
    else if (rIdent == u"delete")
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(
            GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
            SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return;

        maDeleteTemplateHdl.Call(maSelectedItem);
        reload();
    }
    else if (rIdent == u"default")
    {
        maDefaultTemplateHdl.Call(maSelectedItem);
        ListView::refreshDefaultColumn();
    }
    else if (rIdent == u"move")
    {
        maMoveTemplateHdl.Call(maSelectedItem);
    }
    else if (rIdent == u"export")
    {
        maExportTemplateHdl.Call(maSelectedItem);
    }
}

void TemplateDlgLocalView::insertFilteredItems()
{
    ListView::clearListView();
    for (const ThumbnailViewItem* rItem : mFilteredItemList)
    {
        const TemplateViewItem* pViewItem = static_cast<const TemplateViewItem*>(rItem);
        if (!pViewItem)
            return;
        bool isDefault = pViewItem->IsDefaultTemplate();
        OUString sId = OUString::number(pViewItem->mnId);
        ListView::AppendItem(sId, rItem->maTitle, getRegionName(pViewItem->mnRegionId),
                             pViewItem->getPath(), isDefault);
    }
    ListView::sort();
}

void TemplateDlgLocalView::insertItems(const std::vector<TemplateItemProperties>& rTemplates,
                                       bool isRegionSelected = true,
                                       bool bShowCategoryInTooltip = false)
{
    TemplateLocalView::insertItems(rTemplates, isRegionSelected, bShowCategoryInTooltip);
    insertFilteredItems();
}

void TemplateDlgLocalView::setTemplateViewMode(TemplateViewMode eMode) { mViewMode = eMode; }

void TemplateDlgLocalView::Show()
{
    if (mViewMode == TemplateViewMode::eListView)
    {
        ThumbnailView::Hide();
        ListView::ShowListView();
    }
    else
    {
        ThumbnailView::Show();
        ListView::HideListView();
    }
    syncCursor();
}
void TemplateDlgLocalView::Hide()
{
    ThumbnailView::Hide();
    ListView::HideListView();
}

bool TemplateDlgLocalView::IsVisible() const
{
    return ThumbnailView::IsVisible() || ListView::IsListViewVisible();
}

void TemplateDlgLocalView::syncCursor()
{
    if (mViewMode == TemplateViewMode::eListView)
    {
        ListView::unselect_all();
        int nIndex = -1;

        for (auto it = mFilteredItemList.cbegin(); it != mFilteredItemList.cend(); ++it)
        {
            if ((*it)->mbSelected)
            {
                nIndex = ListView::get_index((*it)->mnId);
                if (nIndex >= 0)
                {
                    ListView::set_cursor(nIndex);
                    ListView::select(nIndex);
                    break;
                }
            }
        }
        updateSelection();
    }
    else
    {
        ThumbnailView::deselectItems();
        std::vector<int> aSelRows = ListView::get_selected_rows();
        if (aSelRows.empty())
            return;
        sal_uInt16 nCursorId = ListView::get_cursor_nId();
        ThumbnailView::SelectItem(nCursorId);
        MakeItemVisible(nCursorId);

        for (auto it = mFilteredItemList.begin(); it != mFilteredItemList.end(); ++it)
        {
            if ((*it)->mnId == nCursorId)
            {
                mpStartSelRange = it;
                break;
            }
        }

        size_t nPos = GetItemPos(nCursorId);
        ThumbnailViewItem* pItem = ImplGetItem(nPos);
        const TemplateViewItem* pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);
        if (pViewItem)
            maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
    }
}

void TemplateDlgLocalView::updateSelection()
{
    ThumbnailView::deselectItems();
    for (auto nIndex : ListView::get_selected_rows())
    {
        ThumbnailView::SelectItem(ListView::get_nId(nIndex));
    }

    sal_uInt16 nCursorId = ListView::get_cursor_nId();
    size_t nPos = GetItemPos(nCursorId);
    ThumbnailViewItem* pItem = ImplGetItem(nPos);
    const TemplateViewItem* pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);
    if (pViewItem)
        maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
    return;
}

IMPL_LINK_NOARG(TemplateDlgLocalView, RowActivatedHdl, weld::TreeView&, bool)
{
    maOpenTemplateHdl.Call(maSelectedItem);
    return true;
}

IMPL_LINK(TemplateDlgLocalView, PopupMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    if (rCEvt.IsMouseEvent())
    {
        if (ListView::get_selected_rows().empty())
            return true;
        Point aPosition(rCEvt.GetMousePosPixel());
        maPosition = aPosition;
        updateSelection();
        if (maSelectedItem)
            maCreateContextMenuHdl.Call(maSelectedItem);
        return true;
    }
    else
    {
        if (ListView::get_selected_rows().empty())
            return true;
        maPosition = Point(0, 0);
        updateSelection();
        if (maSelectedItem)
            maCreateContextMenuHdl.Call(maSelectedItem);
        return true;
    }
}

IMPL_LINK_NOARG(TemplateDlgLocalView, ListViewChangedHdl, weld::TreeView&, void)
{
    updateSelection();
}

bool TemplateDlgLocalView::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if (aKeyCode == (KEY_MOD1 | KEY_A))
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (!pItem->isSelected())
            {
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
            }
        }

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
        return true;
    }
    else if (aKeyCode == KEY_DELETE && !mFilteredItemList.empty())
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(
            GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
            SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return true;

        maDeleteTemplateHdl.Call(maSelectedItem);
        reload();
    }

    return ThumbnailView::KeyInput(rKEvt);
}

IMPL_LINK(TemplateDlgLocalView, KeyPressHdl, const KeyEvent&, rKEvt, bool)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if (aKeyCode == KEY_DELETE && !mFilteredItemList.empty()
        && !ListView::get_selected_rows().empty())
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(
            mxTreeView.get(), VclMessageType::Question, VclButtonsType::YesNo,
            SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return true;

        maDeleteTemplateHdl.Call(maSelectedItem);
        reload();
    }
    return false;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
