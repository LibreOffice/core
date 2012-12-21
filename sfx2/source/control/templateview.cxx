/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "templatedlg.hxx"
#include <sfx2/templateview.hxx>
#include <sfx2/templateabstractview.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templateviewitem.hxx>
#include <vcl/edit.hxx>

#include "templateview.hrc"

#define EDIT_HEIGHT 30

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateView::TemplateView (Window *pParent)
    : ThumbnailView(pParent,WB_VSCROLL | WB_TABSTOP),
      mpMasterView(NULL),
      maAllButton(this, SfxResId(BTN_ALL_TEMPLATES)),
      maFTName(this, SfxResId(FT_NAME)),
      mnId(0)
{
    mnHeaderHeight = maAllButton.GetSizePixel().getHeight() + maAllButton.GetPosPixel().Y() * 2;
    maAllButton.SetStyle(maAllButton.GetStyle() | WB_FLATBUTTON);
}

TemplateView::~TemplateView ()
{
}

void TemplateView::setName (const OUString &rName)
{
    maName = rName;
    maFTName.SetText(maName);
}

void TemplateView::InsertItems (const std::vector<TemplateItemProperties> &rTemplates)
{
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        TemplateViewItem *pItem = new TemplateViewItem(*this);
        const TemplateItemProperties *pCur = &rTemplates[i];

        pItem->mnId = pCur->nId;
        pItem->mnDocId = pCur->nDocId;
        pItem->mnRegionId = pCur->nRegionId;
        pItem->maTitle = pCur->aName;
        pItem->setPath(pCur->aPath);
        pItem->maPreview1 = pCur->aThumbnail;
        if ( pCur->aThumbnail.IsEmpty() )
        {
            // Use the default thumbnail if we have nothing else
            pItem->maPreview1 = SfxTemplateManagerDlg::getDefaultThumbnail( pItem->getPath() );
        }
        pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

        mItemList.push_back(pItem);
    }

    CalculateItemPositions();

    Invalidate();
}

void TemplateView::Resize()
{
    Size aWinSize = GetOutputSize();

    // Set the buttons panel and buttons size
    Size aNameSize = maFTName.GetSizePixel();
    aNameSize.setWidth( aWinSize.getWidth() - maFTName.GetPosPixel().X());
    maFTName.SetSizePixel(aNameSize);

    ThumbnailView::Resize();
}

bool TemplateView::renameItem(ThumbnailViewItem* pItem, rtl::OUString sNewTitle)
{
    if (mpMasterView)
        return mpMasterView->renameItem(pItem, sNewTitle);
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


