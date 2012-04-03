/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*n***********************************************************************
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


#include <sal/alloca.h>
#include <sal/macros.h>

#include "vcl/window.hxx"
#include "vcl/svapp.hxx"

#include "ios/salinst.h"
#include "ios/salgdi.h"
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
		IosSalGraphics* pGraphics = (mpFrame && IosSalFrame::isAlive(mpFrame)) ? mpFrame->mpGraphics : NULL;
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
