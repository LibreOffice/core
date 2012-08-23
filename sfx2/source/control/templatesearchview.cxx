/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "templatesearchview.hxx"

#include "templatesearchviewitem.hxx"

TemplateSearchView::TemplateSearchView (Window *pParent, WinBits nWinStyle)
    : ThumbnailView(pParent,nWinStyle)
{
}

TemplateSearchView::~TemplateSearchView ()
{
}

void TemplateSearchView::AppendItem(sal_uInt16 nItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                                    const OUString &rTitle, const OUString &rSubtitle,
                                    const OUString &rPath,
                                    const BitmapEx &rImage)
{
    TemplateSearchViewItem *pItem = new TemplateSearchViewItem(*this,this);
    pItem->mnId = nItemId;
    pItem->mnIdx = nIdx;
    pItem->mnRegionId = nRegionId;
    pItem->maPreview1 = rImage;
    pItem->maTitle = rTitle;
    pItem->setSubTitle(rSubtitle);
    pItem->setPath(rPath);
    pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    mItemList.push_back(pItem);

    CalculateItemPositions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


