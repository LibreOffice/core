/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateviewitem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <vcl/button.hxx>
#include <vcl/graph.hxx>

#define SUBTITLE_SCALE_FACTOR 0.85

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateViewItem::TemplateViewItem (ThumbnailView &rView, sal_uInt16 nId)
    : ThumbnailViewItem(rView, nId),
      mnRegionId(USHRT_MAX),
      mnDocId(USHRT_MAX)
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

    int nCount = maSubTitle.isEmpty() ? 5 : 6;
    drawinglayer::primitive2d::Primitive2DSequence aSeq(nCount);

    // Draw background
    if ( mbSelected || mbHover )
        aFillColor = pAttrs->aHighlightColor;

    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                               B2DPolyPolygon(Polygon(maDrawArea,5,5).getB2DPolygon()),
                                               aFillColor));

    // Draw thumbnail
    Size aImageSize = maPreview1.GetSizePixel();

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

    aSeq[1] = drawinglayer::primitive2d::Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                        B2DPolyPolygon(aBounds), Color(COL_WHITE).getBColor()));

    aSeq[2] = drawinglayer::primitive2d::Primitive2DReference( new FillGraphicPrimitive2D(
                                        createTranslateB2DHomMatrix(maPrev1Pos.X(),maPrev1Pos.Y()),
                                        FillGraphicAttribute(Graphic(maPreview1),
                                                            B2DRange(
                                                                B2DPoint(0,0),
                                                                B2DPoint(aImageSize.Width(),aImageSize.Height())),
                                                            false)
                                        ));

    // draw thumbnail borders
    aSeq[3] = drawinglayer::primitive2d::Primitive2DReference(createBorderLine(aBounds));

    addTextPrimitives(maTitle, pAttrs, maTextPos, aSeq);

    if (!maSubTitle.isEmpty())
    {
        addTextPrimitives(maSubTitle, pAttrs, maSubTitlePos, aSeq);
    }

    pProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


