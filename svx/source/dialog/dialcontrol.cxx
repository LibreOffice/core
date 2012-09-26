/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "svx/dialcontrol.hxx"

#include <math.h>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/field.hxx>
#include <svtools/colorcfg.hxx>

namespace svx {

// ============================================================================

const long DIAL_OUTER_WIDTH = 8;

// ============================================================================

class DialControlBmp : public VirtualDevice
{
public:
    explicit            DialControlBmp( Window& rParent );

    void                InitBitmap( const Size& rSize, const Font& rFont );
    void                CopyBackground( const DialControlBmp& rSrc );
    void                DrawBackground( const Size& rSize, bool bEnabled );
    void                DrawElements( const String& rText, sal_Int32 nAngle );

private:
    const Color&        GetBackgroundColor() const;
    const Color&        GetTextColor() const;
    const Color&        GetScaleLineColor() const;
    const Color&        GetButtonLineColor() const;
    const Color&        GetButtonFillColor( bool bMain ) const;

    void                Init( const Size& rSize );
    void                DrawBackground();

    Window&             mrParent;
    Rectangle           maRect;
    long                mnCenterX;
    long                mnCenterY;
    bool                mbEnabled;
};

// ----------------------------------------------------------------------------

DialControlBmp::DialControlBmp( Window& rParent ) :
    VirtualDevice( rParent, 0, 0 ),
    mrParent( rParent ),
    mnCenterX(0),
    mnCenterY(0),
    mbEnabled( true )
{
    EnableRTL( sal_False );
}

void DialControlBmp::InitBitmap( const Size& rSize, const Font& rFont )
{
    Init( rSize );
    SetFont( rFont );
}

void DialControlBmp::CopyBackground( const DialControlBmp& rSrc )
{
    Init( rSrc.maRect.GetSize() );
    mbEnabled = rSrc.mbEnabled;
    Point aPos;
    DrawBitmapEx( aPos, rSrc.GetBitmapEx( aPos, maRect.GetSize() ) );
}

void DialControlBmp::DrawBackground( const Size& rSize, bool bEnabled )
{
    Init( rSize );
    mbEnabled = bEnabled;
    DrawBackground();
}

void DialControlBmp::DrawElements( const String& rText, sal_Int32 nAngle )
{
    // *** rotated text ***

    Font aFont( GetFont() );
    aFont.SetColor( GetTextColor() );
    aFont.SetOrientation( static_cast< short >( (nAngle + 5) / 10 ) );  // Font uses 1/10 degrees
    aFont.SetWeight( WEIGHT_BOLD );
    SetFont( aFont );

    double fAngle = nAngle * F_PI180 / 100.0;
    double fSin = sin( fAngle );
    double fCos = cos( fAngle );
    double fWidth = GetTextWidth( rText ) / 2.0;
    double fHeight = GetTextHeight() / 2.0;
    long nX = static_cast< long >( mnCenterX - fWidth * fCos - fHeight * fSin );
    long nY = static_cast< long >( mnCenterY + fWidth * fSin - fHeight * fCos );
    Rectangle aRect( nX, nY, 2 * mnCenterX - nX, 2 * mnCenterY - nY );
    DrawText( aRect, rText, mbEnabled ? 0 : TEXT_DRAW_DISABLE );

    // *** drag button ***

    bool bMain = (nAngle % 4500) != 0;
    SetLineColor( GetButtonLineColor() );
    SetFillColor( GetButtonFillColor( bMain ) );

    nX = mnCenterX - static_cast< long >( (DIAL_OUTER_WIDTH / 2 - mnCenterX) * fCos );
    nY = mnCenterY - static_cast< long >( (mnCenterY - DIAL_OUTER_WIDTH / 2) * fSin );
    long nSize = bMain ? (DIAL_OUTER_WIDTH / 4) : (DIAL_OUTER_WIDTH / 2 - 1);
    DrawEllipse( Rectangle( nX - nSize, nY - nSize, nX + nSize, nY + nSize ) );
}

// private --------------------------------------------------------------------

const Color& DialControlBmp::GetBackgroundColor() const
{
    return GetSettings().GetStyleSettings().GetDialogColor();
}

const Color& DialControlBmp::GetTextColor() const
{
    return GetSettings().GetStyleSettings().GetLabelTextColor();
}

const Color& DialControlBmp::GetScaleLineColor() const
{
    const StyleSettings& rSett = GetSettings().GetStyleSettings();
    return mbEnabled ? rSett.GetButtonTextColor() : rSett.GetDisableColor();
}

const Color& DialControlBmp::GetButtonLineColor() const
{
    const StyleSettings& rSett = GetSettings().GetStyleSettings();
    return mbEnabled ? rSett.GetButtonTextColor() : rSett.GetDisableColor();
}

const Color& DialControlBmp::GetButtonFillColor( bool bMain ) const
{
    const StyleSettings& rSett = GetSettings().GetStyleSettings();
    return mbEnabled ? (bMain ? rSett.GetMenuColor() : rSett.GetHighlightColor()) : rSett.GetDisableColor();
}

void DialControlBmp::Init( const Size& rSize )
{
    SetSettings( mrParent.GetSettings() );
    maRect.SetPos( Point( 0, 0 ) );
    maRect.SetSize( rSize );
    mnCenterX = rSize.Width() / 2;
    mnCenterY = rSize.Height() / 2;
    SetOutputSize( rSize );
    SetBackground();
}

void DialControlBmp::DrawBackground()
{
    // *** background with 3D effect ***

    SetLineColor();
    SetFillColor();
    Erase();

    EnableRTL( sal_True ); // draw 3D effect in correct direction

    sal_uInt8 nDiff = mbEnabled ? 0x18 : 0x10;
    Color aColor;

    aColor = GetBackgroundColor();
    SetFillColor( aColor );
    DrawPie( maRect, maRect.TopRight(), maRect.TopCenter() );
    DrawPie( maRect, maRect.BottomLeft(), maRect.BottomCenter() );

    aColor.DecreaseLuminance( nDiff );
    SetFillColor( aColor );
    DrawPie( maRect, maRect.BottomCenter(), maRect.TopRight() );

    aColor.DecreaseLuminance( nDiff );
    SetFillColor( aColor );
    DrawPie( maRect, maRect.BottomRight(), maRect.RightCenter() );

    aColor = GetBackgroundColor();
    aColor.IncreaseLuminance( nDiff );
    SetFillColor( aColor );
    DrawPie( maRect, maRect.TopCenter(), maRect.BottomLeft() );

    aColor.IncreaseLuminance( nDiff );
    SetFillColor( aColor );
    DrawPie( maRect, maRect.TopLeft(), maRect.LeftCenter() );

    EnableRTL( sal_False );

    // *** calibration ***

    Point aStartPos( mnCenterX, mnCenterY );
    Color aFullColor( GetScaleLineColor() );
    Color aLightColor( GetBackgroundColor() );
    aLightColor.Merge( aFullColor, 128 );

    for( int nAngle = 0; nAngle < 360; nAngle += 15 )
    {
        SetLineColor( (nAngle % 45) ? aLightColor : aFullColor );
        double fAngle = nAngle * F_PI180;
        long nX = static_cast< long >( -mnCenterX * cos( fAngle ) );
        long nY = static_cast< long >( mnCenterY * sin( fAngle ) );
        DrawLine( aStartPos, Point( mnCenterX - nX, mnCenterY - nY ) );
    }

    // *** clear inner area ***

    SetLineColor();
    SetFillColor( GetBackgroundColor() );
    DrawEllipse( Rectangle( maRect.Left() + DIAL_OUTER_WIDTH, maRect.Top() + DIAL_OUTER_WIDTH,
        maRect.Right() - DIAL_OUTER_WIDTH, maRect.Bottom() - DIAL_OUTER_WIDTH ) );
}

// ============================================================================

struct DialControl_Impl
{
    DialControlBmp      maBmpEnabled;
    DialControlBmp      maBmpDisabled;
    DialControlBmp      maBmpBuffered;
    Link                maModifyHdl;
    NumericField*       mpLinkField;
    Size                maWinSize;
    Font                maWinFont;
    sal_Int32           mnAngle;
    sal_Int32           mnInitialAngle;
    sal_Int32           mnOldAngle;
    long                mnCenterX;
    long                mnCenterY;
    bool                mbNoRot;

    explicit            DialControl_Impl( Window& rParent );
    void                Init( const Size& rWinSize, const Font& rWinFont );
};

// ----------------------------------------------------------------------------

DialControl_Impl::DialControl_Impl( Window& rParent ) :
    maBmpEnabled( rParent ),
    maBmpDisabled( rParent ),
    maBmpBuffered( rParent ),
    mpLinkField( 0 ),
    mnAngle( 0 ),
    mnInitialAngle( 0 ),
    mnCenterX( 0 ),
    mnCenterY( 0 ),
    mbNoRot( false )
{
}

void DialControl_Impl::Init( const Size& rWinSize, const Font& rWinFont )
{
    // "(x - 1) | 1" creates odd value <= x, to have a well-defined center pixel position
    maWinSize = Size( (rWinSize.Width() - 1) | 1, (rWinSize.Height() - 1) | 1 );
    maWinFont = rWinFont;

    mnCenterX = maWinSize.Width() / 2;
    mnCenterY = maWinSize.Height() / 2;
    maWinFont.SetTransparent( sal_True );

    maBmpEnabled.DrawBackground( maWinSize, true );
    maBmpDisabled.DrawBackground( maWinSize, false );
    maBmpBuffered.InitBitmap( maWinSize, maWinFont );
}

// ============================================================================

DialControl::DialControl( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    mpImpl( new DialControl_Impl( *this ) )
{
    Init( GetOutputSizePixel() );
}

DialControl::~DialControl()
{
}

void DialControl::Paint( const Rectangle&  )
{
    Point aPos;
    DrawBitmapEx( aPos, mpImpl->maBmpBuffered.GetBitmapEx( aPos, mpImpl->maWinSize ) );
}

void DialControl::StateChanged( StateChangedType nStateChange )
{
    if( nStateChange == STATE_CHANGE_ENABLE )
        InvalidateControl();

    // update the linked edit field
    if( mpImpl->mpLinkField )
    {
        NumericField& rField = *mpImpl->mpLinkField;
        switch( nStateChange )
        {
            case STATE_CHANGE_VISIBLE:  rField.Show( IsVisible() );     break;
            case STATE_CHANGE_ENABLE:   rField.Enable( IsEnabled() );   break;
        }
    }

    Control::StateChanged( nStateChange );
}

void DialControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Init( mpImpl->maWinSize, mpImpl->maWinFont );
        InvalidateControl();
    }
    Control::DataChanged( rDCEvt );
}

void DialControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        GrabFocus();
        CaptureMouse();
        mpImpl->mnOldAngle = mpImpl->mnAngle;
        HandleMouseEvent( rMEvt.GetPosPixel(), true );
    }
    Control::MouseButtonDown( rMEvt );
}

void DialControl::MouseMove( const MouseEvent& rMEvt )
{
    if( IsMouseCaptured() && rMEvt.IsLeft() )
        HandleMouseEvent( rMEvt.GetPosPixel(), false );
    Control::MouseMove(rMEvt );
}

void DialControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( IsMouseCaptured() )
    {
        ReleaseMouse();
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->GrabFocus();
    }
    Control::MouseButtonUp( rMEvt );
}

void DialControl::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKCode = rKEvt.GetKeyCode();
    if( !rKCode.GetModifier() && (rKCode.GetCode() == KEY_ESCAPE) )
        HandleEscapeEvent();
    else
        Control::KeyInput( rKEvt );
}

void DialControl::LoseFocus()
{
    // release captured mouse
    HandleEscapeEvent();
    Control::LoseFocus();
}

bool DialControl::HasRotation() const
{
    return !mpImpl->mbNoRot;
}

void DialControl::SetNoRotation()
{
    if( !mpImpl->mbNoRot )
    {
        mpImpl->mbNoRot = true;
        InvalidateControl();
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->SetText( String() );
    }
}

sal_Int32 DialControl::GetRotation() const
{
    return mpImpl->mnAngle;
}

void DialControl::SetRotation( sal_Int32 nAngle )
{
    ImplSetRotation( nAngle, false );
}

void DialControl::SetLinkedField( NumericField* pField )
{
    // remove modify handler from old linked field
    ImplSetFieldLink( Link() );
    // remember the new linked field
    mpImpl->mpLinkField = pField;
    // set modify handler at new linked field
    ImplSetFieldLink( LINK( this, DialControl, LinkedFieldModifyHdl ) );
}

void DialControl::SaveValue()
{
    mpImpl->mnInitialAngle = mpImpl->mnAngle;
}

bool DialControl::IsValueModified()
{
    return mpImpl->mnInitialAngle != mpImpl->mnAngle;
}

// private --------------------------------------------------------------------

void DialControl::Init( const Size& rWinSize, const Font& rWinFont )
{
    mpImpl->Init( rWinSize, rWinFont );
    EnableRTL( sal_False ); // don't mirror mouse handling
    SetOutputSizePixel( mpImpl->maWinSize );
    SetBackground();
}

void DialControl::Init( const Size& rWinSize )
{
    Font aFont( OutputDevice::GetDefaultFont(
        DEFAULTFONT_UI_SANS, Application::GetSettings().GetUILanguage(), DEFAULTFONT_FLAGS_ONLYONE ) );
    Init( rWinSize, aFont );
}

void DialControl::InvalidateControl()
{
    mpImpl->maBmpBuffered.CopyBackground( IsEnabled() ? mpImpl->maBmpEnabled : mpImpl->maBmpDisabled );
    if( !mpImpl->mbNoRot )
        mpImpl->maBmpBuffered.DrawElements( GetText(), mpImpl->mnAngle );
    Invalidate();
}

void DialControl::ImplSetRotation( sal_Int32 nAngle, bool bBroadcast )
{
    bool bOldSel = mpImpl->mbNoRot;
    mpImpl->mbNoRot = false;

    while( nAngle < 0 ) nAngle += 36000;
    nAngle = (((nAngle + 50) / 100) * 100) % 36000;
    if( !bOldSel || (mpImpl->mnAngle != nAngle) )
    {
        mpImpl->mnAngle = nAngle;
        InvalidateControl();
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->SetValue( static_cast< long >( GetRotation() / 100 ) );
        if( bBroadcast )
            mpImpl->maModifyHdl.Call( this );
    }
}

void DialControl::ImplSetFieldLink( const Link& rLink )
{
    if( mpImpl->mpLinkField )
    {
        NumericField& rField = *mpImpl->mpLinkField;
        rField.SetModifyHdl( rLink );
        rField.SetUpHdl( rLink );
        rField.SetDownHdl( rLink );
        rField.SetFirstHdl( rLink );
        rField.SetLastHdl( rLink );
        rField.SetLoseFocusHdl( rLink );
    }
}

void DialControl::HandleMouseEvent( const Point& rPos, bool bInitial )
{
    long nX = rPos.X() - mpImpl->mnCenterX;
    long nY = mpImpl->mnCenterY - rPos.Y();
    double fH = sqrt( static_cast< double >( nX ) * nX + static_cast< double >( nY ) * nY );
    if( fH != 0.0 )
    {
        double fAngle = acos( nX / fH );
        sal_Int32 nAngle = static_cast< sal_Int32 >( fAngle / F_PI180 * 100.0 );
        if( nY < 0 )
            nAngle = 36000 - nAngle;
        if( bInitial )  // round to entire 15 degrees
            nAngle = ((nAngle + 750) / 1500) * 1500;
        ImplSetRotation( nAngle, true );
    }
}

void DialControl::HandleEscapeEvent()
{
    if( IsMouseCaptured() )
    {
        ReleaseMouse();
        ImplSetRotation( mpImpl->mnOldAngle, true );
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->GrabFocus();
    }
}

IMPL_LINK( DialControl, LinkedFieldModifyHdl, NumericField*, pField )
{
    if( pField )
        ImplSetRotation( static_cast< sal_Int32 >( pField->GetValue() * 100 ), false );
    return 0;
}

// ============================================================================

DialControlWrapper::DialControlWrapper( DialControl& rDial ) :
    SingleControlWrapperType( rDial )
{
}

bool DialControlWrapper::IsControlDontKnow() const
{
    return !GetControl().HasRotation();
}

void DialControlWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet )
        GetControl().SetNoRotation();
}

sal_Int32 DialControlWrapper::GetControlValue() const
{
    return GetControl().GetRotation();
}

void DialControlWrapper::SetControlValue( sal_Int32 nValue )
{
    GetControl().SetRotation( nValue );
}

// ============================================================================

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
