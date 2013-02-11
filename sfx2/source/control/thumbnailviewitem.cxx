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

#include "thumbnailview.hxx"
#include "thumbnailviewacc.hxx"

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <vcl/button.hxx>
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
        ResizableMultiLineEdit (Window* pParent, ThumbnailViewItem* pItem);
        ~ResizableMultiLineEdit ();

        void SetInGrabFocus(bool bInGrabFocus) { mbIsInGrabFocus = bInGrabFocus; }

        virtual long PreNotify(NotifyEvent& rNEvt);
        virtual void Modify();
};

ResizableMultiLineEdit::ResizableMultiLineEdit (Window* pParent, ThumbnailViewItem* pItem) :
    VclMultiLineEdit (pParent, WB_CENTER | WB_BORDER),
    mpItem(pItem),
    mbIsInGrabFocus(false)
{
}

ResizableMultiLineEdit::~ResizableMultiLineEdit ()
{
}

long ResizableMultiLineEdit::PreNotify(NotifyEvent& rNEvt)
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        KeyCode aCode = rKEvt.GetKeyCode();
        switch (aCode.GetCode())
        {
            case KEY_RETURN:
                mpItem->setTitle( GetText() );
            case KEY_ESCAPE:
                mpItem->setEditTitle(false);
                nDone = 1;
                break;
            default:
                break;
        }
    }
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS && !mbIsInGrabFocus )
    {
        mpItem->setTitle( GetText() );
        mpItem->setEditTitle(false, false);
    }
    return nDone ? nDone : VclMultiLineEdit::PreNotify(rNEvt);
}

void ResizableMultiLineEdit::Modify()
{
    VclMultiLineEdit::Modify();
    mpItem->updateTitleEditSize();
}

ThumbnailViewItem::ThumbnailViewItem(ThumbnailView &rView)
    : mrParent(rView)
    , mnId(0)
    , mbVisible(true)
    , mbSelected(false)
    , mbHover(false)
    , mpxAcc(NULL)
    , mbEditTitle(false)
    , mpTitleED(NULL)
    , maTextEditMaxArea()
{
    mpTitleED = new ResizableMultiLineEdit(&rView, this);
}

ThumbnailViewItem::~ThumbnailViewItem()
{
    delete mpTitleED;
    if( mpxAcc )
    {
        static_cast< ThumbnailViewItemAcc* >( mpxAcc->get() )->ParentDestroyed();
        delete mpxAcc;
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

void ThumbnailViewItem::setEditTitle (bool edit, bool bChangeFocus)
{
    mbEditTitle = edit;
    mpTitleED->Show(edit);
    if (edit)
    {
        mpTitleED->SetText(maTitle);
        updateTitleEditSize();
        static_cast<ResizableMultiLineEdit*>(mpTitleED)->SetInGrabFocus(true);
        mpTitleED->GrabFocus();
        static_cast<ResizableMultiLineEdit*>(mpTitleED)->SetInGrabFocus(false);
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

void ThumbnailViewItem::setTitle (const rtl::OUString& rTitle)
{
    mrParent.renameItem(this, rTitle);
    maTitle = rTitle;
}

uno::Reference< accessibility::XAccessible > ThumbnailViewItem::GetAccessible( bool bIsTransientChildrenDisabled )
{
    if( !mpxAcc )
        mpxAcc = new uno::Reference< accessibility::XAccessible >( new ThumbnailViewItemAcc( this, bIsTransientChildrenDisabled ) );

    return *mpxAcc;
}

void ThumbnailViewItem::setDrawArea (const Rectangle &area)
{
    maDrawArea = area;
}

void ThumbnailViewItem::calculateItemsPosition (const long nThumbnailHeight, const long,
                                                const long nPadding, sal_uInt32 nMaxTextLenght,
                                                const ThumbnailItemAttributes *pAttrs)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;
    aTextDev.setFontAttribute(pAttrs->aFontAttr,
                              pAttrs->aFontSize.getX(), pAttrs->aFontSize.getY(),
                              com::sun::star::lang::Locale() );

    Size aRectSize = maDrawArea.GetSize();
    Size aImageSize = maPreview1.GetSizePixel();

    // Calculate thumbnail position
    Point aPos = maDrawArea.TopLeft();
    aPos.X() = maDrawArea.getX() + (aRectSize.Width()-aImageSize.Width())/2;
    aPos.Y() = maDrawArea.getY() + nPadding + (nThumbnailHeight-aImageSize.Height())/2;
    maPrev1Pos = aPos;

    // Calculate text position
    aPos.Y() = maDrawArea.getY() + nThumbnailHeight + nPadding * 2;
    aPos.X() = maDrawArea.Left() + (aRectSize.Width() - aTextDev.getTextWidth(maTitle,0,nMaxTextLenght))/2;
    maTextPos = aPos;

    // Calculate the text edit max area
    aPos = Point(maDrawArea.getX() + nPadding, maTextPos.getY());
    Size aEditSize(maDrawArea.GetWidth() - nPadding * 2,
                   maDrawArea.Bottom() - maTextPos.Y());
    maTextEditMaxArea = Rectangle( aPos, aEditSize );
}

void ThumbnailViewItem::setSelectClickHdl (const Link &link)
{
    maClickHdl = link;
}

void ThumbnailViewItem::Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                               const ThumbnailItemAttributes *pAttrs)
{
    BColor aFillColor = pAttrs->aFillColor;
    Primitive2DSequence aSeq(4);

    // Draw background
    if ( mbSelected || mbHover )
        aFillColor = pAttrs->aHighlightColor;

    sal_uInt32 nPrimitive = 0;
    aSeq[nPrimitive++] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
                                               B2DPolyPolygon(Polygon(maDrawArea,5,5).getB2DPolygon()),
                                               aFillColor));

    // Draw thumbnail
    Point aPos = maPrev1Pos;
    Size aImageSize = maPreview1.GetSizePixel();

    aSeq[nPrimitive++] = Primitive2DReference( new FillBitmapPrimitive2D(
                                        createTranslateB2DHomMatrix(aPos.X(),aPos.Y()),
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

    B2DPolygon aBounds;
    aBounds.append(B2DPoint(fPosX,fPosY));
    aBounds.append(B2DPoint(fPosX+fWidth,fPosY));
    aBounds.append(B2DPoint(fPosX+fWidth,fPosY+fHeight));
    aBounds.append(B2DPoint(fPosX,fPosY+fHeight));
    aBounds.setClosed(true);

    aSeq[nPrimitive++] = Primitive2DReference(createBorderLine(aBounds));

    // Draw text below thumbnail
    aPos = maTextPos;
    addTextPrimitives( maTitle, pAttrs, aPos, aSeq );

    pProcessor->process(aSeq);
}

void ThumbnailViewItem::addTextPrimitives (const rtl::OUString& rText, const ThumbnailItemAttributes *pAttrs, Point aPos, Primitive2DSequence& rSeq)
{
    drawinglayer::primitive2d::TextLayouterDevice aTextDev;

    aPos.setY(aPos.getY() + aTextDev.getTextHeight());

    rtl::OUString aText (rText);

    TextEngine aTextEngine;
    aTextEngine.SetMaxTextWidth(maDrawArea.getWidth());
    aTextEngine.SetText(rText);

    sal_Int32 nPrimitives = rSeq.getLength();
    rSeq.realloc(nPrimitives + aTextEngine.GetLineCount(0));

    // Create the text primitives
    sal_uInt16 nLineStart = 0;
    for (sal_uInt16 i=0; i<aTextEngine.GetLineCount(0); ++i)
    {
        sal_uInt16 nLineLength = aTextEngine.GetLineLen(0, i);
        double nLineWidth = aTextDev.getTextWidth (aText, nLineStart, nLineLength);

        bool bTooLong = (aPos.getY() + aTextEngine.GetCharHeight()) > maDrawArea.Bottom();
        if (bTooLong && (nLineLength + nLineStart) < rText.getLength())
        {
            // Add the '...' to the last line to show, even though it may require to shorten the line
            double nDotsWidth = aTextDev.getTextWidth(rtl::OUString("..."),0,3);

            sal_uInt16 nLength = nLineLength - 1;
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

        rSeq[nPrimitives++] = Primitive2DReference(
                    new TextSimplePortionPrimitive2D(aTextMatrix,
                                                     aText,nLineStart,nLineLength,
                                                     std::vector< double >( ),
                                                     pAttrs->aFontAttr,
                                                     com::sun::star::lang::Locale(),
                                                     Color(COL_BLACK).getBColor() ) );
        nLineStart += nLineLength;
        aPos.setY(aPos.getY() + aTextEngine.GetCharHeight());

        if (bTooLong)
            break;
    }
}

drawinglayer::primitive2d::PolygonHairlinePrimitive2D*
ThumbnailViewItem::createBorderLine (const basegfx::B2DPolygon& rPolygon)
{
    return new PolygonHairlinePrimitive2D(rPolygon, Color(186,186,186).getBColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


