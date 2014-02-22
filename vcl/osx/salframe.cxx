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


#include <string>

#include "rtl/ustrbuf.hxx"

#include "osl/file.h"

#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/syswin.hxx"
#include <vcl/settings.hxx>

#include "osx/saldata.hxx"
#include "quartz/salgdi.h"
#include "osx/salframe.h"
#include "osx/salmenu.h"
#include "osx/salinst.h"
#include "osx/salframeview.h"
#include "osx/a11yfactory.h"
#include "quartz/utils.h"

#include "salwtype.hxx"

#include "premac.h"
#include <objc/objc-runtime.h>


#include <Carbon/Carbon.h>
#include "postmac.h"

using namespace std;



AquaSalFrame* AquaSalFrame::s_pCaptureFrame = NULL;



AquaSalFrame::AquaSalFrame( SalFrame* pParent, sal_uLong salFrameStyle ) :
    mpNSWindow(nil),
    mpNSView(nil),
    mpDockMenuEntry(nil),
    mpGraphics(NULL),
    mpParent(NULL),
    mnMinWidth(0),
    mnMinHeight(0),
    mnMaxWidth(0),
    mnMaxHeight(0),
    mbGraphics(false),
    mbFullScreen( false ),
    mbShown(false),
    mbInitShow(true),
    mbPositioned(false),
    mbSized(false),
    mbPresentation( false ),
    mnStyle( salFrameStyle ),
    mnStyleMask( 0 ),
    mnLastEventTime( 0 ),
    mnLastModifierFlags( 0 ),
    mpMenu( NULL ),
    mnExtStyle( 0 ),
    mePointerStyle( POINTER_ARROW ),
    mnTrackingRectTag( 0 ),
    mrClippingPath( 0 ),
    mnICOptions( 0 )
{
    maSysData.nSize     = sizeof( SystemEnvData );

    mpParent = dynamic_cast<AquaSalFrame*>(pParent);

    initWindowAndView();

    SalData* pSalData = GetSalData();
    pSalData->maFrames.push_front( this );
    pSalData->maFrameCheck.insert( this );
}



AquaSalFrame::~AquaSalFrame()
{
    
    
    if( mpMenu && mpMenu->mbMenuBar && AquaSalMenu::pCurrentMenuBar == mpMenu )
        AquaSalMenu::setDefaultMenu();

    
    ResetClipRegion();

    [SalFrameView unsetMouseFrame: this];

    SalData* pSalData = GetSalData();
    pSalData->maFrames.remove( this );
    pSalData->maFrameCheck.erase( this );
    pSalData->maPresentationFrames.remove( this );

    DBG_ASSERT( this != s_pCaptureFrame, "capture frame destroyed" );
    if( this == s_pCaptureFrame )
        s_pCaptureFrame = NULL;

    delete mpGraphics;

    if( mpDockMenuEntry )
        
        [AquaSalInstance::GetDynamicDockMenu() removeItem: mpDockMenuEntry];
    if ( mpNSView ) {
        [AquaA11yFactory revokeView: mpNSView];
        [mpNSView release];
    }
    if ( mpNSWindow )
        [mpNSWindow release];
}



void AquaSalFrame::initWindowAndView()
{
    
    
    NSScreen* pNSScreen = [mpNSWindow screen];
    if( pNSScreen == nil )
        pNSScreen = [NSScreen mainScreen];
    maScreenRect = [pNSScreen frame];

    
    NSRect aVisibleRect = [pNSScreen visibleFrame];
    CocoaToVCL( aVisibleRect );

    maGeometry.nX = static_cast<int>(aVisibleRect.origin.x + aVisibleRect.size.width / 10);
    maGeometry.nY = static_cast<int>(aVisibleRect.origin.y + aVisibleRect.size.height / 10);
    maGeometry.nWidth = static_cast<unsigned int>(aVisibleRect.size.width * 0.8);
    maGeometry.nHeight = static_cast<unsigned int>(aVisibleRect.size.height * 0.8);

    
    if( (mnStyle & SAL_FRAME_STYLE_FLOAT) ||
        (mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        mnStyleMask = NSBorderlessWindowMask;
    else if( mnStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        mnStyleMask = NSTitledWindowMask            |
                      NSMiniaturizableWindowMask    |
                      NSResizableWindowMask         |
                      NSClosableWindowMask;
        
        maGeometry.nX = static_cast<int>(aVisibleRect.origin.x);
        maGeometry.nY = static_cast<int>(aVisibleRect.origin.y);
        maGeometry.nWidth = static_cast<int>(aVisibleRect.size.width);
        maGeometry.nHeight = static_cast<int>(aVisibleRect.size.height);
        mbPositioned = mbSized = true;
    }
    else
    {
        if( (mnStyle & SAL_FRAME_STYLE_MOVEABLE) )
        {
            mnStyleMask |= NSTitledWindowMask;
            if( mpParent == NULL )
                mnStyleMask |= NSMiniaturizableWindowMask;
        }
        if( (mnStyle & SAL_FRAME_STYLE_SIZEABLE) )
            mnStyleMask |= NSResizableWindowMask;
        if( (mnStyle & SAL_FRAME_STYLE_CLOSEABLE) )
            mnStyleMask |= NSClosableWindowMask;
        
        
        if( mnStyleMask != 0 )
            mnStyleMask |= NSTitledWindowMask;
    }

    
    @try
    {
        mpNSWindow = [[SalFrameWindow alloc] initWithSalFrame: this];
        mpNSView = [[SalFrameView alloc] initWithSalFrame: this];
    }
    @catch ( id exception )
    {
        return;
    }

    if( (mnStyle & SAL_FRAME_STYLE_TOOLTIP) )
        [mpNSWindow setIgnoresMouseEvents: YES];
    else
        [mpNSWindow setAcceptsMouseMovedEvents: YES];
    [mpNSWindow setHasShadow: YES];
    [mpNSWindow setDelegate: static_cast<id<NSWindowDelegate> >(mpNSWindow)];

    if( [mpNSWindow respondsToSelector: @selector(setRestorable:)])
    {
        objc_msgSend(mpNSWindow, @selector(setRestorable:), NO);
    }
    NSRect aRect = { { 0,0 }, { static_cast<CGFloat>(maGeometry.nWidth), static_cast<CGFloat>(maGeometry.nHeight) } };
    mnTrackingRectTag = [mpNSView addTrackingRect: aRect owner: mpNSView userData: nil assumeInside: NO];

    maSysData.mpNSView = mpNSView;

    UpdateFrameGeometry();

    [mpNSWindow setContentView: mpNSView];
}



void AquaSalFrame::CocoaToVCL( NSRect& io_rRect, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rRect.origin.y = maScreenRect.size.height - (io_rRect.origin.y+io_rRect.size.height);
    else
        io_rRect.origin.y = maGeometry.nHeight - (io_rRect.origin.y+io_rRect.size.height);
}

void AquaSalFrame::VCLToCocoa( NSRect& io_rRect, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rRect.origin.y = maScreenRect.size.height - (io_rRect.origin.y+io_rRect.size.height);
    else
        io_rRect.origin.y = maGeometry.nHeight - (io_rRect.origin.y+io_rRect.size.height);
}

void AquaSalFrame::CocoaToVCL( NSPoint& io_rPoint, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rPoint.y = maScreenRect.size.height - io_rPoint.y;
    else
        io_rPoint.y = maGeometry.nHeight - io_rPoint.y;
}

void AquaSalFrame::VCLToCocoa( NSPoint& io_rPoint, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rPoint.y = maScreenRect.size.height - io_rPoint.y;
    else
        io_rPoint.y = maGeometry.nHeight - io_rPoint.y;
}



void AquaSalFrame::screenParametersChanged()
{
    UpdateFrameGeometry();

    if( mpGraphics )
        mpGraphics->updateResolution();
    CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
}



SalGraphics* AquaSalFrame::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( !mpGraphics )
    {
        mpGraphics = new AquaSalGraphics;
        mpGraphics->SetWindowGraphics( this );
    }

    mbGraphics = TRUE;
    return mpGraphics;
}



void AquaSalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    (void)pGraphics;
    DBG_ASSERT( pGraphics == mpGraphics, "graphics released on wrong frame" );
    mbGraphics = FALSE;
}



bool AquaSalFrame::PostEvent( void *pData )
{
    GetSalData()->mpFirstInstance->PostUserEvent( this, SALEVENT_USEREVENT, pData );
    return TRUE;
}


void AquaSalFrame::SetTitle(const OUString& rTitle)
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    NSString* pTitle = CreateNSString( rTitle );
    [mpNSWindow setTitle: pTitle];

    
    const sal_uLong nAppWindowStyle = (SAL_FRAME_STYLE_CLOSEABLE | SAL_FRAME_STYLE_MOVEABLE);
    if( mpParent == NULL &&
        (mnStyle & nAppWindowStyle) == nAppWindowStyle )
    {
        if( mpDockMenuEntry == NULL )
        {
            NSMenu* pDock = AquaSalInstance::GetDynamicDockMenu();
            mpDockMenuEntry = [pDock insertItemWithTitle: pTitle
                                     action: @selector(dockMenuItemTriggered:)
                                     keyEquivalent: @""
                                     atIndex: 0];
            [mpDockMenuEntry setTarget: mpNSWindow];

            
            
        }
        else
            [mpDockMenuEntry setTitle: pTitle];
    }

    if (pTitle)
        [pTitle release];
}



void AquaSalFrame::SetIcon( sal_uInt16 )
{
}



void AquaSalFrame::SetRepresentedURL( const OUString& i_rDocURL )
{
    
    SalData::ensureThreadAutoreleasePool();

    if( i_rDocURL.startsWith( "file:" ) )
    {
        OUString aSysPath;
        osl_getSystemPathFromFileURL( i_rDocURL.pData, &aSysPath.pData );
        NSString* pStr = CreateNSString( aSysPath );
        if( pStr )
        {
            [pStr autorelease];
            [mpNSWindow setRepresentedFilename: pStr];
        }
    }
}



void AquaSalFrame::initShow()
{
    mbInitShow = false;
    if( ! mbPositioned && ! mbFullScreen )
    {
        Rectangle aScreenRect;
        GetWorkArea( aScreenRect );
        if( mpParent ) 
        {
            
            long nNewX = mpParent->maGeometry.nX + ((long)mpParent->maGeometry.nWidth - (long)maGeometry.nWidth)/2;
            if( nNewX < aScreenRect.Left() )
                nNewX = aScreenRect.Left();
            if( long(nNewX + maGeometry.nWidth) > aScreenRect.Right() )
                nNewX = aScreenRect.Right() - maGeometry.nWidth-1;
            long nNewY = mpParent->maGeometry.nY + ((long)mpParent->maGeometry.nHeight - (long)maGeometry.nHeight)/2;
            if( nNewY < aScreenRect.Top() )
                nNewY = aScreenRect.Top();
            if( nNewY > aScreenRect.Bottom() )
                nNewY = aScreenRect.Bottom() - maGeometry.nHeight-1;
            SetPosSize( nNewX - mpParent->maGeometry.nX,
                        nNewY - mpParent->maGeometry.nY,
                        0, 0,  SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
        }
        else if( ! (mnStyle & SAL_FRAME_STYLE_SIZEABLE) )
        {
            
            long nNewX = (aScreenRect.GetWidth() - maGeometry.nWidth)/2;
            long nNewY = (aScreenRect.GetHeight() - maGeometry.nHeight)/2;
            SetPosSize( nNewX, nNewY, 0, 0,  SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
        }
    }

    
    [AquaA11yFactory registerView: mpNSView];
}

void AquaSalFrame::SendPaintEvent( const Rectangle* pRect )
{
    SalPaintEvent aPaintEvt( 0, 0, maGeometry.nWidth, maGeometry.nHeight, true );
    if( pRect )
    {
        aPaintEvt.mnBoundX      = pRect->Left();
        aPaintEvt.mnBoundY      = pRect->Top();
        aPaintEvt.mnBoundWidth  = pRect->GetWidth();
        aPaintEvt.mnBoundHeight = pRect->GetHeight();
    }

    CallCallback(SALEVENT_PAINT, &aPaintEvt);
}



void AquaSalFrame::Show(sal_Bool bVisible, sal_Bool bNoActivate)
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    mbShown = bVisible;
    if(bVisible)
    {
        if( mbInitShow )
            initShow();

        CallCallback(SALEVENT_RESIZE, 0);
        
        SendPaintEvent();

        if( bNoActivate || [mpNSWindow canBecomeKeyWindow] == NO )
            [mpNSWindow orderFront: NSApp];
        else
            [mpNSWindow makeKeyAndOrderFront: NSApp];

        if( mpParent )
        {
            /* #i92674# #i96433# we do not want an invisible parent to show up (which adding a visible
               child implicitly does). However we also do not want a parentless toolbar.

               HACK: try to decide when we should not insert a child to its parent
               floaters and ownerdraw windows have not yet shown up in cases where
               we don't want the parent to become visible
            */
            if( mpParent->mbShown || (mnStyle & (SAL_FRAME_STYLE_OWNERDRAWDECORATION | SAL_FRAME_STYLE_FLOAT) ) )
            {
                [mpParent->mpNSWindow addChildWindow: mpNSWindow ordered: NSWindowAbove];
            }
        }

        if( mbPresentation )
            [mpNSWindow makeMainWindow];
    }
    else
    {
        
        
        if( mpMenu && mpMenu->mbMenuBar && AquaSalMenu::pCurrentMenuBar == mpMenu )
            AquaSalMenu::setDefaultMenu();

        
        
        if( mpParent && mpParent->mbShown && [mpNSWindow isKeyWindow] )
            [mpParent->mpNSWindow makeKeyAndOrderFront: NSApp];

        [SalFrameView unsetMouseFrame: this];
        if( mpParent && [mpNSWindow parentWindow] == mpParent->mpNSWindow )
            [mpParent->mpNSWindow removeChildWindow: mpNSWindow];

        [mpNSWindow orderOut: NSApp];
    }
}



void AquaSalFrame::Enable( sal_Bool )
{
}



void AquaSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    
    SalData::ensureThreadAutoreleasePool();

    mnMinWidth = nWidth;
    mnMinHeight = nHeight;

    if( mpNSWindow )
    {
        
        
        nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

        NSSize aSize = { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) };

        
        
        [mpNSWindow setMinSize: aSize];
    }
}



void AquaSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    
    SalData::ensureThreadAutoreleasePool();

    mnMaxWidth = nWidth;
    mnMaxHeight = nHeight;

    if( mpNSWindow )
    {
        
        
        nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

        
        if (nWidth>32767) nWidth=32767;
        if (nHeight>32767) nHeight=32767;

        NSSize aSize = { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) };

        
        
        [mpNSWindow setMaxSize: aSize];
    }
}



void AquaSalFrame::SetClientSize( long nWidth, long nHeight )
{
    
    SalData::ensureThreadAutoreleasePool();

    if( mpNSWindow )
    {
        NSSize aSize = { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) };

        [mpNSWindow setContentSize: aSize];
        UpdateFrameGeometry();
        if( mbShown )
            
            SendPaintEvent();
    }
}



void AquaSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    if( mbShown || mbInitShow )
    {
        rWidth  = maGeometry.nWidth;
        rHeight = maGeometry.nHeight;
    }
    else
    {
        rWidth  = 0;
        rHeight = 0;
    }
}



void AquaSalFrame::SetWindowState( const SalFrameState* pState )
{
    
    SalData::ensureThreadAutoreleasePool();

    if ( mpNSWindow )
    {
    
    NSRect aStateRect = [mpNSWindow frame];
    aStateRect = [NSWindow contentRectForFrameRect: aStateRect styleMask: mnStyleMask];
    CocoaToVCL( aStateRect );
    if( pState->mnMask & WINDOWSTATE_MASK_X )
        aStateRect.origin.x = float(pState->mnX);
    if( pState->mnMask & WINDOWSTATE_MASK_Y )
        aStateRect.origin.y = float(pState->mnY);
    if( pState->mnMask & WINDOWSTATE_MASK_WIDTH )
        aStateRect.size.width = float(pState->mnWidth);
    if( pState->mnMask & WINDOWSTATE_MASK_HEIGHT )
        aStateRect.size.height = float(pState->mnHeight);
    VCLToCocoa( aStateRect );
    aStateRect = [NSWindow frameRectForContentRect: aStateRect styleMask: mnStyleMask];

    [mpNSWindow setFrame: aStateRect display: NO];
    if( pState->mnState == WINDOWSTATE_STATE_MINIMIZED )
        [mpNSWindow miniaturize: NSApp];
    else if( [mpNSWindow isMiniaturized] )
        [mpNSWindow deminiaturize: NSApp];


    /* ZOOMED is not really maximized (actually it toggles between a user set size and
       the program specified one), but comes closest since the default behavior is
       "maximized" if the user did not intervene
    */
    if( pState->mnState == WINDOWSTATE_STATE_MAXIMIZED )
    {
        if(! [mpNSWindow isZoomed])
            [mpNSWindow zoom: NSApp];
    }
    else
    {
        if( [mpNSWindow isZoomed] )
            [mpNSWindow zoom: NSApp];
    }
    }

    
    UpdateFrameGeometry();

    sal_uInt16 nEvent = 0;
    if( pState->mnMask & (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y) )
    {
        mbPositioned = true;
        nEvent = SALEVENT_MOVE;
    }

    if( pState->mnMask & (WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT) )
    {
        mbSized = true;
        nEvent = (nEvent == SALEVENT_MOVE) ? SALEVENT_MOVERESIZE : SALEVENT_RESIZE;
    }
    
    if( nEvent )
        CallCallback( nEvent, NULL );

    if( mbShown && mpNSWindow )
    {
        
        SendPaintEvent();

        
        [mpNSWindow display];
    }
}



bool AquaSalFrame::GetWindowState( SalFrameState* pState )
{
    if ( !mpNSWindow )
        return FALSE;

    
    SalData::ensureThreadAutoreleasePool();

    pState->mnMask = WINDOWSTATE_MASK_X                 |
                     WINDOWSTATE_MASK_Y                 |
                     WINDOWSTATE_MASK_WIDTH             |
                     WINDOWSTATE_MASK_HEIGHT            |
                     WINDOWSTATE_MASK_STATE;

    NSRect aStateRect = [mpNSWindow frame];
    aStateRect = [NSWindow contentRectForFrameRect: aStateRect styleMask: mnStyleMask];
    CocoaToVCL( aStateRect );
    pState->mnX         = long(aStateRect.origin.x);
    pState->mnY         = long(aStateRect.origin.y);
    pState->mnWidth     = long(aStateRect.size.width);
    pState->mnHeight    = long(aStateRect.size.height);

    if( [mpNSWindow isMiniaturized] )
        pState->mnState = WINDOWSTATE_STATE_MINIMIZED;
    else if( ! [mpNSWindow isZoomed] )
        pState->mnState = WINDOWSTATE_STATE_NORMAL;
    else
        pState->mnState = WINDOWSTATE_STATE_MAXIMIZED;

    return TRUE;
}



void AquaSalFrame::SetScreenNumber(unsigned int nScreen)
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    NSArray* pScreens = [NSScreen screens];
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
    {
        
        pScreen = [pScreens objectAtIndex: nScreen];
        NSRect aNewScreen = [pScreen frame];

        
        pScreen = [mpNSWindow screen];
        if( pScreen )
        {
            NSRect aCurScreen = [pScreen frame];
            if( aCurScreen.origin.x != aNewScreen.origin.x ||
                aCurScreen.origin.y != aNewScreen.origin.y )
            {
                NSRect aFrameRect = [mpNSWindow frame];
                aFrameRect.origin.x += aNewScreen.origin.x - aCurScreen.origin.x;
                aFrameRect.origin.y += aNewScreen.origin.y - aCurScreen.origin.y;
                [mpNSWindow setFrame: aFrameRect display: NO];
                UpdateFrameGeometry();
            }
        }
    }
}

void AquaSalFrame::SetApplicationID( const OUString &/*rApplicationID*/ )
{
}



void AquaSalFrame::ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay )
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    SAL_INFO("vcl.osx", OSL_THIS_FUNC << ": mbFullScreen=" << mbFullScreen << ", bFullScreen=" << bFullScreen);

    if( mbFullScreen == bFullScreen )
        return;

    mbFullScreen = bFullScreen;

    if( bFullScreen )
    {
        
        
        bool bHideMenu = (nDisplay == 0);

        NSRect aNewContentRect = { { 0, 0 }, { 0, 0 } };
        
        NSScreen* pScreen = nil;
        NSArray* pScreens = [NSScreen screens];
        if( pScreens )
        {
            if( nDisplay >= 0 && (unsigned int)nDisplay < [pScreens count] )
                pScreen = [pScreens objectAtIndex: nDisplay];
            else
            {
                
                bHideMenu = true;
                NSEnumerator* pEnum = [pScreens objectEnumerator];
                while( (pScreen = [pEnum nextObject]) != nil )
                {
                    NSRect aScreenRect = [pScreen frame];
                    if( aScreenRect.origin.x < aNewContentRect.origin.x )
                    {
                        aNewContentRect.size.width += aNewContentRect.origin.x - aScreenRect.origin.x;
                        aNewContentRect.origin.x = aScreenRect.origin.x;
                    }
                    if( aScreenRect.origin.y < aNewContentRect.origin.y )
                    {
                        aNewContentRect.size.height += aNewContentRect.origin.y - aScreenRect.origin.y;
                        aNewContentRect.origin.y = aScreenRect.origin.y;
                    }
                    if( aScreenRect.origin.x + aScreenRect.size.width > aNewContentRect.origin.x + aNewContentRect.size.width )
                        aNewContentRect.size.width = aScreenRect.origin.x + aScreenRect.size.width - aNewContentRect.origin.x;
                    if( aScreenRect.origin.y + aScreenRect.size.height > aNewContentRect.origin.y + aNewContentRect.size.height )
                        aNewContentRect.size.height = aScreenRect.origin.y + aScreenRect.size.height - aNewContentRect.origin.y;
                }
            }
        }
        if( aNewContentRect.size.width == 0 && aNewContentRect.size.height == 0 )
        {
            if( pScreen == nil )
                pScreen = [mpNSWindow screen];
            if( pScreen == nil )
                pScreen = [NSScreen mainScreen];

            aNewContentRect = [pScreen frame];
        }

        if( bHideMenu )
            [NSMenu setMenuBarVisible:NO];

        maFullScreenRect = [mpNSWindow frame];
        {
            [mpNSWindow setFrame: [NSWindow frameRectForContentRect: aNewContentRect styleMask: mnStyleMask] display: mbShown ? YES : NO];
        }

        UpdateFrameGeometry();

        if( mbShown )
            CallCallback( SALEVENT_MOVERESIZE, NULL );
    }
    else
    {
        {
            [mpNSWindow setFrame: maFullScreenRect display: mbShown ? YES : NO];
        }
        UpdateFrameGeometry();

        if( mbShown )
            CallCallback( SALEVENT_MOVERESIZE, NULL );

        
        [NSMenu setMenuBarVisible:YES];
    }
    if( mbShown )
        
        SendPaintEvent();
}



void AquaSalFrame::StartPresentation( sal_Bool bStart )
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    if( bStart )
    {
        GetSalData()->maPresentationFrames.push_back( this );
        IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                    kIOPMAssertionLevelOn,
                                    CFSTR("LibreOffice presentation running"),
                                    &mnAssertionID);
        [mpNSWindow setLevel: NSPopUpMenuWindowLevel];
        if( mbShown )
            [mpNSWindow makeMainWindow];
    }
    else
    {
        GetSalData()->maPresentationFrames.remove( this );
        IOPMAssertionRelease(mnAssertionID);
        [mpNSWindow setLevel: NSNormalWindowLevel];
    }
}



void AquaSalFrame::SetAlwaysOnTop( sal_Bool )
{
}



void AquaSalFrame::ToTop(sal_uInt16 nFlags)
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    if( ! (nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN) )
    {
        if( ! [mpNSWindow isVisible] || [mpNSWindow isMiniaturized] )
            return;
    }
    if( nFlags & SAL_FRAME_TOTOP_GRABFOCUS )
        [mpNSWindow makeKeyAndOrderFront: NSApp];
    else
        [mpNSWindow orderFront: NSApp];
}



NSCursor* AquaSalFrame::getCurrentCursor() const
{
    NSCursor* pCursor = nil;
    switch( mePointerStyle )
    {
    case POINTER_TEXT:      pCursor = [NSCursor IBeamCursor];           break;
    case POINTER_CROSS:     pCursor = [NSCursor crosshairCursor];       break;
    case POINTER_HAND:
    case POINTER_MOVE:      pCursor = [NSCursor openHandCursor];        break;
    case POINTER_NSIZE:     pCursor = [NSCursor resizeUpCursor];        break;
    case POINTER_SSIZE:     pCursor = [NSCursor resizeDownCursor];      break;
    case POINTER_ESIZE:     pCursor = [NSCursor resizeRightCursor];      break;
    case POINTER_WSIZE:     pCursor = [NSCursor resizeLeftCursor];     break;
    case POINTER_ARROW:     pCursor = [NSCursor arrowCursor];           break;
    case POINTER_VSPLIT:
    case POINTER_VSIZEBAR:
    case POINTER_WINDOW_NSIZE:
    case POINTER_WINDOW_SSIZE:
                            pCursor = [NSCursor resizeUpDownCursor];    break;
    case POINTER_HSPLIT:
    case POINTER_HSIZEBAR:
    case POINTER_WINDOW_ESIZE:
    case POINTER_WINDOW_WSIZE:
                            pCursor = [NSCursor resizeLeftRightCursor]; break;
    case POINTER_REFHAND:   pCursor = [NSCursor pointingHandCursor];    break;
    case POINTER_NULL:      [NSCursor hide];                            break;

    default:
        pCursor = GetSalData()->getCursor( mePointerStyle );
        if( pCursor == nil )
        {
            OSL_FAIL( "unmapped cursor" );
            pCursor = [NSCursor arrowCursor];
        }
        break;
    }
    return pCursor;
}

void AquaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    if( ePointerStyle >= POINTER_COUNT || ePointerStyle == mePointerStyle )
        return;
    mePointerStyle = ePointerStyle;

    [mpNSWindow invalidateCursorRectsForView: mpNSView];
}



void AquaSalFrame::SetPointerPos( long nX, long nY )
{
    

    
    CGPoint aPoint = { static_cast<CGFloat>(nX + maGeometry.nX), static_cast<CGFloat>(nY + maGeometry.nY) };
    CGDirectDisplayID mainDisplayID = CGMainDisplayID();
    CGDisplayMoveCursorToPoint( mainDisplayID, aPoint );
}



void AquaSalFrame::Flush( void )
{
    if( !(mbGraphics && mpGraphics && mpNSView && mbShown) )
        return;

    
    SalData::ensureThreadAutoreleasePool();


    [mpNSView setNeedsDisplay: YES];

    
    
    
    if( ImplGetSVData()->maAppData.mnDispatchLevel <= 0 )
    {
        [mpNSView display];
    }
}



void AquaSalFrame::Flush( const Rectangle& rRect )
{
    if( !(mbGraphics && mpGraphics && mpNSView && mbShown) )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    NSRect aNSRect = { { static_cast<CGFloat>(rRect.Left()), static_cast<CGFloat>(rRect.Top()) }, { static_cast<CGFloat>(rRect.GetWidth()), static_cast<CGFloat>(rRect.GetHeight()) } };
    VCLToCocoa( aNSRect, false );
    [mpNSView setNeedsDisplayInRect: aNSRect];

    
    
    
    if( ImplGetSVData()->maAppData.mnDispatchLevel <= 0 )
    {
        [mpNSView display];
    }
}



void AquaSalFrame::Sync()
{
    if( mbGraphics && mpGraphics && mpNSView && mbShown )
    {
        
        SalData::ensureThreadAutoreleasePool();

        [mpNSView setNeedsDisplay: YES];
        [mpNSView display];
    }
}



void AquaSalFrame::SetInputContext( SalInputContext* pContext )
{
    if (!pContext)
    {
        mnICOptions = 0;
        return;
    }

    mnICOptions = pContext->mnOptions;

    if(!(pContext->mnOptions & SAL_INPUTCONTEXT_TEXT))
        return;
}



void AquaSalFrame::EndExtTextInput( sal_uInt16 )
{
}



OUString AquaSalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
    static std::map< sal_uInt16, OUString > aKeyMap;
    if( aKeyMap.empty() )
    {
        sal_uInt16 i;
        for( i = KEY_A; i <= KEY_Z; i++ )
            aKeyMap[ i ] = OUString( sal_Unicode( 'A' + (i - KEY_A) ) );
        for( i = KEY_0; i <= KEY_9; i++ )
            aKeyMap[ i ] = OUString( sal_Unicode( '0' + (i - KEY_0) ) );
        for( i = KEY_F1; i <= KEY_F26; i++ )
        {
            OUStringBuffer aKey( 3 );
            aKey.append( 'F' );
            aKey.append( sal_Int32( i - KEY_F1 + 1 ) );
            aKeyMap[ i ] = aKey.makeStringAndClear();
        }

        aKeyMap[ KEY_DOWN ]     = OUString( sal_Unicode( 0x21e3 ) );
        aKeyMap[ KEY_UP ]       = OUString( sal_Unicode( 0x21e1 ) );
        aKeyMap[ KEY_LEFT ]     = OUString( sal_Unicode( 0x21e0 ) );
        aKeyMap[ KEY_RIGHT ]    = OUString( sal_Unicode( 0x21e2 ) );
        aKeyMap[ KEY_HOME ]     = OUString( sal_Unicode( 0x2196 ) );
        aKeyMap[ KEY_END ]      = OUString( sal_Unicode( 0x2198 ) );
        aKeyMap[ KEY_PAGEUP ]   = OUString( sal_Unicode( 0x21de ) );
        aKeyMap[ KEY_PAGEDOWN ] = OUString( sal_Unicode( 0x21df ) );
        aKeyMap[ KEY_RETURN ]   = OUString( sal_Unicode( 0x21a9 ) );
        aKeyMap[ KEY_ESCAPE ]   = OUString( "esc" );
        aKeyMap[ KEY_TAB ]      = OUString( sal_Unicode( 0x21e5 ) );
        aKeyMap[ KEY_BACKSPACE ]= OUString( sal_Unicode( 0x232b ) );
        aKeyMap[ KEY_SPACE ]    = OUString( sal_Unicode( 0x2423 ) );
        aKeyMap[ KEY_DELETE ]   = OUString( sal_Unicode( 0x2326 ) );
        aKeyMap[ KEY_ADD ]      = OUString( '+' );
        aKeyMap[ KEY_SUBTRACT ] = OUString( '-' );
        aKeyMap[ KEY_DIVIDE ]   = OUString( '/' );
        aKeyMap[ KEY_MULTIPLY ] = OUString( '*' );
        aKeyMap[ KEY_POINT ]    = OUString( '.' );
        aKeyMap[ KEY_COMMA ]    = OUString( ',' );
        aKeyMap[ KEY_LESS ]     = OUString( '<' );
        aKeyMap[ KEY_GREATER ]  = OUString( '>' );
        aKeyMap[ KEY_EQUAL ]    = OUString( '=' );
        aKeyMap[ KEY_OPEN ]     = OUString( sal_Unicode( 0x23cf ) );

        /* yet unmapped KEYCODES:
        aKeyMap[ KEY_INSERT ]   = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_CUT ]      = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_COPY ]     = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_PASTE ]    = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_UNDO ]     = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_REPEAT ]   = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_FIND ]     = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_PROPERTIES ]     = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_FRONT ]    = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_CONTEXTMENU ]    = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_MENU ]     = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_HELP ]     = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_HANGUL_HANJA ]   = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_DECIMAL ]  = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_TILDE ]    = OUString( sal_Unicode( ) );
        aKeyMap[ KEY_QUOTELEFT ]= OUString( sal_Unicode( ) );
        */

    }

    OUStringBuffer aResult( 16 );

    sal_uInt16 nUnmodifiedCode = (nKeyCode & KEY_CODE);
    std::map< sal_uInt16, OUString >::const_iterator it = aKeyMap.find( nUnmodifiedCode );
    if( it != aKeyMap.end() )
    {
        if( (nKeyCode & KEY_SHIFT) != 0 )
            aResult.append( sal_Unicode( 0x21e7 ) );
        if( (nKeyCode & KEY_MOD1) != 0 )
            aResult.append( sal_Unicode( 0x2318 ) );
        
        
        if( (nKeyCode & (KEY_MOD2|KEY_MOD3)) != 0 )
            aResult.append( sal_Unicode( 0x2303 ) );

        aResult.append( it->second );
    }

    return aResult.makeStringAndClear();
}



static void getAppleScrollBarVariant(StyleSettings &rSettings)
{
    bool bIsScrollbarDoubleMax = true; 

    CFStringRef AppleScrollBarType = CFSTR("AppleScrollBarVariant");
    if( AppleScrollBarType )
    {
        CFStringRef ScrollBarVariant = ((CFStringRef)CFPreferencesCopyAppValue( AppleScrollBarType, kCFPreferencesCurrentApplication ));
        if( ScrollBarVariant )
        {
            if( CFGetTypeID( ScrollBarVariant ) == CFStringGetTypeID() )
            {
                
                CFStringRef DoubleMax = CFSTR("DoubleMax");
                if (DoubleMax)
                {
                    if ( !CFStringCompare(ScrollBarVariant, DoubleMax, kCFCompareCaseInsensitive) )
                        bIsScrollbarDoubleMax = true;
                    else
                        bIsScrollbarDoubleMax = false;
                    CFRelease(DoubleMax);
                }
            }
            CFRelease( ScrollBarVariant );
        }
        CFRelease(AppleScrollBarType);
    }

    GetSalData()->mbIsScrollbarDoubleMax = bIsScrollbarDoubleMax;

    CFStringRef jumpScroll = CFSTR("AppleScrollerPagingBehavior");
    if( jumpScroll )
    {
        CFBooleanRef jumpStr = ((CFBooleanRef)CFPreferencesCopyAppValue( jumpScroll, kCFPreferencesCurrentApplication ));
        if( jumpStr )
        {
            if( CFGetTypeID( jumpStr ) == CFBooleanGetTypeID() )
                rSettings.SetPrimaryButtonWarpsSlider(jumpStr == kCFBooleanTrue);
            CFRelease( jumpStr );
        }
        CFRelease( jumpScroll );
    }
}

static Color getColor( NSColor* pSysColor, const Color& rDefault, NSWindow* pWin )
{
    Color aRet( rDefault );
    if( pSysColor )
    {
        
        NSColor* pRBGColor = [pSysColor colorUsingColorSpaceName: NSDeviceRGBColorSpace device: [pWin deviceDescription]];
        if( pRBGColor )
        {
            CGFloat r = 0, g = 0, b = 0, a = 0;
            [pRBGColor getRed: &r green: &g blue: &b alpha: &a];
            aRet = Color( int(r*255.999), int(g*255.999), int(b*255.999) );
            /*
            do not release here; leads to duplicate free in yield
            it seems the converted color comes out autoreleased, although this
            is not documented
            [pRBGColor release];
            */
        }
    }
    return aRet;
}

static Font getFont( NSFont* pFont, long nDPIY, const Font& rDefault )
{
    Font aResult( rDefault );
    if( pFont )
    {
        aResult.SetName( GetOUString( [pFont familyName] ) );
        aResult.SetHeight( static_cast<int>(([pFont pointSize] * 72.0 / (float)nDPIY)+0.5) );
        aResult.SetItalic( ([pFont italicAngle] != 0.0) ? ITALIC_NORMAL : ITALIC_NONE );
        
    }

    return aResult;
}

void AquaSalFrame::getResolution( sal_Int32& o_rDPIX, sal_Int32& o_rDPIY )
{
    if( ! mpGraphics )
    {
        GetGraphics();
        ReleaseGraphics( mpGraphics );
    }
    mpGraphics->GetResolution( o_rDPIX, o_rDPIY );
}







void AquaSalFrame::UpdateSettings( AllSettings& rSettings )
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    [mpNSView lockFocus];

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();

    
    Color aBackgroundColor = Color( 0xEC, 0xEC, 0xEC );
    aStyleSettings.Set3DColors( aBackgroundColor );
    aStyleSettings.SetFaceColor( aBackgroundColor );
    Color aInactiveTabColor( aBackgroundColor );
    aInactiveTabColor.DecreaseLuminance( 32 );
    aStyleSettings.SetInactiveTabColor( aInactiveTabColor );

    aStyleSettings.SetDialogColor( aBackgroundColor );
    aStyleSettings.SetLightBorderColor( aBackgroundColor );
    Color aShadowColor( aStyleSettings.GetShadowColor() );
    aShadowColor.IncreaseLuminance( 32 );
    aStyleSettings.SetShadowColor( aShadowColor );

    
    Font aAppFont = aStyleSettings.GetAppFont();
    sal_Int32 nDPIX = 72, nDPIY = 72;
    getResolution( nDPIX, nDPIY );
    aAppFont = getFont( [NSFont systemFontOfSize: 0], nDPIY, aAppFont );

    aStyleSettings.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_LARGE );

    
    aStyleSettings.SetAppFont( aAppFont );
    aStyleSettings.SetHelpFont( aAppFont );
    aStyleSettings.SetPushButtonFont( aAppFont );

    Font aTitleFont( getFont( [NSFont titleBarFontOfSize: 0], nDPIY, aAppFont ) );
    aStyleSettings.SetTitleFont( aTitleFont );
    aStyleSettings.SetFloatTitleFont( aTitleFont );

    Font aMenuFont( getFont( [NSFont menuFontOfSize: 0], nDPIY, aAppFont ) );
    aStyleSettings.SetMenuFont( aMenuFont );

    aStyleSettings.SetToolFont( aAppFont );

    Font aLabelFont( getFont( [NSFont labelFontOfSize: 0], nDPIY, aAppFont ) );
    aStyleSettings.SetLabelFont( aLabelFont );
    aStyleSettings.SetInfoFont( aLabelFont );
    aStyleSettings.SetRadioCheckFont( aLabelFont );
    aStyleSettings.SetFieldFont( aLabelFont );
    aStyleSettings.SetGroupFont( aLabelFont );
    aStyleSettings.SetIconFont( aLabelFont );

    Color aHighlightColor( getColor( [NSColor selectedTextBackgroundColor],
                                      aStyleSettings.GetHighlightColor(), mpNSWindow ) );
    aStyleSettings.SetHighlightColor( aHighlightColor );
    Color aHighlightTextColor( getColor( [NSColor selectedTextColor],
                                         aStyleSettings.GetHighlightTextColor(), mpNSWindow ) );
    aStyleSettings.SetHighlightTextColor( aHighlightTextColor );

    Color aMenuHighlightColor( getColor( [NSColor selectedMenuItemColor],
                                         aStyleSettings.GetMenuHighlightColor(), mpNSWindow ) );
    aStyleSettings.SetMenuHighlightColor( aMenuHighlightColor );
    Color aMenuHighlightTextColor( getColor( [NSColor selectedMenuItemTextColor],
                                             aStyleSettings.GetMenuHighlightTextColor(), mpNSWindow ) );
    aStyleSettings.SetMenuHighlightTextColor( aMenuHighlightTextColor );

    aStyleSettings.SetMenuColor( aBackgroundColor );
    Color aMenuTextColor( getColor( [NSColor textColor],
                                    aStyleSettings.GetMenuTextColor(), mpNSWindow ) );
    aStyleSettings.SetMenuTextColor( aMenuTextColor );
    aStyleSettings.SetMenuBarTextColor( aMenuTextColor );
    aStyleSettings.SetMenuBarRolloverTextColor( aMenuTextColor );

    aStyleSettings.SetCursorBlinkTime( 500 );

    
    aStyleSettings.SetOptions( aStyleSettings.GetOptions() | STYLE_OPTION_NOMNEMONICS );

    getAppleScrollBarVariant(aStyleSettings);

    
    aStyleSettings.SetScrollBarSize( static_cast<long int>([NSScroller scrollerWidth]) );

    
    aStyleSettings.SetPreferredUseImagesInMenus( false );
    aStyleSettings.SetHideDisabledMenuItems( true );
    aStyleSettings.SetAcceleratorsInContextMenus( false );

    rSettings.SetStyleSettings( aStyleSettings );

    
    ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames = true;

    [mpNSView unlockFocus];
}



const SystemEnvData* AquaSalFrame::GetSystemData() const
{
    return &maSysData;
}



void AquaSalFrame::Beep()
{
    NSBeep();
}



void AquaSalFrame::SetPosSize(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags)
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    sal_uInt16 nEvent = 0;

    if( [mpNSWindow isMiniaturized] )
        [mpNSWindow deminiaturize: NSApp]; 

    if (nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y))
    {
        mbPositioned = true;
        nEvent = SALEVENT_MOVE;
    }

    if (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT))
    {
        mbSized = true;
        nEvent = (nEvent == SALEVENT_MOVE) ? SALEVENT_MOVERESIZE : SALEVENT_RESIZE;
    }

    NSRect aFrameRect = [mpNSWindow frame];
    NSRect aContentRect = [NSWindow contentRectForFrameRect: aFrameRect styleMask: mnStyleMask];

    
    NSRect aParentContentRect;

    if( mpParent )
    {
        if( Application::GetSettings().GetLayoutRTL() )
        {
            if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
                nX = mpParent->maGeometry.nWidth - nWidth-1 - nX;
            else
                nX = mpParent->maGeometry.nWidth - static_cast<long int>( aContentRect.size.width-1) - nX;
        }
        NSRect aParentFrameRect = [mpParent->mpNSWindow frame];
        aParentContentRect = [NSWindow contentRectForFrameRect: aParentFrameRect styleMask: mpParent->mnStyleMask];
    }
    else
        aParentContentRect = maScreenRect; 

    CocoaToVCL( aContentRect );
    CocoaToVCL( aParentContentRect );

    bool bPaint = false;
    if( (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT)) != 0 )
    {
        if( nWidth != aContentRect.size.width || nHeight != aContentRect.size.height )
            bPaint = true;
    }

    
    if( (nFlags & SAL_FRAME_POSSIZE_X) != 0 )
        aContentRect.origin.x = nX + aParentContentRect.origin.x;
    if( (nFlags & SAL_FRAME_POSSIZE_Y) != 0)
        aContentRect.origin.y = nY + aParentContentRect.origin.y;

    
    if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
        aContentRect.size.width = nWidth;
    if( (nFlags & SAL_FRAME_POSSIZE_HEIGHT) != 0)
        aContentRect.size.height = nHeight;

    VCLToCocoa( aContentRect );

    
    {
        [mpNSWindow setFrame: [NSWindow frameRectForContentRect: aContentRect styleMask: mnStyleMask] display: NO];
    }

    UpdateFrameGeometry();

    if (nEvent)
        CallCallback(nEvent, NULL);

    if( mbShown && bPaint )
    {
        
        SendPaintEvent();

        
        [mpNSWindow display];
    }
}

void AquaSalFrame::GetWorkArea( Rectangle& rRect )
{
    if ( !mpNSWindow )
        return;

    
    SalData::ensureThreadAutoreleasePool();

    NSScreen* pScreen = [mpNSWindow screen];
    if( pScreen ==  nil )
        pScreen = [NSScreen mainScreen];
    NSRect aRect = [pScreen visibleFrame];
    CocoaToVCL( aRect );
    rRect.Left()     = static_cast<long>(aRect.origin.x);
    rRect.Top()      = static_cast<long>(aRect.origin.y);
    rRect.Right()    = static_cast<long>(aRect.origin.x + aRect.size.width - 1);
    rRect.Bottom()   = static_cast<long>(aRect.origin.y + aRect.size.height - 1);
}

SalPointerState AquaSalFrame::GetPointerState()
{
    
    SalData::ensureThreadAutoreleasePool();

    SalPointerState state;
    state.mnState = 0;

    
    NSPoint aPt = [mpNSWindow mouseLocationOutsideOfEventStream];
    CocoaToVCL( aPt, false );
    state.maPos = Point(static_cast<long>(aPt.x), static_cast<long>(aPt.y));

    NSEvent* pCur = [NSApp currentEvent];
    bool bMouseEvent = false;
    if( pCur )
    {
        bMouseEvent = true;
        switch( [pCur type] )
        {
        case NSLeftMouseDown:       state.mnState |= MOUSE_LEFT; break;
        case NSLeftMouseUp:         break;
        case NSRightMouseDown:      state.mnState |= MOUSE_RIGHT; break;
        case NSRightMouseUp:        break;
        case NSOtherMouseDown:      state.mnState |= ([pCur buttonNumber] == 2) ? MOUSE_MIDDLE : 0; break;
        case NSOtherMouseUp:        break;
        case NSMouseMoved:          break;
        case NSLeftMouseDragged:    state.mnState |= MOUSE_LEFT; break;
        case NSRightMouseDragged:   state.mnState |= MOUSE_RIGHT; break;
        case NSOtherMouseDragged:   state.mnState |= ([pCur buttonNumber] == 2) ? MOUSE_MIDDLE : 0; break;
            break;
        default:
            bMouseEvent = false;
            break;
        }
    }
    if( bMouseEvent )
    {
        unsigned int nMask = (unsigned int)[pCur modifierFlags];
        if( (nMask & NSShiftKeyMask) != 0 )
            state.mnState |= KEY_SHIFT;
        if( (nMask & NSControlKeyMask) != 0 )
            state.mnState |= KEY_MOD3;
        if( (nMask & NSAlternateKeyMask) != 0 )
            state.mnState |= KEY_MOD2;
        if( (nMask & NSCommandKeyMask) != 0 )
            state.mnState |= KEY_MOD1;

    }
    else
    {
        
        
        
        
        
        
        

        
        UInt32 nState = GetCurrentEventButtonState();
        state.mnState = 0;
        if( nState & 1 )
            state.mnState |= MOUSE_LEFT;    
        if( nState & 2 )
            state.mnState |= MOUSE_RIGHT;   
        if( nState & 4 )
            state.mnState |= MOUSE_MIDDLE;  

        
        nState = GetCurrentEventKeyModifiers();
        if( nState & shiftKey )
            state.mnState |= KEY_SHIFT;
        if( nState & controlKey )
            state.mnState |= KEY_MOD3;
        if( nState & optionKey )
            state.mnState |= KEY_MOD2;
        if( nState & cmdKey )
            state.mnState |= KEY_MOD1;
    }


    return state;
}

SalFrame::SalIndicatorState AquaSalFrame::GetIndicatorState()
{
    SalIndicatorState aState;
    aState.mnState = 0;
    return aState;
}

void AquaSalFrame::SimulateKeyPress( sal_uInt16 /*nKeyCode*/ )
{
}

bool AquaSalFrame::SetPluginParent( SystemParentData* )
{
    
    

    
    return false;
}

bool AquaSalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    
    return FALSE;
}

LanguageType AquaSalFrame::GetInputLanguage()
{
    
    return LANGUAGE_DONTKNOW;
}

void AquaSalFrame::DrawMenuBar()
{
}

void AquaSalFrame::SetMenu( SalMenu* pSalMenu )
{
    
    SalData::ensureThreadAutoreleasePool();

    AquaSalMenu* pMenu = static_cast<AquaSalMenu*>(pSalMenu);
    DBG_ASSERT( ! pMenu || pMenu->mbMenuBar, "setting non menubar on frame" );
    mpMenu = pMenu;
    if( mpMenu  )
        mpMenu->setMainMenu();
}

void AquaSalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    if ( mpNSWindow )
    {
    
    SalData::ensureThreadAutoreleasePool();

    if( (mnExtStyle & SAL_FRAME_EXT_STYLE_DOCMODIFIED) != (nStyle & SAL_FRAME_EXT_STYLE_DOCMODIFIED) )
        [mpNSWindow setDocumentEdited: (nStyle & SAL_FRAME_EXT_STYLE_DOCMODIFIED) ? YES : NO];
    }

    mnExtStyle = nStyle;
}

SalFrame* AquaSalFrame::GetParent() const
{
    return mpParent;
}

void AquaSalFrame::SetParent( SalFrame* pNewParent )
{
    bool bShown = mbShown;
    
    Show( FALSE );
    mpParent = (AquaSalFrame*)pNewParent;
    
    Show( bShown );
}

void AquaSalFrame::UpdateFrameGeometry()
{
    if ( !mpNSWindow )
    {
        return;
    }

    
    

    
    NSScreen * pScreen = [mpNSWindow screen];
    if( pScreen )
    {
        maScreenRect = [pScreen frame];
        NSArray* pScreens = [NSScreen screens];
        if( pScreens )
            maGeometry.nDisplayScreenNumber = [pScreens indexOfObject: pScreen];
    }

    NSRect aFrameRect = [mpNSWindow frame];
    NSRect aContentRect = [NSWindow contentRectForFrameRect: aFrameRect styleMask: mnStyleMask];

    
    [mpNSView removeTrackingRect: mnTrackingRectTag];
    
    NSRect aTrackRect = { { 0, 0 }, aContentRect.size };
    mnTrackingRectTag = [mpNSView addTrackingRect: aTrackRect owner: mpNSView userData: nil assumeInside: NO];

    
    CocoaToVCL( aFrameRect );
    CocoaToVCL( aContentRect );

    maGeometry.nX = static_cast<int>(aContentRect.origin.x);
    maGeometry.nY = static_cast<int>(aContentRect.origin.y);

    maGeometry.nLeftDecoration = static_cast<unsigned int>(aContentRect.origin.x - aFrameRect.origin.x);
    maGeometry.nRightDecoration = static_cast<unsigned int>((aFrameRect.origin.x + aFrameRect.size.width) -
                                  (aContentRect.origin.x + aContentRect.size.width));

    maGeometry.nTopDecoration = static_cast<unsigned int>(aContentRect.origin.y - aFrameRect.origin.y);
    maGeometry.nBottomDecoration = static_cast<unsigned int>((aFrameRect.origin.y + aFrameRect.size.height) -
                                   (aContentRect.origin.y + aContentRect.size.height));

    maGeometry.nWidth = static_cast<unsigned int>(aContentRect.size.width);
    maGeometry.nHeight = static_cast<unsigned int>(aContentRect.size.height);
}



void AquaSalFrame::CaptureMouse( sal_Bool bCapture )
{
    /* Remark:
       we'll try to use a pidgin version of capture mouse
       on MacOSX (neither carbon nor cocoa) there is a
       CaptureMouse equivalent (in Carbon there is TrackMouseLocation
       but this is useless to use since it is blocking)

       However on cocoa the active frame seems to get mouse events
       also outside the window, so we'll try to forward mouse events
       to the capture frame in the hope that one of our frames
       gets a mouse event.

       This will break as soon as the user activates another app, but
       a mouse click will normally lead to a release of the mouse anyway.

       Let's see how far we get this way. Alternatively we could use one
       large overlay window like we did for the carbon implementation,
       however that is resource intensive.
    */

    if( bCapture )
        s_pCaptureFrame = this;
    else if( ! bCapture && s_pCaptureFrame == this )
        s_pCaptureFrame = NULL;
}

void AquaSalFrame::ResetClipRegion()
{
    if ( !mpNSWindow )
    {
        return;
    }

    
    SalData::ensureThreadAutoreleasePool();

    
    CGPathRelease( mrClippingPath );
    mrClippingPath = NULL;

    if( mpNSView && mbShown )
        [mpNSView setNeedsDisplay: YES];
    if( mpNSWindow )
    {
        [mpNSWindow setOpaque: YES];
        [mpNSWindow invalidateShadow];
    }
}

void AquaSalFrame::BeginSetClipRegion( sal_uLong nRects )
{
    if ( !mpNSWindow )
    {
        return;
    }

    
    SalData::ensureThreadAutoreleasePool();

    
    if( mrClippingPath )
    {
        CGPathRelease( mrClippingPath );
        mrClippingPath = NULL;
    }

    if( maClippingRects.size() > SAL_CLIPRECT_COUNT && nRects < maClippingRects.size() )
    {
        std::vector<CGRect> aEmptyVec;
        maClippingRects.swap( aEmptyVec );
    }
    maClippingRects.clear();
    maClippingRects.reserve( nRects );
}

void AquaSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    
    SalData::ensureThreadAutoreleasePool();

    if( nWidth && nHeight )
    {
        NSRect aRect = { { static_cast<CGFloat>(nX), static_cast<CGFloat>(nY) }, { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) } };
        VCLToCocoa( aRect, false );
        maClippingRects.push_back( CGRectMake(aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height) );
    }
}

void AquaSalFrame::EndSetClipRegion()
{
    if ( !mpNSWindow )
    {
        return;
    }

    
    SalData::ensureThreadAutoreleasePool();

    if( ! maClippingRects.empty() )
    {
        mrClippingPath = CGPathCreateMutable();
        CGPathAddRects( mrClippingPath, NULL, &maClippingRects[0], maClippingRects.size() );
    }
    if( mpNSView && mbShown )
        [mpNSView setNeedsDisplay: YES];
    if( mpNSWindow )
    {
        [mpNSWindow setOpaque: (mrClippingPath != NULL) ? NO : YES];
        [mpNSWindow setBackgroundColor: [NSColor clearColor]];
        
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
