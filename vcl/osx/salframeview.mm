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

#include <sal/config.h>

#include <memory>

#include <basegfx/numeric/ftools.hxx>
#include <sal/macros.h>
#include <tools/helpers.hxx>
#include <tools/long.hxx>
#include <vcl/event.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/commandevent.hxx>

#include <osx/a11yfactory.h>
#include <osx/salframe.h>
#include <osx/salframeview.h>
#include <osx/salinst.h>
#include <quartz/salgdi.h>
#include <quartz/utils.h>

#if HAVE_FEATURE_SKIA
#include <vcl/skia/SkiaHelper.hxx>
#endif

#define WHEEL_EVENT_FACTOR 1.5

static sal_uInt16 ImplGetModifierMask( unsigned int nMask )
{
    sal_uInt16 nRet = 0;
    if( (nMask & NSEventModifierFlagShift) != 0 )
        nRet |= KEY_SHIFT;
    if( (nMask & NSEventModifierFlagControl) != 0 )
        nRet |= KEY_MOD3;
    if( (nMask & NSEventModifierFlagOption) != 0 )
        nRet |= KEY_MOD2;
    if( (nMask & NSEventModifierFlagCommand) != 0 )
        nRet |= KEY_MOD1;
    return nRet;
}

static sal_uInt16 ImplMapCharCode( sal_Unicode aCode )
{
    static sal_uInt16 aKeyCodeMap[ 128 ] =
    {
                    0,                0,                0,                0,                0,                0,                0,                0,
        KEY_BACKSPACE,          KEY_TAB,       KEY_RETURN,                0,                0,       KEY_RETURN,                0,                0,
                    0,                0,                0,                0,                0,                0,                0,                0,
                    0,          KEY_TAB,                0,       KEY_ESCAPE,                0,                0,                0,                0,
            KEY_SPACE,                0,                0,                0,                0,                0,                0,                0,
                    0,                0,     KEY_MULTIPLY,          KEY_ADD,        KEY_COMMA,     KEY_SUBTRACT,        KEY_POINT,       KEY_DIVIDE,
                KEY_0,            KEY_1,            KEY_2,            KEY_3,            KEY_4,            KEY_5,            KEY_6,            KEY_7,
                KEY_8,            KEY_9,                0,                0,         KEY_LESS,        KEY_EQUAL,      KEY_GREATER,                0,
                    0,            KEY_A,            KEY_B,            KEY_C,            KEY_D,            KEY_E,            KEY_F,            KEY_G,
                KEY_H,            KEY_I,            KEY_J,            KEY_K,            KEY_L,            KEY_M,            KEY_N,            KEY_O,
                KEY_P,            KEY_Q,            KEY_R,            KEY_S,            KEY_T,            KEY_U,            KEY_V,            KEY_W,
                KEY_X,            KEY_Y,            KEY_Z,                0,                0,                0,                0,                0,
        KEY_QUOTELEFT,            KEY_A,            KEY_B,            KEY_C,            KEY_D,            KEY_E,            KEY_F,            KEY_G,
                KEY_H,            KEY_I,            KEY_J,            KEY_K,            KEY_L,            KEY_M,            KEY_N,            KEY_O,
                KEY_P,            KEY_Q,            KEY_R,            KEY_S,            KEY_T,            KEY_U,            KEY_V,            KEY_W,
                KEY_X,            KEY_Y,            KEY_Z,                0,                0,                0,        KEY_TILDE,    KEY_BACKSPACE
    };

    // Note: the mapping 0x7f should by rights be KEY_DELETE
    // however if you press "backspace" 0x7f is reported
    // whereas for "delete" 0xf728 gets reported

    // Note: the mapping of 0x19 to KEY_TAB is because for unknown reasons
    // tab alone is reported as 0x09 (as expected) but shift-tab is
    // reported as 0x19 (end of medium)

    static sal_uInt16 aFunctionKeyCodeMap[ 128 ] =
    {
            KEY_UP,         KEY_DOWN,         KEY_LEFT,        KEY_RIGHT,           KEY_F1,           KEY_F2,           KEY_F3,           KEY_F4,
            KEY_F5,           KEY_F6,           KEY_F7,           KEY_F8,           KEY_F9,          KEY_F10,          KEY_F11,          KEY_F12,
           KEY_F13,          KEY_F14,          KEY_F15,          KEY_F16,          KEY_F17,          KEY_F18,          KEY_F19,          KEY_F20,
           KEY_F21,          KEY_F22,          KEY_F23,          KEY_F24,          KEY_F25,          KEY_F26,                0,                0,
                 0,                0,                0,                0,                0,                0,                0,       KEY_INSERT,
        KEY_DELETE,         KEY_HOME,                0,          KEY_END,        KEY_PAGEUP,    KEY_PAGEDOWN,                0,                0,
                 0,                0,                0,                0,                 0,        KEY_MENU,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,         KEY_UNDO,        KEY_REPEAT,        KEY_FIND,         KEY_HELP,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0,
                 0,                0,                0,                0,                 0,               0,                0,                0
    };

    sal_uInt16 nKeyCode = 0;
    if( aCode < SAL_N_ELEMENTS( aKeyCodeMap)  )
        nKeyCode = aKeyCodeMap[ aCode ];
    else if( aCode >= 0xf700 && aCode < 0xf780 )
        nKeyCode = aFunctionKeyCodeMap[ aCode - 0xf700 ];
    return nKeyCode;
}

static sal_uInt16 ImplMapKeyCode(sal_uInt16 nKeyCode)
{
    /*
      http://stackoverflow.com/questions/2080312/where-can-i-find-a-list-of-key-codes-for-use-with-cocoas-nsevent-class/2080324#2080324
      /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Events.h
     */

    static sal_uInt16 aKeyCodeMap[ 0x80 ] =
    {
            KEY_A,            KEY_S,            KEY_D,            KEY_F,            KEY_H,            KEY_G,            KEY_Z,            KEY_X,
            KEY_C,            KEY_V,                0,            KEY_B,            KEY_Q,            KEY_W,            KEY_E,            KEY_R,
            KEY_Y,            KEY_T,            KEY_1,            KEY_2,            KEY_3,            KEY_4,            KEY_6,            KEY_5,
        KEY_EQUAL,            KEY_9,            KEY_7,     KEY_SUBTRACT,            KEY_8,            KEY_0, KEY_BRACKETRIGHT, KEY_RIGHTCURLYBRACKET,
            KEY_U,  KEY_BRACKETLEFT,            KEY_I,            KEY_P,       KEY_RETURN,            KEY_L,            KEY_J,   KEY_QUOTERIGHT,
            KEY_K,    KEY_SEMICOLON,                0,        KEY_COMMA,       KEY_DIVIDE,            KEY_N,            KEY_M,        KEY_POINT,
          KEY_TAB,        KEY_SPACE,    KEY_QUOTELEFT,       KEY_DELETE,                0,       KEY_ESCAPE,                0,                0,
                0,     KEY_CAPSLOCK,                0,                0,                0,                0,                0,                0,
          KEY_F17,      KEY_DECIMAL,                0,     KEY_MULTIPLY,                0,          KEY_ADD,                0,                0,
                0,                0,                0,       KEY_DIVIDE,       KEY_RETURN,                0,     KEY_SUBTRACT,          KEY_F18,
          KEY_F19,        KEY_EQUAL,                0,                0,                0,                0,                0,                0,
                0,                0,          KEY_F20,                0,                0,                0,                0,                0,
           KEY_F5,           KEY_F6,           KEY_F7,           KEY_F3,           KEY_F8,           KEY_F9,                0,          KEY_F11,
                0,          KEY_F13,          KEY_F16,          KEY_F14,                0,          KEY_F10,                0,          KEY_F12,
                0,          KEY_F15,         KEY_HELP,         KEY_HOME,       KEY_PAGEUP,       KEY_DELETE,           KEY_F4,          KEY_END,
           KEY_F2,     KEY_PAGEDOWN,           KEY_F1,         KEY_LEFT,        KEY_RIGHT,         KEY_DOWN,           KEY_UP,                0
    };

    if (nKeyCode < SAL_N_ELEMENTS(aKeyCodeMap))
        return aKeyCodeMap[nKeyCode];
    return 0;
}

// store the frame the mouse last entered
static AquaSalFrame* s_pMouseFrame = nullptr;
// store the last pressed button for enter/exit events
// which lack that information
static sal_uInt16 s_nLastButton = 0;

static AquaSalFrame* getMouseContainerFrame()
{
    AquaSalFrame* pDispatchFrame = nullptr;
    NSArray* aWindows = [NSWindow windowNumbersWithOptions:0];
    for(NSUInteger i = 0; i < [aWindows count] && ! pDispatchFrame; i++ )
    {
        NSWindow* pWin = [NSApp windowWithWindowNumber:[[aWindows objectAtIndex:i] integerValue]];
        if( pWin && [pWin isMemberOfClass: [SalFrameWindow class]] && [static_cast<SalFrameWindow*>(pWin) containsMouse] )
            pDispatchFrame = [static_cast<SalFrameWindow*>(pWin) getSalFrame];
    }
    return pDispatchFrame;
}

static NSArray *getMergedAccessibilityChildren(NSArray *pDefaultChildren, NSArray *pUnignoredChildrenToAdd)
{
    NSArray *pRet = pDefaultChildren;

    if (pUnignoredChildrenToAdd && [pUnignoredChildrenToAdd count])
    {
        NSMutableArray *pNewChildren = [NSMutableArray arrayWithCapacity:(pRet ? [pRet count] : 0) + 1];
        if (pNewChildren)
        {
            if (pRet)
                [pNewChildren addObjectsFromArray:pRet];

            for (AquaA11yWrapper *pWrapper : pUnignoredChildrenToAdd)
            {
                if (pWrapper && ![pNewChildren containsObject:pWrapper])
                    [pNewChildren addObject:pWrapper];
            }

            pRet = pNewChildren;
        }
        else
        {
            pRet = pUnignoredChildrenToAdd;
        }
    }

    return pRet;
}

// Update ImplGetSVData()->mpWinData->mbIsLiveResize
static void updateWinDataInLiveResize(bool bInLiveResize)
{
    ImplSVData* pSVData = ImplGetSVData();
    assert( pSVData );
    if ( pSVData )
    {
        if ( pSVData->mpWinData->mbIsLiveResize != bInLiveResize )
        {
            pSVData->mpWinData->mbIsLiveResize = bInLiveResize;
            Scheduler::Wakeup();
        }
    }
}

@interface NSResponder (SalFrameWindow)
-(BOOL)accessibilityIsIgnored;
@end

@implementation SalFrameWindow
-(id)initWithSalFrame: (AquaSalFrame*)pFrame
{
    mDraggingDestinationHandler = nil;
    mbInWindowDidResize = NO;
    mpLiveResizeTimer = nil;
    mpFrame = pFrame;
    NSRect aRect = { { static_cast<CGFloat>(pFrame->maGeometry.x()), static_cast<CGFloat>(pFrame->maGeometry.y()) },
                     { static_cast<CGFloat>(pFrame->maGeometry.width()), static_cast<CGFloat>(pFrame->maGeometry.height()) } };
    pFrame->VCLToCocoa( aRect );
    NSWindow* pNSWindow = [super initWithContentRect: aRect
                                 styleMask: mpFrame->getStyleMask()
                                 backing: NSBackingStoreBuffered
                                 defer: Application::IsHeadlessModeEnabled()];

    // Disallow full-screen mode on macOS >= 10.11 where it is enabled by default. We don't want it
    // for now as it will just be confused with LibreOffice's home-grown full-screen concept, with
    // which it has nothing to do, and one can get into all kinds of weird states by using them
    // intermixedly.

    // Ideally we should use the system full-screen mode and adapt the code for the home-grown thing
    // to be in sync with that instead. (And we would then not need the button to get out of
    // full-screen mode, as the normal way to get out of it is to either click on the green bubble
    // again, or invoke the keyboard command again.)

    // (Confusingly, at the moment the home-grown full-screen mode is bound to Cmd+Shift+F, which is
    // the keyboard command normally used in apps to get in and out of the system full-screen mode.)

    // Disabling system full-screen mode makes the green button on the title bar (on macOS >= 10.11)
    // show a plus sign instead, and clicking it becomes identical to double-clicking the title bar,
    // i.e. it maximizes / unmaximises the window. Sure, that state can also be confused with LO's
    // home-grown full-screen mode. Oh well.

    [pNSWindow setCollectionBehavior: NSWindowCollectionBehaviorFullScreenNone];

    // Disable window restoration until we support it directly
    [pNSWindow setRestorable: NO];

    // tdf#137468: Restrict to 24-bit RGB as that is all that we can
    // handle anyway. HDR is far off in the future for LibreOffice.
    [pNSWindow setDynamicDepthLimit: NO];
    [pNSWindow setDepthLimit: NSWindowDepthTwentyfourBitRGB];

    return static_cast<SalFrameWindow *>(pNSWindow);
}

-(void)clearLiveResizeTimer
{
    if ( mpLiveResizeTimer )
    {
        [mpLiveResizeTimer invalidate];
        [mpLiveResizeTimer release];
        mpLiveResizeTimer = nil;
    }
}

-(void)dealloc
{
    [self clearLiveResizeTimer];
    [super dealloc];
}

-(AquaSalFrame*)getSalFrame
{
    return mpFrame;
}

-(void)displayIfNeeded
{
    if( GetSalData() && GetSalData()->mpInstance )
    {
        SolarMutexGuard aGuard;
        [super displayIfNeeded];
    }
}

-(BOOL)containsMouse
{
    // is this event actually inside that NSWindow ?
    NSPoint aPt = [NSEvent mouseLocation];
    NSRect aFrameRect = [self frame];
    bool bInRect = NSPointInRect( aPt, aFrameRect );
    return bInRect;
}

-(BOOL)canBecomeKeyWindow
{
    if( (mpFrame->mnStyle &
            ( SalFrameStyleFlags::FLOAT                 |
              SalFrameStyleFlags::TOOLTIP               |
              SalFrameStyleFlags::INTRO
            )) == SalFrameStyleFlags::NONE )
        return YES;
    if( mpFrame->mnStyle & SalFrameStyleFlags::OWNERDRAWDECORATION )
        return YES;
    if( mpFrame->mbFullScreen )
        return YES;
    return [super canBecomeKeyWindow];
}

-(void)windowDidBecomeKey: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        static const SalFrameStyleFlags nGuessDocument = SalFrameStyleFlags::MOVEABLE|
                                            SalFrameStyleFlags::SIZEABLE|
                                            SalFrameStyleFlags::CLOSEABLE;

        // Reset dark mode colors in HITheme controls after printing
        // In dark mode, after an NSPrintOperation has completed, macOS draws
        // HITheme controls with light mode colors so reset all dark mode
        // colors when an NSWindow gains focus.
        mpFrame->UpdateDarkMode();

        if( mpFrame->mpMenu )
            mpFrame->mpMenu->setMainMenu();
        else if( ! mpFrame->mpParent &&
                 ( (mpFrame->mnStyle & nGuessDocument) == nGuessDocument || // set default menu for e.g. help
                    mpFrame->mbFullScreen ) )                               // set default menu for e.g. presentation
        {
            AquaSalMenu::setDefaultMenu();
        }
        mpFrame->CallCallback( SalEvent::GetFocus, nullptr );
        mpFrame->SendPaintEvent(); // repaint controls as active
    }

    // Prevent the same native input method popup that was cancelled in a
    // previous call to [self windowDidResignKey:] from reappearing
    [self endExtTextInput];
}

-(void)windowDidResignKey: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    // Commit any uncommitted text and cancel the native input method session
    // whenever a window loses focus like in Safari, Firefox, and Excel
    [self endExtTextInput];

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->CallCallback(SalEvent::LoseFocus, nullptr);
        mpFrame->SendPaintEvent(); // repaint controls as inactive
    }
}

-(void)windowDidChangeScreen: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        mpFrame->screenParametersChanged();
}

-(void)windowDidMove: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SalEvent::Move, nullptr );
    }
}

-(void)windowDidResize: (NSNotification*)pNotification
{
    SolarMutexGuard aGuard;

    if ( mbInWindowDidResize )
        return;

    mbInWindowDidResize = YES;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SalEvent::Resize, nullptr );

        updateWinDataInLiveResize( [self inLiveResize] );
        if ( ImplGetSVData()->mpWinData->mbIsLiveResize )
        {
#if HAVE_FEATURE_SKIA
            // Related: tdf#152703 Eliminate empty window with Skia/Metal while resizing
            // The window will clear its background so when Skia/Metal is
            // enabled, explicitly flush the Skia graphics to the window
            // during live resizing or else nothing will be drawn until after
            // live resizing has ended.
            // Also, flushing during [self windowDidResize:] eliminates flicker
            // by forcing this window's SkSurface to recreate its underlying
            // CAMetalLayer with the new size. Flushing in
            // [self displayIfNeeded] does not eliminate flicker so apparently
            // [self windowDidResize:] is called earlier.
            if ( SkiaHelper::isVCLSkiaEnabled() )
            {
                AquaSalGraphics* pGraphics = mpFrame->mpGraphics;
                if ( pGraphics )
                    pGraphics->Flush();
            }
#endif

            // tdf#152703 Force relayout during live resizing of window
            // During a live resize, macOS floods the application with
            // windowDidResize: notifications so sending a paint event does
            // not trigger redrawing with the new size.
            // Instead, force relayout by dispatching all pending internal
            // events and firing any pending timers.
            // Also, Application::Reschedule() can potentially display a
            // modal dialog which will cause a hang so temporarily disable
            // live resize by clamping the window's minimum and maximum sizes
            // to the current frame size which in Application::Reschedule().
            NSRect aFrame = [self frame];
            NSSize aMinSize = [self minSize];
            NSSize aMaxSize = [self maxSize];
            [self setMinSize:aFrame.size];
            [self setMaxSize:aFrame.size];
            Application::Reschedule( true );
            [self setMinSize:aMinSize];
            [self setMaxSize:aMaxSize];

            if ( ImplGetSVData()->mpWinData->mbIsLiveResize )
            {
                // tdf#152703 Force repaint after live resizing ends
                // Repost this notification so that this selector will be called
                // at least once after live resizing ends
                if ( !mpLiveResizeTimer )
                {
                    mpLiveResizeTimer = [NSTimer scheduledTimerWithTimeInterval:0.1f target:self selector:@selector(windowDidResizeWithTimer:) userInfo:pNotification repeats:YES];
                    if ( mpLiveResizeTimer )
                    {
                        [mpLiveResizeTimer retain];

                        // The timer won't fire without a call to
                        // Application::Reschedule() unless we copy the fix for
                        // #i84055# from vcl/osx/saltimer.cxx and add the timer
                        // to the NSEventTrackingRunLoopMode run loop mode
                        [[NSRunLoop currentRunLoop] addTimer:mpLiveResizeTimer forMode:NSEventTrackingRunLoopMode];
                    }
                }
            }
        }
        else
        {
            [self clearLiveResizeTimer];
        }

        // tdf#158461 eliminate flicker during live resizing
        // When using Skia/Metal, the window content will flicker while
        // live resizing a window if we don't send a paint event.
        mpFrame->SendPaintEvent();
    }

    mbInWindowDidResize = NO;
}

-(void)windowDidMiniaturize: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mbShown = false;
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SalEvent::Resize, nullptr );
    }
}

-(void)windowDidDeminiaturize: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mbShown = true;
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SalEvent::Resize, nullptr );
    }
}

-(BOOL)windowShouldClose: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    bool bRet = true;
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        // #i84461# end possible input
        [self endExtTextInput];
        if( AquaSalFrame::isAlive( mpFrame ) )
        {
            mpFrame->CallCallback( SalEvent::Close, nullptr );
            bRet = false; // application will close the window or not, AppKit shouldn't
            AquaSalTimer *pTimer = static_cast<AquaSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
            assert( pTimer );
            pTimer->handleWindowShouldClose();
        }
    }

    return bRet;
}

-(void)windowDidEnterFullScreen: (NSNotification*)pNotification
{
    SolarMutexGuard aGuard;

    if( !mpFrame || !AquaSalFrame::isAlive( mpFrame))
        return;
    mpFrame->mbFullScreen = true;
    (void)pNotification;
}

-(void)windowDidExitFullScreen: (NSNotification*)pNotification
{
    SolarMutexGuard aGuard;

    if( !mpFrame || !AquaSalFrame::isAlive( mpFrame))
        return;
    mpFrame->mbFullScreen = false;
    (void)pNotification;
}

-(void)windowDidChangeBackingProperties:(NSNotification *)pNotification
{
    (void)pNotification;
#if HAVE_FEATURE_SKIA
    SolarMutexGuard aGuard;

    sal::aqua::resetWindowScaling();

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        // tdf#147342 Notify Skia that the window's backing properties changed
        if ( SkiaHelper::isVCLSkiaEnabled() )
        {
            AquaSalGraphics* pGraphics = mpFrame->mpGraphics;
            if ( pGraphics )
                pGraphics->WindowBackingPropertiesChanged();
        }
    }
#endif
}

-(void)windowWillStartLiveResize:(NSNotification *)pNotification
{
    SolarMutexGuard aGuard;

    updateWinDataInLiveResize(true);
}

-(void)windowDidEndLiveResize:(NSNotification *)pNotification
{
    SolarMutexGuard aGuard;

    updateWinDataInLiveResize(false);
}

-(void)dockMenuItemTriggered: (id)sender
{
    (void)sender;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        mpFrame->ToTop( SalFrameToTop::RestoreWhenMin | SalFrameToTop::GrabFocus );
}

-(css::uno::Reference < css::accessibility::XAccessibleContext >)accessibleContext
{
    return mpFrame -> GetWindow() -> GetAccessible() -> getAccessibleContext();
}

-(BOOL)isIgnoredWindow
{
    SolarMutexGuard aGuard;

    // Treat tooltip windows as ignored
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        return (mpFrame->mnStyle & SalFrameStyleFlags::TOOLTIP) != SalFrameStyleFlags::NONE;
    return YES;
}

-(id)accessibilityApplicationFocusedUIElement
{
    return [self accessibilityFocusedUIElement];
}

-(id)accessibilityFocusedUIElement
{
    // Treat tooltip windows as ignored
    if ([self isIgnoredWindow])
        return nil;

    return [super accessibilityFocusedUIElement];
}

-(BOOL)accessibilityIsIgnored
{
    // Treat tooltip windows as ignored
    if ([self isIgnoredWindow])
        return YES;

    return [super accessibilityIsIgnored];
}

-(BOOL)isAccessibilityElement
{
    return ![self accessibilityIsIgnored];
}

-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler draggingEntered: sender];
}

-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler draggingUpdated: sender];
}

-(void)draggingExited:(id <NSDraggingInfo>)sender
{
  [mDraggingDestinationHandler draggingExited: sender];
}

-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler prepareForDragOperation: sender];
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler performDragOperation: sender];
}

-(void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
  [mDraggingDestinationHandler concludeDragOperation: sender];
}

-(void)registerDraggingDestinationHandler:(id)theHandler
{
  mDraggingDestinationHandler = theHandler;
}

-(void)unregisterDraggingDestinationHandler:(id)theHandler
{
    (void)theHandler;
    mDraggingDestinationHandler = nil;
}

-(void)endExtTextInput
{
    [self endExtTextInput:EndExtTextInputFlags::Complete];
}

-(void)endExtTextInput:(EndExtTextInputFlags)nFlags
{
    SalFrameView *pView = static_cast<SalFrameView*>([self firstResponder]);
    if (pView && [pView isKindOfClass:[SalFrameView class]])
        [pView endExtTextInput:nFlags];
}

-(void)windowDidResizeWithTimer:(NSTimer *)pTimer
{
    if ( pTimer )
        [self windowDidResize:[pTimer userInfo]];
}

@end

@implementation SalFrameView
+(void)unsetMouseFrame: (AquaSalFrame*)pFrame
{
    if( pFrame == s_pMouseFrame )
        s_pMouseFrame = nullptr;
}

-(id)initWithSalFrame: (AquaSalFrame*)pFrame
{
    if ((self = [super initWithFrame: [NSWindow contentRectForFrameRect: [pFrame->getNSWindow() frame] styleMask: pFrame->mnStyleMask]]) != nil)
    {
        mDraggingDestinationHandler = nil;
        mpFrame = pFrame;
        mpChildWrapper = nil;
        mbNeedChildWrapper = NO;
        mpLastEvent = nil;
        mMarkedRange = NSMakeRange(NSNotFound, 0);
        mSelectedRange = NSMakeRange(NSNotFound, 0);
        mpMouseEventListener = nil;
        mpLastSuperEvent = nil;
        mfLastMagnifyTime = 0.0;

        mbInEndExtTextInput = NO;
        mbInCommitMarkedText = NO;
        mpLastMarkedText = nil;
        mbTextInputWantsNonRepeatKeyDown = NO;
    }

    return self;
}

-(void)dealloc
{
    [self clearLastEvent];
    [self clearLastMarkedText];
    [self revokeWrapper];

    [super dealloc];
}

-(AquaSalFrame*)getSalFrame
{
    return mpFrame;
}

-(void)resetCursorRects
{
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        // FIXME: does this leak the returned NSCursor of getCurrentCursor ?
        const NSRect aRect = { NSZeroPoint, NSMakeSize(mpFrame->maGeometry.width(), mpFrame->maGeometry.height()) };
        [self addCursorRect: aRect cursor: mpFrame->getCurrentCursor()];
    }
}

-(BOOL)acceptsFirstResponder
{
    return YES;
}

-(BOOL)acceptsFirstMouse: (NSEvent*)pEvent
{
    (void)pEvent;
    return YES;
}

-(BOOL)isOpaque
{
    if( !mpFrame)
        return YES;
    if( !AquaSalFrame::isAlive( mpFrame))
        return YES;
    if( !mpFrame->getClipPath())
        return YES;
    return NO;
}

-(void)drawRect: (NSRect)aRect
{
    ImplSVData* pSVData = ImplGetSVData();
    assert( pSVData );
    if ( !pSVData )
        return;

    SolarMutexGuard aGuard;
    if (!mpFrame || !AquaSalFrame::isAlive(mpFrame))
        return;

    updateWinDataInLiveResize([self inLiveResize]);

    AquaSalGraphics* pGraphics = mpFrame->mpGraphics;
    if (pGraphics)
    {
        pGraphics->UpdateWindow(aRect);
        if (mpFrame->getClipPath())
            [mpFrame->getNSWindow() invalidateShadow];
    }
}

-(void)sendMouseEventToFrame: (NSEvent*)pEvent button:(sal_uInt16)nButton eventtype:(SalEvent)nEvent
{
    SolarMutexGuard aGuard;

    AquaSalFrame* pDispatchFrame = AquaSalFrame::GetCaptureFrame();
    bool bIsCaptured = false;
    if( pDispatchFrame )
    {
        bIsCaptured = true;
        if( nEvent == SalEvent::MouseLeave ) // no leave events if mouse is captured
            nEvent = SalEvent::MouseMove;
    }
    else if( s_pMouseFrame )
        pDispatchFrame = s_pMouseFrame;
    else
        pDispatchFrame = mpFrame;

    /* #i81645# Cocoa reports mouse events while a button is pressed
       to the window in which it was first pressed. This is reasonable and fine and
       gets one around most cases where on other platforms one uses CaptureMouse or XGrabPointer,
       however vcl expects mouse events to occur in the window the mouse is over, unless the
       mouse is explicitly captured. So we need to find the window the mouse is actually
       over for conformance with other platforms.
    */
    if( ! bIsCaptured && nButton && pDispatchFrame && AquaSalFrame::isAlive( pDispatchFrame ) )
    {
        // is this event actually inside that NSWindow ?
        NSPoint aPt = [NSEvent mouseLocation];
        NSRect aFrameRect = [pDispatchFrame->getNSWindow() frame];

        if ( ! NSPointInRect( aPt, aFrameRect ) )
        {
            // no, it is not
            // now we need to find the one it may be in
            /* #i93756# we ant to get enumerate the application windows in z-order
               to check if any contains the mouse. This could be elegantly done with this
               code:

               // use NSApp to check windows in ZOrder whether they contain the mouse pointer
               NSWindow* pWindow = [NSApp makeWindowsPerform: @selector(containsMouse) inOrder: YES];
               if( pWindow && [pWindow isMemberOfClass: [SalFrameWindow class]] )
                   pDispatchFrame = [(SalFrameWindow*)pWindow getSalFrame];

               However if a non SalFrameWindow is on screen (like e.g. the file dialog)
               it can be hit with the containsMouse selector, which it doesn't support.
               Sadly NSApplication:makeWindowsPerform does not check (for performance reasons
               I assume) whether a window supports a selector before sending it.
            */
            AquaSalFrame* pMouseFrame = getMouseContainerFrame();
            if( pMouseFrame )
                pDispatchFrame = pMouseFrame;
        }
    }

    if( pDispatchFrame && AquaSalFrame::isAlive( pDispatchFrame ) )
    {
        pDispatchFrame->mnLastEventTime = static_cast<sal_uInt64>( [pEvent timestamp] * 1000.0 );
        pDispatchFrame->mnLastModifierFlags = [pEvent modifierFlags];

        NSPoint aPt = [NSEvent mouseLocation];
        pDispatchFrame->CocoaToVCL( aPt );

        sal_uInt16 nModMask = ImplGetModifierMask( [pEvent modifierFlags] );
        // #i82284# emulate ctrl left
        if( nModMask == KEY_MOD3 && nButton == MOUSE_LEFT )
        {
            nModMask    = 0;
            nButton     = MOUSE_RIGHT;
        }

        SalMouseEvent aEvent;
        aEvent.mnTime   = pDispatchFrame->mnLastEventTime;
        aEvent.mnX = static_cast<tools::Long>(aPt.x) - pDispatchFrame->maGeometry.x();
        aEvent.mnY = static_cast<tools::Long>(aPt.y) - pDispatchFrame->maGeometry.y();
        aEvent.mnButton = nButton;
        aEvent.mnCode   =  aEvent.mnButton | nModMask;

        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = pDispatchFrame->maGeometry.width() - 1 - aEvent.mnX;

        pDispatchFrame->CallCallback( nEvent, &aEvent );

        // tdf#155266 force flush after scrolling
        if (nButton == MOUSE_LEFT && nEvent == SalEvent::MouseMove)
            mpFrame->mbForceFlush = true;
    }
}

-(void)mouseDown: (NSEvent*)pEvent
{
    if ( mpMouseEventListener != nil &&
        [mpMouseEventListener respondsToSelector: @selector(mouseDown:)])
    {
        [mpMouseEventListener mouseDown: [pEvent copyWithZone: nullptr]];
    }

    s_nLastButton = MOUSE_LEFT;
    [self sendMouseEventToFrame:pEvent button:MOUSE_LEFT eventtype:SalEvent::MouseButtonDown];
}

-(void)mouseDragged: (NSEvent*)pEvent
{
    if ( mpMouseEventListener != nil &&
         [mpMouseEventListener respondsToSelector: @selector(mouseDragged:)])
    {
        [mpMouseEventListener mouseDragged: [pEvent copyWithZone: nullptr]];
    }
    s_nLastButton = MOUSE_LEFT;
    [self sendMouseEventToFrame:pEvent button:MOUSE_LEFT eventtype:SalEvent::MouseMove];
}

-(void)mouseUp: (NSEvent*)pEvent
{
    s_nLastButton = 0;
    [self sendMouseEventToFrame:pEvent button:MOUSE_LEFT eventtype:SalEvent::MouseButtonUp];
}

-(void)mouseMoved: (NSEvent*)pEvent
{
    s_nLastButton = 0;
    [self sendMouseEventToFrame:pEvent button:0 eventtype:SalEvent::MouseMove];
}

-(void)mouseEntered: (NSEvent*)pEvent
{
    s_pMouseFrame = mpFrame;

    // #i107215# the only mouse events we get when inactive are enter/exit
    // actually we would like to have all of them, but better none than some
    if( [NSApp isActive] )
        [self sendMouseEventToFrame:pEvent button:s_nLastButton eventtype:SalEvent::MouseMove];
}

-(void)mouseExited: (NSEvent*)pEvent
{
    if( s_pMouseFrame == mpFrame )
        s_pMouseFrame = nullptr;

    // #i107215# the only mouse events we get when inactive are enter/exit
    // actually we would like to have all of them, but better none than some
    if( [NSApp isActive] )
        [self sendMouseEventToFrame:pEvent button:s_nLastButton eventtype:SalEvent::MouseLeave];
}

-(void)rightMouseDown: (NSEvent*)pEvent
{
    s_nLastButton = MOUSE_RIGHT;
    [self sendMouseEventToFrame:pEvent button:MOUSE_RIGHT eventtype:SalEvent::MouseButtonDown];
}

-(void)rightMouseDragged: (NSEvent*)pEvent
{
    s_nLastButton = MOUSE_RIGHT;
    [self sendMouseEventToFrame:pEvent button:MOUSE_RIGHT eventtype:SalEvent::MouseMove];
}

-(void)rightMouseUp: (NSEvent*)pEvent
{
    s_nLastButton = 0;
    [self sendMouseEventToFrame:pEvent button:MOUSE_RIGHT eventtype:SalEvent::MouseButtonUp];
}

-(void)otherMouseDown: (NSEvent*)pEvent
{
    if( [pEvent buttonNumber] == 2 )
    {
        s_nLastButton = MOUSE_MIDDLE;
        [self sendMouseEventToFrame:pEvent button:MOUSE_MIDDLE eventtype:SalEvent::MouseButtonDown];
    }
    else
        s_nLastButton = 0;
}

-(void)otherMouseDragged: (NSEvent*)pEvent
{
    if( [pEvent buttonNumber] == 2 )
    {
        s_nLastButton = MOUSE_MIDDLE;
        [self sendMouseEventToFrame:pEvent button:MOUSE_MIDDLE eventtype:SalEvent::MouseMove];
    }
    else
        s_nLastButton = 0;
}

-(void)otherMouseUp: (NSEvent*)pEvent
{
    s_nLastButton = 0;
    if( [pEvent buttonNumber] == 2 )
        [self sendMouseEventToFrame:pEvent button:MOUSE_MIDDLE eventtype:SalEvent::MouseButtonUp];
}

- (void)magnifyWithEvent: (NSEvent*)pEvent
{
    SolarMutexGuard aGuard;

    // TODO: ??  -(float)magnification;
    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        const NSTimeInterval fMagnifyTime = [pEvent timestamp];
        mpFrame->mnLastEventTime = static_cast<sal_uInt64>( fMagnifyTime * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        // check if this is a new series of magnify events
        static const NSTimeInterval fMaxDiffTime = 0.3;
        const bool bNewSeries = (fMagnifyTime - mfLastMagnifyTime > fMaxDiffTime);

        if( bNewSeries )
            mfMagnifyDeltaSum = 0.0;
        mfMagnifyDeltaSum += [pEvent magnification];

        mfLastMagnifyTime = [pEvent timestamp];
// TODO: change to 0.1 when CommandWheelMode::ZOOM handlers allow finer zooming control
        static const float fMagnifyFactor = 0.25*500; // steps are 500 times smaller for -magnification
        static const float fMinMagnifyStep = 15.0 / fMagnifyFactor;
        if( fabs(mfMagnifyDeltaSum) <= fMinMagnifyStep )
            return;

        // adapt NSEvent-sensitivity to application expectations
        // TODO: rather make CommandWheelMode::ZOOM handlers smarter
        const float fDeltaZ = mfMagnifyDeltaSum * fMagnifyFactor;
        int nDeltaZ = basegfx::fround<int>( fDeltaZ );
        if( !nDeltaZ )
        {
            // handle new series immediately
            if( !bNewSeries )
                return;
            nDeltaZ = (fDeltaZ >= 0.0) ? +1 : -1;
        }
        // eventually give credit for delta sum
        mfMagnifyDeltaSum -= nDeltaZ / fMagnifyFactor;

        NSPoint aPt = [NSEvent mouseLocation];
        mpFrame->CocoaToVCL( aPt );

        SalWheelMouseEvent aEvent;
        aEvent.mnTime           = mpFrame->mnLastEventTime;
        aEvent.mnX = static_cast<tools::Long>(aPt.x) - mpFrame->maGeometry.x();
        aEvent.mnY = static_cast<tools::Long>(aPt.y) - mpFrame->maGeometry.y();
        aEvent.mnCode           = ImplGetModifierMask( mpFrame->mnLastModifierFlags );
        aEvent.mnCode           |= KEY_MOD1; // we want zooming, no scrolling
        aEvent.mbDeltaIsPixel   = true;

        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = mpFrame->maGeometry.width() - 1 - aEvent.mnX;

        aEvent.mnDelta = nDeltaZ;
        aEvent.mnNotchDelta = (nDeltaZ >= 0) ? +1 : -1;
        if( aEvent.mnDelta == 0 )
            aEvent.mnDelta = aEvent.mnNotchDelta;
        aEvent.mbHorz = false;
        sal_uInt32 nScrollLines = nDeltaZ;
        if (nScrollLines == 0)
            nScrollLines = 1;
        aEvent.mnScrollLines = nScrollLines;
        mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
    }
}

- (void)rotateWithEvent: (NSEvent*)pEvent
{
    //Rotation : -(float)rotation;
    // TODO: create new CommandType so rotation is available to the applications
    (void)pEvent;
}

- (void)swipeWithEvent: (NSEvent*)pEvent
{
    SolarMutexGuard aGuard;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mnLastEventTime = static_cast<sal_uInt64>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        // merge pending scroll wheel events
        CGFloat dX = 0.0;
        CGFloat dY = 0.0;
        for(;;)
        {
            dX += [pEvent deltaX];
            dY += [pEvent deltaY];
            NSEvent* pNextEvent = [NSApp nextEventMatchingMask: NSEventMaskScrollWheel
            untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES ];
            if( !pNextEvent )
                break;
            pEvent = pNextEvent;
        }

        NSPoint aPt = [NSEvent mouseLocation];
        mpFrame->CocoaToVCL( aPt );

        SalWheelMouseEvent aEvent;
        aEvent.mnTime           = mpFrame->mnLastEventTime;
        aEvent.mnX = static_cast<tools::Long>(aPt.x) - mpFrame->maGeometry.x();
        aEvent.mnY = static_cast<tools::Long>(aPt.y) - mpFrame->maGeometry.y();
        aEvent.mnCode           = ImplGetModifierMask( mpFrame->mnLastModifierFlags );
        aEvent.mbDeltaIsPixel   = true;

        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = mpFrame->maGeometry.width() - 1 - aEvent.mnX;

        if( dX != 0.0 )
        {
            aEvent.mnDelta = static_cast<tools::Long>(dX < 0 ? floor(dX) : ceil(dX));
            aEvent.mnNotchDelta = (dX < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = true;
            aEvent.mnScrollLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }
        if( dY != 0.0 && AquaSalFrame::isAlive( mpFrame ))
        {
            aEvent.mnDelta = static_cast<tools::Long>(dY < 0 ? floor(dY) : ceil(dY));
            aEvent.mnNotchDelta = (dY < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = false;
            aEvent.mnScrollLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }
    }
}

-(void)scrollWheel: (NSEvent*)pEvent
{
    SolarMutexGuard aGuard;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mnLastEventTime = static_cast<sal_uInt64>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        // merge pending scroll wheel events
        CGFloat dX = 0.0;
        CGFloat dY = 0.0;
        for(;;)
        {
            dX += [pEvent deltaX];
            dY += [pEvent deltaY];
            NSEvent* pNextEvent = [NSApp nextEventMatchingMask: NSEventMaskScrollWheel
                untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES ];
            if( !pNextEvent )
                break;
            pEvent = pNextEvent;
        }

        NSPoint aPt = [NSEvent mouseLocation];
        mpFrame->CocoaToVCL( aPt );

        SalWheelMouseEvent aEvent;
        aEvent.mnTime         = mpFrame->mnLastEventTime;
        aEvent.mnX = static_cast<tools::Long>(aPt.x) - mpFrame->maGeometry.x();
        aEvent.mnY = static_cast<tools::Long>(aPt.y) - mpFrame->maGeometry.y();
        aEvent.mnCode         = ImplGetModifierMask( mpFrame->mnLastModifierFlags );
        aEvent.mbDeltaIsPixel = false;

        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = mpFrame->maGeometry.width() - 1 - aEvent.mnX;

        if( dX != 0.0 )
        {
            aEvent.mnDelta = static_cast<tools::Long>(dX < 0 ? floor(dX) : ceil(dX));
            aEvent.mnNotchDelta = (dX < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = true;
            sal_uInt32 nScrollLines = fabs(dX) / WHEEL_EVENT_FACTOR;
            if (nScrollLines == 0)
                nScrollLines = 1;
            aEvent.mnScrollLines = nScrollLines;

            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }
        if( dY != 0.0 && AquaSalFrame::isAlive( mpFrame ) )
        {
            aEvent.mnDelta = static_cast<tools::Long>(dY < 0 ? floor(dY) : ceil(dY));
            aEvent.mnNotchDelta = (dY < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = false;
            sal_uInt32 nScrollLines = fabs(dY) / WHEEL_EVENT_FACTOR;
            if (nScrollLines == 0)
                nScrollLines = 1;
            aEvent.mnScrollLines = nScrollLines;

            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }

        // tdf#155266 force flush after scrolling
        mpFrame->mbForceFlush = true;
    }
}


-(void)keyDown: (NSEvent*)pEvent
{
    SolarMutexGuard aGuard;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        // Retain the event as it will be released sometime before a key up
        // event is dispatched
        [self clearLastEvent];
        mpLastEvent = [pEvent retain];

        mbInKeyInput = true;
        mbNeedSpecialKeyHandle = false;
        mbKeyHandled = false;

        mpFrame->mnLastEventTime = static_cast<sal_uInt64>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        if( ! [self handleKeyDownException: pEvent] )
        {
            sal_uInt16 nKeyCode = ImplMapKeyCode( [pEvent keyCode] );
            if ( nKeyCode == KEY_DELETE && mbTextInputWantsNonRepeatKeyDown )
            {
                // tdf#42437 Enable press-and-hold special character input method
                // Emulate the press-and-hold behavior of the TextEdit
                // application by deleting the marked text when only the
                // Delete key is pressed and keep the marked text when the
                // Backspace key or Fn-Delete keys are pressed.
                if ( [pEvent keyCode] == 51 )
                {
                    [self deleteTextInputWantsNonRepeatKeyDown];
                }
                else
                {
                    [self unmarkText];
                    mbKeyHandled = true;
                    mbInKeyInput = false;
                }

                [self endExtTextInput];
                return;
            }

            NSArray* pArray = [NSArray arrayWithObject: pEvent];
            [self interpretKeyEvents: pArray];

            // Handle repeat key events by explicitly inserting the text if
            // -[NSResponder interpretKeyEvents:] does not insert or mark any
            // text. Note: do not do this step if there is uncommitted text.
            // Related: tdf#42437 Skip special press-and-hold handling for action keys
            // Pressing and holding action keys such as arrow keys must not be
            // handled like pressing and holding a character key as it will
            // insert unexpected text.
            if ( !mbKeyHandled && !mpLastMarkedText && mpLastEvent && [mpLastEvent type] == NSEventTypeKeyDown && [mpLastEvent isARepeat] )
            {
                NSString *pChars = [mpLastEvent characters];
                if ( pChars )
                    [self insertText:pChars replacementRange:NSMakeRange( 0, [pChars length] )];
            }
            // tdf#42437 Enable press-and-hold special character input method
            // Emulate the press-and-hold behavior of the TextEdit application
            // by committing an empty string for key down events dispatched
            // while the special character input method popup is displayed.
            else if ( mpLastMarkedText && mbTextInputWantsNonRepeatKeyDown && mpLastEvent && [mpLastEvent type] == NSEventTypeKeyDown && ![mpLastEvent isARepeat] )
            {
                // If the escape or return key is pressed, unmark the text to
                // skip deletion of marked text
                if ( nKeyCode == KEY_ESCAPE || nKeyCode == KEY_RETURN )
                    [self unmarkText];
                [self insertText:[NSString string] replacementRange:NSMakeRange( NSNotFound, 0 )];
            }
        }

        mbInKeyInput = false;
    }
}

-(BOOL)handleKeyDownException:(NSEvent*)pEvent
{
    // check for a very special set of modified characters
    NSString* pUnmodifiedString = [pEvent charactersIgnoringModifiers];

    if( pUnmodifiedString && [pUnmodifiedString length] == 1 )
    {
        /* #i103102# key events with command and alternate don't make it through
           interpretKeyEvents (why?). Try to dispatch them here first,
           if not successful continue normally
        */
        if( (mpFrame->mnLastModifierFlags & (NSEventModifierFlagOption | NSEventModifierFlagCommand))
                    == (NSEventModifierFlagOption | NSEventModifierFlagCommand) )
        {
            if( [self sendSingleCharacter: mpLastEvent] )
                return YES;
        }
    }
    return NO;
}

-(void)flagsChanged: (NSEvent*)pEvent
{
    SolarMutexGuard aGuard;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mnLastEventTime = static_cast<sal_uInt64>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];
    }
}

-(void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
    (void) replacementRange; // FIXME: surely it must be used

    SolarMutexGuard aGuard;

    [self deleteTextInputWantsNonRepeatKeyDown];

    // Ignore duplicate events that are sometimes posted during cancellation
    // of the native input method session. This usually happens when
    // [self endExtTextInput] is called from [self windowDidBecomeKey:] and,
    // if the native input method popup, that was cancelled in a
    // previous call to [self windowDidResignKey:], has reappeared. In such
    // cases, the native input context posts the reappearing popup's
    // uncommitted text.
    if (mbInEndExtTextInput && !mbInCommitMarkedText)
        return;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        NSString* pInsert = nil;
        if( [aString isKindOfClass: [NSAttributedString class]] )
            pInsert = [aString string];
        else
            pInsert = aString;

        int nLen = 0;
        if( pInsert && ( nLen = [pInsert length] ) > 0 )
        {
            OUString aInsertString( GetOUString( pInsert ) );
             // aCharCode initializer is safe since aInsertString will at least contain '\0'
            sal_Unicode aCharCode = *aInsertString.getStr();

            if( nLen == 1 &&
                aCharCode < 0x80 &&
                aCharCode > 0x1f &&
                ! [self hasMarkedText ]
                )
            {
                sal_uInt16 nKeyCode = ImplMapCharCode( aCharCode );
                unsigned int nLastModifiers = mpFrame->mnLastModifierFlags;

                // #i99567#
                // find out the unmodified key code

                // sanity check
                if( mpLastEvent && ( [mpLastEvent type] == NSEventTypeKeyDown || [mpLastEvent type] == NSEventTypeKeyUp ) )
                {
                    // get unmodified string
                    NSString* pUnmodifiedString = [mpLastEvent charactersIgnoringModifiers];
                    if( pUnmodifiedString && [pUnmodifiedString length] == 1 )
                    {
                        // map the unmodified key code
                        unichar keyChar = [pUnmodifiedString characterAtIndex: 0];
                        nKeyCode = ImplMapCharCode( keyChar );
                    }
                    nLastModifiers = [mpLastEvent modifierFlags];

                }
                // #i99567#
                // applications and vcl's edit fields ignore key events with ALT
                // however we're at a place where we know text should be inserted
                // so it seems we need to strip the Alt modifier here
                if( (nLastModifiers & (NSEventModifierFlagControl | NSEventModifierFlagOption | NSEventModifierFlagCommand))
                    == NSEventModifierFlagOption )
                {
                    nLastModifiers = 0;
                }
                [self sendKeyInputAndReleaseToFrame: nKeyCode character: aCharCode modifiers: nLastModifiers];
            }
            else
            {
                SalExtTextInputEvent aEvent;
                aEvent.maText           = aInsertString;
                aEvent.mpTextAttr       = nullptr;
                aEvent.mnCursorPos      = aInsertString.getLength();
                aEvent.mnCursorFlags    = 0;
                mpFrame->CallCallback( SalEvent::ExtTextInput, &aEvent );
                if( AquaSalFrame::isAlive( mpFrame ) )
                    mpFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
            }
        }
        else
        {
            SalExtTextInputEvent aEvent;
            aEvent.maText.clear();
            aEvent.mpTextAttr       = nullptr;
            aEvent.mnCursorPos      = 0;
            aEvent.mnCursorFlags    = 0;
            mpFrame->CallCallback( SalEvent::ExtTextInput, &aEvent );
            if( AquaSalFrame::isAlive( mpFrame ) )
                mpFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );

        }
        [self unmarkText];
    }

    // Mark event as handled even if the frame isn't valid like is done in
    // [self setMarkedText:selectedRange:replacementRange:] and
    // [self doCommandBySelector:]
    mbKeyHandled = true;
}

-(void)insertTab: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_TAB character: '\t' modifiers: 0];
}

-(void)insertBacktab: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: (KEY_TAB | KEY_SHIFT) character: '\t' modifiers: 0];
}

-(void)moveLeft: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_LEFT character: 0 modifiers: 0];
}

-(void)moveLeftAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_LEFT character: 0 modifiers: NSEventModifierFlagShift];
}

-(void)moveBackwardAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_BACKWARD character: 0  modifiers: 0];
}

-(void)moveRight: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_RIGHT character: 0 modifiers: 0];
}

-(void)moveRightAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_RIGHT character: 0 modifiers: NSEventModifierFlagShift];
}

-(void)moveForwardAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_FORWARD character: 0  modifiers: 0];
}

-(void)moveWordLeft: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_WORD_BACKWARD character: 0  modifiers: 0];
}

-(void)moveWordBackward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_WORD_BACKWARD character: 0  modifiers: 0];
}

-(void)moveWordBackwardAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_WORD_BACKWARD character: 0  modifiers: 0];
}

-(void)moveWordLeftAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_WORD_BACKWARD character: 0  modifiers: 0];
}

-(void)moveWordRight: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_WORD_FORWARD character: 0  modifiers: 0];
}

-(void)moveWordForward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_WORD_FORWARD character: 0  modifiers: 0];
}

-(void)moveWordForwardAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_WORD_FORWARD character: 0  modifiers: 0];
}

-(void)moveWordRightAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_WORD_FORWARD character: 0  modifiers: 0];
}

-(void)moveToEndOfLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_END_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToRightEndOfLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_END_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToEndOfLineAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_END_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToRightEndOfLineAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_END_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToBeginningOfLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_BEGIN_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToLeftEndOfLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_BEGIN_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToBeginningOfLineAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_BEGIN_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToLeftEndOfLineAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_BEGIN_OF_LINE character: 0  modifiers: 0];
}

-(void)moveToEndOfParagraph: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_END_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveToEndOfParagraphAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_END_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveParagraphForward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_END_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveParagraphForwardAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_END_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveToBeginningOfParagraph: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveParagraphBackward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_BEGIN_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveToBeginningOfParagraphAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveParagraphBackwardAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_BEGIN_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)moveToEndOfDocument: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_END_OF_DOCUMENT character: 0  modifiers: 0];
}

-(void)scrollToEndOfDocument: (id)aSender
{
    (void)aSender;
    // this is not exactly what we should do, but it makes "End" and "Shift-End" behave consistent
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_END_OF_DOCUMENT character: 0  modifiers: 0];
}

-(void)moveToEndOfDocumentAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_END_OF_DOCUMENT character: 0  modifiers: 0];
}

-(void)moveToBeginningOfDocument: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT character: 0  modifiers: 0];
}

-(void)scrollToBeginningOfDocument: (id)aSender
{
    (void)aSender;
    // this is not exactly what we should do, but it makes "Home" and "Shift-Home" behave consistent
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::MOVE_TO_BEGIN_OF_DOCUMENT character: 0  modifiers: 0];
}

-(void)moveToBeginningOfDocumentAndModifySelection: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_TO_BEGIN_OF_DOCUMENT character: 0  modifiers: 0];
}

-(void)moveUp: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_UP character: 0 modifiers: 0];
}

-(void)moveDown: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_DOWN character: 0 modifiers: 0];
}

-(void)insertNewline: (id)aSender
{
    (void)aSender;
    // #i91267# make enter and shift-enter work by evaluating the modifiers
    [self sendKeyInputAndReleaseToFrame: KEY_RETURN character: '\n' modifiers: mpFrame->mnLastModifierFlags];
}

-(void)deleteBackward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_BACKSPACE character: '\b' modifiers: 0];
}

-(void)deleteForward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_DELETE character: 0x7f modifiers: 0];
}

-(void)deleteBackwardByDecomposingPreviousCharacter: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_BACKSPACE character: '\b' modifiers: 0];
}

-(void)deleteWordBackward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::DELETE_WORD_BACKWARD character: 0  modifiers: 0];
}

-(void)deleteWordForward: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::DELETE_WORD_FORWARD character: 0  modifiers: 0];
}

-(void)deleteToBeginningOfLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::DELETE_TO_BEGIN_OF_LINE character: 0  modifiers: 0];
}

-(void)deleteToEndOfLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::DELETE_TO_END_OF_LINE character: 0  modifiers: 0];
}

-(void)deleteToBeginningOfParagraph: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::DELETE_TO_BEGIN_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)deleteToEndOfParagraph: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::DELETE_TO_END_OF_PARAGRAPH character: 0  modifiers: 0];
}

-(void)insertLineBreak: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::INSERT_LINEBREAK character: 0  modifiers: 0];
}

-(void)insertParagraphSeparator: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::INSERT_PARAGRAPH character: 0  modifiers: 0];
}

-(void)selectWord: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_WORD character: 0  modifiers: 0];
}

-(void)selectLine: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_LINE character: 0  modifiers: 0];
}

-(void)selectParagraph: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_PARAGRAPH character: 0  modifiers: 0];
}

-(void)selectAll: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: css::awt::Key::SELECT_ALL character: 0  modifiers: 0];
}

-(void)cancelOperation: (id)aSender
{
    (void)aSender;
    [self sendKeyInputAndReleaseToFrame: KEY_ESCAPE character: 0x1b modifiers: 0];
}

-(void)noop: (id)aSender
{
    (void)aSender;
    if( ! mbKeyHandled )
    {
        if( ! [self sendSingleCharacter:mpLastEvent] )
        {
            /* prevent recursion */
            if( mpLastEvent != mpLastSuperEvent && [NSApp respondsToSelector: @selector(sendSuperEvent:)] )
            {
                id pLastSuperEvent = mpLastSuperEvent;
                mpLastSuperEvent = mpLastEvent;
                [NSApp performSelector:@selector(sendSuperEvent:) withObject: mpLastEvent];
                mpLastSuperEvent = pLastSuperEvent;

                std::map< NSEvent*, bool >::iterator it = GetSalData()->maKeyEventAnswer.find( mpLastEvent );
                if( it != GetSalData()->maKeyEventAnswer.end() )
                    it->second = true;
            }
        }
    }
}

-(BOOL)sendKeyInputAndReleaseToFrame: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar
{
    return [self sendKeyInputAndReleaseToFrame: nKeyCode character: aChar modifiers: mpFrame->mnLastModifierFlags];
}

-(BOOL)sendKeyInputAndReleaseToFrame: (sal_uInt16)nKeyCode character: (sal_Unicode)aChar modifiers: (unsigned int)nMod
{
    return [self sendKeyToFrameDirect: nKeyCode character: aChar modifiers: nMod] ||
           [self sendSingleCharacter: mpLastEvent];
}

-(BOOL)sendKeyToFrameDirect: (sal_uInt16)nKeyCode  character: (sal_Unicode)aChar modifiers: (unsigned int)nMod
{
    SolarMutexGuard aGuard;

    bool nRet = false;
    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        SalKeyEvent aEvent;
        aEvent.mnCode           = nKeyCode | ImplGetModifierMask( nMod );
        aEvent.mnCharCode       = aChar;
        aEvent.mnRepeat         = FALSE;
        nRet = mpFrame->CallCallback( SalEvent::KeyInput, &aEvent );
        std::map< NSEvent*, bool >::iterator it = GetSalData()->maKeyEventAnswer.find( mpLastEvent );
        if( it != GetSalData()->maKeyEventAnswer.end() )
            it->second = nRet;
        if( AquaSalFrame::isAlive( mpFrame ) )
            mpFrame->CallCallback( SalEvent::KeyUp, &aEvent );
    }
    return nRet;
}


-(BOOL)sendSingleCharacter: (NSEvent *)pEvent
{
    NSString* pUnmodifiedString = [pEvent charactersIgnoringModifiers];

    if( pUnmodifiedString && [pUnmodifiedString length] == 1 )
    {
        unichar keyChar = [pUnmodifiedString characterAtIndex: 0];
        sal_uInt16 nKeyCode = ImplMapCharCode( keyChar );
        if (nKeyCode == 0)
        {
            sal_uInt16 nOtherKeyCode = [pEvent keyCode];
            nKeyCode = ImplMapKeyCode(nOtherKeyCode);
        }
        if( nKeyCode != 0 )
        {
            // don't send code points in the private use area
            if( keyChar >= 0xf700 && keyChar < 0xf780 )
                keyChar = 0;
            bool bRet = [self sendKeyToFrameDirect: nKeyCode character: keyChar modifiers: mpFrame->mnLastModifierFlags];
            mbInKeyInput = false;

            return bRet;
        }
    }
    return NO;
}


// NSTextInput/NSTextInputClient protocol
- (NSArray *)validAttributesForMarkedText
{
    return [NSArray arrayWithObjects:NSUnderlineStyleAttributeName, nil];
}

- (BOOL)hasMarkedText
{
    bool bHasMarkedText;

    bHasMarkedText = ( mMarkedRange.location != NSNotFound ) &&
                     ( mMarkedRange.length != 0 );
    // hack to check keys like "Control-j"
    if( mbInKeyInput )
    {
        mbNeedSpecialKeyHandle = true;
    }

    // FIXME:
    // #i106901#
    // if we come here outside of mbInKeyInput, this is likely to be because
    // of the keyboard viewer. For unknown reasons having no marked range
    // in this case causes a crash. So we say we have a marked range anyway
    // This is a hack, since it is not understood what a) causes that crash
    // and b) why we should have a marked range at this point.
    if( ! mbInKeyInput )
        bHasMarkedText = true;

    return bHasMarkedText;
}

- (NSRange)markedRange
{
    // FIXME:
    // #i106901#
    // if we come here outside of mbInKeyInput, this is likely to be because
    // of the keyboard viewer. For unknown reasons having no marked range
    // in this case causes a crash. So we say we have a marked range anyway
    // This is a hack, since it is not understood what a) causes that crash
    // and b) why we should have a marked range at this point. Stop the native
    // input method popup from appearing in the bottom left corner of the
    // screen by returning the marked range if is valid when called outside of
    // mbInKeyInput. If a zero length range is returned, macOS won't call
    // [self firstRectForCharacterRange:actualRange:] for any newly appended
    // uncommitted text.
    if( ! mbInKeyInput )
        return mMarkedRange.location != NSNotFound ? mMarkedRange : NSMakeRange( 0, 0 );

    return [self hasMarkedText] ? mMarkedRange : NSMakeRange( NSNotFound, 0 );
}

- (NSRange)selectedRange
{
    // tdf#42437 Enable press-and-hold special character input method
    // Always return a valid range location. If the range location is
    // NSNotFound, -[NSResponder interpretKeyEvents:] will not call
    // [self firstRectForCharacterRange:actualRange:] and will not display the
    // special character input method popup.
    return ( mSelectedRange.location == NSNotFound ? NSMakeRange( 0, 0 ) : mSelectedRange );
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange replacementRange:(NSRange)replacementRange
{
    (void) replacementRange; // FIXME - use it!

    SolarMutexGuard aGuard;

    [self deleteTextInputWantsNonRepeatKeyDown];

    if( ![aString isKindOfClass:[NSAttributedString class]] )
        aString = [[[NSAttributedString alloc] initWithString:aString] autorelease];

    // Reset cached state
    [self unmarkText];

    int len = [aString length];
    SalExtTextInputEvent aInputEvent;
    if( len > 0 ) {
        // Set the marked and selected ranges to the marked text and selected
        // range parameters
        mMarkedRange = NSMakeRange( 0, [aString length] );
        if (selRange.location == NSNotFound || selRange.location >= mMarkedRange.length)
             mSelectedRange = NSMakeRange( NSNotFound, 0 );
        else
             mSelectedRange = NSMakeRange( selRange.location, selRange.location + selRange.length > mMarkedRange.length ? mMarkedRange.length - selRange.location : selRange.length );

        // If we are going to post uncommitted text, cache the string parameter
        // as is needed in both [self endExtTextInput] and
        // [self attributedSubstringForProposedRange:actualRange:]
        mpLastMarkedText = [aString retain];

        NSString *pString = [aString string];
        OUString aInsertString( GetOUString( pString ) );
        std::vector<ExtTextInputAttr> aInputFlags( std::max( 1, len ), ExtTextInputAttr::NONE );
        int nSelectionStart = (mSelectedRange.location == NSNotFound ? len : mSelectedRange.location);
        int nSelectionEnd = (mSelectedRange.location == NSNotFound ? len : mSelectedRange.location + selRange.length);
        for ( int i = 0; i < len; i++ )
        {
            // Highlight all characters in the selected range. Normally
            // uncommitted text is underlined but when an item is selected in
            // the native input method popup or selecting a subblock of
            // uncommitted text using the left or right arrow keys, the
            // selection range is set and the selected range is either
            // highlighted like in Excel or is bold underlined like in
            // Safari. Highlighting the selected range was chosen because
            // using bold and double underlines can get clipped making the
            // selection range indistinguishable from the rest of the
            // uncommitted text.
            if (i >= nSelectionStart && i < nSelectionEnd)
            {
                aInputFlags[i] = ExtTextInputAttr::Highlight;
                continue;
            }

            unsigned int nUnderlineValue;
            NSRange effectiveRange;

            effectiveRange = NSMakeRange(i, 1);
            nUnderlineValue = [[aString attribute:NSUnderlineStyleAttributeName atIndex:i effectiveRange:&effectiveRange] unsignedIntValue];

            switch (nUnderlineValue & 0xff) {
            case NSUnderlineStyleSingle:
                aInputFlags[i] = ExtTextInputAttr::Underline;
                break;
            case NSUnderlineStyleThick:
                aInputFlags[i] = ExtTextInputAttr::BoldUnderline;
                break;
            case NSUnderlineStyleDouble:
                aInputFlags[i] = ExtTextInputAttr::DoubleUnderline;
                break;
            default:
                aInputFlags[i] = ExtTextInputAttr::Highlight;
                break;
            }
        }

        aInputEvent.maText = aInsertString;
        aInputEvent.mnCursorPos = nSelectionStart;
        aInputEvent.mnCursorFlags = 0;
        aInputEvent.mpTextAttr = aInputFlags.data();
        mpFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void *>(&aInputEvent) );
    } else {
        aInputEvent.maText.clear();
        aInputEvent.mnCursorPos = 0;
        aInputEvent.mnCursorFlags = 0;
        aInputEvent.mpTextAttr = nullptr;
        mpFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void *>(&aInputEvent) );
        mpFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
    }
    mbKeyHandled= true;
}

- (void)unmarkText
{
    [self clearLastMarkedText];

    mSelectedRange = mMarkedRange = NSMakeRange(NSNotFound, 0);
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
    (void) aRange;
    (void) actualRange;

    // FIXME - Implement
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)thePoint
{
    (void)thePoint;
    // FIXME
    return 0;
}

- (NSInteger)conversationIdentifier
{
    return reinterpret_cast<long>(self);
}

- (void)doCommandBySelector:(SEL)aSelector
{
    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        if( (mpFrame->mnICOptions & InputContextFlags::Text) &&
            aSelector != nullptr && [self respondsToSelector: aSelector] )
        {
            [self performSelector: aSelector];
        }
        else
        {
            [self sendSingleCharacter:mpLastEvent];
        }
    }

    mbKeyHandled = true;
}

-(void)clearLastEvent
{
    if (mpLastEvent)
    {
        [mpLastEvent release];
        mpLastEvent = nil;
    }
}

-(void)clearLastMarkedText
{
    if (mpLastMarkedText)
    {
        [mpLastMarkedText release];
        mpLastMarkedText = nil;
    }

    mbTextInputWantsNonRepeatKeyDown = NO;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
     // FIXME - These should probably be used?
    (void) aRange;
    (void) actualRange;

    SolarMutexGuard aGuard;

    // tdf#42437 Enable press-and-hold special character input method
    // Some text entry controls, such as Writer comments or the cell editor in
    // Calc's Formula Bar, need to have an input method session open or else
    // the returned position won't be anywhere near the text cursor. So,
    // dispatch an empty SalEvent::ExtTextInput event, fetch the position,
    // and then dispatch a SalEvent::EndExtTextInput event.
    NSString *pNewMarkedText = nullptr;
    NSString *pChars = [mpLastEvent characters];

    // tdf#158124 KEY_DELETE events do not need an ExtTextInput event
    // When using various Japanese input methods, the last event will be a
    // repeating key down event with a single delete character while the
    // Backspace key, Delete key, or Fn-Delete keys are pressed. These key
    // events are now ignored since setting mbTextInputWantsNonRepeatKeyDown
    // to YES for these events will trigger an assert or crash when saving a
    // .docx document.
    bool bNeedsExtTextInput = ( pChars && mbInKeyInput && !mpLastMarkedText && mpLastEvent && [mpLastEvent type] == NSEventTypeKeyDown && [mpLastEvent isARepeat] && ImplMapKeyCode( [mpLastEvent keyCode] ) != KEY_DELETE );
    if ( bNeedsExtTextInput )
    {
        // tdf#154708 Preserve selection for repeating Shift-arrow on Japanese keyboard
        // Skip the posting of SalEvent::ExtTextInput and
        // SalEvent::EndExtTextInput events for private use area characters.
        NSUInteger nLen = [pChars length];
        auto const pBuf = std::make_unique<unichar[]>( nLen + 1 );
        NSUInteger nBufLen = 0;
        for ( NSUInteger i = 0; i < nLen; i++ )
        {
            unichar aChar = [pChars characterAtIndex:i];
            if ( aChar >= 0xf700 && aChar < 0xf780 )
                continue;

            pBuf[nBufLen++] = aChar;
        }
        pBuf[nBufLen] = 0;

        pNewMarkedText = [NSString stringWithCharacters:pBuf.get() length:nBufLen];
        if (!pNewMarkedText || ![pNewMarkedText length])
            bNeedsExtTextInput = false;
    }

    if ( bNeedsExtTextInput )
    {
        SalExtTextInputEvent aInputEvent;
        aInputEvent.maText.clear();
        aInputEvent.mnCursorPos = 0;
        aInputEvent.mnCursorFlags = 0;
        aInputEvent.mpTextAttr = nullptr;
        if ( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
            mpFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void *>(&aInputEvent) );
    }

    SalExtTextInputPosEvent aPosEvent;
    if ( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        mpFrame->CallCallback( SalEvent::ExtTextInputPos, static_cast<void *>(&aPosEvent) );

    if ( bNeedsExtTextInput )
    {
        if ( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
            mpFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );

        // tdf#42437 Enable press-and-hold special character input method
        // Emulate the press-and-hold behavior of the TextEdit application by
        // setting the marked text to the last key down event's characters. The
        // characters will already have been committed by the special character
        // input method so set the mbTextInputWantsNonRepeatKeyDown flag to
        // indicate that the characters need to be deleted if the input method
        // replaces the committed characters.
        if ( pNewMarkedText )
        {
            [self unmarkText];
            mpLastMarkedText = [[NSAttributedString alloc] initWithString:pNewMarkedText];
            mSelectedRange = mMarkedRange = NSMakeRange( 0, [mpLastMarkedText length] );
            mbTextInputWantsNonRepeatKeyDown = YES;
        }
    }

    NSRect rect;

    if ( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        rect.origin.x = aPosEvent.mnX + mpFrame->maGeometry.x();
        rect.origin.y = aPosEvent.mnY + mpFrame->maGeometry.y() + 4; // add some space for underlines
        rect.size.width = aPosEvent.mnWidth;
        rect.size.height = aPosEvent.mnHeight;

        mpFrame->VCLToCocoa( rect );
    }
    else
    {
        rect = NSMakeRect( aPosEvent.mnX, aPosEvent.mnY, aPosEvent.mnWidth, aPosEvent.mnHeight );
    }

    return rect;
}

-(id)parentAttribute {
    return reinterpret_cast<NSView*>(mpFrame->getNSWindow());
        //TODO: odd cast really needed for fdo#74121?
}

-(css::accessibility::XAccessibleContext *)accessibleContext
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibleContext];

    return nil;
}

-(NSWindow*)windowForParent
{
    return mpFrame->getNSWindow();
}

-(void)registerMouseEventListener: (id)theListener
{
  mpMouseEventListener = theListener;
}

-(void)unregisterMouseEventListener: (id)theListener
{
    (void)theListener;
    mpMouseEventListener = nil;
}

-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler draggingEntered: sender];
}

-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler draggingUpdated: sender];
}

-(void)draggingExited:(id <NSDraggingInfo>)sender
{
  [mDraggingDestinationHandler draggingExited: sender];
}

-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler prepareForDragOperation: sender];
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
  return [mDraggingDestinationHandler performDragOperation: sender];
}

-(void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
  [mDraggingDestinationHandler concludeDragOperation: sender];
}

-(void)registerDraggingDestinationHandler:(id)theHandler
{
  mDraggingDestinationHandler = theHandler;
}

-(void)unregisterDraggingDestinationHandler:(id)theHandler
{
    (void)theHandler;
    mDraggingDestinationHandler = nil;
}

-(void)endExtTextInput
{
    [self endExtTextInput:EndExtTextInputFlags::Complete];
}

-(void)endExtTextInput:(EndExtTextInputFlags)nFlags
{
    // Prevent recursion from any additional [self insertText:] calls that
    // may be called when cancelling the native input method session
    if (mbInEndExtTextInput)
        return;

    mbInEndExtTextInput = YES;

    SolarMutexGuard aGuard;

    NSTextInputContext *pInputContext = [NSTextInputContext currentInputContext];
    if (pInputContext)
    {
        // Cancel the native input method session
        [pInputContext discardMarkedText];

        // Commit any uncommitted text. Note: when the delete key is used to
        // remove all uncommitted characters, the marked range will be zero
        // length but a SalEvent::EndExtTextInput must still be dispatched.
        if (mpLastMarkedText && [mpLastMarkedText length] && mMarkedRange.location != NSNotFound && mpFrame && AquaSalFrame::isAlive(mpFrame))
        {
            // If there is any marked text, SalEvent::EndExtTextInput may leave
            // the cursor hidden so commit the marked text to force the cursor
            // to be visible.
            mbInCommitMarkedText = YES;
            if (nFlags & EndExtTextInputFlags::Complete)
            {
                // Retain the last marked text as it will be released in
                // [self insertText:replacementText:]
                NSAttributedString *pText = [mpLastMarkedText retain];
                [self insertText:pText replacementRange:NSMakeRange(0, [mpLastMarkedText length])];
                [pText release];
            }
            else
            {
                [self insertText:[NSString string] replacementRange:NSMakeRange(0, 0)];
            }
            mbInCommitMarkedText = NO;
        }

        [self unmarkText];

        // If a different view is the input context's client, commit that
        // view's uncommitted text as well
        id<NSTextInputClient> pClient = [pInputContext client];
        if (pClient != self)
        {
            SalFrameView *pView = static_cast<SalFrameView*>(pClient);
            if ([pView isKindOfClass:[SalFrameView class]])
                [pView endExtTextInput];
            else
                [pClient unmarkText];
        }
    }

    mbInEndExtTextInput = NO;
}

-(void)deleteTextInputWantsNonRepeatKeyDown
{
    SolarMutexGuard aGuard;

    // tdf#42437 Enable press-and-hold special character input method
    // Emulate the press-and-hold behavior of the TextEdit application by
    // dispatching backspace events to delete any marked characters. The
    // special character input method commits the marked characters so we must
    // delete the marked characters before the input method calls
    // [self insertText:replacementRange:].
    if (mbTextInputWantsNonRepeatKeyDown)
    {
        if ( mpLastMarkedText )
        {
            NSString *pChars = [mpLastMarkedText string];
            if ( pChars )
            {
                NSUInteger nLength = [pChars length];
                for ( NSUInteger i = 0; i < nLength; i++ )
                    [self deleteBackward:self];
            }
        }

        [self unmarkText];
    }
}

-(void)insertRegisteredWrapperIntoWrapperRepository
{
    SolarMutexGuard aGuard;

    if (!mbNeedChildWrapper)
        return;

    vcl::Window *pWindow = mpFrame->GetWindow();
    if (!pWindow)
        return;

    mbNeedChildWrapper = NO;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xAccessibleContext( pWindow->GetAccessible()->getAccessibleContext() );
    assert(!mpChildWrapper);
    mpChildWrapper = [[SalFrameViewA11yWrapper alloc] initWithParent:self accessibleContext:xAccessibleContext];
    [AquaA11yFactory insertIntoWrapperRepository:mpChildWrapper forAccessibleContext:xAccessibleContext];
}

-(void)registerWrapper
{
    [self revokeWrapper];

    mbNeedChildWrapper = YES;
}

-(void)revokeWrapper
{
    mbNeedChildWrapper = NO;

    if (mpChildWrapper)
    {
        [AquaA11yFactory revokeWrapper:mpChildWrapper];
        [mpChildWrapper setAccessibilityParent:nil];
        [mpChildWrapper release];
        mpChildWrapper = nil;
    }
}

-(id)accessibilityAttributeValue:(NSString *)pAttribute
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityAttributeValue:pAttribute];
    else
        return nil;
}

-(BOOL)accessibilityIsIgnored
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityIsIgnored];
    else
        return YES;
}

-(NSArray *)accessibilityAttributeNames
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityAttributeNames];
    else
        return [NSArray array];
}

-(BOOL)accessibilityIsAttributeSettable:(NSString *)pAttribute
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityIsAttributeSettable:pAttribute];
    else
        return NO;
}

-(NSArray *)accessibilityParameterizedAttributeNames
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityParameterizedAttributeNames];
    else
        return [NSArray array];
}

-(BOOL)accessibilitySetOverrideValue:(id)pValue forAttribute:(NSString *)pAttribute
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilitySetOverrideValue:pValue forAttribute:pAttribute];
    else
        return NO;
}

-(void)accessibilitySetValue:(id)pValue forAttribute:(NSString *)pAttribute
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        [mpChildWrapper accessibilitySetValue:pValue forAttribute:pAttribute];
}

-(id)accessibilityAttributeValue:(NSString *)pAttribute forParameter:(id)pParameter
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityAttributeValue:pAttribute forParameter:pParameter];
    else
        return nil;
}

-(id)accessibilityFocusedUIElement
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityFocusedUIElement];
    else
        return nil;
}

-(NSString *)accessibilityActionDescription:(NSString *)pAction
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityActionDescription:pAction];
    else
        return nil;
}

-(void)accessibilityPerformAction:(NSString *)pAction
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        [mpChildWrapper accessibilityPerformAction:pAction];
}

-(NSArray *)accessibilityActionNames
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityActionNames];
    else
        return [NSArray array];
}

-(id)accessibilityHitTest:(NSPoint)aPoint
{
    SolarMutexGuard aGuard;

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        return [mpChildWrapper accessibilityHitTest:aPoint];
    else
        return nil;
}

-(id)accessibilityParent
{
    return [self window];
}

-(NSArray *)accessibilityVisibleChildren
{
    return [self accessibilityChildren];
}

-(NSArray *)accessibilitySelectedChildren
{
    SolarMutexGuard aGuard;

    NSArray *pRet = [super accessibilityChildren];

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        pRet = getMergedAccessibilityChildren(pRet, [mpChildWrapper accessibilitySelectedChildren]);

    return pRet;
}

-(NSArray *)accessibilityChildren
{
    SolarMutexGuard aGuard;

    NSArray *pRet = [super accessibilityChildren];

    [self insertRegisteredWrapperIntoWrapperRepository];
    if (mpChildWrapper)
        pRet = getMergedAccessibilityChildren(pRet, [mpChildWrapper accessibilityChildren]);

    return pRet;
}

-(NSArray <id<NSAccessibilityElement>> *)accessibilityChildrenInNavigationOrder
{
    return [self accessibilityChildren];
}

@end

@implementation SalFrameViewA11yWrapper

-(id)initWithParent:(SalFrameView *)pParentView accessibleContext:(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >&)rxAccessibleContext
{
    [super init];

    maReferenceWrapper.rAccessibleContext = rxAccessibleContext;

    mpParentView = pParentView;
    if (mpParentView)
    {
        [mpParentView retain];
        [self setAccessibilityParent:mpParentView];
    }

    return self;
}

-(void)dealloc
{
    if (mpParentView)
        [mpParentView release];

    [super dealloc];
}

-(id)parentAttribute
{
    if (mpParentView)
        return NSAccessibilityUnignoredAncestor(mpParentView);
    else
        return nil;
}

-(void)setAccessibilityParent:(id)pObject
{
    if (mpParentView)
    {
        [mpParentView release];
        mpParentView = nil;
    }

    if (pObject && [pObject isKindOfClass:[SalFrameView class]])
    {
        mpParentView = static_cast<SalFrameView *>(pObject);
        [mpParentView retain];
    }

    [super setAccessibilityParent:mpParentView];
}

-(id)windowAttribute
{
    if (mpParentView)
        return [mpParentView window];
    else
        return nil;
}

-(NSWindow *)windowForParent
{
    return [self windowAttribute];
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
