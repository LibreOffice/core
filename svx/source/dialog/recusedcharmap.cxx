/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <svx/recusedcharmap.hxx>

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/types.hxx>
#include <svl/itemset.hxx>
#include <unicode/uchar.h>
#include "charmapacc.hxx"
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


SvxShowRecCharSet::SvxShowRecCharSet(vcl::Window* pParent)
    : SvxShowCharSet(pParent)

{
    init();
}
void SvxShowRecCharSet::init()
{
    nSelectedIndex = 1;    // TODO: move into init list when it is no longer static
    m_nXGap = 0;
    m_nYGap = 0;

    SetStyle(GetStyle() | WB_CLIPCHILDREN);
    /*
    aVscrollSB->SetScrollHdl( LINK( this, SvxShowCharSet, VscrollHdl ) );
    aVscrollSB->EnableDrag();
    */
    // other settings like aVscroll depend on selected font => see RecalculateFont

    bDrag = false;
}
void SvxShowRecCharSet::InitSettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (mbUpdateForeground)
    {
        Color aTextColor(rStyleSettings.GetDialogTextColor());

        if (IsControlForeground())
            aTextColor = GetControlForeground();
        rRenderContext.SetTextColor(aTextColor);
        mbUpdateForeground = false;
    }

    if (mbUpdateBackground)
    {

        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else
            rRenderContext.SetBackground(rStyleSettings.GetWindowColor());

        mbUpdateBackground = false;
    }

    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetWeight(WEIGHT_LIGHT);
    aFont.SetAlignment(ALIGN_TOP);
    aFont.SetFontSize(maFontSize);
    aFont.SetTransparent(true);
    rRenderContext.SetFont(aFont);

}
inline Point SvxShowRecCharSet::MapIndexToPixel( int nIndex ) const
{
    const int nBase = 1;//FirstInView();
    int x = ((nIndex - nBase) % COLUMN_COUNT) * nX;
    int y = ((nIndex - nBase) / COLUMN_COUNT) * nY;
    return Point( x + m_nXGap, y + m_nYGap );
}
int SvxShowRecCharSet::PixelToMapIndex( const Point& point) const
{
    int nBase = 1;
    return (nBase + ((point.X() - m_nXGap)/nX) + ((point.Y() - m_nYGap)/nY) * COLUMN_COUNT);
}
Rectangle SvxShowRecCharSet::getGridRectangle(const Point &rPointUL, const Size &rOutputSize)
{
    long x = rPointUL.X() - 1;
    long y = rPointUL.Y() - 1;
    Point aPointUL(x+1, y+1);
    Size aGridSize(nX-1, nY-1);

    long nXDistFromLeft = x - m_nXGap;
    if (nXDistFromLeft <= 1)
    {
        aPointUL.X() = 1;
        aGridSize.Width() += m_nXGap + nXDistFromLeft;
    }
    long nXDistFromRight = rOutputSize.Width() - m_nXGap - nX - x;
    if (nXDistFromRight <= 1)
        aGridSize.Width() += m_nXGap + nXDistFromRight;

    long nXDistFromTop = y - m_nYGap;
    if (nXDistFromTop <= 1)
    {
        aPointUL.Y() = 1;
        aGridSize.Height() += m_nYGap + nXDistFromTop;
    }
    long nXDistFromBottom = rOutputSize.Height() - m_nYGap - nY - y;
    if (nXDistFromBottom <= 1)
        aGridSize.Height() += m_nYGap + nXDistFromBottom;

    return Rectangle(aPointUL, aGridSize);
}
void SvxShowRecCharSet::DrawChars_Impl(vcl::RenderContext& rRenderContext)
{


    Size aOutputSize(GetOutputSizePixel());
    int i;
    for (i = 1; i < COLUMN_COUNT; ++i)
    {
        rRenderContext.DrawLine(Point(nX * i + m_nXGap, 0),
                          Point(nX * i + m_nXGap, aOutputSize.Height()));
    }
    for (i = 1; i < ROW_COUNT; ++i)
    {
        rRenderContext.DrawLine(Point(0, nY * i + m_nYGap),
                                Point(aOutputSize.Width(), nY * i + m_nYGap));
    }
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetFieldTextColor());
    Color aHighlightColor(rStyleSettings.GetHighlightColor());
    Color aHighlightTextColor(rStyleSettings.GetHighlightTextColor());
    Color aFaceColor(rStyleSettings.GetFaceColor());
    Color aLightColor(rStyleSettings.GetLightColor());
    Color aShadowColor(rStyleSettings.GetShadowColor());



    int nTextHeight = rRenderContext.GetTextHeight();
    Rectangle aBoundRect;

    for (i = 1; i <= mRecentlyUsedCharMap.getLenght(); ++i)
    {
        Point pix = MapIndexToPixel(i);
        int x = pix.X();
        int y = pix.Y();

        OUStringBuffer buf;
        buf.appendUtf32(mRecentlyUsedCharMap.getRecentlyChar(i-1));
        OUString aCharStr(buf.makeStringAndClear());
        int nTextWidth = rRenderContext.GetTextWidth(aCharStr);
        int tx = x + (nX - nTextWidth + 1) / 2;
        int ty = y + (nY - nTextHeight + 1) / 2;
        Point aPointTxTy(tx, ty);

        rRenderContext.SetFont(mRecentlyUsedCharMap.getRecentlyCharFont(i-1));
        // adjust position before it gets out of bounds
        if (rRenderContext.GetTextBoundRect(aBoundRect, aCharStr) && !aBoundRect.IsEmpty())
        {
            // zero advance width => use ink width to center glyph
            if (!nTextWidth)
            {
                aPointTxTy.X() = x - aBoundRect.Left() + (nX - aBoundRect.GetWidth() + 1) / 2;
            }

            aBoundRect += aPointTxTy;

            // shift back vertically if needed
            int nYLDelta = aBoundRect.Top() - y;
            int nYHDelta = (y + nY) - aBoundRect.Bottom();
            if (nYLDelta <= 0)
                aPointTxTy.Y() -= nYLDelta - 1;
            else if (nYHDelta <= 0)
                aPointTxTy.Y() += nYHDelta - 1;

            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left() - x;
            int nXHDelta = (x + nX) - aBoundRect.Right();
            if (nXLDelta <= 0)
                aPointTxTy.X() -= nXLDelta - 1;
            else if (nXHDelta <= 0)
                aPointTxTy.X() += nXHDelta - 1;
        }

        Color aTextCol = rRenderContext.GetTextColor();
        if (i != nSelectedIndex)
        {
            rRenderContext.SetTextColor(aWindowTextColor);
            rRenderContext.DrawText(aPointTxTy, aCharStr);
        }
        else
        {
            Color aLineCol = rRenderContext.GetLineColor();
            Color aFillCol = rRenderContext.GetFillColor();
            rRenderContext.SetLineColor();
            Point aPointUL(x + 1, y + 1);
            if (HasFocus())
            {
                rRenderContext.SetFillColor(aHighlightColor);
                rRenderContext.DrawRect(getGridRectangle(aPointUL, aOutputSize));

                rRenderContext.SetTextColor(aHighlightTextColor);
                rRenderContext.DrawText(aPointTxTy, aCharStr);
            }
            else
            {
                rRenderContext.SetFillColor(aFaceColor);
                rRenderContext.DrawRect(getGridRectangle(aPointUL, aOutputSize));

                rRenderContext.SetLineColor(aLightColor);
                rRenderContext.DrawLine(aPointUL, Point(x + nX - 1, y + 1));
                rRenderContext.DrawLine(aPointUL, Point(x + 1, y + nY - 1));

                rRenderContext.SetLineColor(aShadowColor);
                rRenderContext.DrawLine(Point(x + 1, y + nY - 1), Point(x + nX - 1, y + nY - 1));
                rRenderContext.DrawLine(Point(x + nX - 1, y + nY - 1), Point(x + nX - 1, y + 1));

                rRenderContext.DrawText(aPointTxTy, aCharStr);
            }
            rRenderContext.SetLineColor(aLineCol);
            rRenderContext.SetFillColor(aFillCol);
        }
        rRenderContext.SetTextColor(aTextCol);
    }
}
void SvxShowRecCharSet::Paint(vcl::RenderContext& rRenderContext, const Rectangle& )
{
    InitSettings(rRenderContext);

    Size aSize(GetOutputSizePixel());
    nX = aSize.Width() / COLUMN_COUNT;
    nY = aSize.Height() / ROW_COUNT;
    Size aDrawSize(nX * COLUMN_COUNT, nY * ROW_COUNT);
    m_nXGap = (aSize.Width() - aDrawSize.Width()) / 2;
    m_nYGap = (aSize.Height() - aDrawSize.Height()) / 2;

    DrawChars_Impl(rRenderContext);
}
VCL_BUILDER_FACTORY(SvxShowRecCharSet)

sal_UCS4 SvxShowRecCharSet::GetSelectCharacter()
{
    if( nSelectedIndex > 0 )
      return  mRecentlyUsedCharMap.getRecentlyChar( nSelectedIndex -1);
    else
      return (char) 0;
}
vcl::Font SvxShowRecCharSet::GetSelectCharFont(){

      return  mRecentlyUsedCharMap.getRecentlyCharFont( nSelectedIndex -1);
}


void SvxShowRecCharSet::SelectIndex(int index){
  if (index <= mRecentlyUsedCharMap.getLenght()){
    nSelectedIndex=index;
    Invalidate();
  }else{
    nSelectedIndex=-1;
    Invalidate();
  }

  bool bFocus=true;
    if( nSelectedIndex >= 0 )
    {
        getSelectedChar() = mRecentlyUsedCharMap.getRecentlyChar( nSelectedIndex );
        if( m_pAccessible )
        {
            svx::SvxShowCharSetItem* pItem = ImplGetItem(nSelectedIndex);
            // Don't fire the focus event.
            if ( bFocus )
                m_pAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), makeAny(pItem->GetAccessible()) ); // this call assures that m_pItem is set
            else
                m_pAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS, Any(), makeAny(pItem->GetAccessible()) ); // this call assures that m_pItem is set

            assert(pItem->m_pItem && "No accessible created!");
            Any aOldAny, aNewAny;
            aNewAny <<= AccessibleStateType::FOCUSED;
            // Don't fire the focus event.
            if ( bFocus )
                pItem->m_pItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );

            aNewAny <<= AccessibleStateType::SELECTED;
            pItem->m_pItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
        }
    }
    aHighHdl.Call( this );
}


void SvxShowRecCharSet::MouseButtonDown( const MouseEvent& rMEvt )
{
   if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 1 )
        {
            GrabFocus();
            bDrag = true;
            CaptureMouse();

            int nIndex = PixelToMapIndex( rMEvt.GetPosPixel() );
            SelectIndex( nIndex);
        }

        if ( !(rMEvt.GetClicks() % 2) )
            aDoubleClkHdl.Call( this );
    }

}
void SvxShowRecCharSet::MouseButtonUp( const MouseEvent& rMEvt )
{
      if ( bDrag && rMEvt.IsLeft() )
    {
        // released mouse over character map
        if ( Rectangle(Point(), GetOutputSize()).IsInside(rMEvt.GetPosPixel()))
            aSelectHdl.Call( this );
        ReleaseMouse();
        bDrag = false;
    }

}


void SvxShowRecCharSet::MouseMove( const MouseEvent& rMEvt )
{

      if ( rMEvt.IsLeft() && bDrag )
    {
        Point aPos  = rMEvt.GetPosPixel();
        Size  aSize = GetSizePixel();

        if ( aPos.X() < 0 )
            aPos.X() = 0;
        else if ( aPos.X() > aSize.Width()-5 )
            aPos.X() = aSize.Width()-5;
        if ( aPos.Y() < 0 )
            aPos.Y() = 0;
        else if ( aPos.Y() > aSize.Height()-5 )
            aPos.Y() = aSize.Height()-5;

        int nIndex = PixelToMapIndex( aPos );
    // Fire the focus event.
        SelectIndex( nIndex);
    }

}

void SvxShowRecCharSet::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();

    if (aCode.GetModifier())
    {
        Control::KeyInput(rKEvt);
        return;
    }

    int tmpSelected = nSelectedIndex;

    switch (aCode.GetCode())
    {
        case KEY_SPACE:
            aSelectHdl.Call( this );
            break;
        case KEY_LEFT:
            --tmpSelected;
            break;
        case KEY_RIGHT:
            ++tmpSelected;
            break;
        case KEY_UP:
            tmpSelected -= COLUMN_COUNT;
            break;
        case KEY_DOWN:
            tmpSelected += COLUMN_COUNT;
            break;
        case KEY_PAGEUP:
        case KEY_PAGEDOWN:
        case KEY_HOME:
            tmpSelected = 0;
            break;
        case KEY_END:
            tmpSelected = mRecentlyUsedCharMap.getLenght()-1;
            break;
        case KEY_TAB:   // some fonts have a character at these unicode control codes
        case KEY_ESCAPE:
        case KEY_RETURN:
            Control::KeyInput(rKEvt);
            tmpSelected = - 1;  // mark as invalid
            break;

    }

    if ( tmpSelected > 0 && tmpSelected <= mRecentlyUsedCharMap.getLenght() )
    {
        SelectIndex( tmpSelected );
        aPreSelectHdl.Call( this );
    }
}
void SvxShowRecCharSet::GetFocus()
{
    Control::GetFocus();
    SelectIndex( nSelectedIndex);
}

void SvxShowRecCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectIndex( nSelectedIndex);
}
void SvxShowRecCharSet::StateChanged(StateChangedType nType)
{
    if (nType == StateChangedType::ControlForeground)
        mbUpdateForeground = true;
    else if (nType == StateChangedType::ControlBackground)
        mbUpdateBackground = true;

    Invalidate();

    Control::StateChanged( nType );

}

void SvxShowRecCharSet::pushRecentlyChar(sal_UCS4 cChar,vcl::Font aFont){
  mRecentlyUsedCharMap.pushRecentlyChar(cChar,aFont);
  Invalidate();
}

css::uno::Reference< XAccessible > SvxShowRecCharSet::CreateAccessible()
{
    OSL_ENSURE(!m_pAccessible,"Accessible already created!");
    m_pAccessible = new svx::SvxShowCharSetVirtualAcc(this);
    m_xAccessible = m_pAccessible;
    return m_xAccessible;
}

svx::SvxShowCharSetItem* SvxShowRecCharSet::ImplGetItem( int _nPos )
{
    ItemsMap::iterator aFind = m_aItems.find(_nPos);
    if ( aFind == m_aItems.end() )
    {
        OSL_ENSURE(m_pAccessible,"Who wants to create a child of my table without a parent?");
        std::shared_ptr<svx::SvxShowCharSetItem> xItem(new svx::SvxShowCharSetItem(*this,
            m_pAccessible->getTable(), sal::static_int_cast< sal_uInt16 >(_nPos)));
        aFind = m_aItems.insert(ItemsMap::value_type(_nPos, xItem)).first;
        OUStringBuffer buf;
        buf.appendUtf32( mRecentlyUsedCharMap.getRecentlyChar( _nPos ));
        aFind->second->maText = buf.makeStringAndClear();
        Point pix = MapIndexToPixel( _nPos );
        aFind->second->maRect = Rectangle( Point( pix.X() + 1, pix.Y() + 1 ), Size(nX-1,nY-1) );
    }

    return aFind->second.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */