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
#include <sfx2/thumbnailview.hxx>
#include "thumbnailviewacc.hxx"

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonSelectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/texteng.hxx>

using namespace basegfx;
using namespace basegfx::utils;
using namespace ::com::sun::star;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

ThumbnailViewItem::ThumbnailViewItem(ThumbnailView& rView, sal_uInt16 nId)
    : mrParent(rView)
    , mnId(nId)
    , mbVisible(true)
    , mbBorder(true)
    , mbSelected(false)
    , mbHover(false)
{
}

ThumbnailViewItem::~ThumbnailViewItem()
{
    if( mxAcc.is() )
    {
        static_cast< ThumbnailViewItemAcc* >( mxAcc.get() )->ParentDestroyed();
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

::tools::Rectangle ThumbnailViewItem::updateHighlight(bool bVisible, const Point& rPoint)
{
    bool bNeedsPaint = false;

    if (bVisible && getDrawArea().Contains(rPoint))
    {
        if (!isHighlighted())
            bNeedsPaint = true;
        setHighlight(true);
    }
    else
    {
        if (isHighlighted())
            bNeedsPaint = true;
        setHighlight(false);
    }

    if (bNeedsPaint)
        return getDrawArea();

    return ::tools::Rectangle();
}

void ThumbnailViewItem::setTitle (const OUString& rTitle)
{
    if (mrParent.renameItem(this, rTitle))
        maTitle = rTitle;
}

uno::Reference< accessibility::XAccessible > const & ThumbnailViewItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mxAcc.is() )
        mxAcc = new ThumbnailViewItemAcc( this, bIsTransientChildrenDisabled );

    return mxAcc;
}

void ThumbnailViewItem::setDrawArea (const ::tools::Rectangle &area)
{
    maDrawArea = area;
}

void ThumbnailViewItem::calculateItemsPosition (const tools::Long nThumbnailHeight,
                                                const tools::Long nPadding, sal_uInt32 nMaxTextLength,
                                                const ThumbnailItemAttributes *pAttrs)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;
    aTextDev.setFontAttribute(pAttrs->aFontAttr,
                              pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                              css::lang::Locale() );

    Size aImageSize = maPreview1.GetSizePixel();

    // Calculate thumbnail position
    const Point aPos = maDrawArea.TopCenter();
    maPrev1Pos = aPos + Point(-aImageSize.Width() / 2, nPadding + (nThumbnailHeight - aImageSize.Height()) / 2);

    // Calculate text position
    maTextPos = aPos + Point(-aTextDev.getTextWidth(maTitle, 0, nMaxTextLength) / 2, nThumbnailHeight + nPadding * 2);
}

void ThumbnailViewItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                               const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;
    drawinglayer::primitive2d::Primitive2DContainer aSeq(4);
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

    sal_uInt32 nPrimitive = 0;
    aSeq[nPrimitive++] = drawinglayer::primitive2d::Primitive2DReference(
            new PolyPolygonSelectionPrimitive2D( B2DPolyPolygon(::tools::Polygon(maDrawArea, THUMBNAILVIEW_ITEM_CORNER, THUMBNAILVIEW_ITEM_CORNER).getB2DPolygon()),
                                                 aFillColor,
                                                 fTransparence,
                                                 0.0,
                                                 true));

    // Draw thumbnail
    Point aPos = maPrev1Pos;
    Size aImageSize = maPreview1.GetSizePixel();

    aSeq[nPrimitive++] = drawinglayer::primitive2d::Primitive2DReference( new FillGraphicPrimitive2D(
                                        createTranslateB2DHomMatrix(aPos.X(),aPos.Y()),
                                        FillGraphicAttribute(Graphic(maPreview1),
                                                            B2DRange(
                                                                B2DPoint(0,0),
                                                                B2DPoint(aImageSize.Width(),aImageSize.Height())),
                                                            false)
                                        ));

    if (mbBorder)
    {
        // draw thumbnail borders
        float fWidth = aImageSize.Width() - 1;
        float fHeight = aImageSize.Height() - 1;
        float fPosX = maPrev1Pos.getX();
        float fPosY = maPrev1Pos.getY();

        B2DPolygon aBounds;
        aBounds.append(B2DPoint(fPosX,fPosY));
        aBounds.append(B2DPoint(fPosX+fWidth,fPosY));
        aBounds.append(B2DPoint(fPosX+fWidth,fPosY+fHeight));
        aBounds.append(B2DPoint(fPosX,fPosY+fHeight));
        aBounds.setClosed(true);

        aSeq[nPrimitive++] = drawinglayer::primitive2d::Primitive2DReference(createBorderLine(aBounds));
    }

    // Draw text below thumbnail
    addTextPrimitives(maTitle, pAttrs, maTextPos, aSeq);

    pProcessor->process(aSeq);
}

void ThumbnailViewItem::addTextPrimitives (const OUString& rText, const ThumbnailItemAttributes *pAttrs, Point aPos, drawinglayer::primitive2d::Primitive2DContainer& rSeq)
{
    // adjust text drawing position according to text font
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;
    aTextDev.setFontAttribute(
        pAttrs->aFontAttr,
        pAttrs->aFontSize.getX(),
        pAttrs->aFontSize.getY(),
        css::lang::Locale());

    aPos.setY(aPos.getY() + aTextDev.getTextHeight());

    sal_Int32 nMnemonicPos = -1;
    OUString aOrigText(mrParent.isDrawMnemonic() ? OutputDevice::GetNonMnemonicString(rText, nMnemonicPos) : rText);

    TextEngine aTextEngine;
    aTextEngine.SetFont(getVclFontFromFontAttribute(pAttrs->aFontAttr,
                              pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(), 0,
                              css::lang::Locale()));
    aTextEngine.SetMaxTextWidth(maDrawArea.getWidth());
    aTextEngine.SetText(aOrigText);

    sal_Int32 nPrimitives = rSeq.size();
    sal_Int32 nFinalPrimCount = nPrimitives + aTextEngine.GetLineCount(0);
    rSeq.resize(nFinalPrimCount);

    // Create the text primitives
    sal_uInt16 nLineStart = 0;
    OUString aText(aOrigText);
    for (sal_uInt16 i=0; i < aTextEngine.GetLineCount(0); ++i)
    {
        sal_Int32 nLineLength = aTextEngine.GetLineLen(0, i);
        double nLineWidth = aTextDev.getTextWidth (aText, nLineStart, nLineLength);

        bool bTooLong = (aPos.getY() + aTextEngine.GetCharHeight()) > maDrawArea.Bottom();
        if (bTooLong && (nLineLength + nLineStart) < aOrigText.getLength())
        {
            // Add the '...' to the last line to show, even though it may require to shorten the line
            double nDotsWidth = aTextDev.getTextWidth("...",0,3);

            sal_Int32 nLength = nLineLength - 1;
            while ( nDotsWidth + aTextDev.getTextWidth(aText, nLineStart, nLength) > maDrawArea.getWidth() && nLength > 0)
            {
                --nLength;
            }

            aText = OUString::Concat(aText.subView(0, nLineStart+nLength)) + "...";
            nLineLength = nLength + 3;
        }

        double nLineX = maDrawArea.Left() + (maDrawArea.getWidth() - nLineWidth) / 2.0;

        basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                    pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                    nLineX, double( aPos.Y() ) ) );

        // setup color
        BColor aTextColor = pAttrs->aTextColor;
        if(mbSelected)
        {
            if (mbHover)
                aTextColor = pAttrs->aSelectHighlightTextColor;
            else
                aTextColor = pAttrs->aHighlightTextColor;
        }

        rSeq[nPrimitives++] = drawinglayer::primitive2d::Primitive2DReference(
                    new TextSimplePortionPrimitive2D(aTextMatrix,
                                                     aText, nLineStart, nLineLength,
                                                     std::vector<double>(),
                                                     pAttrs->aFontAttr,
                                                     css::lang::Locale(),
                                                     aTextColor));

        if (nMnemonicPos != -1 && nMnemonicPos >= nLineStart && nMnemonicPos < nLineStart + nLineLength)
        {
            rSeq.resize(nFinalPrimCount + 1);

            auto aCaretPositions = aTextDev.getCaretPositions(aText, nLineStart, nLineLength);

            auto lc_x1 = aCaretPositions[2*(nMnemonicPos - nLineStart)];
            auto lc_x2 = aCaretPositions[2*(nMnemonicPos - nLineStart)+1];
            auto fMnemonicWidth = std::abs(lc_x1 - lc_x2);
            auto fMnemonicHeight = aTextDev.getUnderlineHeight();

            auto fPosX = nLineX + std::min(lc_x1, lc_x2);
            auto fPosY = aPos.Y() + aTextDev.getUnderlineOffset();

            B2DPolygon aLine;
            aLine.append(B2DPoint(fPosX, fPosY));
            aLine.append(B2DPoint(fPosX + fMnemonicWidth, fPosY));

            drawinglayer::attribute::LineAttribute aLineAttribute(Color(aTextColor).getBColor(), fMnemonicHeight);

            rSeq[nPrimitives++] = drawinglayer::primitive2d::Primitive2DReference(
                        new PolygonStrokePrimitive2D(aLine, aLineAttribute));
        }

        nLineStart += nLineLength;
        aPos.setY(aPos.getY() + aTextEngine.GetCharHeight());

        if (bTooLong)
            break;
    }
}

rtl::Reference<drawinglayer::primitive2d::PolygonHairlinePrimitive2D>
ThumbnailViewItem::createBorderLine (const basegfx::B2DPolygon& rPolygon)
{
    return new PolygonHairlinePrimitive2D(rPolygon, Color(128, 128, 128).getBColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
