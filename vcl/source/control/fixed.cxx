/*************************************************************************
 *
 *  $RCSfile: fixed.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-27 16:12:36 $
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

#define _SV_FIXED_CXX

#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <fixed.hxx>
#endif

#include <rc.h>

#pragma hdrstop

// =======================================================================

#define FIXEDLINE_TEXT_BORDER    4

#define FIXEDTEXT_VIEW_STYLE    (WB_3DLOOK |                        \
                                 WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                 WB_WORDBREAK | WB_NOLABEL |        \
                                 WB_INFO | WB_PATHELLIPSIS)
#define FIXEDLINE_VIEW_STYLE    (WB_3DLOOK | WB_NOLABEL)
#define FIXEDBITMAP_VIEW_STYLE  (WB_3DLOOK |                        \
                                 WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                 WB_SCALE)
#define FIXEDIMAGE_VIEW_STYLE   (WB_3DLOOK |                        \
                                 WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                 WB_SCALE)

// =======================================================================

static Point ImplCalcPos( WinBits nStyle, const Point& rPos,
                          const Size& rObjSize, const Size& rWinSize )
{
    long    nX;
    long    nY;

    if ( nStyle & WB_LEFT )
        nX = 0;
    else if ( nStyle & WB_RIGHT )
        nX = rWinSize.Width()-rObjSize.Width();
    else
        nX = (rWinSize.Width()-rObjSize.Width())/2;

    if ( nStyle & WB_TOP )
        nY = 0;
    else if ( nStyle & WB_BOTTOM )
        nX = rWinSize.Height()-rObjSize.Height();
    else
        nY = (rWinSize.Height()-rObjSize.Height())/2;

    if ( nStyle & WB_TOPLEFTVISIBLE )
    {
        if ( nX < 0 )
            nX = 0;
        if ( nY < 0 )
            nY = 0;
    }

    Point aPos( nX+rPos.X(), nY+rPos.Y() );
    return aPos;
}

// =======================================================================

void FixedText::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits FixedText::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void FixedText::ImplInitSettings( BOOL bFont,
                                  BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont;
        if ( GetStyle() & WB_INFO )
            aFont = rStyleSettings.GetInfoFont();
        else
            aFont = rStyleSettings.GetLabelFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
        {
            if ( GetStyle() & WB_INFO )
                aColor = rStyleSettings.GetInfoTextColor();
            else
                aColor = rStyleSettings.GetLabelTextColor();
        }
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Window* pParent = GetParent();
        if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
        }
        else
        {
            EnableChildTransparentMode( FALSE );
            SetParentClipMode( 0 );
            SetPaintTransparent( FALSE );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

// -----------------------------------------------------------------------

FixedText::FixedText( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDTEXT )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedText::FixedText( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDTEXT )
{
    rResId.SetRT( RSC_TEXT );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

USHORT FixedText::ImplGetTextStyle( WinBits nWinStyle )
{
    USHORT nTextStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_MULTILINE | TEXT_DRAW_ENDELLIPSIS;

    if ( nWinStyle & WB_RIGHT )
        nTextStyle |= TEXT_DRAW_RIGHT;
    else if ( nWinStyle & WB_CENTER )
        nTextStyle |= TEXT_DRAW_CENTER;
    else
        nTextStyle |= TEXT_DRAW_LEFT;
    if ( nWinStyle & WB_BOTTOM )
        nTextStyle |= TEXT_DRAW_BOTTOM;
    else if ( nWinStyle & WB_VCENTER )
        nTextStyle |= TEXT_DRAW_VCENTER;
    else
        nTextStyle |= TEXT_DRAW_TOP;
    if ( nWinStyle & WB_WORDBREAK )
        nTextStyle |= TEXT_DRAW_WORDBREAK;
    if ( nWinStyle & WB_NOLABEL )
        nTextStyle &= ~TEXT_DRAW_MNEMONIC;

    return nTextStyle;
}

// -----------------------------------------------------------------------

void FixedText::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                          const Point& rPos, const Size& rSize )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    WinBits                 nWinStyle = GetStyle();
    XubString               aText( GetText() );
    USHORT                  nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );

    if ( nWinStyle & WB_PATHELLIPSIS )
    {
        nTextStyle &= ~(TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK);
        nTextStyle |= TEXT_DRAW_PATHELLIPSIS;
    }
    if ( nDrawFlags & WINDOW_DRAW_NOMNEMONIC )
    {
        if ( nTextStyle & TEXT_DRAW_MNEMONIC )
        {
            aText = GetNonMnemonicString( aText );
            nTextStyle &= ~TEXT_DRAW_MNEMONIC;
        }
    }
    if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
    {
        if ( !IsEnabled() )
            nTextStyle |= TEXT_DRAW_DISABLE;
    }
    if ( (nDrawFlags & WINDOW_DRAW_MONO) ||
         (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nTextStyle |= TEXT_DRAW_MONO;

    pDev->DrawText( Rectangle( rPos, rSize ), aText, nTextStyle );
}

// -----------------------------------------------------------------------

void FixedText::Paint( const Rectangle& rRect )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedText::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                      ULONG nFlags )
{
    ImplInitSettings( TRUE, TRUE, TRUE );

    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Font        aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & WINDOW_DRAW_MONO )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    // Border
    if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
    {
        pDev->SetLineColor( Color( COL_BLACK ) );
        pDev->SetFillColor();
        pDev->DrawRect( Rectangle( aPos, aSize ) );
    }

    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedText::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void FixedText::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_ENABLE) ||
         (nType == STATE_CHANGE_TEXT) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDTEXT_VIEW_STYLE) !=
             (GetStyle() & FIXEDTEXT_VIEW_STYLE) )
        {
            ImplInitSettings( TRUE, FALSE, FALSE );
            Invalidate();
        }
    }
    else if ( (nType == STATE_CHANGE_ZOOM)  ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedText::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

Size FixedText::CalcMinimumSize( long nMaxWidth ) const
{
    USHORT nStyle = ImplGetTextStyle( GetStyle() );
    if ( !( GetStyle() & WB_NOLABEL ) )
        nStyle |= TEXT_DRAW_MNEMONIC;

    Size aSize = GetTextRect( Rectangle( Point(), Size( (nMaxWidth ? nMaxWidth : 0x7fffffff), 0x7fffffff ) ),
                              GetText(), nStyle ).GetSize();

    // GetTextRect verkraftet keinen leeren String:
    if ( aSize.Width() < 0 )
        aSize.Width() = 0;
    if ( aSize.Height() <= 0 )
        aSize.Height() = GetTextHeight();

    return CalcWindowSize( aSize );
}


// =======================================================================

void FixedLine::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits FixedLine::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void FixedLine::ImplInitSettings( BOOL bFont,
                                  BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = rStyleSettings.GetGroupTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Window* pParent = GetParent();
        if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
        }
        else
        {
            EnableChildTransparentMode( FALSE );
            SetParentClipMode( 0 );
            SetPaintTransparent( FALSE );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

// -----------------------------------------------------------------------

FixedLine::FixedLine( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDLINE )
{
    ImplInit( pParent, nStyle );
    SetSizePixel( Size( 2, 2 ) );
}

// -----------------------------------------------------------------------

FixedLine::FixedLine( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDLINE )
{
    rResId.SetRT( RSC_FIXEDLINE );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void FixedLine::Paint( const Rectangle& rRect )
{
    Size                    aOutSize = GetOutputSizePixel();
    String                  aText = GetText();
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    WinBits                 nWinStyle = GetStyle();

    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
        SetLineColor( Color( COL_BLACK ) );
    else
        SetLineColor( rStyleSettings.GetShadowColor() );

    if ( !aText.Len() || (nWinStyle & WB_VERT) )
    {
        long nX;
        long nY;

        if ( nWinStyle & WB_VERT )
        {
            nX = (aOutSize.Width()-1)/2;
            DrawLine( Point( nX, 0 ), Point( nX, aOutSize.Height()-1 ) );
        }
        else
        {
            nY = (aOutSize.Height()-1)/2;
            DrawLine( Point( 0, nY ), Point( aOutSize.Width()-1, nY ) );
        }

        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        {
            SetLineColor( rStyleSettings.GetLightColor() );
            if ( nWinStyle & WB_VERT )
                DrawLine( Point( nX+1, 0 ), Point( nX+1, aOutSize.Height()-1 ) );
            else
                DrawLine( Point( 0, nY+1 ), Point( aOutSize.Width()-1, nY+1 ) );
        }
    }
    else
    {
        USHORT      nStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER | TEXT_DRAW_ENDELLIPSIS;
        Rectangle   aRect( 0, 0, aOutSize.Width(), aOutSize.Height() );

        if ( !IsEnabled() )
            nStyle |= TEXT_DRAW_DISABLE;
        if ( GetStyle() & WB_NOLABEL )
            nStyle &= ~TEXT_DRAW_MNEMONIC;
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
            nStyle |= TEXT_DRAW_MONO;

        aRect = GetTextRect( aRect, aText, nStyle );
        DrawText( aRect, aText, nStyle );

        long nTop = aRect.Top() + ((aRect.GetHeight()-1)/2);
        DrawLine( Point( aRect.Right()+FIXEDLINE_TEXT_BORDER, nTop ), Point( aOutSize.Width()-1, nTop ) );
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        {
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( aRect.Right()+FIXEDLINE_TEXT_BORDER, nTop+1 ), Point( aOutSize.Width()-1, nTop+1 ) );
        }
    }
}

// -----------------------------------------------------------------------

void FixedLine::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                      ULONG nFlags )
{
}

// -----------------------------------------------------------------------

void FixedLine::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void FixedLine::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_ENABLE) ||
         (nType == STATE_CHANGE_TEXT) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDLINE_VIEW_STYLE) !=
             (GetStyle() & FIXEDLINE_VIEW_STYLE) )
            Invalidate();
    }
    else if ( (nType == STATE_CHANGE_ZOOM)  ||
              (nType == STATE_CHANGE_STYLE) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedLine::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

// =======================================================================

void FixedBitmap::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings();
}

// -----------------------------------------------------------------------

WinBits FixedBitmap::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void FixedBitmap::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( FALSE );
        SetParentClipMode( 0 );
        SetPaintTransparent( FALSE );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

void FixedBitmap::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    USHORT nObjMask = ReadShortRes();

    if ( RSC_FIXEDBITMAP_BITMAP & nObjMask )
    {
        maBitmap = Bitmap( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
}

// -----------------------------------------------------------------------

FixedBitmap::FixedBitmap( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDBITMAP )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedBitmap::FixedBitmap( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDBITMAP )
{
    rResId.SetRT( RSC_FIXEDBITMAP );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

FixedBitmap::~FixedBitmap()
{
}

// -----------------------------------------------------------------------

void FixedBitmap::ImplDraw( OutputDevice* pDev, ULONG /* nDrawFlags */,
                            const Point& rPos, const Size& rSize )
{
    // Haben wir ueberhaupt eine Bitmap
    if ( !(!maBitmap) )
    {
        if ( GetStyle() & WB_SCALE )
            pDev->DrawBitmap( rPos, rSize, maBitmap );
        else
        {
            Point aPos = ImplCalcPos( GetStyle(), rPos, maBitmap.GetSizePixel(), rSize );
            pDev->DrawBitmap( aPos, maBitmap );
        }
    }
}

// -----------------------------------------------------------------------

void FixedBitmap::Paint( const Rectangle& rRect )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedBitmap::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                        ULONG nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
    {
        DecorationView aDecoView( pDev );
        aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedBitmap::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void FixedBitmap::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_DATA) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDBITMAP_VIEW_STYLE) !=
             (GetStyle() & FIXEDBITMAP_VIEW_STYLE) )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBitmap::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBitmap::SetBitmap( const Bitmap& rBitmap )
{
    maBitmap = rBitmap;
    StateChanged( STATE_CHANGE_DATA );
}

// =======================================================================

void FixedImage::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    mbInUserDraw = FALSE;
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings();
}

// -----------------------------------------------------------------------

WinBits FixedImage::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void FixedImage::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( FALSE );
        SetParentClipMode( 0 );
        SetPaintTransparent( FALSE );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

void FixedImage::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    USHORT nObjMask = ReadShortRes();

    if ( RSC_FIXEDIMAGE_IMAGE & nObjMask )
    {
        maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
}

// -----------------------------------------------------------------------

FixedImage::FixedImage( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDIMAGE )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedImage::FixedImage( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDIMAGE )
{
    rResId.SetRT( RSC_FIXEDIMAGE );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

FixedImage::~FixedImage()
{
}

// -----------------------------------------------------------------------

void FixedImage::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                           const Point& rPos, const Size& rSize )
{
    USHORT nStyle = 0;
    if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
    {
        if ( !IsEnabled() )
            nStyle |= IMAGE_DRAW_DISABLE;
    }

    // Haben wir ueberhaupt ein Image
    if ( !(!maImage) )
    {
        if ( GetStyle() & WB_SCALE )
            pDev->DrawImage( rPos, rSize, maImage, nStyle );
        else
        {
            Point aPos = ImplCalcPos( GetStyle(), rPos, maImage.GetSizePixel(), rSize );
            pDev->DrawImage( aPos, maImage, nStyle );
        }
    }

    mbInUserDraw = TRUE;
    UserDrawEvent aUDEvt( pDev, Rectangle( rPos, rSize ), 0, nStyle );
    UserDraw( aUDEvt );
    mbInUserDraw = FALSE;
}

// -----------------------------------------------------------------------

void FixedImage::Paint( const Rectangle& rRect )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedImage::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void FixedImage::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                       ULONG nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
    {
        DecorationView aDecoView( pDev );
        aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedImage::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void FixedImage::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_ENABLE) ||
         (nType == STATE_CHANGE_DATA) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDIMAGE_VIEW_STYLE) !=
             (GetStyle() & FIXEDIMAGE_VIEW_STYLE) )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedImage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedImage::SetImage( const Image& rImage )
{
    if ( rImage != maImage )
    {
        maImage = rImage;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

Point FixedImage::CalcImagePos( const Point& rPos,
                                const Size& rObjSize, const Size& rWinSize )
{
    return ImplCalcPos( GetStyle(), rPos, rObjSize, rWinSize );
}
