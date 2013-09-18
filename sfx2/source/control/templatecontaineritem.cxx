/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateabstractview.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <sfx2/templateviewitem.hxx>
#include <vcl/button.hxx>
#include <vcl/graph.hxx>

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateContainerItem::TemplateContainerItem (ThumbnailView &rView, sal_uInt16 nId)
    : ThumbnailViewItem(rView, nId)
{
}

TemplateContainerItem::~TemplateContainerItem ()
{
}

void TemplateContainerItem::calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                     const long nPadding, sal_uInt32 nMaxTextLenght,
                                     const ThumbnailItemAttributes *pAttrs)
{
    ThumbnailViewItem::calculateItemsPosition( nThumbnailHeight, nDisplayHeight, nPadding, nMaxTextLenght, pAttrs);
    Point aPos (maDrawArea.getX() + nPadding, maDrawArea.getY() + nPadding);
    maThumbnailArea = Rectangle(aPos, Size(maDrawArea.GetWidth() - 2 * nPadding, nThumbnailHeight));
}

void TemplateContainerItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                    const ThumbnailItemAttributes *pAttrs)
{
    int nCount = 0;
    int nSeqSize = 3;

    if (!maPreview1.IsEmpty())
        nSeqSize += 3;

    if (!maPreview2.IsEmpty())
        nSeqSize += 3;

    if (!maPreview3.IsEmpty())
        nSeqSize += 3;

    if (!maPreview4.IsEmpty())
        nSeqSize += 3;

    BColor aFillColor = pAttrs->aFillColor;
    drawinglayer::primitive2d::Primitive2DSequence aSeq(nSeqSize);

    // Draw background
    if ( mbSelected || mbHover )
        aFillColor = pAttrs->aHighlightColor;

    aSeq[nCount++] = drawinglayer::primitive2d::Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                               B2DPolyPolygon(Polygon(maDrawArea,5,5).getB2DPolygon()),
                                               aFillColor));

    // Create rounded rectangle border
    aSeq[nCount++] = drawinglayer::primitive2d::Primitive2DReference( new PolygonStrokePrimitive2D(
                                              Polygon(maThumbnailArea,5,5).getB2DPolygon(),
                                              LineAttribute(BColor(0.8, 0.8, 0.8), 2.0)));

    // Paint the thumbnails side by side on a 2x2 grid
    long nThumbPadding = 4;
    Size aThumbSize( ( maThumbnailArea.getWidth() - 3 * nThumbPadding ) / 2, ( maThumbnailArea.getHeight() - 3* nThumbPadding ) / 2 );

    // Draw thumbnail
    for (int i=0; i<4; ++i)
    {
        long nPosX = 0;
        long nPosY = 0;
        BitmapEx* pImage = NULL;

        switch (i)
        {
            case 0:
                pImage = &maPreview1;
                break;
            case 1:
                pImage = &maPreview2;
                nPosX = aThumbSize.getWidth() + nThumbPadding;
                break;
            case 2:
                pImage = &maPreview3;
                nPosY = aThumbSize.getHeight() + nThumbPadding;
                break;
            case 3:
                pImage = &maPreview4;
                nPosX = aThumbSize.getWidth() + nThumbPadding;
                nPosY = aThumbSize.getHeight() + nThumbPadding;
                break;
        }

        if (!pImage->IsEmpty())
        {
            // Check the size of the picture and resize if needed
            Size aImageSize = pImage->GetSizePixel();
            if (aImageSize.getWidth() > aThumbSize.getWidth() || aImageSize.getHeight() > aThumbSize.getHeight())
            {
                // Resize the picture and store it for next times
                *pImage = TemplateAbstractView::scaleImg( *pImage, aThumbSize.getWidth(), aThumbSize.getHeight() );
                aImageSize = pImage->GetSizePixel();
            }

            float nOffX = (aThumbSize.getWidth() - aImageSize.getWidth()) / 2;
            float nOffY = (aThumbSize.getHeight() - aImageSize.getHeight()) / 2;

            float fWidth = aImageSize.Width();
            float fHeight = aImageSize.Height();
            float fPosX = maThumbnailArea.Left() + nThumbPadding + nPosX + nOffX;
            float fPosY = maThumbnailArea.Top() + nThumbPadding + nPosY + nOffY;

            B2DPolygon aBounds;
            aBounds.append(B2DPoint(fPosX,fPosY));
            aBounds.append(B2DPoint(fPosX+fWidth,fPosY));
            aBounds.append(B2DPoint(fPosX+fWidth,fPosY+fHeight));
            aBounds.append(B2DPoint(fPosX,fPosY+fHeight));
            aBounds.setClosed(true);

            aSeq[nCount++] = drawinglayer::primitive2d::Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                                B2DPolyPolygon(aBounds), Color(COL_WHITE).getBColor()));
            aSeq[nCount++] = drawinglayer::primitive2d::Primitive2DReference( new FillGraphicPrimitive2D(
                                                createScaleTranslateB2DHomMatrix(1.0,1.0,fPosX,fPosY),
                                                FillGraphicAttribute(Graphic(*pImage),
                                                                    B2DRange(
                                                                        B2DPoint(0.0,0.0),
                                                                        B2DPoint(aImageSize.Width(),aImageSize.Height())),
                                                                    false)
                                                ));

            // draw thumbnail borders
            aSeq[nCount++] = drawinglayer::primitive2d::Primitive2DReference(createBorderLine(aBounds));
        }
    }

    addTextPrimitives(maTitle, pAttrs, maTextPos, aSeq);

    pProcessor->process(aSeq);
}

bool TemplateContainerItem::HasMissingPreview( )
{
    return maPreview1.IsEmpty() || maPreview2.IsEmpty() || maPreview3.IsEmpty() || maPreview4.IsEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


