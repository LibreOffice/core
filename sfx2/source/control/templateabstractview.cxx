/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateabstractview.hxx>

#include <sfx2/templateview.hxx>

TemplateAbstractView::TemplateAbstractView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : ThumbnailView(pParent,nWinStyle,bDisableTransientChildren),
      mpItemView(new TemplateView(this))
{
}

TemplateAbstractView::TemplateAbstractView(Window *pParent, const ResId &rResId, bool bDisableTransientChildren)
    : ThumbnailView(pParent,rResId,bDisableTransientChildren),
      mpItemView(new TemplateView(this))
{
}

TemplateAbstractView::~TemplateAbstractView ()
{
    delete mpItemView;
}

sal_uInt16 TemplateAbstractView::getOverlayRegionId() const
{
    return mpItemView->getId();
}

bool TemplateAbstractView::isOverlayVisible () const
{
    return mpItemView->IsVisible();
}

void TemplateAbstractView::sortOverlayItems(const boost::function<bool (const ThumbnailViewItem*,
                                                                        const ThumbnailViewItem*) > &func)
{
    mpItemView->sortItems(func);
}

void TemplateAbstractView::setOverlayDblClickHdl(const Link &rLink)
{
    mpItemView->setDblClickHdl(rLink);
}

void TemplateAbstractView::setOverlayCloseHdl(const Link &rLink)
{
    mpItemView->setCloseHdl(rLink);
}

void TemplateAbstractView::OnSelectionMode (bool bMode)
{
    if (mpItemView->IsVisible())
    {
        mbSelectionMode = bMode;
        mpItemView->setSelectionMode(bMode);
    }
    else
        ThumbnailView::OnSelectionMode(bMode);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
