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

#include <config_features.h>

#include <vcl/salnativewidgets.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>

#include <quartz/salgdi.h>
#include <osx/salnativewidgets.h>
#include <osx/saldata.hxx>
#include <osx/salframe.h>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include "cuidraw.hxx"

#if !HAVE_FEATURE_MACOSX_SANDBOX

@interface NSWindow(CoreUIRendererPrivate)
+ (CUIRendererRef)coreUIRenderer;
@end

#endif

// Helper returns an HIRect

static HIRect ImplGetHIRectFromRectangle(tools::Rectangle aRect)
{
    HIRect aHIRect;
    aHIRect.origin.x = static_cast<float>(aRect.Left());
    aHIRect.origin.y = static_cast<float>(aRect.Top());
    aHIRect.size.width = static_cast<float>(aRect.GetWidth());
    aHIRect.size.height = static_cast<float>(aRect.GetHeight());
    return aHIRect;
}

static ThemeButtonValue ImplGetButtonValue( ButtonValue aButtonValue )
{
    switch( aButtonValue )
    {
        case ButtonValue::On:
            return kThemeButtonOn;
            break;

        case ButtonValue::Off:
        case ButtonValue::DontKnow:
            return kThemeButtonOff;
            break;

        case ButtonValue::Mixed:
        default:
            return kThemeButtonMixed;
            break;
    }
}

static bool AquaGetScrollRect( /* TODO: int nScreen, */  ControlPart nPart,
    const tools::Rectangle& rControlRect, tools::Rectangle& rResultRect )
{
    bool bRetVal = true;
    rResultRect = rControlRect;

    switch( nPart )
    {
        case ControlPart::ButtonUp:
            rResultRect.SetBottom( rResultRect.Top() );
            break;

        case ControlPart::ButtonDown:
            rResultRect.SetTop( rResultRect.Bottom() );
            break;

        case ControlPart::ButtonLeft:
            rResultRect.SetRight( rResultRect.Left() );
            break;

        case ControlPart::ButtonRight:
            rResultRect.SetLeft( rResultRect.Right() );
            break;

        case ControlPart::TrackHorzArea:
        case ControlPart::TrackVertArea:
        case ControlPart::ThumbHorz:
        case ControlPart::ThumbVert:
        case ControlPart::TrackHorzLeft:
        case ControlPart::TrackHorzRight:
        case ControlPart::TrackVertUpper:
        case ControlPart::TrackVertLower:
            break;
        default:
            bRetVal = false;
    }

    return bRetVal;
}

bool AquaSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    // Native controls are now defaults
    // If you want to disable experimental native controls code,
    // just set the environment variable SAL_NO_NWF to something
    // and vcl controls will be used as default again.

    switch( nType )
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
        case ControlType::ListNode:
            if( nPart == ControlPart::Entire )
                return true;
            break;

        case ControlType::Scrollbar:
            if( nPart == ControlPart::DrawBackgroundHorz ||
                nPart == ControlPart::DrawBackgroundVert ||
                nPart == ControlPart::Entire       ||
                nPart == ControlPart::HasThreeButtons )
                return true;
            break;

        case ControlType::Slider:
            if( nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea )
                return true;
            break;

        case ControlType::Editbox:
            if( nPart == ControlPart::Entire ||
                nPart == ControlPart::HasBackgroundTexture )
                return true;
            break;

        case ControlType::MultilineEditbox:
            if( nPart == ControlPart::Entire ||
                nPart == ControlPart::HasBackgroundTexture )
                return true;
            break;

        case ControlType::Spinbox:
            if( nPart == ControlPart::Entire ||
                nPart == ControlPart::AllButtons    ||
                nPart == ControlPart::HasBackgroundTexture )
                return true;
            break;

        case ControlType::SpinButtons:
                return false;
            break;

        case ControlType::Combobox:
            if( nPart == ControlPart::Entire ||
                nPart == ControlPart::HasBackgroundTexture )
                return true;
            break;

        case ControlType::Listbox:
            if( nPart == ControlPart::Entire ||
                nPart == ControlPart::ListboxWindow ||
                nPart == ControlPart::HasBackgroundTexture ||
                nPart == ControlPart::SubEdit
                )
                return true;
            break;

        case ControlType::TabItem:
        case ControlType::TabPane:
        case ControlType::TabBody:  // see vcl/source/window/tabpage.cxx
            if( nPart == ControlPart::Entire ||
                nPart == ControlPart::TabsDrawRtl ||
                nPart == ControlPart::HasBackgroundTexture )
                return true;
            break;

        // when ControlPart::Button is used, toolbar icons are not highlighted when mouse rolls over.
        // More Aqua compliant
        case ControlType::Toolbar:
            if( nPart == ControlPart::Entire       ||
                nPart == ControlPart::DrawBackgroundHorz ||
                nPart == ControlPart::DrawBackgroundVert)
                return true;
            break;

        case  ControlType::WindowBackground:
            if ( nPart == ControlPart::BackgroundWindow ||
                 nPart == ControlPart::BackgroundDialog )
                 return true;
            break;

        case ControlType::Menubar:
            if( nPart == ControlPart::Entire )
                return true;
            break;

        case ControlType::Tooltip: // ** TO DO
            if (nPart == ControlPart::Entire)
                return true;
            break;

        case ControlType::MenuPopup:
            if( nPart == ControlPart::Entire       ||
                nPart == ControlPart::MenuItem            ||
                nPart == ControlPart::MenuItemCheckMark ||
                nPart == ControlPart::MenuItemRadioMark)
                return true;
            break;
        case ControlType::Progress:
        case ControlType::IntroProgress:
            if( nPart == ControlPart::Entire )
                return true;
            break;
        case ControlType::Frame:
            if( nPart == ControlPart::Border )
                return true;
            break;
        case ControlType::ListNet:
            if( nPart == ControlPart::Entire )
                return true;
            break;
        default: break;
    }

    return false;
}

bool AquaSalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                        const Point& rPos, bool& rIsInside )
{
    if ( nType == ControlType::Scrollbar )
    {
        tools::Rectangle aRect;
        bool bValid = AquaGetScrollRect( /* TODO: m_nScreen */ nPart, rControlRegion, aRect );
        rIsInside = bValid && aRect.IsInside( rPos );
        return bValid;
    }  //  ControlType::Scrollbar

    return false;
}

/*
  kThemeStateInactive = 0,
   kThemeStateActive = 1,
   kThemeStatePressed = 2,
   kThemeStateRollover = 6,
   kThemeStateUnavailable = 7,
   kThemeStateUnavailableInactive = 8
   kThemeStatePressedUp = 2,
   kThemeStatePressedDown = 3
*/
UInt32 AquaSalGraphics::getState( ControlState nState )
{
    const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow];
    if( !(nState & ControlState::ENABLED) || ! bDrawActive )
    {
        return kThemeStateInactive;
    }

    if( nState & ControlState::PRESSED )
        return kThemeStatePressed;

    return kThemeStateActive;
}

UInt32 AquaSalGraphics::getTrackState( ControlState nState )
{
    const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow];
    if( ! (nState & ControlState::ENABLED) || ! bDrawActive )
            return kThemeTrackInactive;

    return kThemeTrackActive;
}

bool AquaSalGraphics::drawNativeControl(ControlType nType,
                    ControlPart nPart,
                    const tools::Rectangle& rControlRegion,
                    ControlState nState,
                    const ImplControlValue& aValue,
                    const OUString& )
{
    bool bOK = false;

    if( ! CheckContext() )
        return false;

    maContextHolder.saveState();

    tools::Rectangle buttonRect = rControlRegion;
    HIRect rc = ImplGetHIRectFromRectangle(buttonRect);

    switch( nType )
    {

    case  ControlType::Combobox:
        if ( nPart == ControlPart::HasBackgroundTexture ||
             nPart == ControlPart::Entire )
        {
            HIThemeButtonDrawInfo aComboInfo;
            aComboInfo.version = 0;
            aComboInfo.kind = kThemeComboBox;
            aComboInfo.state = getState( nState );
            aComboInfo.value = kThemeButtonOn;
            aComboInfo.adornment = kThemeAdornmentNone;

            if( nState & ControlState::FOCUSED )
                aComboInfo.adornment |= kThemeAdornmentFocus;

            HIThemeDrawButton(&rc, &aComboInfo, maContextHolder.get(), kHIThemeOrientationNormal,&rc);
            bOK = true;
        }
        break;

    case ControlType::Toolbar:
        {
#if HAVE_FEATURE_MACOSX_SANDBOX
            HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
            aMenuItemDrawInfo.version = 0;
            aMenuItemDrawInfo.state = kThemeMenuActive;
            aMenuItemDrawInfo.itemType = kThemeMenuItemHierBackground;
            HIThemeDrawMenuItem(&rc,&rc,&aMenuItemDrawInfo,maContextHolder.get(),kHIThemeOrientationNormal,NULL);
#else
            if (rControlRegion.Top() == 0 && nPart == ControlPart::DrawBackgroundHorz)
            {
                const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow];
                CGFloat unifiedHeight = rControlRegion.GetHeight();
                CGRect drawRect = CGRectMake(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.GetWidth(), rControlRegion.GetHeight());
                CUIDraw([NSWindow coreUIRenderer], drawRect, maContextHolder.get(),
                        reinterpret_cast<CFDictionaryRef>([NSDictionary dictionaryWithObjectsAndKeys:
                        @"kCUIWidgetWindowFrame", @"widget",
                        @"regularwin", @"windowtype",
                        (bDrawActive ? @"normal" : @"inactive"), @"state",
                        [NSNumber numberWithDouble:unifiedHeight], @"kCUIWindowFrameUnifiedTitleBarHeightKey",
                        [NSNumber numberWithBool:NO], @"kCUIWindowFrameDrawTitleSeparatorKey",
                        [NSNumber numberWithBool:YES], @"is.flipped",
                        nil]),
                        nil);
            }
            else
            {
                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
                aMenuItemDrawInfo.version = 0;
                aMenuItemDrawInfo.state = kThemeMenuActive;
                aMenuItemDrawInfo.itemType = kThemeMenuItemHierBackground;
                HIThemeDrawMenuItem(&rc, &rc, &aMenuItemDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
            }
#endif
            bOK = true;
        }
        break;

        case ControlType::WindowBackground:
        {
            HIThemeBackgroundDrawInfo aThemeBackgroundInfo;
            aThemeBackgroundInfo.version = 0;
            aThemeBackgroundInfo.state = getState( nState );
            aThemeBackgroundInfo.kind = kThemeBrushDialogBackgroundActive;
            // FIXME: without this magical offset there is a 2 pixel black border on the right and bottom
            rc.size.width += 2;
            rc.size.height += 2;

            HIThemeApplyBackground( &rc, &aThemeBackgroundInfo, maContextHolder.get(), kHIThemeOrientationNormal);
            CGContextFillRect( maContextHolder.get(), rc );
            bOK = true;
        }
        break;

        case ControlType::Tooltip:
        {
            HIThemeBackgroundDrawInfo aThemeBackgroundInfo;
            aThemeBackgroundInfo.version = 0;
            aThemeBackgroundInfo.state = getState( nState );
            aThemeBackgroundInfo.kind = kThemeBrushAlertBackgroundActive;
            rc.size.width += 2;
            rc.size.height += 2;

            HIThemeApplyBackground( &rc, &aThemeBackgroundInfo, maContextHolder.get(), kHIThemeOrientationNormal);
            CGContextFillRect( maContextHolder.get(), rc );
            bOK = true;
        }
        break;

    case ControlType::Menubar:
    case ControlType::MenuPopup:
        {
            if ((nPart == ControlPart::Entire) || (nPart == ControlPart::MenuItem )|| (nPart == ControlPart::HasBackgroundTexture ))
            {
                // FIXME: without this magical offset there is a 2 pixel black border on the right
                rc.size.width += 2;

                HIThemeMenuDrawInfo aMenuInfo;
                aMenuInfo.version = 0;
                aMenuInfo.menuType = kThemeMenuTypePullDown;

                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
                // the Aqua grey theme when the item is selected is drawn here.
                aMenuItemDrawInfo.itemType = kThemeMenuItemPlain;

                if ((nPart == ControlPart::MenuItem ) && (nState & ControlState::SELECTED))
                {
                    // the blue theme when the item is selected is drawn here.
                    aMenuItemDrawInfo.state = kThemeMenuSelected;
                }
                else
                {
                    // normal color for non selected item
                    aMenuItemDrawInfo.state = kThemeMenuActive;
                }

                // repaints the background of the pull down menu
                HIThemeDrawMenuBackground(&rc,&aMenuInfo,maContextHolder.get(),kHIThemeOrientationNormal);

                // repaints the item either blue (selected) and/or Aqua grey (active only)
                HIThemeDrawMenuItem(&rc,&rc,&aMenuItemDrawInfo,maContextHolder.get(),kHIThemeOrientationNormal,&rc);

                bOK = true;
            }
            else if(( nPart == ControlPart::MenuItemCheckMark )||( nPart == ControlPart::MenuItemRadioMark )) {
                if( nState & ControlState::PRESSED ) {//checked, else it is not displayed (see vcl/source/window/menu.cxx)
                    HIThemeTextInfo aTextInfo;
                    aTextInfo.version = 0;
                    aTextInfo.state = (nState & ControlState::ENABLED) ? kThemeStateInactive: kThemeStateActive;
                    aTextInfo.fontID = kThemeMenuItemMarkFont;
                    aTextInfo.horizontalFlushness=kHIThemeTextHorizontalFlushCenter;
                    aTextInfo.verticalFlushness=kHIThemeTextVerticalFlushTop;
                    aTextInfo.options=kHIThemeTextBoxOptionNone;
                    aTextInfo.truncationPosition=kHIThemeTextTruncationNone;
                    //aTextInfo.truncationMaxLines unused because of kHIThemeTextTruncationNone

                    if( nState & ControlState::SELECTED) aTextInfo.state = kThemeStatePressed; //item highlighted

                    UniChar mark=( nPart == ControlPart::MenuItemCheckMark ) ? kCheckUnicode: kBulletUnicode;//0x2713;
                    CFStringRef cfString = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, &mark, 1, kCFAllocatorNull);
                    HIThemeDrawTextBox(cfString, &rc, &aTextInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                    if (cfString)
                        CFRelease(cfString);

                    bOK = true;
                }
            }
        }
        break;

    case ControlType::Pushbutton:
        {
            // [ FIXME] : instead of use a value, vcl can retrieve correct values on the fly (to be implemented)
            const int PB_Mini_Height = 15;
            const int PB_Norm_Height = 21;

            HIThemeButtonDrawInfo aPushInfo;
            aPushInfo.version = 0;

            // no animation
            aPushInfo.animation.time.start = 0;
            aPushInfo.animation.time.current = 0;
            PushButtonValue const * pPBVal = aValue.getType() == ControlType::Pushbutton ? static_cast<PushButtonValue const *>(&aValue) : nullptr;
            int nPaintHeight = static_cast<int>(rc.size.height);

            if( pPBVal && pPBVal->mbBevelButton )
            {
                aPushInfo.kind = kThemeRoundedBevelButton;
            }
            else if( rc.size.height <= PB_Norm_Height )
            {
                aPushInfo.kind = kThemePushButtonMini;
                nPaintHeight = PB_Mini_Height;
            }
            else if( (pPBVal && pPBVal->mbSingleLine) || rc.size.height < (PB_Norm_Height + PB_Norm_Height/2) )
            {
                aPushInfo.kind = kThemePushButtonNormal;
                nPaintHeight = PB_Norm_Height;

                // avoid clipping when focused
                rc.origin.x += FOCUS_RING_WIDTH/2;
                rc.size.width -= FOCUS_RING_WIDTH;
            }
            else
                aPushInfo.kind = kThemeBevelButton;

            // translate the origin for controls with fixed paint height
            // so content ends up somewhere sensible
            int delta_y = static_cast<int>(rc.size.height) - nPaintHeight;
            rc.origin.y += delta_y/2;

            aPushInfo.state = getState( nState );
            aPushInfo.value = ImplGetButtonValue( aValue.getTristateVal() );

            aPushInfo.adornment = ( nState & ControlState::DEFAULT ) ?
               kThemeAdornmentDefault :
               kThemeAdornmentNone;
            if( nState & ControlState::FOCUSED )
                aPushInfo.adornment |= kThemeAdornmentFocus;

            HIThemeDrawButton( &rc, &aPushInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr );
            bOK = true;
        }
        break;

    case ControlType::Radiobutton:
    case ControlType::Checkbox:
        {
            HIThemeButtonDrawInfo aInfo;
            aInfo.version = 0;
            switch( nType )
            {
            case ControlType::Radiobutton: if(rc.size.width >= BUTTON_HEIGHT) aInfo.kind = kThemeRadioButton;
                                    else aInfo.kind = kThemeSmallRadioButton;
                break;
            case ControlType::Checkbox:   if(rc.size.width >= BUTTON_HEIGHT) aInfo.kind = kThemeCheckBox;
                                    else aInfo.kind = kThemeSmallCheckBox;
                break;
            default: break;
            }

            aInfo.state = getState( nState );

            ButtonValue aButtonValue = aValue.getTristateVal();
            aInfo.value = ImplGetButtonValue( aButtonValue );

            aInfo.adornment = ( nState & ControlState::DEFAULT ) ?
              kThemeAdornmentDefault :
              kThemeAdornmentNone;
            if( nState & ControlState::FOCUSED )
                aInfo.adornment |= kThemeAdornmentFocus;
            HIThemeDrawButton( &rc, &aInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr );
            bOK = true;
        }
        break;

    case ControlType::ListNode:
        {
            ButtonValue aButtonValue = aValue.getTristateVal();

            if( AllSettings::GetLayoutRTL() && aButtonValue == ButtonValue::Off )
            {
                // FIXME: a value of kThemeDisclosureLeft
                // should draw a theme compliant left disclosure triangle
                // sadly this does not seem to work, so we'll draw a left
                // grey equilateral triangle here ourselves.
                // Perhaps some other HIThemeButtonDrawInfo setting would do the trick ?

                CGContextSetShouldAntialias( maContextHolder.get(), true );
                CGFloat const aGrey[] = { 0.45, 0.45, 0.45, 1.0 };
                CGContextSetFillColor( maContextHolder.get(), aGrey );
                CGContextBeginPath( maContextHolder.get() );
                float x = rc.origin.x + rc.size.width;
                float y = rc.origin.y;
                CGContextMoveToPoint( maContextHolder.get(), x, y );
                y += rc.size.height;
                CGContextAddLineToPoint( maContextHolder.get(), x, y );
                x -= rc.size.height * 0.866; // cos( 30 degree ) is approx. 0.866
                y -= rc.size.height/2;
                CGContextAddLineToPoint( maContextHolder.get(), x, y );
                CGContextDrawPath( maContextHolder.get(), kCGPathEOFill );
            }
            else
            {
                HIThemeButtonDrawInfo aInfo;
                aInfo.version = 0;
                aInfo.kind = kThemeDisclosureTriangle;
                aInfo.value = kThemeDisclosureRight;
                aInfo.state = getState( nState );

                aInfo.adornment = kThemeAdornmentNone;

                switch( aButtonValue ) {
                    case ButtonValue::On: aInfo.value = kThemeDisclosureDown;//expanded
                        break;
                    case ButtonValue::Off:
                        // FIXME: this should have drawn a theme compliant disclosure triangle
                        // (see above)
                        if( AllSettings::GetLayoutRTL() )
                        {
                            aInfo.value = kThemeDisclosureLeft;//collapsed, RTL
                        }
                        break;
                    case ButtonValue::DontKnow: //what to do?
                    default:
                        break;
                }

                HIThemeDrawButton( &rc, &aInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr );
            }
            bOK = true;
        }
        break;

    case ControlType::Progress:
    case ControlType::IntroProgress:
        {
            long nProgressWidth = aValue.getNumericVal();
            HIThemeTrackDrawInfo aTrackInfo;
            aTrackInfo.version              = 0;
            aTrackInfo.kind                 = (rc.size.height > 10) ? kThemeProgressBarLarge : kThemeProgressBarMedium;
            aTrackInfo.bounds               = rc;
            aTrackInfo.min                  = 0;
            aTrackInfo.max                  = static_cast<SInt32>(rc.size.width);
            aTrackInfo.value                = nProgressWidth;
            aTrackInfo.reserved             = 0;
            aTrackInfo.bounds.origin.y     -= 2; // FIXME: magic for shadow
            aTrackInfo.bounds.size.width   -= 2; // FIXME: magic for shadow
            aTrackInfo.attributes           = kThemeTrackHorizontal;
            if( AllSettings::GetLayoutRTL() )
                aTrackInfo.attributes      |= kThemeTrackRightToLeft;
            aTrackInfo.enableState          = getTrackState( nState );
            // the intro bitmap never gets key anyway; we want to draw that enabled
            if( nType == ControlType::IntroProgress )
                aTrackInfo.enableState          = kThemeTrackActive;
            aTrackInfo.filler1              = 0;
            aTrackInfo.trackInfo.progress.phase   = static_cast<long long>(CFAbsoluteTimeGetCurrent()*10.0);

            HIThemeDrawTrack( &aTrackInfo, nullptr, maContextHolder.get(), kHIThemeOrientationNormal );
            bOK = true;
        }
        break;

    case ControlType::Slider:
        {
            SliderValue const * pSLVal = static_cast<SliderValue const *>(&aValue);

            HIThemeTrackDrawInfo aTrackDraw;
            aTrackDraw.kind = kThemeSliderMedium;
            if( nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea )
            {
                aTrackDraw.bounds = rc;
                aTrackDraw.min   = pSLVal->mnMin;
                aTrackDraw.max   = pSLVal->mnMax;
                aTrackDraw.value = pSLVal->mnCur;
                aTrackDraw.reserved = 0;
                aTrackDraw.attributes = kThemeTrackShowThumb;
                if( nPart == ControlPart::TrackHorzArea )
                    aTrackDraw.attributes |= kThemeTrackHorizontal;
                aTrackDraw.enableState = (nState & ControlState::ENABLED)
                                         ? kThemeTrackActive : kThemeTrackInactive;

                SliderTrackInfo aSlideInfo;
                aSlideInfo.thumbDir = kThemeThumbUpward;
                aSlideInfo.pressState = 0;
                aTrackDraw.trackInfo.slider = aSlideInfo;

                HIThemeDrawTrack( &aTrackDraw, nullptr, maContextHolder.get(), kHIThemeOrientationNormal );
                bOK = true;
            }
        }
        break;

    case ControlType::Scrollbar:
        {
            const ScrollbarValue* pScrollbarVal = (aValue.getType() == ControlType::Scrollbar) ? static_cast<const ScrollbarValue*>(&aValue) : nullptr;

            if( nPart == ControlPart::DrawBackgroundVert ||
                nPart == ControlPart::DrawBackgroundHorz )
            {
                HIThemeTrackDrawInfo aTrackDraw;
                aTrackDraw.kind = kThemeMediumScrollBar;
                // FIXME: the scrollbar length must be adjusted
                if (nPart == ControlPart::DrawBackgroundVert)
                    rc.size.height += 2;
                else
                    rc.size.width += 2;

                aTrackDraw.bounds = rc;
                aTrackDraw.min = pScrollbarVal->mnMin;
                aTrackDraw.max = pScrollbarVal->mnMax - pScrollbarVal->mnVisibleSize;
                aTrackDraw.value = pScrollbarVal->mnCur;
                aTrackDraw.reserved = 0;
                aTrackDraw.attributes = kThemeTrackShowThumb;
                if( nPart == ControlPart::DrawBackgroundHorz )
                    aTrackDraw.attributes |= kThemeTrackHorizontal;
                aTrackDraw.enableState = getTrackState( nState );

                ScrollBarTrackInfo aScrollInfo;
                aScrollInfo.viewsize = pScrollbarVal->mnVisibleSize;
                aScrollInfo.pressState = 0;

                if ( pScrollbarVal->mnButton1State & ControlState::ENABLED )
                {
                    if ( pScrollbarVal->mnButton1State & ControlState::PRESSED )
                        aScrollInfo.pressState = kThemeTopOutsideArrowPressed;
                }

                if ( pScrollbarVal->mnButton2State & ControlState::ENABLED )
                {
                    if ( pScrollbarVal->mnButton2State & ControlState::PRESSED )
                        aScrollInfo.pressState = kThemeBottomOutsideArrowPressed;
                }

                if ( pScrollbarVal->mnThumbState & ControlState::ENABLED )
                {
                    if ( pScrollbarVal->mnThumbState & ControlState::PRESSED )
                        aScrollInfo.pressState = kThemeThumbPressed;
                }

                aTrackDraw.trackInfo.scrollbar = aScrollInfo;

                HIThemeDrawTrack( &aTrackDraw, nullptr, maContextHolder.get(), kHIThemeOrientationNormal );
                bOK = true;
            }
        }
        break;

    case ControlType::TabPane:
        {
            HIThemeTabPaneDrawInfo aTabPaneDrawInfo;
            aTabPaneDrawInfo.version = 1;
            aTabPaneDrawInfo.state = kThemeStateActive;
            aTabPaneDrawInfo.direction=kThemeTabNorth;
            aTabPaneDrawInfo.size=kHIThemeTabSizeNormal;
            aTabPaneDrawInfo.kind=kHIThemeTabKindNormal;

            //the border is outside the rect rc for Carbon
            //but for VCL it should be inside
            rc.origin.x+=1;
            rc.origin.y-=TAB_HEIGHT_NORMAL/2;
            rc.size.height+=TAB_HEIGHT_NORMAL/2;
            rc.size.width-=2;

            HIThemeDrawTabPane(&rc, &aTabPaneDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);

            bOK = true;
        }
        break;

    case ControlType::TabItem:
        {
            HIThemeTabDrawInfo aTabItemDrawInfo;
            aTabItemDrawInfo.version=1;
            aTabItemDrawInfo.style=kThemeTabNonFront;
            aTabItemDrawInfo.direction=kThemeTabNorth;
            aTabItemDrawInfo.size=kHIThemeTabSizeNormal;
            aTabItemDrawInfo.adornment=kHIThemeTabAdornmentTrailingSeparator;
            //State
            if(nState & ControlState::SELECTED) {
                aTabItemDrawInfo.style=kThemeTabFront;
            }
            if(nState & ControlState::FOCUSED) {
                aTabItemDrawInfo.adornment|=kHIThemeTabAdornmentFocus;
            }

            //first, last or middle tab
            aTabItemDrawInfo.position=kHIThemeTabPositionMiddle;

            TabitemValue const * pTabValue = static_cast<TabitemValue const *>(&aValue);
            TabitemFlags nAlignment = pTabValue->mnAlignment;
            //TabitemFlags::LeftAligned (and TabitemFlags::RightAligned) for the leftmost (or rightmost) tab
            //when there are several lines of tabs because there is only one first tab and one
            //last tab and TabitemFlags::FirstInGroup (and TabitemFlags::LastInGroup) because when the
            //line width is different from window width, there may not be TabitemFlags::RightAligned
            if( ( (nAlignment & TabitemFlags::LeftAligned)&&(nAlignment & TabitemFlags::RightAligned) ) ||
                ( (nAlignment & TabitemFlags::FirstInGroup)&&(nAlignment & TabitemFlags::LastInGroup) )
               ) //tab alone
                aTabItemDrawInfo.position=kHIThemeTabPositionOnly;
            else if((nAlignment & TabitemFlags::LeftAligned)||(nAlignment & TabitemFlags::FirstInGroup))
                aTabItemDrawInfo.position=kHIThemeTabPositionFirst;
            else if((nAlignment & TabitemFlags::RightAligned)||(nAlignment & TabitemFlags::LastInGroup))
                aTabItemDrawInfo.position=kHIThemeTabPositionLast;

            //support for RTL
            //see issue 79748
            if( AllSettings::GetLayoutRTL() ) {
                if( aTabItemDrawInfo.position == kHIThemeTabPositionFirst )
                        aTabItemDrawInfo.position = kHIThemeTabPositionLast;
                else if( aTabItemDrawInfo.position == kHIThemeTabPositionLast )
                        aTabItemDrawInfo.position = kHIThemeTabPositionFirst;
            }

            rc.size.width+=2;//because VCL has 2 empty pixels between 2 tabs
            rc.origin.x-=1;

            HIThemeDrawTab(&rc, &aTabItemDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal, &rc );

            bOK=true;
        }
        break;

    case  ControlType::Listbox:
        switch( nPart)
        {
            case ControlPart::Entire:
            case ControlPart::ButtonDown:
            {
                HIThemeButtonDrawInfo aListInfo;
                aListInfo.version = 0;
                aListInfo.kind = kThemePopupButton;
                aListInfo.state = getState( nState );//kThemeStateInactive -> greyed
                aListInfo.value = kThemeButtonOn;

                aListInfo.adornment = kThemeAdornmentDefault;
                if( nState & ControlState::FOCUSED )
                    aListInfo.adornment |= kThemeAdornmentFocus;

                HIThemeDrawButton(&rc, &aListInfo, maContextHolder.get(), kHIThemeOrientationNormal,&rc);
                bOK = true;
                break;
            }
            case ControlPart::ListboxWindow:
            {
                HIThemeFrameDrawInfo aTextDrawInfo;
                aTextDrawInfo.version=0;
                aTextDrawInfo.kind=kHIThemeFrameTextFieldSquare;
                aTextDrawInfo.state=getState( nState );
                aTextDrawInfo.isFocused=false;

                rc.size.width+=1; // else there's a white space because a macOS theme has no 3D border
                rc.size.height+=1;
                HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);

                if(nState & ControlState::FOCUSED) HIThemeDrawFocusRect(&rc, true, maContextHolder.get(), kHIThemeOrientationNormal);

                bOK=true;
                break;
            }
            default: break;
        }
        break;

    case ControlType::Editbox:
    case ControlType::MultilineEditbox:
        {
            HIThemeFrameDrawInfo aTextDrawInfo;
            aTextDrawInfo.version=0;
            aTextDrawInfo.kind=kHIThemeFrameTextFieldSquare;
            aTextDrawInfo.state=getState( nState );
            aTextDrawInfo.isFocused=false;

            rc.size.width  += 1; // else there may be a white space because a macOS theme has no 3D border
            // change rc so that the frame will encompass only the content region
            // see counterpart in GetNativeControlRegion
            rc.size.width  += 2;
            rc.size.height += 2;

            //CGContextSetFillColorWithColor
            CGContextFillRect (maContextHolder.get(), CGRectMake(rc.origin.x, rc.origin.y, rc.size.width, rc.size.height));
            //fill a white background, because drawFrame only draws the border

            HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);

            if(nState & ControlState::FOCUSED) HIThemeDrawFocusRect(&rc, true, maContextHolder.get(), kHIThemeOrientationNormal);

            bOK=true;
        }
        break;

    case ControlType::Spinbox:
        {
            if(nPart == ControlPart::Entire)
            {
                //text field:
                HIThemeFrameDrawInfo aTextDrawInfo;
                aTextDrawInfo.version=0;
                aTextDrawInfo.kind=kHIThemeFrameTextFieldSquare;
                aTextDrawInfo.state=getState( nState );
                aTextDrawInfo.isFocused=false;

                //rc.size.width contains the full size of the spinbox ie textfield + button
                //so we remove the button width and the space between the button and the textfield
                rc.size.width -= SPIN_BUTTON_SPACE + SPIN_BUTTON_WIDTH + 2*FOCUS_RING_WIDTH;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;

                //CGContextSetFillColorWithColor
                CGContextFillRect (maContextHolder.get(), CGRectMake(rc.origin.x, rc.origin.y, rc.size.width, rc.size.height));
                //fill a white background, because drawFrame only draws the border

                HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);

                if(nState & ControlState::FOCUSED) HIThemeDrawFocusRect(&rc, true, maContextHolder.get(), kHIThemeOrientationNormal);

                //buttons:
                const SpinbuttonValue* pSpinButtonVal = (aValue.getType() == ControlType::SpinButtons) ? static_cast<const SpinbuttonValue*>(&aValue) : nullptr;
                ControlState nUpperState = ControlState::ENABLED;//state of the upper button
                ControlState nLowerState = ControlState::ENABLED;//and of the lower button
                if(pSpinButtonVal) {//pSpinButtonVal is sometimes null
                    nUpperState = pSpinButtonVal->mnUpperState;
                    nLowerState = pSpinButtonVal->mnLowerState;

                    HIThemeButtonDrawInfo aSpinInfo;
                    aSpinInfo.kind = kThemeIncDecButton;
                    aSpinInfo.state = kThemeStateActive;
                    if(nUpperState & ControlState::PRESSED)
                        aSpinInfo.state = kThemeStatePressedUp;
                    else if(nLowerState & ControlState::PRESSED)
                        aSpinInfo.state = kThemeStatePressedDown;
                    else if((nUpperState & ~ControlState::ENABLED)||(nLowerState & ~ControlState::ENABLED))
                        aSpinInfo.state = kThemeStateInactive;
                    else if((nUpperState & ControlState::ROLLOVER)||(nLowerState & ControlState::ROLLOVER))
                        aSpinInfo.state = kThemeStateRollover;

                    tools::Rectangle aSpinRect( pSpinButtonVal->maUpperRect );
                    aSpinRect.Union( pSpinButtonVal->maLowerRect );
                    HIRect buttonRc = ImplGetHIRectFromRectangle(aSpinRect);

                    // FIXME: without this fuzz factor there is some unwanted clipping
                    if( AllSettings::GetLayoutRTL() )
                        buttonRc.origin.x -= FOCUS_RING_WIDTH - CLIP_FUZZ;
                    else
                        buttonRc.origin.x += FOCUS_RING_WIDTH + CLIP_FUZZ;

                    switch( aValue.getTristateVal() )
                    {
                        case ButtonValue::On:        aSpinInfo.value = kThemeButtonOn;
                                                    break;
                        case ButtonValue::Off:       aSpinInfo.value = kThemeButtonOff;
                                                    break;
                        case ButtonValue::Mixed:
                        case ButtonValue::DontKnow:
                        default:                    aSpinInfo.value = kThemeButtonMixed;
                                                    break;
                    }

                    aSpinInfo.adornment = ( (nUpperState & ControlState::DEFAULT) ||
                                            (nLowerState & ControlState::DEFAULT) ) ?
                                       kThemeAdornmentDefault :
                                       kThemeAdornmentNone;
                    if( (nUpperState & ControlState::FOCUSED) || (nLowerState & ControlState::FOCUSED))
                        aSpinInfo.adornment |= kThemeAdornmentFocus;

                    HIThemeDrawButton( &buttonRc, &aSpinInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr );
                }

                bOK=true;
            }

        }
        break;

    case ControlType::Frame:
        {
            DrawFrameFlags nStyle = static_cast<DrawFrameFlags>(aValue.getNumericVal());
            if( nPart == ControlPart::Border ) {
                if(!( nStyle & DrawFrameFlags::Menu ) && !(nStyle & DrawFrameFlags::WindowBorder) )
                {
                    // #i84756# strange effects start to happen when HIThemeDrawFrame
                    // meets the border of the window. These can be avoided by clipping
                    // to the boundary of the frame
                    if( rc.origin.y + rc.size.height >= mpFrame->maGeometry.nHeight-3 )
                    {
                        CGMutablePathRef rPath = CGPathCreateMutable();
                        CGPathAddRect( rPath, nullptr, CGRectMake( 0, 0, mpFrame->maGeometry.nWidth-1, mpFrame->maGeometry.nHeight-1 ) );

                        CGContextBeginPath( maContextHolder.get() );
                        CGContextAddPath( maContextHolder.get(), rPath );
                        CGContextClip( maContextHolder.get() );
                        CGPathRelease( rPath );
                    }

                    HIThemeFrameDrawInfo aTextDrawInfo;
                    aTextDrawInfo.version=0;
                    aTextDrawInfo.kind=kHIThemeFrameListBox;
                    aTextDrawInfo.state=kThemeStateActive;
                    aTextDrawInfo.isFocused=false;

                    HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);

                    bOK=true;
                }
            }
        }
        break;

    case ControlType::ListNet:
        {
           //do nothing as there isn't net for listviews on macos
            bOK=true;
        }
        break;
    default: break;
    }

    maContextHolder.restoreState();

    /* #i90291# in most cases invalidating the whole control region instead
       of just the unclipped part of it is sufficient (and probably faster).
       However for the window background we should not unnecessarily enlarge
       the really changed rectangle since the difference is usually quite high
       (the background is always drawn as a whole since we don't know anything
       about its possible contents)
    */
    if( nType == ControlType::WindowBackground )
    {
        CGRect aRect = { { 0, 0 }, { 0, 0 } };
        if( mxClipPath )
            aRect = CGPathGetBoundingBox( mxClipPath );
        if( aRect.size.width != 0 && aRect.size.height != 0 )
            buttonRect.Intersection( tools::Rectangle( Point( static_cast<long int>(aRect.origin.x),
                            static_cast<long int>(aRect.origin.y) ),
                                                Size(   static_cast<long int>(aRect.size.width),
                            static_cast<long int>(aRect.size.height) ) ) );
    }

    RefreshRect( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight() );

    return bOK;
}

bool AquaSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState /*nState*/,
                                               const ImplControlValue& aValue, const OUString&,
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion )

{
    if (rControlRegion.IsEmpty()) // nothing to do
        return false;
    bool toReturn = false;

    tools::Rectangle aCtrlBoundRect( rControlRegion );
    short x = aCtrlBoundRect.Left();
    short y = aCtrlBoundRect.Top();
    short w, h;

    sal_uInt8 nBorderCleanup = 0;

    switch (nType)
    {
        case ControlType::Slider:
            {
                if( nPart == ControlPart::ThumbHorz )
                {
                    w = 19; // taken from HIG
                    h = aCtrlBoundRect.GetHeight();
                    rNativeBoundingRegion = rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    toReturn = true;
                }
                else if( nPart == ControlPart::ThumbVert )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = 18; // taken from HIG
                    rNativeBoundingRegion = rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    toReturn = true;
                }
            }
            break;

        case ControlType::Scrollbar:
            {
                tools::Rectangle aRect;
                if( AquaGetScrollRect( /* m_nScreen */ nPart, aCtrlBoundRect, aRect ) )
                {
                    toReturn = true;
                    rNativeBoundingRegion = aRect;
                    rNativeContentRegion = aRect;
                }
            }
            break;

        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            {
                if ( nType == ControlType::Pushbutton )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = aCtrlBoundRect.GetHeight();
                }
                else
                {
                    // checkbox and radio borders need cleanup after unchecking them
                    nBorderCleanup = 4;

                    // TEXT_SEPARATOR to respect Aqua HIG
                    w = BUTTON_WIDTH + TEXT_SEPARATOR;
                    h = BUTTON_HEIGHT;

                }

                rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h + nBorderCleanup) );
                rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );

                toReturn = true;
            }
            break;
        case ControlType::Progress:
            {
                tools::Rectangle aRect( aCtrlBoundRect );
                if( aRect.GetHeight() < 16 )
                    aRect.SetBottom( aRect.Top() + 9 ); // values taken from HIG for medium progress
                else
                    aRect.SetBottom( aRect.Top() + 15 ); // values taken from HIG for large progress
                rNativeBoundingRegion = aRect;
                rNativeContentRegion = aRect;
                toReturn = true;
            }
            break;

        case ControlType::IntroProgress:
            {
                tools::Rectangle aRect( aCtrlBoundRect );
                aRect.SetBottom( aRect.Top() + INTRO_PROGRESS_HEIGHT ); // values taken from HIG for medium progress
                rNativeBoundingRegion = aRect;
                rNativeContentRegion = aRect;
                toReturn = true;
            }
            break;

         case ControlType::TabItem:

            w = aCtrlBoundRect.GetWidth() + 2*TAB_TEXT_OFFSET - 2*VCL_TAB_TEXT_OFFSET;
            h = TAB_HEIGHT_NORMAL+2;

            rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
            rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );

            toReturn = true;

            break;

        case ControlType::Editbox:
            {
                w = aCtrlBoundRect.GetWidth();
                if( w < 3+2*FOCUS_RING_WIDTH )
                    w = 3+2*FOCUS_RING_WIDTH;
                h = TEXT_EDIT_HEIGHT_NORMAL+2*FOCUS_RING_WIDTH;
                if( h < aCtrlBoundRect.GetHeight() )
                    h = aCtrlBoundRect.GetHeight();

                rNativeContentRegion = tools::Rectangle( Point( x+FOCUS_RING_WIDTH, y+FOCUS_RING_WIDTH ), Size( w-2*(FOCUS_RING_WIDTH+1), h-2*(FOCUS_RING_WIDTH+1) ) );
                rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );

                toReturn = true;
            }
            break;
        case ControlType::Listbox:
        case ControlType::Combobox:
            {
                if( nPart == ControlPart::Entire )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = COMBOBOX_HEIGHT_NORMAL;//listboxes and comboxes have the same height

                    rNativeContentRegion = tools::Rectangle( Point( x+FOCUS_RING_WIDTH, y+FOCUS_RING_WIDTH ), Size( w-2*FOCUS_RING_WIDTH, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
                else if( nPart == ControlPart::ButtonDown )
                {
                    w = aCtrlBoundRect.GetWidth();
                    if( w < 3+2*FOCUS_RING_WIDTH )
                        w = 3+2*FOCUS_RING_WIDTH;
                    h = COMBOBOX_HEIGHT_NORMAL;//listboxes and comboxes have the same height

                    x += w-DROPDOWN_BUTTON_WIDTH - FOCUS_RING_WIDTH;
                    y += FOCUS_RING_WIDTH;
                    w = DROPDOWN_BUTTON_WIDTH;

                    rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w+FOCUS_RING_WIDTH, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
                else if( nPart == ControlPart::SubEdit )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = COMBOBOX_HEIGHT_NORMAL;//listboxes and comboxes have the same height

                    x += FOCUS_RING_WIDTH;
                    x += 3; // add an offset for rounded borders
                    y += 2; // don't draw into upper border
                    y += FOCUS_RING_WIDTH;
                    w -= 3 + DROPDOWN_BUTTON_WIDTH + 2*FOCUS_RING_WIDTH;
                    if( nType == ControlType::Listbox )
                        w -= 9; // HIG specifies 9 units distance between dropdown button area and content
                    h -= 4; // don't draw into lower border

                    rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w+FOCUS_RING_WIDTH, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
            }
            break;
        case ControlType::Spinbox:
                if( nPart == ControlPart::Entire ) {
                    w = aCtrlBoundRect.GetWidth();
                    if( w < 3+2*FOCUS_RING_WIDTH+SPIN_BUTTON_SPACE+SPIN_BUTTON_WIDTH )
                        w = 3+2*FOCUS_RING_WIDTH+SPIN_BUTTON_SPACE+SPIN_BUTTON_WIDTH;
                    h = TEXT_EDIT_HEIGHT_NORMAL;

                    rNativeContentRegion = tools::Rectangle( Point( x+FOCUS_RING_WIDTH, y ), Size( w-2*FOCUS_RING_WIDTH, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
                else if( nPart == ControlPart::SubEdit ) {
                    w = aCtrlBoundRect.GetWidth() - SPIN_BUTTON_SPACE - SPIN_BUTTON_WIDTH;
                    h = TEXT_EDIT_HEIGHT_NORMAL;
                    x += 4; // add an offset for rounded borders
                    y += 2; // don't draw into upper border
                    w -= 8; // offset for left and right rounded border
                    h -= 4; // don't draw into upper or lower border

                    rNativeContentRegion = tools::Rectangle( Point( x + FOCUS_RING_WIDTH, y + FOCUS_RING_WIDTH ), Size( w - 2* FOCUS_RING_WIDTH, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
                else if( nPart == ControlPart::ButtonUp ) {
                    //aCtrlBoundRect.GetWidth() contains the width of the full control
                    //ie the width of the textfield + button
                    //x is the position of the left corner of the full control
                    x += aCtrlBoundRect.GetWidth() - SPIN_BUTTON_WIDTH - SPIN_BUTTON_SPACE - CLIP_FUZZ;
                    y += FOCUS_RING_WIDTH - CLIP_FUZZ;
                    w = SPIN_BUTTON_WIDTH + 2*CLIP_FUZZ;
                    h = SPIN_UPPER_BUTTON_HEIGHT + 2*CLIP_FUZZ;

                    rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );

                    toReturn = true;
                }
                else if( nPart == ControlPart::ButtonDown ) {
                    x += aCtrlBoundRect.GetWidth() - SPIN_BUTTON_WIDTH - SPIN_BUTTON_SPACE - CLIP_FUZZ;
                    y += SPIN_UPPER_BUTTON_HEIGHT + FOCUS_RING_WIDTH - CLIP_FUZZ;
                    w = SPIN_BUTTON_WIDTH + 2*CLIP_FUZZ;
                    h = SPIN_LOWER_BUTTON_HEIGHT + 2*CLIP_FUZZ;

                    rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );

                    toReturn = true;
                }
            break;
        case ControlType::Frame:
            {
                DrawFrameStyle nStyle = static_cast<DrawFrameStyle>(aValue.getNumericVal() & 0x000f);
                DrawFrameFlags nFlags = static_cast<DrawFrameFlags>(aValue.getNumericVal() & 0xfff0);
                if(  ( nPart == ControlPart::Border ) &&
                    !( nFlags & (DrawFrameFlags::Menu | DrawFrameFlags::WindowBorder | DrawFrameFlags::BorderWindowBorder) ) )
                {
                    tools::Rectangle aRect(aCtrlBoundRect);
                    if( nStyle == DrawFrameStyle::DoubleIn )
                    {
                        aRect.AdjustLeft(1);
                        aRect.AdjustTop(1);
                        //rRect.Right() -= 1;
                        //rRect.Bottom()    -= 1;
                    }
                    else
                    {
                        aRect.AdjustLeft(1);
                        aRect.AdjustTop(1);
                        aRect.AdjustRight(-1);
                        aRect.AdjustBottom(-1);
                    }

                    rNativeContentRegion = aRect;
                    rNativeBoundingRegion = aRect;

                    toReturn = true;
                }
            }
            break;

        case ControlType::Menubar:
        case ControlType::MenuPopup:
            {
                if(( nPart == ControlPart::MenuItemCheckMark )||( nPart == ControlPart::MenuItemRadioMark )) {

                    w=10;
                    h=10;//dimensions of the mark (10px font)

                    rNativeContentRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = tools::Rectangle( Point( x, y ), Size( w, h ) );

                    toReturn = true;
                }
            }
            break;
      default: break;

    }

    return toReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
