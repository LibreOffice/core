/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <templateviewitem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonSelectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <tools/poly.hxx>
#include <vcl/graph.hxx>

#include <bitmaps.hlst>

using namespace basegfx;
using namespace basegfx::utils;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateViewItem::TemplateViewItem(ThumbnailView& rView, sal_uInt16 nId)
    : ThumbnailViewItem(rView, nId),
      mnRegionId(USHRT_MAX),
      mnDocId(USHRT_MAX),
      maDefaultBitmap(BMP_DEFAULT),
      mbIsDefaultTemplate(false)
{
}

TemplateViewItem::~TemplateViewItem ()
{
}

::tools::Rectangle TemplateViewItem::getDefaultIconArea() const
{
    ::tools::Rectangle aArea(getDrawArea());
    Size aSize(maDefaultBitmap.GetSizePixel());

    return ::tools::Rectangle(
            Point(aArea.Left() + THUMBNAILVIEW_ITEM_CORNER, aArea.Top() + THUMBNAILVIEW_ITEM_CORNER),
            aSize);
}

void TemplateViewItem::Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                   const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;

    drawinglayer::primitive2d::Primitive2DContainer aSeq(5);
    double fTransparence = 0.0;

    // Draw background
    if( mbSelected && mbHover)
        aFillColor = pAttrs->aSelectHighlightColor;
    else if (mbSelected || mbHover)
    {
        aFillColor = pAttrs->aHighlightColor;
        if (mbHover)
            fTransparence = pAttrs->fHighlightTransparence;
    }

    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
            new PolyPolygonSelectionPrimitive2D( B2DPolyPolygon(::tools::Polygon(maDrawArea,5,5).getB2DPolygon()),
                                                 aFillColor,
                                                 fTransparence,
                                                 0.0,
                                                 true));

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
                                        B2DPolyPolygon(aBounds), COL_WHITE.getBColor()));

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

    if(mbIsDefaultTemplate)
    {
        Point aIconPos(getDefaultIconArea().TopLeft());

        aSeq[4] = drawinglayer::primitive2d::Primitive2DReference(new DiscreteBitmapPrimitive2D( maDefaultBitmap,
                    B2DPoint(aIconPos.X(), aIconPos.Y())));
    }

    addTextPrimitives(maTitle, pAttrs, maTextPos, aSeq);

    pProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


