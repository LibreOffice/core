/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************************************************************************
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


#include <string>

#include "rtl/ustrbuf.hxx"

#include "osl/file.h"

#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/timer.hxx"

#include "aqua/saldata.hxx"
#include "aqua/salgdi.h"
#include "aqua/salframe.h"
#include "aqua/salmenu.h"
#include "aqua/saltimer.h"
#include "aqua/salinst.h"
#include "aqua/salframeview.h"
#include "aqua/aqua11yfactory.h"

#include "salwtype.hxx"

#include "premac.h"
#include <objc/objc-runtime.h>
// needed for theming
// FIXME: move theming code to salnativewidgets.cxx
#include <Carbon/Carbon.h>
#include "postmac.h"


using namespace std;

// =======================================================================

AquaSalFrame* AquaSalFrame::s_pCaptureFrame = NULL;

// =======================================================================

AquaSalFrame::AquaSalFrame( SalFrame* pParent, sal_uLong salFrameStyle ) :
    mpWindow(nil),
    mpView(nil),
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

// -----------------------------------------------------------------------

AquaSalFrame::~AquaSalFrame()
{
    // if the frame is destroyed and has the current menubar
    // set the default menubar
    if( mpMenu && mpMenu->mbMenuBar && AquaSalMenu::pCurrentMenuBar == mpMenu )
        AquaSalMenu::setDefaultMenu();

    // cleanup clipping stuff
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
        // life cycle comment: the menu has ownership of the item, so no release
        [AquaSalInstance::GetDynamicDockMenu() removeItem: mpDockMenuEntry];
    if ( mpView ) {
        [AquaA11yFactory revokeView: mpView];
        [mpView release];
    }
    if ( mpWindow )
        [mpWindow release];
}

// -----------------------------------------------------------------------

void AquaSalFrame::initWindowAndView()
{
    // initialize mirroring parameters
    // FIXME: screens changing
    NSScreen * pScreen = [mpWindow screen];
    if( pScreen == nil )
        pScreen = [NSScreen mainScreen];
    maScreenRect = [pScreen frame];

    // calculate some default geometry
    NSRect aVisibleRect = [pScreen visibleFrame];
    CocoaToVCL( aVisibleRect );

    maGeometry.nX = static_cast<int>(aVisibleRect.origin.x + aVisibleRect.size.width / 10);
    maGeometry.nY = static_cast<int>(aVisibleRect.origin.y + aVisibleRect.size.height / 10);
    maGeometry.nWidth = static_cast<unsigned int>(aVisibleRect.size.width * 0.8);
    maGeometry.nHeight = static_cast<unsigned int>(aVisibleRect.size.height * 0.8);

    // calculate style mask
    if( (mnStyle & SAL_FRAME_STYLE_FLOAT) ||
        (mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        mnStyleMask = NSBorderlessWindowMask;
    else if( mnStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        mnStyleMask = NSTitledWindowMask            |
                      NSMiniaturizableWindowMask    |
                      NSResizableWindowMask         |
                      NSClosableWindowMask;
        // make default window "maximized"
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
        // documentation says anything other than NSBorderlessWindowMask (=0)
        // should also include NSTitledWindowMask;
        if( mnStyleMask != 0 )
            mnStyleMask |= NSTitledWindowMask;
    }

    // #i91990# support GUI-less (daemon) execution
    @try
    {
    mpWindow = [[SalFrameWindow alloc] initWithSalFrame: this];
    mpView = [[SalFrameView alloc] initWithSalFrame: this];
    }
    @catch ( id exception )
    {
        return;
    }

    if( (mnStyle & SAL_FRAME_STYLE_TOOLTIP) )
        [mpWindow setIgnoresMouseEvents: YES];
    else
        [mpWindow setAcceptsMouseMovedEvents: YES];
    [mpWindow setHasShadow: YES];

    // WTF? With the 10.6 SDK and gcc 4.2.1, we get: class 'NSWindow'
    // does not implement the 'NSWindowDelegate' protocol. Anyway,
    // having the window object be its own delegate object is
    // apparently what the code does on purpose, see discussion in
    // https://issues.apache.org/ooo/show_bug.cgi?id=91990

    // So to silence the warning when compiling with -Werror, instead of:
    // [mpWindow setDelegate: mpWindow];
    // do this:
    objc_msgSend(mpWindow, @selector(setDelegate:), mpWindow);

    if( [mpWindow respondsToSelector: @selector(setRestorable:)])
    {
        objc_msgSend(mpWindow, @selector(setRestorable:), NO);
    }
    NSRect aRect = { { 0,0 }, { maGeometry.nWidth, maGeometry.nHeight } };
    mnTrackingRectTag = [mpView addTrackingRect: aRect owner: mpView userData: nil assumeInside: NO];

    maSysData.pView = mpView;

    UpdateFrameGeometry();

    [mpWindow setContentView: mpView];
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void AquaSalFrame::screenParametersChanged()
{
    UpdateFrameGeometry();

    if( mpGraphics )
        mpGraphics->updateResolution();
    CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void AquaSalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    (void)pGraphics;
    DBG_ASSERT( pGraphics == mpGraphics, "graphics released on wrong frame" );
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalFrame::PostEvent( void *pData )
{
    GetSalData()->mpFirstInstance->PostUserEvent( this, SALEVENT_USEREVENT, pData );
    return TRUE;
}

// -----------------------------------------------------------------------
void AquaSalFrame::SetTitle(const rtl::OUString& rTitle)
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    NSString* pTitle = CreateNSString( rTitle );
    [mpWindow setTitle: pTitle];

    // create an entry in the dock menu
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
            [mpDockMenuEntry setTarget: mpWindow];

            // TODO: image (either the generic window image or an icon
            // check mark (for "main" window ?)
        }
        else
            [mpDockMenuEntry setTitle: pTitle];
    }

    if (pTitle)
        [pTitle release];
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetIcon( sal_uInt16 )
{
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetRepresentedURL( const rtl::OUString& i_rDocURL )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( i_rDocURL.indexOfAsciiL( "file:", 5 ) == 0 )
    {
        rtl::OUString aSysPath;
        osl_getSystemPathFromFileURL( i_rDocURL.pData, &aSysPath.pData );
        NSString* pStr = CreateNSString( aSysPath );
        if( pStr )
        {
            [pStr autorelease];
            [mpWindow setRepresentedFilename: pStr];
        }
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::initShow()
{
    mbInitShow = false;
    if( ! mbPositioned && ! mbFullScreen )
    {
        Rectangle aScreenRect;
        GetWorkArea( aScreenRect );
        if( mpParent ) // center relative to parent
        {
            // center on parent
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
            // center on screen
            long nNewX = (aScreenRect.GetWidth() - maGeometry.nWidth)/2;
            long nNewY = (aScreenRect.GetHeight() - maGeometry.nHeight)/2;
            SetPosSize( nNewX, nNewY, 0, 0,  SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
        }
    }

    // make sure the view is present in the wrapper list before any children receive focus
    [AquaA11yFactory registerView: mpView];
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

// -----------------------------------------------------------------------

void AquaSalFrame::Show(sal_Bool bVisible, sal_Bool bNoActivate)
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    mbShown = bVisible;
    if(bVisible)
    {
        if( mbInitShow )
            initShow();

        CallCallback(SALEVENT_RESIZE, 0);
        // trigger filling our backbuffer
        SendPaintEvent();

        if( bNoActivate || [mpWindow canBecomeKeyWindow] == NO )
            [mpWindow orderFront: NSApp];
        else
            [mpWindow makeKeyAndOrderFront: NSApp];

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
                [mpParent->mpWindow addChildWindow: mpWindow ordered: NSWindowAbove];
            }
        }

        if( mbPresentation )
            [mpWindow makeMainWindow];
    }
    else
    {
        // if the frame holding the current menubar gets hidden
        // show the default menubar
        if( mpMenu && mpMenu->mbMenuBar && AquaSalMenu::pCurrentMenuBar == mpMenu )
            AquaSalMenu::setDefaultMenu();

        // #i90440# #i94443# work around the focus going back to some other window
        // if a child gets hidden for a parent window
        if( mpParent && mpParent->mbShown && [mpWindow isKeyWindow] )
            [mpParent->mpWindow makeKeyAndOrderFront: NSApp];

        [SalFrameView unsetMouseFrame: this];
        if( mpParent && [mpWindow parentWindow] == mpParent->mpWindow )
            [mpParent->mpWindow removeChildWindow: mpWindow];

        [mpWindow orderOut: NSApp];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::Enable( sal_Bool )
{
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    mnMinWidth = nWidth;
    mnMinHeight = nHeight;

    if( mpWindow )
    {
        // Always add the decoration as the dimension concerns only
        // the content rectangle
        nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

        NSSize aSize = { nWidth, nHeight };

        // Size of full window (content+structure) although we only
        // have the client size in arguments
        [mpWindow setMinSize: aSize];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    mnMaxWidth = nWidth;
    mnMaxHeight = nHeight;

    if( mpWindow )
    {
        // Always add the decoration as the dimension concerns only
        // the content rectangle
        nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

        // Carbon windows can't have a size greater than 32767x32767
        if (nWidth>32767) nWidth=32767;
        if (nHeight>32767) nHeight=32767;

        NSSize aSize = { nWidth, nHeight };

        // Size of full window (content+structure) although we only
        // have the client size in arguments
        [mpWindow setMaxSize: aSize];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetClientSize( long nWidth, long nHeight )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( mpWindow )
    {
        NSSize aSize = { nWidth, nHeight };

        [mpWindow setContentSize: aSize];
        UpdateFrameGeometry();
        if( mbShown )
            // trigger filling our backbuffer
            SendPaintEvent();
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void AquaSalFrame::SetWindowState( const SalFrameState* pState )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if ( mpWindow )
    {
    // set normal state
    NSRect aStateRect = [mpWindow frame];
    aStateRect = [NSWindow contentRectForFrameRect: aStateRect styleMask: mnStyleMask];
    CocoaToVCL( aStateRect );
    if( pState->mnMask & SAL_FRAMESTATE_MASK_X )
        aStateRect.origin.x = float(pState->mnX);
    if( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
        aStateRect.origin.y = float(pState->mnY);
    if( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
        aStateRect.size.width = float(pState->mnWidth);
    if( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
        aStateRect.size.height = float(pState->mnHeight);
    VCLToCocoa( aStateRect );
    aStateRect = [NSWindow frameRectForContentRect: aStateRect styleMask: mnStyleMask];

    [mpWindow setFrame: aStateRect display: NO];
    if( pState->mnState == SAL_FRAMESTATE_MINIMIZED )
        [mpWindow miniaturize: NSApp];
    else if( [mpWindow isMiniaturized] )
        [mpWindow deminiaturize: NSApp];


    /* ZOOMED is not really maximized (actually it toggles between a user set size and
       the program specified one), but comes closest since the default behavior is
       "maximized" if the user did not intervene
    */
    if( pState->mnState == SAL_FRAMESTATE_MAXIMIZED )
    {
        if(! [mpWindow isZoomed])
            [mpWindow zoom: NSApp];
    }
    else
    {
        if( [mpWindow isZoomed] )
            [mpWindow zoom: NSApp];
    }
    }

    // get new geometry
    UpdateFrameGeometry();

    sal_uInt16 nEvent = 0;
    if( pState->mnMask & (SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_Y) )
    {
        mbPositioned = true;
        nEvent = SALEVENT_MOVE;
    }

    if( pState->mnMask & (SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT) )
    {
        mbSized = true;
        nEvent = (nEvent == SALEVENT_MOVE) ? SALEVENT_MOVERESIZE : SALEVENT_RESIZE;
    }
    // send event that we were moved/sized
    if( nEvent )
        CallCallback( nEvent, NULL );

    if( mbShown && mpWindow )
    {
        // trigger filling our backbuffer
        SendPaintEvent();

        // tell the system the views need to be updated
        [mpWindow display];
    }
}

// -----------------------------------------------------------------------

sal_Bool AquaSalFrame::GetWindowState( SalFrameState* pState )
{
    if ( !mpWindow )
        return FALSE;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    pState->mnMask = SAL_FRAMESTATE_MASK_X                 |
                     SAL_FRAMESTATE_MASK_Y                 |
                     SAL_FRAMESTATE_MASK_WIDTH             |
                     SAL_FRAMESTATE_MASK_HEIGHT            |
                     SAL_FRAMESTATE_MASK_STATE;

    NSRect aStateRect = [mpWindow frame];
    aStateRect = [NSWindow contentRectForFrameRect: aStateRect styleMask: mnStyleMask];
    CocoaToVCL( aStateRect );
    pState->mnX         = long(aStateRect.origin.x);
    pState->mnY         = long(aStateRect.origin.y);
    pState->mnWidth     = long(aStateRect.size.width);
    pState->mnHeight    = long(aStateRect.size.height);

    if( [mpWindow isMiniaturized] )
        pState->mnState = SAL_FRAMESTATE_MINIMIZED;
    else if( ! [mpWindow isZoomed] )
        pState->mnState = SAL_FRAMESTATE_NORMAL;
    else
        pState->mnState = SAL_FRAMESTATE_MAXIMIZED;

    return TRUE;
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetScreenNumber(unsigned int nScreen)
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    NSArray* pScreens = [NSScreen screens];
    Rectangle aRet;
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
    {
        // get new screen frame
        pScreen = [pScreens objectAtIndex: nScreen];
        NSRect aNewScreen = [pScreen frame];

        // get current screen frame
        pScreen = [mpWindow screen];
        if( pScreen )
        {
            NSRect aCurScreen = [pScreen frame];
            if( aCurScreen.origin.x != aNewScreen.origin.x ||
                aCurScreen.origin.y != aNewScreen.origin.y )
            {
                NSRect aFrameRect = [mpWindow frame];
                aFrameRect.origin.x += aNewScreen.origin.x - aCurScreen.origin.x;
                aFrameRect.origin.y += aNewScreen.origin.y - aCurScreen.origin.y;
                [mpWindow setFrame: aFrameRect display: NO];
                UpdateFrameGeometry();
            }
        }
    }
}

void AquaSalFrame::SetApplicationID( const rtl::OUString &/*rApplicationID*/ )
{
}

// -----------------------------------------------------------------------

void AquaSalFrame::ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay )
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( mbFullScreen == bFullScreen )
        return;

    mbFullScreen = bFullScreen;
    if( bFullScreen )
    {
        // hide the dock and the menubar if we are on the menu screen
        // which is always on index 0 according to documentation
        bool bHideMenu = (nDisplay == 0);

        NSRect aNewContentRect = { { 0, 0 }, { 0, 0 } };
        // get correct screen
        NSScreen* pScreen = nil;
        NSArray* pScreens = [NSScreen screens];
        if( pScreens )
        {
            if( nDisplay >= 0 && (unsigned int)nDisplay < [pScreens count] )
                pScreen = [pScreens objectAtIndex: nDisplay];
            else
            {
                // this means span all screens
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
                pScreen = [mpWindow screen];
            if( pScreen == nil )
                pScreen = [NSScreen mainScreen];

            aNewContentRect = [pScreen frame];
        }

        if( bHideMenu )
            [NSMenu setMenuBarVisible:NO];

        maFullScreenRect = [mpWindow frame];
        {
            [mpWindow setFrame: [NSWindow frameRectForContentRect: aNewContentRect styleMask: mnStyleMask] display: mbShown ? YES : NO];
        }

        UpdateFrameGeometry();

        if( mbShown )
            CallCallback( SALEVENT_MOVERESIZE, NULL );
    }
    else
    {
        {
            [mpWindow setFrame: maFullScreenRect display: mbShown ? YES : NO];
        }
        UpdateFrameGeometry();

        if( mbShown )
            CallCallback( SALEVENT_MOVERESIZE, NULL );

        // show the dock and the menubar
        [NSMenu setMenuBarVisible:YES];
    }
    if( mbShown )
        // trigger filling our backbuffer
        SendPaintEvent();
}

// -----------------------------------------------------------------------

class PreventSleepTimer : public AutoTimer
{
public:
    PreventSleepTimer()
    {
        SetTimeout( 30000 );
        Start();
    }

    virtual ~PreventSleepTimer()
    {
    }

    virtual void Timeout()
    {
        UpdateSystemActivity(OverallAct);
    }
};

void AquaSalFrame::StartPresentation( sal_Bool bStart )
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( bStart )
    {
        GetSalData()->maPresentationFrames.push_back( this );
        mpActivityTimer.reset( new PreventSleepTimer() );
        [mpWindow setLevel: NSPopUpMenuWindowLevel];
        if( mbShown )
            [mpWindow makeMainWindow];
    }
    else
    {
        GetSalData()->maPresentationFrames.remove( this );
        mpActivityTimer.reset();
        [mpWindow setLevel: NSNormalWindowLevel];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetAlwaysOnTop( sal_Bool )
{
}

// -----------------------------------------------------------------------

void AquaSalFrame::ToTop(sal_uInt16 nFlags)
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( ! (nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN) )
    {
        if( ! [mpWindow isVisible] || [mpWindow isMiniaturized] )
            return;
    }
    if( nFlags & SAL_FRAME_TOTOP_GRABFOCUS )
        [mpWindow makeKeyAndOrderFront: NSApp];
    else
        [mpWindow orderFront: NSApp];
}

// -----------------------------------------------------------------------

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
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( ePointerStyle >= POINTER_COUNT || ePointerStyle == mePointerStyle )
        return;
    mePointerStyle = ePointerStyle;

    [mpWindow invalidateCursorRectsForView: mpView];
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetPointerPos( long nX, long nY )
{
    // FIXME: use Cocoa functions

    // FIXME: multiscreen support
    CGPoint aPoint = { nX + maGeometry.nX, nY + maGeometry.nY };
    CGDirectDisplayID mainDisplayID = CGMainDisplayID();
    CGDisplayMoveCursorToPoint( mainDisplayID, aPoint );
}

// -----------------------------------------------------------------------

void AquaSalFrame::Flush( void )
{
    if( !(mbGraphics && mpGraphics && mpView && mbShown) )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();


    [mpView setNeedsDisplay: YES];

    // outside of the application's event loop (e.g. IntroWindow)
    // nothing would trigger paint event handling
    // => fall back to synchronous painting
    if( ImplGetSVData()->maAppData.mnDispatchLevel <= 0 )
    {
        [mpView display];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::Flush( const Rectangle& rRect )
{
    if( !(mbGraphics && mpGraphics && mpView && mbShown) )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    NSRect aNSRect = { {rRect.Left(), rRect.Top()}, { rRect.GetWidth(), rRect.GetHeight() } };
    VCLToCocoa( aNSRect, false );
    [mpView setNeedsDisplayInRect: aNSRect];

    // outside of the application's event loop (e.g. IntroWindow)
    // nothing would trigger paint event handling
    // => fall back to synchronous painting
    if( ImplGetSVData()->maAppData.mnDispatchLevel <= 0 )
    {
        [mpView display];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::Sync()
{
    if( mbGraphics && mpGraphics && mpView && mbShown )
    {
        // #i113170# may not be the main thread if called from UNO API
        SalData::ensureThreadAutoreleasePool();

        [mpView setNeedsDisplay: YES];
        [mpView display];
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void AquaSalFrame::EndExtTextInput( sal_uInt16 )
{
}

// -----------------------------------------------------------------------

rtl::OUString AquaSalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
    static std::map< sal_uInt16, rtl::OUString > aKeyMap;
    if( aKeyMap.empty() )
    {
        sal_uInt16 i;
        for( i = KEY_A; i <= KEY_Z; i++ )
            aKeyMap[ i ] = rtl::OUString( sal_Unicode( 'A' + (i - KEY_A) ) );
        for( i = KEY_0; i <= KEY_9; i++ )
            aKeyMap[ i ] = rtl::OUString( sal_Unicode( '0' + (i - KEY_0) ) );
        for( i = KEY_F1; i <= KEY_F26; i++ )
        {
            rtl::OUStringBuffer aKey( 3 );
            aKey.append( sal_Unicode( 'F' ) );
            aKey.append( sal_Int32( i - KEY_F1 + 1 ) );
            aKeyMap[ i ] = aKey.makeStringAndClear();
        }

        aKeyMap[ KEY_DOWN ]     = rtl::OUString( sal_Unicode( 0x21e3 ) );
        aKeyMap[ KEY_UP ]       = rtl::OUString( sal_Unicode( 0x21e1 ) );
        aKeyMap[ KEY_LEFT ]     = rtl::OUString( sal_Unicode( 0x21e0 ) );
        aKeyMap[ KEY_RIGHT ]    = rtl::OUString( sal_Unicode( 0x21e2 ) );
        aKeyMap[ KEY_HOME ]     = rtl::OUString( sal_Unicode( 0x2196 ) );
        aKeyMap[ KEY_END ]      = rtl::OUString( sal_Unicode( 0x2198 ) );
        aKeyMap[ KEY_PAGEUP ]   = rtl::OUString( sal_Unicode( 0x21de ) );
        aKeyMap[ KEY_PAGEDOWN ] = rtl::OUString( sal_Unicode( 0x21df ) );
        aKeyMap[ KEY_RETURN ]   = rtl::OUString( sal_Unicode( 0x21a9 ) );
        aKeyMap[ KEY_ESCAPE ]   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "esc" ) );
        aKeyMap[ KEY_TAB ]      = rtl::OUString( sal_Unicode( 0x21e5 ) );
        aKeyMap[ KEY_BACKSPACE ]= rtl::OUString( sal_Unicode( 0x232b ) );
        aKeyMap[ KEY_SPACE ]    = rtl::OUString( sal_Unicode( 0x2423 ) );
        aKeyMap[ KEY_DELETE ]   = rtl::OUString( sal_Unicode( 0x2326 ) );
        aKeyMap[ KEY_ADD ]      = rtl::OUString( sal_Unicode( '+' ) );
        aKeyMap[ KEY_SUBTRACT ] = rtl::OUString( sal_Unicode( '-' ) );
        aKeyMap[ KEY_DIVIDE ]   = rtl::OUString( sal_Unicode( '/' ) );
        aKeyMap[ KEY_MULTIPLY ] = rtl::OUString( sal_Unicode( '*' ) );
        aKeyMap[ KEY_POINT ]    = rtl::OUString( sal_Unicode( '.' ) );
        aKeyMap[ KEY_COMMA ]    = rtl::OUString( sal_Unicode( ',' ) );
        aKeyMap[ KEY_LESS ]     = rtl::OUString( sal_Unicode( '<' ) );
        aKeyMap[ KEY_GREATER ]  = rtl::OUString( sal_Unicode( '>' ) );
        aKeyMap[ KEY_EQUAL ]    = rtl::OUString( sal_Unicode( '=' ) );
        aKeyMap[ KEY_OPEN ]     = rtl::OUString( sal_Unicode( 0x23cf ) );

        /* yet unmapped KEYCODES:
        aKeyMap[ KEY_INSERT ]   = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_CUT ]      = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_COPY ]     = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_PASTE ]    = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_UNDO ]     = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_REPEAT ]   = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_FIND ]     = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_PROPERTIES ]     = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_FRONT ]    = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_CONTEXTMENU ]    = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_MENU ]     = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_HELP ]     = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_HANGUL_HANJA ]   = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_DECIMAL ]  = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_TILDE ]    = rtl::OUString( sal_Unicode( ) );
        aKeyMap[ KEY_QUOTELEFT ]= rtl::OUString( sal_Unicode( ) );
        */

    }

    rtl::OUStringBuffer aResult( 16 );

    sal_uInt16 nUnmodifiedCode = (nKeyCode & KEY_CODE);
    std::map< sal_uInt16, rtl::OUString >::const_iterator it = aKeyMap.find( nUnmodifiedCode );
    if( it != aKeyMap.end() )
    {
        if( (nKeyCode & KEY_SHIFT) != 0 )
            aResult.append( sal_Unicode( 0x21e7 ) );
        if( (nKeyCode & KEY_MOD1) != 0 )
            aResult.append( sal_Unicode( 0x2318 ) );
        // we do not really handle Alt (see below)
        // we map it to MOD3, whichis actually Command
        if( (nKeyCode & (KEY_MOD2|KEY_MOD3)) != 0 )
            aResult.append( sal_Unicode( 0x2303 ) );

        aResult.append( it->second );
    }

    return aResult.makeStringAndClear();
}

// -----------------------------------------------------------------------

static void getAppleScrollBarVariant(void)
{
    bool bIsScrollbarDoubleMax = true; // default is DoubleMax

    CFStringRef AppleScrollBarType = CFSTR("AppleScrollBarVariant");
    if( AppleScrollBarType )
    {
        CFStringRef ScrollBarVariant = ((CFStringRef)CFPreferencesCopyAppValue( AppleScrollBarType, kCFPreferencesCurrentApplication ));
        if( ScrollBarVariant )
        {
            if( CFGetTypeID( ScrollBarVariant ) == CFStringGetTypeID() )
            {
                // TODO: check for the less important variants "DoubleMin" and "DoubleBoth" too
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
                ImplGetSVData()->maNWFData.mbScrollbarJumpPage = (jumpStr == kCFBooleanTrue);
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
        // transform to RGB
        NSColor* pRBGColor = [pSysColor colorUsingColorSpaceName: NSDeviceRGBColorSpace device: [pWin deviceDescription]];
        if( pRBGColor )
        {
            float r = 0, g = 0, b = 0, a = 0;
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
        // FIMXE: bold ?
    }

    return aResult;
}

void AquaSalFrame::getResolution( long& o_rDPIX, long& o_rDPIY )
{
    if( ! mpGraphics )
    {
        GetGraphics();
        ReleaseGraphics( mpGraphics );
    }
    mpGraphics->GetResolution( o_rDPIX, o_rDPIY );
}

// on OSX-Aqua the style settings are independent of the frame, so it does
// not really belong here. Since the connection to the Appearance_Manager
// is currently done in salnativewidgets.cxx this would be a good place.
// On the other hand VCL's platform independent code currently only asks
// SalFrames for system settings anyway, so moving the code somewhere else
// doesn't make the anything cleaner for now
void AquaSalFrame::UpdateSettings( AllSettings& rSettings )
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    [mpView lockFocus];

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();

    // Background Color
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

    // get the system font settings
    Font aAppFont = aStyleSettings.GetAppFont();
    long nDPIX = 72, nDPIY = 72;
    getResolution( nDPIX, nDPIY );
    aAppFont = getFont( [NSFont systemFontOfSize: 0], nDPIY, aAppFont );

    aStyleSettings.SetToolbarIconSize( nDPIY > 160 ? STYLE_TOOLBAR_ICONSIZE_LARGE : STYLE_TOOLBAR_ICONSIZE_SMALL );

    // TODO: better mapping of aqua<->ooo font settings
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
                                      aStyleSettings.GetHighlightColor(), mpWindow ) );
    aStyleSettings.SetHighlightColor( aHighlightColor );
    Color aHighlightTextColor( getColor( [NSColor selectedTextColor],
                                         aStyleSettings.GetHighlightTextColor(), mpWindow ) );
    aStyleSettings.SetHighlightTextColor( aHighlightTextColor );

    Color aMenuHighlightColor( getColor( [NSColor selectedMenuItemColor],
                                         aStyleSettings.GetMenuHighlightColor(), mpWindow ) );
    aStyleSettings.SetMenuHighlightColor( aMenuHighlightColor );
    Color aMenuHighlightTextColor( getColor( [NSColor selectedMenuItemTextColor],
                                             aStyleSettings.GetMenuHighlightTextColor(), mpWindow ) );
    aStyleSettings.SetMenuHighlightTextColor( aMenuHighlightTextColor );

    aStyleSettings.SetMenuColor( aBackgroundColor );
    Color aMenuTextColor( getColor( [NSColor textColor],
                                    aStyleSettings.GetMenuTextColor(), mpWindow ) );
    aStyleSettings.SetMenuTextColor( aMenuTextColor );
    aStyleSettings.SetMenuBarTextColor( aMenuTextColor );

    aStyleSettings.SetCursorBlinkTime( 500 );

    // no mnemonics on aqua
    aStyleSettings.SetOptions( aStyleSettings.GetOptions() | STYLE_OPTION_NOMNEMONICS );

    getAppleScrollBarVariant();

    // set scrollbar size
    aStyleSettings.SetScrollBarSize( static_cast<long int>([NSScroller scrollerWidth]) );

    // images in menus false for MacOSX
    aStyleSettings.SetPreferredUseImagesInMenus( false );
    aStyleSettings.SetHideDisabledMenuItems( sal_True );
    aStyleSettings.SetAcceleratorsInContextMenus( sal_False );

    rSettings.SetStyleSettings( aStyleSettings );

    [mpView unlockFocus];
}

// -----------------------------------------------------------------------

const SystemEnvData* AquaSalFrame::GetSystemData() const
{
    return &maSysData;
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetPosSize(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags)
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    sal_uInt16 nEvent = 0;

    if( [mpWindow isMiniaturized] )
        [mpWindow deminiaturize: NSApp]; // expand the window

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

    NSRect aFrameRect = [mpWindow frame];
    NSRect aContentRect = [NSWindow contentRectForFrameRect: aFrameRect styleMask: mnStyleMask];

    // position is always relative to parent frame
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
        NSRect aParentFrameRect = [mpParent->mpWindow frame];
        aParentContentRect = [NSWindow contentRectForFrameRect: aParentFrameRect styleMask: mpParent->mnStyleMask];
    }
    else
        aParentContentRect = maScreenRect; // use screen if no parent

    CocoaToVCL( aContentRect );
    CocoaToVCL( aParentContentRect );

    bool bPaint = false;
    if( (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT)) != 0 )
    {
        if( nWidth != aContentRect.size.width || nHeight != aContentRect.size.height )
            bPaint = true;
    }

    // use old window pos if no new pos requested
    if( (nFlags & SAL_FRAME_POSSIZE_X) != 0 )
        aContentRect.origin.x = nX + aParentContentRect.origin.x;
    if( (nFlags & SAL_FRAME_POSSIZE_Y) != 0)
        aContentRect.origin.y = nY + aParentContentRect.origin.y;

    // use old size if no new size requested
    if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
        aContentRect.size.width = nWidth;
    if( (nFlags & SAL_FRAME_POSSIZE_HEIGHT) != 0)
        aContentRect.size.height = nHeight;

    VCLToCocoa( aContentRect );

    // do not display yet, we need to update our backbuffer
    {
        [mpWindow setFrame: [NSWindow frameRectForContentRect: aContentRect styleMask: mnStyleMask] display: NO];
    }

    UpdateFrameGeometry();

    if (nEvent)
        CallCallback(nEvent, NULL);

    if( mbShown && bPaint )
    {
        // trigger filling our backbuffer
        SendPaintEvent();

        // now inform the system that the views need to be drawn
        [mpWindow display];
    }
}

void AquaSalFrame::GetWorkArea( Rectangle& rRect )
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    NSScreen* pScreen = [mpWindow screen];
    if( pScreen ==  nil )
        pScreen = [NSScreen mainScreen];
    NSRect aRect = [pScreen visibleFrame];
    CocoaToVCL( aRect );
    rRect.nLeft     = static_cast<long>(aRect.origin.x);
    rRect.nTop      = static_cast<long>(aRect.origin.y);
    rRect.nRight    = static_cast<long>(aRect.origin.x + aRect.size.width - 1);
    rRect.nBottom   = static_cast<long>(aRect.origin.y + aRect.size.height - 1);
}

SalPointerState AquaSalFrame::GetPointerState()
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    SalPointerState state;
    state.mnState = 0;

    // get position
    NSPoint aPt = [mpWindow mouseLocationOutsideOfEventStream];
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
        // FIXME: replace Carbon by Cocoa
        // Cocoa does not have an equivalent for GetCurrentEventButtonState
        // and GetCurrentEventKeyModifiers.
        // we could try to get away with tracking all events for modifierKeys
        // and all mouse events for button state in VCL_NSApllication::sendEvent,
        // but it is unclear whether this will get us the same result.
        // leave in GetCurrentEventButtonState and GetCurrentEventKeyModifiers for now

        // fill in button state
        UInt32 nState = GetCurrentEventButtonState();
        state.mnState = 0;
        if( nState & 1 )
            state.mnState |= MOUSE_LEFT;    // primary button
        if( nState & 2 )
            state.mnState |= MOUSE_RIGHT;   // secondary button
        if( nState & 4 )
            state.mnState |= MOUSE_MIDDLE;  // tertiary button

        // fill in modifier state
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
    // plugin parent may be killed unexpectedly by
    // plugging process;

    //TODO: implement
    return sal_False;
}

sal_Bool AquaSalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    // not supported yet
    return FALSE;
}

LanguageType AquaSalFrame::GetInputLanguage()
{
    //TODO: implement
    return LANGUAGE_DONTKNOW;
}

void AquaSalFrame::DrawMenuBar()
{
}

void AquaSalFrame::SetMenu( SalMenu* pSalMenu )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    AquaSalMenu* pMenu = static_cast<AquaSalMenu*>(pSalMenu);
    DBG_ASSERT( ! pMenu || pMenu->mbMenuBar, "setting non menubar on frame" );
    mpMenu = pMenu;
    if( mpMenu  )
        mpMenu->setMainMenu();
}

void AquaSalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    if ( mpWindow )
    {
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( (mnExtStyle & SAL_FRAME_EXT_STYLE_DOCMODIFIED) != (nStyle & SAL_FRAME_EXT_STYLE_DOCMODIFIED) )
        [mpWindow setDocumentEdited: (nStyle & SAL_FRAME_EXT_STYLE_DOCMODIFIED) ? YES : NO];
    }

    mnExtStyle = nStyle;
}

void AquaSalFrame::SetBackgroundBitmap( SalBitmap* )
{
    //TODO: implement
}

SalFrame* AquaSalFrame::GetParent() const
{
    return mpParent;
}

void AquaSalFrame::SetParent( SalFrame* pNewParent )
{
    bool bShown = mbShown;
    // remove from child list
    Show( FALSE );
    mpParent = (AquaSalFrame*)pNewParent;
    // insert to correct parent and paint
    Show( bShown );
}

void AquaSalFrame::UpdateFrameGeometry()
{
    if ( !mpWindow )
    {
        return;
    }

    // keep in mind that view and window coordinates are lower left
    // whereas vcl's are upper left

    // update screen rect
    NSScreen * pScreen = [mpWindow screen];
    if( pScreen )
    {
        maScreenRect = [pScreen frame];
        NSArray* pScreens = [NSScreen screens];
        if( pScreens )
            maGeometry.nDisplayScreenNumber = [pScreens indexOfObject: pScreen];
    }

    NSRect aFrameRect = [mpWindow frame];
    NSRect aContentRect = [NSWindow contentRectForFrameRect: aFrameRect styleMask: mnStyleMask];

    // release old track rect
    [mpView removeTrackingRect: mnTrackingRectTag];
    // install the new track rect
    NSRect aTrackRect = { { 0, 0 }, aContentRect.size };
    mnTrackingRectTag = [mpView addTrackingRect: aTrackRect owner: mpView userData: nil assumeInside: NO];

    // convert to vcl convention
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

// -----------------------------------------------------------------------

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
    if ( !mpWindow )
    {
        return;
    }

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // release old path and indicate no clipping
    CGPathRelease( mrClippingPath );
    mrClippingPath = NULL;

    if( mpView && mbShown )
        [mpView setNeedsDisplay: YES];
    if( mpWindow )
    {
        [mpWindow setOpaque: YES];
        [mpWindow invalidateShadow];
    }
}

void AquaSalFrame::BeginSetClipRegion( sal_uLong nRects )
{
    if ( !mpWindow )
    {
        return;
    }

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // release old path
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
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( nWidth && nHeight )
    {
        NSRect aRect = { { nX, nY }, { nWidth, nHeight } };
        VCLToCocoa( aRect, false );
        maClippingRects.push_back( CGRectMake(aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height) );
    }
}

void AquaSalFrame::EndSetClipRegion()
{
    if ( !mpWindow )
    {
        return;
    }

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( ! maClippingRects.empty() )
    {
        mrClippingPath = CGPathCreateMutable();
        CGPathAddRects( mrClippingPath, NULL, &maClippingRects[0], maClippingRects.size() );
    }
    if( mpView && mbShown )
        [mpView setNeedsDisplay: YES];
    if( mpWindow )
    {
        [mpWindow setOpaque: (mrClippingPath != NULL) ? NO : YES];
        [mpWindow setBackgroundColor: [NSColor clearColor]];
        // shadow is invalidated when view gets drawn again
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
