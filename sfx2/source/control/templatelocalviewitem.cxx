/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatelocalviewitem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
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

TemplateLocalViewItem::TemplateLocalViewItem (ThumbnailView &rView, Window *pParent)
    : ThumbnailViewItem(rView,pParent)
{
}

TemplateLocalViewItem::~TemplateLocalViewItem ()
{
}

void TemplateLocalViewItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                    const ThumbnailItemAttributes *pAttrs)
{
    int nCount = 0;
    int nSeqSize = 7;

    if (!maPreview2.IsEmpty())
        nSeqSize += 5;

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

        aSeq[nCount++] = Primitive2DReference( new FillBitmapPrimitive2D(
                                            createScaleTranslateB2DHomMatrix(fScaleX,fScaleY,aPos.X(),aPos.Y()),
                                            FillBitmapAttribute(maPreview2,
                                                                B2DPoint(35,20),
                                                                B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                                false)
                                            ));

        // draw thumbnail borders
        float fWidth = aImageSize.Width()*fScaleX;
        float fHeight = aImageSize.Height()*fScaleY;
        float fPosX = aPos.getX()+35*fScaleX;
        float fPosY = aPos.getY()+20*fScaleY;

        aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX,fPosY),
                                                               B2DPoint(fPosX+fWidth,fPosY)));
        aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX+fWidth,fPosY),
                                                               B2DPoint(fPosX+fWidth,fPosY+fHeight)));
        aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX+fWidth,fPosY+fHeight),
                                                               B2DPoint(fPosX,fPosY+fHeight)));
        aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX,fPosY+fHeight),
                                                               B2DPoint(fPosX,fPosY)));
    }

    aSeq[nCount++] = Primitive2DReference( new FillBitmapPrimitive2D(
                                        createScaleTranslateB2DHomMatrix(fScaleX,fScaleY,aPos.X(),aPos.Y()),
                                        FillBitmapAttribute(maPreview1,
                                                            B2DPoint(0,0),
                                                            B2DVector(aImageSize.Width(),aImageSize.Height()),
                                                            false)
                                        ));

    // draw thumbnail borders
    float fWidth = aImageSize.Width()*fScaleX;
    float fHeight = aImageSize.Height()*fScaleY;
    float fPosX = aPos.getX();
    float fPosY = aPos.getY();

    aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX,fPosY),
                                                           B2DPoint(fPosX+fWidth,fPosY)));
    aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX+fWidth,fPosY),
                                                           B2DPoint(fPosX+fWidth,fPosY+fHeight)));
    aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX+fWidth,fPosY+fHeight),
                                                           B2DPoint(fPosX,fPosY+fHeight)));
    aSeq[nCount++] = Primitive2DReference(createBorderLine(B2DPoint(fPosX,fPosY+fHeight),
                                                           B2DPoint(fPosX,fPosY)));

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

    if (mbMode || mbHover || mbSelected)
        mpSelectBox->Paint(maDrawArea);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


