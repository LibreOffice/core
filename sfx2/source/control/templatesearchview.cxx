/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <templatesearchview.hxx>
#include <templatesearchviewitem.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

#include <sfx2/strings.hrc>
#include <bitmaps.hlst>

#define MNI_OPEN               "open"
#define MNI_EDIT               "edit"
#define MNI_DEFAULT_TEMPLATE   "default"
#define MNI_DELETE             "delete"

TemplateSearchView::TemplateSearchView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                                       std::unique_ptr<weld::Menu> xMenu,
                                       std::unique_ptr<weld::TreeView> xTreeView)
    : ThumbnailView(std::move(xWindow), std::move(xMenu))
    , ListView(std::move(xTreeView))
    , maSelectedItem(nullptr)
    , maPosition(0,0)
    , mViewMode(TemplateViewMode::eThumbnailView)
{
    mxTreeView->connect_row_activated(LINK(this, TemplateSearchView, RowActivatedHdl));
    mxTreeView->connect_column_clicked(LINK(this, ListView, ColumnClickedHdl));
    mxTreeView->connect_changed(LINK(this, TemplateSearchView, ListViewChangedHdl));
    mxTreeView->connect_popup_menu(LINK(this, TemplateSearchView, PopupMenuHdl));
    mxTreeView->connect_key_press(LINK(this, TemplateSearchView, KeyPressHdl));
}

bool TemplateSearchView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    return ThumbnailView::MouseButtonDown(rMEvt);
}

bool TemplateSearchView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if(aKeyCode == ( KEY_MOD1 | KEY_A ) )
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
    else if( aKeyCode == KEY_DELETE && !mFilteredItemList.empty())
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return true;

        //copy to avoid changing filtered item list during deletion
        ThumbnailValueItemList mFilteredItemListCopy = mFilteredItemList;

        for (ThumbnailViewItem* pItem : mFilteredItemListCopy)
        {
            if (pItem->isSelected())
            {
                maDeleteTemplateHdl.Call(pItem);
                ListView::remove(OUString::number(pItem->mnId));
                ThumbnailView::RemoveItem(pItem->mnId);
                CalculateItemPositions();
            }
        }
    }

    return ThumbnailView::KeyInput(rKEvt);
}

bool TemplateSearchView::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return CustomWidgetController::Command(rCEvt);

    if (rCEvt.IsMouseEvent())
    {
        deselectItems();
        size_t nPos = ImplGetItem(rCEvt.GetMousePosPixel());
        Point aPosition(rCEvt.GetMousePosPixel());
        maPosition = aPosition;
        ThumbnailViewItem* pItem = ImplGetItem(nPos);
        const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

        if(pViewItem)
        {
            maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
            maCreateContextMenuHdl.Call(pItem);
        }
    }
    else
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            //create context menu for the first selected item
            if (pItem->isSelected())
            {
                deselectItems();
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
                tools::Rectangle aRect = pItem->getDrawArea();
                maPosition = aRect.Center();
                maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                maCreateContextMenuHdl.Call(pItem);
                break;
            }
        }
    }
    return true;
}

void TemplateSearchView::createContextMenu(const bool bIsDefault, const bool bIsBuiltIn)
{
    mxContextMenu->clear();
    mxContextMenu->append(MNI_OPEN,SfxResId(STR_OPEN));
    mxContextMenu->append(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE));

    if (!bIsDefault)
        mxContextMenu->append(MNI_DEFAULT_TEMPLATE,SfxResId(STR_DEFAULT_TEMPLATE));
    else
        mxContextMenu->append(MNI_DEFAULT_TEMPLATE,SfxResId(STR_RESET_DEFAULT));

    mxContextMenu->append_separator("separator");
    mxContextMenu->append(MNI_DELETE,SfxResId(STR_DELETE));
    if(bIsBuiltIn)
    {
        mxContextMenu->set_sensitive(MNI_EDIT, false);
        mxContextMenu->set_sensitive(MNI_DELETE, false);
    }
    if(mViewMode == TemplateViewMode::eThumbnailView)
    {
        maSelectedItem->setSelection(true);
        maItemStateHdl.Call(maSelectedItem);
        ContextMenuSelectHdl(mxContextMenu->popup_at_rect(GetDrawingArea(), tools::Rectangle(maPosition, Size(1,1))));
        Invalidate();
    }
    else if(mViewMode == TemplateViewMode::eListView)
        ContextMenuSelectHdl(mxContextMenu->popup_at_rect(mxTreeView.get(), tools::Rectangle(maPosition, Size(1,1))));
}

void TemplateSearchView::ContextMenuSelectHdl(std::string_view rIdent)
{
    if (rIdent == MNI_OPEN)
        maOpenTemplateHdl.Call(maSelectedItem);
    else if (rIdent == MNI_EDIT)
        maEditTemplateHdl.Call(maSelectedItem);
    else if (rIdent ==  MNI_DELETE)
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return;

        maDeleteTemplateHdl.Call(maSelectedItem);
        ListView::remove(OUString::number(maSelectedItem->mnId));
        ThumbnailView::RemoveItem(maSelectedItem->mnId);

        CalculateItemPositions();
    }
    else if (rIdent == MNI_DEFAULT_TEMPLATE)
    {
        maDefaultTemplateHdl.Call(maSelectedItem);
        ListView::refreshDefaultColumn();
    }
}

void TemplateSearchView::setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maCreateContextMenuHdl = rLink;
}

void TemplateSearchView::setOpenTemplateHdl(const Link<ThumbnailViewItem*, void> &rLink)
{
    maOpenTemplateHdl = rLink;
}

void TemplateSearchView::setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maEditTemplateHdl = rLink;
}

void TemplateSearchView::setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDeleteTemplateHdl = rLink;
}

void TemplateSearchView::setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDefaultTemplateHdl = rLink;
}

void TemplateSearchView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    maOpenTemplateHdl.Call(pItem);
}

void TemplateSearchView::AppendItem(sal_uInt16 nAssocItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                                    const OUString &rTitle, const OUString &rSubtitle,
                                    const OUString &rPath,
                                    const BitmapEx &rImage)
{
    std::unique_ptr<TemplateSearchViewItem> pItem(new TemplateSearchViewItem(*this, getNextItemId()));
    pItem->mnAssocId = nAssocItemId;
    pItem->mnDocId = nIdx;
    pItem->mnRegionId = nRegionId;

    if(!rImage.IsEmpty())
        pItem->maPreview1 = rImage;
    else
        pItem->maPreview1 = getDefaultThumbnail(rPath);

    pItem->maTitle = rTitle;
    pItem->setHelpText(rSubtitle);
    pItem->setPath(rPath);

    if (TemplateLocalView::IsDefaultTemplate(rPath))
        pItem->showDefaultIcon(true);

    bool isDefault = pItem->IsDefaultTemplate();
    OUString sId = OUString::number(pItem->mnId);
    ListView::AppendItem(sId, rTitle, rSubtitle, rPath, isDefault);
    ThumbnailView::AppendItem(std::move(pItem));

    CalculateItemPositions();
}

void TemplateSearchView::Clear()
{
    ThumbnailView::Clear();
    ListView::clearListView();
}

void TemplateSearchView::setTemplateViewMode ( TemplateViewMode eMode )
{
    mViewMode = eMode;
}

void TemplateSearchView::Show()
{
    if ( mViewMode == TemplateViewMode::eListView)
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

void TemplateSearchView::Hide()
{
    ThumbnailView::Hide();
    ListView::HideListView();
}

bool TemplateSearchView::IsVisible()
{
    return ThumbnailView::IsVisible() || ListView::IsListViewVisible();
}

void TemplateSearchView::syncCursor()
{
    if ( mViewMode == TemplateViewMode::eListView)
    {
        ListView::unselect_all();
        int nIndex = -1;

        for(auto it = mFilteredItemList.cbegin(); it != mFilteredItemList.cend() ; ++it )
        {
            if((*it)->mbSelected)
            {
                nIndex = -1;
                nIndex = ListView::get_index((*it)->mnId);
                if(nIndex >= 0)
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
        if(aSelRows.empty())
            return;
        sal_uInt16 nCursorId = ListView::get_cursor_nId();
        ThumbnailView::SelectItem(nCursorId);
        MakeItemVisible(nCursorId);

        for(auto it = mFilteredItemList.begin(); it != mFilteredItemList.end() ; ++it )
        {
            if((*it)->mnId == nCursorId)
            {
                mpStartSelRange = it;
                break;
            }
        }

        size_t nPos =  GetItemPos(nCursorId);
        ThumbnailViewItem* pItem = ImplGetItem(nPos);
        const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);
        if(pViewItem)
            maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
    }
}

void TemplateSearchView::updateSelection()
{
    ThumbnailView::deselectItems();
    for(auto nIndex : ListView::get_selected_rows())
    {
        ThumbnailView::SelectItem(ListView::get_nId(nIndex) );
    }

    sal_uInt16  nCursorId =  ListView::get_cursor_nId();
    size_t nPos =  GetItemPos(nCursorId);
    ThumbnailViewItem* pItem = ImplGetItem(nPos);
    const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);
    if(pViewItem)
        maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
    return;
}

IMPL_LINK_NOARG(TemplateSearchView, RowActivatedHdl, weld::TreeView&, bool)
{
    maOpenTemplateHdl.Call(maSelectedItem);
    return true;
}

IMPL_LINK(TemplateSearchView, PopupMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    if (rCEvt.IsMouseEvent())
    {
        if(ListView::get_selected_rows().empty())
            return true;
        int nIndex = ListView::get_cursor_index();
        ListView::unselect_all();
        ListView::select(nIndex);
        ListView::set_cursor(nIndex);
        Point aPosition (rCEvt.GetMousePosPixel());
        maPosition = aPosition;
        updateSelection();
        if(maSelectedItem)
            maCreateContextMenuHdl.Call(maSelectedItem);
        return true;
    }
    else
    {
        if(ListView::get_selected_rows().empty())
            return true;
        int nIndex = ListView::get_cursor_index();
        ListView::unselect_all();
        ListView::select(nIndex) ;
        ListView::set_cursor(nIndex) ;
        maPosition = Point(0,0);
        updateSelection();
        if(maSelectedItem)
            maCreateContextMenuHdl.Call(maSelectedItem);
        return true;
    }
}

IMPL_LINK_NOARG(TemplateSearchView, ListViewChangedHdl, weld::TreeView&, void)
{
    updateSelection();
}

BitmapEx TemplateSearchView::getDefaultThumbnail( const OUString& rPath )
{
    BitmapEx aImg;
    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();

    if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::WRITER, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_TEXT);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::CALC, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_SHEET);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::IMPRESS, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_PRESENTATION);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::DRAW, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_DRAWING);

    return aImg;
}

void TemplateSearchView::RemoveDefaultTemplateIcon(std::u16string_view rPath)
{
    for (const std::unique_ptr<ThumbnailViewItem>& pItem : mItemList)
    {
        TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem.get());
        if (pViewItem && pViewItem->getPath().match(rPath))
        {
            pViewItem->showDefaultIcon(false);
            Invalidate();
            return;
        }
    }
}

IMPL_LINK(TemplateSearchView, KeyPressHdl, const KeyEvent&, rKEvt, bool)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if( aKeyCode == KEY_DELETE && !mFilteredItemList.empty() && !ListView::get_selected_rows().empty())
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(mxTreeView.get(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return true;

        //copy to avoid changing filtered item list during deletion
        ThumbnailValueItemList mFilteredItemListCopy = mFilteredItemList;

        for (ThumbnailViewItem* pItem : mFilteredItemListCopy)
        {
            if (pItem->isSelected())
            {
                maDeleteTemplateHdl.Call(pItem);
                ListView::remove(OUString::number(pItem->mnId));
                ThumbnailView::RemoveItem(pItem->mnId);

                CalculateItemPositions();
            }
        }
    }

    return false;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
