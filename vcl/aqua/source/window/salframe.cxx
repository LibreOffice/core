/*n***********************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.54 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:49:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string>

#include "saldata.hxx"
#include "salgdi.h"
#include "salframe.h"
#include "salmenu.h"
#include "saltimer.h"
#include "salinst.h"
#include "vcl/salwtype.hxx"
#include "vcl/window.hxx"

#include "premac.h"
// needed for theming
// FIXME: move theming code to salnativewidgets.cxx
#include <Carbon/Carbon.h>
#include "postmac.h"

#include "boost/assert.hpp"
#include "vcl/svapp.hxx"
#include "rtl/ustrbuf.hxx"

using namespace std;

// =======================================================================

AquaSalFrame* AquaSalFrame::s_pCaptureFrame = NULL;


// =======================================================================

AquaSalFrame::AquaSalFrame( SalFrame* pParent, ULONG salFrameStyle ) :
    mpWindow(nil),
    mpView(nil),
    mpDockMenuEntry(nil),
    mpGraphics(NULL),
    mpParent(NULL),
    mnMinWidth(0),
    mnMinHeight(0),
    mnMaxWidth(0),
    mnMaxHeight(0),
    mbGraphics(FALSE),
    mbShown(false),
    mbInitShow(true),
    mbPositioned(false),
    mbSized(false),
    mnStyle( salFrameStyle ),
    mnStyleMask( 0 ),
    mnLastEventTime( 0 ),
    mnLastModifierFlags( 0 ),
    mpMenu( NULL ),
    mnExtStyle( 0 ),
    mePointerStyle( POINTER_ARROW ),
    mnTrackingRectTag( 0 )
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
    SalData* pSalData = GetSalData();
    pSalData->maFrames.remove( this );
    pSalData->maFrameCheck.erase( this );

    DBG_ASSERT( this != s_pCaptureFrame, "capture frame destroyed" );
    if( this == s_pCaptureFrame )
        s_pCaptureFrame = NULL;

    if ( mpGraphics )
        delete mpGraphics;

    if( mpDockMenuEntry )
        // life cycle comment: the menu has ownership of the item, so no release
        [AquaSalInstance::GetDynamicDockMenu() removeItem: mpDockMenuEntry];
    if( mpView )
        [mpView release];
    if (mpWindow)
        [mpWindow release];
}

// -----------------------------------------------------------------------

void AquaSalFrame::initWindowAndView()
{
    // initialize mirroring parameters
    // FIXME: multiple screens, screens changing
    maScreenRect = [[NSScreen mainScreen] frame];

    // calculate some default geometry
    NSRect aVisibleRect = [[NSScreen mainScreen] visibleFrame];
    CocoaToVCL( aVisibleRect );

    maGeometry.nX = aVisibleRect.origin.x + aVisibleRect.size.width / 10;
    maGeometry.nY = aVisibleRect.origin.y + aVisibleRect.size.height / 10;
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
        maGeometry.nX = aVisibleRect.origin.x;
        maGeometry.nY = aVisibleRect.origin.y;
        maGeometry.nWidth = aVisibleRect.size.width;
        maGeometry.nHeight = aVisibleRect.size.height;
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

    mpWindow = [[SalFrameWindow alloc] initWithSalFrame: this];
    mpView = [[SalFrameView alloc] initWithSalFrame: this];
    [mpWindow setContentView: mpView];
    if( (mnStyle & SAL_FRAME_STYLE_TOOLTIP) )
        [mpWindow setIgnoresMouseEvents: YES];
    else
        [mpWindow setAcceptsMouseMovedEvents: YES];
    [mpWindow setHasShadow: YES];
    [mpWindow setDelegate: mpWindow];

    NSRect aRect = { { 0,0 }, { maGeometry.nWidth, maGeometry.nHeight } };
    mnTrackingRectTag = [mpView addTrackingRect: aRect owner: mpView userData: nil assumeInside: NO];

    maSysData.pView = mpView;

    UpdateFrameGeometry();
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
    DBG_ASSERT( pGraphics == mpGraphics, "graphics released on wrong frame" );
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL AquaSalFrame::PostEvent( void *pData )
{
    GetSalData()->mpFirstInstance->PostUserEvent( this, SALEVENT_USEREVENT, pData );
    return TRUE;
}

// -----------------------------------------------------------------------
void AquaSalFrame::SetTitle(const XubString& rTitle)
{
    NSString* pTitle = CreateNSString( rTitle );
    [mpWindow setTitle: pTitle];

    // create an entry in the dock menu
    const ULONG nAppWindowStyle = (SAL_FRAME_STYLE_CLOSEABLE | SAL_FRAME_STYLE_MOVEABLE);
    if( mpParent == NULL &&
        (mnStyle & nAppWindowStyle) == nAppWindowStyle )
    {
        if( mpDockMenuEntry == NULL )
        {
            NSMenu* pDock = AquaSalInstance::GetDynamicDockMenu();
            mpDockMenuEntry = [pDock insertItemWithTitle: pTitle
                                     action: @selector(dockMenuItemTriggered:)
                                     keyEquivalent: @""
                                     atIndex: [pDock numberOfItems]];
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

void AquaSalFrame::SetIcon( USHORT nIcon )
{
    AquaLog( ">*>_> %s\n",__func__);
}

// -----------------------------------------------------------------------

void AquaSalFrame::initShow()
{
    mbInitShow = false;
    if( ! mbPositioned )
    {
        Rectangle aScreenRect;
        GetWorkArea( aScreenRect );
        if( mpParent ) // center relative to parent
        {
            // center on parent
            long nNewX = mpParent->maGeometry.nX + (mpParent->maGeometry.nWidth - maGeometry.nWidth)/2;
            if( nNewX < aScreenRect.Left() )
                nNewX = aScreenRect.Left();
            if( long(nNewX + maGeometry.nWidth) > aScreenRect.Right() )
                nNewX = aScreenRect.Right() - maGeometry.nWidth-1;
            long nNewY = mpParent->maGeometry.nY + (mpParent->maGeometry.nHeight - maGeometry.nHeight)/2;
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

void AquaSalFrame::Show(BOOL bVisible, BOOL bNoActivate)
{
    AquaLog( ">*>_> %s(%s) %p (%ldx%ld)\n",__func__, bVisible ? "true" : "false", this, maGeometry.nWidth, maGeometry.nHeight );

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
            [mpParent->mpWindow addChildWindow: mpWindow ordered: NSWindowAbove];
    }
    else
    {
        if( mpParent )
            [mpParent->mpWindow removeChildWindow: mpWindow];

        [mpWindow orderOut: NSApp];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::Enable( BOOL bEnable )
{
    AquaLog( ">*>_> %s\n",__func__);
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    AquaLog( ">*>_> %s (nWidth=%ld, nHeight=%ld)\n", __func__, nWidth, nHeight);
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
    AquaLog( ">*>_> %s (nWidth=%ld, nHeight=%ld)\n", __func__, nWidth, nHeight);
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
    AquaLog( ">*>_> %s\n",__func__);
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
    AquaLog( ">*>_> %s\n",__func__);
    rWidth  = mbShown ? maGeometry.nWidth : 0;
    rHeight = mbShown ? maGeometry.nHeight : 0;
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetWindowState( const SalFrameState* pState )
{
    AquaLog( ">*>_> %s\n",__func__);

    // set normal state
    NSRect aStateRect = [mpWindow frame];
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
    [mpWindow setFrame: aStateRect display: FALSE];

    // FIXME: HTH maximized state ?
    if( mbShown )
        // trigger filling our backbuffer
        SendPaintEvent();

    USHORT nEvent = 0;
    if( pState->mnMask & (SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_X) )
    {
        mbPositioned = true;
        nEvent = SALEVENT_MOVE;
    }

    if( pState->mnMask & (SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT) )
    {
        mbSized = true;
        nEvent = (nEvent == SALEVENT_MOVE) ? SALEVENT_MOVERESIZE : SALEVENT_RESIZE;
    }
    if( nEvent )
        CallCallback( nEvent, NULL );
}

// -----------------------------------------------------------------------

BOOL AquaSalFrame::GetWindowState( SalFrameState* pState )
{
    AquaLog( ">*>_> %s\n",__func__);

    pState->mnMask = SAL_FRAMESTATE_MASK_X                 |
                     SAL_FRAMESTATE_MASK_Y                 |
                     SAL_FRAMESTATE_MASK_WIDTH             |
                     SAL_FRAMESTATE_MASK_HEIGHT            |
                     #if 0
                     SAL_FRAMESTATE_MASK_MAXIMIZED_X       |
                     SAL_FRAMESTATE_MASK_MAXIMIZED_Y       |
                     SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH   |
                     SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT  |
                     #endif
                     SAL_FRAMESTATE_MASK_STATE;

    NSRect aStateRect = [mpWindow frame];
    CocoaToVCL( aStateRect );
    pState->mnX         = long(aStateRect.origin.x);
    pState->mnY         = long(aStateRect.origin.y);
    pState->mnWidth     = long(aStateRect.size.width);
    pState->mnHeight    = long(aStateRect.size.height);

    // FIXME: HTH maximized state ?

    if( [mpWindow isMiniaturized] )
        pState->mnState = SAL_FRAMESTATE_MINIMIZED;
    else if( ! [mpWindow isZoomed] )
        pState->mnState = SAL_FRAMESTATE_NORMAL;
    else
        pState->mnState = SAL_FRAMESTATE_MAXIMIZED;

    return TRUE;
}

// -----------------------------------------------------------------------

void AquaSalFrame::ShowFullScreen( BOOL bFullScreen, sal_Int32 /* nDisplay */ )
{
    AquaLog( ">*>_> %s\n",__func__);

    if( mbFullScreen == bFullScreen )
        return;

    mbFullScreen = bFullScreen;
    if( bFullScreen )
    {
        NSRect aNewContentRect = maScreenRect;

        // hide the dock and the menubar
        [NSMenu setMenuBarVisible:NO];

        maFullScreenRect = [mpWindow frame];
        [mpWindow setFrame: [NSWindow frameRectForContentRect: aNewContentRect styleMask: mnStyleMask] display: YES];

        UpdateFrameGeometry();

        if( mbShown )
            CallCallback( SALEVENT_MOVERESIZE, NULL );
    }
    else
    {
        [mpWindow setFrame: maFullScreenRect display: YES];

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

void AquaSalFrame::StartPresentation( BOOL bStart )
{
    AquaLog( ">*>_> %s\n",__func__);
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
    AquaLog( ">*>_> %s\n",__func__);
}

// -----------------------------------------------------------------------

void AquaSalFrame::ToTop(USHORT nFlags)
{
    AquaLog( ">*>_> %s %p %d\n",__func__, this, (int)nFlags);
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
    case POINTER_ESIZE:     pCursor = [NSCursor resizeLeftCursor];      break;
    case POINTER_WSIZE:     pCursor = [NSCursor resizeRightCursor];     break;
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
        DBG_ERROR( "unmapped cursor" );
        pCursor = [NSCursor arrowCursor];
        break;
    }
    return pCursor;
}

void AquaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    if( ePointerStyle >= POINTER_COUNT || ePointerStyle == mePointerStyle )
        return;

    mePointerStyle = ePointerStyle;

    AquaLog( ">*>_> %s\n",__func__);
    [mpWindow invalidateCursorRectsForView: mpView];
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetPointerPos( long nX, long nY )
{
    // FIXME: use Cocoa functions

    AquaLog( ">*>_> %s\n",__func__);

    // FIXME: multiscreen support
    CGPoint aPoint = { nX + maGeometry.nX, nY + maGeometry.nY };
    CGDirectDisplayID mainDisplayID = CGMainDisplayID();
    CGDisplayMoveCursorToPoint( mainDisplayID, aPoint );
}

// -----------------------------------------------------------------------

void AquaSalFrame::Flush()
{
    Sync();
}

// -----------------------------------------------------------------------

void AquaSalFrame::Sync()
{
    AquaLog( ">*>_> %s\n",__func__);
    if( mbGraphics && mpGraphics && mpView )
    {
        [mpView setNeedsDisplay: YES];
        [mpView display];
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetInputContext( SalInputContext* pContext )
{
    AquaLog( ">*>_> %s\n",__func__);

    if (!pContext)
        return;

    if(!(pContext->mnOptions & SAL_INPUTCONTEXT_TEXT))
        return;

    // FIXME: implementation
}

// -----------------------------------------------------------------------

void AquaSalFrame::EndExtTextInput( USHORT nFlags )
{
    AquaLog( ">*>_> %s\n",__func__);
}

// -----------------------------------------------------------------------

XubString AquaSalFrame::GetKeyName( USHORT nKeyCode )
{
    AquaLog( ">*>_> %s\n",__func__);

    static std::map< USHORT, rtl::OUString > aKeyMap;
    if( aKeyMap.empty() )
    {
        USHORT i;
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

    USHORT nUnmodifiedCode = (nKeyCode & KEY_CODE);
    std::map< USHORT, rtl::OUString >::const_iterator it = aKeyMap.find( nUnmodifiedCode );
    if( it != aKeyMap.end() )
    {
        if( (nKeyCode & KEY_SHIFT) != 0 )
            aResult.append( sal_Unicode( 0x21e7 ) );
        if( (nKeyCode & KEY_MOD1) != 0 )
            aResult.append( sal_Unicode( 0x2303 ) );
        // we do not really handle Alt (see below)
        // we map it to MOD5, whichis actually Command
        if( (nKeyCode & (KEY_MOD2|KEY_MOD5)) != 0 )
            aResult.append( sal_Unicode( 0x2318 ) );

        aResult.append( it->second );
    }

    return aResult.makeStringAndClear();
}

// -----------------------------------------------------------------------

XubString AquaSalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
    AquaLog( ">*>_> %s\n",__func__);
    return GetKeyName( nKeyCode );
}

// we have to get Quartz color in preferences, and convert them into sal colors
static short getHighlightColorFromPrefs( Color* pColor )
{
    // default value, when never modified, is light blue ( 0.7098 , 0.8353 , 1.00 )
    BYTE aRed = static_cast<BYTE>( 0.7098*255);
    BYTE aGreen = static_cast<BYTE>( 0.8353*255);
    BYTE aBlue = static_cast<BYTE>( 1.0000*255);

    // get the key in ~/Library/Preferences/.GlobalPreferences.plist
    CFStringRef aPreferedHighlightColor = CFSTR("AppleHighlightColor");
    if (aPreferedHighlightColor)
    {
        CFStringRef aHighLightColor = ( (CFStringRef)CFPreferencesCopyAppValue( aPreferedHighlightColor, kCFPreferencesCurrentApplication ) );
        if (aHighLightColor)
        {
            // create a CFArray containing all the values, as CFString
            CFStringRef aSeparator = CFSTR(" ");
            if (aSeparator)
            {
                CFArrayRef aCFArray = CFStringCreateArrayBySeparatingStrings ( kCFAllocatorDefault, aHighLightColor, aSeparator);
                if (aCFArray)
                {
                    // create an array of double, containing Quartz values
                    double aColorArray[3];
                    short i;
                    for (i=0; i<3 ; i++)
                    {
                        aColorArray[i] = CFStringGetDoubleValue ( (CFStringRef)CFArrayGetValueAtIndex(aCFArray, i) );
                    }

                    // we no longer need The CFArray
                    CFRelease(aCFArray);
                    AquaLog( ">*>_> %s R %f V %f B %f \n",__func__, aColorArray[0],aColorArray[1],aColorArray[2]);

                    // the colors (uff)
                    aRed = static_cast<BYTE>( aColorArray[0]*255);
                    aGreen = static_cast<BYTE>( aColorArray[1]*255);
                    aBlue = static_cast<BYTE>( aColorArray[2]*255);
                }
                CFRelease (aSeparator);
            }
            CFRelease(aHighLightColor);
        }
        CFRelease(aPreferedHighlightColor);
    }
    pColor->SetRed( aRed );
    pColor->SetGreen( aGreen );
    pColor->SetBlue( aBlue );
    return 0;
}

// -----------------------------------------------------------------------

OSStatus AquaGetThemeFont( ThemeFontID eThemeFontID, ScriptCode eScriptCode, Str255 aFontFamilyName, SInt16 *nFontSize, Str255 aFontStyleName ) {

    OSStatus eStatus = ::GetThemeFont( eThemeFontID, eScriptCode, aFontFamilyName, nFontSize, aFontStyleName );

    // #i78983# GetThemeFont doesn't return its corresponding Font
    // with script code for some languages
    switch ( eScriptCode ) {
    case kFontArabicScript:
        strcpy( (char *)&aFontFamilyName[1], "Geeza Pro" );
        aFontFamilyName[0] = strlen( (char *)&aFontFamilyName[1] );
    break;
    // TODO: any other language?
    default:
    break;
    }

    return eStatus;
}

static std::hash_map<rtl::OUString, ScriptCode, rtl::OUStringHash> LocaleScriptMapInit() {
    std::hash_map <rtl::OUString, ScriptCode, rtl::OUStringHash> m;

    // FIXME: the mapping mechanism leads to unsuitable fonts in some languages (e.g. cs)
    // need to check each language and add the correct mapping
    #if 0
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "es" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "de" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "fr" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "it" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ca" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "gl" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "da" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "fi" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "is" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nl" ) )] = kFontRomanScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no" ) )] = kFontRomanScript;
    #endif

    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ja" ) )] = kFontJapaneseScript;

    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "zh_HK" ) )] = kFontTraditionalChineseScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "zh_MO" ) )] = kFontTraditionalChineseScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "zh_SG" ) )] = kFontTraditionalChineseScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "zh_TW" ) )] = kFontTraditionalChineseScript;

    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ko" ) )] = kFontKoreanScript;

    #if 0
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ar" ) )] = kFontArabicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "he" ) )] = kFontHebrewScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "el" ) )] = kFontGreekScript;

    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "be" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "bg" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cv" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ky" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "mk" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ru" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "sh" ) )] = kFontCyrillicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "sr" ) )] = kFontCyrillicScript;

    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hi" ) )] = kFontDevanagariScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "pa" ) )] = kFontGurmukhiScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "gu" ) )] = kFontGujaratiScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "or" ) )] = kFontOriyaScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "bn" ) )] = kFontBengaliScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ta" ) )] = kFontTamilScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "te" ) )] = kFontTeluguScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "kn" ) )] = kFontKannadaScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ml" ) )] = kFontMalayalamScript;
    // kFontSinhaleseScript; // si
    // kFontBurmeseScript; // my
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hm" ) )] = kFontKhmerScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "th" ) )] = kFontThaiScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "lo" ) )] = kFontLaotianScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ka" ) )] = kFontGeorgianScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hy" ) )] = kFontArmenianScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "zh_CN" ) )] = kFontSimpleChineseScript;

    // kFontTibetanScript; // bo
    // kFontMongolianScript; // mm
    // kFontGeezScript; // gez in ISO 639-2
    // kFontSlavicScript; // sla in ISO 639-2
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "bs" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cs" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hr" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "lv" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "mk" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "pl" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "sk" ) )] = kFontSlavicScript;
    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "sl" ) )] = kFontSlavicScript;

    m[rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vi" ) )] = kFontVietnameseScript;
    // kFontExtendedArabicScript; // ??
    // kFontSindhiScript; // sd
    #endif

    return m;
}

static ScriptCode GetScriptCodeForUiLocale()
{
    static std::hash_map<rtl::OUString, ScriptCode, rtl::OUStringHash> aLocaleScriptMap( LocaleScriptMapInit() );

    const com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILocale();
    rtl::OUString aLocaleStr( rLocale.Language );
    ScriptCode eScriptCode = smSystemScript;

    // special case for zh
    if ( aLocaleStr.equalsAscii( "zh" ) )
    {
        rtl::OUString aCountryStr( rLocale.Country );
        rtl::OUStringBuffer aBuf( 8 );
        aBuf.append( aLocaleStr );
        if ( aCountryStr.equalsAscii("TW") ||
             aCountryStr.equalsAscii("HK") ||
             aCountryStr.equalsAscii("MO") ||
             aCountryStr.equalsAscii("SG") )
        {
            aBuf.appendAscii( "_" );
            aBuf.append( aCountryStr );
        }
        else
        {
            aBuf.appendAscii( "_CN" );
        }
        aLocaleStr = aBuf.makeStringAndClear();
    }

    std::hash_map<rtl::OUString, ScriptCode, rtl::OUStringHash>::const_iterator it;
    it = aLocaleScriptMap.find( aLocaleStr );
    if ( it != aLocaleScriptMap.end() )
    {
        eScriptCode = it->second;
    }
    else
    {
        eScriptCode = smSystemScript;
    }

    return eScriptCode;
}

static bool GetSystemFontSetting( ThemeFontID eThemeFontID, int nDPIY, Font* pFont )
{
    // TODO: also allow non-roman font names
    Str255 aFontFamilyName = "";
    Str255 aFontStyleName = "";
    SInt16 nFontPixelSize;
    const rtl_TextEncoding eNameEncoding = RTL_TEXTENCODING_APPLE_ROMAN;

    ScriptCode eScriptCode = GetScriptCodeForUiLocale();
    OSStatus eStatus = AquaGetThemeFont( eThemeFontID, eScriptCode, aFontFamilyName, &nFontPixelSize, aFontStyleName );
    AquaLog("GetSystemFontSetting(id=%d) => err=%d => (\"%s\", \"%s\", h=%d)\n",eThemeFontID,eStatus,aFontFamilyName+1,aFontStyleName+1,nFontPixelSize);
    if( eStatus != noErr )
        return false;

    // Convert font name using a font specific encoding into the roman name
    ATSUFontID fontID;
    ByteCount oNameLen;
    ItemCount oNameIndex;
    char oNameString[512];

    eStatus = ATSUFindFontFromName(&aFontFamilyName[1], aFontFamilyName[0], kFontFamilyName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &fontID);
    if ( eStatus == noErr )
    {
        eStatus = ATSUFindFontName(fontID, kFontFamilyName, kFontNoPlatformCode, kFontRomanScript, kFontNoLanguageCode, sizeof(oNameString), oNameString, &oNameLen, &oNameIndex);
    }
    else
    {
        // See http://lists.apple.com/archives/carbon-dev/2006/Nov/msg00046.html
        eStatus = ATSUFindFontFromName(&aFontFamilyName[1], aFontFamilyName[0], kFontFullName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &fontID);
        if( eStatus != noErr )
            return false;
        eStatus = ATSUFindFontName(fontID, kFontFullName, kFontNoPlatformCode, kFontRomanScript, kFontNoLanguageCode, sizeof(oNameString), oNameString, &oNameLen, &oNameIndex);
    }
    if( eStatus != noErr )
        return false;
    oNameString[oNameLen] = '\0';

    pFont->SetName( String( oNameString, eNameEncoding ) );
    pFont->SetStyleName( String( (const sal_Char*)aFontStyleName+1, aFontStyleName[0], eNameEncoding ) );

    const long nFontPointSize = static_cast<long>(nFontPixelSize * 72.0 / nDPIY);
    pFont->SetHeight( nFontPointSize );
    return true;
}

static void getAppleScrollBarVariant(void)
{
    bool bIsScrollbarDoubleMax = true; // default is DoubleMax

    CFStringRef AppleScrollBarType = CFSTR("AppleScrollBarVariant");
    if (AppleScrollBarType)
    {
        CFStringRef ScrollBarVariant = ((CFStringRef)CFPreferencesCopyAppValue( AppleScrollBarType, kCFPreferencesCurrentApplication ));
        if (ScrollBarVariant)
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
            CFRelease( ScrollBarVariant );
        }
        CFRelease(AppleScrollBarType);
    }

    GetSalData()->mbIsScrollbarDoubleMax = bIsScrollbarDoubleMax;
}

static bool GetSystemFontColor( ThemeTextColor eThemeTextColor, Color* pColor )
{
    RGBColor aRGBColor;
    OSStatus eStatus = ::GetThemeTextColor( eThemeTextColor, 24, true, &aRGBColor );
    AquaLog("GetSystemFontColor(%d) => err=%d => (#%02X%02X%02X)\n",eThemeTextColor,(aRGBColor.red>>8),(aRGBColor.green>>8),(aRGBColor.blue>>8));
    if( eStatus != noErr )
        return false;

    pColor->SetRed( static_cast<UINT8>(aRGBColor.red >> 8U) );
    pColor->SetGreen( static_cast<UINT8>(aRGBColor.green >> 8U) );
    pColor->SetBlue( static_cast<UINT8>(aRGBColor.blue >> 8U) );
    return true;
}

// on OSX-Aqua the style settings are independent of the frame, so it does
// not really belong here. Since the connection to the Appearance_Manager
// is currently done in salnativewidgets.cxx this would be a good place.
// On the other hand VCL's platform independent code currently only asks
// SalFrames for system settings anyway, so moving the code somewhere else
// doesn't make the anything cleaner for now
void AquaSalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();

    // Background Color
    Color aBackgroundColor = Color( 0xEC, 0xEC, 0xEC );
    aStyleSettings.Set3DColors( aBackgroundColor );
    aStyleSettings.SetFaceColor( aBackgroundColor );

    // [FIXME] Dialog Color is the one to modify to complete Aqua Theme on windows
    aStyleSettings.SetDialogColor( aBackgroundColor );
    aStyleSettings.SetLightBorderColor( aBackgroundColor );

    // get the system font settings
    Font aFont = aStyleSettings.GetAppFont();
    GetGraphics();
    long nDPIX = 72, nDPIY = 72;
    mpGraphics->GetResolution( nDPIX, nDPIY );
    if( GetSystemFontSetting( kThemeApplicationFont, nDPIY, &aFont ) )
    {
        // TODO: better mapping of aqua<->ooo font settings
    aStyleSettings.SetAppFont( aFont );
    aStyleSettings.SetHelpFont( aFont );
    aStyleSettings.SetTitleFont( aFont );
    aStyleSettings.SetFloatTitleFont( aFont );

    GetSystemFontSetting( kThemeMenuItemFont, nDPIY, &aFont );
    aStyleSettings.SetMenuFont( aFont );

    GetSystemFontSetting( kThemeToolbarFont, nDPIY, &aFont );
    aStyleSettings.SetToolFont( aFont );

    GetSystemFontSetting( kThemeLabelFont, nDPIY, &aFont );
    aStyleSettings.SetLabelFont( aFont );
    aStyleSettings.SetInfoFont( aFont );
    aStyleSettings.SetRadioCheckFont( aFont );
    aStyleSettings.SetFieldFont( aFont );
    aStyleSettings.SetGroupFont( aFont );
    aStyleSettings.SetIconFont( aFont );

    GetSystemFontSetting( kThemePushButtonFont, nDPIY, &aFont );
    aStyleSettings.SetPushButtonFont( aFont );
    }

    Color aSelectTextBackgroundColor;
    if( getHighlightColorFromPrefs( &aSelectTextBackgroundColor ) == 0 )
    {
        aStyleSettings.SetHighlightTextColor( Color(0x0,0x0,0x0) );
        aStyleSettings.SetHighlightColor( aSelectTextBackgroundColor );
    }

    Color aColor = aStyleSettings.GetMenuTextColor();
    if( GetSystemFontColor( kThemeTextColorRootMenuActive, &aColor ) )
    {
    // TODO: better mapping of aqua<->ooo color settings
    aStyleSettings.SetMenuTextColor( aColor );
    // TODO: ...
    }

    aStyleSettings.SetCursorBlinkTime( 500 );

    // no mnemonics on aqua
    aStyleSettings.SetOptions( aStyleSettings.GetOptions() | STYLE_OPTION_NOMNEMONICS );

    getAppleScrollBarVariant();

    rSettings.SetStyleSettings( aStyleSettings );
}

// -----------------------------------------------------------------------

const SystemEnvData* AquaSalFrame::GetSystemData() const
{
    AquaLog( ">*>_> %s\n",__func__);
    return &maSysData;
}

// -----------------------------------------------------------------------

void AquaSalFrame::Beep( SoundType eSoundType )
{
    SysBeep(1);
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetPosSize(long nX, long nY, long nWidth, long nHeight, USHORT nFlags)
{
    /*
    AquaLog( "SetPosSize: nX: %d nY: %d nWidth: %d nHeight: %d, set_x: %s, set_y: %s, set_width: %s, set_height: %s\n",
            nX, nY, nWidth, nHeight,
            (nFlags&SAL_FRAME_POSSIZE_X) ? "yes" : "no",
            (nFlags&SAL_FRAME_POSSIZE_Y) ? "yes" : "no",
            (nFlags&SAL_FRAME_POSSIZE_WIDTH) ? "yes" : "no",
            (nFlags&SAL_FRAME_POSSIZE_HEIGHT) ? "yes" : "no" );

    AquaLog( "maGeometry: left: %d top: %d right: %d bottom: %d\n",
            maGeometry.nLeftDecoration, maGeometry.nTopDecoration,
            maGeometry.nRightDecoration, maGeometry.nBottomDecoration );
    */
    USHORT nEvent = 0;

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
    [mpWindow setFrame: [NSWindow frameRectForContentRect: aContentRect styleMask: mnStyleMask] display: NO];

    UpdateFrameGeometry();

    if (nEvent)
        CallCallback(nEvent, NULL);

    if( mbShown && bPaint )
        // trigger filling our backbuffer
        SendPaintEvent();

    // now inform the system that the views need to be drawn
    [mpWindow display];
}

void AquaSalFrame::GetWorkArea( Rectangle& rRect )
{
    // FIXME: multiple screens
    NSRect aRect = [[NSScreen mainScreen] visibleFrame];
    CocoaToVCL( aRect );
    rRect.nLeft     = aRect.origin.x;
    rRect.nTop      = aRect.origin.y;
    rRect.nRight    = aRect.origin.x + aRect.size.width - 1;
    rRect.nBottom   = aRect.origin.y + aRect.size.height - 1;
}

SalPointerState AquaSalFrame::GetPointerState()
{
    AquaLog( ">*>_> %s\n",__func__);

    SalPointerState state;

    // get position
    NSPoint aPt = [mpWindow mouseLocationOutsideOfEventStream];
    CocoaToVCL( aPt, false );
    state.maPos = Point( aPt.x, aPt.y );

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
        state.mnState |= KEY_MOD1;
    if( nState & optionKey )
        state.mnState |= KEY_MOD2;

    return state;
}

bool AquaSalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    // plugin parent may be killed unexpectedly by
    // plugging process;

    AquaLog( ">*>_> %s\n",__func__);
    //TODO: implement
    return sal_False;
}

BOOL AquaSalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    AquaLog( ">*>_> %s\n",__func__);
    // not supported yet
    return FALSE;
}

LanguageType AquaSalFrame::GetInputLanguage()
{
    AquaLog( ">*>_> %s\n",__func__);
    //TODO: implement
    return LANGUAGE_DONTKNOW;
}

void AquaSalFrame::DrawMenuBar()
{
}

void AquaSalFrame::SetMenu( SalMenu* pSalMenu )
{
    AquaSalMenu* pMenu = static_cast<AquaSalMenu*>(pSalMenu);
    DBG_ASSERT( ! pMenu || pMenu->mbMenuBar, "setting non menubar on frame" );
    mpMenu = pMenu;
    if( mpMenu  )
        mpMenu->setMainMenu();
}

void AquaSalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    mnExtStyle = nStyle;
}

void AquaSalFrame::SetBackgroundBitmap( SalBitmap* )
{
    AquaLog( ">*>_> %s\n",__func__);
    //TODO: implement
}

SalBitmap* AquaSalFrame::SnapShot()
{
    AquaLog( ">*>_> %s\n",__func__);
    return mpGraphics ? mpGraphics->getBitmap( 0, 0, maGeometry.nWidth, maGeometry.nHeight ) : NULL;
}

SalFrame* AquaSalFrame::GetParent() const
{
    AquaLog( ">*>_> %s\n",__func__);
    return mpParent;
}

void AquaSalFrame::SetParent( SalFrame* pNewParent )
{
    AquaLog( ">*>_> %s\n",__func__);


    bool bShown = mbShown;
    // remove from child list
    Show( FALSE );
    mpParent = (AquaSalFrame*)pNewParent;
    // insert to correct parent and paint
    Show( bShown );
}

void DbgPrintFrameGeometry(Rect fullWndRect, Rect cntRect, Rect titleBarRect, SalFrameGeometry salGeo)
{
    AquaLog( "=======================================\n");
    AquaLog( "Full window rect (l: %d, t: %d, b: %d, r: %d)\n", fullWndRect.left, fullWndRect.top, fullWndRect.bottom, fullWndRect.right);
    AquaLog( "Content rect (l: %d, t: %d, b: %d, r: %d)\n", cntRect.left, cntRect.top, cntRect.bottom, cntRect.right);
    AquaLog( "Title bar rect (l: %d, t: %d, b: %d, r: %d)\n", titleBarRect.left, titleBarRect.top, titleBarRect.bottom, titleBarRect.right);
    AquaLog( "nX: %d, nY: %d, nWidth: %d, nHeight: %d, LeftDeco %d, RightDeco %d, TopDeco %d, BottomDeco %d\n", salGeo.nX, salGeo.nY, salGeo.nWidth, salGeo.nHeight, salGeo.nLeftDecoration, salGeo.nRightDecoration, salGeo.nTopDecoration, salGeo.nBottomDecoration);
    AquaLog( "=======================================\n");
}

void AquaSalFrame::UpdateFrameGeometry()
{
    // keep in mind that view and window coordinates are lower left
    // whereas vcl's are upper left

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

    maGeometry.nX = aContentRect.origin.x;
    maGeometry.nY = aContentRect.origin.y;

    maGeometry.nLeftDecoration = aContentRect.origin.x - aFrameRect.origin.x;
    maGeometry.nRightDecoration = (aFrameRect.origin.x + aFrameRect.size.width) -
                                  (aContentRect.origin.x + aContentRect.size.width);

    maGeometry.nTopDecoration = aContentRect.origin.y - aFrameRect.origin.y;
    maGeometry.nBottomDecoration = (aFrameRect.origin.y + aFrameRect.size.height) -
                                   (aContentRect.origin.y + aContentRect.size.height);

    maGeometry.nWidth = aContentRect.size.width;
    maGeometry.nHeight = aContentRect.size.height;

    //DbgPrintFrameGeometry(fullWindowRect, contentRect, titleBarRect, maGeometry);
}

// -----------------------------------------------------------------------

void AquaSalFrame::CaptureMouse( BOOL bCapture )
{
    AquaLog( ">*>_> %s\n",__func__);

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
    /* FIXME: implement */
}

void AquaSalFrame::BeginSetClipRegion( ULONG nRects )
{
    /* FIXME: implement */
}

void AquaSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    /* FIXME: implement */
}

void AquaSalFrame::EndSetClipRegion()
{
    /* FIXME: implement */
}

static USHORT ImplGetModifierMask( unsigned int nMask, bool bKeyEvent )
{
    USHORT nRet = 0;
    if( (nMask & NSShiftKeyMask) != 0 )
        nRet |= KEY_SHIFT;
    if( (nMask & NSControlKeyMask) != 0 )
        nRet |= KEY_MOD1;
    if( (nMask & NSAlternateKeyMask) != 0 )
        nRet |= bKeyEvent ? KEY_MOD5 : KEY_MOD2;
    if( (nMask & NSCommandKeyMask) != 0 )
        nRet |= KEY_MOD5;
    return nRet;
}

static USHORT ImplMapCharCode( sal_Unicode aCode )
{
    static USHORT aKeyCodeMap[ 128 ] =
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        KEY_BACKSPACE, KEY_TAB, KEY_RETURN, 0, 0, KEY_RETURN, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, KEY_ESCAPE, 0, 0, 0, 0,
        KEY_SPACE, 0, 0, 0, 0, 0, 0, 0,
        0, 0, KEY_MULTIPLY, KEY_ADD, KEY_COMMA, KEY_SUBTRACT, KEY_POINT, KEY_DIVIDE,
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7,
        KEY_8, KEY_9, 0, 0, KEY_LESS, KEY_EQUAL, KEY_GREATER, 0,
        0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
        KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
        KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W,
        KEY_X, KEY_Y, KEY_Z, 0, 0, 0, 0, 0,
        KEY_QUOTELEFT, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
        KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
        KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W,
        KEY_X, KEY_Y, KEY_Z, 0, 0, 0, KEY_TILDE, KEY_DELETE
    };

    static USHORT aFunctionKeyCodeMap[ 128 ] =
    {
        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_F1, KEY_F2, KEY_F3, KEY_F4,
        KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
        KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20,
        KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_F25, KEY_F26, 0, 0,
        0, 0, 0, 0, 0, 0, 0, KEY_INSERT,
        KEY_DELETE, KEY_HOME, 0, KEY_END, KEY_PAGEUP, KEY_PAGEDOWN, 0, 0,
        0, 0, 0, 0, 0, KEY_MENU, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, KEY_UNDO, KEY_REPEAT, KEY_FIND, KEY_HELP, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    USHORT nKeyCode = 0;
    if( aCode < sizeof( aKeyCodeMap) / sizeof( aKeyCodeMap[0] ) )
        nKeyCode = aKeyCodeMap[ aCode ];
    else if( aCode >= 0xf700 && aCode < 0xf780 )
        nKeyCode = aFunctionKeyCodeMap[ aCode - 0xf700 ];
    return nKeyCode;
}

@implementation SalFrameWindow
-(id)initWithSalFrame: (AquaSalFrame*)pFrame
{
    mpFrame = pFrame;
    NSRect aRect = { { pFrame->maGeometry.nX, pFrame->maGeometry.nY },
                     { pFrame->maGeometry.nWidth, pFrame->maGeometry.nHeight } };
    pFrame->VCLToCocoa( aRect );
    return [super initWithContentRect: aRect styleMask: mpFrame->getStyleMask() backing: NSBackingStoreBuffered defer: NO ];
}

-(AquaSalFrame*)getSalFrame
{
    return mpFrame;
}

-(void)windowDidBecomeKey: (NSNotification*)pNotification
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        if( mpFrame->mpMenu )
            mpFrame->mpMenu->setMainMenu();
        #if 0
        // FIXME: we should disable menus while in modal mode
        // however from down here there is currently no reliable way to
        // find out when to do this
        if( (mpFrame->mpParent && mpFrame->mpParent->GetWindow()->IsInModalMode()) )
            AquaSalMenu::enableMainMenu( false );
        #endif
        mpFrame->CallCallback( SALEVENT_GETFOCUS, 0 );
    }
}

-(void)windowDidResignKey: (NSNotification*)pNotification
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        mpFrame->CallCallback(SALEVENT_LOSEFOCUS, 0);
}

-(void)windowDidChangeScreen: (NSNotification*)pNotification
{
    // FIXME: multiscreen
}

-(void)windowDidMove: (NSNotification*)pNotification
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_MOVE, 0 );
    }
}

-(void)windowDidResize: (NSNotification*)pNotification
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_RESIZE, 0 );
    }
}

-(void)windowDidMiniaturize: (NSNotification*)pNotification
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mbShown = false;
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_RESIZE, 0 );
    }
}

-(void)windowDidDeminiaturize: (NSNotification*)pNotification
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mbShown = true;
        mpFrame->UpdateFrameGeometry();
        mpFrame->CallCallback( SALEVENT_RESIZE, 0 );
    }
}

-(MacOSBOOL)windowShouldClose: (NSNotification*)pNotification
{
    YIELD_GUARD;

    MacOSBOOL bRet = YES;
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->CallCallback( SALEVENT_CLOSE, 0 );
        bRet = NO; // application will close the window or not, AppKit shouldn't
    }
    return bRet;
}

-(void)dockMenuItemTriggered: (id)sender
{
    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
        mpFrame->ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );
}

@end

@implementation SalFrameView
-(id)initWithSalFrame: (AquaSalFrame*)pFrame
{
    mpFrame = pFrame;
    if ((self = [super initWithFrame: [NSWindow contentRectForFrameRect: [mpFrame->getWindow() frame] styleMask: mpFrame->mnStyleMask]]) != nil)
    {
        mMarkedRange = NSMakeRange(NSNotFound, 0);
        mSelectedRange = NSMakeRange(NSNotFound, 0);
    }

    return self;
}

-(void)resetCursorRects
{
    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        // FIXME: does this leak the returned NSCursor of getCurrentCursor ?
        NSRect aRect = { { 0, 0 }, { mpFrame->maGeometry.nWidth, mpFrame->maGeometry.nHeight } };
        [self addCursorRect: aRect cursor: mpFrame->getCurrentCursor()];
    }
}

-(MacOSBOOL)acceptsFirstResponder
{
    return YES;
}

-(MacOSBOOL)acceptsFirstMouse: (NSEvent*)pEvent
{
    return YES;
}

-(MacOSBOOL)isOpaque
{
    return YES;
}

-(void)drawRect: (NSRect)aRect
{
    AquaLog( "drawRect\n" );

    YIELD_GUARD;

    if( mpFrame && AquaSalFrame::isAlive( mpFrame ) )
    {
        // FIXME: optimize UpdateWindow wrt to aRect
        if( mpFrame->mpGraphics )
            mpFrame->mpGraphics->UpdateWindow( [NSGraphicsContext currentContext] );
    }
}

-(void)sendMouseEventToFrame: (NSEvent*)pEvent button:(USHORT)nButton eventtype:(USHORT)nEvent
{
    YIELD_GUARD;

    AquaSalFrame* pDispatchFrame = AquaSalFrame::GetCaptureFrame();
    if( pDispatchFrame )
    {
        if( nEvent == SALEVENT_MOUSELEAVE ) // no leave events if mouse is captured
            nEvent = SALEVENT_MOUSEMOVE;
    }
    else
        pDispatchFrame = mpFrame;


    if( pDispatchFrame && AquaSalFrame::isAlive( pDispatchFrame ) )
    {
        pDispatchFrame->mnLastEventTime = static_cast<ULONG>( [pEvent timestamp] * 1000.0 );
        pDispatchFrame->mnLastModifierFlags = [pEvent modifierFlags];

        NSPoint aPt = [NSEvent mouseLocation];
        pDispatchFrame->CocoaToVCL( aPt );

        SalMouseEvent aEvent;
        aEvent.mnTime   = pDispatchFrame->mnLastEventTime;
        aEvent.mnX      = static_cast<long>(aPt.x) - pDispatchFrame->maGeometry.nX;
        aEvent.mnY      = static_cast<long>(aPt.y) - pDispatchFrame->maGeometry.nY;
        aEvent.mnButton = nButton;
        aEvent.mnCode   =  aEvent.mnButton | ImplGetModifierMask( [pEvent modifierFlags], false );

        pDispatchFrame->CallCallback( nEvent, &aEvent );
    }
}

-(void)mouseDown: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:MOUSE_LEFT eventtype:SALEVENT_MOUSEBUTTONDOWN];
}

-(void)mouseDragged: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:MOUSE_LEFT eventtype:SALEVENT_MOUSEMOVE];
}

-(void)mouseUp: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:MOUSE_LEFT eventtype:SALEVENT_MOUSEBUTTONUP];
}

-(void)mouseMoved: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:0 eventtype:SALEVENT_MOUSEMOVE];
}

-(void)mouseEntered: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:0 eventtype:SALEVENT_MOUSEMOVE];
}

-(void)mouseExited: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:0 eventtype:SALEVENT_MOUSELEAVE];
}

-(void)rightMouseDown: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:MOUSE_RIGHT eventtype:SALEVENT_MOUSEBUTTONDOWN];
}

-(void)rightMouseDragged: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:MOUSE_RIGHT eventtype:SALEVENT_MOUSEMOVE];
}

-(void)rightMouseUp: (NSEvent*)pEvent
{
    [self sendMouseEventToFrame:pEvent button:MOUSE_RIGHT eventtype:SALEVENT_MOUSEBUTTONUP];
}

-(void)otherMouseDown: (NSEvent*)pEvent
{
    if( [pEvent buttonNumber] == 2 )
        [self sendMouseEventToFrame:pEvent button:MOUSE_MIDDLE eventtype:SALEVENT_MOUSEBUTTONDOWN];
}

-(void)otherMouseDragged: (NSEvent*)pEvent
{
    if( [pEvent buttonNumber] == 2 )
        [self sendMouseEventToFrame:pEvent button:MOUSE_MIDDLE eventtype:SALEVENT_MOUSEMOVE];
}

-(void)otherMouseUp: (NSEvent*)pEvent
{
    if( [pEvent buttonNumber] == 2 )
        [self sendMouseEventToFrame:pEvent button:MOUSE_MIDDLE eventtype:SALEVENT_MOUSEBUTTONUP];
}

-(void)scrollWheel: (NSEvent*)pEvent
{
    YIELD_GUARD;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mnLastEventTime = static_cast<ULONG>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        NSPoint aPt = [NSEvent mouseLocation];
        mpFrame->CocoaToVCL( aPt );

        SalWheelMouseEvent aEvent;
        aEvent.mnTime   = mpFrame->mnLastEventTime;
        aEvent.mnX      = static_cast<long>(aPt.x) - mpFrame->maGeometry.nX;
        aEvent.mnY      = static_cast<long>(aPt.y) - mpFrame->maGeometry.nY;
        aEvent.mnCode   = ImplGetModifierMask( mpFrame->mnLastModifierFlags, false ); // FIXME: button code

        float dX = [pEvent deltaX];
        float dY = [pEvent deltaY];
        if( dX != 0.0 )
        {
            aEvent.mnDelta = dX;
            aEvent.mnNotchDelta = dX;
            aEvent.mnScrollLines = dX < 0.0 ? -1 : 1;
            aEvent.mbHorz = TRUE;
            mpFrame->CallCallback( SALEVENT_WHEELMOUSE, &aEvent );
        }
        if( dY != 0.0 && AquaSalFrame::isAlive( mpFrame ) )
        {
            aEvent.mnDelta = dY;
            aEvent.mnNotchDelta = dY;
            aEvent.mnScrollLines = dY < 0.0 ? -1 : 1;
            aEvent.mbHorz = FALSE;
            mpFrame->CallCallback( SALEVENT_WHEELMOUSE, &aEvent );
        }
    }
}

-(void)keyDown: (NSEvent*)pEvent
{
    YIELD_GUARD;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpLastEvent = pEvent;
        mbInKeyInput = true;
        mbNeedSpecialKeyHandle = false;
        mbKeyHandled = false;

        mpFrame->mnLastEventTime = static_cast<ULONG>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];

        bool bHandleCommandKey = mpFrame->mpMenu ? false : true;
        if( (mpFrame->mnLastModifierFlags & NSControlKeyMask) ||
            ( (mpFrame->mnLastModifierFlags & NSCommandKeyMask) && bHandleCommandKey )
            )
        {
            // note: checkSpecialCharacters sets mbInKeyInput to false as a side effect
            //       if the event is consumed
            if ( [self checkSpecialCharacters:pEvent] )
                return;
        }

        NSArray* pArray = [NSArray arrayWithObject: pEvent];
        [self interpretKeyEvents: pArray];

        mbInKeyInput = false;
    }
}

-(void)flagsChanged: (NSEvent*)pEvent
{
    YIELD_GUARD;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        mpFrame->mnLastEventTime = static_cast<ULONG>( [pEvent timestamp] * 1000.0 );
        mpFrame->mnLastModifierFlags = [pEvent modifierFlags];
    }
}

-(void)insertText:(id)aString
{
    YIELD_GUARD;

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
            USHORT nKeyCode = 0;
             // aCharCode initializer is safe since aInsertString will at least contain '\0'
            sal_Unicode aCharCode = *aInsertString.getStr();
            // FIXME: will probably break somehow in less than trivial text input mode
            if( nLen == 1 &&
                aCharCode < 0x80 &&
                aCharCode > 0x31 &&
                ( nKeyCode = ImplMapCharCode( aCharCode ) ) != 0
                )
            {
                [self sendKeyInputAndReleaseToFrame: nKeyCode character: aCharCode];
            }
            else
            {
                SalExtTextInputEvent aEvent;
                aEvent.mnTime           = mpFrame->mnLastEventTime;
                aEvent.maText           = aInsertString;
                aEvent.mpTextAttr       = NULL;
                aEvent.mnCursorPos      = aInsertString.getLength();
                aEvent.mnDeltaStart     = 0;
                aEvent.mnCursorFlags    = 0;
                aEvent.mbOnlyCursor     = FALSE;
                mpFrame->CallCallback( SALEVENT_EXTTEXTINPUT, &aEvent );
                if( AquaSalFrame::isAlive( mpFrame ) )
                    mpFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, 0 );
            }
        }
        else
        {
            SalExtTextInputEvent aEvent;
            aEvent.mnTime           = mpFrame->mnLastEventTime;
            aEvent.maText           = String();
            aEvent.mpTextAttr       = NULL;
            aEvent.mnCursorPos      = 0;
            aEvent.mnDeltaStart     = 0;
            aEvent.mnCursorFlags    = 0;
            aEvent.mbOnlyCursor     = FALSE;
            mpFrame->CallCallback( SALEVENT_EXTTEXTINPUT, &aEvent );
            if( AquaSalFrame::isAlive( mpFrame ) )
                mpFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, 0 );

        }
        mbKeyHandled = true;
    }
}

-(void)insertTab: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_TAB character: '\t'];
}

-(void)moveLeft: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_LEFT character: 0];
}

-(void)moveRight: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_RIGHT character: 0];
}

-(void)moveUp: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_UP character: 0];
}

-(void)moveDown: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_DOWN character: 0];
}

-(void)insertNewline: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_RETURN character: '\n'];
}

-(void)deleteBackward: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_BACKSPACE character: '\b'];
}

-(void)deleteForward: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_DELETE character: 0x7f];
}

-(void)cancelOperation: (id)aSender
{
    [self sendKeyInputAndReleaseToFrame: KEY_ESCAPE character: 0x1b];
}

-(void)sendKeyInputAndReleaseToFrame: (USHORT)nKeyCode  character: (sal_Unicode)aChar
{
    YIELD_GUARD;

    if( AquaSalFrame::isAlive( mpFrame ) )
    {
        SalKeyEvent aEvent;
        aEvent.mnTime           = mpFrame->mnLastEventTime;
        aEvent.mnCode           = nKeyCode | ImplGetModifierMask( mpFrame->mnLastModifierFlags, true );
        aEvent.mnCharCode       = aChar;
        aEvent.mnRepeat         = FALSE;
        mpFrame->CallCallback( SALEVENT_KEYINPUT, &aEvent );
        if( AquaSalFrame::isAlive( mpFrame ) )
            mpFrame->CallCallback( SALEVENT_KEYUP, &aEvent );
    }
}


-(MacOSBOOL)checkSpecialCharacters: (NSEvent *)pEvent
{
    // check for special characters
    NSString* pUnmodifiedString = [pEvent charactersIgnoringModifiers];

    if( pUnmodifiedString && [pUnmodifiedString length] == 1 )
    {
        unichar keyChar = [pUnmodifiedString characterAtIndex: 0];
        USHORT nKeyCode = ImplMapCharCode( keyChar );
        bool bUseKeyInput = false;
        if( nKeyCode != 0 )
        {
            // handle function keys, let unmodified cursor keys through to input manager
            if( keyChar >= 0xf704 && keyChar < 0xf900 )
                bUseKeyInput = true;
            else if( keyChar >= 0xf700 && keyChar < 0xf704 &&
                    mpFrame->mnLastModifierFlags != 0 )
                bUseKeyInput = true;
            else if( ((mpFrame->mnLastModifierFlags & ~(NSControlKeyMask | NSAlternateKeyMask | NSShiftKeyMask)) &  NSDeviceIndependentModifierFlagsMask) == 0 &&
                    (mpFrame->mnLastModifierFlags & (NSControlKeyMask | NSAlternateKeyMask)) != 0 )
                bUseKeyInput = true;
            else if( (mpFrame->mnLastModifierFlags & NSCommandKeyMask) && mpFrame->mpMenu == NULL )
                bUseKeyInput = true;
        }
        if( bUseKeyInput )
        {
            [self sendKeyInputAndReleaseToFrame: nKeyCode character: 0];
            mbInKeyInput = false;

            return TRUE;
        }
    }
    return FALSE;
}


// NSTextInput protocol
- (NSArray *)validAttributesForMarkedText
{
    // FIXME
    return [NSArray array];
}

- (MacOSBOOL)hasMarkedText
{
    MacOSBOOL bHasMarkedText;

    bHasMarkedText = ( mMarkedRange.location != NSNotFound ) &&
                     ( mMarkedRange.length != 0 );
    // hack to check keys like "Control-j"
    if( mbInKeyInput )
    {
        mbNeedSpecialKeyHandle = true;
    }
    return bHasMarkedText;
}

- (NSRange)markedRange
{
    return [self hasMarkedText] ? mMarkedRange : NSMakeRange( NSNotFound, 0 );
}

- (NSRange)selectedRange
{
    return mSelectedRange;
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
    AquaLog( ">*>_> %s\n",__func__ );
    AquaLog( "location: %d, length: %d\n", selRange.location, selRange.length );
    AquaLog( "aString = '%@'\n", aString );
    AquaLog( "length %d\n", [aString length] );

    if( ![aString isKindOfClass:[NSAttributedString class]] )
        aString = [[[NSAttributedString alloc] initWithString:aString] autorelease];
    NSRange rangeToReplace = [self hasMarkedText] ? [self markedRange] : [self selectedRange];
    mMarkedRange = NSMakeRange( rangeToReplace.location, [aString length] );
    mSelectedRange = NSMakeRange( rangeToReplace.location + selRange.location, selRange.length );

    int len = [aString length];
    SalExtTextInputEvent aInputEvent;
    aInputEvent.mnTime = mpFrame->mnLastEventTime;
    aInputEvent.mnDeltaStart = 0;
    aInputEvent.mbOnlyCursor = FALSE;
    if( len > 0 ) {
        NSString *pString = [aString string];
        OUString aInsertString( GetOUString( pString ) );
        std::vector<USHORT> aInputFlags = std::vector<USHORT>( std::max( 1, len ), 0 );
        for ( int i = 0; i < len; i++ )
        {
            // FIXME
            aInputFlags[i] = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
        }

        aInputEvent.maText = aInsertString;
        aInputEvent.mnCursorPos = selRange.location;
        aInputEvent.mpTextAttr = &aInputFlags[0];
        mpFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void *)&aInputEvent );
    } else {
        aInputEvent.maText = String();
        aInputEvent.mnCursorPos = 0;
        aInputEvent.mnCursorFlags = 0;
        aInputEvent.mpTextAttr = 0;
        mpFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void *)&aInputEvent );
        mpFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, 0 );
    }
    mbKeyHandled= true;
}

- (void)unmarkText
{
    AquaLog( ">*>_> %s\n", __func__ );

    mSelectedRange = mMarkedRange = NSMakeRange(NSNotFound, 0);
}

- (NSAttributedString *)attributedSubstringFromRange:(NSRange)theRange
{
    AquaLog( ">*>_> %s\n", __func__ );
    AquaLog( "theRange = %d, %d\n", theRange.location, theRange.length);

    // FIXME
    return nil;
}

- (unsigned int)characterIndexForPoint:(NSPoint)thePoint
{
    AquaLog( ">*>_> %s\n",__func__ );

    // FIXME
    return 0;
}

- (long)conversationIdentifier
{
    return (long)self;
}

- (void)doCommandBySelector:(SEL)aSelector
{
    AquaLog( ">*>_> %s\n",__func__ );
    AquaLog( "aSelector %s", aSelector );

    // check for special characters with OOo way
    if ( [self checkSpecialCharacters:mpLastEvent] )
    {
        mbKeyHandled = true;
        return;
    }

    // Cocoa way
    (void)[self performSelector: aSelector];
    mbKeyHandled = true;
}

- (NSRect)firstRectForCharacterRange:(NSRange)theRange
{
    AquaLog( ">*>_> %s\n",__func__ );
    AquaLog( "mSelectedRange = %d, %d", mSelectedRange.location, mSelectedRange.length );
    AquaLog( " theRange = %d, %d\n", theRange.location, theRange.length );

    SalExtTextInputPosEvent aPosEvent;
    mpFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void *)&aPosEvent );

    NSRect rect;

    rect.origin.x = aPosEvent.mnX + mpFrame->maGeometry.nX;
    rect.origin.y = aPosEvent.mnY + mpFrame->maGeometry.nY;
    rect.size.width = aPosEvent.mnWidth;
    rect.size.height = aPosEvent.mnHeight;

    // FIXME how to convert the geometry??
    //rect = [self convertRect:rect toView:nil];
    //rect.origin = [[self window] convertBaseToScreen:rect.origin];
    return rect;
}

@end

