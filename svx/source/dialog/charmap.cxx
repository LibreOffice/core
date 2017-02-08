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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/fontcharmap.hxx>
#include <svtools/colorcfg.hxx>
#include <tools/resary.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>

#include <svx/dialogs.hrc>

#include <svx/charmap.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svxdlg.hxx>

#include "charmapacc.hxx"
#include "uiobject.hxx"

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


sal_uInt32& SvxShowCharSet::getSelectedChar()
{
    static sal_uInt32 cSelectedChar = ' '; // keeps selected character over app lifetime
    return cSelectedChar;
}

SvxShowCharSet::SvxShowCharSet(vcl::Window* pParent)
    : Control(pParent, WB_TABSTOP | WB_BORDER)
    , maFontSize(0, 0)
    , aVscrollSB( VclPtr<ScrollBar>::Create(this, WB_VERT) )
    , mbRecalculateFont(true)
    , mbUpdateForeground(true)
    , mbUpdateBackground(true)
{
    init();
}

void SvxShowCharSet::ApplySettings(vcl::RenderContext& /*rRenderContext*/ )
{
}

void SvxShowCharSet::init()
{
    nSelectedIndex = -1;    // TODO: move into init list when it is no longer static
    m_nXGap = 0;
    m_nYGap = 0;

    SetStyle(GetStyle() | WB_CLIPCHILDREN);
    aVscrollSB->SetScrollHdl( LINK( this, SvxShowCharSet, VscrollHdl ) );
    aVscrollSB->EnableDrag();
    // other settings like aVscroll depend on selected font => see RecalculateFont

    bDrag = false;
}

void SvxShowCharSet::Resize()
{
    Control::Resize();
    mbRecalculateFont = true;
    Invalidate();
}

VCL_BUILDER_FACTORY(SvxShowCharSet)

void SvxShowCharSet::GetFocus()
{
    Control::GetFocus();
    SelectIndex( nSelectedIndex, true );
}


void SvxShowCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectIndex( nSelectedIndex );
}


void SvxShowCharSet::StateChanged(StateChangedType nType)
{
    if (nType == StateChangedType::ControlForeground)
        mbUpdateForeground = true;
    else if (nType == StateChangedType::ControlBackground)
        mbUpdateBackground = true;

    Invalidate();

    Control::StateChanged( nType );

}


void SvxShowCharSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS)
     && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        mbUpdateForeground = true;
        mbUpdateBackground = true;
    }
    else
    {
        Control::DataChanged(rDCEvt);
    }
}


void SvxShowCharSet::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 1 )
        {
            GrabFocus();
            bDrag = true;
            CaptureMouse();

            int nIndex = PixelToMapIndex( rMEvt.GetPosPixel() );
        // Fire the focus event
            SelectIndex( nIndex, true);
        }

        if ( !(rMEvt.GetClicks() % 2) )
            aDoubleClkHdl.Call( this );
    }
}


void SvxShowCharSet::MouseButtonUp( const MouseEvent& rMEvt )
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
    // Fire the focus event.
        SelectIndex( nIndex, true );
    }
}


void SvxShowCharSet::Command( const CommandEvent& rCEvt )
{
    if( !HandleScrollCommand( rCEvt, nullptr, aVscrollSB.get() ) )
        Control::Command( rCEvt );
}


sal_uInt16 SvxShowCharSet::GetRowPos(sal_uInt16 _nPos)
{
    return _nPos / COLUMN_COUNT ;
}


sal_uInt16 SvxShowCharSet::GetColumnPos(sal_uInt16 _nPos)
{
    return _nPos % COLUMN_COUNT ;
}


int SvxShowCharSet::FirstInView() const
{
    int nIndex = 0;
    if (aVscrollSB->IsVisible())
        nIndex += aVscrollSB->GetThumbPos() * COLUMN_COUNT;
    return nIndex;
}


int SvxShowCharSet::LastInView() const
{
    sal_uIntPtr nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT - 1;
    sal_uIntPtr nCompare = sal::static_int_cast<sal_uIntPtr>(mxFontCharMap->GetCharCount() - 1);
    if (nIndex > nCompare)
        nIndex = nCompare;
    return nIndex;
}


inline Point SvxShowCharSet::MapIndexToPixel( int nIndex ) const
{
    const int nBase = FirstInView();
    int x = ((nIndex - nBase) % COLUMN_COUNT) * nX;
    int y = ((nIndex - nBase) / COLUMN_COUNT) * nY;
    return Point( x + m_nXGap, y + m_nYGap );
}


int SvxShowCharSet::PixelToMapIndex( const Point& point) const
{
    int nBase = FirstInView();
    return (nBase + ((point.X() - m_nXGap)/nX) + ((point.Y() - m_nYGap)/nY) * COLUMN_COUNT);
}


void SvxShowCharSet::KeyInput(const KeyEvent& rKEvt)
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
            tmpSelected -= ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_PAGEDOWN:
            tmpSelected += ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_HOME:
            tmpSelected = 0;
            break;
        case KEY_END:
            tmpSelected = mxFontCharMap->GetCharCount() - 1;
            break;
        case KEY_TAB:   // some fonts have a character at these unicode control codes
        case KEY_ESCAPE:
        case KEY_RETURN:
            Control::KeyInput(rKEvt);
            tmpSelected = - 1;  // mark as invalid
            break;
        default:
            {
                sal_UCS4 cChar = rKEvt.GetCharCode();
                sal_UCS4 cNext = mxFontCharMap->GetNextChar(cChar - 1);
                tmpSelected = mxFontCharMap->GetIndexFromChar(cNext);
                if (tmpSelected < 0 || (cChar != cNext))
                {
                    Control::KeyInput(rKEvt);
                    tmpSelected = - 1;  // mark as invalid
                }
            }
    }

    if ( tmpSelected >= 0 )
    {
        SelectIndex( tmpSelected, true );
        aPreSelectHdl.Call( this );
    }
}


void SvxShowCharSet::Paint( vcl::RenderContext& rRenderContext, const Rectangle& )
{
    InitSettings(rRenderContext);
    RecalculateFont(rRenderContext);
    DrawChars_Impl(rRenderContext, FirstInView(), LastInView());
}

void SvxShowCharSet::SetFont( const vcl::Font& rFont )
{
    Control::SetFont(rFont);
    mbRecalculateFont = true;
    Invalidate();
}

void SvxShowCharSet::DeSelect()
{
    Invalidate();
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

void SvxShowCharSet::DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2)
{
    if (n1 > LastInView() || n2 < FirstInView())
        return;

    Size aOutputSize(GetOutputSizePixel());
    if (aVscrollSB->IsVisible())
        aOutputSize.Width() -= aVscrollSB->GetOptimalSize().Width();

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
    for (i = n1; i <= n2; ++i)
    {
        Point pix = MapIndexToPixel(i);
        int x = pix.X();
        int y = pix.Y();

        OUStringBuffer buf;
        buf.appendUtf32(mxFontCharMap->GetCharFromIndex(i));
        OUString aCharStr(buf.makeStringAndClear());
        int nTextWidth = rRenderContext.GetTextWidth(aCharStr);
        int tx = x + (nX - nTextWidth + 1) / 2;
        int ty = y + (nY - nTextHeight + 1) / 2;
        Point aPointTxTy(tx, ty);

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


void SvxShowCharSet::InitSettings(vcl::RenderContext& rRenderContext)
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


sal_UCS4 SvxShowCharSet::GetSelectCharacter() const
{
    if( nSelectedIndex >= 0 )
        getSelectedChar() = mxFontCharMap->GetCharFromIndex( nSelectedIndex );
    return getSelectedChar();
}


void SvxShowCharSet::RecalculateFont(vcl::RenderContext& rRenderContext)
{
    if (!mbRecalculateFont)
        return;

    // save last selected unicode
    if (nSelectedIndex >= 0)
        getSelectedChar() = mxFontCharMap->GetCharFromIndex(nSelectedIndex);

    Size aSize(GetOutputSizePixel());
    long nSBWidth = aVscrollSB->GetOptimalSize().Width();
    aSize.Width() -= nSBWidth;

    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetWeight(WEIGHT_LIGHT);
    aFont.SetAlignment(ALIGN_TOP);
    int nFontHeight = (aSize.Height() - 5) * 2 / (3 * ROW_COUNT);
    maFontSize = rRenderContext.PixelToLogic(Size(0, nFontHeight));
    aFont.SetFontSize(maFontSize);
    aFont.SetTransparent(true);
    rRenderContext.SetFont(aFont);
    rRenderContext.GetFontCharMap(mxFontCharMap);

    nX = aSize.Width() / COLUMN_COUNT;
    nY = aSize.Height() / ROW_COUNT;

    aVscrollSB->setPosSizePixel(aSize.Width(), 0, nSBWidth, aSize.Height());
    aVscrollSB->SetRangeMin(0);
    int nLastRow = (mxFontCharMap->GetCharCount() - 1 + COLUMN_COUNT) / COLUMN_COUNT;
    aVscrollSB->SetRangeMax(nLastRow);
    aVscrollSB->SetPageSize(ROW_COUNT - 1);
    aVscrollSB->SetVisibleSize(ROW_COUNT);

    // restore last selected unicode
    int nMapIndex = mxFontCharMap->GetIndexFromChar(getSelectedChar());
    SelectIndex(nMapIndex);

    aVscrollSB->Show();

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aDrawSize(nX * COLUMN_COUNT, nY * ROW_COUNT);
    m_nXGap = (aSize.Width() - aDrawSize.Width()) / 2;
    m_nYGap = (aSize.Height() - aDrawSize.Height()) / 2;

    mbRecalculateFont = false;
}


void SvxShowCharSet::SelectIndex( int nNewIndex, bool bFocus )
{
    if( !aVscrollSB )
        return;

    if ( !mxFontCharMap.is() )
        RecalculateFont( *this );

    if( nNewIndex < 0 )
    {
        // need to scroll see closest unicode
        sal_uInt32 cPrev = mxFontCharMap->GetPrevChar( getSelectedChar() );
        int nMapIndex = mxFontCharMap->GetIndexFromChar( cPrev );
        int nNewPos = nMapIndex / COLUMN_COUNT;
        aVscrollSB->SetThumbPos( nNewPos );
        nSelectedIndex = bFocus ? nMapIndex+1 : -1;
        Invalidate();
    }
    else if( nNewIndex < FirstInView() )
    {
        // need to scroll up to see selected item
        int nOldPos = aVscrollSB->GetThumbPos();
        int nDelta = (FirstInView() - nNewIndex + COLUMN_COUNT-1) / COLUMN_COUNT;
        aVscrollSB->SetThumbPos( nOldPos - nDelta );
        nSelectedIndex = nNewIndex;
        Invalidate();
    }
    else if( nNewIndex > LastInView() )
    {
        // need to scroll down to see selected item
        int nOldPos = aVscrollSB->GetThumbPos();
        int nDelta = (nNewIndex - LastInView() + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB->SetThumbPos( nOldPos + nDelta );
        if( nNewIndex < mxFontCharMap->GetCharCount() )
        {
            nSelectedIndex = nNewIndex;
            Invalidate();
        }
        else if (nOldPos != aVscrollSB->GetThumbPos())
        {
            Invalidate();
        }
    }
    else
    {
        nSelectedIndex = nNewIndex;
        Invalidate();
    }

    if( nSelectedIndex >= 0 )
    {
        getSelectedChar() = mxFontCharMap->GetCharFromIndex( nSelectedIndex );
        if( m_xAccessible.is() )
        {
            svx::SvxShowCharSetItem* pItem = ImplGetItem(nSelectedIndex);
            // Don't fire the focus event.
            if ( bFocus )
                m_xAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), makeAny(pItem->GetAccessible()) ); // this call assures that m_pItem is set
            else
                m_xAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS, Any(), makeAny(pItem->GetAccessible()) ); // this call assures that m_pItem is set

            assert(pItem->m_xItem.is() && "No accessible created!");
            Any aOldAny, aNewAny;
            aNewAny <<= AccessibleStateType::FOCUSED;
            // Don't fire the focus event.
            if ( bFocus )
                pItem->m_xItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );

            aNewAny <<= AccessibleStateType::SELECTED;
            pItem->m_xItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
        }
    }
    aHighHdl.Call( this );
}


void SvxShowCharSet::OutputIndex( int nNewIndex )
{
    SelectIndex( nNewIndex, true );
    aSelectHdl.Call( this );

}


void SvxShowCharSet::SelectCharacter( sal_UCS4 cNew )
{
    if ( !mxFontCharMap.is() )
        RecalculateFont( *this );

    // get next available char of current font
    sal_UCS4 cNext = mxFontCharMap->GetNextChar( (cNew > 0) ? cNew - 1 : cNew );

    int nMapIndex = mxFontCharMap->GetIndexFromChar( cNext );
    SelectIndex( nMapIndex );
    // move selected item to top row if not in focus
    aVscrollSB->SetThumbPos( nMapIndex / COLUMN_COUNT );
    Invalidate();
}


IMPL_LINK_NOARG(SvxShowCharSet, VscrollHdl, ScrollBar*, void)
{
    if( nSelectedIndex < FirstInView() )
    {
        SelectIndex( FirstInView() + (nSelectedIndex % COLUMN_COUNT) );
    }
    else if( nSelectedIndex > LastInView() )
    {
        if( m_xAccessible.is() )
        {
            css::uno::Any aOldAny, aNewAny;
            int nLast = LastInView();
            for ( ; nLast != nSelectedIndex; ++nLast)
            {
                aOldAny <<= ImplGetItem(nLast)->GetAccessible();
                m_xAccessible ->fireEvent( AccessibleEventId::CHILD, aOldAny, aNewAny );
            }
        }
        SelectIndex( (LastInView() - COLUMN_COUNT + 1) + (nSelectedIndex % COLUMN_COUNT) );
    }

    Invalidate();
}


SvxShowCharSet::~SvxShowCharSet()
{
    disposeOnce();
}

void SvxShowCharSet::dispose()
{
    if ( m_xAccessible.is() )
        ReleaseAccessible();
    aVscrollSB.disposeAndClear();
    Control::dispose();
}

void SvxShowCharSet::ReleaseAccessible()
{
    m_aItems.clear();
    m_xAccessible.clear();
}

css::uno::Reference< XAccessible > SvxShowCharSet::CreateAccessible()
{
    OSL_ENSURE(!m_xAccessible.is(),"Accessible already created!");
    m_xAccessible = new svx::SvxShowCharSetVirtualAcc(this);
    return m_xAccessible.get();
}

svx::SvxShowCharSetItem* SvxShowCharSet::ImplGetItem( int _nPos )
{
    ItemsMap::iterator aFind = m_aItems.find(_nPos);
    if ( aFind == m_aItems.end() )
    {
        OSL_ENSURE(m_xAccessible.is(), "Who wants to create a child of my table without a parent?");
        std::shared_ptr<svx::SvxShowCharSetItem> xItem(new svx::SvxShowCharSetItem(*this,
            m_xAccessible->getTable(), sal::static_int_cast< sal_uInt16 >(_nPos)));
        aFind = m_aItems.insert(ItemsMap::value_type(_nPos, xItem)).first;
        OUStringBuffer buf;
        buf.appendUtf32( mxFontCharMap->GetCharFromIndex( _nPos ) );
        aFind->second->maText = buf.makeStringAndClear();
        Point pix = MapIndexToPixel( _nPos );
        aFind->second->maRect = Rectangle( Point( pix.X() + 1, pix.Y() + 1 ), Size(nX-1,nY-1) );
    }

    return aFind->second.get();
}


sal_Int32 SvxShowCharSet::getMaxCharCount() const
{
    return mxFontCharMap->GetCharCount();
}

FactoryFunction SvxShowCharSet::GetUITestFactory() const
{
    return SvxShowCharSetUIObject::create;
}

// TODO: should be moved into Font Attributes stuff
// we let it mature here though because it is currently the only use

SubsetMap::SubsetMap( const FontCharMapRef& rxFontCharMap )
{
    InitList();
    ApplyCharMap(rxFontCharMap);
}

const Subset* SubsetMap::GetNextSubset( bool bFirst ) const
{
    if( bFirst )
        maSubsetIterator = maSubsets.begin();
    if( maSubsetIterator == maSubsets.end() )
        return nullptr;
    const Subset* s = &*(maSubsetIterator++);
    return s;
}

const Subset* SubsetMap::GetSubsetByUnicode( sal_UCS4 cChar ) const
{
    // TODO: is it worth to avoid a linear search?
    for( const Subset* s = GetNextSubset( true ); s; s = GetNextSubset( false ) )
        if( (s->GetRangeMin() <= cChar) && (cChar <= s->GetRangeMax()) )
            return s;
    return nullptr;
}

inline Subset::Subset(sal_UCS4 nMin, sal_UCS4 nMax, const OUString& rName)
:   mnRangeMin(nMin), mnRangeMax(nMax), maRangeName(rName)
{}

void SubsetMap::InitList()
{
    static SubsetList aAllSubsets;
    static bool bInit = true;
    if( bInit )
    {
        bInit = false;

        ResStringArray aStringList(SVX_RES(RID_SUBSETMAP));

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
                    aAllSubsets.push_back( Subset( 0x0000, 0x007F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BASIC_LATIN)) ) );
                    break;
                case UBLOCK_LATIN_1_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x0080, 0x00FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_1)) ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0x0100, 0x017F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_EXTENDED_A)) ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0x0180, 0x024F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_EXTENDED_B)) ) );
                    break;
                case UBLOCK_IPA_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x0250, 0x02AF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_IPA_EXTENSIONS)) ) );
                    break;
                case UBLOCK_SPACING_MODIFIER_LETTERS:
                    aAllSubsets.push_back( Subset( 0x02B0, 0x02FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SPACING_MODIFIERS)) ) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS:
                    aAllSubsets.push_back( Subset( 0x0300, 0x036F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COMB_DIACRITICAL)) ) );
                    break;
                case UBLOCK_GREEK:
                    aAllSubsets.push_back( Subset( 0x0370, 0x03FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BASIC_GREEK)) ) );
                    break;
                case UBLOCK_CYRILLIC:
                    aAllSubsets.push_back( Subset( 0x0400, 0x04FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CYRILLIC)) ) );
                    break;
                case UBLOCK_ARMENIAN:
                    aAllSubsets.push_back( Subset( 0x0530, 0x058F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARMENIAN)) ) );
                    break;
                case UBLOCK_HEBREW:
                    aAllSubsets.push_back( Subset( 0x0590, 0x05FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BASIC_HEBREW)) ) );
                    break;
                case UBLOCK_ARABIC:
                    aAllSubsets.push_back( Subset( 0x0600, 0x065F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BASIC_ARABIC)) ) );
                    break;
                case UBLOCK_SYRIAC:
                    aAllSubsets.push_back( Subset( 0x0700, 0x074F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SYRIAC)) ) );
                    break;
                case UBLOCK_THAANA:
                    aAllSubsets.push_back( Subset( 0x0780, 0x07BF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_THAANA)) ) );
                    break;
                case UBLOCK_DEVANAGARI:
                    aAllSubsets.push_back( Subset( 0x0900, 0x097F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_DEVANAGARI)) ) );
                    break;
                case UBLOCK_BENGALI:
                    aAllSubsets.push_back( Subset( 0x0980, 0x09FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BENGALI)) ) );
                    break;
                case UBLOCK_GURMUKHI:
                    aAllSubsets.push_back( Subset( 0x0A00, 0x0A7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GURMUKHI)) ) );
                    break;
                case UBLOCK_GUJARATI:
                    aAllSubsets.push_back( Subset( 0x0A80, 0x0AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GUJARATI)) ) );
                    break;
                case UBLOCK_ORIYA:
                    aAllSubsets.push_back( Subset( 0x0B00, 0x0B7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ODIA)) ) );
                    break;
                case UBLOCK_TAMIL:
                    aAllSubsets.push_back( Subset( 0x0B80, 0x0BFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAMIL)) ) );
                    break;
                case UBLOCK_TELUGU:
                    aAllSubsets.push_back( Subset( 0x0C00, 0x0C7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TELUGU)) ) );
                    break;
                case UBLOCK_KANNADA:
                    aAllSubsets.push_back( Subset( 0x0C80, 0x0CFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KANNADA)) ) );
                    break;
                case UBLOCK_MALAYALAM:
                    aAllSubsets.push_back( Subset( 0x0D00, 0x0D7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MALAYALAM)) ) );
                    break;
                case UBLOCK_SINHALA:
                    aAllSubsets.push_back( Subset( 0x0D80, 0x0DFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SINHALA)) ) );
                    break;
                case UBLOCK_THAI:
                    aAllSubsets.push_back( Subset( 0x0E00, 0x0E7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_THAI)) ) );
                    break;
                case UBLOCK_LAO:
                    aAllSubsets.push_back( Subset( 0x0E80, 0x0EFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LAO)) ) );
                    break;
                case UBLOCK_TIBETAN:
                    aAllSubsets.push_back( Subset( 0x0F00, 0x0FBF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TIBETAN)) ) );
                    break;
                case UBLOCK_MYANMAR:
                    aAllSubsets.push_back( Subset( 0x1000, 0x109F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MYANMAR)) ) );
                    break;
                case UBLOCK_GEORGIAN:
                    aAllSubsets.push_back( Subset( 0x10A0, 0x10FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BASIC_GEORGIAN)) ) );
                    break;
                case UBLOCK_HANGUL_JAMO:
                    aAllSubsets.push_back( Subset( 0x1100, 0x11FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HANGUL_JAMO)) ) );
                    break;
                case UBLOCK_ETHIOPIC:
                    aAllSubsets.push_back( Subset( 0x1200, 0x137F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ETHIOPIC)) ) );
                    break;
                case UBLOCK_CHEROKEE:
                    aAllSubsets.push_back( Subset( 0x13A0, 0x13FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CHEROKEE)) ) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS:
                    aAllSubsets.push_back( Subset( 0x1400, 0x167F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CANADIAN_ABORIGINAL)) ) );
                    break;
                case UBLOCK_OGHAM:
                    aAllSubsets.push_back( Subset( 0x1680, 0x169F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OGHAM)) ) );
                    break;
                case UBLOCK_RUNIC:
                    aAllSubsets.push_back( Subset( 0x16A0, 0x16F0, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_RUNIC)) ) );
                    break;
                case UBLOCK_KHMER:
                    aAllSubsets.push_back( Subset( 0x1780, 0x17FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KHMER)) ) );
                    break;
                case UBLOCK_MONGOLIAN:
                    aAllSubsets.push_back( Subset( 0x1800, 0x18AF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MONGOLIAN)) ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_ADDITIONAL:
                    aAllSubsets.push_back( Subset( 0x1E00, 0x1EFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_EXTENDED_ADDS)) ) );
                    break;
                case UBLOCK_GREEK_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x1F00, 0x1FFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GREEK_EXTENDED)) ) );
                    break;
                case UBLOCK_GENERAL_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x2000, 0x206F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GENERAL_PUNCTUATION)) ) );
                    break;
                case UBLOCK_SUPERSCRIPTS_AND_SUBSCRIPTS:
                    aAllSubsets.push_back( Subset( 0x2070, 0x209F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUB_SUPER_SCRIPTS)) ) );
                    break;
                case UBLOCK_CURRENCY_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x20A0, 0x20CF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CURRENCY_SYMBOLS)) ) );
                    break;
                case UBLOCK_COMBINING_MARKS_FOR_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x20D0, 0x20FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COMB_DIACRITIC_SYMS)) ) );
                    break;
                case UBLOCK_LETTERLIKE_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x2100, 0x214F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LETTERLIKE_SYMBOLS)) ) );
                    break;
                case UBLOCK_NUMBER_FORMS:
                    aAllSubsets.push_back( Subset( 0x2150, 0x218F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_NUMBER_FORMS)) ) );
                    break;
                case UBLOCK_ARROWS:
                    aAllSubsets.push_back( Subset( 0x2190, 0x21FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARROWS)) ) );
                    break;
                case UBLOCK_MATHEMATICAL_OPERATORS:
                    aAllSubsets.push_back( Subset( 0x2200, 0x22FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MATH_OPERATORS)) ) );
                    break;
                case UBLOCK_MISCELLANEOUS_TECHNICAL:
                    aAllSubsets.push_back( Subset( 0x2300, 0x23FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISC_TECHNICAL)) ) );
                    break;
                case UBLOCK_CONTROL_PICTURES:
                    aAllSubsets.push_back( Subset( 0x2400, 0x243F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CONTROL_PICTURES)) ) );
                    break;
                case UBLOCK_OPTICAL_CHARACTER_RECOGNITION:
                    aAllSubsets.push_back( Subset( 0x2440, 0x245F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OPTICAL_CHAR_REC)) ) );
                    break;
                case UBLOCK_ENCLOSED_ALPHANUMERICS:
                    aAllSubsets.push_back( Subset( 0x2460, 0x24FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ENCLOSED_ALPHANUM)) ) );
                    break;
                case UBLOCK_BOX_DRAWING:
                    aAllSubsets.push_back( Subset( 0x2500, 0x257F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BOX_DRAWING)) ) );
                    break;
                case UBLOCK_BLOCK_ELEMENTS:
                    aAllSubsets.push_back( Subset( 0x2580, 0x259F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BLOCK_ELEMENTS)) ) );
                    break;
                case UBLOCK_GEOMETRIC_SHAPES:
                    aAllSubsets.push_back( Subset( 0x25A0, 0x25FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GEOMETRIC_SHAPES)) ) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x2600, 0x26FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISC_DINGBATS)) ) );
                    break;
                case UBLOCK_DINGBATS:
                    aAllSubsets.push_back( Subset( 0x2700, 0x27BF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_DINGBATS)) ) );
                    break;
                case UBLOCK_BRAILLE_PATTERNS:
                    aAllSubsets.push_back( Subset( 0x2800, 0x28FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BRAILLE_PATTERNS)) ) );
                    break;
                case UBLOCK_CJK_RADICALS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x2E80, 0x2EFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_RADICAL_SUPPL)) ) );
                    break;
                case UBLOCK_KANGXI_RADICALS:
                    aAllSubsets.push_back( Subset( 0x2F00, 0x2FDF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KANGXI_RADICALS)) ) );
                    break;
                case UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS:
                    aAllSubsets.push_back( Subset( 0x2FF0, 0x2FFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_IDEO_DESC_CHARS)) ) );
                    break;
                case UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x3000, 0x303F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_SYMS_PUNCTUATION)) ) );
                    break;
                case UBLOCK_HIRAGANA:
                    aAllSubsets.push_back( Subset( 0x3040, 0x309F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HIRAGANA)) ) );
                    break;
                case UBLOCK_KATAKANA:
                    aAllSubsets.push_back( Subset( 0x30A0, 0x30FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KATAKANA)) ) );
                    break;
                case UBLOCK_BOPOMOFO:
                    aAllSubsets.push_back( Subset( 0x3100, 0x312F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BOPOMOFO)) ) );
                    break;
                case UBLOCK_HANGUL_COMPATIBILITY_JAMO:
                    aAllSubsets.push_back( Subset( 0x3130, 0x318F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HANGUL_COMPAT_JAMO)) ) );
                    break;
                case UBLOCK_KANBUN:
                    aAllSubsets.push_back( Subset( 0x3190, 0x319F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KANBUN)) ) );
                    break;
                case UBLOCK_BOPOMOFO_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x31A0, 0x31BF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BOPOMOFO_EXTENDED)) ) );
                    break;
                case UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS:
                    aAllSubsets.push_back( Subset( 0x3200, 0x32FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ENCLOSED_CJK_LETTERS)) ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY:
                    aAllSubsets.push_back( Subset( 0x3300, 0x33FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_COMPATIBILITY)) ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A:
                    aAllSubsets.push_back( Subset( 0x3400, 0x4DBF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_EXT_A_UNIFIED_IDGRAPH)) ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS:
                    aAllSubsets.push_back( Subset( 0x4E00, 0x9FA5, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_UNIFIED_IDGRAPH)) ) );
                    break;
                case UBLOCK_YI_SYLLABLES:
                    aAllSubsets.push_back( Subset( 0xA000, 0xA48F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_YI_SYLLABLES)) ) );
                    break;
                case UBLOCK_YI_RADICALS:
                    aAllSubsets.push_back( Subset( 0xA490, 0xA4CF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_YI_RADICALS)) ) );
                    break;
                case UBLOCK_HANGUL_SYLLABLES:
                    aAllSubsets.push_back( Subset( 0xAC00, 0xD7AF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HANGUL)) ) );
                    break;
                case UBLOCK_PRIVATE_USE_AREA:
                    aAllSubsets.push_back( Subset( 0xE000, 0xF8FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PRIVATE_USE_AREA)) ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS:
                    aAllSubsets.push_back( Subset( 0xF900, 0xFAFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_COMPAT_IDGRAPHS)) ) );
                    break;
                case UBLOCK_ALPHABETIC_PRESENTATION_FORMS:
                    aAllSubsets.push_back( Subset( 0xFB00, 0xFB4F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ALPHA_PRESENTATION)) ) );
                    break;
                case UBLOCK_ARABIC_PRESENTATION_FORMS_A:
                    aAllSubsets.push_back( Subset( 0xFB50, 0xFDFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARABIC_PRESENT_A)) ) );
                    break;
                case UBLOCK_COMBINING_HALF_MARKS:
                    aAllSubsets.push_back( Subset( 0xFE20, 0xFE2F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COMBINING_HALF_MARKS)) ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_FORMS:
                    aAllSubsets.push_back( Subset( 0xFE30, 0xFE4F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_COMPAT_FORMS)) ) );
                    break;
                case UBLOCK_SMALL_FORM_VARIANTS:
                    aAllSubsets.push_back( Subset( 0xFE50, 0xFE6F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SMALL_FORM_VARIANTS)) ) );
                    break;
                case UBLOCK_ARABIC_PRESENTATION_FORMS_B:
                    aAllSubsets.push_back( Subset( 0xFE70, 0xFEFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARABIC_PRESENT_B)) ) );
                    break;
                case UBLOCK_SPECIALS:
                    aAllSubsets.push_back( Subset( 0xFFF0, 0xFFFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SPECIALS)) ) );
                    break;
                case UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS:
                    aAllSubsets.push_back( Subset( 0xFF00, 0xFFEF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HALFW_FULLW_FORMS)) ) );
                    break;
                case UBLOCK_OLD_ITALIC:
                    aAllSubsets.push_back( Subset( 0x10300, 0x1032F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_ITALIC)) ) );
                    break;
                case UBLOCK_GOTHIC:
                    aAllSubsets.push_back( Subset( 0x10330, 0x1034F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GOTHIC)) ) );
                    break;
                case UBLOCK_DESERET:
                    aAllSubsets.push_back( Subset( 0x10400, 0x1044F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_DESERET)) ) );
                    break;
                case UBLOCK_BYZANTINE_MUSICAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D000, 0x1D0FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BYZANTINE_MUSICAL_SYMBOLS)) ) );
                    break;
                case UBLOCK_MUSICAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D100, 0x1D1FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MUSICAL_SYMBOLS)) ) );
                    break;
                case UBLOCK_MATHEMATICAL_ALPHANUMERIC_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D400, 0x1D7FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MATHEMATICAL_ALPHANUMERIC_SYMBOLS)) ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B:
                    aAllSubsets.push_back( Subset( 0x20000, 0x2A6DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B)) ) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x2F800, 0x2FA1F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_TAGS:
                    aAllSubsets.push_back( Subset( 0xE0000, 0xE007F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAGS)) ) );
                    break;
                case UBLOCK_CYRILLIC_SUPPLEMENTARY:
                    aAllSubsets.push_back( Subset( 0x0500, 0x052F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CYRILLIC_SUPPLEMENTARY)) ) );
                    break;
                case UBLOCK_TAGALOG:
                    aAllSubsets.push_back( Subset( 0x1700, 0x171F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAGALOG)) ) );
                    break;
                case UBLOCK_HANUNOO:
                    aAllSubsets.push_back( Subset( 0x1720, 0x173F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HANUNOO)) ) );
                    break;
                case UBLOCK_BUHID:
                    aAllSubsets.push_back( Subset( 0x1740, 0x175F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BUHID)) ) );
                    break;
                case UBLOCK_TAGBANWA:
                    aAllSubsets.push_back( Subset( 0x1760, 0x177F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAGBANWA)) ) );
                    break;
                case UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A:
                    aAllSubsets.push_back( Subset( 0x27C0, 0x27EF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISC_MATH_SYMS_A)) ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_A:
                    aAllSubsets.push_back( Subset( 0x27F0, 0x27FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPL_ARROWS_A)) ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_B:
                    aAllSubsets.push_back( Subset( 0x2900, 0x297F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPL_ARROWS_B)) ) );
                    break;
                case UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B:
                    aAllSubsets.push_back( Subset( 0x2980, 0x29FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISC_MATH_SYMS_B)) ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_MATHEMATICAL_OPERATORS:
                    aAllSubsets.push_back( Subset( 0x2A00, 0x2AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISC_MATH_SYMS_B)) ) );
                    break;
                case UBLOCK_KATAKANA_PHONETIC_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x31F0, 0x31FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KATAKANA_PHONETIC)) ) );
                    break;
                case UBLOCK_VARIATION_SELECTORS:
                    aAllSubsets.push_back( Subset( 0xFE00, 0xFE0F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_VARIATION_SELECTORS)) ) );
                    break;
                case UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A:
                    aAllSubsets.push_back( Subset( 0xF0000, 0xFFFFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPLEMENTARY_PRIVATE_USE_AREA_A)) ) );
                    break;
                case UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B:
                    aAllSubsets.push_back( Subset( 0x100000, 0x10FFFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPLEMENTARY_PRIVATE_USE_AREA_B)) ) );
                    break;
                case UBLOCK_LIMBU:
                    aAllSubsets.push_back( Subset( 0x1900, 0x194F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LIMBU)) ) );
                    break;
                case UBLOCK_TAI_LE:
                    aAllSubsets.push_back( Subset( 0x1950, 0x197F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAI_LE)) ) );
                    break;
                case UBLOCK_KHMER_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x19E0, 0x19FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KHMER_SYMBOLS)) ) );
                    break;
                case UBLOCK_PHONETIC_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x1D00, 0x1D7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PHONETIC_EXTENSIONS)) ) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_ARROWS:
                    aAllSubsets.push_back( Subset( 0x2B00, 0x2BFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISCELLANEOUS_SYMBOLS_AND_ARROWS)) ) );
                    break;
                case UBLOCK_YIJING_HEXAGRAM_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x4DC0, 0x4DFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_YIJING_HEXAGRAM_SYMBOLS)) ) );
                    break;
                case UBLOCK_LINEAR_B_SYLLABARY:
                    aAllSubsets.push_back( Subset( 0x10000, 0x1007F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LINEAR_B_SYLLABARY)) ) );
                    break;
                case UBLOCK_LINEAR_B_IDEOGRAMS:
                    aAllSubsets.push_back( Subset( 0x10080, 0x100FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LINEAR_B_IDEOGRAMS)) ) );
                    break;
                case UBLOCK_AEGEAN_NUMBERS:
                    aAllSubsets.push_back( Subset( 0x10100, 0x1013F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_AEGEAN_NUMBERS)) ) );
                    break;
                case UBLOCK_UGARITIC:
                    aAllSubsets.push_back( Subset( 0x10380, 0x1039F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_UGARITIC)) ) );
                    break;
                case UBLOCK_SHAVIAN:
                    aAllSubsets.push_back( Subset( 0x10450, 0x1047F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SHAVIAN)) ) );
                    break;
                case UBLOCK_OSMANYA:
                    aAllSubsets.push_back( Subset( 0x10480, 0x104AF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OSMANYA)) ) );
                    break;
                case UBLOCK_CYPRIOT_SYLLABARY:
                    aAllSubsets.push_back( Subset( 0x10800, 0x1083F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CYPRIOT_SYLLABARY)) ) );
                    break;
                case UBLOCK_TAI_XUAN_JING_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1D300, 0x1D35F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAI_XUAN_JING_SYMBOLS)) ) );
                    break;
                case UBLOCK_VARIATION_SELECTORS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0xE0100, 0xE01EF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_VARIATION_SELECTORS_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_ANCIENT_GREEK_MUSICAL_NOTATION:
                    aAllSubsets.push_back( Subset(0x1D200, 0x1D24F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ANCIENT_GREEK_MUSICAL_NOTATION)) ) );
                    break;
                case UBLOCK_ANCIENT_GREEK_NUMBERS:
                    aAllSubsets.push_back( Subset(0x10140, 0x1018F , aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ANCIENT_GREEK_NUMBERS)) ) );
                    break;
                case UBLOCK_ARABIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset(0x0750, 0x077F , aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARABIC_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_BUGINESE:
                    aAllSubsets.push_back( Subset(0x1A00, 0x1A1F , aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BUGINESE)) ) );
                    break;
                case UBLOCK_CJK_STROKES:
                    aAllSubsets.push_back( Subset( 0x31C0, 0x31EF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_STROKES)) ) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1DC0, 0x1DFF , aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_COPTIC:
                    aAllSubsets.push_back( Subset( 0x2C80, 0x2CFF , aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COPTIC)) ) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x2D80, 0x2DDF , aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ETHIOPIC_EXTENDED)) ) );
                    break;
                case UBLOCK_ETHIOPIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1380, 0x139F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ETHIOPIC_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_GEORGIAN_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x2D00, 0x2D2F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GEORGIAN_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_GLAGOLITIC:
                    aAllSubsets.push_back( Subset( 0x2C00, 0x2C5F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GLAGOLITIC)) ) );
                    break;
                case UBLOCK_KHAROSHTHI:
                    aAllSubsets.push_back( Subset( 0x10A00, 0x10A5F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KHAROSHTHI)) ) );
                    break;
                case UBLOCK_MODIFIER_TONE_LETTERS:
                    aAllSubsets.push_back( Subset( 0xA700, 0xA71F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MODIFIER_TONE_LETTERS)) ) );
                    break;
                case UBLOCK_NEW_TAI_LUE:
                    aAllSubsets.push_back( Subset( 0x1980, 0x19DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_NEW_TAI_LUE)) ) );
                    break;
                case UBLOCK_OLD_PERSIAN:
                    aAllSubsets.push_back( Subset( 0x103A0, 0x103DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_PERSIAN)) ) );
                    break;
                case UBLOCK_PHONETIC_EXTENSIONS_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1D80, 0x1DBF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PHONETIC_EXTENSIONS_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x2E00, 0x2E7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPLEMENTAL_PUNCTUATION)) ) );
                    break;
                case UBLOCK_SYLOTI_NAGRI:
                    aAllSubsets.push_back( Subset( 0xA800, 0xA82F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SYLOTI_NAGRI)) ) );
                    break;
                case UBLOCK_TIFINAGH:
                    aAllSubsets.push_back( Subset( 0x2D30, 0x2D7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TIFINAGH)) ) );
                    break;
                case UBLOCK_VERTICAL_FORMS:
                    aAllSubsets.push_back( Subset( 0xFE10, 0xFE1F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_VERTICAL_FORMS)) ) );
                    break;
                case UBLOCK_NKO:
                    aAllSubsets.push_back( Subset( 0x07C0, 0x07FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_NKO)) ) );
                    break;
                case UBLOCK_BALINESE:
                    aAllSubsets.push_back( Subset( 0x1B00, 0x1B7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BALINESE)) ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_C:
                    aAllSubsets.push_back( Subset( 0x2C60, 0x2C7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_EXTENDED_C)) ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_D:
                    aAllSubsets.push_back( Subset( 0xA720, 0xA7FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_EXTENDED_D)) ) );
                    break;
                case UBLOCK_PHAGS_PA:
                    aAllSubsets.push_back( Subset( 0xA840, 0xA87F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PHAGS_PA)) ) );
                    break;
                case UBLOCK_PHOENICIAN:
                    aAllSubsets.push_back( Subset( 0x10900, 0x1091F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PHOENICIAN)) ) );
                    break;
                case UBLOCK_CUNEIFORM:
                    aAllSubsets.push_back( Subset( 0x12000, 0x123FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CUNEIFORM)) ) );
                    break;
                case UBLOCK_CUNEIFORM_NUMBERS_AND_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x12400, 0x1247F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CUNEIFORM_NUMBERS_AND_PUNCTUATION)) ) );
                    break;
                case UBLOCK_COUNTING_ROD_NUMERALS:
                    aAllSubsets.push_back( Subset( 0x1D360, 0x1D37F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COUNTING_ROD_NUMERALS)) ) );
                    break;
                case UBLOCK_SUNDANESE:
                    aAllSubsets.push_back( Subset( 0x1B80, 0x1BBF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUNDANESE)) ) );
                    break;
                case UBLOCK_LEPCHA:
                    aAllSubsets.push_back( Subset( 0x1C00, 0x1C4F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LEPCHA)) ) );
                    break;
                case UBLOCK_OL_CHIKI:
                    aAllSubsets.push_back( Subset( 0x1C50, 0x1C7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OL_CHIKI)) ) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0x2DE0, 0x2DFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CYRILLIC_EXTENDED_A)) ) );
                    break;
                case UBLOCK_VAI:
                    aAllSubsets.push_back( Subset( 0xA500, 0xA63F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_VAI)) ) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0xA640, 0xA69F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CYRILLIC_EXTENDED_B)) ) );
                    break;
                case UBLOCK_SAURASHTRA:
                    aAllSubsets.push_back( Subset( 0xA880, 0xA8DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SAURASHTRA)) ) );
                    break;
                case UBLOCK_KAYAH_LI:
                    aAllSubsets.push_back( Subset( 0xA900, 0xA92F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KAYAH_LI)) ) );
                    break;
                case UBLOCK_REJANG:
                    aAllSubsets.push_back( Subset( 0xA930, 0xA95F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_REJANG)) ) );
                    break;
                case UBLOCK_CHAM:
                    aAllSubsets.push_back( Subset( 0xAA00, 0xAA5F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CHAM)) ) );
                    break;
                case UBLOCK_ANCIENT_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x10190, 0x101CF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ANCIENT_SYMBOLS)) ) );
                    break;
                case UBLOCK_PHAISTOS_DISC:
                    aAllSubsets.push_back( Subset( 0x101D0, 0x101FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PHAISTOS_DISC)) ) );
                    break;
                case UBLOCK_LYCIAN:
                    aAllSubsets.push_back( Subset( 0x10280, 0x1029F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LYCIAN)) ) );
                    break;
                case UBLOCK_CARIAN:
                    aAllSubsets.push_back( Subset( 0x102A0, 0x102DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CARIAN)) ) );
                    break;
                case UBLOCK_LYDIAN:
                    aAllSubsets.push_back( Subset( 0x10920, 0x1093F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LYDIAN)) ) );
                    break;
                case UBLOCK_MAHJONG_TILES:
                    aAllSubsets.push_back( Subset( 0x1F000, 0x1F02F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MAHJONG_TILES)) ) );
                    break;
                case UBLOCK_DOMINO_TILES:
                    aAllSubsets.push_back( Subset( 0x1F030, 0x1F09F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_DOMINO_TILES)) ) );
                    break;
#if (U_ICU_VERSION_MAJOR_NUM > 4) || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 4)
                case UBLOCK_SAMARITAN:
                    aAllSubsets.push_back( Subset( 0x0800, 0x083F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SAMARITAN)) ) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x18B0, 0x18FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED)) ) );
                    break;
                case UBLOCK_TAI_THAM:
                    aAllSubsets.push_back( Subset( 0x1A20, 0x1AAF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAI_THAM)) ) );
                    break;
                case UBLOCK_VEDIC_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0x1CD0, 0x1CFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_VEDIC_EXTENSIONS)) ) );
                    break;
                case UBLOCK_LISU:
                    aAllSubsets.push_back( Subset( 0xA4D0, 0xA4FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LISU)) ) );
                    break;
                case UBLOCK_BAMUM:
                    aAllSubsets.push_back( Subset( 0xA6A0, 0xA6FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BAMUM)) ) );
                    break;
                case UBLOCK_COMMON_INDIC_NUMBER_FORMS:
                    aAllSubsets.push_back( Subset( 0xA830, 0xA83F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COMMON_INDIC_NUMBER_FORMS)) ) );
                    break;
                case UBLOCK_DEVANAGARI_EXTENDED:
                    aAllSubsets.push_back( Subset( 0xA8E0, 0xA8FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_DEVANAGARI_EXTENDED)) ) );
                    break;
                case UBLOCK_HANGUL_JAMO_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0xA960, 0xA97F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HANGUL_JAMO_EXTENDED_A)) ) );
                    break;
                case UBLOCK_JAVANESE:
                    aAllSubsets.push_back( Subset( 0xA980, 0xA9DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_JAVANESE)) ) );
                    break;
                case UBLOCK_MYANMAR_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0xAA60, 0xAA7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MYANMAR_EXTENDED_A)) ) );
                    break;
                case UBLOCK_TAI_VIET:
                    aAllSubsets.push_back( Subset( 0xAA80, 0xAADF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAI_VIET)) ) );
                    break;
                case UBLOCK_MEETEI_MAYEK:
                    aAllSubsets.push_back( Subset( 0xABC0, 0xABFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MEETEI_MAYEK)) ) );
                    break;
                case UBLOCK_HANGUL_JAMO_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0xD7B0, 0xD7FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HANGUL_JAMO_EXTENDED_B)) ) );
                    break;
                case UBLOCK_IMPERIAL_ARAMAIC:
                    aAllSubsets.push_back( Subset( 0x10840, 0x1085F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_IMPERIAL_ARAMAIC)) ) );
                    break;
                case UBLOCK_OLD_SOUTH_ARABIAN:
                    aAllSubsets.push_back( Subset( 0x10A60, 0x10A7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_SOUTH_ARABIAN)) ) );
                    break;
                case UBLOCK_AVESTAN:
                    aAllSubsets.push_back( Subset( 0x10B00, 0x10B3F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_AVESTAN)) ) );
                    break;
                case UBLOCK_INSCRIPTIONAL_PARTHIAN:
                    aAllSubsets.push_back( Subset( 0x10B40, 0x10B5F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_INSCRIPTIONAL_PARTHIAN)) ) );
                    break;
                case UBLOCK_INSCRIPTIONAL_PAHLAVI:
                    aAllSubsets.push_back( Subset( 0x10B60, 0x10B7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_INSCRIPTIONAL_PAHLAVI)) ) );
                    break;
                case UBLOCK_OLD_TURKIC:
                    aAllSubsets.push_back( Subset( 0x10C00, 0x10C4F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_TURKIC)) ) );
                    break;
                case UBLOCK_RUMI_NUMERAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x10E60, 0x10E7F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_RUMI_NUMERAL_SYMBOLS)) ) );
                    break;
                case UBLOCK_KAITHI:
                    aAllSubsets.push_back( Subset( 0x11080, 0x110CF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KAITHI)) ) );
                    break;
                case UBLOCK_EGYPTIAN_HIEROGLYPHS:
                    aAllSubsets.push_back( Subset( 0x13000, 0x1342F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_EGYPTIAN_HIEROGLYPHS)) ) );
                    break;
                case UBLOCK_ENCLOSED_ALPHANUMERIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1F100, 0x1F1FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ENCLOSED_ALPHANUMERIC_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1F200, 0x1F2FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C:
                    aAllSubsets.push_back( Subset( 0x2A700, 0x2B73F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C)) ) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM > 4) || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 6)
                case UBLOCK_MANDAIC:
                    aAllSubsets.push_back( Subset( 0x0840, 0x085F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MANDAIC)) ) );
                    break;
                case UBLOCK_BATAK:
                    aAllSubsets.push_back( Subset( 0x1BC0, 0x1BFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BATAK)) ) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0xAB00, 0xAB2F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ETHIOPIC_EXTENDED_A)) ) );
                    break;
                case UBLOCK_BRAHMI:
                    aAllSubsets.push_back( Subset( 0x11000, 0x1107F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BRAHMI)) ) );
                    break;
                case UBLOCK_BAMUM_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x16800, 0x16A3F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BAMUM_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_KANA_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1B000, 0x1B0FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KANA_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_PLAYING_CARDS:
                    aAllSubsets.push_back( Subset( 0x1F0A0, 0x1F0FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PLAYING_CARDS)) ) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS:
                    aAllSubsets.push_back( Subset( 0x1F300, 0x1F5FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS)) ) );
                    break;
                case UBLOCK_EMOTICONS:
                    aAllSubsets.push_back( Subset( 0x1F600, 0x1F64F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_EMOTICONS)) ) );
                    break;
                case UBLOCK_TRANSPORT_AND_MAP_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1F680, 0x1F6FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TRANSPORT_AND_MAP_SYMBOLS)) ) );
                    break;
                case UBLOCK_ALCHEMICAL_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1F700, 0x1F77F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ALCHEMICAL_SYMBOLS)) ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D:
                    aAllSubsets.push_back( Subset( 0x2B740, 0x2B81F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D)) ) );
                    break;
#endif
// Note ICU version 49 (NOT 4.9), so the MAJOR_NUM is two digits.
#if U_ICU_VERSION_MAJOR_NUM >= 49
                case UBLOCK_ARABIC_EXTENDED_A:
                    aAllSubsets.push_back( Subset( 0x08A0, 0x08FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARABIC_EXTENDED_A)) ) );
                    break;
                case UBLOCK_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS:
                    aAllSubsets.push_back( Subset( 0x1EE00, 0x1EEFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS)) ) );
                    break;
                case UBLOCK_CHAKMA:
                    aAllSubsets.push_back( Subset( 0x11100, 0x1114F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CHAKMA)) ) );
                    break;
                case UBLOCK_MEETEI_MAYEK_EXTENSIONS:
                    aAllSubsets.push_back( Subset( 0xAAE0, 0xAAFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MEETEI_MAYEK_EXTENSIONS)) ) );
                    break;
                case UBLOCK_MEROITIC_CURSIVE:
                    aAllSubsets.push_back( Subset( 0x109A0, 0x109FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MEROITIC_CURSIVE)) ) );
                    break;
                case UBLOCK_MEROITIC_HIEROGLYPHS:
                    aAllSubsets.push_back( Subset( 0x10980, 0x1099F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MEROITIC_HIEROGLYPHS)) ) );
                    break;
                case UBLOCK_MIAO:
                    aAllSubsets.push_back( Subset( 0x16F00, 0x16F9F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MIAO)) ) );
                    break;
                case UBLOCK_SHARADA:
                    aAllSubsets.push_back( Subset( 0x11180, 0x111DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SHARADA)) ) );
                    break;
                case UBLOCK_SORA_SOMPENG:
                    aAllSubsets.push_back( Subset( 0x110D0, 0x110FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SORA_SOMPENG)) ) );
                    break;
                case UBLOCK_SUNDANESE_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1CC0, 0x1CCF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUNDANESE_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_TAKRI:
                    aAllSubsets.push_back( Subset( 0x11680, 0x116CF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TAKRI)) ) );
                    break;
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54
                case UBLOCK_BASSA_VAH:
                    aAllSubsets.push_back( Subset( 0x16AD0, 0x16AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BASSA_VAH)) ) );
                    break;
                case UBLOCK_CAUCASIAN_ALBANIAN:
                    aAllSubsets.push_back( Subset( 0x10530, 0x1056F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CAUCASIAN_ALBANIAN)) ) );
                    break;
                case UBLOCK_COPTIC_EPACT_NUMBERS:
                    aAllSubsets.push_back( Subset( 0x102E0, 0x102FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COPTIC_EPACT_NUMBERS)) ) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x1AB0, 0x1AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_COMBINING_DIACRITICAL_MARKS_EXTENDED)) ) );
                    break;
                case UBLOCK_DUPLOYAN:
                    aAllSubsets.push_back( Subset( 0x1BC00, 0x1BC9F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_DUPLOYAN)) ) );
                    break;
                case UBLOCK_ELBASAN:
                    aAllSubsets.push_back( Subset( 0x10500, 0x1052F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ELBASAN)) ) );
                    break;
                case UBLOCK_GEOMETRIC_SHAPES_EXTENDED:
                    aAllSubsets.push_back( Subset( 0x1F780, 0x1F7FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GEOMETRIC_SHAPES_EXTENDED)) ) );
                    break;
                case UBLOCK_GRANTHA:
                    aAllSubsets.push_back( Subset( 0x11300, 0x1137F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GRANTHA)) ) );
                    break;
                case UBLOCK_KHOJKI:
                    aAllSubsets.push_back( Subset( 0x11200, 0x1124F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KHOJKI)) ) );
                    break;
                case UBLOCK_KHUDAWADI:
                    aAllSubsets.push_back( Subset( 0x112B0, 0x112FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_KHUDAWADI)) ) );
                    break;
                case UBLOCK_LATIN_EXTENDED_E:
                    aAllSubsets.push_back( Subset( 0xAB30, 0xAB6F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LATIN_EXTENDED_E)) ) );
                    break;
                case UBLOCK_LINEAR_A:
                    aAllSubsets.push_back( Subset( 0x10600, 0x1077F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_LINEAR_A)) ) );
                    break;
                case UBLOCK_MAHAJANI:
                    aAllSubsets.push_back( Subset( 0x11150, 0x1117F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MAHAJANI)) ) );
                    break;
                case UBLOCK_MANICHAEAN:
                    aAllSubsets.push_back( Subset( 0x10AC0, 0x10AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MANICHAEAN)) ) );
                    break;
                case UBLOCK_MENDE_KIKAKUI:
                    aAllSubsets.push_back( Subset( 0x1E800, 0x1E8DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MENDE_KIKAKUI)) ) );
                    break;
                case UBLOCK_MODI:
                    aAllSubsets.push_back( Subset( 0x11600, 0x1165F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MODI)) ) );
                    break;
                case UBLOCK_MRO:
                    aAllSubsets.push_back( Subset( 0x16A40, 0x16A6F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MRO)) ) );
                    break;
                case UBLOCK_MYANMAR_EXTENDED_B:
                    aAllSubsets.push_back( Subset( 0xA9E0, 0xA9FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MYANMAR_EXTENDED_B)) ) );
                    break;
                case UBLOCK_NABATAEAN:
                    aAllSubsets.push_back( Subset( 0x10880, 0x108AF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_NABATAEAN)) ) );
                    break;
                case UBLOCK_OLD_NORTH_ARABIAN:
                    aAllSubsets.push_back( Subset( 0x10A80, 0x10A9F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_NORTH_ARABIAN)) ) );
                    break;
                case UBLOCK_OLD_PERMIC:
                    aAllSubsets.push_back( Subset( 0x10350, 0x1037F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_PERMIC)) ) );
                    break;
                case UBLOCK_ORNAMENTAL_DINGBATS:
                    aAllSubsets.push_back( Subset( 0x1F650, 0x1F67F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ORNAMENTAL_DINGBATS)) ) );
                    break;
                case UBLOCK_PAHAWH_HMONG:
                    aAllSubsets.push_back( Subset( 0x16B00, 0x16B8F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PAHAWH_HMONG)) ) );
                    break;
                case UBLOCK_PALMYRENE:
                    aAllSubsets.push_back( Subset( 0x10860, 0x1087F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PALMYRENE)) ) );
                    break;
                case UBLOCK_PAU_CIN_HAU:
                    aAllSubsets.push_back( Subset( 0x11AC0, 0x11AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PAU_CIN_HAU)) ) );
                    break;
                case UBLOCK_PSALTER_PAHLAVI:
                    aAllSubsets.push_back( Subset( 0x10B80, 0x10BAF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_PSALTER_PAHLAVI)) ) );
                    break;
                case UBLOCK_SHORTHAND_FORMAT_CONTROLS:
                    aAllSubsets.push_back( Subset( 0x1BCA0, 0x1BCAF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SHORTHAND_FORMAT_CONTROLS)) ) );
                    break;
                case UBLOCK_SIDDHAM:
                    aAllSubsets.push_back( Subset( 0x11580, 0x115FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SIDDHAM)) ) );
                    break;
                case UBLOCK_SINHALA_ARCHAIC_NUMBERS:
                    aAllSubsets.push_back( Subset( 0x111E0, 0x111FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SINHALA_ARCHAIC_NUMBERS)) ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_C:
                    aAllSubsets.push_back( Subset( 0x1F800, 0x1F8FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPLEMENTAL_ARROWS_C)) ) );
                    break;
                case UBLOCK_TIRHUTA:
                    aAllSubsets.push_back( Subset( 0x11480, 0x114DF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TIRHUTA)) ) );
                    break;
                case UBLOCK_WARANG_CITI:
                    aAllSubsets.push_back( Subset( 0x118A0, 0x118FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_WARANG_CITI)) ) );
                    break;
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 56
                case UBLOCK_AHOM:
                    aAllSubsets.push_back( Subset( 0x11700, 0x1173F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_AHOM)) ) );
                    break;
                case UBLOCK_ANATOLIAN_HIEROGLYPHS:
                    aAllSubsets.push_back( Subset( 0x14400, 0x1467F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ANATOLIAN_HIEROGLYPHS)) ) );
                    break;
                case UBLOCK_CHEROKEE_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0xAB70, 0xABBF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CHEROKEE_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E:
                    aAllSubsets.push_back( Subset( 0x2B820, 0x2CEAF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E)) ) );
                    break;
                case UBLOCK_EARLY_DYNASTIC_CUNEIFORM:
                    aAllSubsets.push_back( Subset( 0x12480, 0x1254F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_EARLY_DYNASTIC_CUNEIFORM)) ) );
                    break;
                case UBLOCK_HATRAN:
                    aAllSubsets.push_back( Subset( 0x108E0, 0x108FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_HATRAN)) ) );
                    break;
                case UBLOCK_MULTANI:
                    aAllSubsets.push_back( Subset( 0x11280, 0x112AF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MULTANI)) ) );
                    break;
                case UBLOCK_OLD_HUNGARIAN:
                    aAllSubsets.push_back( Subset( 0x10C80, 0x10CFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OLD_HUNGARIAN)) ) );
                    break;
                case UBLOCK_SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS:
                    aAllSubsets.push_back( Subset( 0x1F900, 0x1F9FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS)) ) );
                    break;
                case UBLOCK_SUTTON_SIGNWRITING:
                    aAllSubsets.push_back( Subset( 0x1D800, 0x1DAAF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_SUTTON_SIGNWRITING)) ) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 58)
                case UBLOCK_ADLAM:
                    aAllSubsets.push_back( Subset( 0x1E900, 0x1E95F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_ADLAM)) ) );
                    break;
                case UBLOCK_BHAIKSUKI:
                    aAllSubsets.push_back( Subset( 0x11C00, 0x11C6F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_BHAIKSUKI)) ) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_C:
                    aAllSubsets.push_back( Subset( 0x1C80, 0x1C8F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_CYRILLIC_EXTENDED_C)) ) );
                    break;
                case UBLOCK_GLAGOLITIC_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x1E000, 0x1E02F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_GLAGOLITIC_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION:
                    aAllSubsets.push_back( Subset( 0x16FE0, 0x16FFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION)) ) );
                    break;
                case UBLOCK_MARCHEN:
                    aAllSubsets.push_back( Subset( 0x11C70, 0x11CBF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MARCHEN)) ) );
                    break;
                case UBLOCK_MONGOLIAN_SUPPLEMENT:
                    aAllSubsets.push_back( Subset( 0x11660, 0x1167F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_MONGOLIAN_SUPPLEMENT)) ) );
                    break;
                case UBLOCK_NEWA:
                    aAllSubsets.push_back( Subset( 0x11400, 0x1147F, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_NEWA)) ) );
                    break;
                case UBLOCK_OSAGE:
                    aAllSubsets.push_back( Subset( 0x104B0, 0x104FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_OSAGE)) ) );
                    break;
                case UBLOCK_TANGUT:
                    aAllSubsets.push_back( Subset( 0x17000, 0x187FF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TANGUT)) ) );
                    break;
                case UBLOCK_TANGUT_COMPONENTS:
                    aAllSubsets.push_back( Subset( 0x18800, 0x18AFF, aStringList.GetString(aStringList.FindIndex(RID_SUBSETSTR_TANGUT_COMPONENTS)) ) );
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

void SubsetMap::ApplyCharMap( const FontCharMapRef& rxFontCharMap )
{
    if( !rxFontCharMap.is() )
        return;

    // remove subsets that are not matched in any range
    SubsetList::iterator it = maSubsets.begin();
    while(it != maSubsets.end())
    {
        const Subset& rSubset = *it;
        sal_uInt32 cMin = rSubset.GetRangeMin();
        sal_uInt32 cMax = rSubset.GetRangeMax();

        int nCount =  rxFontCharMap->CountCharsInRange( cMin, cMax );
        if( nCount <= 0 )
            it = maSubsets.erase(it);
        else
            ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
