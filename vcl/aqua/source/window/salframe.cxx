/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 16:31:54 $
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

#include <string.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <vcl/salgdi.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif
#ifndef _SV_SALTIMER_H
#include <saltimer.h>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif

#include <aquavclevents.hxx>
#include <vcl/cmdevt.hxx>

#include <string>
#include <boost/assert.hpp>
#include <vcl/svapp.hxx>

#include <rtl/ustrbuf.hxx>

using namespace std;

// =======================================================================

AquaSalFrame::SysWindowEventHandlerData_t     AquaSalFrame::s_aOverlayEvtHandler;
AquaSalFrame*                                 AquaSalFrame::s_pCaptureFrame = NULL;
CarbonWindowRef                               AquaSalFrame::s_rOverlay = 0;


static void ImplSalCalcFullScreenSize( const AquaSalFrame* pFrame, Rect* pSize )
{
    /** FIXME **
     * Implement multiple displays here. That's why a pointer
     * to frame is passed. But pointer not currently used.
     **/
    CGDirectDisplayID mainDisplayID = CGMainDisplayID();
    AquaLog("Display ID %p\n", mainDisplayID);

    CGRect rect;
    rect = CGDisplayBounds( mainDisplayID );
    AquaLog( "Screen resolution: %.0fx%.0f\n", rect.size.width, rect.size.height );

    // Stores current resolution in pSize.
    // Rect made out of ints -> cast required -> CGRect made out of floats.
    pSize->top = 0;
    pSize->left = 0;
    pSize->bottom = static_cast<int>(rect.size.height);
    pSize->right = static_cast<int>(rect.size.width);
}

// =======================================================================

// Returns the free area excluding the Dock (when displayed) and main menubar

static void ImplSalCalcMaxClientSize( const AquaSalFrame* pFrame, Rect* pSize )
{
    GetAvailableWindowPositioningBounds(NULL, pSize);
}

// =======================================================================

AquaSalFrame::AquaSalFrame(SalFrame* pParent, ULONG salFrameStyle, AquaSalInstance* pSalInstance) :
    mrWindow(0),
    mpGraphics(NULL),
    mpParent(NULL),
    mpInst(NULL),
    mnWidth(0),
    mnHeight(0),
    mnMinWidth(0),
    mnMinHeight(0),
    mnMaxWidth(0),
    mnMaxHeight(0),
    mbGraphics(FALSE),
    mpSalInstance(pSalInstance),
    mbShown(false),
    mbInitShow(true),
    mbPositioned(false),
    mbSized(false),
    mnStyle( salFrameStyle ),
    maTsmDocumentId(0),
    mpMenu(NULL)
{
    maSysData.nSize     = sizeof( SystemEnvData );

    mpParent = dynamic_cast<AquaSalFrame*>(pParent);
    CreateNewSystemWindow(mpParent ? mpParent->mrWindow : NULL, salFrameStyle);

    SalData* pSalData = GetSalData();
    pSalData->maFrames.push_front( this );
    pSalData->maFrameCheck.insert( this );
}

// -----------------------------------------------------------------------

AquaSalFrame::~AquaSalFrame()
{
    AquaLog( ">*>_> %s %p\n",__func__, this);

    SalData* pSalData = GetSalData();
    pSalData->maFrames.remove( this );
    pSalData->maFrameCheck.erase( this );

    if ( mpGraphics )
        delete mpGraphics;

    if (mrWindow)
    {
        HideWindow(mrWindow);
        DeinstallAndUnregisterAllEventHandler();
        ReleaseWindow(mrWindow);
    }

    if (maTsmDocumentId)
        DeleteTSMDocument(maTsmDocumentId);

    AquaLog( ">*>_> %s %p end of destructor\n",__func__, this);
}

// -----------------------------------------------------------------------

SalGraphics* AquaSalFrame::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( !mpGraphics )
    {
        CarbonViewRef hView = NULL;

        OSL_ENSURE(mrWindow, "windowless frame !!!");

        // create root control if it does not exist
        // however, this is currently not used
        OSErr errval = GetRootControl( mrWindow, &hView );
        if( errval == errNoRootControl )
            errval = CreateRootControl( mrWindow, &hView );

        if( errval == noErr )
        {
            mpGraphics = new AquaSalGraphics;
            mpGraphics->SetWindowGraphics( hView, mrWindow, true );
        }
        else
            AquaLog( "could not get graphics (get/create root control returned %d)\n", errval);
    }

    mbGraphics = TRUE;
    return mpGraphics;
}

// -----------------------------------------------------------------------

void AquaSalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    AquaLog( ">*>_> %s\n",__func__);
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL AquaSalFrame::ImplPostUserEvent( UInt32 eventKind, void *pData )
{
    //AquaLog( "implpost user event\n");

    // Search for the parent SalFrame that has a native window and
    // use that window to post the event to
    AquaSalFrame *pFrame = this;
    while (!pFrame->mrWindow)
    {
        pFrame = pFrame->mpParent;
        if (!pFrame)
            break;
    }

    bool bret = false;

    if (pFrame)
    {
        EventRef eventRef;
        if (CreateEvent(NULL, cOOoSalUserEventClass, eventKind, 0, kEventAttributeNone, &eventRef) == noErr)
        {
            EventTargetRef eventTargetRef = GetWindowEventTarget(pFrame->mrWindow);
            if (SetEventParameter(eventRef, kEventParamPostTarget, typeEventTargetRef, sizeof(EventTargetRef), &eventTargetRef) == noErr &&
                SetEventParameter(eventRef, cOOoSalEventData, cOOoSalEventParamTypePtr, sizeof(pData), &pData) == noErr &&
                PostEventToQueue(GetMainEventQueue(), eventRef, kEventPriorityStandard) == noErr)
            {
                bret = true;
            }
            ReleaseEvent(eventRef);
        }
    }
    return bret;
}

BOOL AquaSalFrame::PostEvent( void *pData )
{
    AquaLog( ">*>_> %s\n",__func__);
    return ImplPostUserEvent( cOOoSalEventUser, pData );
}

BOOL AquaSalFrame::PostTimerEvent( AquaSalTimer *pTimer )
{
    //AquaLog( ">*>_> %s\n",__func__);
    return ImplPostUserEvent( cOOoSalEventTimer, (void*) pTimer );
}

// -----------------------------------------------------------------------
void AquaSalFrame::SetTitle(const XubString& rTitle)
{
    CFStringRef rStr = CreateCFString( rTitle );
    SetWindowTitleWithCFString(mrWindow, rStr);
    if (rStr)
        CFRelease(rStr);
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

void AquaSalFrame::SendPaintEvent()
{
    SalPaintEvent aPaintEvt( 0, 0, maGeometry.nWidth, maGeometry.nHeight, true );
    CallCallback(SALEVENT_PAINT, &aPaintEvt);
}

// -----------------------------------------------------------------------

void AquaSalFrame::Show(BOOL bVisible, BOOL bNoActivate)
{
    BOOST_ASSERT(mrWindow && "Precondition violation - no window associated with frame yet");

    AquaLog( ">*>_> %s(%s) %p (%ldx%ld)\n",__func__, bVisible ? "true" : "false", this, mnWidth, mnHeight );

    mbShown = bVisible;
    if(bVisible)
    {
        if( mbInitShow )
            initShow();

        // trigger filling our backbuffer
        SendPaintEvent();

        ShowWindow(mrWindow);
        if(!bNoActivate)
            SelectWindow(mrWindow); //ActivateWindow(mrWindow, true);

        // FIXME: create empty menu bar for SalFrame without menu yet!
        if (!mpMenu)
        {
            if (mpParent)             //tooltips need the parent menu, else no menu shows
                mpMenu = mpParent->mpMenu;
            else
                mpMenu = mpSalInstance->CreateMenu(TRUE);
        }

        if (mpMenu)
        {
            AquaSalMenu *pAquaSalMenu = (AquaSalMenu *) mpMenu;
            SetRootMenu(pAquaSalMenu->mrMenuRef);
        }
    }
    else
        HideWindow(mrWindow);
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

    if (mrWindow)
    {
        // Always add the decoration as the dimension concerns only
        // the content rectangle
        nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

        CGSize nMinSize = CGSizeMake(nWidth, nHeight);

        // Size of full window (content+structure) although we only
        // have the client size in arguments
        SetWindowResizeLimits(mrWindow, &nMinSize, NULL);
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    AquaLog( ">*>_> %s (nWidth=%ld, nHeight=%ld)\n", __func__, nWidth, nHeight);
    mnMaxWidth = nWidth;
    mnMaxHeight = nHeight;

    if (mrWindow)
    {
        // Always add the decoration as the dimension concerns only
        // the content rectangle
        nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

        // Carbon windows can't have a size greater than 32767x32767
        if (nWidth>32767) nWidth=32767;
        if (nHeight>32767) nHeight=32767;

        CGSize nMaxSize = CGSizeMake(nWidth, nHeight);

        // Size of full window (content+structure) although we only
        // have the client size in arguments
        SetWindowResizeLimits(mrWindow, NULL, &nMaxSize);
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetClientSize( long nWidth, long nHeight )
{
    AquaLog( ">*>_> %s\n",__func__);
    mnWidth = nWidth;
    mnHeight = nHeight;

    // If this is a native window, resize it
    if ( mrWindow )
    {
        Rect bounds;
        GetWindowBounds( mrWindow, kWindowContentRgn, &bounds );
        if( bounds.right - bounds.left != nWidth ||
            bounds.bottom - bounds.top != nHeight )
        {
            bounds.right = bounds.left + nWidth;
            bounds.bottom = bounds.top + nHeight;
            SetWindowBounds( mrWindow, kWindowContentRgn, &bounds );
            if( mbShown )
                // trigger filling our backbuffer
                SendPaintEvent();
        }
    }
}

// -----------------------------------------------------------------------

void AquaSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    AquaLog( ">*>_> %s\n",__func__);
    rWidth  = mbShown ? mnWidth : 0;
    rHeight = mbShown ? mnHeight : 0;
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetWindowState( const SalFrameState* pState )
{
    AquaLog( ">*>_> %s\n",__func__);

    Rect aStateRect;
    // set normal state
    GetWindowBounds( mrWindow, kWindowContentRgn, &aStateRect );
    if( pState->mnMask & SAL_FRAMESTATE_MASK_X )
        aStateRect.left = short(pState->mnX);
    if( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
        aStateRect.top = short(pState->mnY);
    if( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
        aStateRect.right = aStateRect.left + short(pState->mnWidth);
    else
        aStateRect.right = aStateRect.left + maGeometry.nWidth;
    if( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
        aStateRect.bottom = aStateRect.top + short(pState->mnHeight);
    else
        aStateRect.bottom = aStateRect.top + maGeometry.nHeight;
    SetWindowBounds( mrWindow, kWindowContentRgn, &aStateRect );

    // FIXME: HTH maximized state ?

    if( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
    {
        if( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
            ZoomWindow( mrWindow, inZoomIn, false );
        if( (pState->mnState & SAL_FRAMESTATE_MINIMIZED) )
            CollapseWindow( mrWindow, true );
    }

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

    Rect aStateRect;
    GetWindowBounds( mrWindow, kWindowContentRgn, &aStateRect );
    pState->mnX         = long(aStateRect.left);
    pState->mnY         = long(aStateRect.top);
    pState->mnWidth     = long(aStateRect.right - aStateRect.left);
    pState->mnHeight    = long(aStateRect.bottom - aStateRect.top);

    // FIXME: HTH maximized state ?

    if( IsWindowCollapsed( mrWindow ) || ! IsWindowVisible( mrWindow ) )
        pState->mnState = SAL_FRAMESTATE_MINIMIZED;
    else if( IsWindowInStandardState( mrWindow, NULL, NULL ) )
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
    AquaSalFrame* pFrame = this;
    if( bFullScreen )
    {
        Rect newBounds;
        ImplSalCalcFullScreenSize( pFrame, &newBounds ); // Get new bounds
        GetWindowAttributes( mrWindow, &maFullScreenAttr ); // Save attributes
        ChangeWindowAttributes( mrWindow, kWindowNoAttributes, maFullScreenAttr );
        GetWindowBounds( mrWindow, kWindowContentRgn, &maFullScreenRect );
        SetWindowBounds( mrWindow, kWindowContentRgn, &newBounds );
        SetSystemUIMode( kUIModeAllHidden, kUIOptionAutoShowMenuBar );
        // -> Shows menubar when we move the mouse over it.
    }
    else
    {
        SetWindowBounds( mrWindow, kWindowContentRgn, &maFullScreenRect );
        ChangeWindowAttributes( mrWindow, maFullScreenAttr, kWindowNoAttributes );
        SetSystemUIMode( kUIModeNormal, nil );
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
        if( ! IsWindowVisible( mrWindow ) || IsWindowCollapsed( mrWindow ) )
            return;
    }
    if( ! IsWindowVisible( mrWindow ) )
        ShowWindow( mrWindow );
    if( nFlags & SAL_FRAME_TOTOP_GRABFOCUS )
        SelectWindow( mrWindow );
    else
        BringToFront( mrWindow );
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    AquaLog( ">*>_> %s\n",__func__);


    ThemeCursor aPointer[POINTER_COUNT] = {
        kThemeArrowCursor, //POINTER_ARROW
        0, // POINTER_NULL
        0, // POINTER_WAIT
        kThemeIBeamCursor, //POINTER_TEXT
        kThemeSpinningCursor, // POINTER_HELP
        kThemeCrossCursor, //POINTER_CROSS
        kThemeOpenHandCursor, // POINTER_MOVE
        kThemeResizeUpDownCursor, //kThemeResizeUpCursor, // POINTER_NSIZE
        kThemeResizeUpDownCursor, //kThemeResizeDownCursor, // POINTER_SSIZE
        kThemeResizeLeftRightCursor, //kThemeResizeLeftCursor, // POINTER_WSIZE
        kThemeResizeLeftRightCursor, //kThemeResizeRightCursor // POINTER_ESIZE
        0, // POINTER_NWSIZE
        0, // POINTER_NESIZE
        0, // POINTER_SWSIZE
        0, // POINTER_SESIZE
        kThemeResizeUpDownCursor, // POINTER_WINDOW_NSIZE
        kThemeResizeUpDownCursor, // POINTER_WINDOW_SSIZE
        kThemeResizeLeftRightCursor, // POINTER_WINDOW_WSIZE
        kThemeResizeLeftRightCursor, // POINTER_WINDOW_ESIZE
        0, // POINTER_WINDOW_NWSIZE
        0, // POINTER_WINDOW_NESIZE
        0, // POINTER_WINDOW_SWSIZE
        0, // POINTER_WINDOW_SESIZE
        kThemeResizeLeftRightCursor, // POINTER_HSPLIT
        kThemeResizeUpDownCursor, // POINTER_VSPLIT
        kThemeResizeLeftRightCursor, // POINTER_HSIZEBAR
        kThemeResizeUpDownCursor, // POINTER_VSIZEBAR
        kThemeOpenHandCursor, // POINTER_HAND
        kThemePointingHandCursor, // POINTER_REFHAND
        0, // POINTER_PEN
        0, // POINTER_MAGNIFY
        0, // POINTER_FILL
        0, // POINTER_ROTATE
        0, // POINTER_HSHEAR
        0, // POINTER_VSHEAR
        0, // POINTER_MIRROR
        0, // POINTER_CROOK
        0, // POINTER_CROP
        0, // POINTER_MOVEPOINT
        0, // POINTER_MOVEBEZIERWEIGHT
        0, // POINTER_MOVEDATA
        0, // POINTER_COPYDATA
        0, // POINTER_LINKDATA
        0, // POINTER_MOVEDATALINK
        0, // POINTER_COPYDATALINK
        0, // POINTER_MOVEFILE
        0, // POINTER_COPYFILE
        0, // POINTER_LINKFILE
        0, // POINTER_MOVEFILELINK
        0, // POINTER_COPYFILELINK
        0, // POINTER_MOVEFILES
        0, // POINTER_COPYFILES
        0, // POINTER_NOTALLOWED
        0, // POINTER_DRAW_LINE
        0, // POINTER_DRAW_RECT
        0, // POINTER_DRAW_POLYGON
        0, // POINTER_DRAW_BEZIER
        0, // POINTER_DRAW_ARC
        0, // POINTER_DRAW_PIE
        0, // POINTER_DRAW_CIRCLECUT
        0, // POINTER_DRAW_ELLIPSE
        0, // POINTER_DRAW_FREEHAND
        0, // POINTER_DRAW_CONNECT
        0, // POINTER_DRAW_TEXT
        0, // POINTER_DRAW_CAPTION
        0, // POINTER_CHART
        0, // POINTER_DETECTIVE
        0, // POINTER_PIVOT_COL
        0, // POINTER_PIVOT_ROW
        0, // POINTER_PIVOT_FIELD
        0, // POINTER_CHAIN
        0, // POINTER_CHAIN_NOTALLOWED
        0, // POINTER_TIMEEVENT_MOVE
        0, // POINTER_TIMEEVENT_SIZE
        0, // POINTER_AUTOSCROLL_N
        0, // POINTER_AUTOSCROLL_S
        0, // POINTER_AUTOSCROLL_W
        0, // POINTER_AUTOSCROLL_E
        0, // POINTER_AUTOSCROLL_NW
        0, // POINTER_AUTOSCROLL_NE
        0, // POINTER_AUTOSCROLL_SW
        0, // POINTER_AUTOSCROLL_SE
        0, // POINTER_AUTOSCROLL_NS
        0, // POINTER_AUTOSCROLL_WE
        0, // POINTER_AUTOSCROLL_NSWE
        0, // POINTER_AIRBRUSH
        0, // POINTER_TEXT_VERTICAL
        0, // POINTER_PIVOT_DELETE
// --> FME 2004-07-30 #i32329# Enhanced table selection
        0, // POINTER_TAB_SELECT_S
        0, // POINTER_TAB_SELECT_E
        0, // POINTER_TAB_SELECT_SE
        0, // POINTER_TAB_SELECT_W
        0, // POINTER_TAB_SELECT_SW
// <--
// --> FME 2004-08-16 #i20119# Paintbrush tool
        0 // POINTER_PAINTBRUSH
// <--
    };

#if POINTER_COUNT != 94
#error New Pointer must be defined!
#endif

    SetThemeCursor(aPointer[ePointerStyle]);

}

// -----------------------------------------------------------------------

void AquaSalFrame::SetPointerPos( long nX, long nY )
{
    AquaLog( ">*>_> %s\n",__func__);

    // FIXME: multiscreen support
    CGPoint aPoint = { nX + maGeometry.nX, nY + maGeometry.nY };
    CGDirectDisplayID mainDisplayID = CGMainDisplayID();
    CGDisplayMoveCursorToPoint( mainDisplayID, aPoint );
}

// -----------------------------------------------------------------------

void AquaSalFrame::Flush()
{
    AquaLog( ">*>_> %s\n",__func__);
    if( mbGraphics && mpGraphics )
        mpGraphics->Flush();
}

// -----------------------------------------------------------------------

void AquaSalFrame::Sync()
{
    AquaLog( ">*>_> %s\n",__func__);
    if( mbGraphics && mpGraphics )
        mpGraphics->Flush();
}

// -----------------------------------------------------------------------

void AquaSalFrame::SetInputContext( SalInputContext* pContext )
{
    AquaLog( ">*>_> %s\n",__func__);

    if (!pContext)
        return;

    if(!(pContext->mnOptions & SAL_INPUTCONTEXT_TEXT))
    return;

    // create a new im context
    if (!maTsmDocumentId)
    {
        InterfaceTypeList typeList;
        typeList[0] = kUnicodeDocumentInterfaceType;

        NewTSMDocument(1, typeList, &maTsmDocumentId, (long)pContext);
        AquaLog( "create NewTSMDocument\n");
        ActivateTSM();
    }
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
    return XubString();
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

static bool GetSystemFontSetting( ThemeFontID eThemeFontID, Font* pFont )
{
    // TODO: also allow non-roman font names
    Str255 aFontFamilyName = "";
    Str255 aFontStyleName = "";
    SInt16 nFontSize;
    const rtl_TextEncoding eNameEncoding = RTL_TEXTENCODING_APPLE_ROMAN;

    ScriptCode eScriptCode = GetScriptCodeForUiLocale();
    OSStatus eStatus = AquaGetThemeFont( eThemeFontID, eScriptCode, aFontFamilyName, &nFontSize, aFontStyleName );
    AquaLog("GetSystemFontSetting(%d) => err=%d => (\"%s\", \"%s\", h=%d)\n",eThemeFontID,eStatus,aFontFamilyName+1,aFontStyleName+1,nFontSize);
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
    pFont->SetHeight( nFontSize );
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
    if( GetSystemFontSetting( kThemeApplicationFont, &aFont ) )
    {
        // TODO: better mapping of aqua<->ooo font settings
    aStyleSettings.SetAppFont( aFont );
    aStyleSettings.SetHelpFont( aFont );
    aStyleSettings.SetTitleFont( aFont );
    aStyleSettings.SetFloatTitleFont( aFont );

    GetSystemFontSetting( kThemeMenuItemFont, &aFont );
    aStyleSettings.SetMenuFont( aFont );

    GetSystemFontSetting( kThemeToolbarFont, &aFont );
    aStyleSettings.SetToolFont( aFont );

    GetSystemFontSetting( kThemeLabelFont, &aFont );
    aStyleSettings.SetLabelFont( aFont );
    aStyleSettings.SetInfoFont( aFont );
    aStyleSettings.SetRadioCheckFont( aFont );
    aStyleSettings.SetFieldFont( aFont );
    aStyleSettings.SetGroupFont( aFont );
    aStyleSettings.SetIconFont( aFont );

    GetSystemFontSetting( kThemePushButtonFont, &aFont );
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

    if (IsWindowCollapsed(mrWindow))
        CollapseWindow(mrWindow, false); // expand the window

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

    Rect currentWindowRect;
    GetWindowBounds(mrWindow, kWindowStructureRgn, &currentWindowRect);

    Rect currentContentRect;
    GetWindowBounds(mrWindow, kWindowContentRgn, &currentContentRect);

    // position is always relative to parent frame
    Rect parentContentRect;

    if( mpParent )
        GetWindowBounds(mpParent->mrWindow, kWindowContentRgn, &parentContentRect);
    else
        ImplSalCalcFullScreenSize( this, &parentContentRect); // use screen if no parent

    bool bPaint = false;
    if( (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT)) != 0 )
    {
        if( nWidth != currentContentRect.right - currentContentRect.left ||
            nHeight != currentContentRect.bottom - currentContentRect.top )
            bPaint = true;
    }

    // use old window pos if no new pos requested
    if (!(nFlags & SAL_FRAME_POSSIZE_X))
        nX = currentWindowRect.left;
    else
        nX += parentContentRect.left;

    if (!(nFlags & SAL_FRAME_POSSIZE_Y))
        nY = currentWindowRect.top;
    else
        nY += parentContentRect.top;

    // use old window width if no new width requested
    if (!(nFlags & SAL_FRAME_POSSIZE_WIDTH))
        nWidth = currentContentRect.right - currentContentRect.left;

    // use old window height if no new height requested
    if (!(nFlags & SAL_FRAME_POSSIZE_HEIGHT))
        nHeight = currentContentRect.bottom - currentContentRect.top;

    // --- RTL --- (mirror window pos)
    if( mpParent && Application::GetSettings().GetLayoutRTL() )
    {
        // undo the change above
        if( nFlags & SAL_FRAME_POSSIZE_X )
            nX -= parentContentRect.left;
        // mirror
        nX = parentContentRect.right - nWidth - nX;
    }

    Rect newWindowRect;

    newWindowRect.left = nX;
    newWindowRect.top = nY;

    newWindowRect.right = nX + nWidth;
    // always add the decoration as nWidth concerns only the content rect
    newWindowRect.right += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;

    newWindowRect.bottom = nY + nHeight;
    // always add the decoration as nHeight concerns only the content rect
    newWindowRect.bottom += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;



    /*
    AquaLog( "SetPosSize: Old rect (x: %d, y: %d, w: %d, h: %d)\n",
            currentWindowRect.left, currentWindowRect.top, currentWindowRect.right - currentWindowRect.left, currentWindowRect.bottom - currentWindowRect.top);
    AquaLog( "SetPosSize: New rect (x: %d, y: %d, w: %d, h: %d)\n", nX, nY, nWidth, nHeight);
     */
    SetWindowBounds(mrWindow, kWindowStructureRgn, &newWindowRect);

    UpdateFrameGeometry();

    if( mbShown && bPaint )
        // trigger filling our backbuffer
        SendPaintEvent();

    if (nEvent)
        CallCallback(nEvent, NULL);

}

void AquaSalFrame::DrawMenuBar()
{
    AquaLog( ">*>_> %s\n",__func__);

    if (mpMenu)
    {
        AquaSalMenu *pAquaSalMenu = (AquaSalMenu *) mpMenu;
        SetRootMenu(pAquaSalMenu->mrMenuRef);
    }
}

void AquaSalFrame::GetWorkArea( Rectangle& rRect )
{
    Rect aRect;
    AquaSalFrame* pFrame = this;
    ImplSalCalcMaxClientSize( pFrame, &aRect );
    rRect.nLeft     = aRect.left;
    rRect.nRight    = aRect.right;
    rRect.nTop      = aRect.top;
    rRect.nBottom   = aRect.bottom;
}

SalPointerState AquaSalFrame::GetPointerState()
{
    AquaLog( ">*>_> %s\n",__func__);

    SalPointerState state;

    // get position
    MacOSPoint aMousePos;
    GetMouse( &aMousePos );
    state.maPos = Point( aMousePos.h, aMousePos.v );

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

void AquaSalFrame::SetMenu( SalMenu* pSalMenu )
{
    AquaLog( ">*>_> %s\n",__func__);

    mpMenu = pSalMenu;
    DrawMenuBar();

}

void AquaSalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    AquaLog( ">*>_> %s\n",__func__);
    //TODO: implement
}

void AquaSalFrame::SetBackgroundBitmap( SalBitmap* )
{
    AquaLog( ">*>_> %s\n",__func__);
    //TODO: implement
}

SalBitmap* AquaSalFrame::SnapShot()
{
    AquaLog( ">*>_> %s\n",__func__);
    return mpGraphics ? mpGraphics->getBitmap( 0, 0, mnWidth, mnHeight ) : NULL;
}

SalFrame* AquaSalFrame::GetParent() const
{
    AquaLog( ">*>_> %s\n",__func__);
    return mpParent;
}

void AquaSalFrame::SetParent( SalFrame* pNewParent )
{
    AquaLog( ">*>_> %s\n",__func__);
    mpParent = (AquaSalFrame*)pNewParent;
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
    memset(&maGeometry, 0, sizeof(SalFrameGeometry));

    if (IsWindowCollapsed(mrWindow))
        return;

    Rect fullWindowRect;
    GetWindowBounds(mrWindow, kWindowStructureRgn, &fullWindowRect);

    Rect contentRect;
    GetWindowBounds(mrWindow, kWindowContentRgn, &contentRect);

    maGeometry.nX = contentRect.left;
    maGeometry.nY = contentRect.top;

    maGeometry.nLeftDecoration = contentRect.left - fullWindowRect.left;
    maGeometry.nRightDecoration = fullWindowRect.right - contentRect.right;

    maGeometry.nTopDecoration = contentRect.top - fullWindowRect.top;
    maGeometry.nBottomDecoration = fullWindowRect.bottom - contentRect.bottom;

    Rect titleBarRect;
    GetWindowBounds(mrWindow, kWindowTitleBarRgn, &titleBarRect);

    int width = fullWindowRect.right - fullWindowRect.left - maGeometry.nRightDecoration - maGeometry.nLeftDecoration;
    int height = fullWindowRect.bottom - fullWindowRect.top - maGeometry.nBottomDecoration - maGeometry.nTopDecoration;

    maGeometry.nWidth = width < 0 ? 0 : width;
    maGeometry.nHeight = height < 0 ? 0 : height;

    //DbgPrintFrameGeometry(fullWindowRect, contentRect, titleBarRect, maGeometry);
}

OSStatus HandleCommandProcessEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( ! AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    OSStatus result = eventNotHandledErr;
    HICommand commandStruct;

    GetEventParameter (inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &commandStruct);

    // If the CommandProcess event is from menu, print its MenuRef and MenuItemIndex
    if ( commandStruct.attributes & kHICommandFromMenu ) {
        SalMenuItem *pSalMenuItem;

        OSStatus errval;
        errval = GetMenuItemProperty (commandStruct.menu.menuRef, commandStruct.menu.menuItemIndex,
                                   APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_ITEM_POINTER,
                                   sizeof(pSalMenuItem), NULL, &pSalMenuItem);

        // Only use pSalMenuItem value if the menu item property was found!
        if ( (errval == noErr) && pSalMenuItem)
        {
            // Posting native menu event...
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId = ((AquaSalMenuItem *)pSalMenuItem)->mnId;
            aMenuEvt.mpMenu = ((AquaSalMenuItem *)pSalMenuItem)->mpMenu;
            pSalFrame->CallCallback(SALEVENT_MENUCOMMAND, &aMenuEvt);

            result = noErr;
        }
        else
        {
            int nDialogId = -1;
            switch( commandStruct.commandID )
            {
            case kHICommandQuit:
                pSalFrame->CallCallback( SALEVENT_SHUTDOWN, 0 );
                result = noErr;
                break;
            case kHICommandPreferences:
                nDialogId = SHOWDIALOG_ID_PREFERENCES;
                break;
            case kHICommandAbout:
                nDialogId = SHOWDIALOG_ID_ABOUT;
                break;
            default:
                break;
            }
            if( nDialogId != -1 )
            {
                pSalFrame->CallCallback( SALEVENT_SHOWDIALOG, reinterpret_cast<void*>(nDialogId) );
                result = noErr;
            }
        }
    }

    return result;
}

OSStatus HandleMenuPopulateEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    MenuRef pMenuRef;

    GetEventParameter (inEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof(typeMenuRef), NULL, &pMenuRef);

    SalMenu *pSalMenu;
    OSStatus errval;

    errval = GetMenuItemProperty (pMenuRef, 0,
                               APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_POINTER,
                               sizeof(pSalMenu), NULL, &pSalMenu);

    if ( (errval == noErr) && pSalMenu)
    {

        // To send native menu event, we need its VCL *Menu
        // So we query first item in the menu for it.

        SalMenuItem *pSalMenuItem;

        errval = GetMenuItemProperty (pMenuRef, 1,
                                   APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_ITEM_POINTER,
                                   sizeof(pSalMenuItem), NULL, &pSalMenuItem);

        if ( (errval == noErr) && pSalMenuItem && AquaSalFrame::isAlive( pSalFrame ) )
        {
            // Posting native menu event...
            SalMenuEvent aMenuEvt;
            aMenuEvt.mpMenu = ((AquaSalMenuItem *) pSalMenuItem)->mpMenu;

            // FIXME: looks like it crashes when some menus are activated...
            pSalFrame->CallCallback(SALEVENT_MENUACTIVATE, &aMenuEvt);
            pSalFrame->CallCallback(SALEVENT_MENUDEACTIVATE, &aMenuEvt);
        }
    }

    return noErr;
}

OSStatus HandleMenuClosedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{

    YIELD_GUARD;

    MenuRef pMenuRef;
    GetEventParameter (inEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof(typeMenuRef), NULL, &pMenuRef);

    SalMenu *pSalMenu;
    OSStatus errval;

    errval = GetMenuItemProperty (pMenuRef, 0,
                               APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_POINTER,
                               sizeof(pSalMenu), NULL, &pSalMenu);

    if ( (errval == noErr) && pSalMenu)
    {
        // Posting native menu event...
        SalMenuEvent aMenuEvt;
        aMenuEvt.mpMenu = pSalMenu;
        // FIXME: This does not propagate to the application yet. WHY?
        // pSalFrame->CallCallback(SALEVENT_MENUDEACTIVATE, &aMenuEvt);
    }

    return noErr;
}

OSStatus HandleMenuTargetItemEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    OSStatus result = eventNotHandledErr;

    MenuRef pMenuRef;
    MenuItemIndex mnMenuItemIndex;

    GetEventParameter (inEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof(typeMenuRef), NULL, &pMenuRef);
    GetEventParameter (inEvent, kEventParamMenuItemIndex, typeMenuItemIndex, NULL, sizeof(typeMenuItemIndex), NULL, &mnMenuItemIndex);

    // kEventMenuTargetItem is sent also for top-level items. Ignore
    // them (MenuItemIndex == 0 for them). Or should we send
    // MENUACTIVATE events for them?
    if (mnMenuItemIndex != 0)
    {
        SalMenuItem *pSalMenuItem;
        OSStatus errval;

        errval = GetMenuItemProperty (pMenuRef, mnMenuItemIndex,
                                   APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_ITEM_POINTER,
                                   sizeof(pSalMenuItem), NULL, &pSalMenuItem);

        // We also receive kEventMenuTargetItem events for
        // non-application menus (Apple menu or Application menu). But
        // they do not have SalMenuItem attached with them.
        if ( (errval == noErr) && pSalMenuItem && AquaSalFrame::isAlive( pSalFrame ) )
        {
            // Posting native menu event...
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId = ((AquaSalMenuItem *)pSalMenuItem)->mnId;
            aMenuEvt.mpMenu = ((AquaSalMenuItem *)pSalMenuItem)->mpMenu;
            pSalFrame->CallCallback(SALEVENT_MENUHIGHLIGHT, &aMenuEvt);

            result = noErr;
        }
    }

    return result;
}

OSStatus HandleWindowFocusEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;
#if 0
    // This must be handled in HandleWindowActivatedEvent().
    AquaLog( "sending focus event %s for AquaSalFrame %p\n",
            (GetEventKind(inEvent) == kEventWindowFocusAcquired) ? "GETFOCUS" : "LOSEFOCUS", pSalFrame);

    pSalFrame->CallCallback( (GetEventKind(inEvent) == kEventWindowFocusAcquired) ? SALEVENT_GETFOCUS : SALEVENT_LOSEFOCUS, 0);
#endif

    return noErr;
}


OSStatus HandleWindowCloseEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    BOOST_ASSERT(GetEventClass(inEvent) == kEventClassWindow && GetEventKind(inEvent) == kEventWindowClose && "Only WindowClose event expected");

    YIELD_GUARD;
    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);

    pSalFrame->CallCallback(SALEVENT_CLOSE, 0);
    return noErr;
}

OSStatus HandleAppearanceScrollbarVariantChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    BOOST_ASSERT(GetEventClass(inEvent) == kEventClassAppearance && GetEventKind(inEvent) == kEventAppearanceScrollBarVariantChanged && "Only AppearanceScrollBarVariantChanged event expected");

    YIELD_GUARD;
    getAppleScrollBarVariant();
    return noErr;
}

OSStatus HandleWindowBoundsChangedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    AquaLog("Bounds changed event\n");

    BOOST_ASSERT(GetEventClass(inEvent) == kEventClassWindow && GetEventKind(inEvent) == kEventWindowBoundsChanged && "Only WindowBoundsChanged event expected");

    //AquaLog( "received resize event\n");

    // HACK:
    // during live resize our own event loop will not be active so private timer events
    // that were posted into the event queue will not be handled
    // it seems that all event scheduling takes place in the standard
    // event handlers in this case
    // calling SalInstance::Yield() will allow to dispatch events from the queue and repaints will
    // occur during live resize
    SalData* pSalData = GetSalData();
    pSalData->mpFirstInstance->Yield( false, false /* FIXME: bAllEvents */ );

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    pSalFrame->UpdateFrameGeometry();

    // compose sal resize event
    Rect windowBounds;
    GetEventParameter(inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &windowBounds);

    long windowWidth = windowBounds.right - windowBounds.left;
    long windowHeight = windowBounds.bottom - windowBounds.top;

    // Get the size of the window's content area
    if (windowWidth != pSalFrame->mnWidth || windowHeight != pSalFrame->mnHeight)
    {
        // Cache the new size
        pSalFrame->mnWidth = windowWidth;
        pSalFrame->mnHeight = windowHeight;


        pSalFrame->CallCallback( SALEVENT_RESIZE, NULL);
    }

    return noErr;
}

OSStatus HandleOOoSalUserEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    BOOST_ASSERT(GetEventClass(inEvent) == cOOoSalUserEventClass && "Only SalUserEvents expected");

    //AquaLog( "received user event\n");

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    void* pData = NULL;

    GetEventParameter(inEvent, cOOoSalEventData, cOOoSalEventParamTypePtr, NULL, sizeof(pData), NULL, &pData);

    pSalFrame->CallCallback(SALEVENT_USEREVENT, pData);

    return noErr;
}
OSStatus HandleOOoSalTimerEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    BOOST_ASSERT(GetEventClass(inEvent) == cOOoSalUserEventClass && "Only SalUserEvents expected");

    //AquaLog( "received timer event\n");

    YIELD_GUARD;

    void* pData = NULL;

    GetEventParameter(inEvent, cOOoSalEventData, cOOoSalEventParamTypePtr, NULL, sizeof(pData), NULL, &pData);

    AquaSalTimer *pTimer = reinterpret_cast<AquaSalTimer*>(pData);
    pTimer->CallCallback();

    return noErr;
}

OSStatus HandleWindowActivatedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    switch (GetEventKind(inEvent))
    {
    case kEventWindowActivated:
        pSalFrame->DrawMenuBar();
        pSalFrame->CallCallback(SALEVENT_GETFOCUS, 0);
        pSalFrame->ActivateTSM();
        break;
    case kEventWindowDeactivated:
        pSalFrame->CallCallback(SALEVENT_LOSEFOCUS, 0);
        pSalFrame->DeactivateTSM();
        break;
    default:
        break;
    }

    return noErr;
}

OSStatus HandleWindowPaintEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    YIELD_GUARD;
    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    AquaLog( "received full paint event\n");

    if( pSalFrame->mpGraphics )
        pSalFrame->mpGraphics->UpdateWindow();

    return noErr;
}

OSStatus HandleWindowDrawContentEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    //AquaLog( "received draw content event\n");
    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    if( pSalFrame->mpGraphics )
        pSalFrame->mpGraphics->UpdateWindow();

    return noErr;
}

OSStatus HandleWindowResizeStarted(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    pSalFrame->mpSalInstance->StartForceDispatchingPaintEvents();
    return noErr;
}

OSStatus HandleWindowResizeCompleted(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    pSalFrame->mpSalInstance->StopForceDispatchingPaintEvents();

    pSalFrame->UpdateFrameGeometry();
    pSalFrame->SendPaintEvent();

    return noErr;
}

OSStatus HandleMouseEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    BOOST_ASSERT( ( ( GetEventClass(inEvent) == kEventClassMouse && (
                      GetEventKind(inEvent) == kEventMouseDown ||
                      GetEventKind(inEvent) == kEventMouseUp ||
                      GetEventKind(inEvent) == kEventMouseMoved ||
                      GetEventKind(inEvent) == kEventMouseDragged
                  ) )
                  || ( GetEventClass(inEvent) == kEventClassControl && (
                      GetEventKind(inEvent) == kEventControlTrackingAreaEntered ||
                      GetEventKind(inEvent) == kEventControlTrackingAreaExited )
                  ) )
                  && "Mouse up, mouse down, mouse moved, mouse drag or enter/exit events expected.");

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    // use window bounds to convert from screen to window coordinates
    Rect bounds;
    GetWindowBounds( pSalFrame->mrWindow, kWindowContentRgn, &bounds );

    // get mouse coordinates in screen space
    MacOSPoint aPt;
    GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(aPt), NULL, &aPt);

    SalMouseEvent aMouseEvt;

    // --- RTL --- (mirror mouse pos)
    if( Application::GetSettings().GetLayoutRTL() )
        aMouseEvt.mnX = bounds.right - aPt.h;
    else
        aMouseEvt.mnX = aPt.h - bounds.left;

    aMouseEvt.mnY = aPt.v - bounds.top;
    aMouseEvt.mnCode = 0;
    aMouseEvt.mnTime = static_cast<ULONG>(GetEventTime(inEvent) * 1000);

    // Which modifier keys pressed?
    //
    // Please note:
    // The system automatically preserves the state of the modifier keys during
    // mouse down for the corresponding MouseUp event
    UInt32 modifierKeys;
    GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifierKeys), NULL, &modifierKeys);

    if (modifierKeys & shiftKey)
        aMouseEvt.mnCode |= KEY_SHIFT;
    if (modifierKeys & controlKey)
        aMouseEvt.mnCode |= KEY_MOD1;
    if (modifierKeys & optionKey)
        aMouseEvt.mnCode |= KEY_MOD2;

    // Which mouse button pressed?
    EventMouseButton aButtonEvt;
    GetEventParameter(inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(aButtonEvt), NULL, &aButtonEvt);

    switch (aButtonEvt)
    {
    case kEventMouseButtonPrimary:
        aMouseEvt.mnButton  = MOUSE_LEFT;
        aMouseEvt.mnCode |= MOUSE_LEFT;
        break;

    case kEventMouseButtonSecondary:
        aMouseEvt.mnButton  = MOUSE_RIGHT;
        aMouseEvt.mnCode |= MOUSE_RIGHT;
        break;

    case kEventMouseButtonTertiary:
        aMouseEvt.mnButton  = MOUSE_MIDDLE;
        aMouseEvt.mnCode |= MOUSE_MIDDLE;
        break;
    }

    USHORT eventkind = 0;

    switch (GetEventKind(inEvent))
    {
        case kEventMouseDown:
            eventkind = SALEVENT_MOUSEBUTTONDOWN;
            break;
        case kEventMouseUp:
            eventkind = SALEVENT_MOUSEBUTTONUP;
            break;
        case kEventMouseMoved:
        case kEventMouseDragged:
            eventkind = SALEVENT_MOUSEMOVE;
            break;
        case kEventControlTrackingAreaEntered:
            eventkind = SALEVENT_MOUSEMOVE;
            break;
        case kEventControlTrackingAreaExited:
            eventkind = SALEVENT_MOUSELEAVE;
            break;
    }

    pSalFrame->CallCallback(eventkind, &aMouseEvt);

    // Note: we have to return eventNotHandledErr otherwise
    // the window will not be activated and stay in the background
    return eventNotHandledErr;
}

OSStatus HandleOverlayMouseEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    AquaSalFrame* pCap = AquaSalFrame::GetCaptureFrame();
    return pCap ? HandleMouseEvent( inHandlerCallRef, inEvent, pCap ) : eventNotHandledErr;
}

OSStatus HandleMouseWheelMovedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    BOOST_ASSERT(GetEventClass(inEvent) == kEventClassMouse && GetEventKind(inEvent) == kEventMouseWheelMoved && "Mouse wheel moved event expected");

    AquaLog( "WheelMoved event received.\n");

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    // use window bounds to convert from screen to window coordinates
    Rect bounds;
    GetWindowBounds( pSalFrame->mrWindow, kWindowContentRgn, &bounds );

    // get mouse coordinates in screen space
    MacOSPoint aPt;
    GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(aPt), NULL, &aPt);

    SalWheelMouseEvent aWheelEvt;
    // --- RTL --- (mirror mouse pos)
    if( Application::GetSettings().GetLayoutRTL() )
        aWheelEvt.mnX = bounds.right - aPt.h;
    else
        aWheelEvt.mnX = aPt.h - bounds.left;

    aWheelEvt.mnY = aPt.v - bounds.top;

    aWheelEvt.mnCode = 0;
    aWheelEvt.mnTime = static_cast<ULONG>(GetEventTime(inEvent) * 1000);

    // Which modifier keys pressed?
    UInt32 modifierKeys;
    GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifierKeys), NULL, &modifierKeys);

    if (modifierKeys & shiftKey)
        aWheelEvt.mnCode |= KEY_SHIFT;
    if (modifierKeys & controlKey)
        aWheelEvt.mnCode |= KEY_MOD1;
    if (modifierKeys & optionKey)
        aWheelEvt.mnCode |= KEY_MOD2;

    EventMouseWheelAxis wheelAxis; // kEventMouseWheelAxisX or kEventMouseWheelAxisY
    SInt32 wheelDelta;
    GetEventParameter(inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(wheelAxis), NULL, &wheelAxis);

    if ( wheelAxis == kEventMouseWheelAxisX )
        aWheelEvt.mbHorz = TRUE;
    else
        aWheelEvt.mbHorz = FALSE;

    GetEventParameter(inEvent, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(wheelDelta), NULL, &wheelDelta);

    static ULONG nLines = 0;
    if( ! nLines )
    {
        char* pEnv = getenv( "SAL_WHEELLINES" );
        nLines = pEnv ? atoi( pEnv ) : 3;
        if( nLines > 10 )
            nLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
    }

    aWheelEvt.mnScrollLines=nLines;

    // FIXME: This all is a bit suspect ;-)
    aWheelEvt.mnDelta = wheelDelta;
    aWheelEvt.mnNotchDelta = (wheelDelta>0) ? 1 : -1;

    AquaLog( "... %ld, %ld - %d, %ld\n", aWheelEvt.mnX, aWheelEvt.mnY,
            wheelAxis, aWheelEvt.mnDelta);

    pSalFrame->CallCallback(SALEVENT_WHEELMOUSE, &aWheelEvt);

    return noErr;
}

void DbgPrintModifiers (UInt32 keyModifiers) {

    AquaLog( "...... keyModifiers: ");
    if (keyModifiers & cmdKey)
        AquaLog( "cmdKey, ");
    if (keyModifiers & shiftKey)
        AquaLog( "shiftKey, ");
    if (keyModifiers & alphaLock)
        AquaLog( "alphaLock (CapsLock), ");
    if (keyModifiers & optionKey)
        AquaLog( "optionKey, ");
    if (keyModifiers & controlKey)
        AquaLog( "controlKey, ");
    if (keyModifiers & kEventKeyModifierNumLockMask)
        AquaLog( "NumLock key, ");
    if (keyModifiers & kEventKeyModifierFnMask)
        AquaLog( "Fn key, ");
    AquaLog( "\n");
}

// Table for translation of system key code to sal keycodes
#define KEY_TAB_SIZE 131

static USHORT aImplTranslateKeyTab[KEY_TAB_SIZE] =
{
  // sal key code constant
  KEY_A, /*   0 */
  KEY_S,
  KEY_D,
  KEY_F,
  KEY_H,
  KEY_G,
  KEY_Z,
  KEY_X,
  KEY_C,
  KEY_V,
  0, /*  10 */
  KEY_B,
  KEY_Q,
  KEY_W,
  KEY_E,
  KEY_R,
  KEY_Y,
  KEY_T,
  KEY_1,
  KEY_2,
  KEY_3, /*  20 */
  KEY_4,
  KEY_6,
  KEY_5,
  0,
  KEY_9,
  KEY_7,
  0,
  KEY_8,
  KEY_0,
  0, /*  30 */
  KEY_O,
  KEY_U,
  0,
  KEY_I,
  KEY_P,
  KEY_RETURN,
  KEY_L,
  KEY_J,
  0,
  KEY_K, /*  40 */
  0,
  0,
  0,
  0,
  KEY_N,
  KEY_M,
  0,
  KEY_TAB,
  KEY_SPACE,
  0, /*  50 */
  KEY_BACKSPACE,
  0,
  KEY_ESCAPE,
  0,
  0,
  0,
  0,
  0,
  0,
  0, /*  60 */
  0,
  0,
  0,
  0,
  KEY_DECIMAL, /* ? Dot in numpad */
  0,
  0,
  0,
  0,
  0, /*  70 */
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0, /*  80 */
  0,
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  0, /*  90 */
  KEY_8,
  KEY_9,
  0,
  0,
  0,
  KEY_F5,
  KEY_F6,
  KEY_F7,
  KEY_F3,
  KEY_F8, /* 100 */
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0, /* 110 */
  0,
  0,
  0,
  KEY_INSERT,
  KEY_HOME,
  KEY_PAGEUP,
  KEY_DELETE,
  KEY_F3,
  KEY_END,
  KEY_F2, /*  120 */
  KEY_PAGEDOWN,
  KEY_F1,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_DOWN, /* 125 */
  KEY_UP,   /* 126 */
  0,
  0,
  0,
  0 /*  130 */
};

OSStatus HandleKeyboardEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    static ULONG nRepeatCount = 0;
    UInt32 keyModifiers;
    USHORT nModCode = 0;

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);
    if( !AquaSalFrame::isAlive( pSalFrame ) )
        return noErr;

    AquaLog( ">>>> HandleKeyboardEvent\n");

    UInt32 eventKind = GetEventKind(inEvent);

    switch (eventKind) {
    case kEventRawKeyUp:
    case kEventRawKeyDown:
    case kEventRawKeyRepeat:

        if (eventKind == kEventRawKeyDown)
            AquaLog( " ... kEventRawKeyDown\n");
        else if (eventKind == kEventRawKeyUp)
            AquaLog( " ... kEventRawKeyUp\n");
        else if (eventKind == kEventRawKeyRepeat)
            AquaLog( " ... kEventRawKeyRepeat\n");

        UInt32 keyCode, keyboardType;
        char keyChar;

        GetEventParameter (inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode);
        AquaLog( " ...... keyCode = %ld (%lx)\n", keyCode, keyCode);

        GetEventParameter (inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(keyChar), NULL, &keyChar);
        AquaLog( " ...... keyChar = %c (%x)\n", keyChar, keyChar);

        GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(keyModifiers), NULL, &keyModifiers);
        AquaLog( " ...... keyModifiers = %ld (%ld)\n", keyModifiers, keyModifiers);

        DbgPrintModifiers(keyModifiers);

        GetEventParameter (inEvent, kEventParamKeyboardType, typeUInt32, NULL, sizeof(keyboardType), NULL, &keyboardType);
        AquaLog( " ...... keyboardType = %ld (%lx)\n", keyboardType, keyboardType);

        SalKeyEvent aKeyEvt;

        if ( (keyChar >= '0') && (keyChar <= '9') )
            aKeyEvt.mnCode = sal::static_int_cast<USHORT>(KEYGROUP_NUM + keyChar - '0');
        else if ( (keyChar >= 'A') && (keyChar <= 'Z') )
            aKeyEvt.mnCode = sal::static_int_cast<USHORT>(KEYGROUP_ALPHA + keyChar - 'A');
        else if ( (keyChar >= 'a') && (keyChar <= 'z') )
            aKeyEvt.mnCode = sal::static_int_cast<USHORT>(KEYGROUP_ALPHA + keyChar - 'a');
        else if ( keyChar == 0x0d )    // RETURN
            aKeyEvt.mnCode = KEY_RETURN;
        else if ( keyChar == 0x1b )    // ESCAPE
            aKeyEvt.mnCode = KEY_ESCAPE;
        else if ( keyChar == 0x09 )    // TAB
            aKeyEvt.mnCode = KEY_TAB;
        else if ( keyChar == 0x20 )    // SPACE
            aKeyEvt.mnCode = KEY_SPACE;
        else
            aKeyEvt.mnCode = aImplTranslateKeyTab[keyCode];

        aKeyEvt.mnCharCode = keyChar;

        // Set modifiers
        // FIXME: how to map them? shift, option, control and command!
        if (keyModifiers & shiftKey)
            nModCode |= KEY_SHIFT;
        if (keyModifiers & controlKey)
            nModCode |= KEY_MOD1;
        if (keyModifiers & optionKey)
            nModCode |= KEY_MOD5;

        aKeyEvt.mnCode |= nModCode;
        aKeyEvt.mnTime = static_cast<ULONG>(GetEventTime(inEvent) * 1000);

        if (eventKind == kEventRawKeyRepeat)
            nRepeatCount++;
        else
            // Not repeat event -> reset repeat counter
            nRepeatCount = 0;

        aKeyEvt.mnRepeat = nRepeatCount;

        if (eventKind == kEventRawKeyDown || eventKind == kEventRawKeyRepeat)
            pSalFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
        if (eventKind == kEventRawKeyUp)
            pSalFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );

        break;

    case kEventRawKeyModifiersChanged:
        AquaLog( " ... kEventRawKeyModifiersChanged\n");

        GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(keyModifiers), NULL, &keyModifiers);
        AquaLog( " ...... keyModifiers = %ld (%ld)\n", keyModifiers, keyModifiers);

        DbgPrintModifiers(keyModifiers);

        // FIXME: send KEYMODCHANGE event...

        break;

    default:
        AquaLog( " ... UNKNOWN!!!\n");
    }

    AquaLog( "<<<< HandleKeyboardEvent\n");

    return noErr;
}

OSStatus GetInputText(EventRef inEvent, CFMutableStringRef outString, UInt32 *outLength)
{
    UInt32 len;
    OSStatus status;
    UniChar *text;

    status = GetEventParameter(inEvent, kEventParamTextInputSendText,
                               typeUnicodeText, NULL, 0, &len, NULL);

    *outLength = len / sizeof(UniChar);

    if (status != noErr)
        return status;

    AquaLog( "GetInputText len: %ld (%ld)\n", len, len/sizeof(UniChar));
    if (len > 0)
    {
        text = (UniChar *)malloc(len);
        status= GetEventParameter(inEvent, kEventParamTextInputSendText,
                                  typeUnicodeText, NULL, len, NULL, text);

        CFStringAppendCharacters(outString, text, len / sizeof(UniChar));
        free((void *)text);
    }

    return status;
}

static void CommitInputText(CFMutableStringRef text, UInt32 len, AquaSalFrame* pSalFrame)
{
    SalExtTextInputEvent InputEvent;
    char str[BUFSIZ];

    CFStringGetCString(text, str, BUFSIZ, kCFStringEncodingUTF8);
    AquaLog( "CommitInputText: %s\n", str);

    InputEvent.mnTime = 0;
    InputEvent.mpTextAttr = 0;
    InputEvent.maText = String(str, RTL_TEXTENCODING_UTF8);
    InputEvent.mnCursorPos = len / sizeof(UniChar);
    InputEvent.mnCursorFlags = 0;
    InputEvent.mnDeltaStart = 0;
    InputEvent.mbOnlyCursor = FALSE;

    pSalFrame->CallCallback(SALEVENT_EXTTEXTINPUT, (void *)&InputEvent);
    pSalFrame->CallCallback(SALEVENT_ENDEXTTEXTINPUT, NULL);
}

OSStatus HandleUpdateActiveInputArea(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, AquaSalFrame* pSalFrame )
{
    UInt32 textLength, fixLength, rngSize;
    OSStatus status = eventParameterNotFoundErr;
    long refcon;
    CFMutableStringRef mutString;

    AquaLog( ">>>> HandleUpdateActiveInputArea\n");

    status = GetEventParameter(inEvent, kEventParamTextInputSendRefCon,
                               typeLongInteger, NULL, sizeof(UInt32), NULL, &refcon);

    if (status != noErr)
        return status;

    mutString = CFStringCreateMutable(NULL, 0);
    status = GetInputText(inEvent, mutString, &textLength);

    char p[BUFSIZ];
    CFStringGetCString(mutString, p, BUFSIZ, kCFStringEncodingUTF8);
    AquaLog( "text: %s, textLength %d\n", p, textLength);

    if (status != noErr)
        goto error;

    status = GetEventParameter(inEvent, kEventParamTextInputSendFixLen,
                               typeLongInteger, NULL, sizeof(UInt32), NULL, &fixLength);

    if (status != noErr)
        goto error;

    AquaLog( "fixLen: %ld (%ld)\n", fixLength, fixLength/sizeof(UniChar));

    if (fixLength)
    {
        CommitInputText(mutString, fixLength, pSalFrame);
    }

    if (textLength > fixLength/sizeof(UniChar))
    {
        status = GetEventParameter(inEvent, kEventParamTextInputSendHiliteRng,
                typeTextRangeArray, NULL, 0, &rngSize, NULL);

        if (status == noErr)
        {
            TextRangeArray *hiliteRngArray = (TextRangeArray *)malloc(rngSize);
            if (hiliteRngArray)
            {
                status = GetEventParameter(inEvent,
                        kEventParamTextInputSendHiliteRng,
                        typeTextRangeArray, NULL, rngSize, NULL,
                        hiliteRngArray);

                if (status == noErr)
                {
                    int rangeCount = hiliteRngArray->fNumOfRanges;
                    int nCursorPos = 0;
                    std::vector<USHORT> aInputFlags = std::vector<USHORT>(std::max(1, (int)textLength), 0);
                    AquaLog("rangeCount %d\n", rangeCount);
                    for (int i = 0; i < rangeCount; i++)
                    {
                        AquaLog("fStart %d\n", hiliteRngArray->fRange[i].fStart);
                        AquaLog("fEnd %d\n", hiliteRngArray->fRange[i].fEnd);
                        AquaLog("fHiliteStyle %d\n", hiliteRngArray->fRange[i].fHiliteStyle);
                        long nStart = hiliteRngArray->fRange[i].fStart / sizeof(UniChar);
                        long nEnd = hiliteRngArray->fRange[i].fEnd / sizeof(UniChar);
                        short nHiliteStyle = hiliteRngArray->fRange[i].fHiliteStyle;

                        if (nHiliteStyle == kTSMHiliteCaretPosition)
                        {
                            nCursorPos = nStart;
                            AquaLog("nCursorPos %d\n", nCursorPos);
                        }

                        for (long j = nStart; j < nEnd; j++)
                        {
                            USHORT nAttr = 0;
                            switch (nHiliteStyle)
                            {
                            case kTSMHiliteRawText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
                                break;
                            case kTSMHiliteSelectedRawText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
                                break;
                            case kTSMHiliteConvertedText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
                                break;
                            case kTSMHiliteSelectedConvertedText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_BOLDUNDERLINE;
                                break;
                            case kTSMHiliteBlockFillText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
                                break;
                            case kTSMHiliteOutlineText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
                                break;
                            case kTSMHiliteSelectedText:
                                nAttr = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
                                break;
                            case kTSMHiliteNoHilite:
                            default:
                                break;
                            }
                            aInputFlags[j] = nAttr;
                        }
                    }
                    SalExtTextInputEvent aInputEvent;
                    aInputEvent.mnTime = 0;
                    aInputEvent.maText = String(p, RTL_TEXTENCODING_UTF8);
                    aInputEvent.mnCursorPos = nCursorPos;
                    aInputEvent.mnDeltaStart = 0;
                    aInputEvent.mbOnlyCursor = FALSE;
                    aInputEvent.mpTextAttr = &aInputFlags[0];

                    pSalFrame->CallCallback(SALEVENT_EXTTEXTINPUT, (void *)&aInputEvent);
                }
                free(hiliteRngArray);
            }
        }
    }

    if (textLength == 0)
    {
        SalExtTextInputEvent aEmptyEv;
        aEmptyEv.mnTime = 0;
        aEmptyEv.mpTextAttr = 0;
        aEmptyEv.maText = String();
        aEmptyEv.mnCursorPos = 0;
        aEmptyEv.mnCursorFlags = 0;
        aEmptyEv.mnDeltaStart = 0;
        aEmptyEv.mbOnlyCursor = FALSE;
        pSalFrame->CallCallback(SALEVENT_EXTTEXTINPUT, (void *)&aEmptyEv);
        pSalFrame->CallCallback(SALEVENT_ENDEXTTEXTINPUT, NULL);
    }

error:
    CFRelease(mutString);
    return status;
}

static USHORT GetKeyCode(UniChar uc, UInt32 keyCode)
{
    USHORT nCode = 0;
    if ( (uc >= '0') && (uc <= '9') )
    nCode = sal::static_int_cast<USHORT>(KEYGROUP_NUM + uc - '0');
    else if ( (uc >= 'A') && (uc <= 'Z') )
        nCode = sal::static_int_cast<USHORT>(KEYGROUP_ALPHA + uc - 'A');
    else if ( (uc >= 'a') && (uc <= 'z') )
        nCode = sal::static_int_cast<USHORT>(KEYGROUP_ALPHA + uc - 'a');
    else if ( uc == 0x0d || ( uc == 0 && keyCode == 0x4c ) )    // RETURN || ENTER
        //virtual keycode for numeric keypad enter is 0x4c (76 decimal for gdb users)
        nCode = KEY_RETURN;
    else if ( uc == 0x1b )    // ESCAPE
        nCode = KEY_ESCAPE;
    else if ( uc == 0x09 )    // TAB
        nCode = KEY_TAB;
    else if ( uc == 0x20 )    // SPACE
        nCode = KEY_SPACE;
    else
        nCode = aImplTranslateKeyTab[keyCode];

    return nCode;
}

static USHORT GetKeyModCode(UInt32 modifier)
{
    USHORT nCode = 0;

    // FIXME: how to map them? shift, option, control and command!
    if (modifier & shiftKey)
    nCode |= KEY_SHIFT;
    if (modifier & controlKey)
    nCode |= KEY_MOD1;
    if (modifier & optionKey)
    nCode |= KEY_MOD5;

    return nCode;
}

OSStatus HandleUnicodeForKeyEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, AquaSalFrame* pSalFrame)
{
    // gettting the Unicode char in the TextInputUnicodeForKey event

    UniChar uc, translated_uc = 0;
    AquaLog( ">>>> HandleUnicodeForKeyEvent\n");

    GetEventParameter(inEvent, kEventParamTextInputSendText, typeUnicodeText,
            NULL, sizeof(uc), NULL, &uc);
    AquaLog( "in HandleTextInputUnicodeForKeyEvent, uniChar is %04x=%c\n", uc, (char)uc);

    // gettting the original keyDown event in the TextInputUnicodeForKey event
    EventRef origEvent;
    GetEventParameter(inEvent, kEventParamTextInputSendKeyboardEvent,
            typeEventRef, NULL, sizeof(origEvent), NULL, &origEvent);

    // gettting the Unicode char in the original keyDown event
    GetEventParameter(origEvent, kEventParamKeyUnicodes,
            typeUnicodeText, NULL, sizeof(uc), NULL, &uc);
    AquaLog( ", in original Keyboard Event, uniChar is %04x=%c\n", uc, (char)uc);

    // gettting the Mac OS ASCII char in the original keyDown event (may be meaningless)
    unsigned char c;
    GetEventParameter(origEvent, kEventParamKeyMacCharCodes, typeChar, NULL,
            sizeof(c), NULL, &c);
    AquaLog( ", mac char is %04x=%c\n", c, c);

    // gettting the key code in the original keyDown event (may be meaningless)
    UInt32 keyCode;
    GetEventParameter(origEvent, kEventParamKeyCode, typeUInt32, NULL,
            sizeof(keyCode), NULL, &keyCode);
    AquaLog( ", key code is %ld\n", keyCode);

    // gettting the keyboard type in the original keyDown event
    UInt32 keyboardType = 0;
    GetEventParameter(origEvent, kEventParamKeyboardType, typeUInt32, NULL,
            sizeof(keyboardType), NULL, &keyboardType);

    // gettting the key modifiers in the original keyDown event
    UInt32 modifier;
    GetEventParameter(origEvent, kEventParamKeyModifiers, typeUInt32, NULL,
                      sizeof(modifier), NULL, &modifier);
    AquaLog( ", modifier is %ld\n", modifier);
    if (modifier & controlKey) {
        KeyboardLayoutRef keyboardLayout;
        const UCKeyboardLayout *uchrData;
        UInt32 dummy = 0;

        KLGetCurrentKeyboardLayout(&keyboardLayout);
        KLGetKeyboardLayoutProperty(keyboardLayout, kKLuchrData,
                                    (const void **)&uchrData);
        UCKeyTranslate(uchrData, keyCode,
                       kUCKeyActionDisplay, (modifier & ~controlKey) >> 8,
                       keyboardType, kUCKeyTranslateNoDeadKeysMask, &dummy,
                       1, &dummy, &translated_uc);
        AquaLog( ", it's a Control-%c\n", translated_uc);
    }

    SalKeyEvent aKeyEvt;

    aKeyEvt.mnTime = static_cast<ULONG>(GetEventTime(origEvent) * 1000);
    aKeyEvt.mnCharCode = uc;
    aKeyEvt.mnRepeat = 0;
    aKeyEvt.mnCode = GetKeyCode(translated_uc, keyCode);
    // Set modifiers
    aKeyEvt.mnCode |= GetKeyModCode(modifier);

    pSalFrame->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
    if( AquaSalFrame::isAlive( pSalFrame ) )
        pSalFrame->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
    AquaLog( "<<<< HandleUnicodeForKeyEvent\n");

    return eventNotHandledErr;
}

OSStatus HandleOffsetToPos(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, AquaSalFrame* pSalFrame)
{
    OSStatus status = eventNotHandledErr;
    UInt32 offset;
    SalExtTextInputPosEvent aPosEvent;
    MacOSPoint aPt;

    AquaLog( ">>>> HandleOffsetToPos\n");

    status = GetEventParameter(inEvent, kEventParamTextInputSendTextOffset,
                               typeLongInteger, NULL, sizeof(UInt32), NULL, &offset);

    if (status != noErr)
        return status;

    pSalFrame->CallCallback(SALEVENT_EXTTEXTINPUTPOS, (void *)&aPosEvent);

    aPt.h = aPosEvent.mnX + aPosEvent.mnWidth + pSalFrame->maGeometry.nX;
    aPt.v = aPosEvent.mnY + aPosEvent.mnHeight + pSalFrame->maGeometry.nY + 2; // add some space for underlines

    status = SetEventParameter(inEvent, kEventParamTextInputReplyPoint,
                               typeQDPoint, sizeof(aPt), &aPt);

    return status;
}


OSStatus HandleTSMEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
{
    OSStatus result = eventNotHandledErr;

    YIELD_GUARD;

    AquaSalFrame* pSalFrame = reinterpret_cast<AquaSalFrame*>(inUserData);

    if( AquaSalFrame::isAlive( pSalFrame ) )
    {
        AquaLog( ">>>> HandleTSMEvent\n");

        UInt32 eventClass = GetEventClass(inEvent);
        UInt32 eventKind = GetEventKind(inEvent);

        if (eventClass == kEventClassTextInput)
        {
            switch (eventKind) {
                case kEventTextInputUpdateActiveInputArea:
                    result = HandleUpdateActiveInputArea(inHandlerCallRef, inEvent, pSalFrame);
                    break;
                case kEventTextInputUnicodeForKeyEvent:
                    result = HandleUnicodeForKeyEvent(inHandlerCallRef, inEvent, pSalFrame);
                    break;
                case kEventTextInputOffsetToPos:
                    result = HandleOffsetToPos(inHandlerCallRef, inEvent, pSalFrame);
                    break;
                default:
                    AquaLog( " ... unknown eventkind in HandleTSMEvent!!!\n");
                    break;
            }
        }
    }

    AquaLog( "<<<< HandleTSMEvent\n");

    return result;
}

OSStatus GetOptimalWindowSize(Rect* rect);

void DbgWhichSalFrameStyle(ULONG style)
{
    AquaLog( "==============================\n");

    if (style & SAL_FRAME_STYLE_DEFAULT)
        AquaLog( "SAL_FRAME_STYLE_DEFAULT\n");
    if (style & SAL_FRAME_STYLE_MOVEABLE)
        AquaLog( "SAL_FRAME_STYLE_MOVEABLE\n");
    if (style & SAL_FRAME_STYLE_SIZEABLE)
        AquaLog( "SAL_FRAME_STYLE_SIZEABLE\n");
    if (style & SAL_FRAME_STYLE_CLOSEABLE)
        AquaLog( "SAL_FRAME_STYLE_CLOSEABLE\n");
    if (style & SAL_FRAME_STYLE_NOSHADOW)
        AquaLog( "SAL_FRAME_STYLE_NOSHADOW\n");
    if (style & SAL_FRAME_STYLE_TOOLTIP)
        AquaLog( "SAL_FRAME_STYLE_TOOLTIP\n");
    if (style & SAL_FRAME_STYLE_OWNERDRAWDECORATION)
        AquaLog( "SAL_FRAME_STYLE_OWNERDRAWDECORATION\n");
    if (style & SAL_FRAME_STYLE_DIALOG)
        AquaLog( "SAL_FRAME_STYLE_DIALOG\n");
    if (style & SAL_FRAME_STYLE_CHILD)
        AquaLog( "SAL_FRAME_STYLE_CHILD\n");
    if (style & SAL_FRAME_STYLE_FLOAT)
        AquaLog( "SAL_FRAME_STYLE_FLOAT\n");
    if (style & SAL_FRAME_STYLE_TOOLWINDOW)
        AquaLog( "SAL_FRAME_STYLE_TOOLWINDOW\n");
    if (style & SAL_FRAME_STYLE_INTRO)
        AquaLog( "SAL_FRAME_STYLE_INTRO\n");

    AquaLog( "==============================\n");
}

void AquaSalFrame::CreateNewSystemWindow(CarbonWindowRef pParent,  ULONG nSalFrameStyle)
{
    DbgWhichSalFrameStyle(nSalFrameStyle);

    // in order to receive windowupdate events we must not use compositing (kWindowCompositingAttribute)!

    // initialize with useful defaults
    unsigned int nWindowAttributes = kWindowStandardHandlerAttribute;
    WindowClass windowClass = kDocumentWindowClass;

    if (nSalFrameStyle & SAL_FRAME_STYLE_CHILD)
    {
        BOOST_ASSERT(false && "Not yet implemented! How to handle child windows on Mac OS X?");
    }
    else // none-child windows
    {
        if (nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT)
            nWindowAttributes |= kWindowStandardDocumentAttributes;

        if( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT || nSalFrameStyle & SAL_FRAME_STYLE_TOOLWINDOW)
            windowClass = kFloatingWindowClass;
        if (nSalFrameStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION)
            windowClass = kToolbarWindowClass;
        if (nSalFrameStyle & SAL_FRAME_STYLE_TOOLTIP)
            windowClass = kHelpWindowClass;
        if( nSalFrameStyle & SAL_FRAME_STYLE_INTRO )
            windowClass = kUtilityWindowClass;

        // check moveable, sizeable, closeable
        if( !(nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) )  // window without decoration
        {
            if( windowClass != kHelpWindowClass &&   // creation fails with kHelpWindowClass and kWindowNoTitleBarAttribute
                windowClass != kToolbarWindowClass ) // creation fails with kToolbarWindowClass and kWindowNoTitleBarAttribute
                nWindowAttributes |= kWindowNoTitleBarAttribute;
        }
        else
        {
            // close box requires a titlebar
            if( nSalFrameStyle & SAL_FRAME_STYLE_CLOSEABLE )
                nWindowAttributes |= kWindowCloseBoxAttribute;
            else
                nWindowAttributes &= ~kWindowCloseBoxAttribute;
        }

        if( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE )
        {
            nWindowAttributes |= (kWindowResizableAttribute | kWindowLiveResizeAttribute);
            if( (nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE) && (windowClass == kDocumentWindowClass))
                nWindowAttributes |= (kWindowCollapseBoxAttribute | kWindowFullZoomAttribute);   // requires titlebar (moveable) and not allowed for floating windows
        }
        else
            nWindowAttributes &= ~(kWindowResizableAttribute | kWindowLiveResizeAttribute | kWindowCollapseBoxAttribute | kWindowFullZoomAttribute);
    }

    Rect aContentRect;
    GetOptimalWindowSize(&aContentRect);

    // Directly connect the frame with the window in contrast to Win32 where frame and
    // window will be connected in the WM_CREATE message handler
    OSStatus rc = CreateNewWindow(windowClass, nWindowAttributes, &aContentRect, &mrWindow);

    if (rc != noErr)
        throw runtime_error("System window creation failed");

    HIViewRef content = NULL;
    rc = HIViewFindByID( HIViewGetRoot( mrWindow ), kHIViewWindowContentID, &content );

    HIViewTrackingAreaRef track_area = NULL;
    rc = HIViewNewTrackingArea( content, NULL, 0, &track_area );
    rc = InstallControlEventHandler( content, HandleMouseEvent, GetEventTypeCount( cMouseEnterExitEvent ), cMouseEnterExitEvent, this, NULL );

    UpdateFrameGeometry();

    // Store window handle
    maSysData.rWindow = mrWindow;

    // Cache the size of the content area of the window
    mnHeight = aContentRect.bottom - aContentRect.top;
    mnWidth = aContentRect.right - aContentRect.left;

    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowCloseEvent), 1, &cWindowCloseEvent);

    if(windowClass != kHelpWindowClass)     //workaround for tooltip crash #74392
        InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowBoundsChangedEvent), 1, &cWindowBoundsChangedEvent);

    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowFocusEvent), GetEventTypeCount(cWindowFocusEvent), cWindowFocusEvent);

    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleOOoSalUserEvent), 1, &cOOoSalUserEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleOOoSalTimerEvent), 1, &cOOoSalTimerEvent);

    // do not register for paint events as this would result in no draw content events during resize anymore
    // draw content events, however, are more effective as they do not require a full window repaint
    //InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowPaintEvent), 1, &cWindowPaintEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowDrawContentEvent), 1, &cWindowDrawContentEvent);

    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowActivatedEvent), GetEventTypeCount(cWindowActivatedEvent), cWindowActivatedEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleMouseEvent), GetEventTypeCount(cMouseEvent), cMouseEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleMouseWheelMovedEvent), 1, &cMouseWheelMovedEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowResizeStarted), 1, &cWindowResizeStarted);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleWindowResizeCompleted), 1, &cWindowResizeCompleted);

    /* Menu event handlers */

    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleCommandProcessEvent), 1, &cCommandProcessEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleMenuPopulateEvent), 1, &cMenuPopulateEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleMenuClosedEvent), 1, &cMenuClosedEvent);
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleMenuTargetItemEvent), 1, &cMenuTargetItemEvent);

    /* Keyboard event handlers*/

#if 0
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleKeyboardEvent), GetEventTypeCount(cKeyboardRawKeyEvents), cKeyboardRawKeyEvents);
#else
    // TSM!
    InstallAndRegisterEventHandler(NewEventHandlerUPP(HandleTSMEvent), GetEventTypeCount(cTextInputEvents), cTextInputEvents);
#endif
    /* Events for scrollbar */
    EventHandlerRef aEventHandlerRef;
    InstallEventHandler( GetApplicationEventTarget(), NewEventHandlerUPP(HandleAppearanceScrollbarVariantChanged), 1, &cAppearanceScrollbarVariantChangedEvent, NULL, &aEventHandlerRef);
}

OSStatus AquaSalFrame::InstallAndRegisterEventHandler(EventHandlerUPP upp, size_t nEvents, const EventTypeSpec* eventSpec)
{
    EventHandlerRef evtHandler;
    OSStatus rc = InstallWindowEventHandler(mrWindow, upp, nEvents, eventSpec, this, &evtHandler);

    if (rc == noErr)
        mSysWindowEventHandlerDataContainer.push_back(make_pair(upp, evtHandler));

    return rc;
}

void AquaSalFrame::DeinstallAndUnregisterAllEventHandler()
{
    SysWindowEventHandlerDataContainer_t::iterator iter = mSysWindowEventHandlerDataContainer.begin();
    SysWindowEventHandlerDataContainer_t::iterator iter_end = mSysWindowEventHandlerDataContainer.end();
    for (/*NOP*/; iter != iter_end; ++iter)
    {
        DisposeEventHandlerUPP((*iter).first);
        RemoveEventHandler((*iter).second);
    }
    mSysWindowEventHandlerDataContainer.clear();
}

// -----------------------------------------------------------------------

void AquaSalFrame::CaptureMouse( BOOL bCapture )
{
    AquaLog( ">*>_> %s\n",__func__);
    if( bCapture )
    {
        if( s_rOverlay == 0 )
        {
            Rect aFullRect;
            ImplSalCalcFullScreenSize( this, &aFullRect );

            OSStatus rc = CreateNewWindow( kOverlayWindowClass,
                                           kWindowOpaqueForEventsAttribute,
                                           &aFullRect,
                                           &s_rOverlay);
            if( rc != noErr )
                return;
            EventHandlerRef evtHandler;
            EventHandlerUPP upp( NewEventHandlerUPP(HandleOverlayMouseEvent) );
            rc = InstallWindowEventHandler( s_rOverlay, upp, GetEventTypeCount(cMouseEvent), cMouseEvent, this, &evtHandler);
            if( rc != noErr )
            {
                ReleaseWindow( s_rOverlay );
                s_rOverlay = 0;
                return;
            }
            s_aOverlayEvtHandler.first = upp;
            s_aOverlayEvtHandler.second = evtHandler;
            ShowWindow( s_rOverlay );
        }
        s_pCaptureFrame = this;
    }
    else if( ! bCapture && s_pCaptureFrame == this )
    {
        s_pCaptureFrame = NULL;
        if( s_rOverlay != 0 )
        {
            DisposeEventHandlerUPP( s_aOverlayEvtHandler.first );
            RemoveEventHandler( s_aOverlayEvtHandler.second );
            ReleaseWindow( s_rOverlay );
            s_rOverlay = 0;
        }
    }
}

OSStatus GetOptimalWindowSize(Rect* rect)
{
    OSStatus err = noErr;

    BOOST_ASSERT(rect != NULL && "Precondition violated");

    // [FIXME] : define better the main window size and implement screen detection
    // In multiscreen mode, the fullscreen always fill the active windows
    Rect aRect;
    ImplSalCalcFullScreenSize( NULL, &aRect );

    rect->left   = aRect.left + floor( 0.10 * (aRect.right - aRect.left) );
    rect->right  = aRect.right - floor( 0.10 * (aRect.right - aRect.left) );
    rect->top    = aRect.top + floor( 0.10 * (aRect.bottom - aRect.top) );
    rect->bottom = aRect.bottom - floor( 0.10 * (aRect.bottom - aRect.top) );

    return err;
}

void AquaSalFrame::ActivateTSM()
{
    AquaLog( ">*>_> %s\n",__func__);
    if (maTsmDocumentId)
    ActivateTSMDocument(maTsmDocumentId);
}

void AquaSalFrame::DeactivateTSM()
{
    AquaLog( ">*>_> %s\n",__func__);
    if (maTsmDocumentId)
    DeactivateTSMDocument(maTsmDocumentId);
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
