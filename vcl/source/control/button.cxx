/*************************************************************************
 *
 *  $RCSfile: button.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
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

#define _SV_BUTTON_CXX

#include <tools/debug.hxx>

#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_IAMGE_HXX
#include <image.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <fixed.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

#define PUSHBUTTON_VIEW_STYLE       (WB_3DLOOK |                        \
                                     WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                     WB_WORDBREAK | WB_NOLABEL |        \
                                     WB_DEFBUTTON | WB_NOLIGHTBORDER |  \
                                     WB_RECTSTYLE | WB_SMALLSTYLE)
#define RADIOBUTTON_VIEW_STYLE      (WB_3DLOOK |                        \
                                     WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                     WB_WORDBREAK | WB_NOLABEL)
#define CHECKBOX_VIEW_STYLE         (WB_3DLOOK |                        \
                                     WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                     WB_WORDBREAK | WB_NOLABEL)

// =======================================================================

Button::Button( WindowType nType ) :
    Control( nType )
{
}

// -----------------------------------------------------------------------

Button::Button( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_BUTTON )
{
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Button::Button( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_BUTTON )
{
    rResId.SetRT( RSC_BUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void Button::Click()
{
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

XubString Button::GetStandardText( StandardButtonType eButton )
{
    static USHORT aResIdAry[BUTTON_COUNT] =
    {
        SV_BUTTONTEXT_OK,
        SV_BUTTONTEXT_CANCEL,
        SV_BUTTONTEXT_YES,
        SV_BUTTONTEXT_NO,
        SV_BUTTONTEXT_RETRY,
        SV_BUTTONTEXT_HELP,
        SV_BUTTONTEXT_CLOSE,
        SV_BUTTONTEXT_MORE
    };

    ResId aResId( aResIdAry[(USHORT)eButton], ImplGetResMgr() );
    XubString aText( aResId );
    return aText;
}

// -----------------------------------------------------------------------

XubString Button::GetStandardHelpText( StandardButtonType /* eButton */ )
{
    XubString aHelpText;
    return aHelpText;
}

// =======================================================================

void PushButton::ImplInitData()
{
    mbPushButton    = TRUE;

    meSymbol        = SYMBOL_NOSYMBOL;
    meImageAlign    = IMAGEALIGN_TOP;
    meState         = STATE_NOCHECK;
    meSaveValue     = STATE_NOCHECK;
    mnDDStyle       = 0;
    mnButtonState   = 0;
    mbPressed       = FALSE;
    mbInUserDraw    = FALSE;
    mpBitmapEx      = NULL;
}

// -----------------------------------------------------------------------

void PushButton::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( pParent->GetWindow( WINDOW_LASTCHILD ), nStyle );
    Button::ImplInit( pParent, nStyle, NULL );

    if ( nStyle & WB_NOLIGHTBORDER )
        mnButtonState |= BUTTON_DRAW_NOLIGHTBORDER;

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits PushButton::ImplInitStyle( const Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) &&
         (!pPrevWindow ||
          ((pPrevWindow->GetType() != WINDOW_PUSHBUTTON) &&
           (pPrevWindow->GetType() != WINDOW_OKBUTTON) &&
           (pPrevWindow->GetType() != WINDOW_CANCELBUTTON) &&
           (pPrevWindow->GetType() != WINDOW_HELPBUTTON)) ) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void PushButton::ImplInitSettings( BOOL bFont,
                                   BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetPushButtonFont();
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
            aColor = rStyleSettings.GetButtonTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
        SetBackground();
}

// -----------------------------------------------------------------------

void PushButton::ImplDrawPushButtonFrame( Window* pDev,
                                          Rectangle& rRect, USHORT nStyle )
{
    if ( !(pDev->GetStyle() & (WB_RECTSTYLE | WB_SMALLSTYLE)) )
    {
        StyleSettings aStyleSettings = pDev->GetSettings().GetStyleSettings();
        if ( pDev->IsControlBackground() )
            aStyleSettings.Set3DColors( pDev->GetControlBackground() );

        USHORT nPushButtonSysStyle = aStyleSettings.GetPushButtonStyle() & STYLE_PUSHBUTTON_STYLE;
        if ( nPushButtonSysStyle == STYLE_PUSHBUTTON_MAC )
        {
            pDev->SetLineColor();
            pDev->SetFillColor( aStyleSettings.GetFaceColor() );
            pDev->DrawRect( rRect );

            if ( (aStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
                 (pDev->GetOutDevType() == OUTDEV_PRINTER) )
                nStyle |= BUTTON_DRAW_MONO;

            if ( nStyle & BUTTON_DRAW_DEFAULT )
            {
                if ( nStyle & BUTTON_DRAW_MONO )
                    pDev->SetLineColor( Color( COL_BLACK ) );
                else
                    pDev->SetLineColor( aStyleSettings.GetDarkShadowColor() );

                pDev->DrawLine( Point( rRect.Left()+3, rRect.Top() ),
                                Point( rRect.Right()-3, rRect.Top() ) );
                pDev->DrawLine( Point( rRect.Left()+3, rRect.Bottom() ),
                                Point( rRect.Right()-3, rRect.Bottom() ) );
                pDev->DrawLine( Point( rRect.Left(), rRect.Top()+3 ),
                                Point( rRect.Left(), rRect.Bottom()-3 ) );
                pDev->DrawLine( Point( rRect.Right(), rRect.Top()+3 ),
                                Point( rRect.Right(), rRect.Bottom()-3 ) );
                pDev->DrawPixel( Point( rRect.Left()+2, rRect.Top()+1 ) );
                pDev->DrawPixel( Point( rRect.Left()+1, rRect.Top()+2 ) );
                pDev->DrawPixel( Point( rRect.Right()-2, rRect.Top()+1 ) );
                pDev->DrawPixel( Point( rRect.Right()-1, rRect.Top()+2 ) );
                pDev->DrawPixel( Point( rRect.Left()+2, rRect.Bottom()-1 ) );
                pDev->DrawPixel( Point( rRect.Left()+1, rRect.Bottom()-2 ) );
                pDev->DrawPixel( Point( rRect.Right()-2, rRect.Bottom()-1 ) );
                pDev->DrawPixel( Point( rRect.Right()-1, rRect.Bottom()-2 ) );

                if ( nStyle & BUTTON_DRAW_MONO )
                    pDev->SetLineColor( Color( COL_BLACK ) );
                else
                    pDev->SetLineColor( aStyleSettings.GetShadowColor() );
                pDev->DrawLine( Point( rRect.Left()+3, rRect.Bottom()-1 ),
                                Point( rRect.Right()-3, rRect.Bottom()-1 ) );
                pDev->DrawLine( Point( rRect.Right()-1, rRect.Top()+3 ),
                                Point( rRect.Right()-1, rRect.Bottom()-3 ) );
                pDev->DrawPixel( Point( rRect.Right()-3, rRect.Bottom()-2 ) );
                pDev->DrawPixel( Point( rRect.Right()-2, rRect.Bottom()-2 ) );
                pDev->DrawPixel( Point( rRect.Right()-2, rRect.Bottom()-3 ) );
            }

            rRect.Left()   += 2;
            rRect.Top()    += 2;
            rRect.Right()  -= 2;
            rRect.Bottom() -= 2;

            if ( nStyle & BUTTON_DRAW_MONO )
                pDev->SetLineColor( Color( COL_BLACK ) );
            else
                pDev->SetLineColor( aStyleSettings.GetDarkShadowColor() );

            pDev->DrawLine( Point( rRect.Left()+2, rRect.Top() ),
                            Point( rRect.Right()-2, rRect.Top() ) );
            pDev->DrawLine( Point( rRect.Left()+2, rRect.Bottom() ),
                            Point( rRect.Right()-2, rRect.Bottom() ) );
            pDev->DrawLine( Point( rRect.Left(), rRect.Top()+2 ),
                            Point( rRect.Left(), rRect.Bottom()-2 ) );
            pDev->DrawLine( Point( rRect.Right(), rRect.Top()+2 ),
                            Point( rRect.Right(), rRect.Bottom()-2 ) );
            pDev->DrawPixel( Point( rRect.Left()+1, rRect.Top()+1 ) );
            pDev->DrawPixel( Point( rRect.Right()-1, rRect.Top()+1 ) );
            pDev->DrawPixel( Point( rRect.Left()+1, rRect.Bottom()-1 ) );
            pDev->DrawPixel( Point( rRect.Right()-1, rRect.Bottom()-1 ) );

            pDev->SetLineColor();
            if ( nStyle & BUTTON_DRAW_CHECKED )
                pDev->SetFillColor( aStyleSettings.GetCheckedColor() );
            else
                pDev->SetFillColor( aStyleSettings.GetFaceColor() );
            pDev->DrawRect( Rectangle( rRect.Left()+2, rRect.Top()+2, rRect.Right()-2, rRect.Bottom()-2 ) );

            if ( !(nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED)) )
            {
                if ( nStyle & BUTTON_DRAW_MONO )
                    pDev->SetLineColor( Color( COL_BLACK ) );
                else
                    pDev->SetLineColor( aStyleSettings.GetShadowColor() );
                pDev->DrawLine( Point( rRect.Left()+2, rRect.Bottom()-1 ),
                                Point( rRect.Right()-2, rRect.Bottom()-1 ) );
                pDev->DrawLine( Point( rRect.Right()-1, rRect.Top()+2 ),
                                Point( rRect.Right()-1, rRect.Bottom()-2 ) );
                pDev->DrawPixel( Point( rRect.Right()-2, rRect.Bottom()-2 ) );
                pDev->SetLineColor( aStyleSettings.GetLightColor() );
            }
            else
                pDev->SetLineColor( aStyleSettings.GetShadowColor() );

            if ( !(nStyle & BUTTON_DRAW_MONO) )
            {
                pDev->DrawLine( Point( rRect.Left()+2, rRect.Top()+1 ),
                                Point( rRect.Right()-2, rRect.Top()+1 ) );
                pDev->DrawLine( Point( rRect.Left()+1, rRect.Top()+2 ),
                                Point( rRect.Left()+1, rRect.Bottom()-2 ) );
                pDev->DrawPixel( Point( rRect.Top()+2, rRect.Right()+2 ) );
            }

            rRect.Left()   += 2;
            rRect.Top()    += 2;
            rRect.Right()  -= 2;
            rRect.Bottom() -= 2;

            if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
            {
                rRect.Left()++;
                rRect.Top()++;
                rRect.Right()++;
                rRect.Bottom()++;
            }

            return;
        }
    }

    DecorationView aDecoView( pDev );
    if ( pDev->IsControlBackground() )
    {
        AllSettings     aSettings = pDev->GetSettings();
        AllSettings     aOldSettings = aSettings;
        StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
        aStyleSettings.Set3DColors( pDev->GetControlBackground() );
        aSettings.SetStyleSettings( aStyleSettings );
        ((OutputDevice*)pDev)->SetSettings( aSettings );
        rRect = aDecoView.DrawButton( rRect, nStyle );
        ((OutputDevice*)pDev)->SetSettings( aOldSettings );
    }
    else
        rRect = aDecoView.DrawButton( rRect, nStyle );
}

// -----------------------------------------------------------------------

BOOL PushButton::ImplHitTestPushButton( Window* pDev,
                                        const Point& rPos, USHORT nStyle )
{
    Point       aTempPoint;
    Rectangle   aTestRect( aTempPoint, pDev->GetOutputSizePixel() );

    if ( !(pDev->GetStyle() & (WB_RECTSTYLE | WB_SMALLSTYLE)) )
    {
        const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();

        USHORT nPushButtonSysStyle = rStyleSettings.GetPushButtonStyle() & STYLE_PUSHBUTTON_STYLE;
        if ( nPushButtonSysStyle == STYLE_PUSHBUTTON_MAC )
        {
            aTestRect.Left()    += 2;
            aTestRect.Top()     += 2;
            aTestRect.Right()   -= 2;
            aTestRect.Bottom()  -= 2;
        }
    }

    return aTestRect.IsInside( rPos );
}

// -----------------------------------------------------------------------

USHORT PushButton::ImplGetTextStyle() const
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    USHORT nTextStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_MULTILINE | TEXT_DRAW_ENDELLIPSIS;

    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
        nTextStyle |= TEXT_DRAW_MONO;
    if ( GetStyle() & WB_WORDBREAK )
        nTextStyle |= TEXT_DRAW_WORDBREAK;
    if ( GetStyle() & WB_NOLABEL )
        nTextStyle &= ~TEXT_DRAW_MNEMONIC;
    nTextStyle |= TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER;

    return nTextStyle;
}

// -----------------------------------------------------------------------

static void ImplDrawBtnDropDownArrow( OutputDevice* pDev,
                                      long nX, long nY,
                                      Color& rColor, BOOL bBlack )
{
    Color aOldLineColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();

    pDev->SetLineColor();
    if ( bBlack )
        pDev->SetFillColor( Color( COL_BLACK ) );
    else
        pDev->SetFillColor( rColor );
    pDev->DrawRect( Rectangle( nX+0, nY+0, nX+6, nY+0 ) );
    pDev->DrawRect( Rectangle( nX+1, nY+1, nX+5, nY+1 ) );
    pDev->DrawRect( Rectangle( nX+2, nY+2, nX+4, nY+2 ) );
    pDev->DrawRect( Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
    if ( bBlack )
    {
        pDev->SetFillColor( rColor );
        pDev->DrawRect( Rectangle( nX+2, nY+1, nX+4, nY+1 ) );
        pDev->DrawRect( Rectangle( nX+3, nY+2, nX+3, nY+2 ) );
    }
    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

// -----------------------------------------------------------------------

void PushButton::ImplDrawPushButtonContent( OutputDevice* pDev, ULONG nDrawFlags,
                                            const Rectangle& rRect, Rectangle& rTextRect )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle               aInRect = rRect;
    Color                   aColor = rStyleSettings.GetButtonTextColor();
    XubString               aText = PushButton::GetText(); // PushButton:: wegen MoreButton
    USHORT                  nTextStyle;
    USHORT                  nStyle;

    if ( nDrawFlags & WINDOW_DRAW_MONO )
        aColor = Color( COL_BLACK );
    else
    {
        if ( IsControlForeground() )
            aColor = GetControlForeground();
    }

    rTextRect = aInRect;
    if ( mnDDStyle == PUSHBUTTON_DROPDOWN_MENUBUTTON )
    {
        if ( aText.Len() )
        {
            // Symbol- und Textrect ermitteln
            long nSymbolSize    = pDev->GetTextHeight();
            aInRect.Right()    -= 5;
            rTextRect.Left()   += 2;
            rTextRect.Right()   = aInRect.Right()-nSymbolSize;
            aInRect.Left()      = rTextRect.Right();

            nTextStyle = ImplGetTextStyle();
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
            nTextStyle &= ~(TEXT_DRAW_RIGHT | TEXT_DRAW_CENTER);
            nTextStyle |= TEXT_DRAW_LEFT;
            rTextRect = pDev->GetTextRect( rTextRect, aText, nTextStyle );
            pDev->SetTextColor( aColor );
            pDev->DrawText( rTextRect, aText, nTextStyle );
        }
        else
            ImplCalcSymbolRect( aInRect );

        nStyle = 0;
        if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
        {
            if ( !IsEnabled() )
                nStyle |= SYMBOL_DRAW_DISABLE;
        }

        DecorationView aDecoView( pDev );
        aDecoView.DrawSymbol( aInRect, SYMBOL_SPIN_DOWN, aColor, nStyle );
    }
    else
    {
        if ( aText.Len() )
        {
            nTextStyle = ImplGetTextStyle();
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
            rTextRect = pDev->GetTextRect( aInRect, aText, nTextStyle );
        }

        if ( IsSymbol() )
        {
            ImplCalcSymbolRect( aInRect );

            nStyle = 0;
            if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
            {
                if ( !IsEnabled() )
                    nStyle |= SYMBOL_DRAW_DISABLE;
            }

            DecorationView aDecoView( pDev );
            aDecoView.DrawSymbol( aInRect, meSymbol, aColor, nStyle );
        }

        if ( aText.Len() )
        {
            pDev->SetTextColor( aColor );
            pDev->DrawText( rTextRect, aText, nTextStyle );
        }

        // Den Fall Text+Image erstmal ignoriert, TH ignoriert auch Text+Symbol
        if ( IsImage() )
        {
            // Image zentrieren...
            Size aImageSize( maImage.GetSizePixel() );
            aImageSize.Width()  = CalcZoom( aImageSize.Width() );
            aImageSize.Height() = CalcZoom( aImageSize.Height() );
            if ( mpBitmapEx && ( pDev->GetOutDevType() == OUTDEV_PRINTER ) )
            {
                // Die Groesse richtet sich nach dem Bildschirm, soll auf
                // dem Drucker genau so aussehen...
                aImageSize = PixelToLogic( aImageSize, MAP_100TH_MM );
                aImageSize = pDev->LogicToPixel( aImageSize, MAP_100TH_MM );
            }

            Point aImagePos( rRect.Left()+((aInRect.GetWidth()-aImageSize.Width())/2),
                             rRect.Top()+((aInRect.GetHeight()-aImageSize.Height())/2) );
            nStyle = 0;
            if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
            {
                if ( !IsEnabled() )
                    nStyle |= IMAGE_DRAW_DISABLE;
            }
            if ( mpBitmapEx && ( pDev->GetOutDevType() == OUTDEV_PRINTER ) )
            {
                // Fuer die BitmapEx ueberlegt sich KA noch, wie man die disablete
                // Darstellung hinbekommt...
                aImageSize = pDev->PixelToLogic( aImageSize );
                mpBitmapEx->Draw( pDev, aImagePos, aImageSize /*, nStyle*/ );
            }
            else
            {
                if ( IsZoom() )
                    pDev->DrawImage( aImagePos, aImageSize, maImage, nStyle );
                else
                    pDev->DrawImage( aImagePos, maImage, nStyle );
            }
        }

        if ( mnDDStyle == PUSHBUTTON_DROPDOWN_TOOLBOX )
        {
            BOOL    bBlack = FALSE;
            Color   aArrowColor( COL_BLACK );
            if ( !(nDrawFlags & WINDOW_DRAW_MONO) )
            {
                if ( !IsEnabled() )
                    aArrowColor = rStyleSettings.GetShadowColor();
                else
                {
                    aArrowColor = Color( COL_LIGHTGREEN );
                    bBlack = TRUE;
                }
            }

            ImplDrawBtnDropDownArrow( pDev, aInRect.Right()-6, aInRect.Top()+1,
                                      aArrowColor, bBlack );
        }
    }

    UserDrawEvent aUDEvt( this, aInRect, 0 );
    UserDraw( aUDEvt );
}

// -----------------------------------------------------------------------

void PushButton::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void PushButton::ImplDrawPushButton()
{
    HideFocus();

    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    USHORT                  nButtonStyle = mnButtonState;
    Point                   aPoint;
    Size                    aOutSz( GetOutputSizePixel() );
    Rectangle               aRect( aPoint, aOutSz );
    Rectangle               aInRect = aRect;
    Rectangle               aTextRect;

    // Wenn Button gedrueckt gezeichnet werden soll, dann Pressed dazuordern
    if ( mbPressed )
        nButtonStyle |= BUTTON_DRAW_PRESSED;

    // PushButtonFrame ausgeben
    ImplDrawPushButtonFrame( this, aInRect, nButtonStyle );

    // PushButton-Inhalt ausgeben
    ImplDrawPushButtonContent( this, 0, aInRect, aTextRect );

    maFocusRect = aTextRect;
    maFocusRect.Left()--;
    maFocusRect.Top()--;
    maFocusRect.Right()++;
    maFocusRect.Bottom()++;
    if ( HasFocus() )
        ShowFocus( maFocusRect );
}

// -----------------------------------------------------------------------

void PushButton::ImplSetDefButton( BOOL bSet )
{
    if ( bSet )
        mnButtonState |= BUTTON_DRAW_DEFAULT;
    else
        mnButtonState &= ~BUTTON_DRAW_DEFAULT;
    Invalidate();
}

// -----------------------------------------------------------------------

BOOL PushButton::ImplIsDefButton() const
{
    return (mnButtonState & BUTTON_DRAW_DEFAULT) != 0;
}

// -----------------------------------------------------------------------

PushButton::PushButton( WindowType nType ) :
    Button( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

PushButton::PushButton( Window* pParent, WinBits nStyle ) :
    Button( WINDOW_PUSHBUTTON )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

PushButton::PushButton( Window* pParent, const ResId& rResId ) :
    Button( WINDOW_PUSHBUTTON )
{
    ImplInitData();
    rResId.SetRT( RSC_PUSHBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

PushButton::~PushButton()
{
    delete mpBitmapEx;
}

// -----------------------------------------------------------------------

void PushButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() &&
         ImplHitTestPushButton( this, rMEvt.GetPosPixel(), mnButtonState ) )
    {
        USHORT nTrackFlags = 0;

        if ( GetStyle() & WB_REPEAT )
            nTrackFlags |= STARTTRACK_BUTTONREPEAT;

        mnButtonState |= BUTTON_DRAW_PRESSED;
        ImplDrawPushButton();
        StartTracking( nTrackFlags );

        if ( GetStyle() & WB_REPEAT )
            Click();
    }
}

// -----------------------------------------------------------------------

void PushButton::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mnButtonState & BUTTON_DRAW_PRESSED )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            mnButtonState &= ~BUTTON_DRAW_PRESSED;
            ImplDrawPushButton();

            // Bei Abbruch kein Click-Handler rufen
            if ( !rTEvt.IsTrackingCanceled() )
            {
                if ( !(GetStyle() & WB_REPEAT) )
                    Click();
            }
        }
    }
    else
    {
        if ( ImplHitTestPushButton( this, rTEvt.GetMouseEvent().GetPosPixel(), mnButtonState ) )
        {
            if ( mnButtonState & BUTTON_DRAW_PRESSED )
            {
                if ( rTEvt.IsTrackingRepeat() && (GetStyle() & WB_REPEAT) )
                    Click();
            }
            else
            {
                mnButtonState |= BUTTON_DRAW_PRESSED;
                ImplDrawPushButton();
            }
        }
        else
        {
            if ( mnButtonState & BUTTON_DRAW_PRESSED )
            {
                mnButtonState &= ~BUTTON_DRAW_PRESSED;
                ImplDrawPushButton();
            }
        }
    }
}

// -----------------------------------------------------------------------

void PushButton::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( !aKeyCode.GetModifier() &&
         ((aKeyCode.GetCode() == KEY_RETURN) || (aKeyCode.GetCode() == KEY_SPACE)) )
    {
        if ( !(mnButtonState & BUTTON_DRAW_PRESSED) )
        {
            mnButtonState |= BUTTON_DRAW_PRESSED;
            ImplDrawPushButton();
        }

        if ( GetStyle() & WB_REPEAT )
            Click();
    }
    else if ( (mnButtonState & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplDrawPushButton();
    }
    else
        Button::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void PushButton::KeyUp( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (mnButtonState & BUTTON_DRAW_PRESSED) &&
         ((aKeyCode.GetCode() == KEY_RETURN) || (aKeyCode.GetCode() == KEY_SPACE)) )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplDrawPushButton();

        if ( !(GetStyle() & WB_REPEAT) )
            Click();
    }
    else
        Button::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void PushButton::Paint( const Rectangle& rRect )
{
    ImplDrawPushButton();
}

// -----------------------------------------------------------------------

void PushButton::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                       ULONG nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );
    Rectangle   aTextRect;
    Font        aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & WINDOW_DRAW_MONO )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    DecorationView aDecoView( pDev );
    USHORT nButtonStyle = 0;
    if ( nFlags & WINDOW_DRAW_MONO )
        nButtonStyle |= BUTTON_DRAW_MONO;
    if ( IsChecked() )
        nButtonStyle |= BUTTON_DRAW_CHECKED;
    aRect = aDecoView.DrawButton( aRect, nButtonStyle );

    ImplDrawPushButtonContent( pDev, nFlags, aRect, aTextRect );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void PushButton::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void PushButton::GetFocus()
{
    ShowFocus( maFocusRect );
    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

// -----------------------------------------------------------------------

void PushButton::LoseFocus()
{
    EndSelection();
    HideFocus();
    Button::LoseFocus();
}

// -----------------------------------------------------------------------

void PushButton::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( (nType == STATE_CHANGE_ENABLE) ||
         (nType == STATE_CHANGE_TEXT) ||
         (nType == STATE_CHANGE_IMAGE) ||
         (nType == STATE_CHANGE_DATA) ||
         (nType == STATE_CHANGE_STATE) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();

        if ( nType == STATE_CHANGE_STATE )
            Toggle();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetWindow( WINDOW_PREV ), GetStyle() ) );

        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & PUSHBUTTON_VIEW_STYLE) !=
                 (GetStyle() & PUSHBUTTON_VIEW_STYLE) )
                Invalidate();
        }
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
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

void PushButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    Button::DataChanged( rDCEvt );

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

void PushButton::Toggle()
{
    maToggleHdl.Call( this );
}

// -----------------------------------------------------------------------

void PushButton::SetSymbol( SymbolType eSymbol )
{
    if ( meSymbol != eSymbol )
    {
        meSymbol = eSymbol;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void PushButton::SetImage( const Image& rImage )
{
    delete mpBitmapEx;
    mpBitmapEx = NULL;
    if ( rImage != maImage )
    {
        maImage = rImage;
        meSymbol = SYMBOL_IMAGE;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void PushButton::SetBitmap( const BitmapEx& rBmp )
{
    SetImage( rBmp );
    DBG_ASSERT( !mpBitmapEx, "BitmapEx nach SetImage?!" );
    mpBitmapEx = new BitmapEx( rBmp );
}

// -----------------------------------------------------------------------

BitmapEx PushButton::GetBitmap() const
{
    BitmapEx aBmp;
    if ( mpBitmapEx )
        aBmp = *mpBitmapEx;
    return aBmp;
}

// -----------------------------------------------------------------------

void PushButton::SetImageAlign( ImageAlign eAlign )
{
    if ( meImageAlign != eAlign )
    {
        meImageAlign = eAlign;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void PushButton::SetDropDown( USHORT nStyle )
{
    if ( mnDDStyle != nStyle )
    {
        mnDDStyle = nStyle;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void PushButton::SetState( TriState eState )
{
    if ( meState != eState )
    {
        meState = eState;
        if ( meState == STATE_NOCHECK )
            mnButtonState &= ~(BUTTON_DRAW_CHECKED | BUTTON_DRAW_DONTKNOW);
        else if ( meState == STATE_CHECK )
        {
            mnButtonState &= ~BUTTON_DRAW_DONTKNOW;
            mnButtonState |= BUTTON_DRAW_CHECKED;
        }
        else // STATE_DONTKNOW
        {
            mnButtonState &= ~BUTTON_DRAW_CHECKED;
            mnButtonState |= BUTTON_DRAW_DONTKNOW;
        }

        StateChanged( STATE_CHANGE_STATE );
    }
}

// -----------------------------------------------------------------------

void PushButton::SetPressed( BOOL bPressed )
{
    if ( mbPressed != bPressed )
    {
        mbPressed = bPressed;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void PushButton::EndSelection()
{
    EndTracking( ENDTRACK_CANCEL );
    if ( mnButtonState & BUTTON_DRAW_PRESSED )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        if ( !mbPressed )
            ImplDrawPushButton();
    }
}

// -----------------------------------------------------------------------

Size PushButton::CalcMinimumSize( long nMaxWidth ) const
{
    Size aSize;

    if ( IsSymbol() )
        aSize = Size( 12, 12 );
    else if ( IsImage() )
        aSize = maImage.GetSizePixel();
    else if ( PushButton::GetText().Len() )
    {
        aSize = GetTextRect( Rectangle( Point(), Size( nMaxWidth ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                             PushButton::GetText(), ImplGetTextStyle() ).GetSize();
    }

    return CalcWindowSize( aSize );
}

// =======================================================================

void OKButton::ImplInit( Window* pParent, WinBits nStyle )
{
    PushButton::ImplInit( pParent, nStyle );

    SetText( Button::GetStandardText( BUTTON_OK ) );
    SetHelpText( Button::GetStandardHelpText( BUTTON_OK ) );
}

// -----------------------------------------------------------------------

OKButton::OKButton( Window* pParent, WinBits nStyle ) :
    PushButton( WINDOW_OKBUTTON )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

OKButton::OKButton( Window* pParent, const ResId& rResId ) :
    PushButton( WINDOW_OKBUTTON )
{
    rResId.SetRT( RSC_OKBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void OKButton::Click()
{
    // Ist kein Link gesetzt, dann schliesse Parent
    if ( !GetClickHdl() )
    {
        Window* pParent = GetParent();
        if ( pParent->IsSystemWindow() )
        {
            if ( pParent->IsDialog() )
            {
                // gegen rekursive Aufrufe schuetzen
                if ( !((Dialog*)pParent)->IsInClose() )
                {
                    if ( ((Dialog*)pParent)->IsInExecute() )
                        ((Dialog*)pParent)->EndDialog( TRUE );
                    else if ( pParent->GetStyle() & WB_CLOSEABLE )
                        ((Dialog*)pParent)->Close();
                }
            }
            else
            {
                if ( pParent->GetStyle() & WB_CLOSEABLE )
                    ((SystemWindow*)pParent)->Close();
            }
        }
    }
    else
        PushButton::Click();
}

// =======================================================================

void CancelButton::ImplInit( Window* pParent, WinBits nStyle )
{
    PushButton::ImplInit( pParent, nStyle );

    SetText( Button::GetStandardText( BUTTON_CANCEL ) );
    SetHelpText( Button::GetStandardHelpText( BUTTON_CANCEL ) );
}

// -----------------------------------------------------------------------

CancelButton::CancelButton( Window* pParent, WinBits nStyle ) :
    PushButton( WINDOW_CANCELBUTTON )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

CancelButton::CancelButton( Window* pParent, const ResId& rResId ) :
    PushButton( WINDOW_CANCELBUTTON )
{
    rResId.SetRT( RSC_CANCELBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void CancelButton::Click()
{
    // Ist kein Link gesetzt, dann schliesse Parent
    if ( !GetClickHdl() )
    {
        Window* pParent = GetParent();
        if ( pParent->IsSystemWindow() )
        {
            if ( pParent->IsDialog() )
            {
                // gegen rekursive Aufrufe schuetzen
                if ( !((Dialog*)pParent)->IsInClose() )
                {
                    if ( ((Dialog*)pParent)->IsInExecute() )
                        ((Dialog*)pParent)->EndDialog( FALSE );
                    else if ( pParent->GetStyle() & WB_CLOSEABLE )
                        ((Dialog*)pParent)->Close();
                }
            }
            else
            {
                if ( pParent->GetStyle() & WB_CLOSEABLE )
                    ((SystemWindow*)pParent)->Close();
            }
        }
    }
    else
        PushButton::Click();
}

// =======================================================================

void HelpButton::ImplInit( Window* pParent, WinBits nStyle )
{
    PushButton::ImplInit( pParent, nStyle | WB_NOPOINTERFOCUS );

    SetText( Button::GetStandardText( BUTTON_HELP ) );
    SetHelpText( Button::GetStandardHelpText( BUTTON_HELP ) );
}

// -----------------------------------------------------------------------

HelpButton::HelpButton( Window* pParent, WinBits nStyle ) :
    PushButton( WINDOW_HELPBUTTON )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

HelpButton::HelpButton( Window* pParent, const ResId& rResId ) :
    PushButton( WINDOW_HELPBUTTON )
{
    rResId.SetRT( RSC_HELPBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void HelpButton::Click()
{
    // Ist kein Link gesetzt, loese Hilfe aus
    if ( !GetClickHdl() )
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if ( !pFocusWin )
            pFocusWin = this;

        HelpEvent aEvt( pFocusWin->GetPointerPosPixel(), HELPMODE_CONTEXT );
        pFocusWin->RequestHelp( aEvt );
    }
    else
        PushButton::Click();
}

// =======================================================================

void RadioButton::ImplInitData()
{
    mnButtonState   = 0;
    mbChecked       = FALSE;
    mbSaveValue     = FALSE;
    mbRadioCheck    = TRUE;
    mbStateChanged  = FALSE;
}

// -----------------------------------------------------------------------

void RadioButton::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( pParent->GetWindow( WINDOW_LASTCHILD ), nStyle );
    Button::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits RadioButton::ImplInitStyle( const Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) &&
         (!pPrevWindow || (pPrevWindow->GetType() != WINDOW_RADIOBUTTON)) )
        nStyle |= WB_GROUP;
    if ( !(nStyle & WB_NOTABSTOP) )
    {
        if ( IsChecked() )
            nStyle |= WB_TABSTOP;
        else
            nStyle &= ~WB_TABSTOP;
    }
    return nStyle;
}

// -----------------------------------------------------------------------

void RadioButton::ImplInitSettings( BOOL bFont,
                                    BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetRadioCheckFont();
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
            aColor = rStyleSettings.GetRadioCheckTextColor();
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

void RadioButton::ImplDrawRadioButtonState()
{
    USHORT nStyle = 0;

    // kein Image-RadioButton
    if ( !maImage )
    {
        USHORT nStyle = mnButtonState;
        if ( !IsEnabled() )
            nStyle |= BUTTON_DRAW_DISABLED;
        if ( mbChecked )
            nStyle |= BUTTON_DRAW_CHECKED;
        Image aImage = GetRadioImage( GetSettings(), nStyle );
        if ( IsZoom() )
            DrawImage( maStateRect.TopLeft(), maStateRect.GetSize(), aImage );
        else
            DrawImage( maStateRect.TopLeft(), aImage );
    }
    else
    {
        HideFocus();

        DecorationView          aDecoView( this );
        const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
        Rectangle               aImageRect  = maStateRect;
        Size                    aImageSize  = maImage.GetSizePixel();
        BOOL                    bEnabled    = IsEnabled();

        aImageSize.Width()  = CalcZoom( aImageSize.Width() );
        aImageSize.Height() = CalcZoom( aImageSize.Height() );

        // Border und Selektionsstatus ausgeben
        nStyle = FRAME_DRAW_DOUBLEIN;
        aImageRect = aDecoView.DrawFrame( aImageRect, nStyle );
        if ( (mnButtonState & BUTTON_DRAW_PRESSED) || !bEnabled )
            SetFillColor( rStyleSettings.GetFaceColor() );
        else
            SetFillColor( rStyleSettings.GetFieldColor() );
        SetLineColor();
        DrawRect( aImageRect );

        // Image ausgeben
        nStyle = 0;
        if ( !bEnabled )
            nStyle |= IMAGE_DRAW_DISABLE;
        Point aImagePos( aImageRect.TopLeft() );
        aImagePos.X() += (aImageRect.GetWidth()-aImageSize.Width())/2;
        aImagePos.Y() += (aImageRect.GetHeight()-aImageSize.Height())/2;
        if ( IsZoom() )
            DrawImage( aImagePos, aImageSize, maImage, nStyle );
        else
            DrawImage( aImagePos, maImage, nStyle );

        aImageRect.Left()++;
        aImageRect.Top()++;
        aImageRect.Right()--;
        aImageRect.Bottom()--;

        maFocusRect = aImageRect;

        if ( mbChecked )
        {
            SetLineColor( rStyleSettings.GetHighlightColor() );
            SetFillColor();
            if ( (aImageSize.Width() >= 20) || (aImageSize.Height() >= 20) )
            {
                aImageRect.Left()++;
                aImageRect.Top()++;
                aImageRect.Right()--;
                aImageRect.Bottom()--;
            }
            DrawRect( aImageRect );
            aImageRect.Left()++;
            aImageRect.Top()++;
            aImageRect.Right()--;
            aImageRect.Bottom()--;
            DrawRect( aImageRect );
        }

        if ( HasFocus() )
            ShowFocus( maFocusRect );
    }
}

// -----------------------------------------------------------------------

void RadioButton::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                            const Point& rPos, const Size& rSize,
                            const Size& rImageSize, long nImageSep,
                            Rectangle& rStateRect,
                            Rectangle& rMouseRect,
                            Rectangle& rFocusRect )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    WinBits                 nWinStyle = GetStyle();
    XubString               aText( GetText() );
    Rectangle               aRect( rPos, rSize );

    // kein Image-RadioButton
    if ( !maImage )
    {
        if ( aText.Len() )
        {
            USHORT nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
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

            aRect.Left() += rImageSize.Width()+nImageSep;
            rMouseRect = pDev->GetTextRect( aRect, aText, nTextStyle );

            pDev->DrawText( aRect, aText, nTextStyle );
            rFocusRect = rMouseRect;
            rFocusRect.Left()--;
            rFocusRect.Right()++;

            rMouseRect.Left()   = rPos.X();
            rStateRect.Left()   = rPos.X();
            rStateRect.Top()    = rMouseRect.Top();
            long nTextHeight = GetTextHeight();
            if ( nTextHeight > rImageSize.Height() )
                rStateRect.Top() += (nTextHeight-rImageSize.Height())/2;
            rStateRect.Right()  = rStateRect.Left()+rImageSize.Width()-1;
            rStateRect.Bottom() = rStateRect.Top()+rImageSize.Height()-1;
            if ( rStateRect.Bottom() > rMouseRect.Bottom() )
                rMouseRect.Bottom() = rStateRect.Bottom();
        }
        else
        {
            if ( nWinStyle & WB_CENTER )
                rStateRect.Left() = rPos.X()+((rSize.Width()-rImageSize.Width())/2);
            else if ( nWinStyle & WB_RIGHT )
                rStateRect.Left() = rPos.X()+rSize.Width()-rImageSize.Width(); //-1;
            else
                rStateRect.Left() = rPos.X(); //+1;
            if ( nWinStyle & WB_VCENTER )
                rStateRect.Top() = rPos.Y()+((rSize.Height()-rImageSize.Height())/2);
            else if ( nWinStyle & WB_BOTTOM )
                rStateRect.Top() = rPos.Y()+rSize.Height()-rImageSize.Height(); //-1;
            else
                rStateRect.Top() = rPos.Y(); //+1;
            rStateRect.Right()  = rStateRect.Left()+rImageSize.Width()-1;
            rStateRect.Bottom() = rStateRect.Top()+rImageSize.Height()-1;
            rMouseRect          = rStateRect;
            rFocusRect          = Rectangle();
/*  und oben -1, da CalcSize() auch Focus-Rechteck nicht mit einrechnet,
da im Writer ansonsten die Images noch weiter oben haengen
            rFocusRect          = rStateRect;
            rFocusRect.Left()--;
            rFocusRect.Top()--;
            rFocusRect.Right()++;
            rFocusRect.Bottom()++;
*/
        }
    }
    else
    {
        BOOL        bTopImage   = (nWinStyle & WB_TOP) != 0;
        Size        aImageSize  = maImage.GetSizePixel();
        Rectangle   aImageRect( rPos, rSize );
        long        nTextHeight = pDev->GetTextHeight();
        long        nTextWidth  = pDev->GetCtrlTextWidth( aText );

        // Positionen und Groessen berechnen
        if ( aText.Len() )
        {
            Size aTmpSize( (aImageSize.Width()+8), (aImageSize.Height()+8) );
            if ( bTopImage )
            {
                aImageRect.Left() = (rSize.Width()-aTmpSize.Width())/2;
                aImageRect.Top()  = (rSize.Height()-(aTmpSize.Height()+nTextHeight+6))/2;
            }
            else
                aImageRect.Top()  = (rSize.Height()-aTmpSize.Height())/2;

            aImageRect.Right()  = aImageRect.Left()+aTmpSize.Width();
            aImageRect.Bottom() = aImageRect.Top()+aTmpSize.Height();
        }

        // Text ausgeben
        if ( aText.Len() )
        {
            Point aTxtPos = rPos;
            if ( bTopImage )
            {
                aTxtPos.X() += (rSize.Width()-nTextWidth)/2;
                aTxtPos.Y() += aImageRect.Bottom()+6;
            }
            else
            {
                aTxtPos.X() += aImageRect.Right()+8;
                aTxtPos.Y() += (rSize.Height()-nTextHeight)/2;
            }
            pDev->DrawCtrlText( aTxtPos, aText );
        }

        rMouseRect = aImageRect;
        rStateRect = aImageRect;
    }
}

// -----------------------------------------------------------------------

void RadioButton::ImplDrawRadioButton()
{
    HideFocus();

    Size aImageSize;
    if ( !maImage )
        aImageSize = GetRadioImage( GetSettings(), 0 ).GetSizePixel();
    else
        aImageSize  = maImage.GetSizePixel();
    aImageSize.Width()  = CalcZoom( aImageSize.Width() );
    aImageSize.Height() = CalcZoom( aImageSize.Height() );

    ImplDraw( this, 0, Point(), GetOutputSizePixel(),
              aImageSize, IMPL_SEP_BUTTON_IMAGE, maStateRect, maMouseRect, maFocusRect );

    if ( !maImage )
    {
        if ( HasFocus() && !maFocusRect.IsEmpty() )
            ShowFocus( maFocusRect );
    }
    ImplDrawRadioButtonState();
}

// -----------------------------------------------------------------------

void RadioButton::ImplUncheckAllOther()
{
    mnStyle |= WB_TABSTOP;

    // Gruppe mit RadioButtons durchgehen und die gecheckten Buttons
    Window* pWindow;
    WinBits nStyle;
    if ( !(GetStyle() & WB_GROUP) )
    {
        pWindow = GetWindow( WINDOW_PREV );
        while ( pWindow )
        {
            nStyle = pWindow->GetStyle();

            if ( pWindow->GetType() == WINDOW_RADIOBUTTON )
            {
                if ( ((RadioButton*)pWindow)->IsChecked() )
                {
                    ImplDelData aDelData;
                    pWindow->ImplAddDel( &aDelData );
                    ((RadioButton*)pWindow)->SetState( FALSE );
                    if ( aDelData.IsDelete() )
                        return;
                    pWindow->ImplRemoveDel( &aDelData );
                }
                // Um falsch gesetzt WB_TABSTOPS immer zu entfernen, nicht
                // innerhalb der if-Abfrage
                pWindow->mnStyle &= ~WB_TABSTOP;
            }

            if ( nStyle & WB_GROUP )
                break;

            pWindow = pWindow->GetWindow( WINDOW_PREV );
        }
    }

    pWindow = GetWindow( WINDOW_NEXT );
    while ( pWindow )
    {
        nStyle = pWindow->GetStyle();

        if ( nStyle & WB_GROUP )
            break;

        if ( pWindow->GetType() == WINDOW_RADIOBUTTON )
        {
            if ( ((RadioButton*)pWindow)->IsChecked() )
            {
                ImplDelData aDelData;
                pWindow->ImplAddDel( &aDelData );
                ((RadioButton*)pWindow)->SetState( FALSE );
                if ( aDelData.IsDelete() )
                    return;
                pWindow->ImplRemoveDel( &aDelData );
            }
            // Um falsch gesetzt WB_TABSTOPS immer zu entfernen, nicht
            // innerhalb der if-Abfrage
            pWindow->mnStyle &= ~WB_TABSTOP;
        }

        pWindow = pWindow->GetWindow( WINDOW_NEXT );
    }
}

// -----------------------------------------------------------------------

void RadioButton::ImplCallClick( BOOL bGrabFocus, USHORT nFocusFlags )
{
    mbStateChanged = !mbChecked;
    mbChecked = TRUE;
    mnStyle |= WB_TABSTOP;
    ImplDrawRadioButtonState();
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    if ( mbRadioCheck )
        ImplUncheckAllOther();
    if ( aDelData.IsDelete() )
        return;
    if ( bGrabFocus )
        ImplGrabFocus( nFocusFlags );
    if ( aDelData.IsDelete() )
        return;
    if ( mbStateChanged )
        Toggle();
    if ( aDelData.IsDelete() )
        return;
    Click();
    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );
    mbStateChanged = FALSE;
}

// -----------------------------------------------------------------------

RadioButton::RadioButton( Window* pParent, WinBits nStyle ) :
    Button( WINDOW_RADIOBUTTON )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

RadioButton::RadioButton( Window* pParent, const ResId& rResId ) :
    Button( WINDOW_RADIOBUTTON )
{
    ImplInitData();
    rResId.SetRT( RSC_RADIOBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void RadioButton::ImplLoadRes( const ResId& rResId )
{
    Button::ImplLoadRes( rResId );

    //anderer Wert als Default ?
    USHORT nChecked = ReadShortRes();
    if ( nChecked )
        SetState( TRUE );
}

// -----------------------------------------------------------------------

RadioButton::~RadioButton()
{
}

// -----------------------------------------------------------------------

void RadioButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && maMouseRect.IsInside( rMEvt.GetPosPixel() ) )
    {
        mnButtonState |= BUTTON_DRAW_PRESSED;
        ImplDrawRadioButtonState();
        StartTracking();
        return;
    }

    Button::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void RadioButton::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mnButtonState & BUTTON_DRAW_PRESSED )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            mnButtonState &= ~BUTTON_DRAW_PRESSED;

            // Bei Abbruch kein Click-Handler rufen
            if ( !rTEvt.IsTrackingCanceled() )
                ImplCallClick();
            else
                ImplDrawRadioButtonState();
        }
    }
    else
    {
        if ( maMouseRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( !(mnButtonState & BUTTON_DRAW_PRESSED) )
            {
                mnButtonState |= BUTTON_DRAW_PRESSED;
                ImplDrawRadioButtonState();
            }
        }
        else
        {
            if ( mnButtonState & BUTTON_DRAW_PRESSED )
            {
                mnButtonState &= ~BUTTON_DRAW_PRESSED;
                ImplDrawRadioButtonState();
            }
        }
    }
}

// -----------------------------------------------------------------------

void RadioButton::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( !aKeyCode.GetModifier() && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        if ( !(mnButtonState & BUTTON_DRAW_PRESSED) )
        {
            mnButtonState |= BUTTON_DRAW_PRESSED;
            ImplDrawRadioButtonState();
        }
    }
    else if ( (mnButtonState & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplDrawRadioButtonState();
    }
    else
        Button::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void RadioButton::KeyUp( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (mnButtonState & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplCallClick();
    }
    else
        Button::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void RadioButton::Paint( const Rectangle& rRect )
{
    ImplDrawRadioButton();
}

// -----------------------------------------------------------------------

void RadioButton::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                        ULONG nFlags )
{
    if ( !maImage )
    {
        MapMode     aResMapMode( MAP_100TH_MM );
        Point       aPos  = pDev->LogicToPixel( rPos );
        Size        aSize = pDev->LogicToPixel( rSize );
        Size        aImageSize = pDev->LogicToPixel( Size( 300, 300 ), aResMapMode );
        Size        aBrd1Size = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode );
        Size        aBrd2Size = pDev->LogicToPixel( Size( 60, 60 ), aResMapMode );
        Font        aFont = GetDrawPixelFont( pDev );
        Rectangle   aStateRect;
        Rectangle   aMouseRect;
        Rectangle   aFocusRect;

        aImageSize.Width()  = CalcZoom( aImageSize.Width() );
        aImageSize.Height() = CalcZoom( aImageSize.Height() );
        aBrd1Size.Width()   = CalcZoom( aBrd1Size.Width() );
        aBrd1Size.Height()  = CalcZoom( aBrd1Size.Height() );
        aBrd2Size.Width()   = CalcZoom( aBrd2Size.Width() );
        aBrd2Size.Height()  = CalcZoom( aBrd2Size.Height() );

        if ( !aBrd1Size.Width() )
            aBrd1Size.Width() = 1;
        if ( !aBrd1Size.Height() )
            aBrd1Size.Height() = 1;
        if ( !aBrd2Size.Width() )
            aBrd2Size.Width() = 1;
        if ( !aBrd2Size.Height() )
            aBrd2Size.Height() = 1;

        pDev->Push();
        pDev->SetMapMode();
        pDev->SetFont( aFont );
        if ( nFlags & WINDOW_DRAW_MONO )
            pDev->SetTextColor( Color( COL_BLACK ) );
        else
            pDev->SetTextColor( GetTextColor() );
        pDev->SetTextFillColor();

        ImplDraw( pDev, nFlags, aPos, aSize,
                  aImageSize, GetDrawPixel( pDev, IMPL_SEP_BUTTON_IMAGE ),
                  aStateRect, aMouseRect, aFocusRect );

        Point   aCenterPos = aStateRect.Center();
        long    nRadX = aImageSize.Width()/2;
        long    nRadY = aImageSize.Height()/2;

        pDev->SetLineColor();
        pDev->SetFillColor( Color( COL_BLACK ) );
        pDev->DrawPolygon( Polygon( aCenterPos, nRadX, nRadY ) );
        nRadX -= aBrd1Size.Width();
        nRadY -= aBrd1Size.Height();
        pDev->SetFillColor( Color( COL_WHITE ) );
        pDev->DrawPolygon( Polygon( aCenterPos, nRadX, nRadY ) );
        if ( mbChecked )
        {
            nRadX -= aBrd1Size.Width();
            nRadY -= aBrd1Size.Height();
            if ( !nRadX )
                nRadX = 1;
            if ( !nRadY )
                nRadY = 1;
            pDev->SetFillColor( Color( COL_BLACK ) );
            pDev->DrawPolygon( Polygon( aCenterPos, nRadX, nRadY ) );
        }

        pDev->Pop();
    }
    else
    {
        DBG_ERROR( "RadioButton::Draw() - not implemented for RadioButton with Image" );
    }
}

// -----------------------------------------------------------------------

void RadioButton::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void RadioButton::GetFocus()
{
    ShowFocus( maFocusRect );
    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

// -----------------------------------------------------------------------

void RadioButton::LoseFocus()
{
    if ( mnButtonState & BUTTON_DRAW_PRESSED )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplDrawRadioButtonState();
    }

    HideFocus();
    Button::LoseFocus();
}

// -----------------------------------------------------------------------

void RadioButton::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( nType == STATE_CHANGE_STATE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( HasPaintEvent() )
                Invalidate( maStateRect );
            else
                ImplDrawRadioButtonState();
        }
        Toggle();
    }
    else if ( (nType == STATE_CHANGE_ENABLE) ||
              (nType == STATE_CHANGE_TEXT) ||
              (nType == STATE_CHANGE_IMAGE) ||
              (nType == STATE_CHANGE_DATA) ||
              (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetWindow( WINDOW_PREV ), GetStyle() ) );

        if ( (GetPrevStyle() & RADIOBUTTON_VIEW_STYLE) !=
             (GetStyle() & RADIOBUTTON_VIEW_STYLE) )
        {
            if ( IsUpdateMode() )
                Invalidate();
        }
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
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

void RadioButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    Button::DataChanged( rDCEvt );

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

void RadioButton::Toggle()
{
    maToggleHdl.Call( this );
}

// -----------------------------------------------------------------------

void RadioButton::SetImage( const Image& rImage )
{
    if ( rImage != maImage )
    {
        maImage = rImage;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void RadioButton::SetState( BOOL bCheck )
{
    // TabStop-Flag richtig mitfuehren
    if ( bCheck )
        mnStyle |= WB_TABSTOP;
    else
        mnStyle &= ~WB_TABSTOP;

    if ( mbChecked != bCheck )
    {
        mbChecked = bCheck;
        StateChanged( STATE_CHANGE_STATE );
    }
}

// -----------------------------------------------------------------------

void RadioButton::Check( BOOL bCheck )
{
    // TabStop-Flag richtig mitfuehren
    if ( bCheck )
        mnStyle |= WB_TABSTOP;
    else
        mnStyle &= ~WB_TABSTOP;

    if ( mbChecked != bCheck )
    {
        mbChecked = bCheck;
        ImplDelData aDelData;
        ImplAddDel( &aDelData );
        StateChanged( STATE_CHANGE_STATE );
        if ( aDelData.IsDelete() )
            return;
        ImplRemoveDel( &aDelData );
        if ( bCheck && mbRadioCheck )
            ImplUncheckAllOther();
    }
}

// -----------------------------------------------------------------------

Image RadioButton::GetRadioImage( const AllSettings& rSettings, USHORT nFlags )
{
    ImplSVData*             pSVData = ImplGetSVData();
    const StyleSettings&    rStyleSettings = rSettings.GetStyleSettings();
    USHORT                  nStyle = rStyleSettings.GetRadioButtonStyle() & STYLE_RADIOBUTTON_STYLE;

    if ( !pSVData->maCtrlData.mpRadioImgList ||
         (pSVData->maCtrlData.mnRadioStyle != nStyle) ||
         (pSVData->maCtrlData.mnLastRadioFColor != rStyleSettings.GetFaceColor().GetColor()) ||
         (pSVData->maCtrlData.mnLastRadioWColor != rStyleSettings.GetWindowColor().GetColor()) ||
         (pSVData->maCtrlData.mnLastRadioLColor != rStyleSettings.GetLightColor().GetColor()) )
    {
        if ( pSVData->maCtrlData.mpRadioImgList )
            delete pSVData->maCtrlData.mpRadioImgList;

        pSVData->maCtrlData.mnLastRadioFColor = rStyleSettings.GetFaceColor().GetColor();
        pSVData->maCtrlData.mnLastRadioWColor = rStyleSettings.GetWindowColor().GetColor();
        pSVData->maCtrlData.mnLastRadioLColor = rStyleSettings.GetLightColor().GetColor();

        long    aTempAry1[(6*sizeof(Color))/sizeof(long)];
        long    aTempAry2[(6*sizeof(Color))/sizeof(long)];
        Color*  pColorAry1 = (Color*)aTempAry1;
        Color*  pColorAry2 = (Color*)aTempAry2;
        pColorAry1[0] = Color( 0xC0, 0xC0, 0xC0 );
        pColorAry1[1] = Color( 0xFF, 0xFF, 0x00 );
        pColorAry1[2] = Color( 0xFF, 0xFF, 0xFF );
        pColorAry1[3] = Color( 0x80, 0x80, 0x80 );
        pColorAry1[4] = Color( 0x00, 0x00, 0x00 );
        pColorAry1[5] = Color( 0x00, 0xFF, 0x00 );
        pColorAry2[0] = rStyleSettings.GetFaceColor();
        pColorAry2[1] = rStyleSettings.GetWindowColor();
        pColorAry2[2] = rStyleSettings.GetLightColor();
        pColorAry2[3] = rStyleSettings.GetShadowColor();
        pColorAry2[4] = rStyleSettings.GetDarkShadowColor();
        pColorAry2[5] = rStyleSettings.GetWindowTextColor();

        Bitmap aBmp( ResId( SV_RESID_BITMAP_RADIO+nStyle, ImplGetResMgr() ) );
        aBmp.Replace( pColorAry1, pColorAry2, 6, NULL );
        pSVData->maCtrlData.mpRadioImgList = new ImageList( aBmp, Color( 0x00, 0x00, 0xFF ), 6 );
        pSVData->maCtrlData.mnRadioStyle = nStyle;
    }

    USHORT nId;
    if ( nFlags & BUTTON_DRAW_DISABLED )
    {
        if ( nFlags & BUTTON_DRAW_CHECKED )
            nId = 6;
        else
            nId = 5;
    }
    else if ( nFlags & BUTTON_DRAW_PRESSED )
    {
        if ( nFlags & BUTTON_DRAW_CHECKED )
            nId = 4;
        else
            nId = 3;
    }
    else
    {
        if ( nFlags & BUTTON_DRAW_CHECKED )
            nId = 2;
        else
            nId = 1;
    }
    return pSVData->maCtrlData.mpRadioImgList->GetImage( nId );
}

// -----------------------------------------------------------------------

Size RadioButton::CalcMinimumSize( long nMaxWidth ) const
{
    Size aSize;
    if ( !maImage )
        aSize = GetRadioImage( GetSettings(), 0 ).GetSizePixel();
    else
        aSize = maImage.GetSizePixel();

    XubString aText = GetText();
    if ( aText.Len() )
    {
        Size aTextSize = GetTextRect( Rectangle( Point(), Size( nMaxWidth ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                                      aText, FixedText::ImplGetTextStyle( GetStyle() ) ).GetSize();
        aSize.Width() += IMPL_SEP_BUTTON_IMAGE;
        aSize.Width() += aTextSize.Width();
        if ( aSize.Height() < aTextSize.Height() )
            aSize.Height() = aTextSize.Height();
    }
    else if ( !maImage )
    {
/* da ansonsten im Writer die Control zu weit oben haengen
        aSize.Width() += 2;
        aSize.Height() += 2;
*/
    }

    return CalcWindowSize( aSize );
}

// =======================================================================

void CheckBox::ImplInitData()
{
    mnButtonState   = 0;
    meState         = STATE_NOCHECK;
    meSaveValue     = STATE_NOCHECK;
    mbTriState      = FALSE;
}

// -----------------------------------------------------------------------

void CheckBox::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( pParent->GetWindow( WINDOW_LASTCHILD ), nStyle );
    Button::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits CheckBox::ImplInitStyle( const Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) &&
         (!pPrevWindow || (pPrevWindow->GetType() != WINDOW_CHECKBOX)) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void CheckBox::ImplInitSettings( BOOL bFont,
                                 BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetRadioCheckFont();
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
            aColor = rStyleSettings.GetRadioCheckTextColor();
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

void CheckBox::ImplLoadRes( const ResId& rResId )
{
    Button::ImplLoadRes( rResId );

    if ( rResId.GetRT() != RSC_TRISTATEBOX )
    {
        USHORT nChecked = ReadShortRes();
        //anderer Wert als Default ?
        if( nChecked )
            Check( TRUE );
    }
}

// -----------------------------------------------------------------------

void CheckBox::ImplDrawCheckBoxState()
{
    USHORT nStyle = mnButtonState;
    if ( !IsEnabled() )
        nStyle |= BUTTON_DRAW_DISABLED;
    if ( meState == STATE_DONTKNOW )
        nStyle |= BUTTON_DRAW_DONTKNOW;
    else if ( meState == STATE_CHECK )
        nStyle |= BUTTON_DRAW_CHECKED;
    Image aImage = GetCheckImage( GetSettings(), nStyle );
    if ( IsZoom() )
        DrawImage( maStateRect.TopLeft(), maStateRect.GetSize(), aImage );
    else
        DrawImage( maStateRect.TopLeft(), aImage );
}

// -----------------------------------------------------------------------

void CheckBox::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                         const Point& rPos, const Size& rSize,
                         const Size& rImageSize, long nImageSep,
                         Rectangle& rStateRect,
                         Rectangle& rMouseRect,
                         Rectangle& rFocusRect )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    WinBits                 nWinStyle = GetStyle();
    XubString               aText( GetText() );
    Rectangle               aRect( rPos, rSize );

    if ( aText.Len() )
    {
        USHORT nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
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

        aRect.Left() += rImageSize.Width()+nImageSep;
        rMouseRect = pDev->GetTextRect( aRect, aText, nTextStyle );

        pDev->DrawText( aRect, aText, nTextStyle );
        rFocusRect = rMouseRect;
        rFocusRect.Left()--;
        rFocusRect.Right()++;

        rMouseRect.Left()   = rPos.X();
        rStateRect.Left()   = rPos.X();
        rStateRect.Top()    = rMouseRect.Top();
        long nTextHeight = GetTextHeight();
        if ( nTextHeight > rImageSize.Height() )
            rStateRect.Top() += (nTextHeight-rImageSize.Height())/2;
        rStateRect.Right()  = rStateRect.Left()+rImageSize.Width()-1;
        rStateRect.Bottom() = rStateRect.Top()+rImageSize.Height()-1;
        if ( rStateRect.Bottom() > rMouseRect.Bottom() )
            rMouseRect.Bottom() = rStateRect.Bottom();
    }
    else
    {
        if ( nWinStyle & WB_CENTER )
            rStateRect.Left() = rPos.X()+((rSize.Width()-rImageSize.Width())/2);
        else if ( nWinStyle & WB_RIGHT )
            rStateRect.Left() = rPos.X()+rSize.Width()-rImageSize.Width(); //-1;
        else
            rStateRect.Left() = rPos.X(); //+1;
        if ( nWinStyle & WB_VCENTER )
            rStateRect.Top() = rPos.Y()+((rSize.Height()-rImageSize.Height())/2);
        else if ( nWinStyle & WB_BOTTOM )
            rStateRect.Top() = rPos.Y()+rSize.Height()-rImageSize.Height(); //-1;
        else
            rStateRect.Top() = rPos.Y(); //+1;
        rStateRect.Right()  = rStateRect.Left()+rImageSize.Width()-1;
        rStateRect.Bottom() = rStateRect.Top()+rImageSize.Height()-1;
        rMouseRect          = rStateRect;
        rFocusRect          = Rectangle();
/*  und oben -1, da CalcSize() auch Focus-Rechteck nicht mit einrechnet,
da im Writer ansonsten die Images noch weiter oben haengen
        rFocusRect          = rStateRect;
        rFocusRect.Left()--;
        rFocusRect.Top()--;
        rFocusRect.Right()++;
        rFocusRect.Bottom()++;
*/
    }
}

// -----------------------------------------------------------------------

void CheckBox::ImplDrawCheckBox()
{
    Size aImageSize = GetCheckImage( GetSettings(), 0 ).GetSizePixel();
    aImageSize.Width()  = CalcZoom( aImageSize.Width() );
    aImageSize.Height() = CalcZoom( aImageSize.Height() );

    HideFocus();
    ImplDraw( this, 0, Point(), GetOutputSizePixel(),
              aImageSize, IMPL_SEP_BUTTON_IMAGE, maStateRect, maMouseRect, maFocusRect );
    if ( HasFocus() && !maFocusRect.IsEmpty() )
        ShowFocus( maFocusRect );
    ImplDrawCheckBoxState();
}

// -----------------------------------------------------------------------

void CheckBox::ImplCheck()
{
    TriState eNewState;
    if ( meState == STATE_NOCHECK )
        eNewState = STATE_CHECK;
    else if ( !mbTriState )
        eNewState = STATE_NOCHECK;
    else if ( meState == STATE_CHECK )
        eNewState = STATE_DONTKNOW;
    else
        eNewState = STATE_NOCHECK;
    meState = eNewState;
    ImplDrawCheckBoxState();
    Toggle();
    Click();
}

// -----------------------------------------------------------------------

CheckBox::CheckBox( Window* pParent, WinBits nStyle ) :
    Button( WINDOW_CHECKBOX )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

CheckBox::CheckBox( Window* pParent, const ResId& rResId ) :
    Button( WINDOW_CHECKBOX )
{
    ImplInitData();
    rResId.SetRT( RSC_CHECKBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void CheckBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && maMouseRect.IsInside( rMEvt.GetPosPixel() ) )
    {
        mnButtonState |= BUTTON_DRAW_PRESSED;
        ImplDrawCheckBoxState();
        StartTracking();
        return;
    }

    Button::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void CheckBox::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( mnButtonState & BUTTON_DRAW_PRESSED )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            mnButtonState &= ~BUTTON_DRAW_PRESSED;

            // Bei Abbruch kein Click-Handler rufen
            if ( !rTEvt.IsTrackingCanceled() )
                ImplCheck();
            else
                ImplDrawCheckBoxState();
        }
    }
    else
    {
        if ( maMouseRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( !(mnButtonState & BUTTON_DRAW_PRESSED) )
            {
                mnButtonState |= BUTTON_DRAW_PRESSED;
                ImplDrawCheckBoxState();
            }
        }
        else
        {
            if ( mnButtonState & BUTTON_DRAW_PRESSED )
            {
                mnButtonState &= ~BUTTON_DRAW_PRESSED;
                ImplDrawCheckBoxState();
            }
        }
    }
}

// -----------------------------------------------------------------------

void CheckBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( !aKeyCode.GetModifier() && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        if ( !(mnButtonState & BUTTON_DRAW_PRESSED) )
        {
            mnButtonState |= BUTTON_DRAW_PRESSED;
            ImplDrawCheckBoxState();
        }
    }
    else if ( (mnButtonState & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplDrawCheckBoxState();
    }
    else
        Button::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void CheckBox::KeyUp( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (mnButtonState & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplCheck();
    }
    else
        Button::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void CheckBox::Paint( const Rectangle& rRect )
{
    ImplDrawCheckBox();
}

// -----------------------------------------------------------------------

void CheckBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                     ULONG nFlags )
{
    MapMode     aResMapMode( MAP_100TH_MM );
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Size        aImageSize = pDev->LogicToPixel( Size( 300, 300 ), aResMapMode );
    Size        aBrd1Size = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode );
    Size        aBrd2Size = pDev->LogicToPixel( Size( 30, 30 ), aResMapMode );
    long        nCheckWidth = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode ).Width();
    Font        aFont = GetDrawPixelFont( pDev );
    Rectangle   aStateRect;
    Rectangle   aMouseRect;
    Rectangle   aFocusRect;

    aImageSize.Width()  = CalcZoom( aImageSize.Width() );
    aImageSize.Height() = CalcZoom( aImageSize.Height() );
    aBrd1Size.Width()   = CalcZoom( aBrd1Size.Width() );
    aBrd1Size.Height()  = CalcZoom( aBrd1Size.Height() );
    aBrd2Size.Width()   = CalcZoom( aBrd2Size.Width() );
    aBrd2Size.Height()  = CalcZoom( aBrd2Size.Height() );

    if ( !aBrd1Size.Width() )
        aBrd1Size.Width() = 1;
    if ( !aBrd1Size.Height() )
        aBrd1Size.Height() = 1;
    if ( !aBrd2Size.Width() )
        aBrd2Size.Width() = 1;
    if ( !aBrd2Size.Height() )
        aBrd2Size.Height() = 1;
    if ( !nCheckWidth )
        nCheckWidth = 1;

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & WINDOW_DRAW_MONO )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    ImplDraw( pDev, nFlags, aPos, aSize,
              aImageSize, GetDrawPixel( pDev, IMPL_SEP_BUTTON_IMAGE ),
              aStateRect, aMouseRect, aFocusRect );

    pDev->SetLineColor();
    pDev->SetFillColor( Color( COL_BLACK ) );
    pDev->DrawRect( aStateRect );
    aStateRect.Left()   += aBrd1Size.Width();
    aStateRect.Top()    += aBrd1Size.Height();
    aStateRect.Right()  -= aBrd1Size.Width();
    aStateRect.Bottom() -= aBrd1Size.Height();
    if ( meState == STATE_DONTKNOW )
        pDev->SetFillColor( Color( COL_LIGHTGRAY ) );
    else
        pDev->SetFillColor( Color( COL_WHITE ) );
    pDev->DrawRect( aStateRect );

    if ( meState == STATE_CHECK )
    {
        aStateRect.Left()   += aBrd2Size.Width();
        aStateRect.Top()    += aBrd2Size.Height();
        aStateRect.Right()  -= aBrd2Size.Width();
        aStateRect.Bottom() -= aBrd2Size.Height();
        Point   aPos11( aStateRect.TopLeft() );
        Point   aPos12( aStateRect.BottomRight() );
        Point   aPos21( aStateRect.TopRight() );
        Point   aPos22( aStateRect.BottomLeft() );
        Point   aTempPos11( aPos11 );
        Point   aTempPos12( aPos12 );
        Point   aTempPos21( aPos21 );
        Point   aTempPos22( aPos22 );
        pDev->SetLineColor( Color( COL_BLACK ) );
        long nDX = 0;
        for ( long i = 0; i < nCheckWidth; i++ )
        {
            if ( !(i % 2) )
            {
                aTempPos11.X() = aPos11.X()+nDX;
                aTempPos12.X() = aPos12.X()+nDX;
                aTempPos21.X() = aPos21.X()+nDX;
                aTempPos22.X() = aPos22.X()+nDX;
            }
            else
            {
                nDX++;
                aTempPos11.X() = aPos11.X()-nDX;
                aTempPos12.X() = aPos12.X()-nDX;
                aTempPos21.X() = aPos21.X()-nDX;
                aTempPos22.X() = aPos22.X()-nDX;
            }
            pDev->DrawLine( aTempPos11, aTempPos12 );
            pDev->DrawLine( aTempPos21, aTempPos22 );
        }
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void CheckBox::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void CheckBox::GetFocus()
{
    ShowFocus( maFocusRect );
    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

// -----------------------------------------------------------------------

void CheckBox::LoseFocus()
{
    if ( mnButtonState & BUTTON_DRAW_PRESSED )
    {
        mnButtonState &= ~BUTTON_DRAW_PRESSED;
        ImplDrawCheckBoxState();
    }

    HideFocus();
    Button::LoseFocus();
}

// -----------------------------------------------------------------------

void CheckBox::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( nType == STATE_CHANGE_STATE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( HasPaintEvent() )
                Invalidate( maStateRect );
            else
                ImplDrawCheckBoxState();
        }
        Toggle();
    }
    else if ( (nType == STATE_CHANGE_ENABLE) ||
              (nType == STATE_CHANGE_TEXT) ||
              (nType == STATE_CHANGE_IMAGE) ||
              (nType == STATE_CHANGE_DATA) ||
              (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetWindow( WINDOW_PREV ), GetStyle() ) );

        if ( (GetPrevStyle() & CHECKBOX_VIEW_STYLE) !=
             (GetStyle() & CHECKBOX_VIEW_STYLE) )
        {
            if ( IsUpdateMode() )
                Invalidate();
        }
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
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

void CheckBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Button::DataChanged( rDCEvt );

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

void CheckBox::Toggle()
{
    maToggleHdl.Call( this );
}

// -----------------------------------------------------------------------

void CheckBox::SetState( TriState eState )
{
    if ( !mbTriState && (eState == STATE_DONTKNOW) )
        eState = STATE_NOCHECK;

    if ( meState != eState )
    {
        meState = eState;
        StateChanged( STATE_CHANGE_STATE );
    }
}

// -----------------------------------------------------------------------

void CheckBox::EnableTriState( BOOL bTriState )
{
    if ( mbTriState != bTriState )
    {
        mbTriState = bTriState;

        if ( !bTriState && (meState == STATE_DONTKNOW) )
            SetState( STATE_NOCHECK );
    }
}

// -----------------------------------------------------------------------

Image CheckBox::GetCheckImage( const AllSettings& rSettings, USHORT nFlags )
{
    ImplSVData*             pSVData = ImplGetSVData();
    const StyleSettings&    rStyleSettings = rSettings.GetStyleSettings();
    USHORT                  nStyle = rStyleSettings.GetCheckBoxStyle() & STYLE_CHECKBOX_STYLE;

    if ( !pSVData->maCtrlData.mpCheckImgList ||
         (pSVData->maCtrlData.mnCheckStyle != nStyle) ||
         (pSVData->maCtrlData.mnLastCheckFColor != rStyleSettings.GetFaceColor().GetColor()) ||
         (pSVData->maCtrlData.mnLastCheckWColor != rStyleSettings.GetWindowColor().GetColor()) ||
         (pSVData->maCtrlData.mnLastCheckLColor != rStyleSettings.GetLightColor().GetColor()) )
    {
        if ( pSVData->maCtrlData.mpCheckImgList )
            delete pSVData->maCtrlData.mpCheckImgList;

        pSVData->maCtrlData.mnLastCheckFColor = rStyleSettings.GetFaceColor().GetColor();
        pSVData->maCtrlData.mnLastCheckWColor = rStyleSettings.GetWindowColor().GetColor();
        pSVData->maCtrlData.mnLastCheckLColor = rStyleSettings.GetLightColor().GetColor();

        long    aTempAry1[(6*sizeof(Color))/sizeof(long)];
        long    aTempAry2[(6*sizeof(Color))/sizeof(long)];
        Color*  pColorAry1 = (Color*)aTempAry1;
        Color*  pColorAry2 = (Color*)aTempAry2;
        pColorAry1[0] = Color( 0xC0, 0xC0, 0xC0 );
        pColorAry1[1] = Color( 0xFF, 0xFF, 0x00 );
        pColorAry1[2] = Color( 0xFF, 0xFF, 0xFF );
        pColorAry1[3] = Color( 0x80, 0x80, 0x80 );
        pColorAry1[4] = Color( 0x00, 0x00, 0x00 );
        pColorAry1[5] = Color( 0x00, 0xFF, 0x00 );
        pColorAry2[0] = rStyleSettings.GetFaceColor();
        pColorAry2[1] = rStyleSettings.GetWindowColor();
        pColorAry2[2] = rStyleSettings.GetLightColor();
        pColorAry2[3] = rStyleSettings.GetShadowColor();
        pColorAry2[4] = rStyleSettings.GetDarkShadowColor();
        pColorAry2[5] = rStyleSettings.GetWindowTextColor();

        Bitmap aBmp( ResId( SV_RESID_BITMAP_CHECK+nStyle, ImplGetResMgr() ) );
        aBmp.Replace( pColorAry1, pColorAry2, 6, NULL );
        pSVData->maCtrlData.mpCheckImgList = new ImageList( aBmp, 9 );
        pSVData->maCtrlData.mnCheckStyle = nStyle;
    }

    USHORT nId;
    if ( nFlags & BUTTON_DRAW_DISABLED )
    {
        if ( nFlags & BUTTON_DRAW_DONTKNOW )
            nId = 9;
        else if ( nFlags & BUTTON_DRAW_CHECKED )
            nId = 6;
        else
            nId = 5;
    }
    else if ( nFlags & BUTTON_DRAW_PRESSED )
    {
        if ( nFlags & BUTTON_DRAW_DONTKNOW )
            nId = 8;
        else if ( nFlags & BUTTON_DRAW_CHECKED )
            nId = 4;
        else
            nId = 3;
    }
    else
    {
        if ( nFlags & BUTTON_DRAW_DONTKNOW )
            nId = 7;
        else if ( nFlags & BUTTON_DRAW_CHECKED )
            nId = 2;
        else
            nId = 1;
    }
    return pSVData->maCtrlData.mpCheckImgList->GetImage( nId );
}

// -----------------------------------------------------------------------

Size CheckBox::CalcMinimumSize( long nMaxWidth ) const
{
    Size aSize = GetCheckImage( GetSettings(), 0 ).GetSizePixel();

    XubString aText = GetText();
    if ( aText.Len() )
    {
        Size aTextSize = GetTextRect( Rectangle( Point(), Size( nMaxWidth ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                                      aText, FixedText::ImplGetTextStyle( GetStyle() ) ).GetSize();
        aSize.Width() += IMPL_SEP_BUTTON_IMAGE;
        aSize.Width() += aTextSize.Width();
        if ( aSize.Height() < aTextSize.Height() )
            aSize.Height() = aTextSize.Height();
    }
    else
    {
/* da ansonsten im Writer die Control zu weit oben haengen
        aSize.Width() += 2;
        aSize.Height() += 2;
*/
    }

    return CalcWindowSize( aSize );
}

// =======================================================================

ImageButton::ImageButton( WindowType nType ) :
    PushButton( nType )
{
}

// -----------------------------------------------------------------------

ImageButton::ImageButton( Window* pParent, WinBits nStyle ) :
    PushButton( pParent, nStyle )
{
}

// -----------------------------------------------------------------------

ImageButton::ImageButton( Window* pParent, const ResId& rResId ) :
    PushButton( pParent, rResId.SetRT( RSC_IMAGEBUTTON ) )
{
    USHORT nObjMask = ReadShortRes();

    if ( RSC_IMAGEBUTTON_IMAGE & nObjMask )
    {
        SetImage( Image( ResId( (RSHEADER_TYPE*)GetClassRes() ) ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }

    if ( RSC_IMAGEBUTTON_SYMBOL & nObjMask )
        SetSymbol( (SymbolType)ReadShortRes() );

    if ( RSC_IMAGEBUTTON_STATE & nObjMask )
        SetState( (TriState)ReadShortRes() );
}

// -----------------------------------------------------------------------

ImageButton::~ImageButton()
{
}

// =======================================================================

ImageRadioButton::ImageRadioButton( Window* pParent, WinBits nStyle ) :
    RadioButton( pParent, nStyle )
{
}

// -----------------------------------------------------------------------

ImageRadioButton::ImageRadioButton( Window* pParent, const ResId& rResId ) :
    RadioButton( pParent, rResId.SetRT( RSC_IMAGERADIOBUTTON ) )
{
    USHORT nObjMask = ReadShortRes();

    if ( RSC_IMAGERADIOBUTTON_IMAGE & nObjMask )
    {
        SetImage( Image( ResId( (RSHEADER_TYPE*)GetClassRes() ) ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
}

// -----------------------------------------------------------------------

ImageRadioButton::~ImageRadioButton()
{
}

// =======================================================================

TriStateBox::TriStateBox( Window* pParent, WinBits nStyle ) :
    CheckBox( pParent, nStyle )
{
    EnableTriState( TRUE );
}

// -----------------------------------------------------------------------

TriStateBox::TriStateBox( Window* pParent, const ResId& rResId ) :
    CheckBox( pParent, rResId.SetRT( RSC_TRISTATEBOX ) )
{
    EnableTriState( TRUE );

    USHORT nTriState        = ReadShortRes();
    USHORT bDisableTriState = ReadShortRes();
    //anderer Wert als Default ?
    if ( (TriState)nTriState != STATE_NOCHECK )
        SetState( (TriState)nTriState );
    if ( bDisableTriState )
        EnableTriState( FALSE );
}

// -----------------------------------------------------------------------

TriStateBox::~TriStateBox()
{
}
