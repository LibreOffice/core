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

#include "ios/saldata.hxx"
#include "ios/salgdi.h"
#include "ios/salframe.h"
#include "ios/salmenu.h"
#include "ios/saltimer.h"
#include "ios/salinst.h"
#include "ios/salframeview.h"

#include "salwtype.hxx"

using namespace std;

// =======================================================================

IosSalFrame* IosSalFrame::s_pCaptureFrame = NULL;

// =======================================================================

IosSalFrame::IosSalFrame( SalFrame* pParent, sal_uLong salFrameStyle ) :
    mpWindow(nil),
    mpView(nil),
    mpGraphics(NULL),
    mpParent(NULL),
    mnMinWidth(0),
    mnMinHeight(0),
    mnMaxWidth(0),
    mnMaxHeight(0),
    mbGraphics(false),
    mbShown(false),
    mbInitShow(true),
    mbPresentation( false ),
    mnStyle( salFrameStyle ),
    mnStyleMask( 0 ),
    mnLastEventTime( 0 ),
    mnLastModifierFlags( 0 ),
    mpMenu( NULL ),
    mnExtStyle( 0 ),
    mePointerStyle( POINTER_ARROW ),
    mrClippingPath( 0 ),
    mnICOptions( 0 )
{
    maSysData.nSize     = sizeof( SystemEnvData );

    mpParent = dynamic_cast<IosSalFrame*>(pParent);

    initWindowAndView();

    SalData* pSalData = GetSalData();
    pSalData->maFrames.push_front( this );
    pSalData->maFrameCheck.insert( this );
}

// -----------------------------------------------------------------------

IosSalFrame::~IosSalFrame()
{
    // cleanup clipping stuff
    ResetClipRegion();

    SalData* pSalData = GetSalData();
    pSalData->maFrames.remove( this );
    pSalData->maFrameCheck.erase( this );

    DBG_ASSERT( this != s_pCaptureFrame, "capture frame destroyed" );
    if( this == s_pCaptureFrame )
        s_pCaptureFrame = NULL;

    delete mpGraphics;

    if ( mpView ) {
        [mpView release];
    }
    if ( mpWindow )
        [mpWindow release];
}

// -----------------------------------------------------------------------

void IosSalFrame::initWindowAndView()
{
    // initialize mirroring parameters
    // FIXME: screens changing
    UIScreen * pScreen = [mpWindow screen];
    if( pScreen == nil )
        pScreen = [UIScreen mainScreen];
    maScreenRect = [pScreen applicationFrame];

    // calculate some default geometry
    CGRect aVisibleRect = [pScreen applicationFrame];
    CocoaTouchToVCL( aVisibleRect );

    maGeometry.nX = static_cast<int>(aVisibleRect.origin.x + aVisibleRect.size.width / 10);
    maGeometry.nY = static_cast<int>(aVisibleRect.origin.y + aVisibleRect.size.height / 10);
    maGeometry.nWidth = static_cast<unsigned int>(aVisibleRect.size.width * 0.8);
    maGeometry.nHeight = static_cast<unsigned int>(aVisibleRect.size.height * 0.8);

    // calculate style mask
    if( (mnStyle & SAL_FRAME_STYLE_FLOAT) ||
        (mnStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        ;
    else if( mnStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        // make default window "maximized"
        maGeometry.nX = static_cast<int>(aVisibleRect.origin.x);
        maGeometry.nY = static_cast<int>(aVisibleRect.origin.y);
        maGeometry.nWidth = static_cast<int>(aVisibleRect.size.width);
        maGeometry.nHeight = static_cast<int>(aVisibleRect.size.height);
    }
    else
    {
        if( (mnStyle & SAL_FRAME_STYLE_MOVEABLE) )
        {
        }
    }

    mpWindow = [[SalFrameWindow alloc] initWithSalFrame: this];
    mpView = [[SalFrameView alloc] initWithSalFrame: this];

    maSysData.pView = mpView;

    UpdateFrameGeometry();
}

// -----------------------------------------------------------------------

void IosSalFrame::CocoaTouchToVCL( CGRect& io_rRect, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rRect.origin.y = maScreenRect.size.height - (io_rRect.origin.y+io_rRect.size.height);
    else
        io_rRect.origin.y = maGeometry.nHeight - (io_rRect.origin.y+io_rRect.size.height);
}

void IosSalFrame::VCLToCocoaTouch( CGRect& io_rRect, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rRect.origin.y = maScreenRect.size.height - (io_rRect.origin.y+io_rRect.size.height);
    else
        io_rRect.origin.y = maGeometry.nHeight - (io_rRect.origin.y+io_rRect.size.height);
}

void IosSalFrame::CocoaTouchToVCL( CGPoint& io_rPoint, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rPoint.y = maScreenRect.size.height - io_rPoint.y;
    else
        io_rPoint.y = maGeometry.nHeight - io_rPoint.y;
}

void IosSalFrame::VCLToCocoaTouch( CGPoint& io_rPoint, bool bRelativeToScreen )
{
    if( bRelativeToScreen )
        io_rPoint.y = maScreenRect.size.height - io_rPoint.y;
    else
        io_rPoint.y = maGeometry.nHeight - io_rPoint.y;
}

// -----------------------------------------------------------------------

void IosSalFrame::screenParametersChanged()
{
    UpdateFrameGeometry();

    if( mpGraphics )
        mpGraphics->updateResolution();
    CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
}

// -----------------------------------------------------------------------

SalGraphics* IosSalFrame::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( !mpGraphics )
    {
        mpGraphics = new IosSalGraphics;
        mpGraphics->SetWindowGraphics( this );
    }

    mbGraphics = TRUE;
    return mpGraphics;
}

// -----------------------------------------------------------------------

void IosSalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    (void)pGraphics;
    DBG_ASSERT( pGraphics == mpGraphics, "graphics released on wrong frame" );
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

sal_Bool IosSalFrame::PostEvent( void *pData )
{
    GetSalData()->mpFirstInstance->PostUserEvent( this, SALEVENT_USEREVENT, pData );
    return TRUE;
}

// -----------------------------------------------------------------------
void IosSalFrame::SetTitle(const rtl::OUString& /* rTitle */)
{
}

// -----------------------------------------------------------------------

void IosSalFrame::SetIcon( sal_uInt16 )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::SetRepresentedURL( const rtl::OUString& /* i_rDocURL */ )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::initShow()
{
    mbInitShow = false;
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
}

void IosSalFrame::SendPaintEvent( const Rectangle* pRect )
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

void IosSalFrame::Show(sal_Bool bVisible, sal_Bool bNoActivate)
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

        if( !bNoActivate )
            [mpWindow makeKeyAndVisible];
#if 0 // ???
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
                [mpParent->mpWindow addChildWindow: mpWindow];
            }
        }

        if( mbPresentation )
            [mpWindow makeMainWindow];
#endif
    }
    else
    {
#if 0 // ???
        if( mpParent && [mpWindow parentWindow] == mpParent->mpWindow )
            [mpParent->mpWindow removeChildWindow: mpWindow];
#endif
    }
}

// -----------------------------------------------------------------------

void IosSalFrame::Enable( sal_Bool )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::SetMinClientSize( long nWidth, long nHeight )
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

#if 0 // ???
        CGSize aSize = { nWidth, nHeight };
        // Size of full window (content+structure) although we only
        // have the client size in arguments
        [mpWindow setMinSize: aSize];
#endif
    }
}

// -----------------------------------------------------------------------

void IosSalFrame::SetMaxClientSize( long nWidth, long nHeight )
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

#if 0 // ???
        CGSize aSize = { nWidth, nHeight };
        // Size of full window (content+structure) although we only
        // have the client size in arguments
        [mpWindow setMaxSize: aSize];
#endif
    }
}

// -----------------------------------------------------------------------

void IosSalFrame::SetClientSize( long /*nWidth*/, long /*nHeight*/ )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( mpWindow )
    {
#if 0 // ???
        CGSize aSize = { nWidth, nHeight };
        [mpWindow setContentSize: aSize];
#endif
        UpdateFrameGeometry();
        if( mbShown )
            // trigger filling our backbuffer
            SendPaintEvent();
    }
}

// -----------------------------------------------------------------------

void IosSalFrame::GetClientSize( long& rWidth, long& rHeight )
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

void IosSalFrame::SetWindowState( const SalFrameState* pState )
{
    // ???

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
    }
}

// -----------------------------------------------------------------------

sal_Bool IosSalFrame::GetWindowState( SalFrameState* pState )
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

#if 0 // ???
    CGRect aStateRect = [mpWindow frame];
    aStateRect = [UIWindow contentRectForFrameRect: aStateRect styleMask: mnStyleMask];
    CocoaTouchToVCL( aStateRect );
    pState->mnX         = long(aStateRect.origin.x);
    pState->mnY         = long(aStateRect.origin.y);
    pState->mnWidth     = long(aStateRect.size.width);
    pState->mnHeight    = long(aStateRect.size.height);
#endif
    pState->mnState = SAL_FRAMESTATE_MAXIMIZED;

    return TRUE;
}

// -----------------------------------------------------------------------

void IosSalFrame::SetScreenNumber(unsigned int /*nScreen*/)
{
    // ???
}

void IosSalFrame::SetApplicationID( const rtl::OUString &/*rApplicationID*/ )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::ShowFullScreen( sal_Bool /*bFullScreen*/, sal_Int32 /*nDisplay*/ )
{
    // ???
}

// -----------------------------------------------------------------------

void IosSalFrame::StartPresentation( sal_Bool /*bStart*/ )
{
    if ( !mpWindow )
        return;

    // ???
}

// -----------------------------------------------------------------------

void IosSalFrame::SetAlwaysOnTop( sal_Bool )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::ToTop(sal_uInt16 /*nFlags*/)
{
    if ( !mpWindow )
        return;

    // ???
}

// -----------------------------------------------------------------------

void IosSalFrame::SetPointer( PointerStyle /*ePointerStyle*/ )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::SetPointerPos( long /* nX */ , long /* nY */ )
{
}

// -----------------------------------------------------------------------

void IosSalFrame::Flush( void )
{
    // ???
}

// -----------------------------------------------------------------------

void IosSalFrame::Flush( const Rectangle& /*rRect*/ )
{
    // ???
}

// -----------------------------------------------------------------------

void IosSalFrame::Sync()
{
    // ???
}

// -----------------------------------------------------------------------

void IosSalFrame::SetInputContext( SalInputContext* pContext )
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

void IosSalFrame::EndExtTextInput( sal_uInt16 )
{
}

// -----------------------------------------------------------------------

rtl::OUString IosSalFrame::GetKeyName( sal_uInt16 nKeyCode )
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

void IosSalFrame::getResolution( long& o_rDPIX, long& o_rDPIY )
{
    if( ! mpGraphics )
    {
        GetGraphics();
        ReleaseGraphics( mpGraphics );
    }
    mpGraphics->GetResolution( o_rDPIX, o_rDPIY );
}

void IosSalFrame::UpdateSettings( AllSettings& rSettings )
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

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

    aStyleSettings.SetToolbarIconSize( nDPIY > 160 ? STYLE_TOOLBAR_ICONSIZE_LARGE : STYLE_TOOLBAR_ICONSIZE_SMALL );

    aStyleSettings.SetCursorBlinkTime( 500 );

    // no mnemonics on iOs
    aStyleSettings.SetOptions( aStyleSettings.GetOptions() | STYLE_OPTION_NOMNEMONICS );

    // images in menus false for iOS
    aStyleSettings.SetPreferredUseImagesInMenus( false );
    aStyleSettings.SetHideDisabledMenuItems( sal_True );
    aStyleSettings.SetAcceleratorsInContextMenus( sal_False );

    rSettings.SetStyleSettings( aStyleSettings );
}

// -----------------------------------------------------------------------

const SystemEnvData* IosSalFrame::GetSystemData() const
{
    return &maSysData;
}

// -----------------------------------------------------------------------

void IosSalFrame::SetPosSize(long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/, sal_uInt16 nFlags)
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    sal_uInt16 nEvent = 0;

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

#if 0 // ???
    CGRect aFrameRect = [mpWindow frame];
    CGRect aContentRect = [NSWindow contentRectForFrameRect: aFrameRect styleMask: mnStyleMask];

    // position is always relative to parent frame
    CGRect aParentContentRect;

    if( mpParent )
    {
        if( Application::GetSettings().GetLayoutRTL() )
        {
            if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
                nX = mpParent->maGeometry.nWidth - nWidth-1 - nX;
            else
                nX = mpParent->maGeometry.nWidth - static_cast<long int>( aContentRect.size.width-1) - nX;
        }
        CGRect aParentFrameRect = [mpParent->mpWindow frame];
        aParentContentRect = [NSWindow contentRectForFrameRect: aParentFrameRect styleMask: mpParent->mnStyleMask];
    }
    else
        aParentContentRect = maScreenRect; // use screen if no parent

    CocoaTouchToVCL( aContentRect );
    CocoaTouchToVCL( aParentContentRect );

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

    VCLToCocoaTouch( aContentRect );

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
#endif
}

void IosSalFrame::GetWorkArea( Rectangle& rRect )
{
    if ( !mpWindow )
        return;

    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    UIScreen* pScreen = [mpWindow screen];
    if( pScreen ==  nil )
        pScreen = [UIScreen mainScreen];
    CGRect aRect = [pScreen applicationFrame];
    CocoaTouchToVCL( aRect );
    rRect.nLeft     = static_cast<long>(aRect.origin.x);
    rRect.nTop      = static_cast<long>(aRect.origin.y);
    rRect.nRight    = static_cast<long>(aRect.origin.x + aRect.size.width - 1);
    rRect.nBottom   = static_cast<long>(aRect.origin.y + aRect.size.height - 1);
}

SalPointerState IosSalFrame::GetPointerState()
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    SalPointerState state;
    state.mnState = 0;

    // ???

    return state;
}

SalFrame::SalIndicatorState IosSalFrame::GetIndicatorState()
{
    SalIndicatorState aState;
    aState.mnState = 0;
    return aState;
}

void IosSalFrame::SimulateKeyPress( sal_uInt16 /*nKeyCode*/ )
{
}

bool IosSalFrame::SetPluginParent( SystemParentData* )
{
    // plugin parent may be killed unexpectedly by
    // plugging process;

    //TODO: implement
    return sal_False;
}

sal_Bool IosSalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    // not supported yet
    return FALSE;
}

LanguageType IosSalFrame::GetInputLanguage()
{
    //TODO: implement
    return LANGUAGE_DONTKNOW;
}

void IosSalFrame::DrawMenuBar()
{
}

void IosSalFrame::SetMenu( SalMenu* /*pSalMenu*/ )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // ???
}

void IosSalFrame::SetExtendedFrameStyle( SalExtStyle /*nStyle*/ )
{
    // ???
}

void IosSalFrame::SetBackgroundBitmap( SalBitmap* )
{
    //TODO: implement
}

SalFrame* IosSalFrame::GetParent() const
{
    return mpParent;
}

void IosSalFrame::SetParent( SalFrame* pNewParent )
{
    bool bShown = mbShown;
    // remove from child list
    Show( FALSE );
    mpParent = (IosSalFrame*)pNewParent;
    // insert to correct parent and paint
    Show( bShown );
}

void IosSalFrame::UpdateFrameGeometry()
{
    if ( !mpWindow )
    {
        return;
    }

    // keep in mind that view and window coordinates are lower left
    // whereas vcl's are upper left

#if 0 // ???
    // update screen rect
    NSScreen * pScreen = [mpWindow screen];
    if( pScreen )
    {
        maScreenRect = [pScreen frame];
        NSArray* pScreens = [NSScreen screens];
        if( pScreens )
            maGeometry.nDisplayScreenNumber = [pScreens indexOfObject: pScreen];
    }

    CGRect aFrameRect = [mpWindow frame];

    CGRect aContentRect = [NSWindow contentRectForFrameRect: aFrameRect styleMask: mnStyleMask];

    // release old track rect
    [mpView removeTrackingRect: mnTrackingRectTag];
    // install the new track rect
    CGRect aTrackRect = { { 0, 0 }, aContentRect.size };
    mnTrackingRectTag = [mpView addTrackingRect: aTrackRect owner: mpView userData: nil assumeInside: NO];

    // convert to vcl convention
    CocoaTouchToVCL( aFrameRect );
    CocoaTouchToVCL( aContentRect );

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
#endif
}

// -----------------------------------------------------------------------

void IosSalFrame::CaptureMouse( sal_Bool bCapture )
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

void IosSalFrame::ResetClipRegion()
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

    if( mpWindow )
    {
        [mpWindow setOpaque: YES];
    }
}

void IosSalFrame::BeginSetClipRegion( sal_uLong nRects )
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

void IosSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    if( nWidth && nHeight )
    {
        CGRect aRect = { { nX, nY }, { nWidth, nHeight } };
        VCLToCocoaTouch( aRect, false );
        maClippingRects.push_back( CGRectMake(aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height) );
    }
}

void IosSalFrame::EndSetClipRegion()
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
    if( mpWindow )
    {
        [mpWindow setOpaque: (mrClippingPath != NULL) ? NO : YES];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
