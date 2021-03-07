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
#include <tools/long.hxx>
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

// presentation of native widgets consists of two important methods:

// AquaSalGraphics::getNativeControlRegion to determine native rectangle in pixels to draw the widget
// AquaSalGraphics::drawNativeControl to do the drawing operation itself

// getNativeControlRegion has to calculate a content rectangle within it is safe to draw the widget. Furthermore a bounding rectangle
// has to be calculated by getNativeControlRegion to consider adornments like a focus rectangle. As drawNativeControl uses Carbon
// API calls, all widgets are drawn without text. Drawing of text is done separately by VCL on top of graphical Carbon widget
// representation. drawNativeControl is called by VCL using content rectangle determined by getNativeControlRegion.

// FIXME: when calculation bounding rectangle larger then content rectangle, text displayed by VCL will become misaligned. To avoid
// misalignment bounding rectangle and content rectangle are calculated equally including adornments. Reduction of size for content
// is done by drawNativeControl subsequently. Only exception is editbox: As other widgets have distinct ControlPart::SubEdit control
// parts, editbox bounding rectangle and content rectangle are both calculated to reflect content area. Extending size for
// adornments is done by drawNativeControl subsequently.

#if !HAVE_FEATURE_MACOSX_SANDBOX

@interface NSWindow(CoreUIRendererPrivate)
+ (CUIRendererRef)coreUIRenderer;
@end

#endif

static HIRect ImplGetHIRectFromRectangle(tools::Rectangle aRect)
{
    HIRect aHIRect;
    aHIRect.origin.x = static_cast<float>(aRect.Left());
    aHIRect.origin.y = static_cast<float>(aRect.Top());
    aHIRect.size.width = static_cast<float>(aRect.GetWidth());
    aHIRect.size.height = static_cast<float>(aRect.GetHeight());
    return aHIRect;
}

static ThemeButtonValue ImplGetButtonValue(ButtonValue aButtonValue)
{
    switch (aButtonValue)
    {
        case ButtonValue::On:
            return kThemeButtonOn;
        case ButtonValue::Off:
        case ButtonValue::DontKnow:
            return kThemeButtonOff;
        case ButtonValue::Mixed:
        default:
            return kThemeButtonMixed;
    }
}

static bool AquaGetScrollRect(/* TODO: int nScreen, */
                              ControlPart nPart, const tools::Rectangle &rControlRect, tools::Rectangle &rResultRect)
{
    bool bRetVal = true;
    rResultRect = rControlRect;
    switch (nPart)
    {
        case ControlPart::ButtonUp:
            rResultRect.SetBottom(rResultRect.Top());
            break;
        case ControlPart::ButtonDown:
            rResultRect.SetTop(rResultRect.Bottom());
            break;
        case ControlPart::ButtonLeft:
            rResultRect.SetRight(rResultRect.Left());
            break;
        case ControlPart::ButtonRight:
            rResultRect.SetLeft(rResultRect.Right());
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

bool AquaSalGraphics::isNativeControlSupported(ControlType nType, ControlPart nPart)
{
    // native controls are now defaults. If you want to disable native controls, set the environment variable SAL_NO_NWF to
    // something and VCL controls will be used as default again.

    switch (nType)
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
        case ControlType::ListNode:
            if (nPart == ControlPart::Entire)
                return true;
            break;
        case ControlType::Scrollbar:
            if (nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert
                || nPart == ControlPart::Entire || nPart == ControlPart::HasThreeButtons)
                return true;
            break;
        case ControlType::Slider:
            if (nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea)
                return true;
            break;
        case ControlType::Editbox:
            if (nPart == ControlPart::Entire || nPart == ControlPart::HasBackgroundTexture)
                return true;
            break;
        case ControlType::MultilineEditbox:
            if (nPart == ControlPart::Entire || nPart == ControlPart::HasBackgroundTexture)
                return true;
            break;
        case ControlType::Spinbox:
            if (nPart == ControlPart::Entire || nPart == ControlPart::AllButtons || nPart == ControlPart::HasBackgroundTexture)
                return true;
            break;
        case ControlType::SpinButtons:
            return false;
        case ControlType::Combobox:
            if (nPart == ControlPart::Entire || nPart == ControlPart::HasBackgroundTexture)
                return true;
            break;
        case ControlType::Listbox:
            if (nPart == ControlPart::Entire || nPart == ControlPart::ListboxWindow || nPart == ControlPart::HasBackgroundTexture
                || nPart == ControlPart::SubEdit)
                return true;
            break;
        case ControlType::TabItem:
        case ControlType::TabPane:
        case ControlType::TabBody:
            if (nPart == ControlPart::Entire || nPart == ControlPart::TabsDrawRtl || nPart == ControlPart::HasBackgroundTexture)
                return true;
            break;
        case ControlType::Toolbar:
            if (nPart == ControlPart::Entire || nPart == ControlPart::DrawBackgroundHorz
                || nPart == ControlPart::DrawBackgroundVert)
                return true;
            break;
        case  ControlType::WindowBackground:
            if (nPart == ControlPart::BackgroundWindow || nPart == ControlPart::BackgroundDialog)
                 return true;
            break;
        case ControlType::Menubar:
            if (nPart == ControlPart::Entire)
                return true;
            break;
        case ControlType::Tooltip:
            if (nPart == ControlPart::Entire)
                return true;
            break;
        case ControlType::MenuPopup:
            if (nPart == ControlPart::Entire || nPart == ControlPart::MenuItem || nPart == ControlPart::MenuItemCheckMark
                || nPart == ControlPart::MenuItemRadioMark)
                return true;
            break;
        case ControlType::Progress:
        case ControlType::IntroProgress:
            if (nPart == ControlPart::Entire)
                return true;
            break;
        case ControlType::Frame:
            if (nPart == ControlPart::Border)
                return true;
            break;
        case ControlType::ListNet:
            if (nPart == ControlPart::Entire)
                return true;
            break;
        default:
            break;
    }
    return false;
}

bool AquaSalGraphics::hitTestNativeControl(ControlType nType, ControlPart nPart, const tools::Rectangle &rControlRegion,
                                           const Point &rPos, bool& rIsInside)
{
    if (nType == ControlType::Scrollbar)
    {
        tools::Rectangle aRect;
        bool bValid = AquaGetScrollRect(/* TODO: int nScreen, */
                                        nPart, rControlRegion, aRect);
        rIsInside = bValid && aRect.IsInside(rPos);
        return bValid;
    }
    return false;
}

UInt32 AquaSalGraphics::getState(ControlState nState)
{

    // there are non key windows which are children of key windows, e.g. autofilter configuration dialog or sidebar dropdown dialogs.
    // To handle these windows correctly, parent frame's key window state is considered here additionally.

    const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow]
                             || mpFrame->mpParent == nullptr || [mpFrame->mpParent->getNSWindow() isKeyWindow];
    if (!(nState & ControlState::ENABLED) || !bDrawActive)
    {
        return kThemeStateInactive;
    }
    if (nState & ControlState::PRESSED)
        return kThemeStatePressed;
    return kThemeStateActive;
}

UInt32 AquaSalGraphics::getTrackState(ControlState nState)
{
    const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow];
    if (!(nState & ControlState::ENABLED) || !bDrawActive)
        return kThemeTrackInactive;
    return kThemeTrackActive;
}

bool AquaSalGraphics::drawNativeControl(ControlType nType,
                                        ControlPart nPart,
                                        const tools::Rectangle &rControlRegion,
                                        ControlState nState,
                                        const ImplControlValue &aValue,
                                        const OUString &,
                                        const Color&)
{
    bool bOK = false;
    if (!CheckContext())
        return false;
    maContextHolder.saveState();
    tools::Rectangle buttonRect = rControlRegion;
    HIRect rc = ImplGetHIRectFromRectangle(buttonRect);
    switch (nType)
    {
        case ControlType::Toolbar:
            {
#if HAVE_FEATURE_MACOSX_SANDBOX
                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
                aMenuItemDrawInfo.version = 0;
                aMenuItemDrawInfo.state = kThemeMenuActive;
                aMenuItemDrawInfo.itemType = kThemeMenuItemHierBackground;
                HIThemeDrawMenuItem(&rc, &rc, &aMenuItemDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
#else
                if (rControlRegion.Top() == 0 && nPart == ControlPart::DrawBackgroundHorz)
                {
                    const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow];
                    CGFloat unifiedHeight = rControlRegion.GetHeight();
                    CGRect drawRect = CGRectMake(rControlRegion.Left(), rControlRegion.Top(),
                                                 rControlRegion.GetWidth(), rControlRegion.GetHeight());
                    CUIDraw([NSWindow coreUIRenderer], drawRect, maContextHolder.get(),
                            reinterpret_cast<CFDictionaryRef>([NSDictionary dictionaryWithObjectsAndKeys:
                                                               @"kCUIWidgetWindowFrame",
                                                               @"widget",
                                                               @"regularwin",
                                                               @"windowtype",
                                                               (bDrawActive ? @"normal" : @"inactive"),
                                                               @"state",
                                                               [NSNumber numberWithDouble:unifiedHeight],
                                                               @"kCUIWindowFrameUnifiedTitleBarHeightKey",
                                                               [NSNumber numberWithBool:NO],
                                                               @"kCUIWindowFrameDrawTitleSeparatorKey",
                                                               [NSNumber numberWithBool:YES],
                                                               @"is.flipped",
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
                aThemeBackgroundInfo.state = getState(nState);
                aThemeBackgroundInfo.kind = kThemeBrushDialogBackgroundActive;

                // FIXME: without this magical offset there is a 2 pixel black border on the right and bottom

                rc.size.width += 2;
                rc.size.height += 2;
                HIThemeApplyBackground( &rc, &aThemeBackgroundInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                CGContextFillRect(maContextHolder.get(), rc);
                bOK = true;
            }
            break;
        case ControlType::Tooltip:
            {
                HIThemeBackgroundDrawInfo aThemeBackgroundInfo;
                aThemeBackgroundInfo.version = 0;
                aThemeBackgroundInfo.state = getState(nState);
                aThemeBackgroundInfo.kind = kThemeBrushAlertBackgroundActive;
                rc.size.width += 2;
                rc.size.height += 2;
                HIThemeApplyBackground(&rc, &aThemeBackgroundInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                CGContextFillRect(maContextHolder.get(), rc);
                bOK = true;
            }
            break;
        case ControlType::Menubar:
        case ControlType::MenuPopup:
            if (nPart == ControlPart::Entire || nPart == ControlPart::MenuItem || nPart == ControlPart::HasBackgroundTexture)
            {

                // FIXME: without this magical offset there is a 2 pixel black border on the right

                rc.size.width += 2;
                HIThemeMenuDrawInfo aMenuInfo;
                aMenuInfo.version = 0;
                aMenuInfo.menuType = kThemeMenuTypePullDown;
                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;

                // grey theme when the item is selected is drawn here.

                aMenuItemDrawInfo.itemType = kThemeMenuItemPlain;
                if ((nPart == ControlPart::MenuItem) && (nState & ControlState::SELECTED))

                    // blue theme when the item is selected is drawn here.

                    aMenuItemDrawInfo.state = kThemeMenuSelected;
                else

                    // normal color for non selected item

                    aMenuItemDrawInfo.state = kThemeMenuActive;

                // repaints the background of the pull down menu

                HIThemeDrawMenuBackground(&rc, &aMenuInfo,maContextHolder.get(), kHIThemeOrientationNormal);

                // repaints the item either blue (selected) and/or grey (active only)

                HIThemeDrawMenuItem(&rc, &rc, &aMenuItemDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal, &rc);
                bOK = true;
            }
            else if (nPart == ControlPart::MenuItemCheckMark || nPart == ControlPart::MenuItemRadioMark)
            {

                // checked, else it is not displayed (see vcl/source/window/menu.cxx)

                if (nState & ControlState::PRESSED)
                {
                    HIThemeTextInfo aTextInfo;
                    aTextInfo.version = 0;
                    aTextInfo.state = (nState & ControlState::ENABLED) ? kThemeStateInactive: kThemeStateActive;
                    aTextInfo.fontID = kThemeMenuItemMarkFont;
                    aTextInfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
                    aTextInfo.verticalFlushness = kHIThemeTextVerticalFlushTop;
                    aTextInfo.options = kHIThemeTextBoxOptionNone;
                    aTextInfo.truncationPosition = kHIThemeTextTruncationNone;

                    // aTextInfo.truncationMaxLines unused because of kHIThemeTextTruncationNone item highlighted

                    if (nState & ControlState::SELECTED) aTextInfo.state = kThemeStatePressed;
                    UniChar mark=(nPart == ControlPart::MenuItemCheckMark) ? kCheckUnicode: kBulletUnicode;
                    CFStringRef cfString = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, &mark, 1, kCFAllocatorNull);
                    HIThemeDrawTextBox(cfString, &rc, &aTextInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                    if (cfString)
                        CFRelease(cfString);
                    bOK = true;
                }
            }
            break;
        case ControlType::Pushbutton:
            {

                // FIXME: instead of use a value, VCL can retrieve correct values on the fly (to be implemented)

                HIThemeButtonDrawInfo aPushInfo;
                aPushInfo.version = 0;

                // no animation

                aPushInfo.animation.time.start = 0;
                aPushInfo.animation.time.current = 0;
                PushButtonValue const *pPBVal = aValue.getType() == ControlType::Pushbutton ?
                                                static_cast<PushButtonValue const *>(&aValue) : nullptr;
                int nPaintHeight = static_cast<int>(rc.size.height);
                if (pPBVal && pPBVal->mbBevelButton)
                {
                    aPushInfo.kind = kThemeRoundedBevelButton;
                }
                else if (rc.size.height <= PUSH_BUTTON_NORMAL_HEIGHT)
                {
                    aPushInfo.kind = kThemePushButtonMini;
                    nPaintHeight = PUSH_BUTTON_SMALL_HEIGHT;
                }
                else if ((pPBVal && pPBVal->mbSingleLine) || rc.size.height < PUSH_BUTTON_NORMAL_HEIGHT * 3 / 2)
                {
                    aPushInfo.kind = kThemePushButtonNormal;
                    nPaintHeight = PUSH_BUTTON_NORMAL_HEIGHT;

                    // avoid clipping when focused

                    rc.origin.x += FOCUS_RING_WIDTH / 2;
                    rc.size.width -= FOCUS_RING_WIDTH;
                }
                else
                    aPushInfo.kind = kThemeBevelButton;

                // translate the origin for controls with fixed paint height so content ends up somewhere sensible

                rc.origin.y += (rc.size.height - nPaintHeight) / 2;
                aPushInfo.state = getState(nState);
                aPushInfo.value = ImplGetButtonValue(aValue.getTristateVal());
                aPushInfo.adornment = (nState & ControlState::DEFAULT) ? kThemeAdornmentDefault : kThemeAdornmentNone;
                if (nState & ControlState::FOCUSED)
                    aPushInfo.adornment |= kThemeAdornmentFocus;
                HIThemeDrawButton(&rc, &aPushInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                bOK = true;
            }
            break;
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            {
                HIThemeButtonDrawInfo aInfo;
                aInfo.version = 0;
                switch (nType)
                {
                    case ControlType::Radiobutton:
                        if (rc.size.width >= RADIO_BUTTON_SMALL_SIZE)
                            aInfo.kind = kThemeRadioButton;
                        else
                            aInfo.kind = kThemeSmallRadioButton;
                        break;
                    case ControlType::Checkbox:
                        if (rc.size.width >= CHECKBOX_SMALL_SIZE)
                            aInfo.kind = kThemeCheckBox;
                        else
                            aInfo.kind = kThemeSmallCheckBox;
                        break;
                    default:
                        break;
                }
                aInfo.state = getState(nState);
                ButtonValue aButtonValue = aValue.getTristateVal();
                aInfo.value = ImplGetButtonValue(aButtonValue);
                aInfo.adornment = (nState & ControlState::DEFAULT) ? kThemeAdornmentDefault : kThemeAdornmentNone;
                if (nState & ControlState::FOCUSED)
                    aInfo.adornment |= kThemeAdornmentFocus;
                rc.size.width -= 2 * FOCUS_RING_WIDTH;
                rc.size.height = RADIO_BUTTON_SMALL_SIZE;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;
                HIThemeDrawButton(&rc, &aInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                bOK = true;
            }
            break;
        case ControlType::ListNode:
            {
                ButtonValue aButtonValue = aValue.getTristateVal();
                HIThemeButtonDrawInfo aInfo;
                aInfo.version = 0;
                aInfo.kind = kThemeDisclosureTriangle;
                aInfo.value = kThemeDisclosureRight;
                aInfo.state = getState(nState);
                aInfo.adornment = kThemeAdornmentNone;
                switch (aButtonValue)
                {
                    case ButtonValue::On:
                        aInfo.value = kThemeDisclosureDown;
                        break;
                    case ButtonValue::Off:
                        if (AllSettings::GetLayoutRTL())
                            aInfo.value = kThemeDisclosureLeft;
                        break;
                    case ButtonValue::DontKnow:
                    default:
                        break;
                }
                HIThemeDrawButton(&rc, &aInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                bOK = true;
            }
            break;
        case ControlType::Progress:
        case ControlType::IntroProgress:
            {
                tools::Long nProgressWidth = aValue.getNumericVal();
                HIThemeTrackDrawInfo aTrackInfo;
                aTrackInfo.version = 0;
                aTrackInfo.kind  = (rc.size.height > 10) ? kThemeProgressBarLarge : kThemeProgressBarMedium;
                aTrackInfo.bounds  = rc;
                aTrackInfo.min  = 0;
                aTrackInfo.max  = static_cast<SInt32>(rc.size.width);
                aTrackInfo.value  = nProgressWidth;
                aTrackInfo.reserved  = 0;
                aTrackInfo.attributes = kThemeTrackHorizontal;
                if (AllSettings::GetLayoutRTL())
                    aTrackInfo.attributes |= kThemeTrackRightToLeft;
                aTrackInfo.enableState  = getTrackState(nState);

                // the intro bitmap never gets key anyway; we want to draw that enabled

                if (nType == ControlType::IntroProgress)
                    aTrackInfo.enableState  = kThemeTrackActive;
                aTrackInfo.filler1  = 0;
                aTrackInfo.trackInfo.progress.phase = static_cast<long long>(CFAbsoluteTimeGetCurrent() * 10.0);
                HIThemeDrawTrack(&aTrackInfo, nullptr, maContextHolder.get(), kHIThemeOrientationNormal);
                bOK = true;
            }
            break;
        case ControlType::Slider:
            {
                const SliderValue *pSliderVal = static_cast<SliderValue const *>(&aValue);
                HIThemeTrackDrawInfo aTrackDraw;
                aTrackDraw.kind = kThemeSliderMedium;
                if (nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea)
                {
                    aTrackDraw.bounds = rc;
                    aTrackDraw.min = pSliderVal->mnMin;
                    aTrackDraw.max = pSliderVal->mnMax;
                    aTrackDraw.value = pSliderVal->mnCur;
                    aTrackDraw.reserved = 0;
                    aTrackDraw.attributes = kThemeTrackShowThumb;
                    if (nPart == ControlPart::TrackHorzArea)
                        aTrackDraw.attributes |= kThemeTrackHorizontal;
                    aTrackDraw.enableState = (nState & ControlState::ENABLED) ? kThemeTrackActive : kThemeTrackInactive;
                    SliderTrackInfo aSlideInfo;
                    aSlideInfo.thumbDir = kThemeThumbUpward;
                    aSlideInfo.pressState = 0;
                    aTrackDraw.trackInfo.slider = aSlideInfo;
                    HIThemeDrawTrack(&aTrackDraw, nullptr, maContextHolder.get(), kHIThemeOrientationNormal);
                    bOK = true;
                }
            }
            break;
        case ControlType::Scrollbar:
            {
                const ScrollbarValue *pScrollbarVal = (aValue.getType() == ControlType::Scrollbar)
                                                    ? static_cast<const ScrollbarValue *>(&aValue) : nullptr;
                if (nPart == ControlPart::DrawBackgroundVert || nPart == ControlPart::DrawBackgroundHorz)
                {
                    HIThemeTrackDrawInfo aTrackDraw;
                    aTrackDraw.kind = kThemeMediumScrollBar;
                    aTrackDraw.bounds = rc;
                    aTrackDraw.min = pScrollbarVal->mnMin;
                    aTrackDraw.max = pScrollbarVal->mnMax - pScrollbarVal->mnVisibleSize;
                    aTrackDraw.value = pScrollbarVal->mnCur;
                    aTrackDraw.reserved = 0;
                    aTrackDraw.attributes = kThemeTrackShowThumb;
                    if (nPart == ControlPart::DrawBackgroundHorz)
                        aTrackDraw.attributes |= kThemeTrackHorizontal;
                    aTrackDraw.enableState = getTrackState(nState);
                    ScrollBarTrackInfo aScrollInfo;
                    aScrollInfo.viewsize = pScrollbarVal->mnVisibleSize;
                    aScrollInfo.pressState = 0;
                    if (pScrollbarVal->mnButton1State & ControlState::ENABLED)
                        if (pScrollbarVal->mnButton1State & ControlState::PRESSED)
                            aScrollInfo.pressState = kThemeTopOutsideArrowPressed;
                    if (pScrollbarVal->mnButton2State & ControlState::ENABLED )
                        if (pScrollbarVal->mnButton2State & ControlState::PRESSED )
                            aScrollInfo.pressState = kThemeBottomOutsideArrowPressed;
                    if ( pScrollbarVal->mnThumbState & ControlState::ENABLED)
                        if (pScrollbarVal->mnThumbState & ControlState::PRESSED)
                            aScrollInfo.pressState = kThemeThumbPressed;
                    aTrackDraw.trackInfo.scrollbar = aScrollInfo;
                    HIThemeDrawTrack(&aTrackDraw, nullptr, maContextHolder.get(), kHIThemeOrientationNormal);
                    bOK = true;
                }
            }
            break;
        case ControlType::TabPane:
            {
                HIThemeTabPaneDrawInfo aTabPaneDrawInfo;
                aTabPaneDrawInfo.version = 1;
                aTabPaneDrawInfo.state = kThemeStateActive;
                aTabPaneDrawInfo.direction = kThemeTabNorth;
                aTabPaneDrawInfo.size = kHIThemeTabSizeNormal;
                aTabPaneDrawInfo.kind = kHIThemeTabKindNormal;

                // border is outside the rect rc for Carbon but for VCL it should be inside

                rc.origin.x += 1;
                rc.origin.y -= TAB_HEIGHT / 2;
                rc.size.height += TAB_HEIGHT / 2;
                rc.size.width -= 2;
                HIThemeDrawTabPane(&rc, &aTabPaneDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                bOK = true;
            }
            break;
        case ControlType::TabItem:
            {
                HIThemeTabDrawInfo aTabItemDrawInfo;
                aTabItemDrawInfo.version = 1;
                aTabItemDrawInfo.style = kThemeTabNonFront;
                aTabItemDrawInfo.direction = kThemeTabNorth;
                aTabItemDrawInfo.size = kHIThemeTabSizeNormal;
                aTabItemDrawInfo.adornment = kHIThemeTabAdornmentTrailingSeparator;
                if (nState & ControlState::SELECTED)
                    aTabItemDrawInfo.style = kThemeTabFront;
                if(nState & ControlState::FOCUSED)
                    aTabItemDrawInfo.adornment |= kHIThemeTabAdornmentFocus;

                // first, last or middle tab

                aTabItemDrawInfo.position = kHIThemeTabPositionMiddle;
                TabitemValue const * pTabValue = static_cast<TabitemValue const *>(&aValue);
                TabitemFlags nAlignment = pTabValue->mnAlignment;

                // TabitemFlags::LeftAligned (and TabitemFlags::RightAligned) for the leftmost (or rightmost) tab
                // when there are several lines of tabs because there is only one first tab and one
                // last tab and TabitemFlags::FirstInGroup (and TabitemFlags::LastInGroup) because when the
                // line width is different from window width, there may not be TabitemFlags::RightAligned

                if (((nAlignment & TabitemFlags::LeftAligned) && (nAlignment & TabitemFlags::RightAligned))
                    || ((nAlignment & TabitemFlags::FirstInGroup) && (nAlignment & TabitemFlags::LastInGroup)))
                    aTabItemDrawInfo.position = kHIThemeTabPositionOnly;
                else if ((nAlignment & TabitemFlags::LeftAligned) || (nAlignment & TabitemFlags::FirstInGroup))
                    aTabItemDrawInfo.position = kHIThemeTabPositionFirst;
                else if ((nAlignment & TabitemFlags::RightAligned) || (nAlignment & TabitemFlags::LastInGroup))
                    aTabItemDrawInfo.position = kHIThemeTabPositionLast;

                // support for RTL (see issue 79748)

                if (AllSettings::GetLayoutRTL()) {
                    if (aTabItemDrawInfo.position == kHIThemeTabPositionFirst)
                        aTabItemDrawInfo.position = kHIThemeTabPositionLast;
                    else if (aTabItemDrawInfo.position == kHIThemeTabPositionLast)
                        aTabItemDrawInfo.position = kHIThemeTabPositionFirst;
                }
                HIThemeDrawTab(&rc, &aTabItemDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                bOK=true;
            }
            break;
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
            {
                HIThemeFrameDrawInfo aTextDrawInfo;
                aTextDrawInfo.version = 0;
                aTextDrawInfo.kind = kHIThemeFrameTextFieldSquare;
                aTextDrawInfo.state = getState(nState);
                aTextDrawInfo.isFocused = false;
                rc.size.width += 2 * EDITBOX_INSET_MARGIN;
                if (nType == ControlType::Editbox)
                  rc.size.height = EDITBOX_HEIGHT;
                else
                  rc.size.height += 2 * (EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN);
                rc.origin.x -= EDITBOX_INSET_MARGIN;
                rc.origin.y -= EDITBOX_INSET_MARGIN;

                // fill a white background, because HIThemeDrawFrame only draws the border

                CGContextFillRect(maContextHolder.get(), CGRectMake(rc.origin.x, rc.origin.y, rc.size.width, rc.size.height));
                HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                if (nState & ControlState::FOCUSED)
                    HIThemeDrawFocusRect(&rc, true, maContextHolder.get(), kHIThemeOrientationNormal);
                bOK = true;
            }
            break;
        case ControlType::Combobox:
            if (nPart == ControlPart::HasBackgroundTexture || nPart == ControlPart::Entire)
            {
                HIThemeButtonDrawInfo aComboInfo;
                aComboInfo.version = 0;
                aComboInfo.kind = kThemeComboBox;
                aComboInfo.state = getState(nState);
                aComboInfo.value = kThemeButtonOn;
                aComboInfo.adornment = kThemeAdornmentNone;
                if (nState & ControlState::FOCUSED)
                    aComboInfo.adornment |= kThemeAdornmentFocus;
                rc.size.width -= 2 * FOCUS_RING_WIDTH;
                rc.size.height = COMBOBOX_HEIGHT;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;
                HIThemeDrawButton(&rc, &aComboInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                bOK = true;
            }
            break;
        case ControlType::Listbox:
            switch (nPart)
            {
                case ControlPart::Entire:
                case ControlPart::ButtonDown:
                    HIThemeButtonDrawInfo aListInfo;
                    aListInfo.version = 0;
                    aListInfo.kind = kThemePopupButton;
                    aListInfo.state = getState(nState);
                    aListInfo.value = kThemeButtonOn;
                    aListInfo.adornment = kThemeAdornmentDefault;
                    if (nState & ControlState::FOCUSED)
                        aListInfo.adornment |= kThemeAdornmentFocus;
                    rc.size.width -= 2 * FOCUS_RING_WIDTH;
                    rc.size.height = LISTBOX_HEIGHT;
                    rc.origin.x += FOCUS_RING_WIDTH;
                    rc.origin.y += FOCUS_RING_WIDTH;
                    HIThemeDrawButton(&rc, &aListInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                    bOK = true;
                    break;
                case ControlPart::ListboxWindow:
                    HIThemeFrameDrawInfo aTextDrawInfo;
                    aTextDrawInfo.version = 0;
                    aTextDrawInfo.kind = kHIThemeFrameListBox;
                    aTextDrawInfo.state = kThemeStateActive;
                    aTextDrawInfo.isFocused = false;
                    HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                    bOK = true;
                    break;
                default:
                    break;
            }
            break;
        case ControlType::Spinbox:
            if (nPart == ControlPart::Entire)
            {

                // text field

                HIThemeFrameDrawInfo aTextDrawInfo;
                aTextDrawInfo.version = 0;
                aTextDrawInfo.kind = kHIThemeFrameTextFieldSquare;
                aTextDrawInfo.state = getState(nState);
                aTextDrawInfo.isFocused = false;
                rc.size.width -= SPIN_BUTTON_WIDTH + 4 * FOCUS_RING_WIDTH;
                rc.size.height = EDITBOX_HEIGHT;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;

                // fill a white background, because HIThemeDrawFrame only draws the border

                CGContextFillRect(maContextHolder.get(), CGRectMake(rc.origin.x, rc.origin.y, rc.size.width, rc.size.height));
                HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                if (nState & ControlState::FOCUSED)
                    HIThemeDrawFocusRect(&rc, true, maContextHolder.get(), kHIThemeOrientationNormal);

                // buttons

                const SpinbuttonValue *pSpinButtonVal = (aValue.getType() == ControlType::SpinButtons)
                                                      ? static_cast <const SpinbuttonValue *>(&aValue) : nullptr;
                ControlState nUpperState = ControlState::ENABLED;
                ControlState nLowerState = ControlState::ENABLED;
                if (pSpinButtonVal)
                {
                    nUpperState = pSpinButtonVal->mnUpperState;
                    nLowerState = pSpinButtonVal->mnLowerState;
                    HIThemeButtonDrawInfo aSpinInfo;
                    aSpinInfo.kind = kThemeIncDecButton;
                    aSpinInfo.state = kThemeStateActive;
                    if (nUpperState & ControlState::PRESSED)
                        aSpinInfo.state = kThemeStatePressedUp;
                    else if (nLowerState & ControlState::PRESSED)
                        aSpinInfo.state = kThemeStatePressedDown;
                    else if (nUpperState & ~ControlState::ENABLED || nLowerState & ~ControlState::ENABLED)
                        aSpinInfo.state = kThemeStateInactive;
                    else if (nUpperState & ControlState::ROLLOVER || nLowerState & ControlState::ROLLOVER)
                        aSpinInfo.state = kThemeStateRollover;
                    switch (aValue.getTristateVal())
                    {
                        case ButtonValue::On:
                            aSpinInfo.value = kThemeButtonOn;
                            break;
                        case ButtonValue::Off:
                            aSpinInfo.value = kThemeButtonOff;
                            break;
                        case ButtonValue::Mixed:
                        case ButtonValue::DontKnow:
                        default:
                            aSpinInfo.value = kThemeButtonMixed;
                            break;
                    }
                    aSpinInfo.adornment = (nUpperState & ControlState::DEFAULT || nLowerState & ControlState::DEFAULT)
                                        ? kThemeAdornmentDefault : kThemeAdornmentNone;
                    if (nUpperState & ControlState::FOCUSED || nLowerState & ControlState::FOCUSED)
                        aSpinInfo.adornment |= kThemeAdornmentFocus;
                    rc.origin.x += rc.size.width + FOCUS_RING_WIDTH + 1;
                    rc.origin.y -= 1;
                    rc.size.width = SPIN_BUTTON_WIDTH;
                    rc.size.height = SPIN_LOWER_BUTTON_HEIGHT + SPIN_LOWER_BUTTON_HEIGHT;
                    HIThemeDrawButton(&rc, &aSpinInfo, maContextHolder.get(), kHIThemeOrientationNormal, nullptr);
                }
                bOK = true;
            }
            break;
        case ControlType::Frame:
            {
                DrawFrameFlags nStyle = static_cast<DrawFrameFlags>(aValue.getNumericVal());
                if (nPart == ControlPart::Border)
                {
                    if (!(nStyle & DrawFrameFlags::Menu) && !(nStyle & DrawFrameFlags::WindowBorder))
                    {

                        // strange effects start to happen when HIThemeDrawFrame meets the border of the window.
                        // These can be avoided by clipping to the boundary of the frame (see issue 84756)

                        if (rc.origin.y + rc.size.height >= mpFrame->maGeometry.nHeight - 3)
                        {
                            CGMutablePathRef rPath = CGPathCreateMutable();
                            CGPathAddRect(rPath, nullptr,
                                          CGRectMake(0, 0, mpFrame->maGeometry.nWidth - 1, mpFrame->maGeometry.nHeight - 1));
                            CGContextBeginPath(maContextHolder.get());
                            CGContextAddPath(maContextHolder.get(), rPath);
                            CGContextClip(maContextHolder.get());
                            CGPathRelease(rPath);
                        }
                        HIThemeFrameDrawInfo aTextDrawInfo;
                        aTextDrawInfo.version = 0;
                        aTextDrawInfo.kind = kHIThemeFrameListBox;
                        aTextDrawInfo.state = kThemeStateActive;
                        aTextDrawInfo.isFocused = false;
                        HIThemeDrawFrame(&rc, &aTextDrawInfo, maContextHolder.get(), kHIThemeOrientationNormal);
                        bOK = true;
                    }
                }
            }
            break;
        case ControlType::ListNet:

            // do nothing as there isn't net for listviews on macOS

            bOK = true;
            break;
        default:
            break;
    }
    maContextHolder.restoreState();

    // in most cases invalidating the whole control region instead of just the unclipped part of it is sufficient (and probably
    // faster). However for the window background we should not unnecessarily enlarge the really changed rectangle since the
    // difference is usually quite high. Background is always drawn as a whole since we don't know anything about its possible
    // contents (see issue i90291).

    if (nType == ControlType::WindowBackground)
    {
        CGRect aRect = {{0, 0}, {0, 0}};
        if (mxClipPath)
            aRect = CGPathGetBoundingBox(mxClipPath);
        if (aRect.size.width != 0 && aRect.size.height != 0)
            buttonRect.Intersection(tools::Rectangle(Point(static_cast<tools::Long>(aRect.origin.x),
                                                           static_cast<tools::Long>(aRect.origin.y)),
                                                     Size(static_cast<tools::Long>(aRect.size.width),
                                                          static_cast<tools::Long>(aRect.size.height))));
    }
    RefreshRect(buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight());
    return bOK;
}

bool AquaSalGraphics::getNativeControlRegion(ControlType nType,
                                             ControlPart nPart,
                                             const tools::Rectangle &rControlRegion,
                                             ControlState,
                                             const ImplControlValue &aValue,
                                             const OUString &,
                                             tools::Rectangle &rNativeBoundingRegion,
                                             tools::Rectangle &rNativeContentRegion)
{
    bool toReturn = false;
    tools::Rectangle aCtrlBoundRect(rControlRegion);
    short x = aCtrlBoundRect.Left();
    short y = aCtrlBoundRect.Top();
    short w, h;
    switch (nType)
    {
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            {
                if (nType == ControlType::Pushbutton)
                {
                    w = aCtrlBoundRect.GetWidth();
                    h = aCtrlBoundRect.GetHeight();
                }
                else
                {
                    w = RADIO_BUTTON_SMALL_SIZE + 2 * FOCUS_RING_WIDTH + RADIO_BUTTON_TEXT_SEPARATOR;
                    h = RADIO_BUTTON_SMALL_SIZE + 2 * FOCUS_RING_WIDTH;
                }
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Progress:
            {
                tools::Rectangle aRect(aCtrlBoundRect);
                if (aRect.GetHeight() < LARGE_PROGRESS_INDICATOR_HEIGHT)
                    aRect.SetBottom(aRect.Top() + MEDIUM_PROGRESS_INDICATOR_HEIGHT - 1);
                else
                    aRect.SetBottom(aRect.Top() + LARGE_PROGRESS_INDICATOR_HEIGHT - 1);
                rNativeBoundingRegion = aRect;
                rNativeContentRegion = aRect;
                toReturn = true;
            }
            break;
        case ControlType::IntroProgress:
            {
                tools::Rectangle aRect(aCtrlBoundRect);
                aRect.SetBottom(aRect.Top() + MEDIUM_PROGRESS_INDICATOR_HEIGHT - 1);
                rNativeBoundingRegion = aRect;
                rNativeContentRegion = aRect;
                toReturn = true;
            }
            break;
        case ControlType::Slider:
            if (nPart == ControlPart::ThumbHorz)
            {
                w = SLIDER_WIDTH;
                h = aCtrlBoundRect.GetHeight();
                rNativeBoundingRegion = rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::ThumbVert)
            {
                w = aCtrlBoundRect.GetWidth();
                h = SLIDER_HEIGHT;
                rNativeBoundingRegion = rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Scrollbar:
            {
                tools::Rectangle aRect;
                if (AquaGetScrollRect(nPart, aCtrlBoundRect, aRect))
                {
                    toReturn = true;
                    rNativeBoundingRegion = aRect;
                    rNativeContentRegion = aRect;
                }
            }
            break;
        case ControlType::TabItem:
            {
                w = aCtrlBoundRect.GetWidth() + 2 * TAB_TEXT_MARGIN;
                h = TAB_HEIGHT + 2;
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Editbox:
            {
                w = aCtrlBoundRect.GetWidth();
                h = EDITBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                w -= 2 * (FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN);
                h -= 2 * (FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN);
                x += FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN;
                y += FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN;
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Combobox:
          if (nPart == ControlPart::Entire)
            {
                w = aCtrlBoundRect.GetWidth();
                h = COMBOBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::ButtonDown)
            {
                w = COMBOBOX_BUTTON_WIDTH + FOCUS_RING_WIDTH;
                h = COMBOBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                x += aCtrlBoundRect.GetWidth() - w;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::SubEdit)
            {
                w = aCtrlBoundRect.GetWidth() - 2 * FOCUS_RING_WIDTH - COMBOBOX_BUTTON_WIDTH - COMBOBOX_BORDER_WIDTH
                    - 2 * COMBOBOX_TEXT_MARGIN;
                h = COMBOBOX_HEIGHT - 2 * COMBOBOX_BORDER_WIDTH;
                x += FOCUS_RING_WIDTH + COMBOBOX_BORDER_WIDTH + COMBOBOX_TEXT_MARGIN;
                y += FOCUS_RING_WIDTH + COMBOBOX_BORDER_WIDTH;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Listbox:
            if (nPart == ControlPart::Entire)
            {
                w = aCtrlBoundRect.GetWidth();
                h = LISTBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::ButtonDown)
            {
                w = LISTBOX_BUTTON_WIDTH + FOCUS_RING_WIDTH;
                h = LISTBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                x += aCtrlBoundRect.GetWidth() - w;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::SubEdit)
            {
                w = aCtrlBoundRect.GetWidth() - 2 * FOCUS_RING_WIDTH - LISTBOX_BUTTON_WIDTH - LISTBOX_BORDER_WIDTH
                    - 2 * LISTBOX_TEXT_MARGIN;
                h = LISTBOX_HEIGHT - 2 * LISTBOX_BORDER_WIDTH;
                x += FOCUS_RING_WIDTH + LISTBOX_BORDER_WIDTH + LISTBOX_TEXT_MARGIN;
                y += FOCUS_RING_WIDTH + LISTBOX_BORDER_WIDTH;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::ListboxWindow)
            {
                w = aCtrlBoundRect.GetWidth() - 2;
                h = aCtrlBoundRect.GetHeight() - 2;
                x += 1;
                y += 1;
                rNativeBoundingRegion = aCtrlBoundRect;
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Spinbox:
            if (nPart == ControlPart::Entire)
            {
                w = aCtrlBoundRect.GetWidth();
                h = EDITBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                x += SPINBOX_OFFSET;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::SubEdit)
            {
                w = aCtrlBoundRect.GetWidth() - 4 * FOCUS_RING_WIDTH - SPIN_BUTTON_WIDTH - 2 * EDITBOX_BORDER_WIDTH
                    - 2 * EDITBOX_INSET_MARGIN;
                h = EDITBOX_HEIGHT - 2 * (EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN);
                x += FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN + SPINBOX_OFFSET;
                y += FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::ButtonUp)
            {
                w = SPIN_BUTTON_WIDTH +  2 * FOCUS_RING_WIDTH;
                h = SPIN_UPPER_BUTTON_HEIGHT + FOCUS_RING_WIDTH;
                x += aCtrlBoundRect.GetWidth() - SPIN_BUTTON_WIDTH - 2 * FOCUS_RING_WIDTH + SPINBOX_OFFSET;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            else if (nPart == ControlPart::ButtonDown)
            {
                w = SPIN_BUTTON_WIDTH + 2 * FOCUS_RING_WIDTH;
                h = SPIN_LOWER_BUTTON_HEIGHT + FOCUS_RING_WIDTH;
                x += aCtrlBoundRect.GetWidth() - SPIN_BUTTON_WIDTH - 2 * FOCUS_RING_WIDTH + SPINBOX_OFFSET;
                y += FOCUS_RING_WIDTH + SPIN_UPPER_BUTTON_HEIGHT;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        case ControlType::Frame:
            {
                DrawFrameStyle nStyle = static_cast<DrawFrameStyle>(aValue.getNumericVal() & 0x000f);
                DrawFrameFlags nFlags = static_cast<DrawFrameFlags>(aValue.getNumericVal() & 0xfff0);
                if (nPart == ControlPart::Border
                    && !(nFlags & (DrawFrameFlags::Menu | DrawFrameFlags::WindowBorder | DrawFrameFlags::BorderWindowBorder)))
                {
                    tools::Rectangle aRect(aCtrlBoundRect);
                    if (nStyle == DrawFrameStyle::DoubleIn)
                    {
                        aRect.AdjustLeft(1);
                        aRect.AdjustTop(1);
                        // rRect.Right() -= 1;
                        // rRect.Bottom() -= 1;
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
            if (nPart == ControlPart::MenuItemCheckMark || nPart == ControlPart::MenuItemRadioMark)
            {
                w=10;
                h=10;
                rNativeContentRegion = tools::Rectangle(Point(x, y), Size(w, h));
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                toReturn = true;
            }
            break;
        default:
            break;
    }
    return toReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
