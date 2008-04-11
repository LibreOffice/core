/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: button.cxx,v $
 * $Revision: 1.60 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/debug.hxx>

#ifndef _SV_SVIDS_HRC
#include <vcl/svids.hrc>
#endif
#include <vcl/svdata.hxx>
#ifndef _SV_IAMGE_HXX
#include <vcl/image.hxx>
#endif
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <tools/poly.hxx>
#include <vcl/button.hxx>
#include <vcl/window.h>
#include <vcl/controllayout.hxx>
#ifndef _SV_NATIVEWIDGET_HXX
#include <vcl/salnativewidgets.hxx>
#endif
#include <vcl/edit.hxx>

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif



// =======================================================================

#define PUSHBUTTON_VIEW_STYLE       (WB_3DLOOK |                        \
                                     WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                     WB_WORDBREAK | WB_NOLABEL |        \
                                     WB_DEFBUTTON | WB_NOLIGHTBORDER |  \
                                     WB_RECTSTYLE | WB_SMALLSTYLE |     \
                                     WB_TOGGLE )
#define RADIOBUTTON_VIEW_STYLE      (WB_3DLOOK |                        \
                                     WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                     WB_WORDBREAK | WB_NOLABEL)
#define CHECKBOX_VIEW_STYLE         (WB_3DLOOK |                        \
                                     WB_LEFT | WB_CENTER | WB_RIGHT |   \
                                     WB_TOP | WB_VCENTER | WB_BOTTOM |  \
                                     WB_WORDBREAK | WB_NOLABEL)

// =======================================================================

class ImplCommonButtonData
{
public:
    Rectangle       maFocusRect;
    USHORT          mnButtonState;
    BOOL            mbSmallSymbol;

    Image           maImage;
    Image           maImageHC;
    BitmapEx*       mpBitmapEx;
    BitmapEx*       mpBitmapExHC;
    ImageAlign      meImageAlign;
    SymbolAlign     meSymbolAlign;

public:
                    ImplCommonButtonData();
                   ~ImplCommonButtonData();
};

// -----------------------------------------------------------------------
ImplCommonButtonData::ImplCommonButtonData()
{
    mnButtonState   = 0;
    mbSmallSymbol = FALSE;

    mpBitmapEx = NULL;
    mpBitmapExHC = NULL;
    meImageAlign = IMAGEALIGN_TOP;
    meSymbolAlign = SYMBOLALIGN_LEFT;
}

// -----------------------------------------------------------------------
ImplCommonButtonData::~ImplCommonButtonData()
{
    delete mpBitmapEx;
    delete mpBitmapExHC;
}

// =======================================================================

Button::Button( WindowType nType ) :
    Control( nType )
{
    mpButtonData = new ImplCommonButtonData;
}

// -----------------------------------------------------------------------

Button::Button( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_BUTTON )
{
    mpButtonData = new ImplCommonButtonData;
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Button::Button( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_BUTTON )
{
    rResId.SetRT( RSC_BUTTON );
    mpButtonData = new ImplCommonButtonData;
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Button::~Button()
{
    delete mpButtonData;
}

// -----------------------------------------------------------------------

void Button::Click()
{
    ImplCallEventListenersAndHandler( VCLEVENT_BUTTON_CLICK, maClickHdl, this );
}

// -----------------------------------------------------------------------

XubString Button::GetStandardText( StandardButtonType eButton )
{
    static struct
    {
        sal_uInt32 nResId;
        const char* pDefText;
    } aResIdAry[BUTTON_COUNT] =
    {
        { SV_BUTTONTEXT_OK, "~OK" },
        { SV_BUTTONTEXT_CANCEL, "~Cancel" },
        { SV_BUTTONTEXT_YES, "~Yes" },
        { SV_BUTTONTEXT_NO, "~No" },
        { SV_BUTTONTEXT_RETRY, "~Retry" },
        { SV_BUTTONTEXT_HELP, "~Help" },
        { SV_BUTTONTEXT_CLOSE, "~Close" },
        { SV_BUTTONTEXT_MORE, "~More" },
        { SV_BUTTONTEXT_IGNORE, "~Ignore" },
        { SV_BUTTONTEXT_ABORT, "~Abort" },
        { SV_BUTTONTEXT_LESS, "~Less" }
    };

    String aText;
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
    {
        ResId aResId( aResIdAry[(USHORT)eButton].nResId, *pResMgr );
        aText = String( aResId );
    }
    else
    {
        ByteString aT( aResIdAry[(USHORT)eButton].pDefText );
        aText = String( aT, RTL_TEXTENCODING_ASCII_US );
    }
    return aText;
}

// -----------------------------------------------------------------------

XubString Button::GetStandardHelpText( StandardButtonType /* eButton */ )
{
    XubString aHelpText;
    return aHelpText;
}
// -----------------------------------------------------------------------
BOOL Button::SetModeImage( const Image& rImage, BmpColorMode eMode )
{
    if( eMode == BMP_COLOR_NORMAL )
    {
        if ( rImage != mpButtonData->maImage )
        {
            delete mpButtonData->mpBitmapEx;

            mpButtonData->mpBitmapEx = NULL;
            mpButtonData->maImage = rImage;

            StateChanged( STATE_CHANGE_DATA );
        }
    }
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
    {
        if( rImage != mpButtonData->maImageHC )
        {
            delete mpButtonData->mpBitmapExHC;

            mpButtonData->mpBitmapExHC = NULL;
            mpButtonData->maImageHC = rImage;

            StateChanged( STATE_CHANGE_DATA );
        }
    }
    else
        return FALSE;

    return TRUE;
}

// -----------------------------------------------------------------------
const Image Button::GetModeImage( BmpColorMode eMode ) const
{
    if( eMode == BMP_COLOR_NORMAL )
    {
        return mpButtonData->maImage;
    }
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
    {
        return mpButtonData->maImageHC;
    }
    else
        return Image();
}

// -----------------------------------------------------------------------
BOOL Button::HasImage() const
{
    return !!(mpButtonData->maImage);
}

// -----------------------------------------------------------------------
void Button::SetImageAlign( ImageAlign eAlign )
{
    if ( mpButtonData->meImageAlign != eAlign )
    {
        mpButtonData->meImageAlign = eAlign;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------
ImageAlign Button::GetImageAlign() const
{
    return mpButtonData->meImageAlign;
}

// -----------------------------------------------------------------------
BOOL Button::SetModeBitmap( const BitmapEx& rBitmap, BmpColorMode eMode )
{
    if ( SetModeImage( rBitmap, eMode ) )
    {
        if( eMode == BMP_COLOR_NORMAL )
        {
            if ( !mpButtonData->mpBitmapEx )
                mpButtonData->mpBitmapEx = new BitmapEx( rBitmap );
        }
        else if ( eMode == BMP_COLOR_HIGHCONTRAST )
        {
            if ( !mpButtonData->mpBitmapExHC )
                mpButtonData->mpBitmapExHC = new BitmapEx( rBitmap );
        }
        else
            return FALSE;

        return TRUE;
    }
    return FALSE;
}

// -----------------------------------------------------------------------
BitmapEx Button::GetModeBitmap( BmpColorMode eMode ) const
{
    BitmapEx aBmp;

    if ( eMode == BMP_COLOR_NORMAL )
    {
        if ( mpButtonData->mpBitmapEx )
            aBmp = *( mpButtonData->mpBitmapEx );
    }
    else if ( eMode == BMP_COLOR_HIGHCONTRAST )
    {
        if ( mpButtonData->mpBitmapExHC )
            aBmp = *( mpButtonData->mpBitmapExHC );
    }

    return aBmp;
}

// -----------------------------------------------------------------------
void Button::SetFocusRect( const Rectangle& rFocusRect )
{
    ImplSetFocusRect( rFocusRect );
}

// -----------------------------------------------------------------------
const Rectangle& Button::GetFocusRect() const
{
    return ImplGetFocusRect();
}

// -----------------------------------------------------------------------

USHORT Button::ImplGetTextStyle( XubString& rText, WinBits nWinStyle,
                                 ULONG nDrawFlags )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    USHORT nTextStyle = FixedText::ImplGetTextStyle( nWinStyle & ~WB_DEFBUTTON );

    if ( nDrawFlags & WINDOW_DRAW_NOMNEMONIC )
    {
        if ( nTextStyle & TEXT_DRAW_MNEMONIC )
        {
            rText = GetNonMnemonicString( rText );
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

    return nTextStyle;
}

// -----------------------------------------------------------------------

void Button::ImplDrawAlignedImage( OutputDevice* pDev, Point& rPos,
                                   Size& rSize, BOOL bLayout,
                                   ULONG nImageSep, ULONG nDrawFlags,
                                   USHORT nTextStyle, Rectangle *pSymbolRect )
{
    XubString   aText( GetText() );
    BOOL        bDrawImage = HasImage() && ! ( ImplGetButtonState() & BUTTON_DRAW_NOIMAGE );
    BOOL        bDrawText  = aText.Len() && ! ( ImplGetButtonState() & BUTTON_DRAW_NOTEXT );
    BOOL        bHasSymbol = pSymbolRect ? TRUE : FALSE;

    // No text and no image => nothing to do => return
    if ( !bDrawImage && !bDrawText && !bHasSymbol )
        return;

    WinBits         nWinStyle = GetStyle();
    Rectangle       aOutRect( rPos, rSize );
    MetricVector   *pVector = bLayout ? &mpLayoutData->m_aUnicodeBoundRects : NULL;
    String         *pDisplayText = bLayout ? &mpLayoutData->m_aDisplayText : NULL;
    ImageAlign      eImageAlign = mpButtonData->meImageAlign;
    Size            aImageSize = mpButtonData->maImage.GetSizePixel();

    if ( ( nDrawFlags & WINDOW_DRAW_NOMNEMONIC ) &&
         ( nTextStyle & TEXT_DRAW_MNEMONIC ) )
    {
        aText = GetNonMnemonicString( aText );
        nTextStyle &= ~TEXT_DRAW_MNEMONIC;
    }

    aImageSize.Width()  = CalcZoom( aImageSize.Width() );
    aImageSize.Height() = CalcZoom( aImageSize.Height() );

    // Drawing text or symbol only is simple, use style and output rectangle
    if ( bHasSymbol && !bDrawImage && !bDrawText )
    {
        *pSymbolRect = aOutRect;
        return;
    }
    else if ( bDrawText && !bDrawImage && !bHasSymbol )
    {
        aOutRect = pDev->GetTextRect( aOutRect, aText, nTextStyle );
        rSize = aOutRect.GetSize();
        rPos = aOutRect.TopLeft();

        ImplSetFocusRect( aOutRect );

        pDev->DrawText( aOutRect, aText, nTextStyle, pVector, pDisplayText );
        return;
    }

    // check for HC mode ( image only! )
    Image    *pImage    = &(mpButtonData->maImage);
    BitmapEx *pBitmapEx = mpButtonData->mpBitmapEx;

    Color aBackCol;
    if( !!(mpButtonData->maImageHC) && ImplGetCurrentBackgroundColor( aBackCol ) )
    {
        if( aBackCol.IsDark() )
        {
            pImage = &(mpButtonData->maImageHC);
            pBitmapEx = mpButtonData->mpBitmapExHC;
        }
    }

    if ( pBitmapEx && ( pDev->GetOutDevType() == OUTDEV_PRINTER ) )
    {
        // Die Groesse richtet sich nach dem Bildschirm, soll auf
        // dem Drucker genau so aussehen...
        MapMode aMap100thMM( MAP_100TH_MM );
        aImageSize = PixelToLogic( aImageSize, aMap100thMM );
        aImageSize = pDev->LogicToPixel( aImageSize, aMap100thMM );
    }

    Size aTextSize;
    Size aSymbolSize;
    Size aMax;
    Point aImagePos = rPos;
    Point aTextPos = rPos;
    Rectangle aUnion = Rectangle( aImagePos, aImageSize );
    Rectangle aSymbol;
    long nSymbolHeight = 0;

    if ( bDrawText || bHasSymbol )
    {
        // Get the size of the text output area ( the symbol will be drawn in
        // this area as well, so the symbol rectangle will be calculated here, too )

        Rectangle   aRect = Rectangle( Point(), rSize );
        Size        aTSSize;

        if ( bHasSymbol )
        {
            if ( bDrawText )
            {
                nSymbolHeight = pDev->GetTextHeight();
                if ( mpButtonData->mbSmallSymbol )
                    nSymbolHeight = nSymbolHeight * 3 / 4;

                aSymbol = Rectangle( Point(), Size( nSymbolHeight, nSymbolHeight ) );
                ImplCalcSymbolRect( aSymbol );
                aRect.Left() += 3 * nSymbolHeight / 2;
                aTSSize.Width() = 3 * nSymbolHeight / 2;
            }
            else
            {
                aSymbol = Rectangle( Point(), rSize );
                ImplCalcSymbolRect( aSymbol );
                aTSSize.Width() = aSymbol.GetWidth();
            }
            aTSSize.Height() = aSymbol.GetHeight();
            aSymbolSize = aSymbol.GetSize();
        }

        if ( bDrawText )
        {
            if ( ( eImageAlign == IMAGEALIGN_LEFT_TOP ) ||
                ( eImageAlign == IMAGEALIGN_LEFT ) ||
                ( eImageAlign == IMAGEALIGN_LEFT_BOTTOM ) ||
                ( eImageAlign == IMAGEALIGN_RIGHT_TOP ) ||
                ( eImageAlign == IMAGEALIGN_RIGHT ) ||
                ( eImageAlign == IMAGEALIGN_RIGHT_BOTTOM ) )
            {
                aRect.Right() -= ( aImageSize.Width() + nImageSep );
            }
            else if ( ( eImageAlign == IMAGEALIGN_TOP_LEFT ) ||
                ( eImageAlign == IMAGEALIGN_TOP ) ||
                ( eImageAlign == IMAGEALIGN_TOP_RIGHT ) ||
                ( eImageAlign == IMAGEALIGN_BOTTOM_LEFT ) ||
                ( eImageAlign == IMAGEALIGN_BOTTOM ) ||
                ( eImageAlign == IMAGEALIGN_BOTTOM_RIGHT ) )
            {
                aRect.Bottom() -= ( aImageSize.Height() + nImageSep );
            }

            aRect = pDev->GetTextRect( aRect, aText, nTextStyle );
            aTextSize = aRect.GetSize();

            aTSSize.Width()  += aTextSize.Width();

            if ( aTSSize.Height() < aTextSize.Height() )
                aTSSize.Height() = aTextSize.Height();
        }

        aMax.Width() = aTSSize.Width() > aImageSize.Width() ? aTSSize.Width() : aImageSize.Width();
        aMax.Height() = aTSSize.Height() > aImageSize.Height() ? aTSSize.Height() : aImageSize.Height();

        // Now calculate the output area for the image and the text acording to the image align flags

        if ( ( eImageAlign == IMAGEALIGN_LEFT ) ||
             ( eImageAlign == IMAGEALIGN_RIGHT ) )
        {
            aImagePos.Y() = rPos.Y() + ( aMax.Height() - aImageSize.Height() ) / 2;
            aTextPos.Y()  = rPos.Y() + ( aMax.Height() - aTSSize.Height() ) / 2;
        }
        else if ( ( eImageAlign == IMAGEALIGN_LEFT_BOTTOM ) ||
                  ( eImageAlign == IMAGEALIGN_RIGHT_BOTTOM ) )
        {
            aImagePos.Y() = rPos.Y() + aMax.Height() - aImageSize.Height();
            aTextPos.Y()  = rPos.Y() + aMax.Height() - aTSSize.Height();
        }
        else if ( ( eImageAlign == IMAGEALIGN_TOP ) ||
                  ( eImageAlign == IMAGEALIGN_BOTTOM ) )
        {
            aImagePos.X() = rPos.X() + ( aMax.Width() - aImageSize.Width() ) / 2;
            aTextPos.X()  = rPos.X() + ( aMax.Width() - aTSSize.Width() ) / 2;
        }
        else if ( ( eImageAlign == IMAGEALIGN_TOP_RIGHT ) ||
                  ( eImageAlign == IMAGEALIGN_BOTTOM_RIGHT ) )
        {
            aImagePos.X() = rPos.X() + aMax.Width() - aImageSize.Width();
            aTextPos.X()  = rPos.X() + aMax.Width() - aTSSize.Width();
        }

        if ( ( eImageAlign == IMAGEALIGN_LEFT_TOP ) ||
             ( eImageAlign == IMAGEALIGN_LEFT ) ||
             ( eImageAlign == IMAGEALIGN_LEFT_BOTTOM ) )
        {
            aTextPos.X() = rPos.X() + aImageSize.Width() + nImageSep;
        }
        else if ( ( eImageAlign == IMAGEALIGN_RIGHT_TOP ) ||
                  ( eImageAlign == IMAGEALIGN_RIGHT ) ||
                  ( eImageAlign == IMAGEALIGN_RIGHT_BOTTOM ) )
        {
            aImagePos.X() = rPos.X() + aTSSize.Width() + nImageSep;
        }
        else if ( ( eImageAlign == IMAGEALIGN_TOP_LEFT ) ||
                  ( eImageAlign == IMAGEALIGN_TOP ) ||
                  ( eImageAlign == IMAGEALIGN_TOP_RIGHT ) )
        {
            aTextPos.Y() = rPos.Y() + aImageSize.Height() + nImageSep;
        }
        else if ( ( eImageAlign == IMAGEALIGN_BOTTOM_LEFT ) ||
                  ( eImageAlign == IMAGEALIGN_BOTTOM ) ||
                  ( eImageAlign == IMAGEALIGN_BOTTOM_RIGHT ) )
        {
            aImagePos.Y() = rPos.Y() + aTSSize.Height() + nImageSep;
        }
        else if ( eImageAlign == IMAGEALIGN_CENTER )
        {
            aImagePos.X() = rPos.X() + ( aMax.Width()  - aImageSize.Width() ) / 2;
            aImagePos.Y() = rPos.Y() + ( aMax.Height() - aImageSize.Height() ) / 2;
            aTextPos.X()  = rPos.X() + ( aMax.Width()  - aTSSize.Width() ) / 2;
            aTextPos.Y()  = rPos.Y() + ( aMax.Height() - aTSSize.Height() ) / 2;
        }
        aUnion = Rectangle( aImagePos, aImageSize );
        aUnion.Union( Rectangle( aTextPos, aTSSize ) );
    }

    // Now place the combination of text and image in the output area of the button
    // according to the window style (WinBits)
    long nXOffset = 0;
    long nYOffset = 0;

    if ( nWinStyle & WB_CENTER )
    {
        nXOffset = ( rSize.Width() - aUnion.GetWidth() ) / 2;
    }
    else if ( nWinStyle & WB_RIGHT )
    {
        nXOffset = rSize.Width() - aUnion.GetWidth();
    }

    if ( nWinStyle & WB_VCENTER )
    {
        nYOffset = ( rSize.Height() - aUnion.GetHeight() ) / 2;
    }
    else if ( nWinStyle & WB_BOTTOM )
    {
        nYOffset = rSize.Height() - aUnion.GetHeight();
    }

    // the top left corner should always be visible, so we don't allow negative offsets
    if ( nXOffset < 0 ) nXOffset = 0;
    if ( nYOffset < 0 ) nYOffset = 0;

    aImagePos.X() += nXOffset;
    aImagePos.Y() += nYOffset;
    aTextPos.X() += nXOffset;
    aTextPos.Y() += nYOffset;

    // set rPos and rSize to the union
    rSize = aUnion.GetSize();
    rPos.X() += nXOffset;
    rPos.Y() += nYOffset;

    if ( bHasSymbol )
    {
        if ( mpButtonData->meSymbolAlign == SYMBOLALIGN_RIGHT )
        {
            Point aRightPos = Point( aTextPos.X() + aTextSize.Width() + aSymbolSize.Width()/2, aTextPos.Y() );
            *pSymbolRect = Rectangle( aRightPos, aSymbolSize );
        }
        else
        {
            *pSymbolRect = Rectangle( aTextPos, aSymbolSize );
            aTextPos.X() += ( 3 * nSymbolHeight / 2 );
        }
        if ( mpButtonData->mbSmallSymbol )
        {
            nYOffset = (aUnion.GetHeight() - aSymbolSize.Height())/2;
            pSymbolRect->setY( aTextPos.Y() + nYOffset );
        }
    }

    USHORT nStyle = 0;

    if ( ! ( nDrawFlags & WINDOW_DRAW_NODISABLE ) &&
         ! IsEnabled() )
        nStyle |= IMAGE_DRAW_DISABLE;

    if ( pBitmapEx && ( pDev->GetOutDevType() == OUTDEV_PRINTER ) )
    {
        // Fuer die BitmapEx ueberlegt sich KA noch, wie man die disablete
        // Darstellung hinbekommt...
        pBitmapEx->Draw( pDev, aImagePos, aImageSize /*, nStyle*/ );
    }
    else
    {
        if ( IsZoom() )
            pDev->DrawImage( aImagePos, aImageSize, *pImage, nStyle );
        else
            pDev->DrawImage( aImagePos, *pImage, nStyle );
    }

    if ( bDrawText )
    {
        ImplSetFocusRect( Rectangle( aTextPos, aTextSize ) );
        pDev->DrawText( Rectangle( aTextPos, aTextSize ), aText, nTextStyle, pVector, pDisplayText );
    }
    else
    {
        ImplSetFocusRect( Rectangle( aImagePos, aImageSize ) );
    }
}

// -----------------------------------------------------------------------
void Button::ImplSetFocusRect( const Rectangle &rFocusRect )
{
    Rectangle aFocusRect = rFocusRect;
    Rectangle aOutputRect = Rectangle( Point(), GetOutputSizePixel() );

    if ( ! aFocusRect.IsEmpty() )
    {
        aFocusRect.Left()--;
        aFocusRect.Top()--;
        aFocusRect.Right()++;
        aFocusRect.Bottom()++;
    }

    if ( aFocusRect.Left() < aOutputRect.Left() )   aFocusRect.Left() = aOutputRect.Left();
    if ( aFocusRect.Top() < aOutputRect.Top() )     aFocusRect.Top() = aOutputRect.Top();
    if ( aFocusRect.Right() > aOutputRect.Right() ) aFocusRect.Right() = aOutputRect.Right();
    if ( aFocusRect.Bottom() > aOutputRect.Bottom() ) aFocusRect.Bottom() = aOutputRect.Bottom();

    mpButtonData->maFocusRect = aFocusRect;
}

// -----------------------------------------------------------------------
const Rectangle& Button::ImplGetFocusRect() const
{
    return mpButtonData->maFocusRect;
}

// -----------------------------------------------------------------------
USHORT& Button::ImplGetButtonState()
{
    return mpButtonData->mnButtonState;
}

// -----------------------------------------------------------------------
USHORT Button::ImplGetButtonState() const
{
    return mpButtonData->mnButtonState;
}

// -----------------------------------------------------------------------
void Button::ImplSetSymbolAlign( SymbolAlign eAlign )
{
    if ( mpButtonData->meSymbolAlign != eAlign )
    {
        mpButtonData->meSymbolAlign = eAlign;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------
SymbolAlign Button::ImplGetSymbolAlign() const
{
    return mpButtonData->meSymbolAlign;
}
// -----------------------------------------------------------------------
void Button::ImplSetSmallSymbol( BOOL bSmall )
{
    mpButtonData->mbSmallSymbol = bSmall;
}

// -----------------------------------------------------------------------
void Button::EnableImageDisplay( BOOL bEnable )
{
    if( bEnable )
        mpButtonData->mnButtonState &= ~BUTTON_DRAW_NOIMAGE;
    else
        mpButtonData->mnButtonState |= BUTTON_DRAW_NOIMAGE;
}

// -----------------------------------------------------------------------
BOOL Button::IsImageDisplayEnabled()
{
    return (mpButtonData->mnButtonState & BUTTON_DRAW_NOIMAGE) == 0;
}

// -----------------------------------------------------------------------
void Button::EnableTextDisplay( BOOL bEnable )
{
    if( bEnable )
        mpButtonData->mnButtonState &= ~BUTTON_DRAW_NOTEXT;
    else
        mpButtonData->mnButtonState |= BUTTON_DRAW_NOTEXT;
}

// -----------------------------------------------------------------------
BOOL Button::IsTextDisplayEnabled()
{
    return (mpButtonData->mnButtonState & BUTTON_DRAW_NOTEXT) == 0;
}

// -----------------------------------------------------------------------
void Button::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    // The flag SETTINGS_IN_UPDATE_SETTINGS is set when the settings changed due to a
    // Application::SettingsChanged event. In this scenario we want to keep the style settings
    // of our radio buttons and our check boxes.
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) &&
         ( rDCEvt.GetFlags() & SETTINGS_IN_UPDATE_SETTINGS ) )

    {
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if ( pOldSettings )
        {
            BOOL bResetStyleSettings = FALSE;
            AllSettings aAllSettings = GetSettings();
            StyleSettings aStyleSetting = aAllSettings.GetStyleSettings();

            USHORT nCheckBoxStyle = aStyleSetting.GetCheckBoxStyle();
            if ( nCheckBoxStyle != pOldSettings->GetStyleSettings().GetCheckBoxStyle() )
            {
                aStyleSetting.SetCheckBoxStyle( pOldSettings->GetStyleSettings().GetCheckBoxStyle() );
                bResetStyleSettings = TRUE;
            }

            USHORT nRadioButtonStyle = aStyleSetting.GetRadioButtonStyle();
            if ( nRadioButtonStyle != pOldSettings->GetStyleSettings().GetRadioButtonStyle() )
            {
                aStyleSetting.SetRadioButtonStyle( pOldSettings->GetStyleSettings().GetRadioButtonStyle() );
                bResetStyleSettings = TRUE;
            }

            if ( bResetStyleSettings )
            {
                aAllSettings.SetStyleSettings( pOldSettings->GetStyleSettings() );
                SetSettings( aAllSettings );
            }
        }
    }
}


// =======================================================================

void PushButton::ImplInitPushButtonData()
{
    mpWindowImpl->mbPushButton    = TRUE;

    meSymbol        = SYMBOL_NOSYMBOL;
    meState         = STATE_NOCHECK;
    meSaveValue     = STATE_NOCHECK;
    mnDDStyle       = 0;
    mbPressed       = FALSE;
    mbInUserDraw    = FALSE;
}

// -----------------------------------------------------------------------

void PushButton::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( pParent->GetWindow( WINDOW_LASTCHILD ), nStyle );
    Button::ImplInit( pParent, nStyle, NULL );

    if ( nStyle & WB_NOLIGHTBORDER )
        ImplGetButtonState() |= BUTTON_DRAW_NOLIGHTBORDER;

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits PushButton::ImplInitStyle( const Window* pPrevWindow, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    // if no alignment is given, default to "vertically centered". This is because since
    // #i26046#, we respect the vertical alignment flags (previously we didn't completely),
    // but we of course want to look as before when no vertical alignment is specified
    if ( ( nStyle & ( WB_TOP | WB_VCENTER | WB_BOTTOM ) ) == 0 )
        nStyle |= WB_VCENTER;

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
    {
        SetBackground();
        // #i38498#: do not check for GetParent()->IsChildTransparentModeEnabled()
        // otherwise the formcontrol button will be overdrawn due to PARENTCLIPMODE_NOCLIP
        // for radio and checkbox this is ok as they shoud appear transparent in documents
        if ( IsNativeControlSupported( CTRL_PUSHBUTTON, PART_ENTIRE_CONTROL ) )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            mpWindowImpl->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
        }
        else
        {
            EnableChildTransparentMode( FALSE );
            SetParentClipMode( 0 );
            SetPaintTransparent( FALSE );
        }
    }
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
        pDev->OutputDevice::SetSettings( aSettings );
        rRect = aDecoView.DrawButton( rRect, nStyle );
        pDev->OutputDevice::SetSettings( aOldSettings );
    }
    else
        rRect = aDecoView.DrawButton( rRect, nStyle );
}

// -----------------------------------------------------------------------

BOOL PushButton::ImplHitTestPushButton( Window* pDev,
                                        const Point& rPos )
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

USHORT PushButton::ImplGetTextStyle( ULONG nDrawFlags ) const
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    USHORT nTextStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_MULTILINE | TEXT_DRAW_ENDELLIPSIS;

    if ( ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO ) ||
         ( nDrawFlags & WINDOW_DRAW_MONO ) )
        nTextStyle |= TEXT_DRAW_MONO;

    if ( GetStyle() & WB_WORDBREAK )
        nTextStyle |= TEXT_DRAW_WORDBREAK;
    if ( GetStyle() & WB_NOLABEL )
        nTextStyle &= ~TEXT_DRAW_MNEMONIC;

    if ( GetStyle() & WB_LEFT )
        nTextStyle |= TEXT_DRAW_LEFT;
    else if ( GetStyle() & WB_RIGHT )
        nTextStyle |= TEXT_DRAW_RIGHT;
    else
        nTextStyle |= TEXT_DRAW_CENTER;

    if ( GetStyle() & WB_TOP )
        nTextStyle |= TEXT_DRAW_TOP;
    else if ( GetStyle() & WB_BOTTOM )
        nTextStyle |= TEXT_DRAW_BOTTOM;
    else
        nTextStyle |= TEXT_DRAW_VCENTER;

    if ( ! ( (nDrawFlags & WINDOW_DRAW_NODISABLE) || IsEnabled() ) )
        nTextStyle |= TEXT_DRAW_DISABLE;

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
                                            const Rectangle& rRect,
                                            bool bLayout )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle               aInRect = rRect;
    Color                   aColor;
    XubString               aText = PushButton::GetText(); // PushButton:: wegen MoreButton
    USHORT                  nTextStyle = ImplGetTextStyle( nDrawFlags );
    USHORT                  nStyle;

    if( aInRect.nRight < aInRect.nLeft || aInRect.nBottom < aInRect.nTop )
        aInRect.SetEmpty();

    pDev->Push( PUSH_CLIPREGION );
    pDev->IntersectClipRegion( aInRect );

    if ( nDrawFlags & WINDOW_DRAW_MONO )
        aColor = Color( COL_BLACK );
    else if ( IsControlForeground() )
        aColor = GetControlForeground();
    else if( nDrawFlags & WINDOW_DRAW_ROLLOVER )
        aColor = rStyleSettings.GetButtonRolloverTextColor();
    else
        aColor = rStyleSettings.GetButtonTextColor();

    pDev->SetTextColor( aColor );

    if ( IsEnabled() || (nDrawFlags & WINDOW_DRAW_NODISABLE) )
        nStyle = 0;
    else
        nStyle = SYMBOL_DRAW_DISABLE;

    Size aSize = rRect.GetSize();
    Point aPos = rRect.TopLeft();

    if ( mnDDStyle == PUSHBUTTON_DROPDOWN_MENUBUTTON )
    {
        if ( aText.Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
        {
            // calc Symbol- and Textrect
            long nSymbolSize    = pDev->GetTextHeight() / 2 + 1;
            aInRect.Right()    -= 5;
            aInRect.Left()      = aInRect.Right() - nSymbolSize;
            aSize.Width()      -= ( 5 + nSymbolSize );

            ImplDrawAlignedImage( pDev, aPos, aSize, bLayout,
                                  1, nDrawFlags, nTextStyle );
        }
        else
            ImplCalcSymbolRect( aInRect );

        if( ! bLayout )
        {
            DecorationView aDecoView( pDev );
            aDecoView.DrawSymbol( aInRect, SYMBOL_SPIN_DOWN, aColor, nStyle );
        }
    }
    else
    {
        Rectangle aSymbolRect;
        ImplDrawAlignedImage( pDev, aPos, aSize, bLayout, 1, nDrawFlags,
                              nTextStyle, IsSymbol() ? &aSymbolRect : NULL );

        if ( IsSymbol() && ! bLayout )
        {
            DecorationView aDecoView( pDev );
            aDecoView.DrawSymbol( aSymbolRect, meSymbol, aColor, nStyle );
        }

        if ( mnDDStyle == PUSHBUTTON_DROPDOWN_TOOLBOX && !bLayout )
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

    pDev->Pop();  // restore clipregion
}

// -----------------------------------------------------------------------

void PushButton::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void PushButton::ImplDrawPushButton( bool bLayout )
{
    if( !bLayout )
        HideFocus();

    USHORT                  nButtonStyle = ImplGetButtonState();
    Point                   aPoint;
    Size                    aOutSz( GetOutputSizePixel() );
    Rectangle               aRect( aPoint, aOutSz );
    Rectangle               aInRect = aRect;
    Rectangle               aTextRect;
    BOOL                    bNativeOK = FALSE;

    // adjust style if button should be rendered 'pressed'
    if ( mbPressed )
        nButtonStyle |= BUTTON_DRAW_PRESSED;

    // TODO: move this to Window class or make it a member !!!
    ControlType aCtrlType = 0;
    switch( GetParent()->GetType() )
    {
        case WINDOW_LISTBOX:
        case WINDOW_MULTILISTBOX:
        case WINDOW_TREELISTBOX:
            aCtrlType = CTRL_LISTBOX;
            break;

        case WINDOW_COMBOBOX:
        case WINDOW_PATTERNBOX:
        case WINDOW_NUMERICBOX:
        case WINDOW_METRICBOX:
        case WINDOW_CURRENCYBOX:
        case WINDOW_DATEBOX:
        case WINDOW_TIMEBOX:
        case WINDOW_LONGCURRENCYBOX:
            aCtrlType = CTRL_COMBOBOX;
            break;
        default:
            break;
    }

    BOOL bDropDown = ( IsSymbol() && (GetSymbol()==SYMBOL_SPIN_DOWN) && !GetText().Len() );

    if( bDropDown && (aCtrlType == CTRL_COMBOBOX || aCtrlType == CTRL_LISTBOX ) )
    {
        if( GetParent()->IsNativeControlSupported( aCtrlType, PART_ENTIRE_CONTROL) )
        {
            // skip painting if the button was already drawn by the theme
            if( aCtrlType == CTRL_COMBOBOX )
            {
                Edit* pEdit = static_cast<Edit*>(GetParent());
                if( pEdit->ImplUseNativeBorder( pEdit->GetStyle() ) )
                    bNativeOK = TRUE;
            }
            else if( GetParent()->IsNativeControlSupported( aCtrlType, HAS_BACKGROUND_TEXTURE) )
            {
                bNativeOK = TRUE;
            }
            if( !bNativeOK && GetParent()->IsNativeControlSupported( aCtrlType, PART_BUTTON_DOWN ) )
            {
                // let the theme draw it, note we then need support
                // for CTRL_LISTBOX/PART_BUTTON_DOWN and CTRL_COMBOBOX/PART_BUTTON_DOWN

                ImplControlValue    aControlValue;
                Region              aCtrlRegion( aInRect );
                ControlState        nState = 0;

                if ( mbPressed )                        nState |= CTRL_STATE_PRESSED;
                if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )   nState |= CTRL_STATE_PRESSED;
                if ( HasFocus() )                       nState |= CTRL_STATE_FOCUSED;
                if ( ImplGetButtonState() & BUTTON_DRAW_DEFAULT )   nState |= CTRL_STATE_DEFAULT;
                if ( Window::IsEnabled() )              nState |= CTRL_STATE_ENABLED;

                if ( IsMouseOver() && aInRect.IsInside( GetPointerPosPixel() ) )
                    nState |= CTRL_STATE_ROLLOVER;

                bNativeOK = DrawNativeControl( aCtrlType, PART_BUTTON_DOWN, aCtrlRegion, nState,
                                                aControlValue, rtl::OUString() );
            }
        }
    }

    if( bNativeOK )
        return;

    if ( (bNativeOK=IsNativeControlSupported(CTRL_PUSHBUTTON, PART_ENTIRE_CONTROL)) == TRUE )
    {
        PushButtonValue aPBVal;
        ImplControlValue aControlValue;
        aControlValue.setOptionalVal( &aPBVal );
        Region           aCtrlRegion( aInRect );
        ControlState     nState = 0;

        if ( mbPressed || IsChecked() )                   nState |= CTRL_STATE_PRESSED;
        if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED ) nState |= CTRL_STATE_PRESSED;
        if ( HasFocus() )                       nState |= CTRL_STATE_FOCUSED;
        if ( ImplGetButtonState() & BUTTON_DRAW_DEFAULT )   nState |= CTRL_STATE_DEFAULT;
        if ( Window::IsEnabled() )              nState |= CTRL_STATE_ENABLED;

        if ( IsMouseOver() && aInRect.IsInside( GetPointerPosPixel() ) )
            nState |= CTRL_STATE_ROLLOVER;

        if( GetStyle() & WB_BEVELBUTTON )
            aPBVal.mbBevelButton = true;

        bNativeOK = DrawNativeControl( CTRL_PUSHBUTTON, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                         aControlValue, rtl::OUString()/*PushButton::GetText()*/ );

        // draw frame into invisible window to have aInRect modified correctly
        // but do not shift the inner rect for pressed buttons (ie remove BUTTON_DRAW_PRESSED)
        // this assumes the theme has enough visual cues to signalize the button was pressed
        //Window aWin( this );
        //ImplDrawPushButtonFrame( &aWin, aInRect, nButtonStyle & ~BUTTON_DRAW_PRESSED );

        // looks better this way as symbols were displaced slightly using the above approach
        aInRect.Top()+=4;
        aInRect.Bottom()-=4;
        aInRect.Left()+=4;
        aInRect.Right()-=4;

        // draw content using the same aInRect as non-native VCL would do
        ImplDrawPushButtonContent( this,
                                   (nState&CTRL_STATE_ROLLOVER) ? WINDOW_DRAW_ROLLOVER : 0,
                                   aInRect, bLayout );

        if ( HasFocus() )
            ShowFocus( ImplGetFocusRect() );
    }

    if ( bNativeOK == FALSE )
    {
        // draw PushButtonFrame, aInRect has content size afterwards
        if( ! bLayout )
            ImplDrawPushButtonFrame( this, aInRect, nButtonStyle );

        // draw content
        ImplDrawPushButtonContent( this, 0, aInRect, bLayout );

        if( ! bLayout && HasFocus() )
        {
            ShowFocus( ImplGetFocusRect() );
        }
    }
}

// -----------------------------------------------------------------------

void PushButton::ImplSetDefButton( BOOL bSet )
{
    Size aSize( GetSizePixel() );
    Point aPos( GetPosPixel() );
    int dLeft(0), dRight(0), dTop(0), dBottom(0);
    BOOL bSetPos = FALSE;

    if ( (IsNativeControlSupported(CTRL_PUSHBUTTON, PART_ENTIRE_CONTROL)) == TRUE )
    {
        Region aBoundingRgn, aContentRgn;
        Rectangle aCtrlRect( 0, 0, 80, 20 ); // use a constant size to avoid accumulating
                                             // will not work if the theme has dynamic adornment sizes
        ImplControlValue aControlValue;
        Region           aCtrlRegion( aCtrlRect );
        ControlState     nState = CTRL_STATE_DEFAULT|CTRL_STATE_ENABLED;

        // get native size of a 'default' button
        // and adjust the VCL button if more space for adornment is required
        if( GetNativeControlRegion( CTRL_PUSHBUTTON, PART_ENTIRE_CONTROL, aCtrlRegion,
                                nState, aControlValue, rtl::OUString(),
                                aBoundingRgn, aContentRgn ) )
        {
            Rectangle aCont(aContentRgn.GetBoundRect());
            Rectangle aBound(aBoundingRgn.GetBoundRect());

            dLeft = aCont.Left() - aBound.Left();
            dTop = aCont.Top() - aBound.Top();
            dRight = aBound.Right() - aCont.Right();
            dBottom = aBound.Bottom() - aCont.Bottom();
            bSetPos = dLeft || dTop || dRight || dBottom;
        }
    }

    if ( bSet )
    {
        if( !(ImplGetButtonState() & BUTTON_DRAW_DEFAULT) && bSetPos )
        {
            // adjust pos/size when toggling from non-default to default
            aPos.Move(-dLeft, -dTop);
            aSize.Width() += dLeft + dRight;
            aSize.Height() += dTop + dBottom;
        }
        ImplGetButtonState() |= BUTTON_DRAW_DEFAULT;
    }
    else
    {
        if( (ImplGetButtonState() & BUTTON_DRAW_DEFAULT) && bSetPos )
        {
            // adjust pos/size when toggling from default to non-default
            aPos.Move(dLeft, dTop);
            aSize.Width() -= dLeft + dRight;
            aSize.Height() -= dTop + dBottom;
        }
        ImplGetButtonState() &= ~BUTTON_DRAW_DEFAULT;
    }
    if( bSetPos )
        SetPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height(), WINDOW_POSSIZE_ALL );

    Invalidate();
}

// -----------------------------------------------------------------------

BOOL PushButton::ImplIsDefButton() const
{
    return (ImplGetButtonState() & BUTTON_DRAW_DEFAULT) != 0;
}

// -----------------------------------------------------------------------

PushButton::PushButton( WindowType nType ) :
    Button( nType )
{
    ImplInitPushButtonData();
}

// -----------------------------------------------------------------------

PushButton::PushButton( Window* pParent, WinBits nStyle ) :
    Button( WINDOW_PUSHBUTTON )
{
    ImplInitPushButtonData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

PushButton::PushButton( Window* pParent, const ResId& rResId ) :
    Button( WINDOW_PUSHBUTTON )
{
    ImplInitPushButtonData();
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
}

// -----------------------------------------------------------------------

void PushButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() &&
         ImplHitTestPushButton( this, rMEvt.GetPosPixel() ) )
    {
        USHORT nTrackFlags = 0;

        if ( ( GetStyle() & WB_REPEAT ) &&
             ! ( GetStyle() & WB_TOGGLE ) )
            nTrackFlags |= STARTTRACK_BUTTONREPEAT;

        ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
        ImplDrawPushButton();
        StartTracking( nTrackFlags );

        if ( nTrackFlags & STARTTRACK_BUTTONREPEAT )
            Click();
    }
}

// -----------------------------------------------------------------------

void PushButton::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            if ( GetStyle() & WB_TOGGLE )
            {
                // Don't toggle, when aborted
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( IsChecked() )
                    {
                        Check( FALSE );
                        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
                    }
                    else
                        Check( TRUE );
                }
            }
            else
                ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;

            ImplDrawPushButton();

            // Bei Abbruch kein Click-Handler rufen
            if ( !rTEvt.IsTrackingCanceled() )
            {
                if ( ! ( ( GetStyle() & WB_REPEAT ) &&
                         ! ( GetStyle() & WB_TOGGLE ) ) )
                    Click();
            }
        }
    }
    else
    {
        if ( ImplHitTestPushButton( this, rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
            {
                if ( rTEvt.IsTrackingRepeat() && (GetStyle() & WB_REPEAT) &&
                     ! ( GetStyle() & WB_TOGGLE ) )
                    Click();
            }
            else
            {
                ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
                ImplDrawPushButton();
            }
        }
        else
        {
            if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
            {
                ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
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
        if ( !(ImplGetButtonState() & BUTTON_DRAW_PRESSED) )
        {
            ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
            ImplDrawPushButton();
        }

        if ( ( GetStyle() & WB_REPEAT ) &&
             ! ( GetStyle() & WB_TOGGLE ) )
            Click();
    }
    else if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplDrawPushButton();
    }
    else
        Button::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void PushButton::KeyUp( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) &&
         ((aKeyCode.GetCode() == KEY_RETURN) || (aKeyCode.GetCode() == KEY_SPACE)) )
    {
        if ( GetStyle() & WB_TOGGLE )
        {
            if ( IsChecked() )
            {
                Check( FALSE );
                ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
            }
            else
                Check( TRUE );

            Toggle();
        }
        else
            ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;

        ImplDrawPushButton();

        if ( !( ( GetStyle() & WB_REPEAT )  &&
                ! ( GetStyle() & WB_TOGGLE ) ) )
            Click();
    }
    else
        Button::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void PushButton::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
    const_cast<PushButton*>(this)->ImplDrawPushButton( true );
}

// -----------------------------------------------------------------------

void PushButton::Paint( const Rectangle& )
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
    {
        pDev->SetTextColor( Color( COL_BLACK ) );
    }
    else
    {
        pDev->SetTextColor( GetTextColor() );

        // DecoView uses the FaceColor...
        AllSettings aSettings = pDev->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        if ( IsControlBackground() )
            aStyleSettings.SetFaceColor( GetControlBackground() );
        else
            aStyleSettings.SetFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );
        aSettings.SetStyleSettings( aStyleSettings );
        pDev->SetSettings( aSettings );
    }
    pDev->SetTextFillColor();

    DecorationView aDecoView( pDev );
    USHORT nButtonStyle = 0;
    if ( nFlags & WINDOW_DRAW_MONO )
        nButtonStyle |= BUTTON_DRAW_MONO;
    if ( IsChecked() )
        nButtonStyle |= BUTTON_DRAW_CHECKED;
    aRect = aDecoView.DrawButton( aRect, nButtonStyle );

    ImplDrawPushButtonContent( pDev, nFlags, aRect, false );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void PushButton::Resize()
{
    Control::Resize();
    Invalidate();
}

// -----------------------------------------------------------------------

void PushButton::GetFocus()
{
    ShowFocus( ImplGetFocusRect() );
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

long PushButton::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() )
        {
            // trigger redraw as mouse over state has changed

            // TODO: move this to Window class or make it a member !!!
            ControlType aCtrlType = 0;
            switch( GetParent()->GetType() )
            {
                case WINDOW_LISTBOX:
                case WINDOW_MULTILISTBOX:
                case WINDOW_TREELISTBOX:
                    aCtrlType = CTRL_LISTBOX;
                    break;

                case WINDOW_COMBOBOX:
                case WINDOW_PATTERNBOX:
                case WINDOW_NUMERICBOX:
                case WINDOW_METRICBOX:
                case WINDOW_CURRENCYBOX:
                case WINDOW_DATEBOX:
                case WINDOW_TIMEBOX:
                case WINDOW_LONGCURRENCYBOX:
                    aCtrlType = CTRL_COMBOBOX;
                    break;
                default:
                    break;
            }

            BOOL bDropDown = ( IsSymbol() && (GetSymbol()==SYMBOL_SPIN_DOWN) && !GetText().Len() );

            if( bDropDown && GetParent()->IsNativeControlSupported( aCtrlType, PART_ENTIRE_CONTROL) &&
                   !GetParent()->IsNativeControlSupported( aCtrlType, PART_BUTTON_DOWN) )
            {
                Window *pBorder = GetParent()->GetWindow( WINDOW_BORDER );
                if(aCtrlType == CTRL_COMBOBOX)
                {
                    // only paint the button part to avoid flickering of the combobox text
                    Point aPt;
                    Rectangle aClipRect( aPt, GetOutputSizePixel() );
                    aClipRect.SetPos(pBorder->ScreenToOutputPixel(OutputToScreenPixel(aClipRect.TopLeft())));
                    pBorder->Invalidate( aClipRect );
                }
                else
                {
                    pBorder->Invalidate( INVALIDATE_NOERASE );
                    pBorder->Update();
                }
            }
            else if( IsNativeControlSupported(CTRL_PUSHBUTTON, PART_ENTIRE_CONTROL) )
            {
                Invalidate();
            }
        }
    }

    return nDone ? nDone : Button::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void PushButton::Toggle()
{
    ImplCallEventListenersAndHandler( VCLEVENT_PUSHBUTTON_TOGGLE, maToggleHdl, this );
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
void PushButton::SetSymbolAlign( SymbolAlign eAlign )
{
    ImplSetSymbolAlign( eAlign );
}

// -----------------------------------------------------------------------
SymbolAlign PushButton::GetSymbolAlign() const
{
    return ImplGetSymbolAlign();
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
            ImplGetButtonState() &= ~(BUTTON_DRAW_CHECKED | BUTTON_DRAW_DONTKNOW);
        else if ( meState == STATE_CHECK )
        {
            ImplGetButtonState() &= ~BUTTON_DRAW_DONTKNOW;
            ImplGetButtonState() |= BUTTON_DRAW_CHECKED;
        }
        else // STATE_DONTKNOW
        {
            ImplGetButtonState() &= ~BUTTON_DRAW_CHECKED;
            ImplGetButtonState() |= BUTTON_DRAW_DONTKNOW;
        }

        StateChanged( STATE_CHANGE_STATE );
        Toggle();
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
    if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
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
    else if ( IsImage() && ! (ImplGetButtonState() & BUTTON_DRAW_NOIMAGE) )
        aSize = GetModeImage().GetSizePixel();
    if ( PushButton::GetText().Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
    {
        ULONG nDrawFlags = 0;
        Size textSize = GetTextRect( Rectangle( Point(), Size( nMaxWidth ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                                     PushButton::GetText(), ImplGetTextStyle( nDrawFlags ) ).GetSize();
       aSize.Width() += int( textSize.Width () * 1.15 );
       aSize.Height() = std::max( aSize.Height(), long( textSize.Height() * 1.15 ) );
    }

    // cf. ImplDrawPushButton ...
    aSize.Width() += 8;
    aSize.Height() += 8;

    return CalcWindowSize( aSize );
}

Size PushButton::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM: {
        return CalcMinimumSize();
    }
    default:
        return Button::GetOptimalSize( eType );
    }
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
                if ( ((Dialog*)pParent)->IsInExecute() )
                    ((Dialog*)pParent)->EndDialog( TRUE );
                // gegen rekursive Aufrufe schuetzen
                else if ( !((Dialog*)pParent)->IsInClose() )
                {
                    if ( pParent->GetStyle() & WB_CLOSEABLE )
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
    {
        PushButton::Click();
    }
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
                if ( ((Dialog*)pParent)->IsInExecute() )
                    ((Dialog*)pParent)->EndDialog( FALSE );
                // gegen rekursive Aufrufe schuetzen
                else if ( !((Dialog*)pParent)->IsInClose() )
                {
                    if ( pParent->GetStyle() & WB_CLOSEABLE )
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
    {
        PushButton::Click();
    }
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
    PushButton::Click();
}

// =======================================================================

void RadioButton::ImplInitRadioButtonData()
{
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
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled() || IsNativeControlSupported( CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL ) ) )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
            if( IsNativeControlSupported( CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL ) )
                mpWindowImpl->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
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

//---------------------------------------------------------------------
//--- 12.03.2003 18:46:14 ---------------------------------------------

void RadioButton::DrawRadioButtonState( )
{
    ImplDrawRadioButtonState( );
}

// -----------------------------------------------------------------------

void RadioButton::ImplInvalidateOrDrawRadioButtonState()
{
    if( ImplGetSVData()->maNWFData.mbCheckBoxNeedsErase )
    {
        if ( IsNativeControlSupported(CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL) )
        {
            Invalidate();
            Update();
            return;
        }
    }
    ImplDrawRadioButtonState();
}

void RadioButton::ImplDrawRadioButtonState()
{
    USHORT nButtonStyle = 0;
    BOOL   bNativeOK = FALSE;

    // no native drawing for image radio buttons
    if ( !maImage && (bNativeOK=IsNativeControlSupported(CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL)) == TRUE )
    {
        ImplControlValue            aControlValue( mbChecked ? BUTTONVALUE_ON : BUTTONVALUE_OFF, rtl::OUString(), 0 );
        Rectangle                   aCtrlRect( maStateRect.TopLeft(), maStateRect.GetSize() );
        Region                      aCtrlRegion( aCtrlRect );
        ControlState                nState = 0;

        if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )   nState |= CTRL_STATE_PRESSED;
        if ( HasFocus() )                       nState |= CTRL_STATE_FOCUSED;
        if ( ImplGetButtonState() & BUTTON_DRAW_DEFAULT )   nState |= CTRL_STATE_DEFAULT;
        if ( IsEnabled() )                      nState |= CTRL_STATE_ENABLED;

        if ( IsMouseOver() && maMouseRect.IsInside( GetPointerPosPixel() ) )
            nState |= CTRL_STATE_ROLLOVER;

        bNativeOK = DrawNativeControl( CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                    aControlValue,rtl::OUString() );

    }

if ( bNativeOK == FALSE )
{
    // kein Image-RadioButton
    if ( !maImage )
    {
        USHORT nStyle = ImplGetButtonState();
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
        nButtonStyle = FRAME_DRAW_DOUBLEIN;
        aImageRect = aDecoView.DrawFrame( aImageRect, nButtonStyle );
        if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) || !bEnabled )
            SetFillColor( rStyleSettings.GetFaceColor() );
        else
            SetFillColor( rStyleSettings.GetFieldColor() );
        SetLineColor();
        DrawRect( aImageRect );

        // Image ausgeben
        nButtonStyle = 0;
        if ( !bEnabled )
            nButtonStyle |= IMAGE_DRAW_DISABLE;

        // check for HC mode
        Image *pImage = &maImage;
        Color aBackCol;
        if( !!maImageHC && ImplGetCurrentBackgroundColor( aBackCol ) )
        {
            if( aBackCol.IsDark() )
                pImage = &maImageHC;
            // #99902 no col transform required
            //if( aBackCol.IsBright() )
            //  nStyle |= IMAGE_DRAW_COLORTRANSFORM;
        }

        Point aImagePos( aImageRect.TopLeft() );
        aImagePos.X() += (aImageRect.GetWidth()-aImageSize.Width())/2;
        aImagePos.Y() += (aImageRect.GetHeight()-aImageSize.Height())/2;
        if ( IsZoom() )
            DrawImage( aImagePos, aImageSize, *pImage, nButtonStyle );
        else
            DrawImage( aImagePos, *pImage, nButtonStyle );

        aImageRect.Left()++;
        aImageRect.Top()++;
        aImageRect.Right()--;
        aImageRect.Bottom()--;

        ImplSetFocusRect( aImageRect );

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
            ShowFocus( ImplGetFocusRect() );
    }
}
}

// -----------------------------------------------------------------------

void RadioButton::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                            const Point& rPos, const Size& rSize,
                            const Size& rImageSize, long nImageSep,
                            Rectangle& rStateRect,
                            Rectangle& rMouseRect,
                            bool bLayout )
{
    WinBits                 nWinStyle = GetStyle();
    XubString               aText( GetText() );
    Rectangle               aRect( rPos, rSize );
    MetricVector*           pVector = bLayout ? &mpLayoutData->m_aUnicodeBoundRects : NULL;
    String*                 pDisplayText = bLayout ? &mpLayoutData->m_aDisplayText : NULL;

    pDev->Push( PUSH_CLIPREGION );
    pDev->IntersectClipRegion( Rectangle( rPos, rSize ) );

    // kein Image-RadioButton
    if ( !maImage )
    {
        if ( ( aText.Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) ) ||
             ( HasImage() &&  ! (ImplGetButtonState() & BUTTON_DRAW_NOIMAGE) ) )
        {
            USHORT nTextStyle = Button::ImplGetTextStyle( aText, nWinStyle, nDrawFlags );

            Size aSize( rSize );
            Point aPos( rPos );

            aPos.X() += rImageSize.Width() + nImageSep;
            aSize.Width() -= rImageSize.Width() + nImageSep;

            // if the text rect height is smaller than the height of the image
            // then for single lines the default should be centered text
            if( (nWinStyle & (WB_TOP|WB_CENTER|WB_BOTTOM)) == 0 &&
                (rImageSize.Height() > rSize.Height() || ! (nWinStyle & WB_WORDBREAK)  ) )
            {
                nTextStyle &= ~(TEXT_DRAW_TOP|TEXT_DRAW_BOTTOM);
                nTextStyle |= TEXT_DRAW_VCENTER;
                aSize.Height() = rImageSize.Height();
            }

            ImplDrawAlignedImage( pDev, aPos, aSize, bLayout, 1,
                                  nDrawFlags, nTextStyle, NULL );

            rMouseRect          = Rectangle( aPos, aSize );
            rMouseRect.Left()   = rPos.X();

            rStateRect.Left()   = rPos.X();
            rStateRect.Top()    = rMouseRect.Top();

            if ( aSize.Height() > rImageSize.Height() )
                rStateRect.Top() += ( aSize.Height() - rImageSize.Height() ) / 2;
            else
            {
                rStateRect.Top() -= ( rImageSize.Height() - aSize.Height() ) / 2;
                if( rStateRect.Top() < 0 )
                    rStateRect.Top() = 0;
            }

            rStateRect.Right()  = rStateRect.Left() + rImageSize.Width()-1;
            rStateRect.Bottom() = rStateRect.Top() + rImageSize.Height()-1;

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

            ImplSetFocusRect( rStateRect );

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
        if ( aText.Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
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

            // Text ausgeben
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
            pDev->DrawCtrlText( aTxtPos, aText, 0, STRING_LEN, TEXT_DRAW_MNEMONIC, pVector, pDisplayText );
        }

        rMouseRect = aImageRect;
        rStateRect = aImageRect;
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void RadioButton::ImplDrawRadioButton( bool bLayout )
{
    if( !bLayout )
        HideFocus();

    Size aImageSize;
    if ( !maImage )
        aImageSize = ImplGetRadioImageSize();
    else
        aImageSize  = maImage.GetSizePixel();
    aImageSize.Width()  = CalcZoom( aImageSize.Width() );
    aImageSize.Height() = CalcZoom( aImageSize.Height() );

    // Draw control text
    ImplDraw( this, 0, Point(), GetOutputSizePixel(),
              aImageSize, IMPL_SEP_BUTTON_IMAGE, maStateRect, maMouseRect, bLayout );

    if( !bLayout || (IsNativeControlSupported(CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL)==TRUE) )
    {
        if ( !maImage && HasFocus() )
            ShowFocus( ImplGetFocusRect() );

        ImplDrawRadioButtonState();
    }
}

// -----------------------------------------------------------------------

void RadioButton::GetRadioButtonGroup( std::vector< RadioButton* >& io_rGroup, bool bIncludeThis ) const
{
    // empty the list
    io_rGroup.clear();

    // go back to first in group;
    Window* pFirst = const_cast<RadioButton*>(this);
    while( ( pFirst->GetStyle() & WB_GROUP ) == 0 )
    {
        Window* pWindow = pFirst->GetWindow( WINDOW_PREV );
        if( pWindow )
            pFirst = pWindow;
        else
            break;
    }
    // insert radiobuttons up to next group
    do
    {
        if( pFirst->GetType() == WINDOW_RADIOBUTTON )
        {
            if( pFirst != this || bIncludeThis )
                io_rGroup.push_back( static_cast<RadioButton*>(pFirst) );
        }
        pFirst = pFirst->GetWindow( WINDOW_NEXT );
    } while( pFirst && ( ( pFirst->GetStyle() & WB_GROUP ) == 0 ) );
}

// -----------------------------------------------------------------------

void RadioButton::ImplUncheckAllOther()
{
    mpWindowImpl->mnStyle |= WB_TABSTOP;

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
                pWindow->mpWindowImpl->mnStyle &= ~WB_TABSTOP;
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
            pWindow->mpWindowImpl->mnStyle &= ~WB_TABSTOP;
        }

        pWindow = pWindow->GetWindow( WINDOW_NEXT );
    }
}

// -----------------------------------------------------------------------

void RadioButton::ImplCallClick( BOOL bGrabFocus, USHORT nFocusFlags )
{
    mbStateChanged = !mbChecked;
    mbChecked = TRUE;
    mpWindowImpl->mnStyle |= WB_TABSTOP;
    ImplInvalidateOrDrawRadioButtonState();
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
    ImplInitRadioButtonData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

RadioButton::RadioButton( Window* pParent, const ResId& rResId ) :
    Button( WINDOW_RADIOBUTTON )
{
    ImplInitRadioButtonData();
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
        ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
        ImplInvalidateOrDrawRadioButtonState();
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
        if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;

            // Bei Abbruch kein Click-Handler rufen
            if ( !rTEvt.IsTrackingCanceled() )
                ImplCallClick();
            else
                ImplInvalidateOrDrawRadioButtonState();
        }
    }
    else
    {
        if ( maMouseRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( !(ImplGetButtonState() & BUTTON_DRAW_PRESSED) )
            {
                ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
                ImplInvalidateOrDrawRadioButtonState();
            }
        }
        else
        {
            if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
            {
                ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
                ImplInvalidateOrDrawRadioButtonState();
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
        if ( !(ImplGetButtonState() & BUTTON_DRAW_PRESSED) )
        {
            ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
            ImplInvalidateOrDrawRadioButtonState();
        }
    }
    else if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplInvalidateOrDrawRadioButtonState();
    }
    else
        Button::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void RadioButton::KeyUp( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplCallClick();
    }
    else
        Button::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void RadioButton::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
    const_cast<RadioButton*>(this)->ImplDrawRadioButton( true );
}

// -----------------------------------------------------------------------

void RadioButton::Paint( const Rectangle& )
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
                  aStateRect, aMouseRect );

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
    Control::Resize();
    Invalidate();
}

// -----------------------------------------------------------------------

void RadioButton::GetFocus()
{
    ShowFocus( ImplGetFocusRect() );
    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

// -----------------------------------------------------------------------

void RadioButton::LoseFocus()
{
    if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplInvalidateOrDrawRadioButtonState();
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
            Invalidate( maStateRect );
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

long RadioButton::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL) )
            {
                if( ( maMouseRect.IsInside( GetPointerPosPixel()) &&
                     !maMouseRect.IsInside( GetLastPointerPosPixel()) ) ||
                    ( maMouseRect.IsInside( GetLastPointerPosPixel()) &&
                     !maMouseRect.IsInside( GetPointerPosPixel()) ) ||
                     pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
                {
                    Invalidate( maStateRect );
                }
            }
        }
    }

    return nDone ? nDone : Button::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void RadioButton::Toggle()
{
    ImplCallEventListenersAndHandler( VCLEVENT_RADIOBUTTON_TOGGLE, maToggleHdl, this );
}

// -----------------------------------------------------------------------

BOOL RadioButton::SetModeRadioImage( const Image& rImage, BmpColorMode eMode )
{
    if( eMode == BMP_COLOR_NORMAL )
{
    if ( rImage != maImage )
    {
        maImage = rImage;
        StateChanged( STATE_CHANGE_DATA );
    }
}
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
    {
        if( maImageHC != rImage )
        {
            maImageHC = rImage;
            StateChanged( STATE_CHANGE_DATA );
        }
    }
    else
        return FALSE;

    return TRUE;
}

// -----------------------------------------------------------------------

const Image& RadioButton::GetModeRadioImage( BmpColorMode eMode ) const
{
    if( eMode == BMP_COLOR_HIGHCONTRAST )
        return maImageHC;
    else
        return maImage;
}

// -----------------------------------------------------------------------

void RadioButton::SetState( BOOL bCheck )
{
    // TabStop-Flag richtig mitfuehren
    if ( bCheck )
        mpWindowImpl->mnStyle |= WB_TABSTOP;
    else
        mpWindowImpl->mnStyle &= ~WB_TABSTOP;

    if ( mbChecked != bCheck )
    {
        mbChecked = bCheck;
        StateChanged( STATE_CHANGE_STATE );
        Toggle();
    }
}

// -----------------------------------------------------------------------

void RadioButton::Check( BOOL bCheck )
{
    // TabStop-Flag richtig mitfuehren
    if ( bCheck )
        mpWindowImpl->mnStyle |= WB_TABSTOP;
    else
        mpWindowImpl->mnStyle &= ~WB_TABSTOP;

    if ( mbChecked != bCheck )
    {
        mbChecked = bCheck;
        ImplDelData aDelData;
        ImplAddDel( &aDelData );
        StateChanged( STATE_CHANGE_STATE );
        if ( aDelData.IsDelete() )
            return;
        if ( bCheck && mbRadioCheck )
            ImplUncheckAllOther();
        if ( aDelData.IsDelete() )
            return;
        Toggle();
        ImplRemoveDel( &aDelData );
    }
}

// -----------------------------------------------------------------------

Size RadioButton::ImplGetRadioImageSize() const
{
    Size aSize;
    // why are IsNativeControlSupported and GetNativeControlRegion not const ?
    RadioButton* pThis = const_cast<RadioButton*>(this);
    bool bDefaultSize = true;
    if( pThis->IsNativeControlSupported( CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aControlValue;
        // #i45896# workaround gcc3.3 temporary problem
        Region           aCtrlRegion = Region( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
        ControlState     nState = CTRL_STATE_DEFAULT|CTRL_STATE_ENABLED;
        Region aBoundingRgn, aContentRgn;

        // get native size of a radio button
        if( pThis->GetNativeControlRegion( CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL, aCtrlRegion,
                                           nState, aControlValue, rtl::OUString(),
                                           aBoundingRgn, aContentRgn ) )
        {
            Rectangle aCont(aContentRgn.GetBoundRect());
            aSize = aCont.GetSize();
            bDefaultSize = false;
        }
    }
    if( bDefaultSize )
        aSize = GetRadioImage( GetSettings(), 0 ).GetSizePixel();
    return aSize;
}

static void LoadThemedImageList (const StyleSettings &rStyleSettings,
                                 ImageList *pList, const ResId &rResId,
                                 USHORT nImages)
{
    Color aColorAry1[6];
    Color aColorAry2[6];
    aColorAry1[0] = Color( 0xC0, 0xC0, 0xC0 );
    aColorAry1[1] = Color( 0xFF, 0xFF, 0x00 );
    aColorAry1[2] = Color( 0xFF, 0xFF, 0xFF );
    aColorAry1[3] = Color( 0x80, 0x80, 0x80 );
    aColorAry1[4] = Color( 0x00, 0x00, 0x00 );
    aColorAry1[5] = Color( 0x00, 0xFF, 0x00 );
    aColorAry2[0] = rStyleSettings.GetFaceColor();
    aColorAry2[1] = rStyleSettings.GetWindowColor();
    aColorAry2[2] = rStyleSettings.GetLightColor();
    aColorAry2[3] = rStyleSettings.GetShadowColor();
    aColorAry2[4] = rStyleSettings.GetDarkShadowColor();
    aColorAry2[5] = rStyleSettings.GetWindowTextColor();

    Color aMaskColor(0x00, 0x00, 0xFF );
        DBG_ASSERT( sizeof(aColorAry1) == sizeof(aColorAry2), "aColorAry1 must match aColorAry2" );
    // FIXME: do we want the mask for the checkbox ?
    pList->InsertFromHorizontalBitmap (rResId, nImages, &aMaskColor,
        aColorAry1, aColorAry2, sizeof(aColorAry1) / sizeof(Color));
}

Image RadioButton::GetRadioImage( const AllSettings& rSettings, USHORT nFlags )
{
    ImplSVData*             pSVData = ImplGetSVData();
    const StyleSettings&    rStyleSettings = rSettings.GetStyleSettings();
    USHORT                  nStyle = rStyleSettings.GetRadioButtonStyle() & STYLE_RADIOBUTTON_STYLE;

    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
        nStyle = STYLE_RADIOBUTTON_MONO;

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

        Color pColorAry1[6];
        Color pColorAry2[6];
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

        ResMgr* pResMgr = ImplGetResMgr();
        pSVData->maCtrlData.mpRadioImgList = new ImageList();
        if( pResMgr )
        LoadThemedImageList( rStyleSettings,
                 pSVData->maCtrlData.mpRadioImgList,
                 ResId( SV_RESID_BITMAP_RADIO+nStyle, *pResMgr ), 6 );
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

void RadioButton::ImplSetMinimumNWFSize()
{
    Push( PUSH_MAPMODE );
    SetMapMode( MAP_PIXEL );

    ImplControlValue aControlValue;
    Size aCurSize( GetSizePixel() );
    Region aCtrlRegion = Region( Rectangle( Point( 0, 0 ), aCurSize ) );
    Region aBoundingRgn, aContentRgn;

    // get native size of a radiobutton
    if( GetNativeControlRegion( CTRL_RADIOBUTTON, PART_ENTIRE_CONTROL, aCtrlRegion,
                                CTRL_STATE_DEFAULT|CTRL_STATE_ENABLED, aControlValue, rtl::OUString(),
                                aBoundingRgn, aContentRgn ) )
    {
        Rectangle aCont(aContentRgn.GetBoundRect());
        Size aSize = aCont.GetSize();

        if( aSize.Height() > aCurSize.Height() )
        {
            aCurSize.Height() = aSize.Height();
            SetSizePixel( aCurSize );
        }
    }

    Pop();
}

// -----------------------------------------------------------------------

Size RadioButton::CalcMinimumSize( long nMaxWidth ) const
{
    Size aSize;
    if ( !maImage )
        aSize = ImplGetRadioImageSize();
    else
        aSize = maImage.GetSizePixel();

    nMaxWidth -= aSize.Width();

    XubString aText = GetText();
    if ( aText.Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
    {
        // subtract what will be added later
        nMaxWidth-=2;
        nMaxWidth -= IMPL_SEP_BUTTON_IMAGE;

        Size aTextSize = GetTextRect( Rectangle( Point(), Size( nMaxWidth > 0 ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                                      aText, FixedText::ImplGetTextStyle( GetStyle() ) ).GetSize();
        aSize.Width()+=2;   // for focus rect
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

// -----------------------------------------------------------------------

Size RadioButton::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Button::GetOptimalSize( eType );
    }
}

// =======================================================================

void CheckBox::ImplInitCheckBoxData()
{
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
        if ( !IsControlBackground() &&
            (pParent->IsChildTransparentModeEnabled() || IsNativeControlSupported( CTRL_CHECKBOX, PART_ENTIRE_CONTROL ) ) )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
            if( IsNativeControlSupported( CTRL_CHECKBOX, PART_ENTIRE_CONTROL ) )
                ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;
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

void CheckBox::ImplInvalidateOrDrawCheckBoxState()
{
    if( ImplGetSVData()->maNWFData.mbCheckBoxNeedsErase )
    {
        if ( IsNativeControlSupported(CTRL_CHECKBOX, PART_ENTIRE_CONTROL) )
        {
            Invalidate();
            Update();
            return;
        }
    }
    ImplDrawCheckBoxState();
}

void CheckBox::ImplDrawCheckBoxState()
{
    bool    bNativeOK = TRUE;

    if ( (bNativeOK=IsNativeControlSupported(CTRL_CHECKBOX, PART_ENTIRE_CONTROL)) == TRUE )
    {
        ImplControlValue    aControlValue( meState == STATE_CHECK ? BUTTONVALUE_ON : BUTTONVALUE_OFF, rtl::OUString(), 0 );
        Region              aCtrlRegion( maStateRect );
        ControlState        nState = 0;

        if ( HasFocus() )                       nState |= CTRL_STATE_FOCUSED;
        if ( ImplGetButtonState() & BUTTON_DRAW_DEFAULT )   nState |= CTRL_STATE_DEFAULT;
        if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )   nState |= CTRL_STATE_PRESSED;
        if ( IsEnabled() )                      nState |= CTRL_STATE_ENABLED;

        if ( meState == STATE_CHECK )
            aControlValue.setTristateVal( BUTTONVALUE_ON );
        else if ( meState == STATE_DONTKNOW )
            aControlValue.setTristateVal( BUTTONVALUE_MIXED );

        if ( IsMouseOver() && maMouseRect.IsInside( GetPointerPosPixel() ) )
            nState |= CTRL_STATE_ROLLOVER;

        bNativeOK = DrawNativeControl( CTRL_CHECKBOX, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                                 aControlValue, rtl::OUString() );
    }

    if ( bNativeOK == FALSE )
    {
        USHORT nStyle = ImplGetButtonState();
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
}

// -----------------------------------------------------------------------

void CheckBox::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                         const Point& rPos, const Size& rSize,
                         const Size& rImageSize, long nImageSep,
                         Rectangle& rStateRect, Rectangle& rMouseRect,
                         bool bLayout )
{
    WinBits                 nWinStyle = GetStyle();
    XubString               aText( GetText() );

    pDev->Push( PUSH_CLIPREGION | PUSH_LINECOLOR );
    pDev->IntersectClipRegion( Rectangle( rPos, rSize ) );

    long nLineY = rPos.Y() + (rSize.Height()-1)/2;
    if ( ( aText.Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) ) ||
         ( HasImage() && !  (ImplGetButtonState() & BUTTON_DRAW_NOIMAGE) ) )
    {
        USHORT nTextStyle = Button::ImplGetTextStyle( aText, nWinStyle, nDrawFlags );

        Size aSize( rSize );
        Point aPos( rPos );
        aPos.X() += rImageSize.Width() + nImageSep;
        aSize.Width() -= rImageSize.Width() + nImageSep;

        // if the text rect height is smaller than the height of the image
        // then for single lines the default should be centered text
        if( (nWinStyle & (WB_TOP|WB_CENTER|WB_BOTTOM)) == 0 &&
            (rImageSize.Height() > rSize.Height() || ! (nWinStyle & WB_WORDBREAK) ) )
        {
            nTextStyle &= ~(TEXT_DRAW_TOP|TEXT_DRAW_BOTTOM);
            nTextStyle |= TEXT_DRAW_VCENTER;
            aSize.Height() = rImageSize.Height();
        }

        ImplDrawAlignedImage( pDev, aPos, aSize, bLayout, 1,
                              nDrawFlags, nTextStyle, NULL );
        nLineY = aPos.Y() + aSize.Height()/2;

        rMouseRect          = Rectangle( aPos, aSize );
        rMouseRect.Left()   = rPos.X();
        rStateRect.Left()   = rPos.X();
        rStateRect.Top()    = rMouseRect.Top();

        if ( aSize.Height() > rImageSize.Height() )
            rStateRect.Top() += ( aSize.Height() - rImageSize.Height() ) / 2;
        else
        {
            rStateRect.Top() -= ( rImageSize.Height() - aSize.Height() ) / 2;
            if( rStateRect.Top() < 0 )
                rStateRect.Top() = 0;
        }

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
            rStateRect.Left() = rPos.X()+rSize.Width()-rImageSize.Width();
        else
            rStateRect.Left() = rPos.X();
        if ( nWinStyle & WB_VCENTER )
            rStateRect.Top() = rPos.Y()+((rSize.Height()-rImageSize.Height())/2);
        else if ( nWinStyle & WB_BOTTOM )
            rStateRect.Top() = rPos.Y()+rSize.Height()-rImageSize.Height();
        else
            rStateRect.Top() = rPos.Y();
        rStateRect.Right()  = rStateRect.Left()+rImageSize.Width()-1;
        rStateRect.Bottom() = rStateRect.Top()+rImageSize.Height()-1;
        // provide space for focusrect
        // note: this assumes that the control's size was adjusted
        // accordingly in Get/LoseFocus, so the onscreen position won't change
        if( HasFocus() )
            rStateRect.Move( 1, 1 );
        rMouseRect          = rStateRect;

        ImplSetFocusRect( rStateRect );
    }

    const int nLineSpace = 4;
    if( (GetStyle() & WB_CBLINESTYLE) != 0 &&
        rMouseRect.Right()-1-nLineSpace < rPos.X()+rSize.Width() )
    {
        const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
            SetLineColor( Color( COL_BLACK ) );
        else
            SetLineColor( rStyleSettings.GetShadowColor() );
        long nLineX = rMouseRect.Right()+nLineSpace;
        DrawLine( Point( nLineX, nLineY ), Point( rPos.X() + rSize.Width()-1, nLineY ) );
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        {
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( nLineX, nLineY+1 ), Point( rPos.X() + rSize.Width()-1, nLineY+1 ) );
        }
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void CheckBox::ImplDrawCheckBox( bool bLayout )
{
    Size aImageSize = ImplGetCheckImageSize();
    aImageSize.Width()  = CalcZoom( aImageSize.Width() );
    aImageSize.Height() = CalcZoom( aImageSize.Height() );

    if( !bLayout )
        HideFocus();

    ImplDraw( this, 0, Point(), GetOutputSizePixel(), aImageSize,
              IMPL_SEP_BUTTON_IMAGE, maStateRect, maMouseRect, bLayout );

    if( !bLayout )
    {
        ImplDrawCheckBoxState();
        if ( HasFocus() )
            ShowFocus( ImplGetFocusRect() );
    }
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
    ImplInvalidateOrDrawCheckBoxState();

    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    Toggle();
    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );
    Click();
}

// -----------------------------------------------------------------------

CheckBox::CheckBox( Window* pParent, WinBits nStyle ) :
    Button( WINDOW_CHECKBOX )
{
    ImplInitCheckBoxData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

CheckBox::CheckBox( Window* pParent, const ResId& rResId ) :
    Button( WINDOW_CHECKBOX )
{
    ImplInitCheckBoxData();
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
        ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
        ImplInvalidateOrDrawCheckBoxState();
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
        if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) && !rTEvt.IsTrackingCanceled() )
                GrabFocus();

            ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;

            // Bei Abbruch kein Click-Handler rufen
            if ( !rTEvt.IsTrackingCanceled() )
                ImplCheck();
            else
                ImplInvalidateOrDrawCheckBoxState();
        }
    }
    else
    {
        if ( maMouseRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() ) )
        {
            if ( !(ImplGetButtonState() & BUTTON_DRAW_PRESSED) )
            {
                ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
                ImplInvalidateOrDrawCheckBoxState();
            }
        }
        else
        {
            if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
            {
                ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
                ImplInvalidateOrDrawCheckBoxState();
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
        if ( !(ImplGetButtonState() & BUTTON_DRAW_PRESSED) )
        {
            ImplGetButtonState() |= BUTTON_DRAW_PRESSED;
            ImplInvalidateOrDrawCheckBoxState();
        }
    }
    else if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_ESCAPE) )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplInvalidateOrDrawCheckBoxState();
    }
    else
        Button::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void CheckBox::KeyUp( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();

    if ( (ImplGetButtonState() & BUTTON_DRAW_PRESSED) && (aKeyCode.GetCode() == KEY_SPACE) )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplCheck();
    }
    else
        Button::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void CheckBox::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
    const_cast<CheckBox*>(this)->ImplDrawCheckBox( true );
}

// -----------------------------------------------------------------------

void CheckBox::Paint( const Rectangle& )
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
              aStateRect, aMouseRect, false );

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
    Control::Resize();
    Invalidate();
}

// -----------------------------------------------------------------------

void CheckBox::GetFocus()
{
    if ( !GetText().Len() || (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
    {
        // increase button size to have space for focus rect
        // checkboxes without text will draw focusrect around the check
        // See CheckBox::ImplDraw()
        Point aPos( GetPosPixel() );
        Size aSize( GetSizePixel() );
        aPos.Move(-1,-1);
        aSize.Height() += 2;
        aSize.Width() += 2;
        SetPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height(), WINDOW_POSSIZE_ALL );
        ImplDrawCheckBox();
    }
    else
        ShowFocus( ImplGetFocusRect() );

    SetInputContext( InputContext( GetFont() ) );
    Button::GetFocus();
}

// -----------------------------------------------------------------------

void CheckBox::LoseFocus()
{
    if ( ImplGetButtonState() & BUTTON_DRAW_PRESSED )
    {
        ImplGetButtonState() &= ~BUTTON_DRAW_PRESSED;
        ImplInvalidateOrDrawCheckBoxState();
    }

    HideFocus();
    Button::LoseFocus();

    if ( !GetText().Len() || (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
    {
        // decrease button size again (see GetFocus())
        // checkboxes without text will draw focusrect around the check
        Point aPos( GetPosPixel() );
        Size aSize( GetSizePixel() );
        aPos.Move(1,1);
        aSize.Height() -= 2;
        aSize.Width() -= 2;
        SetPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height(), WINDOW_POSSIZE_ALL );
        ImplDrawCheckBox();
    }
}

// -----------------------------------------------------------------------

void CheckBox::StateChanged( StateChangedType nType )
{
    Button::StateChanged( nType );

    if ( nType == STATE_CHANGE_STATE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate( maStateRect );
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

long CheckBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(CTRL_CHECKBOX, PART_ENTIRE_CONTROL) )
            {
                if( ( maMouseRect.IsInside( GetPointerPosPixel()) &&
                     !maMouseRect.IsInside( GetLastPointerPosPixel()) ) ||
                    ( maMouseRect.IsInside( GetLastPointerPosPixel()) &&
                     !maMouseRect.IsInside( GetPointerPosPixel()) ) ||
                    pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
                {
                    Invalidate( maStateRect );
                }
            }
        }
    }

    return nDone ? nDone : Button::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void CheckBox::Toggle()
{
    ImplCallEventListenersAndHandler( VCLEVENT_CHECKBOX_TOGGLE, maToggleHdl, this );
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
        Toggle();
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

Size CheckBox::ImplGetCheckImageSize() const
{
    Size aSize;
    // why are IsNativeControlSupported and GetNativeControlRegion not const ?
    CheckBox* pThis = const_cast<CheckBox*>(this);
    bool bDefaultSize = true;
    if( pThis->IsNativeControlSupported( CTRL_CHECKBOX, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aControlValue;
        // #i45896# workaround gcc3.3 temporary problem
        Region           aCtrlRegion = Region( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
        ControlState     nState = CTRL_STATE_DEFAULT|CTRL_STATE_ENABLED;
        Region aBoundingRgn, aContentRgn;

        // get native size of a check box
        if( pThis->GetNativeControlRegion( CTRL_CHECKBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                           nState, aControlValue, rtl::OUString(),
                                           aBoundingRgn, aContentRgn ) )
        {
            Rectangle aCont(aContentRgn.GetBoundRect());
            aSize = aCont.GetSize();
            bDefaultSize = false;
        }
    }
    if( bDefaultSize )
        aSize = GetCheckImage( GetSettings(), 0 ).GetSizePixel();
    return aSize;
}

Image CheckBox::GetCheckImage( const AllSettings& rSettings, USHORT nFlags )
{
    ImplSVData*             pSVData = ImplGetSVData();
    const StyleSettings&    rStyleSettings = rSettings.GetStyleSettings();
    USHORT                  nStyle = rStyleSettings.GetCheckBoxStyle() & STYLE_CHECKBOX_STYLE;

    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
        nStyle = STYLE_CHECKBOX_MONO;

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

        ResMgr* pResMgr = ImplGetResMgr();
        pSVData->maCtrlData.mpCheckImgList = new ImageList();
        if( pResMgr )
        LoadThemedImageList( rStyleSettings,
                 pSVData->maCtrlData.mpCheckImgList,
                 ResId( SV_RESID_BITMAP_CHECK+nStyle, *pResMgr ), 9 );
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

void CheckBox::ImplSetMinimumNWFSize()
{
    Push( PUSH_MAPMODE );
    SetMapMode( MAP_PIXEL );

    ImplControlValue aControlValue;
    Size aCurSize( GetSizePixel() );
    Region aCtrlRegion = Region( Rectangle( Point( 0, 0 ), aCurSize ) );
    Region aBoundingRgn, aContentRgn;

    // get native size of a radiobutton
    if( GetNativeControlRegion( CTRL_CHECKBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                CTRL_STATE_DEFAULT|CTRL_STATE_ENABLED, aControlValue, rtl::OUString(),
                                aBoundingRgn, aContentRgn ) )
    {
        Rectangle aCont(aContentRgn.GetBoundRect());
        Size aSize = aCont.GetSize();

        if( aSize.Height() > aCurSize.Height() )
        {
            aCurSize.Height() = aSize.Height();
            SetSizePixel( aCurSize );
        }
    }

    Pop();
}

// -----------------------------------------------------------------------

Size CheckBox::CalcMinimumSize( long nMaxWidth ) const
{
    Size aSize = ImplGetCheckImageSize();
    nMaxWidth -= aSize.Width();

    XubString aText = GetText();
    if ( aText.Len() && ! (ImplGetButtonState() & BUTTON_DRAW_NOTEXT) )
    {
        // subtract what will be added later
        nMaxWidth-=2;
        nMaxWidth -= IMPL_SEP_BUTTON_IMAGE;

        Size aTextSize = GetTextRect( Rectangle( Point(), Size( nMaxWidth > 0 ? nMaxWidth : 0x7fffffff, 0x7fffffff ) ),
                                      aText, FixedText::ImplGetTextStyle( GetStyle() ) ).GetSize();
        aSize.Width()+=2;    // for focus rect
        aSize.Width() += IMPL_SEP_BUTTON_IMAGE;
        aSize.Width() += aTextSize.Width();
        if ( aSize.Height() < aTextSize.Height() )
            aSize.Height() = aTextSize.Height();
    }
    else
    {
        // is this still correct ? since the checkbox now
        // shows a focus rect it should be 2 pixels wider and longer
/* da ansonsten im Writer die Control zu weit oben haengen
        aSize.Width() += 2;
        aSize.Height() += 2;
*/
    }

    return CalcWindowSize( aSize );
}

// -----------------------------------------------------------------------

Size CheckBox::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Button::GetOptimalSize( eType );
    }
}

// =======================================================================

ImageButton::ImageButton( WindowType nType ) :
    PushButton( nType )
{
    ImplInitStyle();
}

// -----------------------------------------------------------------------

ImageButton::ImageButton( Window* pParent, WinBits nStyle ) :
    PushButton( pParent, nStyle )
{
    ImplInitStyle();
}

// -----------------------------------------------------------------------

ImageButton::ImageButton( Window* pParent, const ResId& rResId ) :
    PushButton( pParent, rResId.SetRT( RSC_IMAGEBUTTON ) )
{
    ULONG nObjMask = ReadLongRes();

    if ( RSC_IMAGEBUTTON_IMAGE & nObjMask )
    {
        SetModeImage( Image( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }

    if ( RSC_IMAGEBUTTON_SYMBOL & nObjMask )
        SetSymbol( (SymbolType)ReadLongRes() );

    if ( RSC_IMAGEBUTTON_STATE & nObjMask )
        SetState( (TriState)ReadLongRes() );

    ImplInitStyle();
}

// -----------------------------------------------------------------------

ImageButton::~ImageButton()
{
}

// -----------------------------------------------------------------------
void ImageButton::ImplInitStyle()
{
    WinBits nStyle = GetStyle();

    if ( ! ( nStyle & ( WB_RIGHT | WB_LEFT ) ) )
        nStyle |= WB_CENTER;

    if ( ! ( nStyle & ( WB_TOP | WB_BOTTOM ) ) )
        nStyle |= WB_VCENTER;

    SetStyle( nStyle );
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
    ULONG nObjMask = ReadLongRes();

    if ( RSC_IMAGERADIOBUTTON_IMAGE & nObjMask )
    {
        SetModeRadioImage( Image( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) ) );
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

    ULONG  nTriState        = ReadLongRes();
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
