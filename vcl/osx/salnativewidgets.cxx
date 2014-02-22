/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "vcl/salnativewidgets.hxx"
#include "vcl/decoview.hxx"
#include "vcl/svapp.hxx"
#include "vcl/timer.hxx"
#include <vcl/settings.hxx>

#include "quartz/salgdi.h"
#include "osx/salnativewidgets.h"
#include "osx/saldata.hxx"
#include "osx/salframe.h"

#include "premac.h"
#include <Carbon/Carbon.h>
#include "postmac.h"

#ifndef NSAppKitVersionNumber10_7
#define NSAppKitVersionNumber10_7 1138
#endif

extern "C"
{
    typedef CFTypeRef CUIRendererRef;
    void CUIDraw(CUIRendererRef r, CGRect rect, CGContextRef ctx, CFDictionaryRef options, CFDictionaryRef* result);
}

@interface NSWindow(CoreUIRendererPrivate)
+ (CUIRendererRef)coreUIRenderer;
@end

class AquaBlinker : public Timer
{
    AquaSalFrame*       mpFrame;
    Rectangle           maInvalidateRect;

    AquaBlinker( AquaSalFrame* pFrame, const Rectangle& rRect )
    : mpFrame( pFrame ), maInvalidateRect( rRect )
    {
        mpFrame->maBlinkers.push_back( this );
    }

    public:

    static void Blink( AquaSalFrame*, const Rectangle&, int nTimeout = 80 );

    virtual void Timeout()
    {
        Stop();
        if( AquaSalFrame::isAlive( mpFrame ) && mpFrame->mbShown )
        {
            mpFrame->maBlinkers.remove( this );
            mpFrame->SendPaintEvent( &maInvalidateRect );
        }
        delete this;
    }
};

void AquaBlinker::Blink( AquaSalFrame* pFrame, const Rectangle& rRect, int nTimeout )
{
    
    for( std::list< AquaBlinker* >::const_iterator it = pFrame->maBlinkers.begin();
         it != pFrame->maBlinkers.end(); ++it )
    {
        if( (*it)->maInvalidateRect == rRect )
            return;
    }
    AquaBlinker* pNew = new AquaBlinker( pFrame, rRect );
    pNew->SetTimeout( nTimeout );
    pNew->Start();
}



static HIRect ImplGetHIRectFromRectangle(Rectangle aRect)
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
        case BUTTONVALUE_ON:
            return kThemeButtonOn;
            break;

        case BUTTONVALUE_OFF:
            return kThemeButtonOff;
            break;

        case BUTTONVALUE_MIXED:
        case BUTTONVALUE_DONTKNOW:
        default:
            return kThemeButtonMixed;
            break;
    }
}

static bool AquaGetScrollRect( /* TODO: int nScreen, */  ControlPart nPart,
    const Rectangle& rControlRect, Rectangle& rResultRect )
{
    bool bRetVal = true;
    rResultRect = rControlRect;

    switch( nPart )
    {
        case PART_BUTTON_UP:
            if (NSAppKitVersionNumber < NSAppKitVersionNumber10_7)
            {
                if( GetSalData()->mbIsScrollbarDoubleMax )
                    rResultRect.Top() = rControlRect.Bottom() - 2*BUTTON_HEIGHT;
                rResultRect.Bottom() = rResultRect.Top() + BUTTON_HEIGHT;
            }
            else
            {
                rResultRect.Bottom() = rResultRect.Top();
            }
            break;

        case PART_BUTTON_DOWN:
            if (NSAppKitVersionNumber < NSAppKitVersionNumber10_7)
            {
                rResultRect.Top() = rControlRect.Bottom() - BUTTON_HEIGHT;
            }
            else
            {
                rResultRect.Top() = rResultRect.Bottom();
            }
            break;

        case PART_BUTTON_LEFT:
            if (NSAppKitVersionNumber < NSAppKitVersionNumber10_7)
            {
                if( GetSalData()->mbIsScrollbarDoubleMax )
                    rResultRect.Left() = rControlRect.Right() - 2*BUTTON_WIDTH;
                rResultRect.Right() = rResultRect.Left() + BUTTON_WIDTH;
            }
            else
            {
                rResultRect.Right() = rResultRect.Left();
            }
            break;

        case PART_BUTTON_RIGHT:
            if (NSAppKitVersionNumber < NSAppKitVersionNumber10_7)
            {
                rResultRect.Left() = rControlRect.Right() - BUTTON_WIDTH;
            }
            else
            {
                rResultRect.Left() = rResultRect.Right();
            }
            break;

        case PART_TRACK_HORZ_AREA:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            rResultRect.Right() -= BUTTON_WIDTH + 1;
            if( GetSalData()->mbIsScrollbarDoubleMax )
                rResultRect.Right() -= BUTTON_WIDTH;
            else
                rResultRect.Left() += BUTTON_WIDTH + 1;
            break;

        case PART_TRACK_VERT_AREA:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            rResultRect.Bottom() -= BUTTON_HEIGHT + 1;
            if( GetSalData()->mbIsScrollbarDoubleMax )
                rResultRect.Bottom() -= BUTTON_HEIGHT;
            else
                rResultRect.Top() += BUTTON_HEIGHT + 1;
            break;
        case PART_THUMB_HORZ:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            if( GetSalData()->mbIsScrollbarDoubleMax )
            {
                rResultRect.Left() += 8;
                rResultRect.Right() += 6;
            }
            else
            {
                rResultRect.Left() += 4;
                rResultRect.Right() += 4;
            }
            break;
        case PART_THUMB_VERT:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            if( GetSalData()->mbIsScrollbarDoubleMax )
            {
                rResultRect.Top() += 8;
                rResultRect.Bottom() += 8;
            }
            else
            {
                rResultRect.Top() += 4;
                rResultRect.Bottom() += 4;
            }
            break;
        case PART_TRACK_HORZ_LEFT:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            if( GetSalData()->mbIsScrollbarDoubleMax )
                rResultRect.Right() += 8;
            else
                rResultRect.Right() += 4;
            break;
        case PART_TRACK_HORZ_RIGHT:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            if( GetSalData()->mbIsScrollbarDoubleMax )
                rResultRect.Left() += 6;
            else
                rResultRect.Left() += 4;
            break;
        case PART_TRACK_VERT_UPPER:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            if( GetSalData()->mbIsScrollbarDoubleMax )
                rResultRect.Bottom() += 8;
            else
                rResultRect.Bottom() += 4;
            break;
        case PART_TRACK_VERT_LOWER:
            if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
                break;
            if( GetSalData()->mbIsScrollbarDoubleMax )
                rResultRect.Top() += 8;
            else
                rResultRect.Top() += 4;
            break;
        default:
            bRetVal = false;
    }

    return bRetVal;
}

/*
 * IsNativeControlSupported()
 * --------------------------
 * Returns sal_True if the platform supports native
 * drawing of the control defined by nPart.
 *
 */
bool AquaSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    bool bOk = false;

    
    
    
    

    switch( nType )
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
        case CTRL_LISTNODE:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;

        case CTRL_SCROLLBAR:
            if( nPart == PART_DRAW_BACKGROUND_HORZ ||
                nPart == PART_DRAW_BACKGROUND_VERT ||
                nPart == PART_ENTIRE_CONTROL       ||
                nPart == HAS_THREE_BUTTONS )
                return true;
            break;

        case CTRL_SLIDER:
            if( nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA )
                return true;
            break;

        case CTRL_EDITBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_MULTILINE_EDITBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_SPINBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_ALL_BUTTONS    ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_SPINBUTTONS:
                return false;
            break;

        case CTRL_COMBOBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_LISTBOX:
            if( nPart == PART_ENTIRE_CONTROL    ||
                nPart == PART_WINDOW            ||
                nPart == HAS_BACKGROUND_TEXTURE ||
                nPart == PART_SUB_EDIT
                )
                return true;
            break;

        case CTRL_TAB_ITEM:
        case CTRL_TAB_PANE:
        case CTRL_TAB_BODY:  
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_TABS_DRAW_RTL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        
        
        case CTRL_TOOLBAR:
            if( nPart == PART_ENTIRE_CONTROL       ||
                nPart == PART_DRAW_BACKGROUND_HORZ ||
                nPart == PART_DRAW_BACKGROUND_VERT)
                return true;
            break;

        case  CTRL_WINDOW_BACKGROUND:
            if ( nPart == PART_BACKGROUND_WINDOW ||
                 nPart == PART_BACKGROUND_DIALOG )
                 return true;
            break;

        case CTRL_MENUBAR:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;

        case CTRL_TOOLTIP: 
            break;

        case CTRL_MENU_POPUP:
            if( nPart == PART_ENTIRE_CONTROL       ||
                nPart == PART_MENU_ITEM            ||
                nPart == PART_MENU_ITEM_CHECK_MARK ||
                nPart == PART_MENU_ITEM_RADIO_MARK)
                return true;
            break;
        case CTRL_PROGRESS:
        case CTRL_INTROPROGRESS:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;
        case CTRL_FRAME:
            if( nPart == PART_BORDER )
                return true;
            break;
        case CTRL_LISTNET:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;
    }

    return bOk;
}

/*
 * HitTestNativeControl()
 *
 *  If the return value is sal_True, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
bool AquaSalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                        const Point& rPos, bool& rIsInside )
{
    if ( nType == CTRL_SCROLLBAR )
    {
        Rectangle aRect;
        bool bValid = AquaGetScrollRect( /* TODO: m_nScreen */ nPart, rControlRegion, aRect );
        rIsInside = bValid ? aRect.IsInside( rPos ) : sal_False;
        if( NSAppKitVersionNumber < NSAppKitVersionNumber10_7 &&
            GetSalData()->mbIsScrollbarDoubleMax )
        {
            
            
            if( bValid && rIsInside )
            {
                if( nPart == PART_TRACK_HORZ_AREA )
                {
                    
                    if( rPos.X() - aRect.Left() < 4 )
                        rIsInside = false;
                }
                else if( nPart == PART_TRACK_VERT_AREA )
                {
                    
                    if( rPos.Y() - aRect.Top() < 4 )
                        rIsInside = false;
                }
            }
        }
        return bValid;
    }  

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

#define CTRL_STATE_ENABLED      0x0001
#define CTRL_STATE_FOCUSED      0x0002
#define CTRL_STATE_PRESSED      0x0004
#define CTRL_STATE_ROLLOVER     0x0008
#define CTRL_STATE_HIDDEN       0x0010
#define CTRL_STATE_DEFAULT      0x0020
#define CTRL_STATE_SELECTED     0x0040
#define CTRL_CACHING_ALLOWED    0x8000  
*/
UInt32 AquaSalGraphics::getState( ControlState nState )
{
    const bool bDrawActive = mpFrame ? ([mpFrame->getNSWindow() isKeyWindow] ? true : false) : true;
    if( (nState & CTRL_STATE_ENABLED) == 0 || ! bDrawActive )
    {
        if( (nState & CTRL_STATE_HIDDEN) == 0 )
            return kThemeStateInactive;
        else
            return kThemeStateUnavailableInactive;
    }

    if( (nState & CTRL_STATE_HIDDEN) != 0 )
        return kThemeStateUnavailable;

    if( (nState & CTRL_STATE_PRESSED) != 0 )
        return kThemeStatePressed;

    return kThemeStateActive;
}

UInt32 AquaSalGraphics::getTrackState( ControlState nState )
{
    const bool bDrawActive = mpFrame ? ([mpFrame->getNSWindow() isKeyWindow] ? true : false) : true;
    if( (nState & CTRL_STATE_ENABLED) == 0 || ! bDrawActive )
            return kThemeTrackInactive;

    return kThemeTrackActive;
}

/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  aCaption:   A caption or title string (like button text etc)
 */
bool AquaSalGraphics::drawNativeControl(ControlType nType,
                    ControlPart nPart,
                    const Rectangle& rControlRegion,
                    ControlState nState,
                    const ImplControlValue& aValue,
                    const OUString& )
{
    sal_Bool bOK = sal_False;

    if( ! CheckContext() )
        return false;

    CGContextSaveGState( mrContext );

    Rectangle buttonRect = rControlRegion;
    HIRect rc = ImplGetHIRectFromRectangle(buttonRect);

    switch( nType )
    {

    case  CTRL_COMBOBOX:
        if ( nPart == HAS_BACKGROUND_TEXTURE ||
             nPart == PART_ENTIRE_CONTROL )
        {
            HIThemeButtonDrawInfo aComboInfo;
            aComboInfo.version = 0;
            aComboInfo.kind = kThemeComboBox;
            aComboInfo.state = getState( nState );
            aComboInfo.value = kThemeButtonOn;
            aComboInfo.adornment = kThemeAdornmentNone;

            if( (nState & CTRL_STATE_FOCUSED) != 0 )
                aComboInfo.adornment |= kThemeAdornmentFocus;

            HIThemeDrawButton(&rc, &aComboInfo, mrContext, kHIThemeOrientationNormal,&rc);
            bOK = true;
        }
        break;

    case CTRL_TOOLBAR:
        {
            if (rControlRegion.Top() == 0 && nPart == PART_DRAW_BACKGROUND_HORZ)
            {
                BOOL isMain = [mpFrame->getNSWindow() isMainWindow];
                CGFloat unifiedHeight = rControlRegion.GetHeight();
                CGRect drawRect = CGRectMake(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.GetWidth(), rControlRegion.GetHeight());
                CUIDraw([NSWindow coreUIRenderer], drawRect, mrContext,
                        (CFDictionaryRef)[NSDictionary dictionaryWithObjectsAndKeys:
                        @"kCUIWidgetWindowFrame", @"widget",
                        @"regularwin", @"windowtype",
                        (isMain ? @"normal" : @"inactive"), @"state",
                        [NSNumber numberWithDouble:unifiedHeight], @"kCUIWindowFrameUnifiedTitleBarHeightKey",
                        [NSNumber numberWithBool:NO], @"kCUIWindowFrameDrawTitleSeparatorKey",
                        [NSNumber numberWithBool:YES], @"is.flipped",
                        nil],
                        nil);;
            }
            else
            {
                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
                aMenuItemDrawInfo.version = 0;
                aMenuItemDrawInfo.state = kThemeMenuActive;
                aMenuItemDrawInfo.itemType = kThemeMenuItemHierBackground;
                HIThemeDrawMenuItem(&rc, &rc, &aMenuItemDrawInfo, mrContext, kHIThemeOrientationNormal, NULL);
            }
            bOK = true;
        }
        break;

        case CTRL_WINDOW_BACKGROUND:
        {
            HIThemeBackgroundDrawInfo aThemeBackgroundInfo;
            aThemeBackgroundInfo.version = 0;
            aThemeBackgroundInfo.state = getState( nState );
            aThemeBackgroundInfo.kind = kThemeBrushDialogBackgroundActive;
            
            rc.size.width += 2;
            rc.size.height += 2;

            HIThemeApplyBackground( &rc, &aThemeBackgroundInfo, mrContext, kHIThemeOrientationNormal);
            CGContextFillRect( mrContext, rc );
            bOK = true;
        }
        break;

    case CTRL_MENUBAR:
    case CTRL_MENU_POPUP:
        {
            if ((nPart == PART_ENTIRE_CONTROL) || (nPart == PART_MENU_ITEM )|| (nPart == HAS_BACKGROUND_TEXTURE ))
            {
                
                rc.size.width += 2;

                HIThemeMenuDrawInfo aMenuInfo;
                aMenuInfo.version = 0;
                aMenuInfo.menuType = kThemeMenuTypePullDown;

                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
                
                aMenuItemDrawInfo.itemType = kThemeMenuItemPlain;

                if ((nPart == PART_MENU_ITEM ) && (nState & CTRL_STATE_SELECTED))
                {
                    
                    aMenuItemDrawInfo.state = kThemeMenuSelected;
                }
                else
                {
                    
                    aMenuItemDrawInfo.state = kThemeMenuActive;
                }

                
                HIThemeDrawMenuBackground(&rc,&aMenuInfo,mrContext,kHIThemeOrientationNormal);

                
                HIThemeDrawMenuItem(&rc,&rc,&aMenuItemDrawInfo,mrContext,kHIThemeOrientationNormal,&rc);

                bOK = true;
            }
            else if(( nPart == PART_MENU_ITEM_CHECK_MARK )||( nPart == PART_MENU_ITEM_RADIO_MARK )) {
                if( nState & CTRL_STATE_PRESSED ) {
                    HIThemeTextInfo aTextInfo;
                    aTextInfo.version = 0;
                    aTextInfo.state = ((nState & CTRL_STATE_ENABLED)==0) ? kThemeStateInactive: kThemeStateActive;
                    aTextInfo.fontID = kThemeMenuItemMarkFont;
                    aTextInfo.horizontalFlushness=kHIThemeTextHorizontalFlushCenter;
                    aTextInfo.verticalFlushness=kHIThemeTextVerticalFlushTop;
                    aTextInfo.options=kHIThemeTextBoxOptionNone;
                    aTextInfo.truncationPosition=kHIThemeTextTruncationNone;
                    

                    if( nState & CTRL_STATE_SELECTED) aTextInfo.state = kThemeStatePressed; 

                    UniChar mark=( nPart == PART_MENU_ITEM_CHECK_MARK ) ? kCheckUnicode: kBulletUnicode;
                    CFStringRef cfString = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, &mark, 1, kCFAllocatorNull);
                    HIThemeDrawTextBox(cfString, &rc, &aTextInfo, mrContext, kHIThemeOrientationNormal);
                    if (cfString)
                        CFRelease(cfString);

                    bOK = true;
                }
            }
        }
        break;

    case CTRL_PUSHBUTTON:
        {
            
            const int PB_Mini_Height = 15;
            const int PB_Norm_Height = 21;

            HIThemeButtonDrawInfo aPushInfo;
            aPushInfo.version = 0;

            
            aPushInfo.animation.time.start = 0;
            aPushInfo.animation.time.current = 0;
            PushButtonValue* pPBVal = aValue.getType() == CTRL_PUSHBUTTON ? (PushButtonValue*)&aValue : NULL;
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
            else if( pPBVal->mbSingleLine || rc.size.height < (PB_Norm_Height + PB_Norm_Height/2) )
            {
                aPushInfo.kind = kThemePushButtonNormal;
                nPaintHeight = PB_Norm_Height;

                
                rc.origin.x += FOCUS_RING_WIDTH/2;
                rc.size.width -= FOCUS_RING_WIDTH;

                if( (nState & CTRL_STATE_DEFAULT) != 0 )
                {
                    AquaBlinker::Blink( mpFrame, buttonRect );
                    
                    aPushInfo.animation.time.current = CFAbsoluteTimeGetCurrent();
                }
            }
            else
                aPushInfo.kind = kThemeBevelButton;

            
            
            int delta_y = static_cast<int>(rc.size.height) - nPaintHeight;
            rc.origin.y += delta_y/2;

            aPushInfo.state = getState( nState );
            aPushInfo.value = ImplGetButtonValue( aValue.getTristateVal() );

            aPushInfo.adornment = (( nState & CTRL_STATE_DEFAULT ) != 0) ?
            kThemeAdornmentDefault :
            kThemeAdornmentNone;
            if( (nState & CTRL_STATE_FOCUSED) != 0 )
                aPushInfo.adornment |= kThemeAdornmentFocus;

            HIThemeDrawButton( &rc, &aPushInfo, mrContext, kHIThemeOrientationNormal, NULL );
            bOK = true;
        }
        break;

    case CTRL_RADIOBUTTON:
    case CTRL_CHECKBOX:
        {
            HIThemeButtonDrawInfo aInfo;
            aInfo.version = 0;
            switch( nType )
            {
            case CTRL_RADIOBUTTON: if(rc.size.width >= BUTTON_HEIGHT) aInfo.kind = kThemeRadioButton;
                                    else aInfo.kind = kThemeSmallRadioButton;
                break;
            case CTRL_CHECKBOX:   if(rc.size.width >= BUTTON_HEIGHT) aInfo.kind = kThemeCheckBox;
                                    else aInfo.kind = kThemeSmallCheckBox;
                break;
            }

            aInfo.state = getState( nState );

            ButtonValue aButtonValue = aValue.getTristateVal();
            aInfo.value = ImplGetButtonValue( aButtonValue );

            aInfo.adornment = (( nState & CTRL_STATE_DEFAULT ) != 0) ?
            kThemeAdornmentDefault :
            kThemeAdornmentNone;
            if( (nState & CTRL_STATE_FOCUSED) != 0 )
                aInfo.adornment |= kThemeAdornmentFocus;
            HIThemeDrawButton( &rc, &aInfo, mrContext, kHIThemeOrientationNormal, NULL );
            bOK = true;
        }
        break;

    case CTRL_LISTNODE:
        {
            ButtonValue aButtonValue = aValue.getTristateVal();

            if( Application::GetSettings().GetLayoutRTL() && aButtonValue == BUTTONVALUE_OFF )
            {
                
                
                
                
                

                CGContextSetShouldAntialias( mrContext, true );
                CGFloat aGrey[] = { 0.45, 0.45, 0.45, 1.0 };
                CGContextSetFillColor( mrContext, aGrey );
                CGContextBeginPath( mrContext );
                float x = rc.origin.x + rc.size.width;
                float y = rc.origin.y;
                CGContextMoveToPoint( mrContext, x, y );
                y += rc.size.height;
                CGContextAddLineToPoint( mrContext, x, y );
                x -= rc.size.height * 0.866; 
                y -= rc.size.height/2;
                CGContextAddLineToPoint( mrContext, x, y );
                CGContextDrawPath( mrContext, kCGPathEOFill );
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
                    case BUTTONVALUE_ON: aInfo.value = kThemeDisclosureDown;
                        break;
                    case BUTTONVALUE_OFF:
                        
                        
                        if( Application::GetSettings().GetLayoutRTL() )
                        {
                            aInfo.value = kThemeDisclosureLeft;
                        }
                        break;
                    case BUTTONVALUE_DONTKNOW: 
                    default:
                        break;
                }

                HIThemeDrawButton( &rc, &aInfo, mrContext, kHIThemeOrientationNormal, NULL );
            }
            bOK = true;
        }
        break;

    case CTRL_PROGRESS:
    case CTRL_INTROPROGRESS:
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
            aTrackInfo.bounds.origin.y     -= 2; 
            aTrackInfo.bounds.size.width   -= 2; 
            aTrackInfo.attributes           = kThemeTrackHorizontal;
            if( Application::GetSettings().GetLayoutRTL() )
                aTrackInfo.attributes      |= kThemeTrackRightToLeft;
            aTrackInfo.enableState          = getTrackState( nState );
            
            if( nType == CTRL_INTROPROGRESS )
                aTrackInfo.enableState          = kThemeTrackActive;
            aTrackInfo.filler1              = 0;
            aTrackInfo.trackInfo.progress.phase   = static_cast<UInt8>(CFAbsoluteTimeGetCurrent()*10.0);

            HIThemeDrawTrack( &aTrackInfo, NULL, mrContext, kHIThemeOrientationNormal );
            bOK = true;
        }
        break;

    case CTRL_SLIDER:
        {
            SliderValue* pSLVal = (SliderValue*)&aValue;

            HIThemeTrackDrawInfo aTrackDraw;
            aTrackDraw.kind = kThemeSliderMedium;
            if( nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA )
            {
                aTrackDraw.bounds = rc;
                aTrackDraw.min   = pSLVal->mnMin;
                aTrackDraw.max   = pSLVal->mnMax;
                aTrackDraw.value = pSLVal->mnCur;
                aTrackDraw.reserved = 0;
                aTrackDraw.attributes = kThemeTrackShowThumb;
                if( nPart == PART_TRACK_HORZ_AREA )
                    aTrackDraw.attributes |= kThemeTrackHorizontal;
                aTrackDraw.enableState = (nState & CTRL_STATE_ENABLED)
                                         ? kThemeTrackActive : kThemeTrackInactive;

                SliderTrackInfo aSlideInfo;
                aSlideInfo.thumbDir = kThemeThumbUpward;
                aSlideInfo.pressState = 0;
                aTrackDraw.trackInfo.slider = aSlideInfo;

                HIThemeDrawTrack( &aTrackDraw, NULL, mrContext, kHIThemeOrientationNormal );
                bOK = true;
            }
        }
        break;

    case CTRL_SCROLLBAR:
        {
            const ScrollbarValue* pScrollbarVal = (aValue.getType() == CTRL_SCROLLBAR) ? static_cast<const ScrollbarValue*>(&aValue) : NULL;

            if( nPart == PART_DRAW_BACKGROUND_VERT ||
                nPart == PART_DRAW_BACKGROUND_HORZ )
            {
                HIThemeTrackDrawInfo aTrackDraw;
                aTrackDraw.kind = kThemeMediumScrollBar;
                
                if (nPart == PART_DRAW_BACKGROUND_VERT)
                    rc.size.height += 2;
                else
                    rc.size.width += 2;

                aTrackDraw.bounds = rc;
                aTrackDraw.min = pScrollbarVal->mnMin;
                aTrackDraw.max = pScrollbarVal->mnMax - pScrollbarVal->mnVisibleSize;
                aTrackDraw.value = pScrollbarVal->mnCur;
                aTrackDraw.reserved = 0;
                aTrackDraw.attributes = kThemeTrackShowThumb;
                if( nPart == PART_DRAW_BACKGROUND_HORZ )
                    aTrackDraw.attributes |= kThemeTrackHorizontal;
                aTrackDraw.enableState = getTrackState( nState );

                ScrollBarTrackInfo aScrollInfo;
                aScrollInfo.viewsize = pScrollbarVal->mnVisibleSize;
                aScrollInfo.pressState = 0;

                if ( pScrollbarVal->mnButton1State & CTRL_STATE_ENABLED )
                {
                    if ( pScrollbarVal->mnButton1State & CTRL_STATE_PRESSED )
                        aScrollInfo.pressState = kThemeTopOutsideArrowPressed;
                }

                if ( pScrollbarVal->mnButton2State & CTRL_STATE_ENABLED )
                {
                    if ( pScrollbarVal->mnButton2State & CTRL_STATE_PRESSED )
                        aScrollInfo.pressState = kThemeBottomOutsideArrowPressed;
                }

                if ( pScrollbarVal->mnThumbState & CTRL_STATE_ENABLED )
                {
                    if ( pScrollbarVal->mnThumbState & CTRL_STATE_PRESSED )
                        aScrollInfo.pressState = kThemeThumbPressed;
                }

                aTrackDraw.trackInfo.scrollbar = aScrollInfo;

                HIThemeDrawTrack( &aTrackDraw, NULL, mrContext, kHIThemeOrientationNormal );
                bOK = true;
            }
        }
        break;

    case CTRL_TAB_PANE:
        {
            HIThemeTabPaneDrawInfo aTabPaneDrawInfo;
            aTabPaneDrawInfo.version = 1;
            aTabPaneDrawInfo.state = kThemeStateActive;
            aTabPaneDrawInfo.direction=kThemeTabNorth;
            aTabPaneDrawInfo.size=kHIThemeTabSizeNormal;
            aTabPaneDrawInfo.kind=kHIThemeTabKindNormal;

            
            
            rc.origin.x+=1;
            rc.origin.y-=TAB_HEIGHT_NORMAL/2;
            rc.size.height+=TAB_HEIGHT_NORMAL/2;
            rc.size.width-=2;

            HIThemeDrawTabPane(&rc, &aTabPaneDrawInfo, mrContext, kHIThemeOrientationNormal);

            bOK = true;
        }
        break;

    case CTRL_TAB_ITEM:
        {
            HIThemeTabDrawInfo aTabItemDrawInfo;
            aTabItemDrawInfo.version=1;
            aTabItemDrawInfo.style=kThemeTabNonFront;
            aTabItemDrawInfo.direction=kThemeTabNorth;
            aTabItemDrawInfo.size=kHIThemeTabSizeNormal;
            aTabItemDrawInfo.adornment=kHIThemeTabAdornmentTrailingSeparator;
            
            if(nState & CTRL_STATE_SELECTED) {
                aTabItemDrawInfo.style=kThemeTabFront;
            }
            if(nState & CTRL_STATE_FOCUSED) {
                aTabItemDrawInfo.adornment|=kHIThemeTabAdornmentFocus;
            }

            
            aTabItemDrawInfo.position=kHIThemeTabPositionMiddle;

            TabitemValue* pTabValue = (TabitemValue *) &aValue;
            unsigned int nAlignment = pTabValue->mnAlignment;
            
            
            
            
            if( ( (nAlignment & TABITEM_LEFTALIGNED)&&(nAlignment & TABITEM_RIGHTALIGNED) ) ||
                ( (nAlignment & TABITEM_FIRST_IN_GROUP)&&(nAlignment & TABITEM_LAST_IN_GROUP) )
               ) 
                aTabItemDrawInfo.position=kHIThemeTabPositionOnly;
            else if((nAlignment & TABITEM_LEFTALIGNED)||(nAlignment & TABITEM_FIRST_IN_GROUP))
                aTabItemDrawInfo.position=kHIThemeTabPositionFirst;
            else if((nAlignment & TABITEM_RIGHTALIGNED)||(nAlignment & TABITEM_LAST_IN_GROUP))
                aTabItemDrawInfo.position=kHIThemeTabPositionLast;

            
            
            if( Application::GetSettings().GetLayoutRTL() ) {
                if( aTabItemDrawInfo.position == kHIThemeTabPositionFirst )
                        aTabItemDrawInfo.position = kHIThemeTabPositionLast;
                else if( aTabItemDrawInfo.position == kHIThemeTabPositionLast )
                        aTabItemDrawInfo.position = kHIThemeTabPositionFirst;
            }

            rc.size.width+=2;
            rc.origin.x-=1;

            HIThemeDrawTab(&rc, &aTabItemDrawInfo, mrContext, kHIThemeOrientationNormal, &rc );

            bOK=true;
        }
        break;

    case  CTRL_LISTBOX:
        switch( nPart)
        {
            case PART_ENTIRE_CONTROL:
            case PART_BUTTON_DOWN:
            {
                HIThemeButtonDrawInfo aListInfo;
                aListInfo.version = 0;
                aListInfo.kind = kThemePopupButton;
                aListInfo.state = getState( nState );
                aListInfo.value = kThemeButtonOn;

                aListInfo.adornment = kThemeAdornmentDefault;
                if( (nState & CTRL_STATE_FOCUSED) != 0 )
                    aListInfo.adornment |= kThemeAdornmentFocus;

                HIThemeDrawButton(&rc, &aListInfo, mrContext, kHIThemeOrientationNormal,&rc);
                bOK = true;
                break;
            }
            case PART_WINDOW:
            {
                HIThemeFrameDrawInfo aTextDrawInfo;
                aTextDrawInfo.version=0;
                aTextDrawInfo.kind=kHIThemeFrameTextFieldSquare;
                aTextDrawInfo.state=getState( nState );
                aTextDrawInfo.isFocused=false;

                rc.size.width+=1;
                rc.size.height+=1;
                HIThemeDrawFrame(&rc, &aTextDrawInfo, mrContext, kHIThemeOrientationNormal);

                if(nState & CTRL_STATE_FOCUSED) HIThemeDrawFocusRect(&rc, true, mrContext, kHIThemeOrientationNormal);

                bOK=true;
                break;
            }
        }
        break;

    case CTRL_EDITBOX:
    case CTRL_MULTILINE_EDITBOX:
        {
            HIThemeFrameDrawInfo aTextDrawInfo;
            aTextDrawInfo.version=0;
            aTextDrawInfo.kind=kHIThemeFrameTextFieldSquare;
            aTextDrawInfo.state=getState( nState );
            aTextDrawInfo.isFocused=false;

            rc.size.width  += 1; 
            
            
            rc.size.width  += 2;
            rc.size.height += 2;

            
            CGContextFillRect (mrContext, CGRectMake(rc.origin.x, rc.origin.y, rc.size.width, rc.size.height));
            

            HIThemeDrawFrame(&rc, &aTextDrawInfo, mrContext, kHIThemeOrientationNormal);

            if(nState & CTRL_STATE_FOCUSED) HIThemeDrawFocusRect(&rc, true, mrContext, kHIThemeOrientationNormal);

            bOK=true;
        }
        break;

    case CTRL_SPINBOX:
        {
            if(nPart == PART_ENTIRE_CONTROL)
            {
                
                HIThemeFrameDrawInfo aTextDrawInfo;
                aTextDrawInfo.version=0;
                aTextDrawInfo.kind=kHIThemeFrameTextFieldSquare;
                aTextDrawInfo.state=getState( nState );
                aTextDrawInfo.isFocused=false;

                
                
                rc.size.width -= SPIN_BUTTON_SPACE + SPIN_BUTTON_WIDTH + 2*FOCUS_RING_WIDTH;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;

                
                CGContextFillRect (mrContext, CGRectMake(rc.origin.x, rc.origin.y, rc.size.width, rc.size.height));
                

                HIThemeDrawFrame(&rc, &aTextDrawInfo, mrContext, kHIThemeOrientationNormal);

                if(nState & CTRL_STATE_FOCUSED) HIThemeDrawFocusRect(&rc, true, mrContext, kHIThemeOrientationNormal);

                
                const SpinbuttonValue* pSpinButtonVal = (aValue.getType() == CTRL_SPINBUTTONS) ? static_cast<const SpinbuttonValue*>(&aValue) : NULL;
                ControlState nUpperState = CTRL_STATE_ENABLED;
                ControlState nLowerState = CTRL_STATE_ENABLED;
                if(pSpinButtonVal) {
                    nUpperState = (ControlState) pSpinButtonVal->mnUpperState;
                    nLowerState = (ControlState) pSpinButtonVal->mnLowerState;

                    HIThemeButtonDrawInfo aSpinInfo;
                    aSpinInfo.kind = kThemeIncDecButton;
                    aSpinInfo.state = kThemeStateActive;
                    if(nUpperState & CTRL_STATE_PRESSED)
                        aSpinInfo.state = kThemeStatePressedUp;
                    else if(nLowerState & CTRL_STATE_PRESSED)
                        aSpinInfo.state = kThemeStatePressedDown;
                    else if((nUpperState & ~CTRL_STATE_ENABLED)||(nLowerState & ~CTRL_STATE_ENABLED))
                        aSpinInfo.state = kThemeStateInactive;
                    else if((nUpperState & CTRL_STATE_ROLLOVER)||(nLowerState & CTRL_STATE_ROLLOVER))
                        aSpinInfo.state = kThemeStateRollover;

                    Rectangle aSpinRect( pSpinButtonVal->maUpperRect );
                    aSpinRect.Union( pSpinButtonVal->maLowerRect );
                    HIRect buttonRc = ImplGetHIRectFromRectangle(aSpinRect);

                    
                    if( Application::GetSettings().GetLayoutRTL() )
                        buttonRc.origin.x -= FOCUS_RING_WIDTH - CLIP_FUZZ;
                    else
                        buttonRc.origin.x += FOCUS_RING_WIDTH + CLIP_FUZZ;

                    switch( aValue.getTristateVal() )
                    {
                        case BUTTONVALUE_ON:        aSpinInfo.value = kThemeButtonOn;
                                                    break;
                        case BUTTONVALUE_OFF:       aSpinInfo.value = kThemeButtonOff;
                                                    break;
                        case BUTTONVALUE_MIXED:
                        case BUTTONVALUE_DONTKNOW:
                        default:                    aSpinInfo.value = kThemeButtonMixed;
                                                    break;
                    }

                    aSpinInfo.adornment = ( ((nUpperState & CTRL_STATE_DEFAULT) != 0 ) ||
                                            ((nLowerState & CTRL_STATE_DEFAULT) != 0 )) ?
                                       kThemeAdornmentDefault :
                                       kThemeAdornmentNone;
                    if( ((nUpperState & CTRL_STATE_FOCUSED) != 0 ) || ((nLowerState & CTRL_STATE_FOCUSED) != 0 ))
                        aSpinInfo.adornment |= kThemeAdornmentFocus;

                    HIThemeDrawButton( &buttonRc, &aSpinInfo, mrContext, kHIThemeOrientationNormal, NULL );
                }

                bOK=true;
            }

        }
        break;

    case CTRL_FRAME:
        {
            sal_uInt16 nStyle = aValue.getNumericVal();
            if( nPart == PART_BORDER ) {
                if(!( nStyle & FRAME_DRAW_MENU ) && !(nStyle & FRAME_DRAW_WINDOWBORDER) )
                {
                    
                    
                    
                    if( rc.origin.y + rc.size.height >= mpFrame->maGeometry.nHeight-3 )
                    {
                        CGMutablePathRef rPath = CGPathCreateMutable();
                        CGPathAddRect( rPath, NULL, CGRectMake( 0, 0, mpFrame->maGeometry.nWidth-1, mpFrame->maGeometry.nHeight-1 ) );

                        CGContextBeginPath( mrContext );
                        CGContextAddPath( mrContext, rPath );
                        CGContextClip( mrContext );
                        CGPathRelease( rPath );
                    }

                    HIThemeFrameDrawInfo aTextDrawInfo;
                    aTextDrawInfo.version=0;
                    aTextDrawInfo.kind=kHIThemeFrameListBox;
                    aTextDrawInfo.state=kThemeStateActive;
                    aTextDrawInfo.isFocused=false;

                    HIThemeDrawFrame(&rc, &aTextDrawInfo, mrContext, kHIThemeOrientationNormal);

                    bOK=true;
                }
            }
        }
        break;

    case CTRL_LISTNET:
        {
           
            bOK=true;
        }
        break;

    }

    CGContextRestoreGState( mrContext );

    /* #i90291# in most cases invalidating the whole control region instead
       of just the unclipped part of it is sufficient (and probably faster).
       However for the window background we should not unnecessarily enlarge
       the really changed rectangle since the difference is usually quite high
       (the background is always drawn as a whole since we don't know anything
       about its possible contents)
    */
    if( nType == CTRL_WINDOW_BACKGROUND )
    {
        CGRect aRect = { { 0, 0 }, { 0, 0 } };
        if( mxClipPath )
            aRect = CGPathGetBoundingBox( mxClipPath );
        if( aRect.size.width != 0 && aRect.size.height != 0 )
            buttonRect.Intersection( Rectangle( Point( static_cast<long int>(aRect.origin.x),
                            static_cast<long int>(aRect.origin.y) ),
                                                Size(   static_cast<long int>(aRect.size.width),
                            static_cast<long int>(aRect.size.height) ) ) );
    }

    RefreshRect( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight() );

    return bOK;
}

/*
 * GetNativeControlRegion()
 *
 *  If the return value is sal_True, rNativeBoundingRegion
 *  contains the true bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  aCaption:       A caption or title string (like button text etc)
 */
bool AquaSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState /*nState*/,
                                               const ImplControlValue& aValue, const OUString&,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )

{
    sal_Bool toReturn = sal_False;

    Rectangle aCtrlBoundRect( rControlRegion );
    short x = aCtrlBoundRect.Left();
    short y = aCtrlBoundRect.Top();
    short w, h;

    sal_uInt8 nBorderCleanup = 0;

    switch (nType)
    {
        case CTRL_SLIDER:
            {
                if( nPart == PART_THUMB_HORZ )
                {
                    w = 19; 
                    h = aCtrlBoundRect.GetHeight();
                    rNativeBoundingRegion = rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    toReturn = true;
                }
                else if( nPart == PART_THUMB_VERT )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = 18; 
                    rNativeBoundingRegion = rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    toReturn = true;
                }
            }
            break;

        case CTRL_SCROLLBAR:
            {
                Rectangle aRect;
                if( AquaGetScrollRect( /* m_nScreen */ nPart, aCtrlBoundRect, aRect ) )
                {
                    toReturn = sal_True;
                    rNativeBoundingRegion = aRect;
                    rNativeContentRegion = aRect;
                }
            }
            break;

        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
            {
                if ( nType == CTRL_PUSHBUTTON )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = aCtrlBoundRect.GetHeight();
                }
                else
                {
                    
                    nBorderCleanup = 4;

                    
                    w = BUTTON_WIDTH + TEXT_SEPARATOR;
                    h = BUTTON_HEIGHT;

                }

                rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h + nBorderCleanup) );
                rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

                toReturn = sal_True;
            }
            break;
        case CTRL_PROGRESS:
            {
                Rectangle aRect( aCtrlBoundRect );
                if( aRect.GetHeight() < 16 )
                    aRect.Bottom() = aRect.Top() + 9; 
                else
                    aRect.Bottom() = aRect.Top() + 15; 
                rNativeBoundingRegion = aRect;
                rNativeContentRegion = aRect;
                toReturn = sal_True;
            }
            break;

        case CTRL_INTROPROGRESS:
            {
                Rectangle aRect( aCtrlBoundRect );
                aRect.Bottom() = aRect.Top() + INTRO_PROGRESS_HEIGHT; 
                rNativeBoundingRegion = aRect;
                rNativeContentRegion = aRect;
                toReturn = sal_True;
            }
            break;

         case CTRL_TAB_ITEM:

            w = aCtrlBoundRect.GetWidth() + 2*TAB_TEXT_OFFSET - 2*VCL_TAB_TEXT_OFFSET;

#ifdef OLD_TAB_STYLE
            h = TAB_HEIGHT_NORMAL;
#else
            h = TAB_HEIGHT_NORMAL+2;
#endif
            rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
            rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

            toReturn = sal_True;

            break;

        case CTRL_EDITBOX:
            {
                w = aCtrlBoundRect.GetWidth();
                if( w < 3+2*FOCUS_RING_WIDTH )
                    w = 3+2*FOCUS_RING_WIDTH;
                h = TEXT_EDIT_HEIGHT_NORMAL+2*FOCUS_RING_WIDTH;
                if( h < aCtrlBoundRect.GetHeight() )
                    h = aCtrlBoundRect.GetHeight();

                rNativeContentRegion = Rectangle( Point( x+FOCUS_RING_WIDTH, y+FOCUS_RING_WIDTH ), Size( w-2*(FOCUS_RING_WIDTH+1), h-2*(FOCUS_RING_WIDTH+1) ) );
                rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

                toReturn = sal_True;
            }
            break;
        case CTRL_LISTBOX:
        case CTRL_COMBOBOX:
            {
                if( nPart == PART_ENTIRE_CONTROL )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = COMBOBOX_HEIGHT_NORMAL;

                    rNativeContentRegion = Rectangle( Point( x+FOCUS_RING_WIDTH, y+FOCUS_RING_WIDTH ), Size( w-2*FOCUS_RING_WIDTH, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = sal_True;
                }
                else if( nPart == PART_BUTTON_DOWN )
                {
                    w = aCtrlBoundRect.GetWidth();
                    if( w < 3+2*FOCUS_RING_WIDTH )
                        w = 3+2*FOCUS_RING_WIDTH;
                    h = COMBOBOX_HEIGHT_NORMAL;

                    x += w-DROPDOWN_BUTTON_WIDTH - FOCUS_RING_WIDTH;
                    y += FOCUS_RING_WIDTH;
                    w = DROPDOWN_BUTTON_WIDTH;

                    rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w+FOCUS_RING_WIDTH, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
                else if( nPart == PART_SUB_EDIT )
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = COMBOBOX_HEIGHT_NORMAL;

                    x += FOCUS_RING_WIDTH;
                    x += 3; 
                    y += 2; 
                    y += FOCUS_RING_WIDTH;
                    w -= 3 + DROPDOWN_BUTTON_WIDTH + 2*FOCUS_RING_WIDTH;
                    if( nType == CTRL_LISTBOX )
                        w -= 9; 
                    h -= 4; 

                    rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w+FOCUS_RING_WIDTH, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = true;
                }
            }
            break;
        case CTRL_SPINBOX:
                if( nPart == PART_ENTIRE_CONTROL ) {
                    w = aCtrlBoundRect.GetWidth();
                    if( w < 3+2*FOCUS_RING_WIDTH+SPIN_BUTTON_SPACE+SPIN_BUTTON_WIDTH )
                        w = 3+2*FOCUS_RING_WIDTH+SPIN_BUTTON_SPACE+SPIN_BUTTON_WIDTH;
                    h = TEXT_EDIT_HEIGHT_NORMAL;

                    rNativeContentRegion = Rectangle( Point( x+FOCUS_RING_WIDTH, y ), Size( w-2*FOCUS_RING_WIDTH, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = sal_True;
                }
                else if( nPart == PART_SUB_EDIT ) {
                    w = aCtrlBoundRect.GetWidth() - SPIN_BUTTON_SPACE - SPIN_BUTTON_WIDTH;
                    h = TEXT_EDIT_HEIGHT_NORMAL;
                    x += 4; 
                    y += 2; 
                    w -= 8; 
                    h -= 4; 

                    rNativeContentRegion = Rectangle( Point( x + FOCUS_RING_WIDTH, y + FOCUS_RING_WIDTH ), Size( w - 2* FOCUS_RING_WIDTH, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h+2*FOCUS_RING_WIDTH ) );

                    toReturn = sal_True;
                }
                else if( nPart == PART_BUTTON_UP ) {
                    
                    
                    
                    x += aCtrlBoundRect.GetWidth() - SPIN_BUTTON_WIDTH - SPIN_BUTTON_SPACE - CLIP_FUZZ;
                    y += FOCUS_RING_WIDTH - CLIP_FUZZ;
                    w = SPIN_BUTTON_WIDTH + 2*CLIP_FUZZ;
                    h = SPIN_UPPER_BUTTON_HEIGHT + 2*CLIP_FUZZ;

                    rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

                    toReturn = sal_True;
                }
                else if( nPart == PART_BUTTON_DOWN ) {
                    x += aCtrlBoundRect.GetWidth() - SPIN_BUTTON_WIDTH - SPIN_BUTTON_SPACE - CLIP_FUZZ;
                    y += SPIN_UPPER_BUTTON_HEIGHT + FOCUS_RING_WIDTH - CLIP_FUZZ;
                    w = SPIN_BUTTON_WIDTH + 2*CLIP_FUZZ;
                    h = SPIN_LOWER_BUTTON_HEIGHT + 2*CLIP_FUZZ;

                    rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

                    toReturn = sal_True;
                }
            break;
        case CTRL_FRAME:
            {
                sal_uInt16 nStyle = aValue.getNumericVal();
                if(  ( nPart == PART_BORDER ) &&
                    !( nStyle & (FRAME_DRAW_MENU | FRAME_DRAW_WINDOWBORDER | FRAME_DRAW_BORDERWINDOWBORDER) ) )
                {
                    Rectangle aRect(aCtrlBoundRect);
                    if( nStyle & FRAME_DRAW_DOUBLEIN )
                    {
                        aRect.Left()    += 1;
                        aRect.Top()     += 1;
                        
                        
                    }
                    else
                    {
                        aRect.Left()    += 1;
                        aRect.Top()     += 1;
                        aRect.Right()   -= 1;
                        aRect.Bottom()  -= 1;
                    }

                    rNativeContentRegion = aRect;
                    rNativeBoundingRegion = aRect;

                    toReturn = sal_True;
                }
            }
            break;

        case CTRL_MENUBAR:
        case CTRL_MENU_POPUP:
            {
                if(( nPart == PART_MENU_ITEM_CHECK_MARK )||( nPart == PART_MENU_ITEM_RADIO_MARK )) {

                    w=10;
                    h=10;

                    rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h ) );
                    rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

                    toReturn = sal_True;
                }
            }
            break;

    }

    return toReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
