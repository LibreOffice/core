/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "templatesearchview.hxx"
#include "templatesearchviewitem.hxx"
#include <sfx2/templateabstractview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>

#include "../doc/doc.hrc"

#include <vcl/builderfactory.hxx>

#define MNI_OPEN               1
#define MNI_EDIT               2
#define MNI_DEFAULT_TEMPLATE   3
#define MNI_DELETE             4

TemplateSearchView::TemplateSearchView (vcl::Window *pParent, WinBits nWinStyle)
    : ThumbnailView(pParent,nWinStyle),
    maSelectedItem(nullptr),
    maPosition(0,0)
{
}

VCL_BUILDER_FACTORY(TemplateSearchView)

void TemplateSearchView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateSearchView::KeyInput( const KeyEvent& rKEvt )
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
        return;
    }
    else if( aKeyCode == KEY_DELETE && !mFilteredItemList.empty())
    {
        ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE), VclMessageType::VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

        if ( aQueryDlg->Execute() != RET_YES )
            return;

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

    ThumbnailView::KeyInput(rKEvt);
}

void TemplateSearchView::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if(rCEvt.IsMouseEvent())
        {
            deselectItems();
            size_t nPos = ImplGetItem(rCEvt.GetMousePosPixel());
            Point aPosition (rCEvt.GetMousePosPixel());
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
                    Rectangle aRect = pItem->getDrawArea();
                    maPosition = aRect.Center();
                    maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                    maCreateContextMenuHdl.Call(pItem);
                    break;
                }
            }
        }
    }

    ThumbnailView::Command(rCEvt);
}

void TemplateSearchView::createContextMenu( const bool bIsDefault)
{
    std::unique_ptr<PopupMenu> pItemMenu(new PopupMenu);
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN).toString());
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE).toString());

    if(!bIsDefault)
        pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_DEFAULT_TEMPLATE).toString());
    else
        pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_RESET_DEFAULT).toString());

    pItemMenu->InsertSeparator();
    pItemMenu->InsertItem(MNI_DELETE,SfxResId(STR_DELETE).toString());
    maSelectedItem->setSelection(true);
    maItemStateHdl.Call(maSelectedItem);
    pItemMenu->SetSelectHdl(LINK(this, TemplateSearchView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK_TYPED(TemplateSearchView, ContextMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_OPEN:
        maOpenTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_EDIT:
        maEditTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_DELETE:
    {
        ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE), VclMessageType::VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
        if ( aQueryDlg->Execute() != RET_YES )
            break;

        maDeleteTemplateHdl.Call(maSelectedItem);
        RemoveItem(maSelectedItem->mnId);

        CalculateItemPositions();
    }
        break;
    case MNI_DEFAULT_TEMPLATE:
        maDefaultTemplateHdl.Call(maSelectedItem);
        break;
    default:
        break;
    }

    return false;
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
    TemplateSearchViewItem *pItem = new TemplateSearchViewItem(*this, getNextItemId());
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

    if(TemplateAbstractView::IsDefaultTemplate(rPath))
        pItem->showDefaultIcon(true);

    ThumbnailView::AppendItem(pItem);

    CalculateItemPositions();
}

BitmapEx TemplateSearchView::getDefaultThumbnail( const OUString& rPath )
{
    BitmapEx aImg;
    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();

    if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::WRITER, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_TEXT ) );
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::CALC, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_SHEET ) );
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::IMPRESS, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_PRESENTATION ) );
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::DRAW, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_DRAWING ) );

    return aImg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


