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

#include <svx/dialcontrol.hxx>
#include <cmath>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>

namespace svx {

const tools::Long DIAL_OUTER_WIDTH = 8;

DialControlBmp::DialControlBmp(OutputDevice& rReference)
    : VirtualDevice(rReference, DeviceFormat::DEFAULT, DeviceFormat::DEFAULT)
    , mbEnabled(true)
    , mrParent(rReference)
    , mnCenterX(0)
    , mnCenterY(0)
{
    EnableRTL(false);
}

void DialControlBmp::InitBitmap(const vcl::Font& rFont)
{
    Init();
    SetFont(rFont);
}

void DialControlBmp::CopyBackground( const DialControlBmp& rSrc )
{
    Init();
    SetSize(rSrc.maRect.GetSize());
    mbEnabled = rSrc.mbEnabled;
    Point aPos;
    DrawBitmapEx( aPos, rSrc.GetBitmapEx( aPos, maRect.GetSize() ) );
}

void DialControlBmp::DrawBackground( const Size& rSize, bool bEnabled )
{
    Init();
    SetSize(rSize);
    mbEnabled = bEnabled;
    DrawBackground();
}

void DialControlBmp::DrawElements( const OUString& rText, Degree100 nAngle )
{
    double fAngle = toRadians(nAngle);
    double fSin = sin( fAngle );
    double fCos = cos( fAngle );
    double fWidth = GetTextWidth( rText ) / 2.0;
    double fHeight = GetTextHeight() / 2.0;

    if ( !rText.isEmpty() )
    {
        // rotated text
        vcl::Font aFont( GetFont() );
        aFont.SetColor( GetTextColor() );
        aFont.SetOrientation( toDegree10(nAngle) );  // Font uses 1/10 degrees
        aFont.SetWeight( WEIGHT_BOLD );
        SetFont( aFont );

        tools::Long nX = static_cast< tools::Long >( mnCenterX - fWidth * fCos - fHeight * fSin );
        tools::Long nY = static_cast< tools::Long >( mnCenterY + fWidth * fSin - fHeight * fCos );
        tools::Rectangle aRect( nX, nY, 2 * mnCenterX - nX, 2 * mnCenterY - nY );
        DrawText( aRect, rText, mbEnabled ? DrawTextFlags::NONE : DrawTextFlags::Disable );
    }
    else
    {
        // only a line
        const sal_Int32 nDx (fCos * (maRect.GetWidth()-4) / 2);
        const sal_Int32 nDy (-fSin * (maRect.GetHeight()-4) / 2);
        Point pt1( maRect.Center() );
        Point pt2( pt1.X() + nDx, pt1.Y() + nDy);

        SetLineColor( GetTextColor() );
        DrawLine( pt1, pt2 );
    }

    // *** drag button ***

    bool bMain = (nAngle % 4500_deg100) != 0_deg100;
    SetLineColor( GetButtonLineColor() );
    SetFillColor( GetButtonFillColor( bMain ) );

    tools::Long nX = mnCenterX - static_cast< tools::Long >( (DIAL_OUTER_WIDTH / 2 - mnCenterX) * fCos );
    tools::Long nY = mnCenterY - static_cast< tools::Long >( (mnCenterY - DIAL_OUTER_WIDTH / 2) * fSin );
    tools::Long nSize = bMain ? (DIAL_OUTER_WIDTH / 4) : (DIAL_OUTER_WIDTH / 2 - 1);
    DrawEllipse( tools::Rectangle( nX - nSize, nY - nSize, nX + nSize, nY + nSize ) );
}

Color DialControlBmp::GetBackgroundColor() const
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

void DialControlBmp::Init()
{
    SetSettings(mrParent.GetSettings());
    SetBackground(GetBackgroundColor());
}

void DialControlBmp::SetSize( const Size& rSize )
{
    maRect.SetPos( Point( 0, 0 ) );
    maRect.SetSize( rSize );
    mnCenterX = rSize.Width() / 2;
    mnCenterY = rSize.Height() / 2;
    SetOutputSize( rSize );
}

void DialControlBmp::DrawBackground()
{
    // *** background with 3D effect ***

    SetLineColor();
    SetFillColor();
    Erase();

    EnableRTL(); // draw 3D effect in correct direction

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

    EnableRTL( false );

    // *** calibration ***

    Point aStartPos( mnCenterX, mnCenterY );
    Color aFullColor( GetScaleLineColor() );
    Color aLightColor( GetBackgroundColor() );
    aLightColor.Merge( aFullColor, 128 );

    for( int nAngle = 0; nAngle < 360; nAngle += 15 )
    {
        SetLineColor( (nAngle % 45) ? aLightColor : aFullColor );
        double fAngle = basegfx::deg2rad(nAngle);
        tools::Long nX = static_cast< tools::Long >( -mnCenterX * cos( fAngle ) );
        tools::Long nY = static_cast< tools::Long >( mnCenterY * sin( fAngle ) );
        DrawLine( aStartPos, Point( mnCenterX - nX, mnCenterY - nY ) );
    }

    // *** clear inner area ***

    SetLineColor();
    SetFillColor( GetBackgroundColor() );
    tools::Rectangle aEllipseRect = maRect;
    aEllipseRect.shrink(DIAL_OUTER_WIDTH);
    DrawEllipse( aEllipseRect );
}

DialControl::DialControl_Impl::DialControl_Impl(OutputDevice& rReference) :
    mxBmpEnabled(VclPtr<DialControlBmp>::Create(rReference)),
    mxBmpDisabled(VclPtr<DialControlBmp>::Create(rReference)),
    mxBmpBuffered(VclPtr<DialControlBmp>::Create(rReference)),
    mpLinkField( nullptr ),
    mnLinkedFieldValueMultiplyer( 0 ),
    mnAngle( 0 ),
    mnInitialAngle( 0 ),
    mnOldAngle( 0 ),
    mnCenterX( 0 ),
    mnCenterY( 0 ),
    mbNoRot( false )
{
}

void DialControl::DialControl_Impl::Init( const Size& rWinSize, const vcl::Font& rWinFont )
{
    maWinFont = rWinFont;
    maWinFont.SetTransparent(true);
    mxBmpBuffered->InitBitmap(maWinFont);
    SetSize(rWinSize);
}

void DialControl::DialControl_Impl::SetSize( const Size& rWinSize )
{
    // make the control squared, and adjusted so that we have a well-defined
    // center ["(x - 1) | 1" creates odd value <= x]
    tools::Long nMin = (std::min(rWinSize.Width(), rWinSize.Height()) - 1) | 1;

    maWinSize = Size( nMin, nMin );

    mnCenterX = maWinSize.Width() / 2;
    mnCenterY = maWinSize.Height() / 2;

    mxBmpEnabled->DrawBackground( maWinSize, true );
    mxBmpDisabled->DrawBackground( maWinSize, false );
    mxBmpBuffered->SetSize( maWinSize );
}

void DialControl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    //use same logic as DialControl_Impl::SetSize
    int nDim = (std::min<int>(pDrawingArea->get_approximate_digit_width() * 12,
                              pDrawingArea->get_text_height() * 6) - 1) | 1;
    Size aSize(nDim, nDim);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    mpImpl.reset(new DialControl_Impl(pDrawingArea->get_ref_device()));
    //set size and use that
    Init(aSize);
}

void DialControl::Resize()
{
    mpImpl->SetSize(GetOutputSizePixel());
    InvalidateControl();
}

void DialControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    Point aPos;
    rRenderContext.DrawBitmapEx(aPos, mpImpl->mxBmpBuffered->GetBitmapEx(aPos, mpImpl->maWinSize));
}

void DialControl::StyleUpdated()
{
    CustomWidgetController::StyleUpdated();
    Init( mpImpl->maWinSize, mpImpl->maWinFont );
    InvalidateControl();
}

bool DialControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if( rMEvt.IsLeft() )
    {
        GrabFocus();
        CaptureMouse();
        mpImpl->mnOldAngle = mpImpl->mnAngle;
        HandleMouseEvent( rMEvt.GetPosPixel(), true );
    }
    return true;
}

bool DialControl::MouseMove( const MouseEvent& rMEvt )
{
    if( IsMouseCaptured() && rMEvt.IsLeft() )
        HandleMouseEvent( rMEvt.GetPosPixel(), false );
    return true;
}

bool DialControl::MouseButtonUp(const MouseEvent&)
{
    if( IsMouseCaptured() )
    {
        ReleaseMouse();
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->grab_focus();
    }
    return true;
}

bool DialControl::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKCode = rKEvt.GetKeyCode();
    if( !rKCode.GetModifier() && (rKCode.GetCode() == KEY_ESCAPE) )
    {
        HandleEscapeEvent();
        return true;
    }
    return CustomWidgetController::KeyInput(rKEvt);
}

void DialControl::LoseFocus()
{
    // release captured mouse
    HandleEscapeEvent();
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
        if (mpImpl->mpLinkField)
            mpImpl->mpLinkField->set_text("");
    }
}

Degree100 DialControl::GetRotation() const
{
    return mpImpl->mnAngle;
}

void DialControl::SetRotation(Degree100 nAngle)
{
    SetRotation(nAngle, false);
}

void DialControl::SetLinkedField(weld::MetricSpinButton* pField, sal_Int32 nDecimalPlaces)
{
    mpImpl->mnLinkedFieldValueMultiplyer = 100 / std::pow(10.0, double(nDecimalPlaces));

    // remove modify handler from old linked field
    if( mpImpl->mpLinkField )
    {
        weld::MetricSpinButton& rField = *mpImpl->mpLinkField;
        rField.connect_value_changed(Link<weld::MetricSpinButton&,void>());
    }
    // remember the new linked field
    mpImpl->mpLinkField = pField;
    // set modify handler at new linked field
    if( mpImpl->mpLinkField )
    {
        weld::MetricSpinButton& rField = *mpImpl->mpLinkField;
        rField.connect_value_changed(LINK(this, DialControl, LinkedFieldModifyHdl));
    }
}

IMPL_LINK_NOARG(DialControl, LinkedFieldModifyHdl, weld::MetricSpinButton&, void)
{
    SetRotation(Degree100(mpImpl->mpLinkField->get_value(FieldUnit::DEGREE) * mpImpl->mnLinkedFieldValueMultiplyer), true);
}

void DialControl::SaveValue()
{
    mpImpl->mnInitialAngle = mpImpl->mnAngle;
}

bool DialControl::IsValueModified() const
{
    return mpImpl->mnInitialAngle != mpImpl->mnAngle;
}

void DialControl::Init( const Size& rWinSize, const vcl::Font& rWinFont )
{
    mpImpl->Init( rWinSize, rWinFont );
    EnableRTL( false ); // don't mirror mouse handling
    SetOutputSizePixel( mpImpl->maWinSize );
}

void DialControl::Init( const Size& rWinSize )
{
    //hidpi TODO: GetDefaultFont() picks a font size too small, so fix it here.
    vcl::Font aDefaultSize = Application::GetSettings().GetStyleSettings().GetLabelFont();

    vcl::Font aFont( OutputDevice::GetDefaultFont(
        DefaultFontType::UI_SANS, Application::GetSettings().GetUILanguageTag().getLanguageType(), GetDefaultFontFlags::OnlyOne ) );

    aFont.SetFontHeight(aDefaultSize.GetFontHeight());
    Init( rWinSize, aFont );
}

void DialControl::InvalidateControl()
{
    mpImpl->mxBmpBuffered->CopyBackground( IsEnabled() ? *mpImpl->mxBmpEnabled : *mpImpl->mxBmpDisabled );
    if( !mpImpl->mbNoRot )
        mpImpl->mxBmpBuffered->DrawElements(GetText(), mpImpl->mnAngle);
    Invalidate();
}

void DialControl::SetRotation(Degree100 nAngle, bool bBroadcast)
{
    bool bOldSel = mpImpl->mbNoRot;
    mpImpl->mbNoRot = false;

    nAngle = NormAngle36000(nAngle);

    if (!bOldSel || (mpImpl->mnAngle != nAngle))
    {
        mpImpl->mnAngle = nAngle;
        InvalidateControl();
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->set_value(GetRotation().get() / mpImpl->mnLinkedFieldValueMultiplyer, FieldUnit::DEGREE);
        if( bBroadcast )
            mpImpl->maModifyHdl.Call(*this);
    }
}

void DialControl::SetModifyHdl( const Link<DialControl&,void>& rLink )
{
    mpImpl->maModifyHdl = rLink;
}

void DialControl::HandleMouseEvent( const Point& rPos, bool bInitial )
{
    tools::Long nX = rPos.X() - mpImpl->mnCenterX;
    tools::Long nY = mpImpl->mnCenterY - rPos.Y();
    double fH = sqrt( static_cast< double >( nX ) * nX + static_cast< double >( nY ) * nY );
    if( fH != 0.0 )
    {
        double fAngle = acos( nX / fH );
        sal_Int32 nAngle = static_cast<sal_Int32>(basegfx::rad2deg(fAngle) * 100.0);
        if( nY < 0 )
            nAngle = 36000 - nAngle;
        if( bInitial )  // round to entire 15 degrees
            nAngle = ((nAngle + 750) / 1500) * 1500;
        // Round up to 1 degree
        nAngle = (((nAngle + 50) / 100) * 100) % 36000;
        SetRotation(Degree100(nAngle), true);
    }
}

void DialControl::HandleEscapeEvent()
{
    if( IsMouseCaptured() )
    {
        ReleaseMouse();
        SetRotation(mpImpl->mnOldAngle, true);
        if( mpImpl->mpLinkField )
            mpImpl->mpLinkField->grab_focus();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
