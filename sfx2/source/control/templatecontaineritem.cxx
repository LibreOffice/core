/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatecontaineritem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <sfx2/templateviewitem.hxx>
#include <vcl/button.hxx>

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

TemplateContainerItem::TemplateContainerItem (ThumbnailView &rView)
    : ThumbnailViewItem(rView)
{
}

TemplateContainerItem::~TemplateContainerItem ()
{
}

void TemplateContainerItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                    const ThumbnailItemAttributes *pAttrs)
{
    int nCount = 0;
    int nSeqSize = 2;

    if (!maPreview1.IsEmpty())
        nSeqSize += 3;

    if (!maPreview2.IsEmpty())
        nSeqSize += 3;

    BColor aFillColor = pAttrs->aFillColor;
    Primitive2DSequence aSeq(nSeqSize);

    // Draw background
    if ( mbSelected || mbHover )
        aFillColor = pAttrs->aHighlightColor;

    aSeq[nCount++] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                               B2DPolyPolygon(Polygon(maDrawArea,5,5).getB2DPolygon()),
                                               aFillColor));

    // Draw thumbnail
    Point aPos = maPrev1Pos;
    Size aImageSize = maPreview1.GetSizePixel();

    float fScaleX = 1.0f;
    float fScaleY = 1.0f;

    if (!maPreview2.IsEmpty())
    {
        fScaleX = 0.8f;
        fScaleY = 0.8f;

        float fWidth = aImageSize.Width()*fScaleX;
        float fHeight = aImageSize.Height()*fScaleY;
        float fPosX = aPos.getX()+35*fScaleX;
        float fPosY = aPos.getY()+20*fScaleY;

        B2DPolygon aBounds;
        aBounds.append(B2DPoint(fPosX,fPosY));
        aBounds.append(B2DPoint(fPosX+fWidth,fPosY));
        aBounds.append(B2DPoint(fPosX+fWidth,fPosY+fHeight));
        aBounds.append(B2DPoint(fPosX,fPosY+fHeight));
        aBounds.setClosed(true);

        aSeq[nCount++] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                            B2DPolyPolygon(aBounds), Color(COL_WHITE).getBColor()));
        aSeq[nCount++] = Primitive2DReference( new FillBitmapPrimitive2D(
                                            createScaleTranslateB2DHomMatrix(fScaleX,fScaleY,aPos.X(),aPos.Y()),
                                            FillBitmapAttribute(maPreview2,
                                                                B2DPoint(35,20),
                                                                B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                                false)
                                            ));

        // draw thumbnail borders
        aSeq[nCount++] = Primitive2DReference(createBorderLine(aBounds));
    }

    if (!maPreview1.IsEmpty())
    {
        // draw thumbnail borders
        float fWidth = aImageSize.Width()*fScaleX;
        float fHeight = aImageSize.Height()*fScaleY;
        float fPosX = aPos.getX();
        float fPosY = aPos.getY();

        B2DPolygon aBounds;
        aBounds.append(B2DPoint(fPosX,fPosY));
        aBounds.append(B2DPoint(fPosX+fWidth,fPosY));
        aBounds.append(B2DPoint(fPosX+fWidth,fPosY+fHeight));
        aBounds.append(B2DPoint(fPosX,fPosY+fHeight));
        aBounds.setClosed(true);

        aSeq[nCount++] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                            B2DPolyPolygon(aBounds), Color(COL_WHITE).getBColor()));
        aSeq[nCount++] = Primitive2DReference( new FillBitmapPrimitive2D(
                                            createScaleTranslateB2DHomMatrix(fScaleX,fScaleY,aPos.X(),aPos.Y()),
                                            FillBitmapAttribute(maPreview1,
                                                                B2DPoint(0,0),
                                                                B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                                false)
                                            ));

        aSeq[nCount++] = Primitive2DReference(createBorderLine(aBounds));
    }

    // Draw centered text below thumbnail
    aPos = maTextPos;

    // Create the text primitive
    basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                double( aPos.X() ), double( aPos.Y() ) ) );

    aSeq[nCount++] = Primitive2DReference(
                new TextSimplePortionPrimitive2D(aTextMatrix,
                                                 maTitle,0,maTitle.getLength(),
                                                 std::vector< double >( ),
                                                 pAttrs->aFontAttr,
                                                 com::sun::star::lang::Locale(),
                                                 Color(COL_BLACK).getBColor() ) );

    pProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


