/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateviewitem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <vcl/button.hxx>

#define SUBTITLE_SCALE_FACTOR 0.85

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateViewItem::TemplateViewItem (ThumbnailView &rView, Window *pParent)
    : ThumbnailViewItem(rView,pParent)
{
}

TemplateViewItem::~TemplateViewItem ()
{
}

void TemplateViewItem::calculateItemsPosition(sal_uInt32 nMaxTextLength)
{
    ThumbnailViewItem::calculateItemsPosition(nMaxTextLength);

    if (!maSubTitle.isEmpty())
    {
        Size aRectSize = maDrawArea.GetSize();
        Size aImageSize = maPreview1.GetSizePixel();
        aRectSize.Height() -= aImageSize.getHeight();

        drawinglayer::primitive2d::TextLayouterDevice aTextDev;

        long nSpace = (aRectSize.getHeight() - 2*aTextDev.getTextHeight()) / 3;

        // Set title position
        maTextPos.setY(maPrev1Pos.getY() + aImageSize.getHeight() + nSpace);

        // Set subtitle position
        maSubTitlePos.setY(maTextPos.getY() + nSpace);
        maSubTitlePos.setX(maDrawArea.Left() +
                           (aRectSize.Width() - aTextDev.getTextWidth(maSubTitle,0,nMaxTextLength)*SUBTITLE_SCALE_FACTOR)/2);
    }
}

void TemplateViewItem::Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                   const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;

    int nCount = maSubTitle.isEmpty() ? 3 : 4;
    Primitive2DSequence aSeq(nCount);

    // Draw background
    if ( mbSelected || mbHover )
        aFillColor = pAttrs->aHighlightColor;

    aSeq[0] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                               B2DPolyPolygon(Polygon(maDrawArea,5,5).getB2DPolygon()),
                                               aFillColor));

    // Draw thumbnail
    Size aImageSize = maPreview1.GetSizePixel();

    aSeq[1] = Primitive2DReference( new FillBitmapPrimitive2D(
                                        createTranslateB2DHomMatrix(maPrev1Pos.X(),maPrev1Pos.Y()),
                                        FillBitmapAttribute(maPreview1,
                                                            B2DPoint(0,0),
                                                            B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                            false)
                                        ));

    // Draw centered text below thumbnail

    // Create the text primitive
    basegfx::B2DHomMatrix aTitleMatrix( createScaleTranslateB2DHomMatrix(
                pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                double( maTextPos.X() ), double( maTextPos.Y() ) ) );

    aSeq[2] = Primitive2DReference(
                new TextSimplePortionPrimitive2D(aTitleMatrix,
                                                 maTitle,0,pAttrs->nMaxTextLenght,
                                                 std::vector< double >( ),
                                                 pAttrs->aFontAttr,
                                                 com::sun::star::lang::Locale(),
                                                 Color(COL_BLACK).getBColor() ) );

    if (!maSubTitle.isEmpty())
    {
        basegfx::B2DHomMatrix aSubTitleMatrix( createScaleTranslateB2DHomMatrix(
                    pAttrs->aFontSize.getX()*SUBTITLE_SCALE_FACTOR, pAttrs->aFontSize.getY()*SUBTITLE_SCALE_FACTOR,
                    double( maSubTitlePos.X() ), double( maSubTitlePos.Y() ) ) );

        aSeq[3] = Primitive2DReference(
                    new TextSimplePortionPrimitive2D(aSubTitleMatrix,
                                                     maSubTitle,0,pAttrs->nMaxTextLenght,
                                                     std::vector< double >( ),
                                                     pAttrs->aFontAttr,
                                                     com::sun::star::lang::Locale(),
                                                     Color(COL_BLACK).getBColor() ) );
    }

    pProcessor->process(aSeq);

    if (mbMode || mbHover || mbSelected)
        mpSelectBox->Paint(maDrawArea);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


