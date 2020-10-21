/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <emojiviewitem.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/PolyPolygonSelectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustrbuf.hxx>
#include <tools/poly.hxx>

using namespace basegfx;
using namespace basegfx::utils;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

EmojiViewItem::EmojiViewItem (ThumbnailViewBase &rView, sal_uInt16 nId)
    : ThumbnailViewItem(rView, nId)
{
}

EmojiViewItem::~EmojiViewItem ()
{
}


void EmojiViewItem::calculateItemsPosition (const tools::Long /*nThumbnailHeight*/,
                                                const tools::Long /*nPadding*/, sal_uInt32 nMaxTextLength,
                                                const ThumbnailItemAttributes *pAttrs)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;
    aTextDev.setFontAttribute(pAttrs->aFontAttr,
                              pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                              css::lang::Locale() );

    Size aRectSize = maDrawArea.GetSize();
    Point aPos = maDrawArea.TopLeft();

    // Calculate text position
    aPos.setY( maDrawArea.getY() + (aRectSize.Height() - aTextDev.getTextHeight())/3 );
    aPos.setX( maDrawArea.Left() + (aRectSize.Width() - aTextDev.getTextWidth(maTitle,0,nMaxTextLength))/2 );
    maTextPos = aPos;
}


void EmojiViewItem::Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                                   const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;

    drawinglayer::primitive2d::Primitive2DContainer aSeq(2);
    double fTransparence = 0.0;

    // Draw background
    if( mbSelected && mbHover)
        aFillColor = pAttrs->aSelectHighlightColor;
    else if (mbSelected || mbHover)
        aFillColor = pAttrs->aHighlightColor;

    if (mbHover)
        fTransparence = pAttrs->fHighlightTransparence;

    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
            new PolyPolygonSelectionPrimitive2D( B2DPolyPolygon(::tools::Polygon(maDrawArea,5,5).getB2DPolygon()),
                                                 aFillColor,
                                                 fTransparence,
                                                 0.0,
                                                 true));

    OUStringBuffer sHexText = "";
    sHexText.appendUtf32(maTitle.toUInt32(16));

    addTextPrimitives(sHexText.toString(), pAttrs, maTextPos, aSeq);

    pProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
