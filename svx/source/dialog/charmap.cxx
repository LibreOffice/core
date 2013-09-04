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

#include <stdio.h>

#include <vcl/svapp.hxx>
#include <svtools/colorcfg.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>

#include <svx/dialogs.hrc>

#include <svx/charmap.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svxdlg.hxx>

#include "charmapacc.hxx"
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/types.hxx>
#include <svl/itemset.hxx>
#include <unicode/uchar.h>

#include "rtl/ustrbuf.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

// -----------------------------------------------------------------------
sal_uInt32& SvxShowCharSet::getSelectedChar()
{
    static sal_uInt32 cSelectedChar = ' '; // keeps selected character over app livetime
    return cSelectedChar;
}

// class SvxShowCharSet ==================================================

SvxShowCharSet::SvxShowCharSet(Window* pParent, const ResId& rResId)
    : Control(pParent, rResId)
    , m_pAccessible(NULL)
    , aVscrollSB(this, WB_VERT)
{
    init();
    InitSettings( sal_True, sal_True );
}

SvxShowCharSet::SvxShowCharSet(Window* pParent)
    : Control(pParent, WB_TABSTOP)
    , m_pAccessible(NULL)
    , aVscrollSB( this, WB_VERT)
{
    init();
    InitSettings( sal_True, sal_True );
}

void SvxShowCharSet::init()
{
    nSelectedIndex = -1;    // TODO: move into init list when it is no longer static
    m_nXGap = 0;
    m_nYGap = 0;

    SetStyle( GetStyle() | WB_CLIPCHILDREN );
    aVscrollSB.SetScrollHdl( LINK( this, SvxShowCharSet, VscrollHdl ) );
    aVscrollSB.EnableDrag( sal_True );
    // other settings like aVscroll depend on selected font => see SetFont

    bDrag = sal_False;
}

void SvxShowCharSet::Resize()
{
    Control::Resize();
    SetFont(GetFont()); //force recalculation of correct fontsize
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxShowCharSet(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxShowCharSet(pParent);
}

// -----------------------------------------------------------------------

void SvxShowCharSet::GetFocus()
{
    Control::GetFocus();
    SelectIndex( nSelectedIndex, sal_True );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectIndex( nSelectedIndex, sal_False );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( sal_True, sal_False );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( sal_False, sal_True );

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS )
      && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( sal_True, sal_True );
    else
        Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 1 )
        {
            GrabFocus();
            bDrag = sal_True;
            CaptureMouse();

            int nIndex = PixelToMapIndex( rMEvt.GetPosPixel() );
            SelectIndex( nIndex );
        }

        if ( !(rMEvt.GetClicks() % 2) )
            aDoubleClkHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( bDrag && rMEvt.IsLeft() )
    {
        // released mouse over character map
        if ( Rectangle(Point(), GetOutputSize()).IsInside(rMEvt.GetPosPixel()))
            aSelectHdl.Call( this );
        ReleaseMouse();
        bDrag = sal_False;
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::MouseMove( const MouseEvent& rMEvt )
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
        SelectIndex( nIndex );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::Command( const CommandEvent& rCEvt )
{
    if( !HandleScrollCommand( rCEvt, 0, &aVscrollSB ) )
        Control::Command( rCEvt );
}

// -----------------------------------------------------------------------------

sal_uInt16 SvxShowCharSet::GetRowPos(sal_uInt16 _nPos) const
{
    return _nPos / COLUMN_COUNT ;
}

// -----------------------------------------------------------------------------

sal_uInt16 SvxShowCharSet::GetColumnPos(sal_uInt16 _nPos) const
{
    return _nPos % COLUMN_COUNT ;
}

// -----------------------------------------------------------------------

int SvxShowCharSet::FirstInView( void ) const
{
    int nIndex = 0;
    if( aVscrollSB.IsVisible() )
        nIndex += aVscrollSB.GetThumbPos() * COLUMN_COUNT;
    return nIndex;
}

// -----------------------------------------------------------------------

int SvxShowCharSet::LastInView( void ) const
{
    sal_uIntPtr nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT - 1;
    sal_uIntPtr nCompare = sal::static_int_cast<sal_uIntPtr>( maFontCharMap.GetCharCount() - 1 );
    if( nIndex > nCompare )
        nIndex = nCompare;
    return nIndex;
}

// -----------------------------------------------------------------------

inline Point SvxShowCharSet::MapIndexToPixel( int nIndex ) const
{
    const int nBase = FirstInView();
    int x = ((nIndex - nBase) % COLUMN_COUNT) * nX;
    int y = ((nIndex - nBase) / COLUMN_COUNT) * nY;
    return Point( x + m_nXGap, y + m_nYGap );
}
// -----------------------------------------------------------------------------

int SvxShowCharSet::PixelToMapIndex( const Point& point) const
{
    int nBase = FirstInView();
    return (nBase + ((point.X() - m_nXGap)/nX) + ((point.Y() - m_nYGap)/nY) * COLUMN_COUNT);
}

// -----------------------------------------------------------------------

void SvxShowCharSet::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();

    if( aCode.GetModifier() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    int tmpSelected = nSelectedIndex;

    switch ( aCode.GetCode() )
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
            tmpSelected -= ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_PAGEDOWN:
            tmpSelected += ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_HOME:
            tmpSelected = 0;
            break;
        case KEY_END:
            tmpSelected = maFontCharMap.GetCharCount() - 1;
            break;
        case KEY_TAB:   // some fonts have a character at these unicode control codes
        case KEY_ESCAPE:
        case KEY_RETURN:
            Control::KeyInput( rKEvt );
            tmpSelected = - 1;  // mark as invalid
            break;
        default:
            {
                sal_UCS4 cChar = rKEvt.GetCharCode();
                sal_UCS4 cNext = maFontCharMap.GetNextChar( cChar - 1 );
                tmpSelected = maFontCharMap.GetIndexFromChar( cNext );
                if( tmpSelected < 0 || (cChar != cNext) )
                {
                    Control::KeyInput( rKEvt );
                    tmpSelected = - 1;  // mark as invalid
                }
            }
    }

    if ( tmpSelected >= 0 )
    {
        SelectIndex( tmpSelected, sal_True );
        aPreSelectHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::Paint( const Rectangle& )
{
    DrawChars_Impl( FirstInView(), LastInView() );
}
// -----------------------------------------------------------------------------
void SvxShowCharSet::DeSelect()
{
    DrawChars_Impl(nSelectedIndex,nSelectedIndex);
}

// stretch a grid rectangle if its at the edge to fill unused space
Rectangle SvxShowCharSet::getGridRectangle(const Point &rPointUL, const Size &rOutputSize)
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

void SvxShowCharSet::DrawChars_Impl( int n1, int n2 )
{
    if( n1 > LastInView() || n2 < FirstInView() )
        return;

    Size aOutputSize = GetOutputSizePixel();
    if (aVscrollSB.IsVisible())
        aOutputSize.Width() -= aVscrollSB.GetOptimalSize().Width();

    int i;
    for ( i = 1; i < COLUMN_COUNT; ++i )
        DrawLine( Point( nX * i + m_nXGap, 0 ), Point( nX * i + m_nXGap, aOutputSize.Height() ) );
    for ( i = 1; i < ROW_COUNT; ++i )
        DrawLine( Point( 0, nY * i + m_nYGap ), Point( aOutputSize.Width(), nY * i + m_nYGap) );

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor( rStyleSettings.GetFieldTextColor() );
    Color aHighlightColor( rStyleSettings.GetHighlightColor() );
    Color aHighlightTextColor( rStyleSettings.GetHighlightTextColor() );
    Color aFaceColor( rStyleSettings.GetFaceColor() );
    Color aLightColor( rStyleSettings.GetLightColor() );
    Color aShadowColor( rStyleSettings.GetShadowColor() );

    int nTextHeight = GetTextHeight();
    Rectangle aBoundRect;
    for( i = n1; i <= n2; ++i )
    {
        Point pix = MapIndexToPixel( i );
        int x = pix.X();
        int y = pix.Y();

        OUStringBuffer buf;
        buf.appendUtf32( maFontCharMap.GetCharFromIndex( i ) );
        OUString aCharStr(buf.makeStringAndClear());
        int nTextWidth = GetTextWidth(aCharStr);
        int tx = x + (nX - nTextWidth + 1) / 2;
        int ty = y + (nY - nTextHeight + 1) / 2;
        Point aPointTxTy( tx, ty );

        // adjust position before it gets out of bounds
        if( GetTextBoundRect( aBoundRect, aCharStr ) && !aBoundRect.IsEmpty() )
        {
            // zero advance width => use ink width to center glyph
            if( !nTextWidth )
            {
                aPointTxTy.X() = x - aBoundRect.Left()
                               + (nX - aBoundRect.GetWidth() + 1) / 2;
            }

            aBoundRect += aPointTxTy;

            // shift back vertically if needed
            int nYLDelta = aBoundRect.Top() - y;
            int nYHDelta = (y + nY) - aBoundRect.Bottom();
            if( nYLDelta <= 0 )
                aPointTxTy.Y() -= nYLDelta - 1;
            else if( nYHDelta <= 0 )
                aPointTxTy.Y() += nYHDelta - 1;

            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left() - x;
            int nXHDelta = (x + nX) - aBoundRect.Right();
            if( nXLDelta <= 0 )
                aPointTxTy.X() -= nXLDelta - 1;
            else if( nXHDelta <= 0 )
                aPointTxTy.X() += nXHDelta - 1;
        }

        Color aTextCol = GetTextColor();
        if ( i != nSelectedIndex )
        {
            SetTextColor( aWindowTextColor );
            DrawText( aPointTxTy, aCharStr );
        }
        else
        {
            Color aLineCol = GetLineColor();
            Color aFillCol = GetFillColor();
            SetLineColor();
            Point aPointUL( x + 1, y + 1 );
            if( HasFocus() )
            {
                SetFillColor( aHighlightColor );
                DrawRect( getGridRectangle(aPointUL, aOutputSize) );

                SetTextColor( aHighlightTextColor );
                DrawText( aPointTxTy, aCharStr );
            }
            else
            {
                SetFillColor( aFaceColor );
                DrawRect( getGridRectangle(aPointUL, aOutputSize) );

                SetLineColor( aLightColor );
                DrawLine( aPointUL, Point( x+nX-1, y+1) );
                DrawLine( aPointUL, Point( x+1, y+nY-1) );

                SetLineColor( aShadowColor );
                DrawLine( Point( x+1, y+nY-1), Point( x+nX-1, y+nY-1) );
                DrawLine( Point( x+nX-1, y+nY-1), Point( x+nX-1, y+1) );

                DrawText( aPointTxTy, aCharStr );
            }
            SetLineColor( aLineCol );
            SetFillColor( aFillCol );
        }
        SetTextColor( aTextCol );
    }
}

// -----------------------------------------------------------------------

void SvxShowCharSet::InitSettings( sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        Color aTextColor( rStyleSettings.GetDialogTextColor() );

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }

    Invalidate();
}

// -----------------------------------------------------------------------

sal_UCS4 SvxShowCharSet::GetSelectCharacter() const
{
    if( nSelectedIndex >= 0 )
        getSelectedChar() = maFontCharMap.GetCharFromIndex( nSelectedIndex );
    return getSelectedChar();
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SetFont( const Font& rFont )
{
    // save last selected unicode
    if( nSelectedIndex >= 0 )
        getSelectedChar() = maFontCharMap.GetCharFromIndex( nSelectedIndex );

    Size aSize = GetOutputSizePixel();
    long nSBWidth = aVscrollSB.GetOptimalSize().Width();
    aSize.Width() -= nSBWidth;

    Font aFont = rFont;
    aFont.SetWeight( WEIGHT_LIGHT );
    aFont.SetAlign( ALIGN_TOP );
    int nFontHeight = (aSize.Height() - 5) * 2 / (3 * ROW_COUNT);
    aFont.SetSize( PixelToLogic( Size( 0, nFontHeight ) ) );
    aFont.SetTransparent( sal_True );
    Control::SetFont( aFont );
    GetFontCharMap( maFontCharMap );

    nX = aSize.Width() / COLUMN_COUNT;
    nY = aSize.Height() / ROW_COUNT;

    aVscrollSB.setPosSizePixel( aSize.Width(), 0, nSBWidth, aSize.Height() );
    aVscrollSB.SetRangeMin( 0 );
    int nLastRow = (maFontCharMap.GetCharCount() - 1 + COLUMN_COUNT) / COLUMN_COUNT;
    aVscrollSB.SetRangeMax( nLastRow );
    aVscrollSB.SetPageSize( ROW_COUNT-1 );
    aVscrollSB.SetVisibleSize( ROW_COUNT );

    // restore last selected unicode
    int nMapIndex = maFontCharMap.GetIndexFromChar( getSelectedChar() );
    SelectIndex( nMapIndex );

    aVscrollSB.Show();

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aDrawSize(nX * COLUMN_COUNT, nY * ROW_COUNT);
    m_nXGap = (aSize.Width() - aDrawSize.Width()) / 2;
    m_nYGap = (aSize.Height() - aDrawSize.Height()) / 2;

    Invalidate();
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectIndex( int nNewIndex, sal_Bool bFocus )
{
    if( nNewIndex < 0 )
    {
        // need to scroll see closest unicode
        sal_uInt32 cPrev = maFontCharMap.GetPrevChar( getSelectedChar() );
        int nMapIndex = maFontCharMap.GetIndexFromChar( cPrev );
        int nNewPos = nMapIndex / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( nNewPos );
        nSelectedIndex = bFocus ? nMapIndex+1 : -1;
        Invalidate();
        Update();
    }
    else if( nNewIndex < FirstInView() )
    {
        // need to scroll up to see selected item
        int nOldPos = aVscrollSB.GetThumbPos();
        int nDelta = (FirstInView() - nNewIndex + COLUMN_COUNT-1) / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( nOldPos - nDelta );
        nSelectedIndex = nNewIndex;
        Invalidate();
        if( nDelta )
            Update();
    }
    else if( nNewIndex > LastInView() )
    {
        // need to scroll down to see selected item
        int nOldPos = aVscrollSB.GetThumbPos();
        int nDelta = (nNewIndex - LastInView() + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB.SetThumbPos( nOldPos + nDelta );
        if( nNewIndex < maFontCharMap.GetCharCount() )
        {
            nSelectedIndex = nNewIndex;
            Invalidate();
        }
        if( nOldPos != aVscrollSB.GetThumbPos() )
        {
            Invalidate();
            Update();
        }
    }
    else
    {
        // remove highlighted view
        Color aLineCol = GetLineColor();
        Color aFillCol = GetFillColor();
        SetLineColor();
        SetFillColor( GetBackground().GetColor() );

        Point aOldPixel = MapIndexToPixel( nSelectedIndex );
        aOldPixel.Move( +1, +1);
        Size aOutputSize = GetOutputSizePixel();
        if (aVscrollSB.IsVisible())
            aOutputSize.Width() -= aVscrollSB.GetOptimalSize().Width();
        DrawRect( getGridRectangle(aOldPixel, aOutputSize) );
        SetLineColor( aLineCol );
        SetFillColor( aFillCol );

        int nOldIndex = nSelectedIndex;
        nSelectedIndex = nNewIndex;
        DrawChars_Impl( nOldIndex, nOldIndex );
        DrawChars_Impl( nNewIndex, nNewIndex );
    }

    if( nSelectedIndex >= 0 )
    {
        getSelectedChar() = maFontCharMap.GetCharFromIndex( nSelectedIndex );
        if( m_pAccessible )
        {
            ::svx::SvxShowCharSetItem* pItem = ImplGetItem(nSelectedIndex);
            m_pAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), makeAny(pItem->GetAccessible()) ); // this call asures that m_pItem is set

            OSL_ENSURE(pItem->m_pItem,"No accessible created!");
            Any aOldAny, aNewAny;
            aNewAny <<= AccessibleStateType::FOCUSED;
            pItem->m_pItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );

            aNewAny <<= AccessibleStateType::SELECTED;
            pItem->m_pItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
        }
    }


    aHighHdl.Call( this );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectCharacter( sal_UCS4 cNew, sal_Bool bFocus )
{
    // get next available char of current font
    sal_UCS4 cNext = maFontCharMap.GetNextChar( (cNew > 0) ? cNew - 1 : cNew );

    int nMapIndex = maFontCharMap.GetIndexFromChar( cNext );
    SelectIndex( nMapIndex, bFocus );
    if( !bFocus )
    {
        // move selected item to top row if not in focus
        aVscrollSB.SetThumbPos( nMapIndex / COLUMN_COUNT );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxShowCharSet, VscrollHdl)
{
    if( nSelectedIndex < FirstInView() )
    {
        SelectIndex( FirstInView() + (nSelectedIndex % COLUMN_COUNT) );
    }
    else if( nSelectedIndex > LastInView() )
    {
        if( m_pAccessible )
        {
            ::com::sun::star::uno::Any aOldAny, aNewAny;
            int nLast = LastInView();
            for ( ; nLast != nSelectedIndex; ++nLast)
            {
                aOldAny <<= ImplGetItem(nLast)->GetAccessible();
                m_pAccessible ->fireEvent( AccessibleEventId::CHILD, aOldAny, aNewAny );
            }
        }
        SelectIndex( (LastInView() - COLUMN_COUNT + 1) + (nSelectedIndex % COLUMN_COUNT) );
    }

    Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

SvxShowCharSet::~SvxShowCharSet()
{
    if ( m_pAccessible )
        ReleaseAccessible();
}
// -----------------------------------------------------------------------------
void SvxShowCharSet::ReleaseAccessible()
{
    m_aItems.clear();
    m_pAccessible = NULL;
    m_xAccessible = NULL;
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< XAccessible > SvxShowCharSet::CreateAccessible()
{
    OSL_ENSURE(!m_pAccessible,"Accessible already created!");
    m_pAccessible = new ::svx::SvxShowCharSetVirtualAcc(this);
    m_xAccessible = m_pAccessible;
    return m_xAccessible;
}
// -----------------------------------------------------------------------------
::svx::SvxShowCharSetItem* SvxShowCharSet::ImplGetItem( int _nPos )
{
    ItemsMap::iterator aFind = m_aItems.find(_nPos);
    if ( aFind == m_aItems.end() )
    {
        OSL_ENSURE(m_pAccessible,"Who wants to create a child of my table without a parent?");
        boost::shared_ptr<svx::SvxShowCharSetItem> xItem(new svx::SvxShowCharSetItem(*this,
            m_pAccessible->getTable(), sal::static_int_cast< sal_uInt16 >(_nPos)));
        aFind = m_aItems.insert(ItemsMap::value_type(_nPos, xItem)).first;
        OUStringBuffer buf;
        buf.appendUtf32( maFontCharMap.GetCharFromIndex( _nPos ) );
        aFind->second->maText = buf.makeStringAndClear();
        Point pix = MapIndexToPixel( _nPos );
        aFind->second->maRect = Rectangle( Point( pix.X() + 1, pix.Y() + 1 ), Size(nX-1,nY-1) );
    }

    return aFind->second.get();
}

// -----------------------------------------------------------------------------
ScrollBar* SvxShowCharSet::getScrollBar()
{
    return &aVscrollSB;
}
// -----------------------------------------------------------------------
sal_Int32 SvxShowCharSet::getMaxCharCount() const
{
    return maFontCharMap.GetCharCount();
}


// class SubsetMap =======================================================
// TODO: should be moved into Font Attributes stuff
// we let it mature here though because it is currently the only use

SubsetMap::SubsetMap( const FontCharMap* pFontCharMap )
:   Resource( SVX_RES(RID_SUBSETMAP) )
{
    InitList();
    ApplyCharMap( pFontCharMap );
    FreeResource();
}

const Subset* SubsetMap::GetNextSubset( bool bFirst ) const
{
    if( bFirst )
        maSubsetIterator = maSubsets.begin();
    if( maSubsetIterator == maSubsets.end() )
        return NULL;
    const Subset* s = &*(maSubsetIterator++);
    return s;
}

const Subset* SubsetMap::GetSubsetByUnicode( sal_UCS4 cChar ) const
{
    // TODO: is it worth to avoid a linear search?
    for( const Subset* s = GetNextSubset( true ); s; s = GetNextSubset( false ) )
        if( (s->GetRangeMin() <= cChar) && (cChar <= s->GetRangeMax()) )
            return s;
    return NULL;
}

inline Subset::Subset( sal_UCS4 nMin, sal_UCS4 nMax, int resId)
:   mnRangeMin(nMin), mnRangeMax(nMax), maRangeName( SVX_RESSTR(resId) )
{}

void SubsetMap::InitList()
{
    static SubsetList aAllSubsets;
    static bool bInit = true;
    if( bInit )
    {
        bInit = false;
        //I wish icu had a way to give me the block ranges
        for (int i = UBLOCK_BASIC_LATIN; i < UBLOCK_COUNT; ++i)
        {
            UBlockCode eBlock = static_cast<UBlockCode>(i);
            switch (eBlock)
            {
                case UBLOCK_NO_BLOCK:
                case UBLOCK_INVALID_CODE:
                case UBLOCK_COUNT:
                case UBLOCK_HIGH_SURROGATES:
                case UBLOCK_HIGH_PRIVATE_USE_SURROGATES:
                case UBLOCK_LOW_SURROGATES:
                    break;
                case UBLOCK_BASIC_LATIN:
                    aAllSubsets.push_back( Subset( 0x0000, 0x007F, RID_SUBSETSTR_BASIC_LATIN ) );
                    break;
                case UBLOCK_LATIN_1_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x0080, 0x00FF, RID_SUBSETSTR_LATIN_1 ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0x0100, 0x017F, RID_SUBSETSTR_LATIN_EXTENDED_A ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0x0180, 0x024F, RID_SUBSETSTR_LATIN_EXTENDED_B ) );
                    break;
                case UBLOCK_IPA_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x0250, 0x02AF, RID_SUBSETSTR_IPA_EXTENSIONS ) );
                    break;
                case UBLOCK_SPACING_MODIFIER_LETTERS:
                    aAllSubsets.push_back( Subset( 0x02B0, 0x02FF, RID_SUBSETSTR_SPACING_MODIFIERS ) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS:
                    aAllSubsets.push_back( Subset( 0x0300, 0x036F, RID_SUBSETSTR_COMB_DIACRITICAL ) );
                    break;
                case UBLOCK_GREEK:
                    aAllSubsets.push_back( Subset( 0x0370, 0x03FF, RID_SUBSETSTR_BASIC_GREEK ) );
                    break;
                case UBLOCK_CYRILLIC:
                    aAllSubsets.push_back( Subset( 0x0400, 0x04FF, RID_SUBSETSTR_CYRILLIC ) );
                    break;
                case UBLOCK_ARMENIAN:
                    aAllSubsets.push_back( Subset( 0x0530, 0x058F, RID_SUBSETSTR_ARMENIAN ) );
                    break;
                case UBLOCK_HEBREW:
                    aAllSubsets.push_back( Subset( 0x0590, 0x05FF, RID_SUBSETSTR_BASIC_HEBREW ) );
                    break;
                case UBLOCK_ARABIC:
                    aAllSubsets.push_back( Subset( 0x0600, 0x065F, RID_SUBSETSTR_BASIC_ARABIC ) );
                    break;
                case UBLOCK_SYRIAC:
                    aAllSubsets.push_back( Subset( 0x0700, 0x074F, RID_SUBSETSTR_SYRIAC ) );
                    break;
                case UBLOCK_THAANA:
                    aAllSubsets.push_back( Subset( 0x0780, 0x07BF, RID_SUBSETSTR_THAANA ) );
                    break;
                case UBLOCK_DEVANAGARI:
                    aAllSubsets.push_back( Subset( 0x0900, 0x097F, RID_SUBSETSTR_DEVANAGARI ) );
                    break;
                case UBLOCK_BENGALI:
                    aAllSubsets.push_back( Subset( 0x0980, 0x09FF, RID_SUBSETSTR_BENGALI ) );
                    break;
                case UBLOCK_GURMUKHI:
                    aAllSubsets.push_back( Subset( 0x0A00, 0x0A7F, RID_SUBSETSTR_GURMUKHI ) );
                    break;
                case UBLOCK_GUJARATI:
                    aAllSubsets.push_back( Subset( 0x0A80, 0x0AFF, RID_SUBSETSTR_GUJARATI ) );
                    break;
                case UBLOCK_ORIYA:
                    aAllSubsets.push_back( Subset( 0x0B00, 0x0B7F, RID_SUBSETSTR_ORIYA ) );
                    break;
                case UBLOCK_TAMIL:
                    aAllSubsets.push_back( Subset( 0x0B80, 0x0BFF, RID_SUBSETSTR_TAMIL ) );
                    break;
                case UBLOCK_TELUGU:
                    aAllSubsets.push_back( Subset( 0x0C00, 0x0C7F, RID_SUBSETSTR_TELUGU ) );
                    break;
                case UBLOCK_KANNADA:
                    aAllSubsets.push_back( Subset( 0x0C80, 0x0CFF, RID_SUBSETSTR_KANNADA ) );
                    break;
                case UBLOCK_MALAYALAM:
                    aAllSubsets.push_back( Subset( 0x0D00, 0x0D7F, RID_SUBSETSTR_MALAYALAM ) );
                    break;
                case UBLOCK_SINHALA:
                    aAllSubsets.push_back( Subset( 0x0D80, 0x0DFF, RID_SUBSETSTR_SINHALA ) );
                    break;
                case UBLOCK_THAI:
                    aAllSubsets.push_back( Subset( 0x0E00, 0x0E7F, RID_SUBSETSTR_THAI ) );
                    break;
                case UBLOCK_LAO:
                    aAllSubsets.push_back( Subset( 0x0E80, 0x0EFF, RID_SUBSETSTR_LAO ) );
                    break;
                case UBLOCK_TIBETAN:
                    aAllSubsets.push_back( Subset( 0x0F00, 0x0FBF, RID_SUBSETSTR_TIBETAN ) );
                    break;
                case UBLOCK_MYANMAR:
                    aAllSubsets.push_back( Subset( 0x1000, 0x109F, RID_SUBSETSTR_MYANMAR ) );
                    break;
                case UBLOCK_GEORGIAN:
                    aAllSubsets.push_back( Subset( 0x10A0, 0x10FF, RID_SUBSETSTR_BASIC_GEORGIAN ) );
                    break;
                case UBLOCK_HANGUL_JAMO:
                    aAllSubsets.push_back( Subset( 0x1100, 0x11FF, RID_SUBSETSTR_HANGUL_JAMO ) );
                    break;
                case UBLOCK_ETHIOPIC:
                    aAllSubsets.push_back( Subset( 0x1200, 0x137F, RID_SUBSETSTR_ETHIOPIC ) );
                    break;
                case UBLOCK_CHEROKEE:
                    aAllSubsets.push_back( Subset( 0x13A0, 0x13FF, RID_SUBSETSTR_CHEROKEE ) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS:
                    aAllSubsets.push_back( Subset( 0x1400, 0x167F, RID_SUBSETSTR_CANADIAN_ABORIGINAL ) );
                    break;
                case UBLOCK_OGHAM:
                    aAllSubsets.push_back( Subset( 0x1680, 0x169F, RID_SUBSETSTR_OGHAM ) );
                    break;
                case UBLOCK_RUNIC:
                    aAllSubsets.push_back( Subset( 0x16A0, 0x16F0, RID_SUBSETSTR_RUNIC ) );
                    break;
                case UBLOCK_KHMER:
                    aAllSubsets.push_back( Subset( 0x1780, 0x17FF, RID_SUBSETSTR_KHMER ) );
                    break;
                case UBLOCK_MONGOLIAN:
                    aAllSubsets.push_back( Subset( 0x1800, 0x18AF, RID_SUBSETSTR_MONGOLIAN ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_ADDITIONAL:
                    aAllSubsets.push_back( Subset( 0x1E00, 0x1EFF, RID_SUBSETSTR_LATIN_EXTENDED_ADDS ) );
                    break;
                case UBLOCK_GREEK_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x1F00, 0x1FFF, RID_SUBSETSTR_GREEK_EXTENDED ) );
                    break;
                case UBLOCK_GENERAL_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x2000, 0x206F, RID_SUBSETSTR_GENERAL_PUNCTUATION ) );
                    break;
                case UBLOCK_SUPERSCRIPTS_AND_SUBSCRIPTS:
                    aAllSubsets.push_back( Subset( 0x2070, 0x209F, RID_SUBSETSTR_SUB_SUPER_SCRIPTS ) );
                    break;
                case UBLOCK_CURRENCY_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x20A0, 0x20CF, RID_SUBSETSTR_CURRENCY_SYMBOLS ) );
                    break;
                case UBLOCK_COMBINING_MARKS_FOR_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x20D0, 0x20FF, RID_SUBSETSTR_COMB_DIACRITIC_SYMS ) );
                    break;
                case UBLOCK_LETTERLIKE_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x2100, 0x214F, RID_SUBSETSTR_LETTERLIKE_SYMBOLS ) );
                    break;
                case UBLOCK_NUMBER_FORMS:
                    aAllSubsets.push_back( Subset( 0x2150, 0x218F, RID_SUBSETSTR_NUMBER_FORMS ) );
                    break;
                case UBLOCK_ARROWS:
                    aAllSubsets.push_back( Subset( 0x2190, 0x21FF, RID_SUBSETSTR_ARROWS ) );
                    break;
                case UBLOCK_MATHEMATICAL_OPERATORS:
                    aAllSubsets.push_back( Subset( 0x2200, 0x22FF, RID_SUBSETSTR_MATH_OPERATORS ) );
                    break;
                case UBLOCK_MISCELLANEOUS_TECHNICAL:
                    aAllSubsets.push_back( Subset( 0x2300, 0x23FF, RID_SUBSETSTR_MISC_TECHNICAL ) );
                    break;
                case UBLOCK_CONTROL_PICTURES:
                    aAllSubsets.push_back( Subset( 0x2400, 0x243F, RID_SUBSETSTR_CONTROL_PICTURES ) );
                    break;
                case UBLOCK_OPTICAL_CHARACTER_RECOGNITION:
                    aAllSubsets.push_back( Subset( 0x2440, 0x245F, RID_SUBSETSTR_OPTICAL_CHAR_REC ) );
                    break;
                case UBLOCK_ENCLOSED_ALPHANUMERICS:
                    aAllSubsets.push_back( Subset( 0x2460, 0x24FF, RID_SUBSETSTR_ENCLOSED_ALPHANUM ) );
                    break;
                case UBLOCK_BOX_DRAWING:
                    aAllSubsets.push_back( Subset( 0x2500, 0x257F, RID_SUBSETSTR_BOX_DRAWING ) );
                    break;
                case UBLOCK_BLOCK_ELEMENTS:
                    aAllSubsets.push_back( Subset( 0x2580, 0x259F, RID_SUBSETSTR_BLOCK_ELEMENTS ) );
                    break;
                case UBLOCK_GEOMETRIC_SHAPES:
                    aAllSubsets.push_back( Subset( 0x25A0, 0x25FF, RID_SUBSETSTR_GEOMETRIC_SHAPES ) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x2600, 0x26FF, RID_SUBSETSTR_MISC_DINGBATS ) );
                    break;
                case UBLOCK_DINGBATS:
                    aAllSubsets.push_back( Subset( 0x2700, 0x27BF, RID_SUBSETSTR_DINGBATS ) );
                    break;
                case UBLOCK_BRAILLE_PATTERNS:
                    aAllSubsets.push_back( Subset( 0x2800, 0x28FF, RID_SUBSETSTR_BRAILLE_PATTERNS ) );
                    break;
                case UBLOCK_CJK_RADICALS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x2E80, 0x2EFF, RID_SUBSETSTR_CJK_RADICAL_SUPPL ) );
                    break;
                case UBLOCK_KANGXI_RADICALS:
                    aAllSubsets.push_back( Subset( 0x2F00, 0x2FDF, RID_SUBSETSTR_KANGXI_RADICALS ) );
                    break;
                case UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS:
                    aAllSubsets.push_back( Subset( 0x2FF0, 0x2FFF, RID_SUBSETSTR_IDEO_DESC_CHARS ) );
                    break;
                case UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x3000, 0x303F, RID_SUBSETSTR_CJK_SYMS_PUNCTUATION ) );
                    break;
                case UBLOCK_HIRAGANA:
                    aAllSubsets.push_back( Subset( 0x3040, 0x309F, RID_SUBSETSTR_HIRAGANA ) );
                    break;
                case UBLOCK_KATAKANA:
                    aAllSubsets.push_back( Subset( 0x30A0, 0x30FF, RID_SUBSETSTR_KATAKANA ) );
                    break;
                case UBLOCK_BOPOMOFO:
                    aAllSubsets.push_back( Subset( 0x3100, 0x312F, RID_SUBSETSTR_BOPOMOFO ) );
                    break;
                case UBLOCK_HANGUL_COMPATIBILITY_JAMO:
                    aAllSubsets.push_back( Subset( 0x3130, 0x318F, RID_SUBSETSTR_HANGUL_COMPAT_JAMO ) );
                    break;
                case UBLOCK_KANBUN:
                    aAllSubsets.push_back( Subset( 0x3190, 0x319F, RID_SUBSETSTR_KANBUN ) );
                    break;
                case UBLOCK_BOPOMOFO_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x31A0, 0x31BF, RID_SUBSETSTR_BOPOMOFO_EXTENDED ) );
                    break;
                case UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS:
                    aAllSubsets.push_back( Subset( 0x3200, 0x32FF, RID_SUBSETSTR_ENCLOSED_CJK_LETTERS ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY:
                    aAllSubsets.push_back( Subset( 0x3300, 0x33FF, RID_SUBSETSTR_CJK_COMPATIBILITY ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A:
                    aAllSubsets.push_back( Subset( 0x3400, 0x4DBF, RID_SUBSETSTR_CJK_EXT_A_UNIFIED_IDGRAPH ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS:
                    aAllSubsets.push_back( Subset( 0x4E00, 0x9FA5, RID_SUBSETSTR_CJK_UNIFIED_IDGRAPH ) );
                    break;
                case UBLOCK_YI_SYLLABLES:
                    aAllSubsets.push_back( Subset( 0xA000, 0xA48F, RID_SUBSETSTR_YI_SYLLABLES ) );
                    break;
                case UBLOCK_YI_RADICALS:
                    aAllSubsets.push_back( Subset( 0xA490, 0xA4CF, RID_SUBSETSTR_YI_RADICALS ) );
                    break;
                case UBLOCK_HANGUL_SYLLABLES:
                    aAllSubsets.push_back( Subset( 0xAC00, 0xD7AF, RID_SUBSETSTR_HANGUL ) );
                    break;
                case UBLOCK_PRIVATE_USE_AREA:
                    aAllSubsets.push_back( Subset( 0xE000, 0xF8FF, RID_SUBSETSTR_PRIVATE_USE_AREA ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS:
                    aAllSubsets.push_back( Subset( 0xF900, 0xFAFF, RID_SUBSETSTR_CJK_COMPAT_IDGRAPHS ) );
                    break;
                case UBLOCK_ALPHABETIC_PRESENTATION_FORMS:
                    aAllSubsets.push_back( Subset( 0xFB00, 0xFB4F, RID_SUBSETSTR_ALPHA_PRESENTATION ) );
                    break;
                case UBLOCK_ARABIC_PRESENTATION_FORMS_A:
                    aAllSubsets.push_back( Subset( 0xFB50, 0xFDFF, RID_SUBSETSTR_ARABIC_PRESENT_A ) );
                    break;
                case UBLOCK_COMBINING_HALF_MARKS:
                    aAllSubsets.push_back( Subset( 0xFE20, 0xFE2F, RID_SUBSETSTR_COMBINING_HALF_MARKS ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_FORMS:
                    aAllSubsets.push_back( Subset( 0xFE30, 0xFE4F, RID_SUBSETSTR_CJK_COMPAT_FORMS ) );
                    break;
                case UBLOCK_SMALL_FORM_VARIANTS:
                    aAllSubsets.push_back( Subset( 0xFE50, 0xFE6F, RID_SUBSETSTR_SMALL_FORM_VARIANTS ) );
                    break;
                case UBLOCK_ARABIC_PRESENTATION_FORMS_B:
                    aAllSubsets.push_back( Subset( 0xFE70, 0xFEFF, RID_SUBSETSTR_ARABIC_PRESENT_B ) );
                    break;
                case UBLOCK_SPECIALS:
                    aAllSubsets.push_back( Subset( 0xFFF0, 0xFFFF, RID_SUBSETSTR_SPECIALS ) );
                    break;
                case UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS:
                    aAllSubsets.push_back( Subset( 0xFF00, 0xFFEF, RID_SUBSETSTR_HALFW_FULLW_FORMS ) );
                    break;
                case UBLOCK_OLD_ITALIC:
                    aAllSubsets.push_back( Subset( 0x10300, 0x1032F, RID_SUBSETSTR_OLD_ITALIC ) );
                    break;
                case UBLOCK_GOTHIC:
                    aAllSubsets.push_back( Subset( 0x10330, 0x1034F, RID_SUBSETSTR_GOTHIC ) );
                    break;
                case UBLOCK_DESERET:
                    aAllSubsets.push_back( Subset( 0x10400, 0x1044F, RID_SUBSETSTR_DESERET ) );
                    break;
                case UBLOCK_BYZANTINE_MUSICAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D000, 0x1D0FF, RID_SUBSETSTR_BYZANTINE_MUSICAL_SYMBOLS ) );
                    break;
                case UBLOCK_MUSICAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D100, 0x1D1FF, RID_SUBSETSTR_MUSICAL_SYMBOLS ) );
                    break;
                case UBLOCK_MATHEMATICAL_ALPHANUMERIC_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D400, 0x1D7FF, RID_SUBSETSTR_MATHEMATICAL_ALPHANUMERIC_SYMBOLS ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B:
                    aAllSubsets.push_back( Subset( 0x20000, 0x2A6DF, RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x2F800, 0x2FA1F, RID_SUBSETSTR_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT ) );
                    break;
                case UBLOCK_TAGS:
                    aAllSubsets.push_back( Subset( 0xE0000, 0xE007F, RID_SUBSETSTR_TAGS ) );
                    break;
                case UBLOCK_CYRILLIC_SUPPLEMENTARY:
                    aAllSubsets.push_back( Subset( 0x0500, 0x052F, RID_SUBSETSTR_CYRILLIC_SUPPLEMENTARY ) );
                    break;
                case UBLOCK_TAGALOG:
                    aAllSubsets.push_back( Subset( 0x1700, 0x171F, RID_SUBSETSTR_TAGALOG ) );
                    break;
                case UBLOCK_HANUNOO:
                    aAllSubsets.push_back( Subset( 0x1720, 0x173F, RID_SUBSETSTR_HANUNOO ) );
                    break;
                case UBLOCK_BUHID:
                    aAllSubsets.push_back( Subset( 0x1740, 0x175F, RID_SUBSETSTR_BUHID ) );
                    break;
                case UBLOCK_TAGBANWA:
                    aAllSubsets.push_back( Subset( 0x1760, 0x177F, RID_SUBSETSTR_TAGBANWA ) );
                    break;
                case UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A:
                    aAllSubsets.push_back( Subset( 0x27C0, 0x27EF, RID_SUBSETSTR_MISC_MATH_SYMS_A ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_A:
                    aAllSubsets.push_back( Subset( 0x27F0, 0x27FF, RID_SUBSETSTR_SUPPL_ARROWS_A ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_B:
                    aAllSubsets.push_back( Subset( 0x2900, 0x297F, RID_SUBSETSTR_SUPPL_ARROWS_B ) );
                    break;
                case UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B:
                    aAllSubsets.push_back( Subset( 0x2980, 0x29FF, RID_SUBSETSTR_MISC_MATH_SYMS_B ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_MATHEMATICAL_OPERATORS:
                    aAllSubsets.push_back( Subset( 0x2A00, 0x2AFF, RID_SUBSETSTR_MISC_MATH_SYMS_B ) );
                    break;
                case UBLOCK_KATAKANA_PHONETIC_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x31F0, 0x31FF, RID_SUBSETSTR_KATAKANA_PHONETIC ) );
                    break;
                case UBLOCK_VARIATION_SELECTORS:
                    aAllSubsets.push_back( Subset( 0xFE00, 0xFE0F, RID_SUBSETSTR_VARIATION_SELECTORS ) );
                    break;
                case UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A:
                    aAllSubsets.push_back( Subset( 0xF0000, 0xFFFFF, RID_SUBSETSTR_SUPPLEMENTARY_PRIVATE_USE_AREA_A ) );
                    break;
                case UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B:
                    aAllSubsets.push_back( Subset( 0x100000, 0x10FFFF, RID_SUBSETSTR_SUPPLEMENTARY_PRIVATE_USE_AREA_B ) );
                    break;
                case UBLOCK_LIMBU:
                    aAllSubsets.push_back( Subset( 0x1900, 0x194F, RID_SUBSETSTR_LIMBU ) );
                    break;
                case UBLOCK_TAI_LE:
                    aAllSubsets.push_back( Subset( 0x1950, 0x197F, RID_SUBSETSTR_TAI_LE ) );
                    break;
                case UBLOCK_KHMER_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x19E0, 0x19FF, RID_SUBSETSTR_KHMER_SYMBOLS ) );
                    break;
                case UBLOCK_PHONETIC_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x1D00, 0x1D7F, RID_SUBSETSTR_PHONETIC_EXTENSIONS ) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_ARROWS:
                    aAllSubsets.push_back( Subset( 0x2B00, 0x2BFF, RID_SUBSETSTR_MISCELLANEOUS_SYMBOLS_AND_ARROWS ) );
                    break;
                case UBLOCK_YIJING_HEXAGRAM_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x4DC0, 0x4DFF, RID_SUBSETSTR_YIJING_HEXAGRAM_SYMBOLS ) );
                    break;
                case UBLOCK_LINEAR_B_SYLLABARY:
                    aAllSubsets.push_back( Subset( 0x10000, 0x1007F, RID_SUBSETSTR_LINEAR_B_SYLLABARY ) );
                    break;
                case UBLOCK_LINEAR_B_IDEOGRAMS:
                    aAllSubsets.push_back( Subset( 0x10080, 0x100FF, RID_SUBSETSTR_LINEAR_B_IDEOGRAMS ) );
                    break;
                case UBLOCK_AEGEAN_NUMBERS:
                    aAllSubsets.push_back( Subset( 0x10100, 0x1013F, RID_SUBSETSTR_AEGEAN_NUMBERS ) );
                    break;
                case UBLOCK_UGARITIC:
                    aAllSubsets.push_back( Subset( 0x10380, 0x1039F, RID_SUBSETSTR_UGARITIC ) );
                    break;
                case UBLOCK_SHAVIAN:
                    aAllSubsets.push_back( Subset( 0x10450, 0x1047F, RID_SUBSETSTR_SHAVIAN ) );
                    break;
                case UBLOCK_OSMANYA:
                    aAllSubsets.push_back( Subset( 0x10480, 0x104AF, RID_SUBSETSTR_OSMANYA ) );
                    break;
                case UBLOCK_CYPRIOT_SYLLABARY:
                    aAllSubsets.push_back( Subset( 0x10800, 0x1083F, RID_SUBSETSTR_CYPRIOT_SYLLABARY ) );
                    break;
                case UBLOCK_TAI_XUAN_JING_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D300, 0x1D35F, RID_SUBSETSTR_TAI_XUAN_JING_SYMBOLS ) );
                    break;
                case UBLOCK_VARIATION_SELECTORS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0xE0100, 0xE01EF, RID_SUBSETSTR_VARIATION_SELECTORS_SUPPLEMENT ) );
                    break;
                case UBLOCK_ANCIENT_GREEK_MUSICAL_NOTATION:
                    aAllSubsets.push_back( Subset(0x1D200, 0x1D24F, RID_SUBSETSTR_ANCIENT_GREEK_MUSICAL_NOTATION ) );
                    break;
                case UBLOCK_ANCIENT_GREEK_NUMBERS:
                    aAllSubsets.push_back( Subset(0x10140, 0x1018F , RID_SUBSETSTR_ANCIENT_GREEK_NUMBERS ) );
                    break;
                case UBLOCK_ARABIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset(0x0750, 0x077F , RID_SUBSETSTR_ARABIC_SUPPLEMENT ) );
                    break;
                case UBLOCK_BUGINESE:
                    aAllSubsets.push_back( Subset(0x1A00, 0x1A1F , RID_SUBSETSTR_BUGINESE ) );
                    break;
                case UBLOCK_CJK_STROKES:
                    aAllSubsets.push_back( Subset( 0x31C0, 0x31EF, RID_SUBSETSTR_CJK_STROKES ) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1DC0, 0x1DFF , RID_SUBSETSTR_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT ) );
                    break;
                case UBLOCK_COPTIC:
                    aAllSubsets.push_back( Subset( 0x2C80, 0x2CFF , RID_SUBSETSTR_COPTIC ) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x2D80, 0x2DDF , RID_SUBSETSTR_ETHIOPIC_EXTENDED ) );
                    break;
                case UBLOCK_ETHIOPIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1380, 0x139F, RID_SUBSETSTR_ETHIOPIC_SUPPLEMENT ) );
                    break;
                case UBLOCK_GEORGIAN_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x2D00, 0x2D2F, RID_SUBSETSTR_GEORGIAN_SUPPLEMENT ) );
                    break;
                case UBLOCK_GLAGOLITIC:
                    aAllSubsets.push_back( Subset( 0x2C00, 0x2C5F, RID_SUBSETSTR_GLAGOLITIC ) );
                    break;
                case UBLOCK_KHAROSHTHI:
                    aAllSubsets.push_back( Subset( 0x10A00, 0x10A5F, RID_SUBSETSTR_KHAROSHTHI ) );
                    break;
                case UBLOCK_MODIFIER_TONE_LETTERS:
                    aAllSubsets.push_back( Subset( 0xA700, 0xA71F, RID_SUBSETSTR_MODIFIER_TONE_LETTERS ) );
                    break;
                case UBLOCK_NEW_TAI_LUE:
                    aAllSubsets.push_back( Subset( 0x1980, 0x19DF, RID_SUBSETSTR_NEW_TAI_LUE ) );
                    break;
                case UBLOCK_OLD_PERSIAN:
                    aAllSubsets.push_back( Subset( 0x103A0, 0x103DF, RID_SUBSETSTR_OLD_PERSIAN ) );
                    break;
                case UBLOCK_PHONETIC_EXTENSIONS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1D80, 0x1DBF, RID_SUBSETSTR_PHONETIC_EXTENSIONS_SUPPLEMENT ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x2E00, 0x2E7F, RID_SUBSETSTR_SUPPLEMENTAL_PUNCTUATION ) );
                    break;
                case UBLOCK_SYLOTI_NAGRI:
                    aAllSubsets.push_back( Subset( 0xA800, 0xA82F, RID_SUBSETSTR_SYLOTI_NAGRI ) );
                    break;
                case UBLOCK_TIFINAGH:
                    aAllSubsets.push_back( Subset( 0x2D30, 0x2D7F, RID_SUBSETSTR_TIFINAGH ) );
                    break;
                case UBLOCK_VERTICAL_FORMS:
                    aAllSubsets.push_back( Subset( 0xFE10, 0xFE1F, RID_SUBSETSTR_VERTICAL_FORMS ) );
                    break;
                case UBLOCK_NKO:
                    aAllSubsets.push_back( Subset( 0x07C0, 0x07FF, RID_SUBSETSTR_NKO ) );
                    break;
                case UBLOCK_BALINESE:
                    aAllSubsets.push_back( Subset( 0x1B00, 0x1B7F, RID_SUBSETSTR_BALINESE ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_C:
                    aAllSubsets.push_back( Subset( 0x2C60, 0x2C7F, RID_SUBSETSTR_LATIN_EXTENDED_C ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_D:
                    aAllSubsets.push_back( Subset( 0xA720, 0xA7FF, RID_SUBSETSTR_LATIN_EXTENDED_D ) );
                    break;
                case UBLOCK_PHAGS_PA:
                    aAllSubsets.push_back( Subset( 0xA840, 0xA87F, RID_SUBSETSTR_PHAGS_PA ) );
                    break;
                case UBLOCK_PHOENICIAN:
                    aAllSubsets.push_back( Subset( 0x10900, 0x1091F, RID_SUBSETSTR_PHOENICIAN ) );
                    break;
                case UBLOCK_CUNEIFORM:
                    aAllSubsets.push_back( Subset( 0x12000, 0x123FF, RID_SUBSETSTR_CUNEIFORM ) );
                    break;
                case UBLOCK_CUNEIFORM_NUMBERS_AND_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x12400, 0x1247F, RID_SUBSETSTR_CUNEIFORM_NUMBERS_AND_PUNCTUATION ) );
                    break;
                case UBLOCK_COUNTING_ROD_NUMERALS:
                    aAllSubsets.push_back( Subset( 0x1D360, 0x1D37F, RID_SUBSETSTR_COUNTING_ROD_NUMERALS ) );
                    break;
                case UBLOCK_SUNDANESE:
                    aAllSubsets.push_back( Subset( 0x1B80, 0x1BBF, RID_SUBSETSTR_SUNDANESE ) );
                    break;
                case UBLOCK_LEPCHA:
                    aAllSubsets.push_back( Subset( 0x1C00, 0x1C4F, RID_SUBSETSTR_LEPCHA ) );
                    break;
                case UBLOCK_OL_CHIKI:
                    aAllSubsets.push_back( Subset( 0x1C50, 0x1C7F, RID_SUBSETSTR_OL_CHIKI ) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0x2DE0, 0x2DFF, RID_SUBSETSTR_CYRILLIC_EXTENDED_A ) );
                    break;
                case UBLOCK_VAI:
                    aAllSubsets.push_back( Subset( 0xA500, 0xA63F, RID_SUBSETSTR_VAI ) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0xA640, 0xA69F, RID_SUBSETSTR_CYRILLIC_EXTENDED_B ) );
                    break;
                case UBLOCK_SAURASHTRA:
                    aAllSubsets.push_back( Subset( 0xA880, 0xA8DF, RID_SUBSETSTR_SAURASHTRA ) );
                    break;
                case UBLOCK_KAYAH_LI:
                    aAllSubsets.push_back( Subset( 0xA900, 0xA92F, RID_SUBSETSTR_KAYAH_LI ) );
                    break;
                case UBLOCK_REJANG:
                    aAllSubsets.push_back( Subset( 0xA930, 0xA95F, RID_SUBSETSTR_REJANG ) );
                    break;
                case UBLOCK_CHAM:
                    aAllSubsets.push_back( Subset( 0xAA00, 0xAA5F, RID_SUBSETSTR_CHAM ) );
                    break;
                case UBLOCK_ANCIENT_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x10190, 0x101CF, RID_SUBSETSTR_ANCIENT_SYMBOLS ) );
                    break;
                case UBLOCK_PHAISTOS_DISC:
                    aAllSubsets.push_back( Subset( 0x101D0, 0x101FF, RID_SUBSETSTR_PHAISTOS_DISC ) );
                    break;
                case UBLOCK_LYCIAN:
                    aAllSubsets.push_back( Subset( 0x10280, 0x1029F, RID_SUBSETSTR_LYCIAN ) );
                    break;
                case UBLOCK_CARIAN:
                    aAllSubsets.push_back( Subset( 0x102A0, 0x102DF, RID_SUBSETSTR_CARIAN ) );
                    break;
                case UBLOCK_LYDIAN:
                    aAllSubsets.push_back( Subset( 0x10920, 0x1093F, RID_SUBSETSTR_LYDIAN ) );
                    break;
                case UBLOCK_MAHJONG_TILES:
                    aAllSubsets.push_back( Subset( 0x1F000, 0x1F02F, RID_SUBSETSTR_MAHJONG_TILES ) );
                    break;
                case UBLOCK_DOMINO_TILES:
                    aAllSubsets.push_back( Subset( 0x1F030, 0x1F09F, RID_SUBSETSTR_DOMINO_TILES ) );
                    break;
#if (U_ICU_VERSION_MAJOR_NUM > 4) || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 4)
                case UBLOCK_SAMARITAN:
                    aAllSubsets.push_back( Subset( 0x0800, 0x083F, RID_SUBSETSTR_SAMARITAN ) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x18B0, 0x18FF, RID_SUBSETSTR_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED ) );
                    break;
                case UBLOCK_TAI_THAM:
                    aAllSubsets.push_back( Subset( 0x1A20, 0x1AAF, RID_SUBSETSTR_TAI_THAM ) );
                    break;
                case UBLOCK_VEDIC_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x1CD0, 0x1CFF, RID_SUBSETSTR_VEDIC_EXTENSIONS ) );
                    break;
                case UBLOCK_LISU:
                    aAllSubsets.push_back( Subset( 0xA4D0, 0xA4FF, RID_SUBSETSTR_LISU ) );
                    break;
                case UBLOCK_BAMUM:
                    aAllSubsets.push_back( Subset( 0xA6A0, 0xA6FF, RID_SUBSETSTR_BAMUM ) );
                    break;
                case UBLOCK_COMMON_INDIC_NUMBER_FORMS:
                    aAllSubsets.push_back( Subset( 0xA830, 0xA83F, RID_SUBSETSTR_COMMON_INDIC_NUMBER_FORMS ) );
                    break;
                case UBLOCK_DEVANAGARI_EXTENDED:
                    aAllSubsets.push_back( Subset( 0xA8E0, 0xA8FF, RID_SUBSETSTR_DEVANAGARI_EXTENDED ) );
                    break;
                case UBLOCK_HANGUL_JAMO_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0xA960, 0xA97F, RID_SUBSETSTR_HANGUL_JAMO_EXTENDED_A ) );
                    break;
                case UBLOCK_JAVANESE:
                    aAllSubsets.push_back( Subset( 0xA980, 0xA9DF, RID_SUBSETSTR_JAVANESE ) );
                    break;
                case UBLOCK_MYANMAR_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0xAA60, 0xAA7F, RID_SUBSETSTR_MYANMAR_EXTENDED_A ) );
                    break;
                case UBLOCK_TAI_VIET:
                    aAllSubsets.push_back( Subset( 0xAA80, 0xAADF, RID_SUBSETSTR_TAI_VIET ) );
                    break;
                case UBLOCK_MEETEI_MAYEK:
                    aAllSubsets.push_back( Subset( 0xABC0, 0xABFF, RID_SUBSETSTR_MEETEI_MAYEK ) );
                    break;
                case UBLOCK_HANGUL_JAMO_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0xD7B0, 0xD7FF, RID_SUBSETSTR_HANGUL_JAMO_EXTENDED_B ) );
                    break;
                case UBLOCK_IMPERIAL_ARAMAIC:
                    aAllSubsets.push_back( Subset( 0x10840, 0x1085F, RID_SUBSETSTR_IMPERIAL_ARAMAIC ) );
                    break;
                case UBLOCK_OLD_SOUTH_ARABIAN:
                    aAllSubsets.push_back( Subset( 0x10A60, 0x10A7F, RID_SUBSETSTR_OLD_SOUTH_ARABIAN ) );
                    break;
                case UBLOCK_AVESTAN:
                    aAllSubsets.push_back( Subset( 0x10B00, 0x10B3F, RID_SUBSETSTR_AVESTAN ) );
                    break;
                case UBLOCK_INSCRIPTIONAL_PARTHIAN:
                    aAllSubsets.push_back( Subset( 0x10B40, 0x10B5F, RID_SUBSETSTR_INSCRIPTIONAL_PARTHIAN ) );
                    break;
                case UBLOCK_INSCRIPTIONAL_PAHLAVI:
                    aAllSubsets.push_back( Subset( 0x10B60, 0x10B7F, RID_SUBSETSTR_INSCRIPTIONAL_PAHLAVI ) );
                    break;
                case UBLOCK_OLD_TURKIC:
                    aAllSubsets.push_back( Subset( 0x10C00, 0x10C4F, RID_SUBSETSTR_OLD_TURKIC ) );
                    break;
                case UBLOCK_RUMI_NUMERAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x10E60, 0x10E7F, RID_SUBSETSTR_RUMI_NUMERAL_SYMBOLS ) );
                    break;
                case UBLOCK_KAITHI:
                    aAllSubsets.push_back( Subset( 0x11080, 0x110CF, RID_SUBSETSTR_KAITHI ) );
                    break;
                case UBLOCK_EGYPTIAN_HIEROGLYPHS:
                    aAllSubsets.push_back( Subset( 0x13000, 0x1342F, RID_SUBSETSTR_EGYPTIAN_HIEROGLYPHS ) );
                    break;
                case UBLOCK_ENCLOSED_ALPHANUMERIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1F100, 0x1F1FF, RID_SUBSETSTR_ENCLOSED_ALPHANUMERIC_SUPPLEMENT ) );
                    break;
                case UBLOCK_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1F200, 0x1F2FF, RID_SUBSETSTR_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C:
                    aAllSubsets.push_back( Subset( 0x2A700, 0x2B73F, RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C ) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM > 4) || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 6)
                case UBLOCK_MANDAIC:
                    aAllSubsets.push_back( Subset( 0x0840, 0x085F, RID_SUBSETSTR_MANDAIC ) );
                    break;
                case UBLOCK_BATAK:
                    aAllSubsets.push_back( Subset( 0x1BC0, 0x1BFF, RID_SUBSETSTR_BATAK ) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0xAB00, 0xAB2F, RID_SUBSETSTR_ETHIOPIC_EXTENDED_A ) );
                    break;
                case UBLOCK_BRAHMI:
                    aAllSubsets.push_back( Subset( 0x11000, 0x1107F, RID_SUBSETSTR_BRAHMI ) );
                    break;
                case UBLOCK_BAMUM_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x16800, 0x16A3F, RID_SUBSETSTR_BAMUM_SUPPLEMENT ) );
                    break;
                case UBLOCK_KANA_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1B000, 0x1B0FF, RID_SUBSETSTR_KANA_SUPPLEMENT ) );
                    break;
                case UBLOCK_PLAYING_CARDS:
                    aAllSubsets.push_back( Subset( 0x1F0A0, 0x1F0FF, RID_SUBSETSTR_PLAYING_CARDS ) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS:
                    aAllSubsets.push_back( Subset( 0x1F300, 0x1F5FF, RID_SUBSETSTR_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS ) );
                    break;
                case UBLOCK_EMOTICONS:
                    aAllSubsets.push_back( Subset( 0x1F600, 0x1F64F, RID_SUBSETSTR_EMOTICONS ) );
                    break;
                case UBLOCK_TRANSPORT_AND_MAP_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1F680, 0x1F6FF, RID_SUBSETSTR_TRANSPORT_AND_MAP_SYMBOLS ) );
                    break;
                case UBLOCK_ALCHEMICAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1F700, 0x1F77F, RID_SUBSETSTR_ALCHEMICAL_SYMBOLS ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D:
                    aAllSubsets.push_back( Subset( 0x2B740, 0x2B81F, RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D ) );
                    break;
#endif
// Note ICU version 49 (NOT 4.9), so the MAJOR_NUM is two digits.
#if U_ICU_VERSION_MAJOR_NUM >= 49
                case UBLOCK_ARABIC_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0x08A0, 0x08FF, RID_SUBSETSTR_ARABIC_EXTENDED_A ) );
                    break;
                case UBLOCK_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1EE00, 0x1EEFF, RID_SUBSETSTR_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS ) );
                    break;
                case UBLOCK_CHAKMA:
                    aAllSubsets.push_back( Subset( 0x11100, 0x1114F, RID_SUBSETSTR_CHAKMA ) );
                    break;
                case UBLOCK_MEETEI_MAYEK_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0xAAE0, 0xAAFF, RID_SUBSETSTR_MEETEI_MAYEK_EXTENSIONS ) );
                    break;
                case UBLOCK_MEROITIC_CURSIVE:
                    aAllSubsets.push_back( Subset( 0x109A0, 0x109FF, RID_SUBSETSTR_MEROITIC_CURSIVE ) );
                    break;
                case UBLOCK_MEROITIC_HIEROGLYPHS:
                    aAllSubsets.push_back( Subset( 0x10980, 0x1099F, RID_SUBSETSTR_MEROITIC_HIEROGLYPHS ) );
                    break;
                case UBLOCK_MIAO:
                    aAllSubsets.push_back( Subset( 0x16F00, 0x16F9F, RID_SUBSETSTR_MIAO ) );
                    break;
                case UBLOCK_SHARADA:
                    aAllSubsets.push_back( Subset( 0x11180, 0x111DF, RID_SUBSETSTR_SHARADA ) );
                    break;
                case UBLOCK_SORA_SOMPENG:
                    aAllSubsets.push_back( Subset( 0x110D0, 0x110FF, RID_SUBSETSTR_SORA_SOMPENG ) );
                    break;
                case UBLOCK_SUNDANESE_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1CC0, 0x1CCF, RID_SUBSETSTR_SUNDANESE_SUPPLEMENT ) );
                    break;
                case UBLOCK_TAKRI:
                    aAllSubsets.push_back( Subset( 0x11680, 0x116CF, RID_SUBSETSTR_TAKRI ) );
                    break;
#endif
            }

#if OSL_DEBUG_LEVEL > 0
            if (eBlock != UBLOCK_NO_BLOCK &&
                eBlock != UBLOCK_INVALID_CODE &&
                eBlock != UBLOCK_COUNT &&
                eBlock != UBLOCK_HIGH_SURROGATES &&
                eBlock != UBLOCK_HIGH_PRIVATE_USE_SURROGATES &&
                eBlock != UBLOCK_LOW_SURROGATES)

            {
                UBlockCode eBlockStart = ublock_getCode(aAllSubsets.back().GetRangeMin());
                UBlockCode eBlockEnd = ublock_getCode(aAllSubsets.back().GetRangeMax());
                assert(eBlockStart == eBlockEnd && eBlockStart == eBlock);
            }
#endif
        }

        aAllSubsets.sort();
    }

    maSubsets = aAllSubsets;
}

void SubsetMap::ApplyCharMap( const FontCharMap* pFontCharMap )
{
    if( !pFontCharMap )
        return;

    // remove subsets that are not matched in any range
    SubsetList::iterator it_next = maSubsets.begin();
    while( it_next != maSubsets.end() )
    {
        SubsetList::iterator it = it_next++;
        const Subset& rSubset = *it;
        sal_uInt32 cMin = rSubset.GetRangeMin();
        sal_uInt32 cMax = rSubset.GetRangeMax();

        int nCount =  pFontCharMap->CountCharsInRange( cMin, cMax );
        if( nCount <= 0 )
            maSubsets.erase( it );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
