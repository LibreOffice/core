/*************************************************************************
 *
 *  $RCSfile: charmap.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:45:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#include <stdio.h>

#define _SVX_CHARMAP_CXX_

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#pragma hdrstop

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include <rtl/textenc.h>
#include <ucsubset.hxx>

#include "dialogs.hrc"
#include "charmap.hrc"

#include "charmap.hxx"
#include "dialmgr.hxx"

#include "charmapacc.hxx"
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTOBJECT_HPP_
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#include "rtl/ustrbuf.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
//CHINA001 // class SvxShowText =====================================================
//CHINA001
//CHINA001 class SvxShowText : public Control
//CHINA001 {
//CHINA001 public:
//CHINA001 SvxShowText( Window* pParent,
//CHINA001 const ResId& rResId,
//CHINA001 BOOL bCenter = FALSE );
//CHINA001 ~SvxShowText();
//CHINA001
//CHINA001 void            SetFont( const Font& rFont );
//CHINA001 void            SetText( const String& rText );
//CHINA001
//CHINA001 protected:
//CHINA001 virtual void    Paint( const Rectangle& );
//CHINA001
//CHINA001 private:
//CHINA001 long            mnY;
//CHINA001 BOOL            mbCenter;
//CHINA001
//CHINA001 };

// class SvxCharMapData ==================================================

//CHINA001 class SvxCharMapData
//CHINA001 {
//CHINA001 public:
//CHINA001 SvxCharMapData( class SfxModalDialog* pDialog, BOOL bOne_ );
//CHINA001
//CHINA001 void            SetCharFont( const Font& rFont );
//CHINA001
//CHINA001 private:
//CHINA001 friend class SvxCharacterMap;
//CHINA001 SfxModalDialog* mpDialog;
//CHINA001
//CHINA001 SvxShowCharSet  aShowSet;
//CHINA001 //    Edit            aShowText;
//CHINA001 SvxShowText     aShowText;
//CHINA001 OKButton        aOKBtn;
//CHINA001 CancelButton    aCancelBtn;
//CHINA001 HelpButton      aHelpBtn;
//CHINA001 PushButton      aDeleteBtn;
//CHINA001 FixedText       aFontText;
//CHINA001 ListBox         aFontLB;
//CHINA001 FixedText       aSubsetText;
//CHINA001 ListBox         aSubsetLB;
//CHINA001 FixedText       aSymbolText;
//CHINA001 SvxShowText     aShowChar;
//CHINA001 FixedText       aCharCodeText;
//CHINA001
//CHINA001 Font            aFont;
//CHINA001 BOOL            bOne;
//CHINA001 const SubsetMap* pSubsetMap;
//CHINA001
//CHINA001 DECL_LINK( OKHdl, OKButton* );
//CHINA001 DECL_LINK( FontSelectHdl, ListBox* );
//CHINA001 DECL_LINK( SubsetSelectHdl, ListBox* );
//CHINA001 DECL_LINK( CharDoubleClickHdl, Control* pControl );
//CHINA001 DECL_LINK( CharSelectHdl, Control* pControl );
//CHINA001 DECL_LINK( CharHighlightHdl, Control* pControl );
//CHINA001 DECL_LINK( CharPreSelectHdl, Control* pControl );
//CHINA001 DECL_LINK( DeleteHdl, PushButton* pBtn );
//CHINA001 };


// -----------------------------------------------------------------------
sal_Unicode& getSelectedChar()
{
    static sal_Unicode cSelectedChar = ' '; // keeps selected character over app livetime
    return cSelectedChar;
}

// class SvxShowCharSet ==================================================

#define SBWIDTH 16

SvxShowCharSet::SvxShowCharSet( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId )
    ,aVscrollSB( this, WB_VERT)
    ,m_pAccessible(NULL)
{
    nSelectedIndex = -1;    // TODO: move into init list when it is no longer static

    aOrigSize = GetOutputSizePixel();
    aOrigPos = GetPosPixel();

    SetStyle( GetStyle() | WB_CLIPCHILDREN );
    aVscrollSB.SetScrollHdl( LINK( this, SvxShowCharSet, VscrollHdl ) );
    // other settings like aVscroll depend on selected font => see SetFont

    bDrag = FALSE;
    InitSettings( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::GetFocus()
{
    Control::GetFocus();
    SelectIndex( nSelectedIndex, TRUE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::LoseFocus()
{
    Control::LoseFocus();
    SelectIndex( nSelectedIndex, FALSE );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxShowCharSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS )
      && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
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
            bDrag = TRUE;
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
        bDrag = FALSE;
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

USHORT SvxShowCharSet::GetRowPos(USHORT _nPos) const
{
    return _nPos / COLUMN_COUNT ;
}

// -----------------------------------------------------------------------------

USHORT SvxShowCharSet::GetColumnPos(USHORT _nPos) const
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
    ULONG nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT;
    if( nIndex > maFontCharMap.GetCharCount() - 1 )
        nIndex = maFontCharMap.GetCharCount() - 1;
    return nIndex;
}

// -----------------------------------------------------------------------

inline Point SvxShowCharSet::MapIndexToPixel( int nIndex ) const
{
    const int nBase = FirstInView();
    int x = ((nIndex - nBase) % COLUMN_COUNT) * nX;
    int y = ((nIndex - nBase) / COLUMN_COUNT) * nY;
    return Point( x, y );
}
// -----------------------------------------------------------------------------

int SvxShowCharSet::PixelToMapIndex( const Point& point) const
{
    int nBase = FirstInView();
    return (nBase + (point.X()/nX) + (point.Y()/nY) * COLUMN_COUNT);
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
                sal_Unicode cChar = rKEvt.GetCharCode();
                sal_Unicode cNext = maFontCharMap.GetNextChar( cChar - 1 );
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
        SelectIndex( tmpSelected, TRUE );
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
// -----------------------------------------------------------------------

void SvxShowCharSet::DrawChars_Impl( int n1, int n2 )
{
    if( n1 > LastInView() || n2 < FirstInView() )
        return;

    Size aOutputSize = GetOutputSizePixel();
    if( aVscrollSB.IsVisible() )
        aOutputSize.setWidth( aOutputSize.Width() - SBWIDTH );

    int i;
    for ( i = 1; i < COLUMN_COUNT; ++i )
        DrawLine( Point( nX * i, 0 ), Point( nX * i, aOutputSize.Height() ) );
    for ( i = 1; i < ROW_COUNT; ++i )
        DrawLine( Point( 0, nY * i ), Point( aOutputSize.Width(), nY * i ) );

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    Color aWindowTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
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

        rtl::OUStringBuffer buf;
        buf.appendUtf32( maFontCharMap.GetCharFromIndex( i ) );
        String aCharStr(buf.makeStringAndClear());
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
                DrawRect( Rectangle( aPointUL, Size(nX-1,nY-1) ) );

                SetTextColor( aHighlightTextColor );
                DrawText( aPointTxTy, aCharStr );
            }
            else
            {
                SetFillColor( aFaceColor );
                DrawRect( Rectangle( aPointUL, Size( nX-1, nY-1) ) );

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

void SvxShowCharSet::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

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

sal_Unicode SvxShowCharSet::GetSelectCharacter() const
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

    Font aFont = rFont;
    aFont.SetWeight( WEIGHT_LIGHT );
    aFont.SetAlign( ALIGN_TOP );
    int nFontHeight = (aOrigSize.Height() - 5) * 2 / (3 * ROW_COUNT);
    aFont.SetSize( PixelToLogic( Size( 0, nFontHeight ) ) );
    aFont.SetTransparent( TRUE );
    Control::SetFont( aFont );
    GetFontCharMap( maFontCharMap );

    // hide scrollbar when there is nothing to scroll
    BOOL bNeedVscroll = (maFontCharMap.GetCharCount() > ROW_COUNT*COLUMN_COUNT);

    nX = (aOrigSize.Width() - (bNeedVscroll ? SBWIDTH : 0)) / COLUMN_COUNT;
    nY = aOrigSize.Height() / ROW_COUNT;

    if( bNeedVscroll)
    {
        aVscrollSB.SetPosSizePixel( nX * COLUMN_COUNT, 0, SBWIDTH, nY * ROW_COUNT );
        aVscrollSB.SetRangeMin( 0 );
        int nLastRow = (maFontCharMap.GetCharCount() - 1 + COLUMN_COUNT) / COLUMN_COUNT;
        aVscrollSB.SetRangeMax( nLastRow );
        aVscrollSB.SetPageSize( ROW_COUNT-1 );
        aVscrollSB.SetVisibleSize( ROW_COUNT );
    }

    // restore last selected unicode
    int nMapIndex = maFontCharMap.GetIndexFromChar( getSelectedChar() );
    SelectIndex( nMapIndex );

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aNewSize( nX * COLUMN_COUNT + (bNeedVscroll ? SBWIDTH : 0), nY * ROW_COUNT );
    Point aNewPos = aOrigPos + Point( (aOrigSize.Width() - aNewSize.Width()) / 2, 0 );
    SetPosPixel( aNewPos );
    SetOutputSizePixel( aNewSize );

    aVscrollSB.Show( bNeedVscroll );
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxShowCharSet::SelectIndex( int nNewIndex, BOOL bFocus )
{
    if( nNewIndex < 0 )
    {
        // need to scroll see closest unicode
        sal_Unicode cPrev = maFontCharMap.GetPrevChar( getSelectedChar() );
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
        DrawRect( Rectangle( aOldPixel, Size( nX-1, nY-1 ) ) );
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

void SvxShowCharSet::SelectCharacter( sal_Unicode cNew, BOOL bFocus )
{
    // get next available char of current font
    sal_Unicode cNext = maFontCharMap.GetNextChar( cNew - 1 );

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

IMPL_LINK( SvxShowCharSet, VscrollHdl, ScrollBar *, EMPTYARG )
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
            sal_Int32 nLast = LastInView();
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
::svx::SvxShowCharSetItem* SvxShowCharSet::ImplGetItem( USHORT _nPos )
{
    ItemsMap::iterator aFind = m_aItems.find(_nPos);
    if ( aFind == m_aItems.end() )
    {
        OSL_ENSURE(m_pAccessible,"Who wants to create a child of my table without a parent?");
        aFind = m_aItems.insert(ItemsMap::value_type(_nPos,new ::svx::SvxShowCharSetItem(*this,m_pAccessible->getTable(),_nPos))).first;
        rtl::OUStringBuffer buf;
        buf.appendUtf32( maFontCharMap.GetCharFromIndex( _nPos ) );
        aFind->second->maText = buf.makeStringAndClear();
        Point pix = MapIndexToPixel( _nPos );
        aFind->second->maRect = Rectangle( Point( pix.X() + 1, pix.Y() + 1 ), Size(nX-1,nY-1) );
    }

    return aFind->second;
}
// -----------------------------------------------------------------------------
void SvxShowCharSet::ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue )
{
    if( m_pAccessible )
        m_pAccessible->fireEvent( nEventId, rOldValue, rNewValue );
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

// class SvxShowText =====================================================

SvxShowText::SvxShowText( Window* pParent, const ResId& rResId, BOOL bCenter )
:   Control( pParent, rResId ),
    mbCenter( bCenter)
{}

// -----------------------------------------------------------------------

void SvxShowText::Paint( const Rectangle& )
{
    Color aTextCol = GetTextColor();

    svtools::ColorConfig aColorConfig;
    Color aWindowTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    SetTextColor( aWindowTextColor );

    const String aText = GetText();
    const Size aSize = GetOutputSizePixel();
    Point aPoint( 2, mnY );

    // adjust position using ink boundary if possible
    Rectangle aBoundRect;
    if( !GetTextBoundRect( aBoundRect, aText ) || aBoundRect.IsEmpty() )
        aPoint.X() = (aSize.Width() - GetTextWidth( aText )) / 2;
    else
    {
        // adjust position before it gets out of bounds
        aBoundRect += aPoint;

        // shift back vertically if needed
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if( nYLDelta <= 0 )
            aPoint.Y() -= nYLDelta - 1;
        else if( nYHDelta <= 0 )
            aPoint.Y() += nYHDelta - 1;

        if( mbCenter )
        {
            // move glyph to middle of cell
            aPoint.X() = -aBoundRect.Left()
                       + (aSize.Width() - aBoundRect.GetWidth()) / 2;
        }
        else
        {
            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left();
            int nXHDelta = aSize.Width() - aBoundRect.Right();
            if( nXLDelta <= 0 )
                aPoint.X() -= nXLDelta - 1;
            else if( nXHDelta <= 0 )
                aPoint.X() += nXHDelta - 1;
        }
    }

    DrawText( aPoint, aText );
    SetTextColor( aTextCol );
}

// -----------------------------------------------------------------------

void SvxShowText::SetFont( const Font& rFont )
{
    long nWinHeight = GetOutputSizePixel().Height();
    Font aFont = rFont;
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetAlign( ALIGN_TOP );
    aFont.SetSize( PixelToLogic( Size( 0, nWinHeight/2 ) ) );
    aFont.SetTransparent( TRUE );
    Control::SetFont( aFont );
    mnY = ( nWinHeight - GetTextHeight() ) / 2;

    Invalidate();
}

// -----------------------------------------------------------------------

void SvxShowText::SetText( const String& rText )
{
    Control::SetText( rText );
    Invalidate();
}

// -----------------------------------------------------------------------

SvxShowText::~SvxShowText()
{}

//CHINA001 // class SvxCharacterMap =================================================
//CHINA001
//CHINA001 SvxCharacterMap::SvxCharacterMap( Window* pParent, BOOL bOne ) :
//CHINA001 SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_CHARMAP ) ),
//CHINA001 mpCharMapData( new SvxCharMapData( this, bOne ) )
//CHINA001 {
//CHINA001 FreeResource();
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 SvxCharacterMap::~SvxCharacterMap()
//CHINA001 {
//CHINA001 delete mpCharMapData;
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 const Font& SvxCharacterMap::GetCharFont() const
//CHINA001 {
//CHINA001 return mpCharMapData->aFont;
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 void SvxCharacterMap::SetChar( sal_Unicode c )
//CHINA001 {
//CHINA001 mpCharMapData->aShowSet.SelectCharacter( c );
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 sal_Unicode SvxCharacterMap::GetChar() const
//CHINA001 {
//CHINA001 return mpCharMapData->aShowSet.GetSelectCharacter();
//CHINA001 }
//CHINA001
//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 String SvxCharacterMap::GetCharacters() const
//CHINA001 {
//CHINA001 return mpCharMapData->aShowText.GetText();
//CHINA001 }
//CHINA001
// =======================================================================

SvxCharMapData::SvxCharMapData( SfxModalDialog* pDialog, BOOL bOne_ )
:   mpDialog( pDialog ),
    aShowSet        ( pDialog, ResId( CT_SHOWSET ) ),
    aFontText       ( pDialog, ResId( FT_FONT ) ),
    aFontLB         ( pDialog, ResId( LB_FONT ) ),
    aSubsetText     ( pDialog, ResId( FT_SUBSET ) ),
    aSubsetLB       ( pDialog, ResId( LB_SUBSET ) ),
    aSymbolText     ( pDialog, ResId( FT_SYMBOLE ) ),
    aShowText       ( pDialog, ResId( CT_SHOWTEXT ) ),
    aShowChar       ( pDialog, ResId( CT_SHOWCHAR ), TRUE ),
    aCharCodeText   ( pDialog, ResId( FT_CHARCODE ) ),
    aOKBtn          ( pDialog, ResId( BTN_CHAR_OK ) ),
    aCancelBtn      ( pDialog, ResId( BTN_CHAR_CANCEL ) ),
    aHelpBtn        ( pDialog, ResId( BTN_CHAR_HELP ) ),
    aDeleteBtn      ( pDialog, ResId( BTN_DELETE ) ),
    pSubsetMap( NULL ),
    bOne( bOne_ )
{
    aFont = pDialog->GetFont();
    aFont.SetTransparent( TRUE );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );

    if ( bOne )
    {
        Size aDlgSize = pDialog->GetSizePixel();
        pDialog->SetSizePixel( Size( aDlgSize.Width(),
            aDlgSize.Height()-aShowText.GetSizePixel().Height() ) );
        aSymbolText.Hide();
        aShowText.Hide();
        aDeleteBtn.Hide();
    }

    String aDefStr( aFont.GetName() );
    String aLastName;
    xub_StrLen i;
    xub_StrLen nCount = mpDialog->GetDevFontCount();
    for ( i = 0; i < nCount; i++ )
    {
        String aFontName( mpDialog->GetDevFont( i ).GetName() );
        if ( aFontName != aLastName )
        {
            aLastName = aFontName;
            USHORT nPos = aFontLB.InsertEntry( aFontName );
            aFontLB.SetEntryData( nPos, (void*)(ULONG)i );
        }
    }
    // the font may not be in the list =>
    // try to find a font name token in list and select found font,
    // else select topmost entry
    FASTBOOL bFound = (aFontLB.GetEntryPos( aDefStr ) == LISTBOX_ENTRY_NOTFOUND );
    if( !bFound )
    {
        for ( i = 0; i < aDefStr.GetTokenCount(); ++i )
        {
            String aToken = aDefStr.GetToken(i);
            if ( aFontLB.GetEntryPos( aToken ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aDefStr = aToken;
                bFound = TRUE;
                break;
            }
        }
    }

    if ( bFound )
        aFontLB.SelectEntry( aDefStr );
    else if ( aFontLB.GetEntryCount() )
        aFontLB.SelectEntryPos(0);
    FontSelectHdl( &aFontLB );

    aOKBtn.SetClickHdl( LINK( this, SvxCharMapData, OKHdl ) );
    aFontLB.SetSelectHdl( LINK( this, SvxCharMapData, FontSelectHdl ) );
    aSubsetLB.SetSelectHdl( LINK( this, SvxCharMapData, SubsetSelectHdl ) );
    aShowSet.SetDoubleClickHdl( LINK( this, SvxCharMapData, CharDoubleClickHdl ) );
    aShowSet.SetSelectHdl( LINK( this, SvxCharMapData, CharSelectHdl ) );
    aShowSet.SetHighlightHdl( LINK( this, SvxCharMapData, CharHighlightHdl ) );
    aShowSet.SetPreSelectHdl( LINK( this, SvxCharMapData, CharPreSelectHdl ) );
    aDeleteBtn.SetClickHdl( LINK( this, SvxCharMapData, DeleteHdl ) );

    aOKBtn.Disable();

    // left align aShowText field
    int nLeftEdge = aSymbolText.GetPosPixel().X();
    nLeftEdge += aSymbolText.GetTextWidth( aSymbolText.GetText() );
    Size aNewSize = aShowText.GetOutputSizePixel();
    aShowText.SetPosPixel( Point( nLeftEdge+4, aShowText.GetPosPixel().Y() ) );
}

//CHINA001 // -----------------------------------------------------------------------
//CHINA001
//CHINA001 void SvxCharacterMap::DisableFontSelection()
//CHINA001 {
//CHINA001 mpCharMapData->aFontText.Disable();
//CHINA001 mpCharMapData->aFontLB.Disable();
//CHINA001 }
//CHINA001
// -----------------------------------------------------------------------

void SvxCharMapData::SetCharFont( const Font& rFont )
{
    // first get the underlying info in order to get font names
    // like "Times New Roman;Times" resolved
    Font aTmp( mpDialog->GetFontMetric( rFont ) );

    if ( aFontLB.GetEntryPos( aTmp.GetName() ) == LISTBOX_ENTRY_NOTFOUND )
        return;

    aFontLB.SelectEntry( aTmp.GetName() );
    aFont = aTmp;
    FontSelectHdl( &aFontLB );

    // for compatibility reasons
    mpDialog->ModalDialog::SetFont( aFont );
}

// -----------------------------------------------------------------------

//CHINA001 void SvxCharacterMap::SetCharFont( const Font& rFont )
//CHINA001 {
//CHINA001 mpCharMapData->SetCharFont( rFont );
//CHINA001 }

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, OKHdl, OKButton *, EMPTYARG )
{
    String aStr = aShowText.GetText();

    if ( !aStr.Len() )
    {
        if ( aShowSet.GetSelectCharacter() > 0 )
            aStr = aShowSet.GetSelectCharacter();
        aShowText.SetText( aStr );
    }
    mpDialog->EndDialog( TRUE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, FontSelectHdl, ListBox *, EMPTYARG )
{
    USHORT nPos = aFontLB.GetSelectEntryPos(),
        nFont = (USHORT)(ULONG)aFontLB.GetEntryData( nPos );
    aFont = mpDialog->GetDevFont( nFont );

    // notify children using this font
    aShowSet.SetFont( aFont );
    aShowChar.SetFont( aFont );
    aShowText.SetFont( aFont );

    // right align some fields to aShowSet
    int nRightEdge = aShowSet.GetPosPixel().X() + aShowSet.GetOutputSizePixel().Width();
    Size aNewSize = aSubsetLB.GetOutputSizePixel();
    aNewSize.setWidth( nRightEdge - aSubsetLB.GetPosPixel().X() );
    aSubsetLB.SetOutputSizePixel( aNewSize );

    // setup unicode subset listbar with font specific subsets,
    // hide unicode subset listbar for symbol fonts
    // TODO: get info from the Font once it provides it
    if( pSubsetMap)
        delete pSubsetMap;
    pSubsetMap = NULL;

    BOOL bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if( bNeedSubset )
    {
        FontCharMap aFontCharMap;
        aShowSet.GetFontCharMap( aFontCharMap );
        pSubsetMap = new SubsetMap( &aFontCharMap );

        // update subset listbox for new font's unicode subsets
        aSubsetLB.Clear();
        // TODO: is it worth to improve the stupid linear search?
        bool bFirst = true;
        const Subset* s;
        while( NULL != (s = pSubsetMap->GetNextSubset( bFirst ))  )
        {
            USHORT nPos = aSubsetLB.InsertEntry( s->GetName() );
            aSubsetLB.SetEntryData( nPos, (void*)s );
            // NOTE: subset must live at least as long as the selected font
            if( bFirst )
                aSubsetLB.SelectEntryPos( nPos );
            bFirst = false;
        }
        if( aSubsetLB.GetEntryCount() <= 1 )
            bNeedSubset = FALSE;
    }

    aSubsetText.Show( bNeedSubset);
    aSubsetLB.Show( bNeedSubset);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, SubsetSelectHdl, ListBox *, EMPTYARG )
{
    USHORT nPos = aSubsetLB.GetSelectEntryPos();
    const Subset* pSubset = reinterpret_cast<const Subset*> (aSubsetLB.GetEntryData(nPos));
    if( pSubset )
    {
        sal_Unicode cFirst = pSubset->GetRangeMin();
        aShowSet.SelectCharacter( cFirst );
    }
    aSubsetLB.SelectEntryPos( nPos );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, CharDoubleClickHdl, Control *, EMPTYARG )
{
    mpDialog->EndDialog( TRUE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, CharSelectHdl, Control *, EMPTYARG )
{
    if ( !bOne )
    {
        String aText = aShowText.GetText();

        if ( aText.Len() == CHARMAP_MAXLEN )
            Sound::Beep( SOUND_WARNING );
        else
        {
            if ( aShowSet.GetSelectCharacter() > 0 )
                aText += aShowSet.GetSelectCharacter();
            aShowText.SetText( aText );
        }

    }
    aOKBtn.Enable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, CharHighlightHdl, Control *, EMPTYARG )
{
    String aTemp;
    sal_Unicode c = aShowSet.GetSelectCharacter();
    sal_Bool bSelect = ( c > 0 );
    if ( bSelect )
    {
        aTemp = c;
        const Subset* pSubset = NULL;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( c );
        if( pSubset )
            aSubsetLB.SelectEntry( pSubset->GetName() );
        else
            aSubsetLB.SetNoSelection();
    }
    aShowChar.SetText( aTemp );
    aShowChar.Update();
    if ( bSelect )
    {
        // no sprintf or hex-formatter around :-(
        char buf[16] = "U+0000";
        sal_Unicode c_Shifted = c;
        for( int i = 0; i < 4; ++i )
        {
            char h = c_Shifted & 0x0F;
            buf[5-i] = (h > 9) ? (h - 10 + 'A') : (h + '0');
            c_Shifted >>= 4;
        }
        if( c < 256 )
            snprintf( buf+6, 10, " (%d)", c );
        aTemp = String::CreateFromAscii( buf );
    }
    aCharCodeText.SetText( aTemp );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, CharPreSelectHdl, Control *, EMPTYARG )
{
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_Unicode cChar = aShowSet.GetSelectCharacter();
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            aSubsetLB.SelectEntry( pSubset->GetName() );
    }

    aOKBtn.Enable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharMapData, DeleteHdl, PushButton *, EMPTYARG )
{
    aShowText.SetText( String() );
    aOKBtn.Disable();
    return 0;
}

// class SubsetMap =======================================================
// TODO: should be moved into Font Attributes stuff
// we let it mature here though because it is currently the only use

SubsetMap::SubsetMap( const FontCharMap* pFontCharMap )
:   Resource( ResId( SVX_RES(RID_SUBSETMAP) ) )
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

const Subset* SubsetMap::GetSubsetByUnicode( sal_Unicode cChar ) const
{
    // TODO: is it worth to avoid a linear search?
    for( const Subset* s = GetNextSubset( true ); s; s = GetNextSubset( false ) )
        if( (s->GetRangeMin() <= cChar) && (cChar <= s->GetRangeMax()) )
            return s;
    return NULL;
}

inline Subset::Subset( sal_Unicode nMin, sal_Unicode nMax, int resId)
:   mnRangeMin(nMin), mnRangeMax(nMax), maRangeName( ResId(resId) )
{}

void SubsetMap::InitList()
{
    static SubsetList aAllSubsets;
    static bool bInit = true;
    if( bInit )
    {
        bInit = false;

        // TODO: eventually merge or split unicode subranges
        //       a "native writer" should decide for his subsets
        aAllSubsets.push_back( Subset( 0x0020, 0x007F, RID_SUBSETSTR_BASIC_LATIN ) );
        aAllSubsets.push_back( Subset( 0x0080, 0x00FF, RID_SUBSETSTR_LATIN_1 ) );
        aAllSubsets.push_back( Subset( 0x0100, 0x017F, RID_SUBSETSTR_LATIN_EXTENDED_A ) );
        aAllSubsets.push_back( Subset( 0x0180, 0x024F, RID_SUBSETSTR_LATIN_EXTENDED_B ) );
        aAllSubsets.push_back( Subset( 0x0250, 0x02AF, RID_SUBSETSTR_IPA_EXTENSIONS ) );
        aAllSubsets.push_back( Subset( 0x02B0, 0x02FF, RID_SUBSETSTR_SPACING_MODIFIERS ) );
        aAllSubsets.push_back( Subset( 0x0300, 0x036F, RID_SUBSETSTR_COMB_DIACRITICAL ) );
        aAllSubsets.push_back( Subset( 0x0370, 0x03FF, RID_SUBSETSTR_BASIC_GREEK ) );
    //  aAllSubsets.push_back( Subset( 0x03D0, 0x03F3, RID_SUBSETSTR_GREEK_SYMS_COPTIC ) );
        aAllSubsets.push_back( Subset( 0x0400, 0x04FF, RID_SUBSETSTR_CYRILLIC ) );
        aAllSubsets.push_back( Subset( 0x0530, 0x058F, RID_SUBSETSTR_ARMENIAN ) );
        aAllSubsets.push_back( Subset( 0x0590, 0x05FF, RID_SUBSETSTR_BASIC_HEBREW ) );
    //  aAllSubsets.push_back( Subset( 0x0591, 0x05C4, RID_SUBSETSTR_HEBREW_EXTENDED ) );
        aAllSubsets.push_back( Subset( 0x0600, 0x065F, RID_SUBSETSTR_BASIC_ARABIC ) );
        aAllSubsets.push_back( Subset( 0x0660, 0x06FF, RID_SUBSETSTR_ARABIC_EXTENDED ) );
        aAllSubsets.push_back( Subset( 0x0700, 0x074F, RID_SUBSETSTR_SYRIAC ) );
        aAllSubsets.push_back( Subset( 0x0780, 0x07BF, RID_SUBSETSTR_THAANA ) );
        aAllSubsets.push_back( Subset( 0x0900, 0x097F, RID_SUBSETSTR_DEVANAGARI ) );
        aAllSubsets.push_back( Subset( 0x0980, 0x09FF, RID_SUBSETSTR_BENGALI ) );
        aAllSubsets.push_back( Subset( 0x0A00, 0x0A7F, RID_SUBSETSTR_GURMUKHI ) );
        aAllSubsets.push_back( Subset( 0x0A80, 0x0AFF, RID_SUBSETSTR_GUJARATI ) );
        aAllSubsets.push_back( Subset( 0x0B00, 0x0B7F, RID_SUBSETSTR_ORIYA ) );
        aAllSubsets.push_back( Subset( 0x0B80, 0x0BFF, RID_SUBSETSTR_TAMIL ) );
        aAllSubsets.push_back( Subset( 0x0C00, 0x0C7F, RID_SUBSETSTR_TELUGU ) );
        aAllSubsets.push_back( Subset( 0x0C80, 0x0CFF, RID_SUBSETSTR_KANNADA ) );
        aAllSubsets.push_back( Subset( 0x0D00, 0x0D7F, RID_SUBSETSTR_MALAYALAM ) );
        aAllSubsets.push_back( Subset( 0x0D80, 0x0DFF, RID_SUBSETSTR_SINHALA ) );
        aAllSubsets.push_back( Subset( 0x0E00, 0x0E7F, RID_SUBSETSTR_THAI ) );
        aAllSubsets.push_back( Subset( 0x0E80, 0x0EFF, RID_SUBSETSTR_LAO ) );
        aAllSubsets.push_back( Subset( 0x0F00, 0x0FBF, RID_SUBSETSTR_TIBETAN ) );
        aAllSubsets.push_back( Subset( 0x1000, 0x109F, RID_SUBSETSTR_MYANMAR ) );
        aAllSubsets.push_back( Subset( 0x10A0, 0x10FF, RID_SUBSETSTR_BASIC_GEORGIAN ) );
    //  aAllSubsets.push_back( Subset( 0x10A0, 0x10C5, RID_SUBSETSTR_GEORGIAN_EXTENDED ) );
        aAllSubsets.push_back( Subset( 0x1100, 0x11FF, RID_SUBSETSTR_HANGUL_JAMO ) );
        aAllSubsets.push_back( Subset( 0x1200, 0x137F, RID_SUBSETSTR_ETHIOPIC ) );
        aAllSubsets.push_back( Subset( 0x13A0, 0x13FF, RID_SUBSETSTR_CHEROKEE ) );
        aAllSubsets.push_back( Subset( 0x1400, 0x167F, RID_SUBSETSTR_CANADIAN_ABORIGINAL ) );
        aAllSubsets.push_back( Subset( 0x1680, 0x169F, RID_SUBSETSTR_OGHAM ) );
        aAllSubsets.push_back( Subset( 0x16A0, 0x16F0, RID_SUBSETSTR_RUNIC ) );
        aAllSubsets.push_back( Subset( 0x1700, 0x171F, RID_SUBSETSTR_TAGALOG ) );
        aAllSubsets.push_back( Subset( 0x1720, 0x173F, RID_SUBSETSTR_HANUNOO ) );
        aAllSubsets.push_back( Subset( 0x1740, 0x175F, RID_SUBSETSTR_BUHID ) );
        aAllSubsets.push_back( Subset( 0x1760, 0x177F, RID_SUBSETSTR_TAGBANWA ) );
        aAllSubsets.push_back( Subset( 0x1780, 0x17FF, RID_SUBSETSTR_KHMER ) );
        aAllSubsets.push_back( Subset( 0x1800, 0x18AF, RID_SUBSETSTR_MONGOLIAN ) );
        aAllSubsets.push_back( Subset( 0x1E00, 0x1EFF, RID_SUBSETSTR_LATIN_EXTENDED_ADDS ) );
        aAllSubsets.push_back( Subset( 0x1F00, 0x1FFF, RID_SUBSETSTR_GREEK_EXTENDED ) );

        aAllSubsets.push_back( Subset( 0x2000, 0x206F, RID_SUBSETSTR_GENERAL_PUNCTUATION ) );
        aAllSubsets.push_back( Subset( 0x2070, 0x209F, RID_SUBSETSTR_SUB_SUPER_SCRIPTS ) );
        aAllSubsets.push_back( Subset( 0x20A0, 0x20CF, RID_SUBSETSTR_CURRENCY_SYMBOLS ) );
        aAllSubsets.push_back( Subset( 0x20D0, 0x20FF, RID_SUBSETSTR_COMB_DIACRITIC_SYMS ) );
        aAllSubsets.push_back( Subset( 0x2100, 0x214F, RID_SUBSETSTR_LETTERLIKE_SYMBOLS ) );
        aAllSubsets.push_back( Subset( 0x2150, 0x218F, RID_SUBSETSTR_NUMBER_FORMS ) );
        aAllSubsets.push_back( Subset( 0x2190, 0x21FF, RID_SUBSETSTR_ARROWS ) );
        aAllSubsets.push_back( Subset( 0x2200, 0x22FF, RID_SUBSETSTR_MATH_OPERATORS ) );
        aAllSubsets.push_back( Subset( 0x2300, 0x23FF, RID_SUBSETSTR_MISC_TECHNICAL ) );
        aAllSubsets.push_back( Subset( 0x2400, 0x243F, RID_SUBSETSTR_CONTROL_PICTURES ) );
        aAllSubsets.push_back( Subset( 0x2440, 0x245F, RID_SUBSETSTR_OPTICAL_CHAR_REC ) );
        aAllSubsets.push_back( Subset( 0x2460, 0x24FF, RID_SUBSETSTR_ENCLOSED_ALPHANUM ) );
        aAllSubsets.push_back( Subset( 0x2500, 0x257F, RID_SUBSETSTR_BOX_DRAWING ) );
        aAllSubsets.push_back( Subset( 0x2580, 0x259F, RID_SUBSETSTR_BLOCK_ELEMENTS ) );
        aAllSubsets.push_back( Subset( 0x25A0, 0x25FF, RID_SUBSETSTR_GEOMETRIC_SHAPES ) );
        aAllSubsets.push_back( Subset( 0x2600, 0x26FF, RID_SUBSETSTR_MISC_DINGBATS ) );
        aAllSubsets.push_back( Subset( 0x2700, 0x27BF, RID_SUBSETSTR_DINGBATS ) );

        aAllSubsets.push_back( Subset( 0x27C0, 0x27FF, RID_SUBSETSTR_MISC_MATH_SYMS_A ) );
        aAllSubsets.push_back( Subset( 0x27F0, 0x27FF, RID_SUBSETSTR_SUPPL_ARROWS_A ) );
        aAllSubsets.push_back( Subset( 0x2800, 0x28FF, RID_SUBSETSTR_BRAILLE_PATTERNS ) );
        aAllSubsets.push_back( Subset( 0x2900, 0x297F, RID_SUBSETSTR_SUPPL_ARROWS_B ) );
        aAllSubsets.push_back( Subset( 0x2980, 0x29FF, RID_SUBSETSTR_MISC_MATH_SYMS_B ) );
        aAllSubsets.push_back( Subset( 0x2E80, 0x2EFF, RID_SUBSETSTR_CJK_RADICAL_SUPPL ) );
        aAllSubsets.push_back( Subset( 0x2F00, 0x2FDF, RID_SUBSETSTR_KANXI_RADICALS ) );
        aAllSubsets.push_back( Subset( 0x2FF0, 0x2FFF, RID_SUBSETSTR_IDEO_DESC_CHARS ) );

        aAllSubsets.push_back( Subset( 0x3000, 0x303F, RID_SUBSETSTR_CJK_SYMS_PUNCTUATION ) );
        aAllSubsets.push_back( Subset( 0x3040, 0x309F, RID_SUBSETSTR_HIRAGANA ) );
        aAllSubsets.push_back( Subset( 0x30A0, 0x30FF, RID_SUBSETSTR_KATAKANA ) );
        aAllSubsets.push_back( Subset( 0x3100, 0x312F, RID_SUBSETSTR_BOPOMOFO ) );
        aAllSubsets.push_back( Subset( 0x3130, 0x318F, RID_SUBSETSTR_HANGUL_COMPAT_JAMO ) );
        aAllSubsets.push_back( Subset( 0x3190, 0x319F, RID_SUBSETSTR_KANBUN ) );
        aAllSubsets.push_back( Subset( 0x31A0, 0x31BF, RID_SUBSETSTR_BOPOMOFO_EXTENDED ) );
        aAllSubsets.push_back( Subset( 0x31C0, 0x31FF, RID_SUBSETSTR_KATAKANA_PHONETIC ) );
        aAllSubsets.push_back( Subset( 0x3200, 0x32FF, RID_SUBSETSTR_ENCLOSED_CJK_LETTERS ) );
        aAllSubsets.push_back( Subset( 0x3300, 0x33FF, RID_SUBSETSTR_CJK_COMPATIBILITY ) );

        aAllSubsets.push_back( Subset( 0x3400, 0x4DFF, RID_SUBSETSTR_CJK_EXT_A_UNIFIED_IDGRAPH ) );
        aAllSubsets.push_back( Subset( 0x4E00, 0x9FA5, RID_SUBSETSTR_CJK_UNIFIED_IDGRAPH ) );
        aAllSubsets.push_back( Subset( 0xA000, 0xA4CF, RID_SUBSETSTR_YI ) );
        aAllSubsets.push_back( Subset( 0xAC00, 0xB097, RID_SUBSETSTR_HANGUL_GA ) );
        aAllSubsets.push_back( Subset( 0xB098, 0xB2E3, RID_SUBSETSTR_HANGUL_NA ) );
        aAllSubsets.push_back( Subset( 0xB2E4, 0xB77B, RID_SUBSETSTR_HANGUL_DA ) );
        aAllSubsets.push_back( Subset( 0xB77C, 0xB9C7, RID_SUBSETSTR_HANGUL_RA ) );
        aAllSubsets.push_back( Subset( 0xB9C8, 0xBC13, RID_SUBSETSTR_HANGUL_MA ) );
        aAllSubsets.push_back( Subset( 0xBC14, 0xC0AB, RID_SUBSETSTR_HANGUL_BA ) );
        aAllSubsets.push_back( Subset( 0xC0AC, 0xC543, RID_SUBSETSTR_HANGUL_SA ) );
        aAllSubsets.push_back( Subset( 0xC544, 0xC78F, RID_SUBSETSTR_HANGUL_AH ) );
        aAllSubsets.push_back( Subset( 0xC790, 0xCC27, RID_SUBSETSTR_HANGUL_JA ) );
        aAllSubsets.push_back( Subset( 0xCC28, 0xCE73, RID_SUBSETSTR_HANGUL_CHA ) );
        aAllSubsets.push_back( Subset( 0xCE74, 0xD0BF, RID_SUBSETSTR_HANGUL_KA ) );
        aAllSubsets.push_back( Subset( 0xD0C0, 0xD30B, RID_SUBSETSTR_HANGUL_TA ) );
        aAllSubsets.push_back( Subset( 0xD30C, 0xD557, RID_SUBSETSTR_HANGUL_PA ) );
        aAllSubsets.push_back( Subset( 0xD558, 0xD7A3, RID_SUBSETSTR_HANGUL_HA ) );
    //  aAllSubsets.push_back( Subset( 0xAC00, 0xD7AF, RID_SUBSETSTR_HANGUL ) );

    //  aAllSubsets.push_back( Subset( 0xD800, 0xDFFF, RID_SUBSETSTR_SURROGATE ) );
        aAllSubsets.push_back( Subset( 0xE000, 0xF8FF, RID_SUBSETSTR_PRIVATE_USE_AREA ) );
        aAllSubsets.push_back( Subset( 0xF900, 0xFAFF, RID_SUBSETSTR_CJK_COMPAT_IDGRAPHS ) );
        aAllSubsets.push_back( Subset( 0xFB00, 0xFB4F, RID_SUBSETSTR_ALPHA_PRESENTATION ) );
        aAllSubsets.push_back( Subset( 0xFB50, 0xFDFF, RID_SUBSETSTR_ARABIC_PRESENT_A ) );
        aAllSubsets.push_back( Subset( 0xFE20, 0xFE2F, RID_SUBSETSTR_COMBINING_HALF_MARKS ) );
        aAllSubsets.push_back( Subset( 0xFE30, 0xFE4F, RID_SUBSETSTR_CJK_COMPAT_FORMS ) );
        aAllSubsets.push_back( Subset( 0xFE50, 0xFE6F, RID_SUBSETSTR_SMALL_FORM_VARIANTS ) );
        aAllSubsets.push_back( Subset( 0xFE70, 0xFEFF, RID_SUBSETSTR_ARABIC_PRESENT_B ) );
        aAllSubsets.push_back( Subset( 0xFF00, 0xFFEF, RID_SUBSETSTR_HALFW_FULLW_FORMS ) );
        aAllSubsets.push_back( Subset( 0xFFF0, 0xFFFF, RID_SUBSETSTR_SPECIALS ) );
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

