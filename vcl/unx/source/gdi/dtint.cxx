/*************************************************************************
 *
 *  $RCSfile: dtint.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2000-09-27 08:35:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <prex.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <postx.h>

#ifdef USE_CDE
#include <cdeint.hxx>
#endif
#include <kdeint.hxx>
#include <soicon.hxx>
#include <saldisp.hxx>
#include <saldata.hxx>
#include <salbmp.hxx>
#include <salframe.hxx>

#include <strhelper.hxx>
#include <svapp.hxx>

#include <unistd.h>

#ifndef _VOS_PROCESS_HXX
#include <vos/process.hxx>
#endif

#include <tools/urlobj.hxx>

#ifdef SOLARIS
// Solaris 2.5.1 misses it in unistd.h
extern "C" int usleep(unsigned int);
#endif

#define MAX_TRY_CONVERTSELECTION 40
#define EVENTMASK_WHILE_DRAGGING ButtonPressMask | ButtonReleaseMask | PointerMotionMask

BOOL bSymbolLoadFailed = FALSE;

DtIntegratorList DtIntegrator::aIntegratorList;
String DtIntegrator::aHomeDir;

DtIntegrator::DtIntegrator( SalFrame* pFrame ) :
        mpSalFrame( pFrame ),
        mpSalDisplay( pFrame->maFrameData.GetDisplay() ),
        meType( DtGeneric ),
        mnRefCount( 0 ),
        mpLastData( 0 ),
        maDropSource( None ),
        mpDropTarget( NULL ),
        maDragTarget( None ),
        meDragState( DtDragNone )
{
    mpDisplay = mpSalDisplay->GetDisplay();
    maSelectionWindow =
        XCreateSimpleWindow( mpDisplay, DefaultRootWindow( mpDisplay ),
                             10,10,10,10,0,0,1 );
    mnClipboardAtom = XInternAtom( mpDisplay, "CLIPBOARD", False );
    mnTargetsAtom = XInternAtom( mpDisplay, "TARGETS", False );
    mnCompoundAtom = XInternAtom( mpDisplay, "COMPOUND_TEXT", False );
    maExPropertyAtom = XInternAtom( mpDisplay, "VCLExchangeProperty", False );

    mnXdndAware             = XInternAtom( mpDisplay, "XdndAware", False );
    mnXdndSelection         = XInternAtom( mpDisplay, "XdndSelection", False );
    mnXdndEnter             = XInternAtom( mpDisplay, "XdndEnter", False );
    mnXdndLeave             = XInternAtom( mpDisplay, "XdndLeave", False );
    mnXdndStatus            = XInternAtom( mpDisplay, "XdndStatus", False );
    mnXdndTypeList          = XInternAtom( mpDisplay, "XdndTypeList", False );
    mnXdndPosition          = XInternAtom( mpDisplay, "XdndPosition", False );
    mnXdndDrop              = XInternAtom( mpDisplay, "XdndDrop", False );
    mnXdndActionCopy        = XInternAtom( mpDisplay, "XdndActionCopy", False );
    mnXdndActionMove        = XInternAtom( mpDisplay, "XdndActionMove", False );
    mnXdndActionLink        = XInternAtom( mpDisplay, "XdndActionLink", False );
    mnXdndActionCopy        = XInternAtom( mpDisplay, "XdndActionCopy", False );
    mnXdndActionAsk         = XInternAtom( mpDisplay, "XdndActionAsk", False );
    mnXdndActionDescription = XInternAtom( mpDisplay, "XdndActionDescription", False );
    mnXdndFinished          = XInternAtom( mpDisplay, "XdndFinished", False );      mnXdndActionList        = XInternAtom( mpDisplay, "XdndActionList", False );

    aIntegratorList.Insert( this, LIST_APPEND );
    aHomeDir = String( getenv( "HOME" ), gsl_getSystemTextEncoding() );
}

DtIntegrator::~DtIntegrator()
{
    XDestroyWindow( mpDisplay, maSelectionWindow );
    if( mpLastData )
        delete mpLastData;
    while( maDropTypes.Count() )
        delete maDropTypes.Remove( (ULONG)0 );
    while( maDragTypes.Count() )
        delete maDragTypes.Remove( (ULONG)0 );
}

BOOL DtIntegrator::CheckUnxClipboardChanged()
{
    XLIB_Window aPrimWindow = XGetSelectionOwner( mpDisplay, XA_PRIMARY );
    XLIB_Window aClipWindow = XGetSelectionOwner( mpDisplay, mnClipboardAtom );

    if( aPrimWindow != maSelectionWindow  && aPrimWindow != None )
        return TRUE;
    if( aClipWindow != maSelectionWindow && aClipWindow != None )
        return TRUE;
    return FALSE;
}

void DtIntegrator::Copy( DtData* pData )
{
    if( mpLastData )
        delete mpLastData;
    mpLastData = pData;

    XSetSelectionOwner( mpDisplay, XA_PRIMARY,
                        maSelectionWindow, CurrentTime );
    XSetSelectionOwner( mpDisplay, mnClipboardAtom,
                        maSelectionWindow, CurrentTime );

#if defined DBG_UTIL || defined DEBUG
    if( XGetSelectionOwner( mpDisplay, XA_PRIMARY ) != maSelectionWindow )
        fprintf( stderr, "Could not acquire ownership of PRIMARY\n" );
    if( XGetSelectionOwner( mpDisplay, mnClipboardAtom ) !=
        maSelectionWindow )
        fprintf( stderr, "Could not acquire ownership of CLIPBOARD\n" );
#endif
}

void DtIntegrator::RegisterDropzone( SalFrame* pFrame )
{
    for( int i = 0; i < maDropzones.Count(); i++ )
    {
        if( maDropzones.GetObject( i ) == pFrame )
            return;
    }
    maDropzones.Insert( pFrame, LIST_APPEND );

    // create XdndAware property on window to acknowledge DND awareness
    static int nVersion = XDND_PROTOCOL_VERSION;
    XChangeProperty( mpDisplay, pFrame->maFrameData.GetWindow(),
                     mnXdndAware, XA_ATOM, 32, PropModeReplace,
                     (unsigned char*)(&nVersion), 1 );

    ImplRegisterDropzone( pFrame );
}

void DtIntegrator::UnregisterDropzone( SalFrame* pFrame )
{
    maDropzones.Remove( pFrame );

    XDeleteProperty( mpDisplay, pFrame->maFrameData.GetWindow(), mnXdndAware );

    ImplUnregisterDropzone( pFrame );
}

void DtIntegrator::ImplRegisterDropzone( SalFrame* pFrame )
{
}

void DtIntegrator::ImplUnregisterDropzone( SalFrame* pFrame )
{
}

void DtIntegrator::ImplHandleXEvent( XEvent* pEvent )
{
    if( pEvent->type == SelectionClear )
    {
        if( meDragState != DtDragNone &&
            pEvent->xselectionclear.selection == mnXdndSelection )
        {
            meDragState = DtDragNone;
            maDragSource = None;
            maDragTarget = None;
        }
        else
            maClipboardChangedHdl.Call( this );
    }
    else if( pEvent->type == SelectionRequest )
    {
        BOOL bConvertable = FALSE;
        XSelectionRequestEvent& rSelEvent = pEvent->xselectionrequest;
        XEvent aSendEvent;
        aSendEvent.type                   = SelectionNotify;
        aSendEvent.xselection.display     = rSelEvent.display;
        aSendEvent.xselection.send_event  = True;
        aSendEvent.xselection.requestor   = rSelEvent.requestor;
        aSendEvent.xselection.selection   = rSelEvent.selection;
        aSendEvent.xselection.time        = rSelEvent.time;
            // xterm seems to be the only one to care about time = CurrentTime

        // clipboard and xdnd requests land here
        if( rSelEvent.selection == XA_PRIMARY ||
            rSelEvent.selection == mnClipboardAtom )
        {

#if defined DEBUG
            char* pType = XGetAtomName( mpDisplay, rSelEvent.target );
            fprintf( stderr, "Request for conversion of %s with type \"%s\"\n",

                     rSelEvent.selection == XA_PRIMARY ? "PRIMARY" : "CLIPBOARD", pType );
            XFree( pType );
#endif
            if( ( rSelEvent.target == XA_STRING ||
                  rSelEvent.target == mnCompoundAtom )
                && mpLastData && mpLastData->mnBytes )
            {
                bConvertable = TRUE;
                XChangeProperty( mpDisplay, rSelEvent.requestor,
                                 rSelEvent.property, XA_STRING,
                                 8, PropModeReplace,
                                 mpLastData->mpBytes,
                                 mpLastData->mnBytes );
                aSendEvent.xselection.target      = rSelEvent.target;
            }
            else if( rSelEvent.target == mnTargetsAtom )
            {
                bConvertable = TRUE;
                Atom aAtom = XA_STRING;
                XChangeProperty( mpDisplay, rSelEvent.requestor,
                                 rSelEvent.property, XA_ATOM,
                                 32, PropModeReplace,
                                 (unsigned char*)&aAtom, 1 );
                aSendEvent.xselection.target        = mnTargetsAtom;
            }
        }
        else if( rSelEvent.selection == mnXdndSelection && meDragState != DtDragNone )
        {
            DtData aData;
            aData.mpType = (unsigned char*)XGetAtomName( mpDisplay, rSelEvent.target );
#if defined DEBUG
            fprintf( stderr, "Request for convertion of XdndSelection with type \"%s\"\n", aData.mpType );
#endif
            maQueryDragDataHdl.Call( &aData );
            if( aData.mnBytes > 0 )
            {
                bConvertable = TRUE;
                XChangeProperty( mpDisplay, rSelEvent.requestor,
                                 rSelEvent.property, rSelEvent.target,
                                 8, PropModeReplace,
                                 aData.mpBytes,
                                 aData.mnBytes );
                aSendEvent.xselection.target      = rSelEvent.target;
            }
            XFree( aData.mpType );
            // drag and drop does NOT end here
            // the target may ask more than once for the data
            // hopefully this can work with the office
        }
        aSendEvent.xselection.property    =
            bConvertable ? rSelEvent.property : None;
        if( ! XSendEvent( mpDisplay, rSelEvent.requestor,
                          False, 0, &aSendEvent ) )
            fprintf( stderr, "HandleSelectionRequest: XSendEvent failed\n" );
    }
    else if( pEvent->type == ClientMessage )
    {
        int i = pEvent->xclient.message_type;

        if( i == mnXdndEnter &&
            ( pEvent->xclient.data.l[1] >> 24 ) <= XDND_PROTOCOL_VERSION &&
            ! mpDropTarget )
        {
#if defined DEBUG
            fprintf( stderr, "XdndEnter with XDND protocol version %d\n",
                     pEvent->xclient.data.l[1] >> 24 );
#endif
            // a drop begins

            // search the correct target frame
            for( i = 0; i < maDropzones.Count(); i++ )
            {
                SalFrame *pFrame = maDropzones.GetObject( i );
                if( pFrame->maFrameData.GetWindow() ==
                    pEvent->xclient.window )
                {
                    mpDropTarget = pFrame;
                    break;
                }
            }
            if( ! mpDropTarget )
            {
#if defined DEBUG || defined DBG_UTIL
                fprintf( stderr, "Received XdndEnter and have no corresponding dropzone !!!\n" );
#endif
                return;
            }

            maDropSource = pEvent->xclient.data.l[0];
            // build a stringlist of types
            while( maDropTypes.Count() )
                delete maDropTypes.Remove( (ULONG)0 );
            for( int i = 2; i < 5; i++ )
            {
                if( pEvent->xclient.data.l[i] != None )
                {
                    char *pAtomName = XGetAtomName( mpDisplay, pEvent->xclient.data.l[i] );
                    maDropTypes.Insert( new String( pAtomName, RTL_TEXTENCODING_ISO_8859_1 ) );
                    XFree( pAtomName );
                }
            }
            if( maDropTypes.Count() > 2 && (pEvent->xclient.data.l[1] & 1) )
            {
                unsigned long nCount;
                Atom aType;
                int nFormat;
                unsigned long nBytesLeft;
                Atom* pTypeData = 0;
                if( XGetWindowProperty( mpDisplay, maDropSource,
                                        mnXdndTypeList, 0, 256,
                                        False, XA_ATOM,
                                        &aType, &nFormat,
                                        &nCount, &nBytesLeft,
                                        (unsigned char**)(&pTypeData) ) )
                {
                    if( aType == XA_ATOM && nFormat == 32 && nCount )
                    {
                        while( nCount )
                        {
                            char * pTypeName =
                                XGetAtomName( mpDisplay, pTypeData[ --nCount ] );
                            maDropTypes.Insert( new String( pTypeData[ nCount ], RTL_TEXTENCODING_ISO_8859_1 ) );
                        }
                    }
                    if( pTypeData )
                        XFree( pTypeData );
                }
            }

            maBeginDropHdl.Call( NULL );
        }
        else if( i == mnXdndPosition && maDropSource != None && mpDropTarget )
        {
#if defined DEBUG
            fprintf( stderr, "Received XdndPosition\n" );
#endif
            // remember time stamps
            mnDropDataTime = pEvent->xclient.data.l[3];

            // put x,y and action into a structure here
            DtDropQuery aDropQuery;
            aDropQuery.m_pFrame = mpDropTarget;
            int x = pEvent->xclient.data.l[2] >> 16;
            int y = pEvent->xclient.data.l[2] & 0xffff;
            XLIB_Window aChild;
            XTranslateCoordinates( mpDisplay, DefaultRootWindow( mpDisplay ),
                                   mpDropTarget->maFrameData.GetWindow(),
                                   x, y,
                                   &aDropQuery.m_nX,
                                   &aDropQuery.m_nY,
                                   &aChild );
            if( pEvent->xclient.data.l[4] == mnXdndActionLink )
                aDropQuery.m_eAction = DtDropLink;
            else if( pEvent->xclient.data.l[4] == mnXdndActionMove )
                aDropQuery.m_eAction = DtDropMove;
            else
                aDropQuery.m_eAction = DtDropCopy;

            mnLastDropX = aDropQuery.m_nX;
            mnLastDropY = aDropQuery.m_nY;

            // query for dropping on new position
            DtDropAction eResult = (DtDropAction)maQueryDropHdl.Call( &aDropQuery );
#if defined DEBUG
            fprintf( stderr, "QueryDropHdl returned result " );
            switch( eResult )
            {
                case DtDropNone: fprintf( stderr, "DtDropNone\n\n" );break;
                case DtDropCopy: fprintf( stderr, "DtDropCopy\n\n" );break;
                case DtDropLink: fprintf( stderr, "DtDropLink\n\n" );break;
                case DtDropMove: fprintf( stderr, "DtDropMove\n\n" );break;
                case DtDropAny: fprintf( stderr, "DtDropAny\n\n" );break;
                default: fprintf( stderr, "Unknown !!!\n\n" );break;
            }
#endif

            // give source a status
            XEvent aSendEvent;
            aSendEvent.type = ClientMessage;
            aSendEvent.xclient.display      = mpDisplay;
            aSendEvent.xclient.window       = maDropSource;
            aSendEvent.xclient.message_type = mnXdndStatus;
            aSendEvent.xclient.format       = 32;
            aSendEvent.xclient.data.l[0]    = mpDropTarget->maFrameData.GetWindow();
            aSendEvent.xclient.data.l[1]    = 2;
            switch( eResult )
            {
                case DtDropCopy:
                case DtDropMove:
                case DtDropLink:
                case DtDropAny:
                    aSendEvent.xclient.data.l[1] |= 1;
                default: ;
            }
            aSendEvent.xclient.data.l[2]    = 0;
            aSendEvent.xclient.data.l[3]    = 0;
            if( eResult == DtDropCopy )
                aSendEvent.xclient.data.l[4] = mnXdndActionCopy;
            else if( eResult == DtDropMove )
                aSendEvent.xclient.data.l[4] = mnXdndActionMove;
            else if( eResult == DtDropLink )
                aSendEvent.xclient.data.l[4] = mnXdndActionLink;
            else
                aSendEvent.xclient.data.l[4] = None;
            XSendEvent( mpDisplay, maDropSource, False, NoEventMask, &aSendEvent );
        }
        else if( i == mnXdndLeave && maDropSource != None )
        {
#if defined DEBUG
            fprintf( stderr, "Received XdndLeave\n" );
#endif
            if( pEvent->xclient.data.l[0] == maDropSource )
            {
                maDropSource = None;
                mpDropTarget = NULL;

                DtDropQuery aDropQuery;
                aDropQuery.m_nX         = mnLastDropX;
                aDropQuery.m_nY         = mnLastDropY;
                aDropQuery.m_eAction    = DtDropNone;
                aDropQuery.m_pFrame     = mpDropTarget;
                maDropFinishHdl.Call( &aDropQuery );

            }
        }
        else if( i == mnXdndDrop && maDropSource != None )
        {
#if defined DEBUG
            fprintf( stderr, "Received XdndDrop\n" );
#endif
            // remember time stamps
            mnDropDataTime = pEvent->xclient.data.l[2];

            DtDropQuery aDropQuery;
            aDropQuery.m_nX         = mnLastDropX;
            aDropQuery.m_nY         = mnLastDropY;
            aDropQuery.m_eAction    = DtDropAny;
            aDropQuery.m_pFrame     = mpDropTarget;
            // the drop finish handler should relay the data types
            // FinishDrop should be called then which retrieves the data
            // FinishDrop must not be called  after this handler returns
            maDropFinishHdl.Call( &aDropQuery );
            // finally send a drop finished
            XEvent aSendEvent;
            aSendEvent.type = ClientMessage;
            aSendEvent.xclient.display      = mpDisplay;
            aSendEvent.xclient.window       = maDropSource;
            aSendEvent.xclient.message_type = mnXdndFinished;
            aSendEvent.xclient.format       = 32;
            aSendEvent.xclient.data.l[0]    = mpDropTarget->maFrameData.GetWindow();
            aSendEvent.xclient.data.l[1]    = 0;
            aSendEvent.xclient.data.l[2]    = 0;
            aSendEvent.xclient.data.l[3]    = 0;
            aSendEvent.xclient.data.l[4]    = 0;

            XSendEvent( mpDisplay, maDropSource, True, NoEventMask, &aSendEvent );
            maDropSource = None;
            mpDropTarget = NULL;
        }
        else if( meDragState != DtDropNone &&
                 meDragState != DtWaitForDataRequest &&
                 pEvent->xclient.message_type == mnXdndStatus )
        {
            int nAction = pEvent->xclient.data.l[4];
            // set pointer correctly
            XLIB_Cursor aCursor;
            if( pEvent->xclient.data.l[1] & 1 )
            {
                if( nAction == mnXdndActionCopy )
                    aCursor = mpSalDisplay->GetPointer( POINTER_COPYFILES );
                else if( nAction == mnXdndActionMove )
                    aCursor = mpSalDisplay->GetPointer( POINTER_MOVEFILES );
                else if( nAction == mnXdndActionLink )
                    aCursor = mpSalDisplay->GetPointer( POINTER_LINKFILE );
                else
                    aCursor = mpSalDisplay->GetPointer( POINTER_CROP );
            }
            else
                // target will not accept the drop
                aCursor = mpSalDisplay->GetPointer( POINTER_NOTALLOWED );
            XChangeActivePointerGrab( mpDisplay,
                                      EVENTMASK_WHILE_DRAGGING,
                                      aCursor, CurrentTime );
#if defined DEBUG
            char* pAction = nAction != None ?
                     XGetAtomName( mpDisplay, nAction ) : "None";
            fprintf( stderr, "Received XdndStatus %s(%s)\n",
                     pEvent->xclient.data.l[1] & 1 ? "accept" : "deny",
                     pAction );
            if( nAction != None )
                XFree( pAction );
#endif
            meDragState = DtDragging;
            SendXdndPosition( pEvent );
        }
        else if( meDragState != DtDragNone &&
                 pEvent->xclient.message_type == mnXdndFinished )
        {
#if defined DEBUG
            fprintf( stderr, "Received XdndFinished\n" );
#endif
            maDragTarget = None;
            maDragSource = None;
            meDragState = DtDragNone;
        }
    }
    else if( pEvent->type == MotionNotify   ||
             pEvent->type == XLIB_KeyPress      ||
             pEvent->type == KeyRelease
             )
    {
        if( pEvent->type == XLIB_KeyPress )
        {
            char sDummy[2];
            sDummy[0] = sDummy[1] = 0;
            KeySym nKeySym = 0;
            XLookupString( &pEvent->xkey, sDummy, 1, &nKeySym, NULL );
            if( nKeySym == XK_Escape )
                meDragState = DtDragNone;
        }
        if( meDragState != DtDragNone               &&
            meDragState != DtWaitForDataRequest )

        {
            // find XdndAware window beneath pointer
            int nXdndVersion;
            XLIB_Window aWindow = GetXdndAwareWindowBeneathPointer( nXdndVersion, pEvent );
            if( maDragTarget != aWindow )
            {
                // send a XdndLeave to previous window
                SendXdndLeave();
                // then send a XdndEnter to new window
                maDragTarget = aWindow;
                SendXdndEnter();
            }
            // send a XdndPosition
            SendXdndPosition( pEvent );
        }
        else
            CheckXdndTimeout( pEvent->xmotion.time );
    }
    else if( pEvent->type == ButtonRelease )
    {
        if( meDragState != DtDragNone   &&
            meDragState != DtWaitForDataRequest )
        {
            int nXdndVersion;
            XLIB_Window aWindow = GetXdndAwareWindowBeneathPointer( nXdndVersion, pEvent );
            if( maDragTarget != aWindow )
            {
                SendXdndLeave();
                maDragTarget = aWindow;
                SendXdndEnter();
            SendXdndPosition( pEvent );
            }
            if( maDragTarget != None )
            {
                // send XdndDrop
                XEvent aEvent;
                aEvent.type = ClientMessage;
                aEvent.xclient.display = mpDisplay;
                aEvent.xclient.window = maDragTarget;
                aEvent.xclient.format = 32;
                aEvent.xclient.message_type = mnXdndDrop;
                aEvent.xclient.data.l[0] = maDragSource;
                aEvent.xclient.data.l[1] = 0;
                aEvent.xclient.data.l[2] = pEvent->xbutton.time;
                XSendEvent( mpDisplay, maDragTarget, False,
                            NoEventMask, &aEvent );
#if defined DEBUG
                fprintf( stderr, "Sending XdndDrop\n" );
#endif
                meDragState = DtWaitForDataRequest;
                mnWaitTimestamp = pEvent->xbutton.time;
            }
            else
            {
#if defined DEBUG
                fprintf( stderr, "Received ButtonRelease and no DragTarget\n" );
#endif
                maDragTarget = None;
                maDragSource = None;
                meDragState = DtDragNone;
            }
        }
        else
            CheckXdndTimeout( pEvent->xbutton.time );
    }
}

DtData* DtIntegrator::DropFinish( const String& rType )
{
    ByteString aTypeString( rType, RTL_TEXTENCODING_ISO_8859_1 );
#if defined DEBUG
    fprintf( stderr, "DtIntegerator::DropFinish( \"%s\" ) : ", aTypeString.GetBuffer() );
#endif
    if( maDropSource == None || mpDropTarget == NULL )
    {
#if defined DEBUG
        fprintf( stderr, "failed\n" );
#endif
        return NULL;
    }

    DtData* pData = new DtData;
    pData->meType = DtTypeKnown;
    pData->mnX = mnLastDropX;
    pData->mnY = mnLastDropY;

    Atom aType = XInternAtom( mpDisplay, aTypeString.GetBuffer(), False );

    // now initiate communication with selection owner
    XConvertSelection( mpDisplay, mnXdndSelection,
                       aType, maExPropertyAtom, maSelectionWindow, mnDropDataTime );
    // wait for selection notify
    XEvent aXEvent;
    int nTries;
    for( nTries = 0;
         ! XCheckTypedEvent( mpDisplay, SelectionNotify, &aXEvent ) &&
             nTries <= MAX_TRY_CONVERTSELECTION ;
         nTries++ )
        usleep( 50000 );

    // check if request could be converted
    if( nTries >= MAX_TRY_CONVERTSELECTION ||
        aXEvent.xselection.property != maExPropertyAtom )
    {
        delete pData;
        pData = 0;
    }
    else
    {
        Atom aActualType;
        int  nActualFormat;
        unsigned long nItems;
        unsigned long nBytesAfter;
        unsigned char *pBytes=0;
        XGetWindowProperty( mpDisplay, maSelectionWindow, maExPropertyAtom,
                            0,          // long_offset
                            65536,      // long_length, a maximum of 256k will be retrieved
                            True,       // delete property after reading
                            aType,      // the type we want
                            &aActualType, &nActualFormat,
                            &nItems, &nBytesAfter, &pBytes );

        if( pBytes )
        {
            pData->mnBytes = nItems * (nActualFormat/8);
            pData->mpBytes = new unsigned char[ pData->mnBytes + 1 ];
            memcpy( pData->mpBytes, pBytes, pData->mnBytes );
            pData->mpBytes[ pData->mnBytes ] = 0;
            XFree( pBytes );
        }
        else
        {
            delete pData;
            pData = 0;
        }
    }

#if defined DEBUG
    fprintf( stderr, "%s\n", pData ? "success" : "failed" );
#endif
    return pData;
}

DtData* DtIntegrator::Paste()
{
    DtData* pDtData = 0;

    // check if there IS something to copy
    XLIB_Window aOwner = XGetSelectionOwner( mpDisplay, XA_PRIMARY );
    if( aOwner == None || aOwner == maSelectionWindow )
        return 0;

    // get the proper Atoms
    Atom aTypeAtom   = XA_STRING;

    // now initiate communication with selection owner
    XConvertSelection( mpDisplay, XA_PRIMARY, aTypeAtom, maExPropertyAtom,
                       maSelectionWindow, CurrentTime );
    // wait for selection notify
    XEvent aXEvent;
    int nTries;
    for( nTries = 0;
         ! XCheckTypedEvent( mpDisplay, SelectionNotify, &aXEvent ) &&
             nTries <= MAX_TRY_CONVERTSELECTION ;
         nTries++ )
        usleep( 50000 );

    // check if request could be converted
    if( nTries >= MAX_TRY_CONVERTSELECTION ||
        aXEvent.xselection.property != maExPropertyAtom )
    {
        // let us try CLIPBOARD selection
        XConvertSelection( mpDisplay, mnClipboardAtom, aTypeAtom,
                           maExPropertyAtom, maSelectionWindow, CurrentTime );
        for( nTries = 0;
             ! XCheckTypedEvent( mpDisplay, SelectionNotify, &aXEvent ) &&
                 nTries < MAX_TRY_CONVERTSELECTION ;
             nTries++ )
            usleep( 50000 );
    }

    // check if request could be converted
    if( nTries >= MAX_TRY_CONVERTSELECTION ||
        aXEvent.xselection.property != maExPropertyAtom )
        return 0;

    Atom aActualType;
    int  nActualFormat;
    unsigned long nItems;
    unsigned long nBytesAfter;
    unsigned char *pData=0;
    XGetWindowProperty( mpDisplay, maSelectionWindow, maExPropertyAtom,
                        0,          // long_offset
                        16384,      // long_length, a maximum of 64k will e retrieved
                        True,       // delete property after reading
                        XA_STRING,  // the type we want
                        &aActualType, &nActualFormat,
                        &nItems, &nBytesAfter, &pData );

    if( pData )
    {
        pDtData = new DtData;
        pDtData->meType = DtTypeText;
        pDtData->mnBytes = nItems * (nActualFormat/8);
        pDtData->mpBytes = new unsigned char[ pDtData->mnBytes + 1 ];
        memcpy( pDtData->mpBytes, pData, pDtData->mnBytes );
        pDtData->mpBytes[ pDtData->mnBytes ] = 0;
        pDtData->mnBytes += 1;
        XFree( pData );
        return pDtData;
    }
    return 0;
}

BOOL DtIntegrator::StartProcess( String& rFile, String& rParams, const String& rDir )
{
    String aFiles( rFile );
    if( rParams.Len() )
    {
        aFiles += ' ';
        aFiles += rParams;
    }
    // try to launch it
    return LaunchProcess( aFiles, rDir );
}

DtIntegrator* DtIntegrator::CreateDtIntegrator( SalFrame* pFrame )
{
    // hack for sclient
    if( ! pFrame && aIntegratorList.Count() )
        return aIntegratorList.GetObject( 0 );

    for( int i = 0; i < aIntegratorList.Count(); i++ )
    {
        DtIntegrator* pIntegrator = aIntegratorList.GetObject( i );
        if( pIntegrator->mpDisplay == pFrame->maFrameData.GetXDisplay() )
            return pIntegrator;
    }

    if( ! pFrame )
        pFrame = GetSalData()->pFirstFrame_;

#ifndef REMOTE_APPSERVER
    Display* pDisplay = pFrame->maFrameData.GetXDisplay();
    Atom nDtAtom;
    void* pLibrary = NULL;

#ifdef USE_CDE
    // check dt type
    // CDE
    nDtAtom = XInternAtom( pDisplay, "_DT_WM_READY", True );
    if( nDtAtom && ( pLibrary = _LoadLibrary( "libDtSvc.so" ) ) )
    {
        // performance: do not dlopen DtSvc twice
        CDEIntegrator::pDtSvcLib = pLibrary;
        return new CDEIntegrator( pFrame );
    }
#endif

    nDtAtom = XInternAtom( pDisplay, "KWM_RUNNING", True );
    if( nDtAtom ) // perhaps should check getenv( "KDEDIR" )
        return new KDEIntegrator( pFrame );
#endif
    // default: generic implementation
    return new DtIntegrator( pFrame );
}

void DtIntegrator::HandleXEvent( XEvent* pEvent )
{
    Display *pDisplay = pEvent->xany.display;
    for( int i = 0; i < aIntegratorList.Count(); i++ )
    {
        DtIntegrator* pIntegrator = aIntegratorList.GetObject( i );
        if( pIntegrator->mpDisplay == pDisplay )
        {
            pIntegrator->ImplHandleXEvent( pEvent );
            break;
        }
    }
}

BOOL DtIntegrator::LaunchProcess( const String& rParam, const String& rDirectory )
{
    int nArg;

    char *pDisplayName = DisplayString( mpDisplay );
    int nToken = GetCommandLineTokenCount( rParam );

    ::rtl::OUString* pArgs = new ::rtl::OUString[nToken];
    for( nArg = 0; nArg < nToken ; nArg++ )
        pArgs[ nArg ] = GetCommandLineToken( nArg, rParam );
    NAMESPACE_VOS(OArgumentList) aArgList( pArgs+1, nToken-1 );
    delete pArgs;

    ::rtl::OUString aDisplay;
    if( pDisplayName )
    {
        aDisplay = ::rtl::OUString::createFromAscii( "DISPLAY=" );
        aDisplay += ::rtl::OUString::createFromAscii( pDisplayName );
    }
    NAMESPACE_VOS(OEnvironment) aEnvironment( 1, &aDisplay );

    NAMESPACE_VOS( OProcess ) aOProcess( pArgs[0], rDirectory );

    BOOL bSuccess = aOProcess.execute(
        ( NAMESPACE_VOS( OProcess )::TProcessOption)
        ( NAMESPACE_VOS( OProcess )::TOption_Detached |
          NAMESPACE_VOS( OProcess )::TOption_SearchPath ),
        aArgList, aEnvironment )
        == NAMESPACE_VOS( OProcess )::E_None ? TRUE : FALSE;

    return bSuccess;
}

DtDropAction DtIntegrator::ExecuteDrag( const StringList& rTypes, SalFrame* pDragFrame )
{
    int i;
    Atom* pTypes = new Atom[rTypes.Count() ];

#if defined DEBUG
    fprintf( stderr, "DtIntegrator::ExecuteDrag: " );
#endif
    meDragState = DtDragging;

    if( ! pDragFrame )
        pDragFrame = maDropzones.GetObject( maDropzones.Count()-1 );
    else
    {
        for( i = 0; i < maDropzones.Count(); i++ )
            if( maDropzones.GetObject( i ) == pDragFrame )
                break;
        if( i >= maDropzones.Count() )
        {
#if defined DEBUG
            fprintf( stderr, "DragFrame is no Dropzone !!! taking last dropzone as source instead ...\n" );
#endif
            pDragFrame = maDropzones.GetObject( maDropzones.Count()-1 );
        }
    }

    maDragSource = pDragFrame->maFrameData.GetWindow();
    // set up drag types
    while( maDragTypes.Count() )
        delete maDragTypes.Remove( (ULONG)0 );
    for( i = 0; i < rTypes.Count(); i++ )
    {
        maDragTypes.Insert( new String( *rTypes.GetObject( i ) ), LIST_APPEND );
        pTypes[ i ] = XInternAtom( mpDisplay,
                                   ByteString( *rTypes.GetObject( i ), RTL_TEXTENCODING_ISO_8859_1 ).GetBuffer(),
                                   False );
#if defined DEBUG
        fprintf( stderr, " \"%s\"",
                 ByteString( *rTypes.GetObject( i ), RTL_TEXTENCODING_ISO_8859_1 ).GetBuffer() );
#endif
    }
#if defined DEBUG
    fprintf( stderr, "\n" );
#endif

    // set the types in the property XdndTypeList
    XChangeProperty( mpDisplay, maDragSource, mnXdndTypeList, XA_ATOM,
                     32, PropModeReplace, (unsigned char*)pTypes,
                     rTypes.Count() );
    delete pTypes;

    XLIB_Cursor aCur = mpSalDisplay->GetPointer( POINTER_MOVEFILES );
    XLIB_Window aRoot = DefaultRootWindow( mpDisplay );

    // set cursor
    XGrabPointer( mpDisplay, aRoot, False,
                  EVENTMASK_WHILE_DRAGGING,
                  GrabModeAsync, GrabModeAsync, None, aCur, CurrentTime );
    XGrabKeyboard( mpDisplay, maDragSource, False, GrabModeAsync,
                   GrabModeAsync, CurrentTime );
    XSetSelectionOwner( mpDisplay, mnXdndSelection,
                        maDragSource, CurrentTime );

    while( meDragState != DtDragNone )
        Application::Yield();

    XSetSelectionOwner( mpDisplay, mnXdndSelection,
                        None, CurrentTime );
    XUngrabKeyboard( mpDisplay, CurrentTime );
    XUngrabPointer( mpDisplay, CurrentTime );

    return DtDropCopy;
}

XLIB_Window DtIntegrator::GetXdndAwareWindowBeneathPointer( int& rVersion,
                                                            XEvent* pEvent)
{
    XLIB_Window aRoot = DefaultRootWindow( mpDisplay );
    XLIB_Window aWindow = aRoot, aParent = aRoot;
    int nXRoot, nYRoot, nX, nY;

    if( pEvent->type == MotionNotify )
    {
        nX = pEvent->xmotion.x_root;
        nY = pEvent->xmotion.y_root;
    }
    else if( pEvent->type == ButtonPress || pEvent->type == ButtonRelease )
    {
        nX = pEvent->xbutton.x_root;
        nY = pEvent->xbutton.y_root;
    }
    else if( pEvent->type == XLIB_KeyPress || pEvent->type == KeyRelease )
    {
        nX = pEvent->xkey.x_root;
        nY = pEvent->xkey.y_root;
    }

    rVersion = 0;
    while( ! rVersion && aWindow != None )
    {
        XTranslateCoordinates( mpDisplay, aRoot, aParent,
                               nX, nY, &nXRoot, &nYRoot, &aWindow );
        if( aWindow != None )
        {
            aParent = aWindow;

            Atom aActualType;
            int  nActualFormat;
            unsigned long nItems;
            unsigned long nBytesAfter;
            unsigned char *pData=0;
            XGetWindowProperty( mpDisplay, aWindow,
                                mnXdndAware,
                                0, 1, False, XA_ATOM,
                                &aActualType, &nActualFormat,
                                    &nItems, &nBytesAfter, &pData );
            if( pData                   &&
                aActualType  == XA_ATOM &&
                nActualFormat == 32     &&
                nItems == 1 )
            {
                rVersion = *((int*)pData );
            }
            if( pData )
                XFree( pData );
        }
    }
    if( ! rVersion )
        return None;

    return aWindow;
}

void DtIntegrator::SendXdndLeave()
{
    if( maDragTarget != None )
    {
        XEvent aEvent;
        aEvent.type = ClientMessage;
        aEvent.xclient.display = mpDisplay;
        aEvent.xclient.window = maDragTarget;
        aEvent.xclient.format = 32;
        aEvent.xclient.message_type = mnXdndLeave;
        aEvent.xclient.data.l[0] = maDragSource;
        aEvent.xclient.data.l[1] = 0;
        XSendEvent( mpDisplay, maDragTarget, False,
                                NoEventMask, &aEvent );
#if defined DEBUG
        fprintf( stderr, "Sending XdndLeave\n" );
#endif
    }
}

void DtIntegrator::SendXdndEnter()
{
    if( maDragTarget != None )
    {
        int i;
        XEvent aEvent;

        aEvent.type = ClientMessage;
        aEvent.xclient.display = mpDisplay;
        aEvent.xclient.window = maDragTarget;
        aEvent.xclient.format = 32;
        aEvent.xclient.message_type = mnXdndEnter;
        aEvent.xclient.data.l[0] = maDragSource;
        aEvent.xclient.data.l[1] = maDragTypes.Count() > 3 ? 1 : 0;
        aEvent.xclient.data.l[1] |= XDND_PROTOCOL_VERSION << 24;
        aEvent.xclient.data.l[2] = None;
        aEvent.xclient.data.l[3] = None;
        aEvent.xclient.data.l[4] = None;
        for( i = 0; i < maDragTypes.Count() && i < 3; i++ )
            aEvent.xclient.data.l[i+2] =
                XInternAtom( mpDisplay,
                             ByteString( *maDragTypes.GetObject( i ), RTL_TEXTENCODING_ISO_8859_1 ).GetBuffer(),
                             False );
        XSendEvent( mpDisplay, maDragTarget, False,
                    NoEventMask, &aEvent );
#if defined DEBUG
        fprintf( stderr, "Sending XdndEnter\n" );
#endif
    }
}

void DtIntegrator::SendXdndPosition( XEvent* pEvent )
{
    static int nLastSendX = -1;
    static int nLastSendY = -1;
    static int nLastState = 0, nState = 0;

    if( pEvent->type == ButtonPress || pEvent->type == ButtonRelease  )
    {
        mnLastDragX = pEvent->xbutton.x_root;
        mnLastDragY = pEvent->xbutton.y_root;
        mnLastDragTimestamp = pEvent->xbutton.time;
        nState = pEvent->xbutton.state;
    }
    else if( pEvent->type == MotionNotify  )
    {
        mnLastDragX = pEvent->xmotion.x_root;
        mnLastDragY = pEvent->xmotion.y_root;
        mnLastDragTimestamp = pEvent->xmotion.time;
        nState = pEvent->xmotion.state;
    }
    else if( pEvent->type == XLIB_KeyPress || pEvent->type == KeyRelease )
    {
        mnLastDragX = pEvent->xkey.x_root;
        mnLastDragY = pEvent->xkey.y_root;
        mnLastDragTimestamp = pEvent->xkey.time;
        nState = pEvent->xkey.state;
    }
    if( maDragTarget != None                &&
        meDragState != DtWaitForStatus      &&
        meDragState != DtWaitForDataRequest &&
        ( mnLastDragX != nLastSendX ||
          mnLastDragY != nLastSendY ||
          nState != nLastState )
        )
    {
        nLastSendX = mnLastDragX;
        nLastSendY = mnLastDragY;
        nLastState = nState;

        XEvent aEvent;
        aEvent.type = ClientMessage;
        aEvent.xclient.display = mpDisplay;
        aEvent.xclient.window = maDragTarget;
        aEvent.xclient.format = 32;
        aEvent.xclient.message_type = mnXdndPosition;
        aEvent.xclient.data.l[0] = maDragSource;
        aEvent.xclient.data.l[1] = 0;
        aEvent.xclient.data.l[2] =
            ( mnLastDragX << 16 ) | ( mnLastDragY );
        aEvent.xclient.data.l[3] = mnLastDragTimestamp;
        // Insert correct DropAction here
        if( nLastState & ( ShiftMask | ControlMask ) )
            aEvent.xclient.data.l[4] = mnXdndActionLink;
        else if( nLastState & ShiftMask )
            aEvent.xclient.data.l[4] = mnXdndActionMove;
        else if( nLastState & ControlMask )
            aEvent.xclient.data.l[4] = mnXdndActionCopy;
        else
            aEvent.xclient.data.l[4] = mnXdndActionCopy;

        XSendEvent( mpDisplay, maDragTarget, False,
                    NoEventMask, &aEvent );
        meDragState = DtWaitForStatus;
        mnWaitTimestamp = mnLastDragTimestamp;
#if defined DEBUG
        fprintf( stderr, "Sending XdndPosition\n" );
#endif
    }
}

void DtIntegrator::CheckXdndTimeout( int nTime )
{
    if( meDragState == DtWaitForDataRequest &&
        ( nTime - mnWaitTimestamp > 5000 ||
          mnWaitTimestamp > nTime ) )
    {
#if defined DEBUG
        fprintf( stderr, "Timeout on DtWaitForRequestData\n" );
#endif
        maDragTarget = None;
        maDragSource = None;
        meDragState = DtDragNone;
    }
    else if( meDragState == DtWaitForStatus &&
        ( nTime - mnWaitTimestamp > 5000 ||
          mnWaitTimestamp > nTime ) )
    {
#if defined DEBUG
        fprintf( stderr, "Timeout on DtWaitForStatus\n" );
#endif
        meDragState = DtDragging;
    }
}

BOOL DtIntegrator::GetSystemLook( SystemLookInfo& rInfo )
{
    return FALSE;
}
