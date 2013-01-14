/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/thumbnailviewitem.hxx>

#include "thumbnailviewacc.hxx"

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
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

ThumbnailViewItem::ThumbnailViewItem(ThumbnailView &rView)
    : mrParent(rView)
    , mnId(0)
    , mbVisible(true)
    , mbSelected(false)
    , mbHover(false)
    , mpxAcc(NULL)
{
}

ThumbnailViewItem::~ThumbnailViewItem()
{
    if( mpxAcc )
    {
        static_cast< ThumbnailViewItemAcc* >( mpxAcc->get() )->ParentDestroyed();
        delete mpxAcc;
    }
}

void ThumbnailViewItem::show (bool bVisible)
{
    mbVisible = bVisible;
}

void ThumbnailViewItem::setSelection (bool state)
{
    mbSelected = state;
}

void ThumbnailViewItem::setHighlight (bool state)
{
    mbHover = state;
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

void ThumbnailViewItem::calculateItemsPosition (const long nThumbnailHeight, const long,
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
    aPos.Y() = maDrawArea.getY() + nThumbnailHeight + nPadding + aTextDev.getTextHeight();
    aPos.X() = maDrawArea.Left() + (aRectSize.Width() - aTextDev.getTextWidth(maTitle,0,nMaxTextLenght))/2;
    maTextPos = aPos;
}

void ThumbnailViewItem::setSelectClickHdl (const Link &link)
{
    maClickHdl = link;
}

void ThumbnailViewItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                               const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;
    Primitive2DSequence aSeq(4);

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

    B2DPolygon aBounds;
    aBounds.append(B2DPoint(fPosX,fPosY));
    aBounds.append(B2DPoint(fPosX+fWidth,fPosY));
    aBounds.append(B2DPoint(fPosX+fWidth,fPosY+fHeight));
    aBounds.append(B2DPoint(fPosX,fPosY+fHeight));
    aBounds.setClosed(true);

    aSeq[2] = Primitive2DReference(createBorderLine(aBounds));

    // Draw centered text below thumbnail
    aPos = maTextPos;

    // Create the text primitive
    basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                double( aPos.X() ), double( aPos.Y() ) ) );

    aSeq[3] = Primitive2DReference(
                new TextSimplePortionPrimitive2D(aTextMatrix,
                                                 maTitle,0,pAttrs->nMaxTextLenght,
                                                 std::vector< double >( ),
                                                 pAttrs->aFontAttr,
                                                 com::sun::star::lang::Locale(),
                                                 Color(COL_BLACK).getBColor() ) );

    pProcessor->process(aSeq);
}

drawinglayer::primitive2d::PolygonHairlinePrimitive2D*
ThumbnailViewItem::createBorderLine (const basegfx::B2DPolygon& rPolygon)
{
    return new PolygonHairlinePrimitive2D(rPolygon, Color(186,186,186).getBColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


