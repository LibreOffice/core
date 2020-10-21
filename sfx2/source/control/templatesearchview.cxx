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
                                       std::unique_ptr<weld::Menu> xMenu)
    : ThumbnailView(std::move(xWindow), std::move(xMenu))
    , maSelectedItem(nullptr)
    , maPosition(0,0)
{
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
                RemoveItem(pItem->mnId);

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

void TemplateSearchView::createContextMenu(const bool bIsDefault)
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
    maSelectedItem->setSelection(true);
    maItemStateHdl.Call(maSelectedItem);
    ContextMenuSelectHdl(mxContextMenu->popup_at_rect(GetDrawingArea(), tools::Rectangle(maPosition, Size(1,1))));
    Invalidate();
}

void TemplateSearchView::ContextMenuSelectHdl(const OString& rIdent)
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
        RemoveItem(maSelectedItem->mnId);

        CalculateItemPositions();
    }
    else if (rIdent == MNI_DEFAULT_TEMPLATE)
        maDefaultTemplateHdl.Call(maSelectedItem);
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

    ThumbnailView::AppendItem(std::move(pItem));

    CalculateItemPositions();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


