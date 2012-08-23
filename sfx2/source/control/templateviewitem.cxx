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
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
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

void TemplateViewItem::calculateItemsPosition(const long nThumbnailHeight, const long nDisplayHeight,
                                              const long nPadding, sal_uInt32 nMaxTextLenght,
                                              const ThumbnailItemAttributes *pAttrs)
{
    ThumbnailViewItem::calculateItemsPosition(nThumbnailHeight,nDisplayHeight,nPadding,nMaxTextLenght, pAttrs);

    if (!maSubTitle.isEmpty())
    {
        Size aRectSize = maDrawArea.GetSize();

        drawinglayer::primitive2d::TextLayouterDevice aTextDev;
        aTextDev.setFontAttribute(pAttrs->aFontAttr,
                                  pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                                  com::sun::star::lang::Locale() );

        long nSpace = (nDisplayHeight + nPadding - 2*aTextDev.getTextHeight()) / 3;

        // Set title position
        maTextPos.setY(maDrawArea.getY() + nThumbnailHeight + nPadding + nSpace + aTextDev.getTextHeight());

        // Set subtitle position
        maSubTitlePos.setY(maTextPos.getY() + nSpace + aTextDev.getTextHeight());
        maSubTitlePos.setX(maDrawArea.Left() +
                           (aRectSize.Width() - aTextDev.getTextWidth(maSubTitle,0,nMaxTextLenght)*SUBTITLE_SCALE_FACTOR)/2);
    }
}

void TemplateViewItem::Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                   const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;

    int nCount = maSubTitle.isEmpty() ? 7 : 8;
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

    // Create the text primitive
    basegfx::B2DHomMatrix aTitleMatrix( createScaleTranslateB2DHomMatrix(
                pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                double( maTextPos.X() ), double( maTextPos.Y() ) ) );

    aSeq[6] = Primitive2DReference(
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

        aSeq[7] = Primitive2DReference(
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


