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

#include <sal/macros.h>
#include <tools/helpers.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include "osx/a11yfactory.h"
#include "osx/salframe.h"
#include "osx/salframeview.h"
#include "osx/salinst.h"
#include "quartz/salgdi.h"
#include "quartz/utils.h"

#define WHEEL_EVENT_FACTOR 1.5

// for fullscreen support on OS X < 10.7
#if MACOSX_SDK_VERSION < 1070
    #define NSWindowCollectionBehaviorFullScreenPrimary   (1 << 7)
    #define NSWindowCollectionBehaviorFullScreenAuxiliary (1 << 8)
//  #define NSFullScreenWindowMask (1 << 14)
#endif


static sal_uInt16 ImplGetModifierMask( unsigned int nMask )
{
    sal_uInt16 nRet = 0;
    if( (nMask & NSShiftKeyMask) != 0 )
        nRet |= KEY_SHIFT;
    if( (nMask & NSControlKeyMask) != 0 )
        nRet |= KEY_MOD3;
    if( (nMask & NSAlternateKeyMask) != 0 )
        nRet |= KEY_MOD2;
    if( (nMask & NSCommandKeyMask) != 0 )
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
        KEY_EQUAL,            KEY_9,            KEY_7,     KEY_SUBTRACT,            KEY_8,            KEY_0, KEY_BRACKETRIGHT,            KEY_0,
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

// combinations of keys we need to handle ourselves
static const struct ExceptionalKey
{
    const sal_uInt16        nKeyCode;
    const unsigned int  nModifierMask;
} aExceptionalKeys[] =
{
    { KEY_D, NSControlKeyMask | NSShiftKeyMask | NSAlternateKeyMask },
    { KEY_D, NSCommandKeyMask | NSShiftKeyMask | NSAlternateKeyMask }
};

static AquaSalFrame* getMouseContainerFrame()
{
    AquaSalFrame* pDispatchFrame = nullptr;
#if MACOSX_SDK_VERSION < 1060
    NSInteger nWindows = 0;
    NSCountWindows( &nWindows );
    NSInteger* pWindows = (NSInteger*)alloca( nWindows * sizeof(NSInteger) );
    NSWindowList( nWindows, pWindows ); // NSWindowList is supposed to be in z-order front to back
    for(int i = 0; i < nWindows && ! pDispatchFrame; i++ )
    {
        NSWindow* pWin = [NSApp windowWithWindowNumber: pWindows[i]];
        if( pWin && [pWin isMemberOfClass: [SalFrameWindow class]] && [(SalFrameWindow*)pWin containsMouse] )
            pDispatchFrame = [(SalFrameWindow*)pWin getSalFrame];
    }
#else
    NSArray* aWindows = [NSWindow windowNumbersWithOptions:0];
    for(NSUInteger i = 0; i < [aWindows count] && ! pDispatchFrame; i++ )
    {
        NSWindow* pWin = [NSApp windowWithWindowNumber:[[aWindows objectAtIndex:i] integerValue]];
        if( pWin && [pWin isMemberOfClass: [SalFrameWindow class]] && [(SalFrameWindow*)pWin containsMouse] )
            pDispatchFrame = [(SalFrameWindow*)pWin getSalFrame];
    }
#endif
    return pDispatchFrame;
}

@implementation SalFrameWindow
-(id)initWithSalFrame: (AquaSalFrame*)pFrame
{
    mDraggingDestinationHandler = nil;
    mpFrame = pFrame;
    NSRect aRect = { { static_cast<CGFloat>(pFrame->maGeometry.nX), static_cast<CGFloat>(pFrame->maGeometry.nY) },
                     { static_cast<CGFloat>(pFrame->maGeometry.nWidth), static_cast<CGFloat>(pFrame->maGeometry.nHeight) } };
    pFrame->VCLToCocoa( aRect );
    NSWindow* pNSWindow = [super initWithContentRect: aRect styleMask: mpFrame->getStyleMask() backing: NSBackingStoreBuffered defer: NO ];
#if MACOSX_SDK_VERSION < 101000
    [pNSWindow useOptimizedDrawing: YES]; // OSX recommendation when there are no overlapping subviews within the receiver
#endif

    // enable OSX>=10.7 fullscreen options if available and useful
    bool bAllowFullScreen = (SalFrameStyleFlags::NONE == (mpFrame->mnStyle & (SalFrameStyleFlags::DIALOG | SalFrameStyleFlags::TOOLTIP | SalFrameStyleFlags::SYSTEMCHILD | SalFrameStyleFlags::FLOAT | SalFrameStyleFlags::TOOLWINDOW | SalFrameStyleFlags::INTRO)));
    bAllowFullScreen &= (SalFrameStyleFlags::NONE == (~mpFrame->mnStyle & (SalFrameStyleFlags::SIZEABLE)));
    bAllowFullScreen &= (mpFrame->mpParent == nullptr);
    const SEL setCollectionBehavior = @selector(setCollectionBehavior:);
    if( bAllowFullScreen && [pNSWindow respondsToSelector: setCollectionBehavior])
    {
        const int bMode= (bAllowFullScreen ? NSWindowCollectionBehaviorFullScreenPrimary : NSWindowCollectionBehaviorFullScreenAuxiliary);
        [pNSWindow performSelector:setCollectionBehavior withObject:reinterpret_cast<id>(static_cast<intptr_t>(bMode))];
    }

    // disable OSX>=10.7 window restoration until we support it directly
    const SEL setRestorable = @selector(setRestorable:);
    if( [pNSWindow respondsToSelector: setRestorable]) {
        [pNSWindow performSelector:setRestorable withObject:reinterpret_cast<id>(NO)];
    }

    return (SalFrameWindow *)pNSWindow;
}

-(AquaSalFrame*)getSalFrame
{
    return mpFrame;
}

-(void)displayIfNeeded
{
    if( GetSalData() && GetSalData()->mpFirstInstance )
    {
        comphelper::SolarMutex* pMutex = GetSalData()->mpFirstInstance->GetYieldMutex();
        if( pMutex )
        {
            pMutex->acquire();
            [super displayIfNeeded];
            pMutex->release();
        }
    }
}

-(BOOL)containsMouse
{
    // is this event actually inside that NSWindow ?
    NSPoint aPt = [NSEvent mouseLocation];
    NSRect aFrameRect = [self frame];
    BOOL bInRect = NSPointInRect( aPt, aFrameRect );
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

        if( mpFrame->mpMenu )
            mpFrame->mpMenu->setMainMenu();
        else if( ! mpFrame->mpParent &&
                 ( (mpFrame->mnStyle & nGuessDocument) == nGuessDocument || // set default menu for e.g. help
                    mpFrame->mbFullScreen ) )                               // ser default menu for e.g. presentation
        {
            AquaSalMenu::setDefaultMenu();
        }
        #if 0
        // FIXME: we should disable menus while in modal mode
        // however from down here there is currently no reliable way to
        // find out when to do this
        if( (mpFrame->mpParent && mpFrame->mpParent->GetWindow()->IsInModalMode()) )
            AquaSalMenu::enableMainMenu( false );
        #endif
        mpFrame->CallCallback( SalEvent::GetFocus, nullptr );
        mpFrame->SendPaintEvent(); // repaint controls as active
    }
}

-(void)windowDidResignKey: (NSNotification*)pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

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
    (void)pNotification;
    SolarMutexGuard aGuard;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SalEvent::Resize, nullptr );
        mpFrame->SendPaintEvent();
    }
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

    BOOL bRet = YES;
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        // #i84461# end possible input
        mpFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
        if( AquaSalFrame::isAlive( mpFrame ) )
        {
            mpFrame->CallCallback( SalEvent::Close, nullptr );
            bRet = NO; // application will close the window or not, AppKit shouldn't
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
        mMarkedRange = NSMakeRange(NSNotFound, 0);
        mSelectedRange = NSMakeRange(NSNotFound, 0);
        mpReferenceWrapper = reinterpret_cast<ReferenceWrapper*>(nil);
        mpMouseEventListener = nil;
        mpLastSuperEvent = nil;
    }

    mfLastMagnifyTime = 0.0;
    return self;
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
        const NSRect aRect = { NSZeroPoint, NSMakeSize( mpFrame->maGeometry.nWidth, mpFrame->maGeometry.nHeight) };
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

// helper class similar to a osl::Guard< comphelper::SolarMutex > for the
// SalYieldMutex; the difference is that it only does tryToAcquire instead of
// acquire so dreaded deadlocks like #i93512# are prevented
class TryGuard
{
public:
            TryGuard()  { mbGuarded = ImplSalYieldMutexTryToAcquire(); }
            ~TryGuard() { if( mbGuarded ) ImplSalYieldMutexRelease(); }
    bool    IsGuarded() { return mbGuarded; }
private:
    bool    mbGuarded;
};

-(void)drawRect: (NSRect)aRect
{
    // HOTFIX: #i93512# prevent deadlocks if any other thread already has the SalYieldMutex
    TryGuard aTryGuard;
    if( !aTryGuard.IsGuarded() )
    {
        // NOTE: the mpFrame access below is not guarded yet!
        // TODO: mpFrame et al need to be guarded by an independent mutex
        AquaSalGraphics* pGraphics = (mpFrame && AquaSalFrame::isAlive(mpFrame)) ? mpFrame->mpGraphics : nullptr;
        if( pGraphics )
        {
            // we did not get the mutex so we cannot draw now => request to redraw later
            // convert the NSRect to a CGRect for Refreshrect()
            const CGRect aCGRect = {{aRect.origin.x,aRect.origin.y},{aRect.size.width,aRect.size.height}};
            pGraphics->RefreshRect( aCGRect );
        }
        return;
    }

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        if( mpFrame->mpGraphics )
        {
            mpFrame->mpGraphics->UpdateWindow( aRect );
            if( mpFrame->getClipPath() )
                [mpFrame->getNSWindow() invalidateShadow];
        }
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
        aEvent.mnX      = static_cast<long>(aPt.x) - pDispatchFrame->maGeometry.nX;
        aEvent.mnY      = static_cast<long>(aPt.y) - pDispatchFrame->maGeometry.nY;
        aEvent.mnButton = nButton;
        aEvent.mnCode   =  aEvent.mnButton | nModMask;

        // --- RTL --- (mirror mouse pos)
        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = pDispatchFrame->maGeometry.nWidth-1-aEvent.mnX;

        pDispatchFrame->CallCallback( nEvent, &aEvent );
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
#if MACOSX_SDK_VERSION < 1060
        mfMagnifyDeltaSum += [pEvent deltaZ];
#else
        mfMagnifyDeltaSum += [pEvent magnification];
#endif

        mfLastMagnifyTime = [pEvent timestamp];
// TODO: change to 0.1 when CommandWheelMode::ZOOM handlers allow finer zooming control
#if MACOSX_SDK_VERSION < 1060
        static const float fMagnifyFactor = 0.25;
#else
        static const float fMagnifyFactor = 0.25*500; // steps are 500 times smaller for -magnification
#endif
        static const float fMinMagnifyStep = 15.0 / fMagnifyFactor;
        if( fabs(mfMagnifyDeltaSum) <= fMinMagnifyStep )
            return;

        // adapt NSEvent-sensitivity to application expectations
        // TODO: rather make CommandWheelMode::ZOOM handlers smarter
        const float fDeltaZ = mfMagnifyDeltaSum * fMagnifyFactor;
        int nDeltaZ = FRound( fDeltaZ );
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
        aEvent.mnX              = static_cast<long>(aPt.x) - mpFrame->maGeometry.nX;
        aEvent.mnY              = static_cast<long>(aPt.y) - mpFrame->maGeometry.nY;
        aEvent.mnCode           = ImplGetModifierMask( mpFrame->mnLastModifierFlags );
        aEvent.mnCode           |= KEY_MOD1; // we want zooming, no scrolling
        aEvent.mbDeltaIsPixel   = TRUE;

        // --- RTL --- (mirror mouse pos)
        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = mpFrame->maGeometry.nWidth-1-aEvent.mnX;

        aEvent.mnDelta = nDeltaZ;
        aEvent.mnNotchDelta = (nDeltaZ >= 0) ? +1 : -1;
        if( aEvent.mnDelta == 0 )
            aEvent.mnDelta = aEvent.mnNotchDelta;
        aEvent.mbHorz = FALSE;
        aEvent.mnScrollLines = nDeltaZ;
        if( aEvent.mnScrollLines == 0 )
            aEvent.mnScrollLines = 1;
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
            NSEvent* pNextEvent = [NSApp nextEventMatchingMask: NSScrollWheelMask
            untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES ];
            if( !pNextEvent )
                break;
            pEvent = pNextEvent;
        }

        NSPoint aPt = [NSEvent mouseLocation];
        mpFrame->CocoaToVCL( aPt );

        SalWheelMouseEvent aEvent;
        aEvent.mnTime           = mpFrame->mnLastEventTime;
        aEvent.mnX              = static_cast<long>(aPt.x) - mpFrame->maGeometry.nX;
        aEvent.mnY              = static_cast<long>(aPt.y) - mpFrame->maGeometry.nY;
        aEvent.mnCode           = ImplGetModifierMask( mpFrame->mnLastModifierFlags );
        aEvent.mbDeltaIsPixel   = TRUE;

        // --- RTL --- (mirror mouse pos)
        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = mpFrame->maGeometry.nWidth-1-aEvent.mnX;

        if( dX != 0.0 )
        {
            aEvent.mnDelta = static_cast<long>(floor(dX));
            aEvent.mnNotchDelta = (dX < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = TRUE;
            aEvent.mnScrollLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }
        if( dY != 0.0 && AquaSalFrame::isAlive( mpFrame ))
        {
            aEvent.mnDelta = static_cast<long>(floor(dY));
            aEvent.mnNotchDelta = (dY < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = FALSE;
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
            NSEvent* pNextEvent = [NSApp nextEventMatchingMask: NSScrollWheelMask
                untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES ];
            if( !pNextEvent )
                break;
            pEvent = pNextEvent;
        }

        NSPoint aPt = [NSEvent mouseLocation];
        mpFrame->CocoaToVCL( aPt );

        SalWheelMouseEvent aEvent;
        aEvent.mnTime         = mpFrame->mnLastEventTime;
        aEvent.mnX            = static_cast<long>(aPt.x) - mpFrame->maGeometry.nX;
        aEvent.mnY            = static_cast<long>(aPt.y) - mpFrame->maGeometry.nY;
        aEvent.mnCode         = ImplGetModifierMask( mpFrame->mnLastModifierFlags );
        aEvent.mbDeltaIsPixel = FALSE;

        // --- RTL --- (mirror mouse pos)
        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = mpFrame->maGeometry.nWidth-1-aEvent.mnX;

        if( dX != 0.0 )
        {
            aEvent.mnDelta = static_cast<long>(floor(dX));
            aEvent.mnNotchDelta = (dX < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = TRUE;
            aEvent.mnScrollLines = fabs(dX) / WHEEL_EVENT_FACTOR;
            if( aEvent.mnScrollLines == 0 )
                aEvent.mnScrollLines = 1;

            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }
        if( dY != 0.0 && AquaSalFrame::isAlive( mpFrame ) )
        {
            aEvent.mnDelta = static_cast<long>(floor(dY));
            aEvent.mnNotchDelta = (dY < 0) ? -1 : +1;
            if( aEvent.mnDelta == 0 )
                aEvent.mnDelta = aEvent.mnNotchDelta;
            aEvent.mbHorz = FALSE;
            aEvent.mnScrollLines = fabs(dY) / WHEEL_EVENT_FACTOR;
            if( aEvent.mnScrollLines == 0 )
                aEvent.mnScrollLines = 1;

            mpFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
        }
    }
}


-(void)keyDown: (NSEvent*)pEvent
{
    SolarMutexGuard aGuard;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpLastEvent = pEvent;
        mbInKeyInput = true;
        mbNeedSpecialKeyHandle = false;
        mbKeyHandled = false;

        mpFrame->mnLastEventTime = static_cast<sal_uInt64>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        if( ! [self handleKeyDownException: pEvent] )
        {
            NSArray* pArray = [NSArray arrayWithObject: pEvent];
            [self interpretKeyEvents: pArray];
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
        if( (mpFrame->mnLastModifierFlags & (NSAlternateKeyMask | NSCommandKeyMask))
                    == (NSAlternateKeyMask | NSCommandKeyMask) )
        {
            if( [self sendSingleCharacter: mpLastEvent] )
                return YES;
        }
        unichar keyChar = [pUnmodifiedString characterAtIndex: 0];
        sal_uInt16 nKeyCode = ImplMapCharCode( keyChar );

        // Caution: should the table grow to more than 5 or 6 entries,
        // we must consider moving it to a kind of hash map
        const unsigned int nExceptions = SAL_N_ELEMENTS( aExceptionalKeys );
        for( unsigned int i = 0; i < nExceptions; i++ )
        {
            if( nKeyCode == aExceptionalKeys[i].nKeyCode &&
                (mpFrame->mnLastModifierFlags & aExceptionalKeys[i].nModifierMask)
                == aExceptionalKeys[i].nModifierMask )
            {
                [self sendKeyInputAndReleaseToFrame: nKeyCode character: 0];

                return YES;
            }
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

#if MACOSX_SDK_VERSION < 1060
-(void)insertText:(id)aString
#else
-(void)insertText:(id)aString replacementRange:(NSRange)replacementRange
#endif
{
#if MACOSX_SDK_VERSION >= 1060
    (void) replacementRange; // FIXME: surely it must be used
#endif

    SolarMutexGuard aGuard;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        NSString* pInsert = nil;
        if( [aString isMemberOfClass: [NSAttributedString class]] )
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
                if( mpLastEvent && ( [mpLastEvent type] == NSKeyDown || [mpLastEvent type] == NSKeyUp ) )
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
                if( (nLastModifiers & (NSControlKeyMask | NSAlternateKeyMask | NSCommandKeyMask))
                    == NSAlternateKeyMask )
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
        mbKeyHandled = true;
        [self unmarkText];
    }
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
    [self sendKeyInputAndReleaseToFrame: KEY_LEFT character: 0 modifiers: NSShiftKeyMask];
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
    [self sendKeyInputAndReleaseToFrame: KEY_RIGHT character: 0 modifiers: NSShiftKeyMask];
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

    long nRet = 0;
    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        SalKeyEvent aEvent;
        aEvent.mnTime           = mpFrame->mnLastEventTime;
        aEvent.mnCode           = nKeyCode | ImplGetModifierMask( nMod );
        aEvent.mnCharCode       = aChar;
        aEvent.mnRepeat         = FALSE;
        nRet = mpFrame->CallCallback( SalEvent::KeyInput, &aEvent );
        std::map< NSEvent*, bool >::iterator it = GetSalData()->maKeyEventAnswer.find( mpLastEvent );
        if( it != GetSalData()->maKeyEventAnswer.end() )
            it->second = nRet != 0;
        if( AquaSalFrame::isAlive( mpFrame ) )
            mpFrame->CallCallback( SalEvent::KeyUp, &aEvent );
    }
    return nRet ? YES : NO;
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
            // don't send unicodes in the private use area
            if( keyChar >= 0xf700 && keyChar < 0xf780 )
                keyChar = 0;
            BOOL bRet = [self sendKeyToFrameDirect: nKeyCode character: keyChar modifiers: mpFrame->mnLastModifierFlags];
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
    BOOL bHasMarkedText;

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
        bHasMarkedText = YES;

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
    // and b) why we should have a marked range at this point.
    if( ! mbInKeyInput )
        return NSMakeRange( 0, 0 );

    return [self hasMarkedText] ? mMarkedRange : NSMakeRange( NSNotFound, 0 );
}

- (NSRange)selectedRange
{
    return mSelectedRange;
}

#if MACOSX_SDK_VERSION < 1060
- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange
#else
- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange replacementRange:(NSRange)replacementRange
#endif
{
#if MACOSX_SDK_VERSION >= 1060
    (void) replacementRange; // FIXME - use it!
#endif

    SolarMutexGuard aGuard;

    if( ![aString isKindOfClass:[NSAttributedString class]] )
        aString = [[[NSAttributedString alloc] initWithString:aString] autorelease];
    NSRange rangeToReplace = [self hasMarkedText] ? [self markedRange] : [self selectedRange];
    if( rangeToReplace.location == NSNotFound )
    {
        mMarkedRange = NSMakeRange( selRange.location, [aString length] );
        mSelectedRange = NSMakeRange( selRange.location, selRange.length );
    }
    else
    {
        mMarkedRange = NSMakeRange( rangeToReplace.location, [aString length] );
        mSelectedRange = NSMakeRange( rangeToReplace.location + selRange.location, selRange.length );
    }

    int len = [aString length];
    SalExtTextInputEvent aInputEvent;
    if( len > 0 ) {
        NSString *pString = [aString string];
        OUString aInsertString( GetOUString( pString ) );
        std::vector<ExtTextInputAttr> aInputFlags = std::vector<ExtTextInputAttr>( std::max( 1, len ), ExtTextInputAttr::NONE );
        for ( int i = 0; i < len; i++ )
        {
            unsigned int nUnderlineValue;
            NSRange effectiveRange;

            effectiveRange = NSMakeRange(i, 1);
            nUnderlineValue = [[aString attribute:NSUnderlineStyleAttributeName atIndex:i effectiveRange:&effectiveRange] unsignedIntValue];

            switch (nUnderlineValue & 0xff) {
            case NSUnderlineStyleSingle:
                aInputFlags[i] = ExtTextInputAttr::Underline;
                break;
            case NSUnderlineStyleThick:
                aInputFlags[i] = ExtTextInputAttr::Underline | ExtTextInputAttr::Highlight;
                break;
            case NSUnderlineStyleDouble:
                aInputFlags[i] = ExtTextInputAttr::BoldUnderline;
                break;
            default:
                aInputFlags[i] = ExtTextInputAttr::Highlight;
                break;
            }
        }

        aInputEvent.maText = aInsertString;
        aInputEvent.mnCursorPos = selRange.location;
        aInputEvent.mpTextAttr = &aInputFlags[0];
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
    mSelectedRange = mMarkedRange = NSMakeRange(NSNotFound, 0);
}

#if MACOSX_SDK_VERSION < 1060
- (NSAttributedString *)attributedSubstringFromRange:(NSRange)aRange
#else
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
#endif
{
    (void) aRange;
#if MACOSX_SDK_VERSION >= 1060
    (void) actualRange;
#endif

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
    mpLastEvent = nil;
}

#if MACOSX_SDK_VERSION < 1060
- (NSRect)firstRectForCharacterRange:(NSRange)aRange
#else
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
#endif
{
     // FIXME - These should probably be used?
    (void) aRange;
#if MACOSX_SDK_VERSION >= 1060
    (void) actualRange;
#endif

    SolarMutexGuard aGuard;

    SalExtTextInputPosEvent aPosEvent;
    mpFrame->CallCallback( SalEvent::ExtTextInputPos, static_cast<void *>(&aPosEvent) );

    NSRect rect;

    rect.origin.x = aPosEvent.mnX + mpFrame->maGeometry.nX;
    rect.origin.y =   aPosEvent.mnY + mpFrame->maGeometry.nY + 4; // add some space for underlines
    rect.size.width = aPosEvent.mnWidth;
    rect.size.height = aPosEvent.mnHeight;

    mpFrame->VCLToCocoa( rect );
    return rect;
}

-(id)parentAttribute {
    return reinterpret_cast<NSView*>(mpFrame->getNSWindow());
        //TODO: odd cast really needed for fdo#74121?
}

-(css::accessibility::XAccessibleContext *)accessibleContext
{
    if ( !mpReferenceWrapper ) {
        // some frames never become visible ..
        vcl::Window *pWindow = mpFrame -> GetWindow();
        if ( ! pWindow )
            return reinterpret_cast<::com::sun::star::accessibility::XAccessibleContext*>(nil);

        mpReferenceWrapper = new ReferenceWrapper;
        mpReferenceWrapper -> rAccessibleContext =  pWindow -> /*GetAccessibleChildWindow( 0 ) ->*/ GetAccessible() -> getAccessibleContext();
        [ AquaA11yFactory insertIntoWrapperRepository: self forAccessibleContext: mpReferenceWrapper -> rAccessibleContext ];
    }
    return [ super accessibleContext ];
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

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
