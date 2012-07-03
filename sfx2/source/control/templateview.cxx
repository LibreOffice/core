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
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templateviewitem.hxx>
#include <vcl/edit.hxx>

#include "templateview.hrc"

#define EDIT_WIDTH 180
#define EDIT_HEIGHT 20

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateView::TemplateView (Window *pParent, SfxDocumentTemplates *pTemplates)
    : ThumbnailView(pParent,WB_VSCROLL),
      maCloseImg(SfxResId(IMG_TEMPLATE_VIEW_CLOSE)),
      mnRegionId(0),
      mpDocTemplates(pTemplates),
      mpEditName(new Edit(this, WB_BORDER | WB_HIDE))
{
    mnHeaderHeight = 30;

    mpEditName->SetSizePixel(Size(EDIT_WIDTH,EDIT_HEIGHT));
}

TemplateView::~TemplateView ()
{
}

void TemplateView::setRegionId (const sal_uInt16 nRegionId)
{
    mnRegionId = nRegionId;
    maFolderName = mpDocTemplates->GetRegionName(nRegionId);
    mpEditName->SetText(rtl::OUString());
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

void TemplateView::Resize()
{
    // Set editbox size and position
    Size aEditSize = mpEditName->GetSizePixel();
    Size aWinSize = GetOutputSize();

    Point aPos;
    aPos.X() = (aWinSize.getWidth() - aEditSize.getWidth())/2;
    aPos.Y() = (mnHeaderHeight - aEditSize.getHeight())/2;

    mpEditName->SetPosPixel(aPos);

    ThumbnailView::Resize();
}

void TemplateView::MouseButtonDown (const MouseEvent &rMEvt)
{
    if (rMEvt.IsLeft())
    {
        // Check if we are editing title
        if (mpEditName->IsVisible())
        {
            mpEditName->Show(false);

            // Update name if its not empty
            rtl::OUString aTmp = mpEditName->GetText();

            if (!aTmp.isEmpty())
            {
                PostUserEvent(LINK(this,TemplateView,ChangeNameHdl));
            }
            else
            {
                mpEditName->SetText(rtl::OUString());
                Invalidate();
            }

            return;
        }

        Size aWinSize = GetOutputSizePixel();
        Size aImageSize = maCloseImg.GetSizePixel();

        Point aPos;
        aPos.Y() = (mnHeaderHeight - aImageSize.Height())/2;
        aPos.X() = aWinSize.Width() - aImageSize.Width() - aPos.Y();

        Rectangle aImgRect(aPos,aImageSize);

        if (aImgRect.IsInside(rMEvt.GetPosPixel()))
        {
            maCloseHdl.Call(this);
        }
        else
        {
            drawinglayer::primitive2d::TextLayouterDevice aTextDev;

            float fTextWidth = aTextDev.getTextWidth(maFolderName,0,maFolderName.getLength());

            aPos.X() = (aWinSize.getWidth() - fTextWidth)/2;
            aPos.Y() = (mnHeaderHeight - aTextDev.getTextHeight())/2;

            Rectangle aTitleRect(aPos,Size(fTextWidth,aTextDev.getTextHeight()));

            if (aTitleRect.IsInside(rMEvt.GetPosPixel()))
                mpEditName->Show();
        }
    }

    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateView::OnItemDblClicked(ThumbnailViewItem *pItem)
{
    maDblClickHdl.Call(pItem);
}

IMPL_LINK_NOARG(TemplateView, ChangeNameHdl)
{
    rtl::OUString aTmp = maFolderName;
    maFolderName = mpEditName->GetText();

    if (!maChangeNameHdl.Call(this))
        maFolderName = aTmp;

    mpEditName->SetText(rtl::OUString());

    Invalidate();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


