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
}

void TemplateDlgLocalView::showAllTemplates()
{
    mnCurRegionId = 0;

    insertItems(maAllTemplates, false, true);
    insertFilteredItems();

    maOpenRegionHdl.Call(nullptr);
}

void TemplateDlgLocalView::showRegion(TemplateContainerItem const* pItem)
{
    mnCurRegionId = pItem->mnRegionId + 1;

    insertItems(pItem->maTemplates);
    insertFilteredItems();

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

void TemplateDlgLocalView::createContextMenu(const bool bIsDefault, const bool bIsBuiltIn)
{
    mxContextMenu->clear();
    mxContextMenu->append("open", SfxResId(STR_OPEN));
    mxContextMenu->append("edit", SfxResId(STR_EDIT_TEMPLATE));

    if (!bIsDefault)
        mxContextMenu->append("default", SfxResId(STR_DEFAULT_TEMPLATE));
    else
        mxContextMenu->append("default", SfxResId(STR_RESET_DEFAULT));

    mxContextMenu->append_separator("separator");
    mxContextMenu->append("rename", SfxResId(STR_SFX_RENAME));
    mxContextMenu->append("delete", SfxResId(STR_DELETE));
    if (bIsBuiltIn)
    {
        mxContextMenu->set_sensitive("rename", false);
        mxContextMenu->set_sensitive("edit", false);
        mxContextMenu->set_sensitive("delete", false);
    }
    if (mViewMode == TemplateViewMode::eThumbnailView)
    {
        deselectItems();
        maSelectedItem->setSelection(true);
        maItemStateHdl.Call(maSelectedItem);
        ContextMenuSelectHdl(mxContextMenu->popup_at_rect(
            GetDrawingArea(), tools::Rectangle(maPosition, Size(1, 1))));
        Invalidate();
    }
    else if (mViewMode == TemplateViewMode::eListView)
        ContextMenuSelectHdl(mxContextMenu->popup_at_rect(
            mxTreeView.get(), tools::Rectangle(maPosition, Size(1, 1))));
}

void TemplateDlgLocalView::ContextMenuSelectHdl(std::string_view rIdent)
{
    if (rIdent == "open")
        maOpenTemplateHdl.Call(maSelectedItem);
    else if (rIdent == "edit")
        maEditTemplateHdl.Call(maSelectedItem);
    else if (rIdent == "rename")
    {
        InputDialog aTitleEditDlg(GetDrawingArea(), SfxResId(STR_RENAME_TEMPLATE));
        OUString sOldTitle = maSelectedItem->getTitle();
        aTitleEditDlg.SetEntryText(sOldTitle);
        aTitleEditDlg.HideHelpBtn();

        if (!aTitleEditDlg.run())
            return;
        OUString sNewTitle = comphelper::string::strip(aTitleEditDlg.GetEntryText(), ' ');

        if (!sNewTitle.isEmpty() && sNewTitle != sOldTitle)
        {
            maSelectedItem->setTitle(sNewTitle);
        }
        ListView::rename(OUString::number(maSelectedItem->mnId), maSelectedItem->maTitle);
    }
    else if (rIdent == "delete")
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(
            GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
            SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return;

        maDeleteTemplateHdl.Call(maSelectedItem);
        // this remove is probably redundant because reload would throw away
        // the old contents anyway. Maybe there is an argument that removing it
        // immediately means there is possibility to show it missing while the
        // possibly slow reload is operating if a repaint could occur
        ListView::remove(OUString::number(maSelectedItem->mnId));
        reload();
    }
    else if (rIdent == "default")
    {
        maDefaultTemplateHdl.Call(maSelectedItem);
        ListView::refreshDefaultColumn();
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

bool TemplateDlgLocalView::IsVisible()
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
                nIndex = -1;
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
        int nIndex = ListView::get_cursor_index();
        ListView::unselect_all();
        ListView::select(nIndex);
        ListView::set_cursor(nIndex);
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
        int nIndex = ListView::get_cursor_index();
        ListView::unselect_all();
        ListView::select(nIndex);
        ListView::set_cursor(nIndex);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
