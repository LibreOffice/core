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
#include <sfx2/sfxresid.hxx>
#include <comphelper/string.hxx>
#include <sfx2/inputdlg.hxx>

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
    if (rMEvt.IsRight())
    {
        size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
        Point aPosition (rMEvt.GetPosPixel());
        maPosition = aPosition;
        ThumbnailViewItem* pItem = ImplGetItem(nPos);
        const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

        if(pViewItem)
        {
            maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
            maRightClickHdl.Call(pItem);
        }
    }

    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateSearchView::createContextMenu()
{
    std::unique_ptr<PopupMenu> pItemMenu(new PopupMenu);
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN).toString());
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE).toString());
    pItemMenu->InsertItem(MNI_DEFAULT_TEMPLATE,SfxResId(STR_DEFAULT_TEMPLATE).toString());
    pItemMenu->InsertSeparator();
    pItemMenu->InsertItem(MNI_DELETE,SfxResId(STR_DELETE).toString());
    maSelectedItem->setSelection(true);
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
        maDeleteTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_DEFAULT_TEMPLATE:
        maDefaultTemplateHdl.Call(maSelectedItem);
        break;
    default:
        break;
    }

    return false;
}

void TemplateSearchView::setRightClickHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maRightClickHdl = rLink;
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
    pItem->maPreview1 = rImage;
    pItem->maTitle = rTitle;
    pItem->setSubTitle(rSubtitle);
    pItem->setPath(rPath);

    ThumbnailView::AppendItem(pItem);

    CalculateItemPositions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


