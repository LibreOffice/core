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
#include <vcl/threadex.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/themecolors.hxx>

#include <quartz/salgdi.h>
#include <osx/salnativewidgets.h>
#include <osx/saldata.hxx>
#include <osx/salframe.h>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#if HAVE_FEATURE_SKIA
#include <vcl/skia/SkiaHelper.hxx>
#endif

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

static NSControlStateValue ImplGetButtonValue(ButtonValue aButtonValue)
{
    switch (aButtonValue)
    {
        case ButtonValue::On:
            return NSControlStateValueOn;
        case ButtonValue::Off:
        case ButtonValue::DontKnow:
            return NSControlStateValueOff;
        case ButtonValue::Mixed:
        default:
            return NSControlStateValueMixed;
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
        case ControlType::LevelBar:
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
        rIsInside = bValid && aRect.Contains(rPos);
        return bValid;
    }
    return false;
}

static bool getEnabled(ControlState nState, AquaSalFrame* mpFrame)
{

    // there are non key windows which are children of key windows, e.g. autofilter configuration dialog or sidebar dropdown dialogs.
    // To handle these windows correctly, parent frame's key window state is considered here additionally.

    const bool bDrawActive = mpFrame == nullptr || [mpFrame->getNSWindow() isKeyWindow]
                             || mpFrame->mpParent == nullptr || [mpFrame->mpParent->getNSWindow() isKeyWindow];
    if (!(nState & ControlState::ENABLED) || !bDrawActive)
    {
        return false;
    }
    return true;
}

bool AquaSalGraphics::drawNativeControl(ControlType nType,
                                        ControlPart nPart,
                                        const tools::Rectangle &rControlRegion,
                                        ControlState nState,
                                        const ImplControlValue &aValue,
                                        const OUString &,
                                        const Color&)
{
    return mpBackend->drawNativeControl(nType, nPart, rControlRegion, nState, aValue);
}

static NSColor* colorFromRGB(const Color& rColor)
{
    return [NSColor colorWithSRGBRed:(rColor.GetRed() / 255.0f)
                               green:(rColor.GetGreen() / 255.0f)
                                blue:(rColor.GetBlue() / 255.0f)
                               alpha:(rColor.GetAlpha() / 255.0f)];
}

static void paintCell(NSCell* pBtn, const NSRect& bounds, bool bShowsFirstResponder, CGContextRef context, NSView* pView)
{
    //translate and scale because up side down otherwise
    CGContextSaveGState(context);
    CGContextTranslateCTM(context, bounds.origin.x, bounds.origin.y + bounds.size.height);
    CGContextScaleCTM(context, 1, -1);

    NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
    [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:context flipped:NO]];

    NSRect rect = { NSZeroPoint, bounds.size };

    if ([pBtn isKindOfClass: [NSSliderCell class]])
    {
        // NSSliderCell doesn't seem to work with drawWithFrame(?), so draw the elements directly
        [static_cast<NSSliderCell*>(pBtn)
            drawBarInside: [static_cast<NSSliderCell*>(pBtn) barRectFlipped: NO] flipped: NO];
        rect = [static_cast<NSSliderCell*>(pBtn) knobRectFlipped: NO];
        [static_cast<NSSliderCell*>(pBtn) drawKnob: rect];
    }
    else
        [pBtn drawWithFrame: rect inView: pView];

    // setShowsFirstResponder apparently causes a hang when set on NSComboBoxCell
    const bool bIsComboBox = [pBtn isMemberOfClass: [NSComboBoxCell class]];
    if (!bIsComboBox)
        [pBtn setShowsFirstResponder: bShowsFirstResponder];

    if (bShowsFirstResponder)
    {
        NSSetFocusRingStyle(NSFocusRingOnly);

        CGContextBeginTransparencyLayerWithRect(context, rect, nullptr);
        if ([pBtn isMemberOfClass: [NSTextFieldCell class]])
        {
            // I wonder why NSTextFieldCell doesn't work for me in the default else branch.
            // NSComboBoxCell works, and that derives from NSTextFieldCell, on the other
            // hand setShowsFirstResponder causes a hangs when set on NSComboBoxCell
            NSRect out = [pBtn focusRingMaskBoundsForFrame: rect inView: pView];
            CGContextFillRect(context, out);
        }
        else if ([pBtn isKindOfClass: [NSSliderCell class]])
        {
            // Not getting anything useful for a NSSliderCell, so use the knob
            [static_cast<NSSliderCell*>(pBtn) drawKnob: rect];
        }
        else
            [pBtn drawFocusRingMaskWithFrame:rect inView: pView];

        CGContextEndTransparencyLayer(context);
    }

    [NSGraphicsContext setCurrentContext:savedContext];
    CGContextRestoreGState(context);
}

static void paintFocusRect(double radius, const NSRect& rect, CGContextRef context)
{
    NSRect bounds = rect;

    CGPathRef path = CGPathCreateWithRoundedRect(bounds, radius, radius, nullptr);
    CGContextSetStrokeColorWithColor(context, [NSColor keyboardFocusIndicatorColor].CGColor);
    CGContextSetLineWidth(context, FOCUS_RING_WIDTH);
    CGContextBeginPath(context);
    CGContextAddPath(context, path);
    CGContextStrokePath(context);
    CFRelease(path);
}

@interface FixedWidthTabViewItem : NSTabViewItem {
    int m_nWidth;
}
- (NSSize)sizeOfLabel: (BOOL)computeMin;
- (void)setTabWidth: (int)nWidth;
@end

@implementation FixedWidthTabViewItem
- (NSSize)sizeOfLabel: (BOOL)computeMin
{
    NSSize size = [super sizeOfLabel: computeMin];
    size.width = m_nWidth;
    return size;
}
- (void)setTabWidth: (int)nWidth
{
    m_nWidth = nWidth;
}
@end

bool AquaGraphicsBackend::drawNativeControl(ControlType nType,
                                            ControlPart nPart,
                                            const tools::Rectangle &rControlRegion,
                                            ControlState nState,
                                            const ImplControlValue &aValue)
{
    if (!mrShared.checkContext())
        return false;
    mrShared.maContextHolder.saveState();
    bool bOK = performDrawNativeControl(nType, nPart, rControlRegion, nState, aValue,
                                        mrShared.maContextHolder.get(), mrShared.mpFrame);
    mrShared.maContextHolder.restoreState();

    tools::Rectangle buttonRect = rControlRegion;

    // in most cases invalidating the whole control region instead of just the unclipped part of it is sufficient (and probably
    // faster). However for the window background we should not unnecessarily enlarge the really changed rectangle since the
    // difference is usually quite high. Background is always drawn as a whole since we don't know anything about its possible
    // contents (see issue i90291).

    if (nType == ControlType::WindowBackground)
    {
        CGRect aRect = {{0, 0}, {0, 0}};
        if (mrShared.mxClipPath)
            aRect = CGPathGetBoundingBox(mrShared.mxClipPath);
        if (aRect.size.width != 0 && aRect.size.height != 0)
            buttonRect.Intersection(tools::Rectangle(Point(static_cast<tools::Long>(aRect.origin.x),
                                                           static_cast<tools::Long>(aRect.origin.y)),
                                                     Size(static_cast<tools::Long>(aRect.size.width),
                                                          static_cast<tools::Long>(aRect.size.height))));
    }
    mrShared.refreshRect(buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight());
    return bOK;
}

static void drawBox(CGContextRef context, const NSRect& rc, NSColor* pColor)
{
    assert(pColor);

    CGContextSaveGState(context);
    CGContextTranslateCTM(context, rc.origin.x, rc.origin.y + rc.size.height);
    CGContextScaleCTM(context, 1, -1);

    NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:NO];

    NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
    NSBox* pBox = [[NSBox alloc] initWithFrame: rect];
    [pBox setBoxType: NSBoxCustom];
    [pBox setFillColor: pColor];

    // -[NSBox setBorderType: NSNoBorder] is deprecated so hide the border
    // by setting the border color to transparent
    [pBox setBorderColor: [NSColor clearColor]];
    [pBox setTitlePosition: NSNoTitle];

    [pBox displayRectIgnoringOpacity: rect inContext: graphicsContext];

    [pBox release];

    CGContextRestoreGState(context);
}

// if I don't crystallize this bg then the InvertCursor using kCGBlendModeDifference doesn't
// work correctly and the cursor doesn't appear correctly
static void drawEditableBackground(CGContextRef context, const NSRect& rc)
{
    CGContextSaveGState(context);
    if (ThemeColors::IsThemeLoaded())
        CGContextSetFillColorWithColor(context, colorFromRGB(ThemeColors::GetThemeColors().GetBaseColor()).CGColor);
    else
        CGContextSetFillColorWithColor(context, [NSColor controlBackgroundColor].CGColor);
    CGContextFillRect(context, rc);
    CGContextRestoreGState(context);
}

// As seen in macOS 12.3.1. All a bit odd really.
const int RoundedMargin[4] = { 6, 4, 0, 3 };

bool AquaGraphicsBackendBase::performDrawNativeControl(ControlType nType,
                                ControlPart nPart,
                                const tools::Rectangle &rControlRegion,
                                ControlState nState,
                                const ImplControlValue &aValue,
                                CGContextRef context,
                                AquaSalFrame* mpFrame)
{
    bool bOK = false;
    bool bThemeLoaded(ThemeColors::IsThemeLoaded());
    AquaSalInstance* pInst = GetSalData()->mpInstance;
    HIRect rc = ImplGetHIRectFromRectangle(rControlRegion);
    switch (nType)
    {
        case ControlType::Toolbar:
            {
                if (bThemeLoaded)
                    drawBox(context, rc, colorFromRGB(ThemeColors::GetThemeColors().GetWindowColor()));
                else
                    drawBox(context, rc, NSColor.windowBackgroundColor);
                bOK = true;
            }
            break;
        case ControlType::WindowBackground:
            {
                if (bThemeLoaded)
                    drawBox(context, rc, colorFromRGB(ThemeColors::GetThemeColors().GetWindowColor()));
                else
                    drawBox(context, rc, NSColor.windowBackgroundColor);
                bOK = true;
            }
            break;
        case ControlType::Tooltip:
            {
                rc.size.width += 2;
                rc.size.height += 2;
                if (bThemeLoaded)
                    drawBox(context, rc, colorFromRGB(ThemeColors::GetThemeColors().GetBaseColor()));
                else
                    drawBox(context, rc, NSColor.controlBackgroundColor);
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

                HIThemeDrawMenuBackground(&rc, &aMenuInfo, context, kHIThemeOrientationNormal);

                // repaints the item either blue (selected) and/or grey (active only)

                HIThemeDrawMenuItem(&rc, &rc, &aMenuItemDrawInfo, context, kHIThemeOrientationNormal, &rc);
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
                    HIThemeDrawTextBox(cfString, &rc, &aTextInfo, context, kHIThemeOrientationNormal);
                    if (cfString)
                        CFRelease(cfString);
                    bOK = true;
                }
            }
            break;
        case ControlType::Pushbutton:
            {
                NSControlSize eSizeKind = NSControlSizeRegular;
                NSBezelStyle eBezelStyle = NSBezelStyleRounded;

                PushButtonValue const *pPBVal = aValue.getType() == ControlType::Pushbutton ?
                                                static_cast<PushButtonValue const *>(&aValue) : nullptr;

                SInt32 nPaintHeight = rc.size.height;
                if (rc.size.height <= PUSH_BUTTON_NORMAL_HEIGHT)
                {
                    eSizeKind = NSControlSizeMini;
                    GetThemeMetric(kThemeMetricSmallPushButtonHeight, &nPaintHeight);
                }
                else if ((pPBVal && pPBVal->mbSingleLine) || rc.size.height < PUSH_BUTTON_NORMAL_HEIGHT * 3 / 2)
                {
                    GetThemeMetric(kThemeMetricPushButtonHeight, &nPaintHeight);
                }
                else
                {
                    // A simple square bezel style that can scale to any size
                    eBezelStyle = NSBezelStyleSmallSquare;
                }

                // translate the origin for controls with fixed paint height so content ends up somewhere sensible
                rc.origin.y += (rc.size.height - nPaintHeight + 1) / 2;
                rc.size.height = nPaintHeight;

                NSButtonCell* pBtn = pInst->mpButtonCell;
                pBtn.allowsMixedState = YES;

                [pBtn setTitle: @""];
                [pBtn setButtonType: NSButtonTypeMomentaryPushIn];
                [pBtn setBezelStyle: eBezelStyle];
                [pBtn setState: ImplGetButtonValue(aValue.getTristateVal())];
                [pBtn setEnabled: getEnabled(nState, mpFrame)];
                [pBtn setFocusRingType: NSFocusRingTypeExterior];
                [pBtn setHighlighted: (nState & ControlState::PRESSED) ? YES : NO];
                [pBtn setControlSize: eSizeKind];
                if (nState & ControlState::DEFAULT)
                    [pBtn setKeyEquivalent: @"\r"];
                else
                    [pBtn setKeyEquivalent: @""];

                if (eBezelStyle == NSBezelStyleRounded)
                {
                    int nMargin = RoundedMargin[eSizeKind];
                    rc.origin.x -= nMargin;
                    rc.size.width += nMargin * 2;

                    rc.origin.x += FOCUS_RING_WIDTH / 2;
                    rc.size.width -= FOCUS_RING_WIDTH;
                }

                const bool bFocused(nState & ControlState::FOCUSED);
                paintCell(pBtn, rc, bFocused, context, nullptr);

                bOK = true;
            }
            break;
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            {
                rc.size.width -= 2 * FOCUS_RING_WIDTH;
                rc.size.height = RADIO_BUTTON_SMALL_SIZE;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;

                NSButtonCell* pBtn = nType == ControlType::Checkbox ? pInst->mpCheckCell : pInst->mpRadioCell;
                pBtn.allowsMixedState = YES;

                [pBtn setTitle: @""];
                [pBtn setButtonType: nType == ControlType::Checkbox ? NSButtonTypeSwitch : NSButtonTypeRadio];
                [pBtn setState: ImplGetButtonValue(aValue.getTristateVal())];
                [pBtn setEnabled: getEnabled(nState, mpFrame)];
                [pBtn setFocusRingType: NSFocusRingTypeExterior];
                [pBtn setHighlighted: (nState & ControlState::PRESSED) ? YES : NO];

                const bool bFocused(nState & ControlState::FOCUSED);
                paintCell(pBtn, rc, bFocused, context, nullptr);

                bOK = true;
            }
            break;
        case ControlType::ListNode:
            {
                NSButtonCell* pBtn = pInst->mpListNodeCell;
                pBtn.allowsMixedState = YES;

                [pBtn setTitle: @""];
                [pBtn setButtonType: NSButtonTypeOnOff];
                [pBtn setBezelStyle: NSBezelStyleDisclosure];
                [pBtn setState: ImplGetButtonValue(aValue.getTristateVal())];
                [pBtn setEnabled: getEnabled(nState, mpFrame)];
                [pBtn setFocusRingType: NSFocusRingTypeExterior];

                const bool bFocused(nState & ControlState::FOCUSED);
                paintCell(pBtn, rc, bFocused, context, nullptr);

                bOK = true;
            }
            break;
        case ControlType::LevelBar:
            {
                NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
                NSLevelIndicator* pBox = [[NSLevelIndicator alloc] initWithFrame:rect];
                [pBox setLevelIndicatorStyle: NSLevelIndicatorStyleContinuousCapacity];
                [pBox setMinValue: 0];
                [pBox setMaxValue: rc.size.width];
                [pBox setCriticalValue: rc.size.width * 35.0 / 100.0];
                [pBox setWarningValue: rc.size.width * 70.0 / 100.0];
                [pBox setDoubleValue: aValue.getNumericVal()];

                CGContextSaveGState(context);
                CGContextTranslateCTM(context, rc.origin.x, rc.origin.y);

                NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
                NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:NO];
                [NSGraphicsContext setCurrentContext: graphicsContext];

                [pBox drawRect: rect];

                [NSGraphicsContext setCurrentContext: savedContext];

                CGContextRestoreGState(context);

                [pBox release];

                bOK = true;
            }
            break;
        case ControlType::Progress:
        case ControlType::IntroProgress:
            {
                NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
                NSProgressIndicator* pBox = [[NSProgressIndicator alloc] initWithFrame: rect];
                [pBox setControlSize: (rc.size.height > MEDIUM_PROGRESS_INDICATOR_HEIGHT) ?
                                      NSControlSizeRegular : NSControlSizeSmall];
                [pBox setMinValue: 0];
                [pBox setMaxValue: rc.size.width];
                [pBox setDoubleValue: aValue.getNumericVal()];
                pBox.usesThreadedAnimation = NO;
                [pBox setIndeterminate: NO];

                CGContextSaveGState(context);
                CGContextTranslateCTM(context, rc.origin.x, rc.origin.y);

                NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
                NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:NO];
                [NSGraphicsContext setCurrentContext: graphicsContext];

                [pBox drawRect: rect];

                [NSGraphicsContext setCurrentContext: savedContext];

                CGContextRestoreGState(context);

                [pBox release];

#if HAVE_FEATURE_SKIA
                // tdf#164428 Skia/Metal needs flush after drawing progress bar
                if (SkiaHelper::isVCLSkiaEnabled() && SkiaHelper::renderMethodToUse() != SkiaHelper::RenderRaster)
                    mpFrame->mbForceFlushProgressBar = true;
#endif

                bOK = true;
            }
            break;
        case ControlType::Slider:
            {
                const SliderValue *pSliderVal = static_cast<SliderValue const *>(&aValue);
                if (nPart == ControlPart::TrackHorzArea || nPart == ControlPart::TrackVertArea)
                {
                    NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
                    NSSlider* pBox = [[NSSlider alloc] initWithFrame: rect];

                    [pBox setEnabled: getEnabled(nState, mpFrame)];
                    [pBox setVertical: nPart == ControlPart::TrackVertArea];
                    [pBox setMinValue: pSliderVal->mnMin];
                    [pBox setMaxValue: pSliderVal->mnMax];
                    [pBox setIntegerValue: pSliderVal->mnCur];
                    [pBox setSliderType: NSSliderTypeLinear];
                    [pBox setFocusRingType: NSFocusRingTypeExterior];

                    const bool bFocused(nState & ControlState::FOCUSED);
                    paintCell(pBox.cell, rc, bFocused, context, mpFrame->getNSView());

                    [pBox release];

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
                    if (bThemeLoaded)
                        drawBox(context, rc, colorFromRGB(ThemeColors::GetThemeColors().GetBaseColor()));
                    else
                        drawBox(context, rc, NSColor.controlBackgroundColor);

                    NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
                    NSScroller* pBar = [[NSScroller alloc] initWithFrame: rect];

                    double range = pScrollbarVal->mnMax - pScrollbarVal->mnVisibleSize - pScrollbarVal->mnMin;
                    double value = range ? (pScrollbarVal->mnCur - pScrollbarVal->mnMin) / range : 0;

                    double length = pScrollbarVal->mnMax - pScrollbarVal->mnMin;
                    double proportion = pScrollbarVal->mnVisibleSize / length;

                    [pBar setEnabled: getEnabled(nState, mpFrame)];
                    [pBar setScrollerStyle: NSScrollerStyleLegacy];
                    [pBar setFloatValue: value];
                    [pBar setKnobProportion: proportion];
                    bool bPressed = (pScrollbarVal->mnThumbState & ControlState::ENABLED) &&
                                    (pScrollbarVal->mnThumbState & ControlState::PRESSED);

                    CGContextSaveGState(context);
                    CGContextTranslateCTM(context, rc.origin.x, rc.origin.y);

                    NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:NO];

                    NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
                    [NSGraphicsContext setCurrentContext: graphicsContext];

                    // For not-pressed first draw without the knob and then
                    // draw just the knob but with 50% opaque which looks sort of
                    // right

                    [pBar drawKnobSlotInRect: rect highlight: NO];

                    NSBitmapImageRep* pImageRep = [pBar bitmapImageRepForCachingDisplayInRect: rect];

                    NSGraphicsContext* imageContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:pImageRep];
                    [NSGraphicsContext setCurrentContext: imageContext];

                    [pBar drawKnob];

                    [NSGraphicsContext setCurrentContext: graphicsContext];

                    NSImage* pImage = [[NSImage alloc] initWithSize: rect.size];
                    [pImage addRepresentation: pImageRep]; // takes ownership of pImageRep

                    [pImage drawInRect: rect fromRect: rect
                                        operation: NSCompositingOperationSourceOver
                                        fraction: bPressed ? 1.0 : 0.5];

                    [pImage release];

                    [NSGraphicsContext setCurrentContext:savedContext];

                    CGContextRestoreGState(context);

                    bOK = true;

                    [pBar release];
                }
            }
            break;
        case ControlType::TabPane:
            {
                NSTabView* pBox = [[NSTabView alloc] initWithFrame: rc];

                SInt32 nOverlap;
                GetThemeMetric(kThemeMetricTabFrameOverlap, &nOverlap);

                // this calculation is probably more than a little dubious
                rc.origin.x -= pBox.contentRect.origin.x - FOCUS_RING_WIDTH;
                rc.size.width += rc.size.width - pBox.contentRect.size.width - 2 * FOCUS_RING_WIDTH;
                double nTopBorder = pBox.contentRect.origin.y;
                double nBottomBorder = rc.size.height - pBox.contentRect.size.height - nTopBorder;
                double nExtraTop = (nTopBorder - nBottomBorder) / 2;
                rc.origin.y -= (nTopBorder - nExtraTop + nOverlap);
                rc.size.height += (nTopBorder - nExtraTop + nBottomBorder);

                CGContextSaveGState(context);
                CGContextTranslateCTM(context, rc.origin.x, rc.origin.y);

                rc.origin.x = 0;
                rc.origin.y = 0;

                [pBox setBoundsOrigin: rc.origin];
                [pBox setBoundsSize: rc.size];

                // jam this in to force the tab contents area to be left undrawn, the ControlType::TabItem
                // will be drawn in this space.
                const TabPaneValue& rValue = static_cast<const TabPaneValue&>(aValue);
                SInt32 nEndCapWidth;
                GetThemeMetric(kThemeMetricLargeTabCapsWidth, &nEndCapWidth);
                FixedWidthTabViewItem* pItem = [[[FixedWidthTabViewItem alloc] initWithIdentifier: @"tab"] autorelease];
                [pItem setTabWidth: rValue.m_aTabHeaderRect.GetWidth() - 2 * nEndCapWidth];
                [pBox addTabViewItem: pItem];

                NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:NO];

                NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
                [NSGraphicsContext setCurrentContext: graphicsContext];

                [pBox drawRect: rc];

                [NSGraphicsContext setCurrentContext: savedContext];

                [pBox release];

                CGContextRestoreGState(context);

                bOK = true;
            }
            break;
        case ControlType::TabItem:
            {
                // first, last or middle tab

                TabitemValue const * pTabValue = static_cast<TabitemValue const *>(&aValue);
                TabitemFlags nAlignment = pTabValue->mnAlignment;

                // TabitemFlags::LeftAligned (and TabitemFlags::RightAligned) for the leftmost (or rightmost) tab
                // when there are several lines of tabs because there is only one first tab and one
                // last tab and TabitemFlags::FirstInGroup (and TabitemFlags::LastInGroup) because when the
                // line width is different from window width, there may not be TabitemFlags::RightAligned
                int nPaintIndex = 1;
                bool bSolo = false;
                if (((nAlignment & TabitemFlags::LeftAligned) && (nAlignment & TabitemFlags::RightAligned))
                    || ((nAlignment & TabitemFlags::FirstInGroup) && (nAlignment & TabitemFlags::LastInGroup)))
                {
                    nPaintIndex = 0;
                    bSolo = true;
                }
                else if ((nAlignment & TabitemFlags::LeftAligned) || (nAlignment & TabitemFlags::FirstInGroup))
                    nPaintIndex = !AllSettings::GetLayoutRTL() ? 0 : 2;
                else if ((nAlignment & TabitemFlags::RightAligned) || (nAlignment & TabitemFlags::LastInGroup))
                    nPaintIndex = !AllSettings::GetLayoutRTL() ? 2 : 0;

                int nCells = !bSolo ? 3 : 1;
                NSRect ctrlrect = { NSZeroPoint, NSMakeSize(rc.size.width * nCells + FOCUS_RING_WIDTH, rc.size.height) };
                NSSegmentedControl* pCtrl = [[NSSegmentedControl alloc] initWithFrame: ctrlrect];
                [pCtrl setSegmentCount: nCells];
                if (bSolo)
                    [pCtrl setWidth: rc.size.width + FOCUS_RING_WIDTH forSegment: 0];
                else
                {
                    [pCtrl setWidth: rc.size.width + FOCUS_RING_WIDTH/2 forSegment: 0];
                    [pCtrl setWidth: rc.size.width forSegment: 1];
                    [pCtrl setWidth: rc.size.width + FOCUS_RING_WIDTH/2 forSegment: 2];
                }
                [pCtrl setSelected: (nState & ControlState::SELECTED) ? YES : NO forSegment: nPaintIndex];
                [pCtrl setFocusRingType: NSFocusRingTypeExterior];

                NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
                [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:context flipped:NO]];

                NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
                NSRect tabrect = { NSMakePoint(rc.size.width * nPaintIndex + FOCUS_RING_WIDTH / 2, 0),
                                   NSMakeSize(rc.size.width, rc.size.height) };
                NSBitmapImageRep* pImageRep = [pCtrl bitmapImageRepForCachingDisplayInRect: tabrect];
                [pCtrl cacheDisplayInRect: tabrect toBitmapImageRep: pImageRep];

                NSImage* pImage = [[NSImage alloc] initWithSize: rect.size];
                [pImage addRepresentation: pImageRep]; // takes ownership of pImageRep

                [pImage drawInRect: rc fromRect: rect
                        operation: NSCompositingOperationSourceOver
                        fraction: 1.0];

                [pImage release];

                [NSGraphicsContext setCurrentContext:savedContext];

                [pCtrl release];

                if (nState & ControlState::FOCUSED)
                {
                    if (!bSolo)
                    {
                        if (nPaintIndex == 0)
                        {
                            rc.origin.x += FOCUS_RING_WIDTH / 2;
                            rc.size.width -= FOCUS_RING_WIDTH / 2;
                        }
                        else if (nPaintIndex == 2)
                        {
                            rc.size.width -= FOCUS_RING_WIDTH / 2;
                            rc.size.width -= FOCUS_RING_WIDTH / 2;
                        }
                    }

                    paintFocusRect(4.0, rc, context);
                }
                bOK=true;
            }
            break;
        case ControlType::Editbox:
        case ControlType::MultilineEditbox:
            {
                rc.size.width += 2 * EDITBOX_INSET_MARGIN;
                if (nType == ControlType::Editbox)
                  rc.size.height = EDITBOX_HEIGHT;
                else
                  rc.size.height += 2 * (EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN);
                rc.origin.x -= EDITBOX_INSET_MARGIN;
                rc.origin.y -= EDITBOX_INSET_MARGIN;

                NSTextFieldCell* pBtn = pInst->mpTextFieldCell;

                [pBtn setEnabled: getEnabled(nState, mpFrame)];
                [pBtn setBezeled: YES];
                [pBtn setEditable: YES];
                [pBtn setFocusRingType: NSFocusRingTypeExterior];

                drawEditableBackground(context, rc);
                const bool bFocused(nState & ControlState::FOCUSED);
                paintCell(pBtn, rc, bFocused, context, mpFrame->getNSView());

                bOK = true;
            }
            break;
        case ControlType::Combobox:
            if (nPart == ControlPart::HasBackgroundTexture || nPart == ControlPart::Entire)
            {
                rc.origin.y += (rc.size.height - COMBOBOX_HEIGHT + 1) / 2;
                rc.size.height = COMBOBOX_HEIGHT;

                NSComboBoxCell* pBtn = pInst->mpComboBoxCell;

                [pBtn setEnabled: getEnabled(nState, mpFrame)];
                [pBtn setEditable: YES];
                [pBtn setState: ImplGetButtonValue(aValue.getTristateVal())];
                [pBtn setFocusRingType: NSFocusRingTypeExterior];

                {
                    rc.origin.x += 2;
                    rc.size.width -= 1;
                }

                drawEditableBackground(context, rc);
                const bool bFocused(nState & ControlState::FOCUSED);
                paintCell(pBtn, rc, bFocused, context, mpFrame->getNSView());

                bOK = true;
            }
            break;
        case ControlType::Listbox:

            switch (nPart)
            {
                case ControlPart::Entire:
                case ControlPart::ButtonDown:
                {
                    rc.origin.y += (rc.size.height - LISTBOX_HEIGHT + 1) / 2;
                    rc.size.height = LISTBOX_HEIGHT;

                    NSPopUpButtonCell* pBtn = pInst->mpPopUpButtonCell;

                    [pBtn setTitle: @""];
                    [pBtn setEnabled: getEnabled(nState, mpFrame)];
                    [pBtn setFocusRingType: NSFocusRingTypeExterior];
                    [pBtn setHighlighted: (nState & ControlState::PRESSED) ? YES : NO];
                    if (nState & ControlState::DEFAULT)
                        [pBtn setKeyEquivalent: @"\r"];
                    else
                        [pBtn setKeyEquivalent: @""];

                    {
                        rc.size.width += 1;
                    }

                    const bool bFocused(nState & ControlState::FOCUSED);
                    paintCell(pBtn, rc, bFocused, context, nullptr);

                    bOK = true;
                    break;
                }
                case ControlPart::ListboxWindow:
                {
                    NSRect rect = { NSZeroPoint, NSMakeSize(rc.size.width, rc.size.height) };
                    NSScrollView* pBox = [[NSScrollView alloc] initWithFrame: rect];
                    [pBox setBorderType: NSLineBorder];

                    CGContextSaveGState(context);
                    CGContextTranslateCTM(context, rc.origin.x, rc.origin.y);

                    NSGraphicsContext* savedContext = [NSGraphicsContext currentContext];
                    NSGraphicsContext* graphicsContext = [NSGraphicsContext graphicsContextWithCGContext:context flipped:NO];
                    [NSGraphicsContext setCurrentContext: graphicsContext];

                    [pBox drawRect: rect];

                    [NSGraphicsContext setCurrentContext: savedContext];

                    CGContextRestoreGState(context);

                    [pBox release];

                    bOK = true;
                    break;
                }
                default:
                    break;
            }
            break;
        case ControlType::Spinbox:
            if (nPart == ControlPart::Entire)
            {
                // text field

                rc.size.width -= SPIN_BUTTON_WIDTH + 4 * FOCUS_RING_WIDTH;
                rc.size.height = EDITBOX_HEIGHT;
                rc.origin.x += FOCUS_RING_WIDTH;
                rc.origin.y += FOCUS_RING_WIDTH;

                NSTextFieldCell* pEdit = pInst->mpTextFieldCell;

                [pEdit setEnabled: YES];
                [pEdit setBezeled: YES];
                [pEdit setEditable: YES];
                [pEdit setFocusRingType: NSFocusRingTypeExterior];

                drawEditableBackground(context, rc);
                const bool bFocused(nState & ControlState::FOCUSED);
                paintCell(pEdit, rc, bFocused, context, mpFrame->getNSView());

                // buttons

                const SpinbuttonValue *pSpinButtonVal = (aValue.getType() == ControlType::SpinButtons)
                                                      ? static_cast <const SpinbuttonValue *>(&aValue) : nullptr;
                if (pSpinButtonVal)
                {
                    ControlState nUpperState = pSpinButtonVal->mnUpperState;
                    ControlState nLowerState = pSpinButtonVal->mnLowerState;

                    rc.origin.x += rc.size.width + FOCUS_RING_WIDTH + 1;
                    rc.origin.y -= 1;
                    rc.size.width = SPIN_BUTTON_WIDTH;
                    rc.size.height = SPIN_LOWER_BUTTON_HEIGHT + SPIN_LOWER_BUTTON_HEIGHT;

                    NSStepperCell* pBtn = pInst->mpStepperCell;

                    [pBtn setTitle: @""];
                    [pBtn setState: ImplGetButtonValue(aValue.getTristateVal())];
                    [pBtn setEnabled: (nUpperState & ControlState::ENABLED || nLowerState & ControlState::ENABLED) ?
                                    YES : NO];
                    [pBtn setFocusRingType: NSFocusRingTypeExterior];
                    [pBtn setHighlighted: (nState & ControlState::PRESSED) ? YES : NO];

                    const bool bSpinFocused(nUpperState & ControlState::FOCUSED || nLowerState & ControlState::FOCUSED);
                    paintCell(pBtn, rc, bSpinFocused, context, nullptr);
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

                        if (rc.origin.y + rc.size.height >= mpFrame->GetHeight() - 3)
                        {
                            CGMutablePathRef rPath = CGPathCreateMutable();
                            CGPathAddRect(rPath, nullptr,
                                          CGRectMake(0, 0, mpFrame->GetWidth() - 1, mpFrame->GetHeight() - 1));
                            CGContextBeginPath(context);
                            CGContextAddPath(context, rPath);
                            CGContextClip(context);
                            CGPathRelease(rPath);
                        }
                        HIThemeFrameDrawInfo aTextDrawInfo;
                        aTextDrawInfo.version = 0;
                        aTextDrawInfo.kind = kHIThemeFrameListBox;
                        aTextDrawInfo.state = kThemeStateActive;
                        aTextDrawInfo.isFocused = false;
                        HIThemeDrawFrame(&rc, &aTextDrawInfo, context, kHIThemeOrientationNormal);
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
        case ControlType::LevelBar:
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
                const tools::Long nBorderThickness = FOCUS_RING_WIDTH + EDITBOX_BORDER_WIDTH + EDITBOX_INSET_MARGIN;
                // tdf#144241 don't return a negative width, expand the region to the min osx width
                w = std::max(nBorderThickness * 2, aCtrlBoundRect.GetWidth());
                h = EDITBOX_HEIGHT + 2 * FOCUS_RING_WIDTH;
                rNativeBoundingRegion = tools::Rectangle(Point(x, y), Size(w, h));
                w -= 2 * nBorderThickness;
                h -= 2 * nBorderThickness;
                x += nBorderThickness;
                y += nBorderThickness;
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
