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
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <vcl/button.hxx>
#include <vcl/graphic.hxx>
#include <vcl/svapp.hxx>
#include <vcl/texteng.hxx>

using namespace basegfx;
using namespace basegfx::tools;
using namespace ::com::sun::star;
using namespace drawinglayer::attribute;
using namespace drawinglayer::primitive2d;

class ResizableMultiLineEdit : public VclMultiLineEdit
{
    private:
        ThumbnailViewItem* mpItem;
        bool mbIsInGrabFocus;

    public:
        ResizableMultiLineEdit (vcl::Window* pParent, ThumbnailViewItem* pItem);

        void SetInGrabFocus(bool bInGrabFocus) { mbIsInGrabFocus = bInGrabFocus; }

        virtual bool PreNotify(NotifyEvent& rNEvt) override;
        virtual void Modify() override;
};

ResizableMultiLineEdit::ResizableMultiLineEdit (vcl::Window* pParent, ThumbnailViewItem* pItem) :
    VclMultiLineEdit (pParent, WB_CENTER | WB_BORDER),
    mpItem(pItem),
    mbIsInGrabFocus(false)
{
}

bool ResizableMultiLineEdit::PreNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = rKEvt.GetKeyCode();
        switch (aCode.GetCode())
        {
            case KEY_RETURN:
                mpItem->setTitle( GetText() );
                SAL_FALLTHROUGH;
            case KEY_ESCAPE:
                mpItem->setEditTitle(false);
                bDone = true;
                break;
            default:
                break;
        }
    }
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS && !mbIsInGrabFocus )
    {
        mpItem->setTitle( GetText() );
        mpItem->setEditTitle(false, false);
    }
    return bDone || VclMultiLineEdit::PreNotify(rNEvt);
}

void ResizableMultiLineEdit::Modify()
{
    VclMultiLineEdit::Modify();
    mpItem->updateTitleEditSize();
}

ThumbnailViewItem::ThumbnailViewItem(ThumbnailView &rView, sal_uInt16 nId)
    : mrParent(rView)
    , mnId(nId)
    , mbVisible(true)
    , mbSelected(false)
    , mbHover(false)
    , mxAcc()
    , mbEditTitle(false)
    , mpTitleED(nullptr)
    , maTextEditMaxArea()
{
    mpTitleED = VclPtr<ResizableMultiLineEdit>::Create(&rView, this);
}

ThumbnailViewItem::~ThumbnailViewItem()
{
    mpTitleED.disposeAndClear();
    if( mxAcc.is() )
    {
        static_cast< ThumbnailViewItemAcc* >( mxAcc.get() )->ParentDestroyed();
    }
}

void ThumbnailViewItem::show (bool bVisible)
{
    mbVisible = bVisible;
    if (!mbVisible)
        mpTitleED->Show(false);
}

void ThumbnailViewItem::setSelection (bool state)
{
    mbSelected = state;
}

void ThumbnailViewItem::setHighlight (bool state)
{
    mbHover = state;
}

Rectangle ThumbnailViewItem::updateHighlight(bool bVisible, const Point& rPoint)
{
    bool bNeedsPaint = false;

    if (bVisible && getDrawArea().IsInside(rPoint))
    {
        if (!isHighlighted())
            bNeedsPaint = true;
        setHighlight(true);
    }
    else
    {
        if (isHighlighted() || mpTitleED->SupportsDoubleBuffering())
            bNeedsPaint = true;
        setHighlight(false);
    }

    if (bNeedsPaint)
        return getDrawArea();

    return Rectangle();
}

OUString ThumbnailViewItem::getHelpText() const
{
    return maTitle;
}

void ThumbnailViewItem::setEditTitle (bool edit, bool bChangeFocus)
{
    mbEditTitle = edit;
    mpTitleED->Show(edit);
    if (edit)
    {
        mpTitleED->SetText(maTitle);
        updateTitleEditSize();
        static_cast<ResizableMultiLineEdit*>(mpTitleED.get())->SetInGrabFocus(true);
        mpTitleED->GrabFocus();
        static_cast<ResizableMultiLineEdit*>(mpTitleED.get())->SetInGrabFocus(false);
    }
    else if (bChangeFocus)
    {
        mrParent.GrabFocus();
    }
}

Rectangle ThumbnailViewItem::getTextArea() const
{
    Rectangle aTextArea(maTextEditMaxArea);

    TextEngine aTextEngine;
    aTextEngine.SetMaxTextWidth(maDrawArea.getWidth());
    aTextEngine.SetText(maTitle);

    long nTxtHeight = aTextEngine.GetTextHeight() + 6;
    if (nTxtHeight < aTextArea.GetHeight())
        aTextArea.SetSize(Size(aTextArea.GetWidth(), nTxtHeight));

    return aTextArea;
}

void ThumbnailViewItem::updateTitleEditSize()
{
    Rectangle aTextArea = getTextArea();
    Point aPos = aTextArea.TopLeft();
    Size aSize = aTextArea.GetSize();
    mpTitleED->SetPosSizePixel(aPos, aSize);
}

void ThumbnailViewItem::setTitle (const OUString& rTitle)
{
    if (mrParent.renameItem(this, rTitle))
        maTitle = rTitle;
}

uno::Reference< accessibility::XAccessible > ThumbnailViewItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mxAcc.is() )
        mxAcc = new ThumbnailViewItemAcc( this, bIsTransientChildrenDisabled );

    return mxAcc;
}

void ThumbnailViewItem::setDrawArea (const Rectangle &area)
{
    maDrawArea = area;
}

void ThumbnailViewItem::calculateItemsPosition (const long nThumbnailHeight, const long,
                                                const long nPadding, sal_uInt32 nMaxTextLength,
                                                const ThumbnailItemAttributes *pAttrs)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;
    aTextDev.setFontAttribute(pAttrs->aFontAttr,
                              pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                              css::lang::Locale() );

    Size aRectSize = maDrawArea.GetSize();
    Size aImageSize = maPreview1.GetSizePixel();

    // Calculate thumbnail position
    Point aPos = maDrawArea.TopLeft();
    aPos.X() = maDrawArea.getX() + (aRectSize.Width()-aImageSize.Width())/2;
    aPos.Y() = maDrawArea.getY() + nPadding + (nThumbnailHeight-aImageSize.Height())/2;
    maPrev1Pos = aPos;

    // Calculate text position
    aPos.Y() = maDrawArea.getY() + nThumbnailHeight + nPadding * 2;
    aPos.X() = maDrawArea.Left() + (aRectSize.Width() - aTextDev.getTextWidth(maTitle,0,nMaxTextLength))/2;
    maTextPos = aPos;

    // Calculate the text edit max area
    aPos = Point(maDrawArea.getX() + nPadding, maTextPos.getY());
    Size aEditSize(maDrawArea.GetWidth() - nPadding * 2,
                   maDrawArea.Bottom() - maTextPos.Y());
    maTextEditMaxArea = Rectangle( aPos, aEditSize );
}

void ThumbnailViewItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                               const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;
    drawinglayer::primitive2d::Primitive2DSequence aSeq(4);
    double fTransparence = 0.0;

    // Draw background
    if (mbSelected || mbHover)
        aFillColor = pAttrs->aHighlightColor;

    if (mbHover)
        fTransparence = pAttrs->fHighlightTransparence;

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

    // Draw text below thumbnail
    addTextPrimitives(maTitle, pAttrs, maTextPos, aSeq);

    pProcessor->process(aSeq);
}

void ThumbnailViewItem::addTextPrimitives (const OUString& rText, const ThumbnailItemAttributes *pAttrs, Point aPos, drawinglayer::primitive2d::Primitive2DSequence& rSeq)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;

    aPos.setY(aPos.getY() + aTextDev.getTextHeight());

    OUString aText (rText);

    TextEngine aTextEngine;
    aTextEngine.SetMaxTextWidth(maDrawArea.getWidth());
    aTextEngine.SetText(rText);

    sal_Int32 nPrimitives = rSeq.getLength();
    rSeq.realloc(nPrimitives + aTextEngine.GetLineCount(0));

    // Create the text primitives
    sal_uInt16 nLineStart = 0;
    for (sal_uInt16 i=0; i < aTextEngine.GetLineCount(0); ++i)
    {
        sal_Int32 nLineLength = aTextEngine.GetLineLen(0, i);
        double nLineWidth = aTextDev.getTextWidth (aText, nLineStart, nLineLength);

        bool bTooLong = (aPos.getY() + aTextEngine.GetCharHeight()) > maDrawArea.Bottom();
        if (bTooLong && (nLineLength + nLineStart) < rText.getLength())
        {
            // Add the '...' to the last line to show, even though it may require to shorten the line
            double nDotsWidth = aTextDev.getTextWidth("...",0,3);

            sal_Int32 nLength = nLineLength - 1;
            while ( nDotsWidth + aTextDev.getTextWidth(aText, nLineStart, nLength) > maDrawArea.getWidth() && nLength > 0)
            {
                --nLength;
            }

            aText = aText.copy(0, nLineStart+nLength);
            aText += "...";
            nLineLength = nLength + 3;
        }

        double nLineX = maDrawArea.Left() + (maDrawArea.getWidth() - nLineWidth) / 2.0;

        basegfx::B2DHomMatrix aTextMatrix( createScaleTranslateB2DHomMatrix(
                    pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                    nLineX, double( aPos.Y() ) ) );

        // setup color
        BColor aTextColor = pAttrs->aTextColor;
        if (mbSelected || mbHover)
            aTextColor = pAttrs->aHighlightTextColor;

        rSeq[nPrimitives++] = drawinglayer::primitive2d::Primitive2DReference(
                    new TextSimplePortionPrimitive2D(aTextMatrix,
                                                     aText, nLineStart, nLineLength,
                                                     std::vector<double>(),
                                                     pAttrs->aFontAttr,
                                                     css::lang::Locale(),
                                                     aTextColor));
        nLineStart += nLineLength;
        aPos.setY(aPos.getY() + aTextEngine.GetCharHeight());

        if (bTooLong)
            break;
    }
}

drawinglayer::primitive2d::PolygonHairlinePrimitive2D*
ThumbnailViewItem::createBorderLine (const basegfx::B2DPolygon& rPolygon)
{
    return new PolygonHairlinePrimitive2D(rPolygon, Color(128, 128, 128).getBColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
