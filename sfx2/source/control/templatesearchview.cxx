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

#include <vcl/builderfactory.hxx>

TemplateSearchView::TemplateSearchView (vcl::Window *pParent, WinBits nWinStyle)
    : ThumbnailView(pParent,nWinStyle)
{
}

VCL_BUILDER_FACTORY(TemplateSearchView)

void TemplateSearchView::setOpenTemplateHdl(const Link<> &rLink)
{
    maOpenTemplateHdl = rLink;
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
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    ThumbnailView::AppendItem(pItem);

    CalculateItemPositions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


