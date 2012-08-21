/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sfx2/thumbnailviewitem.hxx>

#include "thumbnailviewacc.hxx"

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>

using namespace basegfx;
using namespace basegfx::tools;
using namespace ::com::sun::star;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

ThumbnailViewItem::ThumbnailViewItem(ThumbnailView &rView, Window *pParent)
    : mrParent(rView)
    , mnId(0)
    , mbVisible(true)
    , mbSelected(false)
    , mbHover(false)
    , mpxAcc(NULL)
    , mbMode(false)
    , mpSelectBox(new CheckBox(pParent,WB_HIDE | WB_NOPOINTERFOCUS))
{
    mpSelectBox->SetSizePixel(Size(20,20));
    mpSelectBox->SetClickHdl(LINK(this,ThumbnailViewItem,OnClick));
}

ThumbnailViewItem::~ThumbnailViewItem()
{
    if( mpxAcc )
    {
        static_cast< ThumbnailViewItemAcc* >( mpxAcc->get() )->ParentDestroyed();
        delete mpxAcc;
    }

    delete mpSelectBox;
}

void ThumbnailViewItem::show (bool bVisible)
{
    mbVisible = bVisible;

    if (mbMode)
        mpSelectBox->Show(bVisible);
    else if (!bVisible)
        mpSelectBox->Show(bVisible);
    else if (mbSelected)
        mpSelectBox->Show(bVisible);
}

void ThumbnailViewItem::setSelection (bool state)
{
    mbSelected = state;
    mpSelectBox->SetState(state ? STATE_CHECK : STATE_NOCHECK);

    if (!isHighlighted())
        mpSelectBox->Show(state);
}

void ThumbnailViewItem::setHighlight (bool state)
{
    mbHover = state;

    if (!isSelected())
        mpSelectBox->Show(state);
}

uno::Reference< accessibility::XAccessible > ThumbnailViewItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mpxAcc )
        mpxAcc = new uno::Reference< accessibility::XAccessible >( new ThumbnailViewItemAcc( this, bIsTransientChildrenDisabled ) );

    return *mpxAcc;
}

void ThumbnailViewItem::setDrawArea (const Rectangle &area)
{
    maDrawArea = area;
}

void ThumbnailViewItem::calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                                const long nPadding, sal_uInt32 nMaxTextLenght,
                                                const ThumbnailItemAttributes *pAttrs)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;
    aTextDev.setFontAttribute(pAttrs->aFontAttr,
                              pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                              com::sun::star::lang::Locale() );

    Size aRectSize = maDrawArea.GetSize();
    Size aImageSize = maPreview1.GetSizePixel();

    // Calculate thumbnail position
    Point aPos = maDrawArea.TopLeft();
    aPos.X() = maDrawArea.getX() + (aRectSize.Width()-aImageSize.Width())/2;
    aPos.Y() = maDrawArea.getY() + nPadding + (nThumbnailHeight-aImageSize.Height())/2;
    maPrev1Pos = aPos;

    // Calculate text position
    aPos.Y() = maDrawArea.getY() + nThumbnailHeight + nPadding;
    aPos.Y() = aPos.Y() + aTextDev.getTextHeight() + (nDisplayHeight - aTextDev.getTextHeight())/2;
    aPos.X() = maDrawArea.Left() + (aRectSize.Width() - aTextDev.getTextWidth(maTitle,0,nMaxTextLenght))/2;
    maTextPos = aPos;

    // Calculate checkbox position
    aPos.Y() -= aTextDev.getTextHeight();
    aPos.X() = maDrawArea.Left() + 15;

    mpSelectBox->SetPosPixel(aPos);
}

void ThumbnailViewItem::setSelectionMode (bool mode)
{
    mbMode = mode;

    if (!mbHover && !mbSelected && mbVisible)
        mpSelectBox->Show(mode);
}

void ThumbnailViewItem::setSelectClickHdl (const Link &link)
{
    maClickHdl = link;
}

void ThumbnailViewItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                               const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;
    Primitive2DSequence aSeq(7);

    // Draw background
    if ( mbSelected || mbHover )
        aFillColor = pAttrs->aHighlightColor;

    aSeq[0] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                               B2DPolyPolygon(Polygon(maDrawArea,5,5).getB2DPolygon()),
                                               aFillColor));

    // Draw thumbnail
    Point aPos = maPrev1Pos;
    Size aImageSize = maPreview1.GetSizePixel();

    aSeq[1] = Primitive2DReference( new FillBitmapPrimitive2D(
                                        createTranslateB2DHomMatrix(aPos.X(),aPos.Y()),
                                        FillBitmapAttribute(maPreview1,
                                                            B2DPoint(0,0),
                                                            B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                            false)
                                        ));

    // draw thumbnail borders
    float fWidth = aImageSize.Width();
    float fHeight = aImageSize.Height();
    float fPosX = maPrev1Pos.getX();
    float fPosY = maPrev1Pos.getY();

    aSeq[2] = Primitive2DReference(createBorderLine(B2DPoint(fPosX,fPosY),
                                                           B2DPoint(fPosX+fWidth,fPosY)));
    aSeq[3] = Primitive2DReference(createBorderLine(B2DPoint(fPosX+fWidth,fPosY),
                                                           B2DPoint(fPosX+fWidth,fPosY+fHeight)));
    aSeq[4] = Primitive2DReference(createBorderLine(B2DPoint(fPosX+fWidth,fPosY+fHeight),
                                                           B2DPoint(fPosX,fPosY+fHeight)));
    aSeq[5] = Primitive2DReference(createBorderLine(B2DPoint(fPosX,fPosY+fHeight),
                                                           B2DPoint(fPosX,fPosY)));

    // Draw centered text below thumbnail
    aPos = maTextPos;

    // Create the text primitive
    basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                double( aPos.X() ), double( aPos.Y() ) ) );

    aSeq[6] = Primitive2DReference(
                new TextSimplePortionPrimitive2D(aTextMatrix,
                                                 maTitle,0,pAttrs->nMaxTextLenght,
                                                 std::vector< double >( ),
                                                 pAttrs->aFontAttr,
                                                 com::sun::star::lang::Locale(),
                                                 Color(COL_BLACK).getBColor() ) );

    pProcessor->process(aSeq);

    if (mbMode || mbHover || mbSelected)
        mpSelectBox->Paint(maDrawArea);
}

drawinglayer::primitive2d::BorderLinePrimitive2D*
ThumbnailViewItem::createBorderLine (const basegfx::B2DPoint &rStart, const basegfx::B2DPoint &rEnd)
{
    return new BorderLinePrimitive2D(rStart,rEnd,0.5,0,0,0,0,0,0,
                                     BColor(),Color(COL_BLACK).getBColor(),BColor(),
                                     false,STYLE_SOLID);
}

IMPL_LINK (ThumbnailViewItem, OnClick, CheckBox*, )
{
    mbSelected = mpSelectBox->GetState() == STATE_CHECK;
    mpSelectBox->Invalidate();
    maClickHdl.Call(this);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


