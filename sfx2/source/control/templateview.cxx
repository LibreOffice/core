/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateview.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/thumbnailviewitem.hxx>

using namespace basegfx::tools;
using namespace drawinglayer::primitive2d;

TemplateView::TemplateView (Window *pParent, SfxDocumentTemplates *pTemplates)
    : ThumbnailView(pParent),
      mnRegionId(0),
      mpDocTemplates(pTemplates)
{
    mnHeaderHeight = 30;
}

TemplateView::~TemplateView ()
{
}

void TemplateView::setRegionId (const sal_uInt16 nRegionId)
{
    mnRegionId = nRegionId;
    maFolderName = mpDocTemplates->GetRegionName(nRegionId);
}

void TemplateView::Paint (const Rectangle &rRect)
{
    ThumbnailView::Paint(rRect);

    Primitive2DSequence aSeq(1);
    TextLayouterDevice aTextDev;

    // Draw centered region name
    Point aPos;
    Size aWinSize = GetOutputSizePixel();

    aPos.X() = (aWinSize.getWidth() - aTextDev.getTextWidth(maFolderName,0,maFolderName.getLength()))/2;
    aPos.Y() = aTextDev.getTextHeight() + (mnHeaderHeight - aTextDev.getTextHeight())/2;

    basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                mpItemAttrs->aFontSize.getX(), mpItemAttrs->aFontSize.getY(),
                double( aPos.X() ), double( aPos.Y() ) ) );

    aSeq[0] = Primitive2DReference(
                new TextSimplePortionPrimitive2D(aTextMatrix,
                                                 maFolderName,0,maFolderName.getLength(),
                                                 std::vector< double >( ),
                                                 mpItemAttrs->aFontAttr,
                                                 com::sun::star::lang::Locale(),
                                                 Color(COL_BLACK).getBColor() ) );

    mpProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


