/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateview.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templateviewitem.hxx>

#include "templateview.hrc"

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateView::TemplateView (Window *pParent, SfxDocumentTemplates *pTemplates)
    : ThumbnailView(pParent),
      maCloseImg(SfxResId(IMG_TEMPLATE_VIEW_CLOSE)),
      mnRegionId(0),
      mpDocTemplates(pTemplates)
{
    mnHeaderHeight = 30;
}

TemplateView::~TemplateView ()
{
}

void TemplateView::setRegionId (const sal_uInt16 nRegionId)
{
    mnRegionId = nRegionId;
    maFolderName = mpDocTemplates->GetRegionName(nRegionId);
}

void TemplateView::Paint (const Rectangle &rRect)
{
    ThumbnailView::Paint(rRect);

    Primitive2DSequence aSeq(2);
    TextLayouterDevice aTextDev;

    // Draw centered region name
    Point aPos;
    Size aWinSize = GetOutputSizePixel();

    aPos.X() = (aWinSize.getWidth() - aTextDev.getTextWidth(maFolderName,0,maFolderName.getLength()))/2;
    aPos.Y() = aTextDev.getTextHeight() + (mnHeaderHeight - aTextDev.getTextHeight())/2;

    basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                mpItemAttrs->aFontSize.getX(), mpItemAttrs->aFontSize.getY(),
                double( aPos.X() ), double( aPos.Y() ) ) );

    aSeq[0] = Primitive2DReference(
                new TextSimplePortionPrimitive2D(aTextMatrix,
                                                 maFolderName,0,maFolderName.getLength(),
                                                 std::vector< double >( ),
                                                 mpItemAttrs->aFontAttr,
                                                 com::sun::star::lang::Locale(),
                                                 Color(COL_BLACK).getBColor() ) );

    // Draw close icon
    Size aImageSize = maCloseImg.GetSizePixel();

    aPos.Y() = (mnHeaderHeight - aImageSize.Height())/2;
    aPos.X() = aWinSize.Width() - aImageSize.Width() - aPos.Y();

    aSeq[1] = Primitive2DReference( new FillBitmapPrimitive2D(
                                        createTranslateB2DHomMatrix(aPos.X(),aPos.Y()),
                                        FillBitmapAttribute(maCloseImg.GetBitmapEx(),
                                                            B2DPoint(0,0),
                                                            B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                            false)
                                        ));

    mpProcessor->process(aSeq);
}

void TemplateView::InsertItem (const TemplateViewItem *pItem)
{
    TemplateViewItem *pCloneItem = new TemplateViewItem(*this,this);

    pCloneItem->mnId = pItem->mnId;
    pCloneItem->maText = pItem->maText;
    pCloneItem->setPath(pItem->getPath());
    pCloneItem->setFileType(pItem->getFileType());
    pCloneItem->maPreview1 = pItem->maPreview1;

    mItemList.push_back(pCloneItem);

    CalculateItemPositions();

    Invalidate();
}

void TemplateView::InsertItems (const std::vector<TemplateViewItem*> &rTemplates)
{
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        TemplateViewItem *pItem = new TemplateViewItem(*this,this);
        TemplateViewItem *pCur = rTemplates[i];

        pItem->mnId = pCur->mnId;
        pItem->maText = pCur->maText;
        pItem->setPath(pCur->getPath());
        pItem->setFileType(pCur->getFileType());
        pItem->maPreview1 = pCur->maPreview1;
        pItem->setSelectClickHdl(LINK(this,ThumbnailView,OnFolderSelected));

        mItemList.push_back(pItem);
    }

    CalculateItemPositions();

    Invalidate();
}

void TemplateView::MouseButtonDown (const MouseEvent &rMEvt)
{
    if (rMEvt.IsLeft())
    {
        Size aWinSize = GetOutputSizePixel();
        Size aImageSize = maCloseImg.GetSizePixel();

        Point aPos;
        aPos.Y() = (mnHeaderHeight - aImageSize.Height())/2;
        aPos.X() = aWinSize.Width() - aImageSize.Width() - aPos.Y();

        Rectangle aImgRect(aPos,aImageSize);

        if (aImgRect.IsInside(rMEvt.GetPosPixel()))
        {
            Show(false);
            Clear();
        }
    }

    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateView::OnItemDblClicked(ThumbnailViewItem *pItem)
{
    maDblClickHdl.Call(pItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


