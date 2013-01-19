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


#include <sal/alloca.h>
#include <sal/macros.h>

#include "vcl/window.hxx"
#include "vcl/svapp.hxx"

#include "ios/salinst.h"
#include "coretext/salgdi.h"
#include "ios/salframe.h"
#include "ios/salframeview.h"

#define WHEEL_EVENT_FACTOR 1.5

@implementation SalFrameWindow
-(id)initWithSalFrame: (IosSalFrame*)pFrame
{
    mpFrame = pFrame;
#if 0
    CGRect aRect = { { pFrame->maGeometry.nX, pFrame->maGeometry.nY },
                     { pFrame->maGeometry.nWidth, pFrame->maGeometry.nHeight } };
    NSWindow* pNSWindow = [super initWithContentRect: aRect styleMask: mpFrame->getStyleMask() backing: NSBackingStoreBuffered defer: NO ];
    [pNSWindow useOptimizedDrawing: YES]; // OSX recommendation when there are no overlapping subviews within the receiver
    return pNSWindow;
#endif
    return nil;
}

-(IosSalFrame*)getSalFrame
{
    return mpFrame;
}

-(void)displayIfNeeded
{
    if( GetSalData() && GetSalData()->mpFirstInstance )
    {
        osl::SolarMutex* pMutex = GetSalData()->mpFirstInstance->GetYieldMutex();
        if( pMutex )
        {
            pMutex->acquire();
            // ??? [super displayIfNeeded];
            pMutex->release();
        }
    }
}

-(BOOL)canBecomeKeyWindow
{
    if( (mpFrame->mnStyle & 
            ( SAL_FRAME_STYLE_FLOAT                 |
              SAL_FRAME_STYLE_TOOLTIP               |
              SAL_FRAME_STYLE_INTRO
            )) == 0 )
        return YES;
    if( (mpFrame->mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) != 0 )
        return YES;
    if( (mpFrame->mnStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE) )
        return YES;
    // ??? return [super canBecomeKeyWindow];
    return NO;
}

-(void)windowDidBecomeKey: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
#if 0
        static const sal_uLong nGuessDocument = SAL_FRAME_STYLE_MOVEABLE|
                                            SAL_FRAME_STYLE_SIZEABLE|
                                            SAL_FRAME_STYLE_CLOSEABLE;
#endif
        mpFrame->CallCallback( SALEVENT_GETFOCUS, 0 );
        mpFrame->SendPaintEvent(); // repaint controls as active
    }
}

-(void)windowDidResignKey: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->CallCallback(SALEVENT_LOSEFOCUS, 0);
        mpFrame->SendPaintEvent(); // repaint controls as inactive
    }
}

-(void)windowDidChangeScreen: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
        mpFrame->screenParametersChanged();
}

-(void)windowDidMove: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_MOVE, 0 );
    }
}

-(void)windowDidResize: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_RESIZE, 0 );
        mpFrame->SendPaintEvent();
    }
}

-(void)windowDidMiniaturize: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mbShown = false;
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_RESIZE, 0 );
    }
}

-(void)windowDidDeminiaturize: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mbShown = true;
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_RESIZE, 0 );
    }
}

-(BOOL)windowShouldClose: (NSNotification*)pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    BOOL bRet = YES;
    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        // #i84461# end possible input
        mpFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, 0 );
        if( IosSalFrame::isAlive( mpFrame ) )
        {
            mpFrame->CallCallback( SALEVENT_CLOSE, 0 );
            bRet = NO; // application will close the window or not, AppKit shouldn't
        }
    }

    return bRet;
}

@end

@implementation SalFrameView

-(id)initWithSalFrame: (IosSalFrame*)pFrame
{
    // ???

    (void) pFrame;
    mfLastMagnifyTime = 0.0;
    return self;
}

-(IosSalFrame*)getSalFrame
{
    return mpFrame;
}

-(BOOL)acceptsFirstResponder
{
    return YES;
}

-(BOOL)isOpaque
{
    return mpFrame ? (mpFrame->getClipPath() != 0 ? NO : YES) : YES;
}

// helper class similar to a osl::SolarGuard for the SalYieldMutex
// the difference is that it only does tryToAcquire instead of aquire
// so dreaded deadlocks like #i93512# are prevented
class TryGuard
{
public:
			TryGuard()  { mbGuarded = ImplSalYieldMutexTryToAcquire(); }
			~TryGuard() { if( mbGuarded ) ImplSalYieldMutexRelease(); }
	bool	IsGuarded() { return mbGuarded; }
private:
	bool	mbGuarded;
};

-(void)drawRect: (CGRect)aRect
{
	// HOTFIX: #i93512# prevent deadlocks if any other thread already has the SalYieldMutex
	TryGuard aTryGuard;
	if( !aTryGuard.IsGuarded() )
	{
		// NOTE: the mpFrame access below is not guarded yet!
		// TODO: mpFrame et al need to be guarded by an independent mutex
		QuartzSalGraphics* pGraphics = (mpFrame && IosSalFrame::isAlive(mpFrame)) ? mpFrame->mpGraphics : NULL;
		if( pGraphics )
		{
			pGraphics->RefreshRect( aRect );
		}
		return;
	}

    if( mpFrame && IosSalFrame::isAlive( mpFrame ) )
    {
        if( mpFrame->mpGraphics )
        {
            mpFrame->mpGraphics->UpdateWindow( aRect );
            if( mpFrame->getClipPath() ) {
                // ??? [mpFrame->getWindow() invalidateShadow];
            }
        }
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
