/*************************************************************************
 *
 *  $RCSfile: twain.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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

#include <string.h>
#include <math.h>

#if defined( WNT ) || defined (WIN)
#include <tools/svwin.h>
#endif
#ifdef OS2
#include <tools/svpm.h>
#endif // OS2
#include <vos/module.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/sysdata.hxx>
#include "twain.hxx"

// -----------
// - Defines -
// -----------

#define PFUNC                       (*pDSM)
#define FIXTODOUBLE( nFix )         ((double)nFix.Whole+(double)nFix.Frac/65536.)
#define FIXTOLONG( nFix )           ((long)floor(FIXTODOUBLE(nFix)+0.5))

#if defined WIN
#define TWAIN_LIBNAME               "TWAIN.DLL"
#define TWAIN_FUNCNAME              "DSM_Entry"
#elif defined WNT
#define TWAIN_LIBNAME               "TWAIN_32.DLL"
#define TWAIN_FUNCNAME              "DSM_Entry"
#elif defined OS2
#define TWAIN_LIBNAME               "twain"
#define TWAIN_FUNCNAME              "DSM_ENTRY"
#endif

// -----------
// - Statics -
// -----------

static ImpTwain* pImpTwainInstance = NULL;

// ---------
// - Procs -
// ---------

#ifdef OS2

    #define PTWAINMSG QMSG*

    MRESULT EXPENTRY TwainWndProc( HWND hWnd, ULONG nMsg, MPARAM nParam1, MPARAM nParam2 )
    {
        return (MRESULT) TRUE;
    }


#else // OS2

    #define PTWAINMSG MSG*

    // -------------------------------------------------------------------------

    LRESULT CALLBACK TwainWndProc( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
    {
        return DefWindowProc( hWnd, nMsg, nPar1, nPar2 );
    }

    // -------------------------------------------------------------------------

    LRESULT CALLBACK TwainMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
    {
        MSG* pMsg = (MSG*) lParam;

        if( ( nCode < 0 ) ||
            ( pImpTwainInstance->hTwainWnd != pMsg->hwnd ) ||
            !pImpTwainInstance->ImplHandleMsg( (void*) lParam ) )
        {
            return CallNextHookEx( pImpTwainInstance->hTwainHook, nCode, wParam, lParam );
        }
        else
        {
            pMsg->message = WM_USER;
            pMsg->lParam = 0;

            return 0;
        }
    }

#endif // OS2

// ------------
// - ImpTwain -
// ------------

ImpTwain::ImpTwain( const Link& rNotifyLink ) :
            aNotifyLink ( rNotifyLink ),
            pDSM        ( NULL ),
            pMod        ( NULL ),
            hTwainWnd   ( 0 ),
            hTwainHook  ( 0 ),
            nCurState   ( 1 )
{
    pImpTwainInstance = this;

    aAppIdent.Id = 0;
    aAppIdent.Version.MajorNum = 1;
    aAppIdent.Version.MinorNum = 0;
    aAppIdent.Version.Language = TWLG_USA;
    aAppIdent.Version.Country = TWCY_USA;
    aAppIdent.ProtocolMajor = TWON_PROTOCOLMAJOR;
    aAppIdent.ProtocolMinor = TWON_PROTOCOLMINOR;
    aAppIdent.SupportedGroups = DG_IMAGE | DG_CONTROL;
    strcpy( aAppIdent.Version.Info, "StarOffice" );
    strcpy( aAppIdent.Manufacturer, "Sun Microsystems");
    strcpy( aAppIdent.ProductFamily,"Office");
    strcpy( aAppIdent.ProductName, "StarOffice");

#ifdef OS2

    hAB = Sysdepen::GethAB();
    ImplFallback( TWAIN_EVENT_QUIT );
    // hTwainWnd = WinCreateWindow( HWND_DESKTOP, WC_FRAME, "dummy", 0, 0, 0, 0, 0, HWND_DESKTOP, HWND_BOTTOM, 0, 0, 0 );

#else

    HWND        hParentWnd = HWND_DESKTOP;
    WNDCLASS    aWc = { 0, &TwainWndProc, 0, sizeof( WNDCLASS ), GetModuleHandle( NULL ),
                        NULL, NULL, NULL, NULL, "TwainClass" };

    RegisterClass( &aWc );
    hTwainWnd = CreateWindowEx( WS_EX_TOPMOST, aWc.lpszClassName, "TWAIN", 0, 0, 0, 0, 0, hParentWnd, NULL, aWc.hInstance, 0 );
    hTwainHook = SetWindowsHookEx( WH_GETMESSAGE, &TwainMsgProc, NULL, GetCurrentThreadId() );

#endif
}

// -----------------------------------------------------------------------------

ImpTwain::~ImpTwain()
{
}

// -----------------------------------------------------------------------------

void ImpTwain::Destroy()
{
    ImplFallback( TWAIN_EVENT_NONE );
    Application::PostUserEvent( LINK( this, ImpTwain, ImplDestroyHdl ), NULL );
}

// -----------------------------------------------------------------------------

BOOL ImpTwain::SelectSource()
{
    TW_UINT16 nRet = TWRC_FAILURE;

    if( !!aBitmap )
        aBitmap = Bitmap();

    ImplOpenSourceManager();

    if( 3 == nCurState )
    {
        TW_IDENTITY aIdent;

        aIdent.Id = 0, aIdent.ProductName[ 0 ] = '\0';
        aNotifyLink.Call( (void*) TWAIN_EVENT_SCANNING );
        nRet = PFUNC( &aAppIdent, NULL, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &aIdent );
    }

    ImplFallback( TWAIN_EVENT_QUIT );

    return( nRet == TWRC_SUCCESS || nRet == TWRC_CANCEL );
}

// -----------------------------------------------------------------------------

BOOL ImpTwain::InitXfer()
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
        aBitmap = Bitmap();

    ImplOpenSourceManager();

    if( 3 == nCurState )
    {
        ImplOpenSource();

        if( 4 == nCurState )
            bRet = ImplEnableSource();
    }

    if( !bRet )
        ImplFallback( TWAIN_EVENT_QUIT );

    return bRet;
}

// -----------------------------------------------------------------------------

Bitmap ImpTwain::GetXferBitmap()
{
    Bitmap aRet( aBitmap );
    aBitmap = Bitmap();
    return aRet;
}

// -----------------------------------------------------------------------------

void ImpTwain::ImplOpenSourceManager()
{
    if( 1 == nCurState )
    {
        pMod = new NAMESPACE_VOS( OModule )();

        if( pMod->load( TWAIN_LIBNAME ) )
        {
            nCurState = 2;

            if( ( ( pDSM = (DSMENTRYPROC) pMod->getSymbol( TWAIN_FUNCNAME ) ) != NULL ) &&
                ( PFUNC( &aAppIdent, NULL, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, &hTwainWnd ) == TWRC_SUCCESS ) )
            {
                nCurState = 3;
            }
        }
        else
        {
            delete pMod;
            pMod = NULL;
        }
    }
}

// -----------------------------------------------------------------------------

void ImpTwain::ImplOpenSource()
{
    if( 3 == nCurState )
    {
        if( ( PFUNC( &aAppIdent, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &aSrcIdent ) == TWRC_SUCCESS ) &&
            ( PFUNC( &aAppIdent, NULL, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &aSrcIdent ) == TWRC_SUCCESS ) )
        {
#ifdef OS2

            // negotiate capabilities

#else

            TW_CAPABILITY   aCap = { CAP_XFERCOUNT, TWON_ONEVALUE, GlobalAlloc( GHND, sizeof( TW_ONEVALUE ) ) };
            TW_ONEVALUE*    pVal = (TW_ONEVALUE*) GlobalLock( aCap.hContainer );

            pVal->ItemType = TWTY_INT16, pVal->Item = 1;
            GlobalUnlock( aCap.hContainer );
            PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &aCap );
            GlobalFree( aCap.hContainer );
#endif

            nCurState = 4;
        }
    }
}

// -----------------------------------------------------------------------------

BOOL ImpTwain::ImplEnableSource()
{
    BOOL bRet = FALSE;

    if( 4 == nCurState )
    {
        TW_USERINTERFACE aUI = { TRUE, TRUE, hTwainWnd };

        aNotifyLink.Call( (void*) TWAIN_EVENT_SCANNING );
        nCurState = 5;

        if( PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, &aUI ) == TWRC_SUCCESS )
            bRet = TRUE;
        else
            nCurState = 4;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL ImpTwain::ImplHandleMsg( void* pMsg )
{
    TW_UINT16   nRet;
    PTWAINMSG   pMess = (PTWAINMSG) pMsg;
    TW_EVENT    aEvt = { pMess, MSG_NULL };

    nRet = PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &aEvt );

    if( aEvt.TWMessage != MSG_NULL )
    {
        switch( aEvt.TWMessage )
        {
            case MSG_XFERREADY:
            {
                ULONG nEvent = TWAIN_EVENT_QUIT;

                if( 5 == nCurState )
                {
                    nCurState = 6;
                    ImplXfer();

                    if( !!aBitmap )
                        nEvent = TWAIN_EVENT_XFER;
                }

                ImplFallback( nEvent );
            }
            break;

            case MSG_CLOSEDSREQ:
                ImplFallback( TWAIN_EVENT_QUIT );
            break;

            default:
            break;
        }
    }
    else
        nRet = TWRC_NOTDSEVENT;

    return( TWRC_DSEVENT == nRet );
}

// -----------------------------------------------------------------------------

void ImpTwain::ImplXfer()
{
    if( nCurState == 6 )
    {
        TW_IMAGEINFO    aInfo;
        TW_UINT32       hDIB = 0;
        long            nWidth = aInfo.ImageWidth;
        long            nHeight = aInfo.ImageLength;
        long            nXRes = FIXTOLONG( aInfo.XResolution );
        long            nYRes = FIXTOLONG( aInfo.YResolution );

        if( PFUNC( &aAppIdent, &aSrcIdent, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &aInfo ) == TWRC_SUCCESS )
        {
            nWidth = aInfo.ImageWidth;
            nHeight = aInfo.ImageLength;
            nXRes = FIXTOLONG( aInfo.XResolution );
            nYRes = FIXTOLONG( aInfo.YResolution );
        }
        else
            nWidth = nHeight = nXRes = nYRes = -1L;

        switch( PFUNC( &aAppIdent, &aSrcIdent, DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hDIB ) )
        {
            case( TWRC_CANCEL ):
                nCurState = 7;
            break;

            case( TWRC_XFERDONE ):
            {
#ifdef OS2

                // get OS/2-Bitmap

#else // OS2
                const ULONG nSize = GlobalSize( (HGLOBAL) hDIB );
                char*       pBuf = (char*) GlobalLock( (HGLOBAL) hDIB );

                if( pBuf )
                {
                    SvMemoryStream aMemStm;
                    aMemStm.SetBuffer( pBuf, nSize, FALSE, nSize );
                    aBitmap.Read( aMemStm, FALSE );
                    GlobalUnlock( (HGLOBAL) hDIB );
                }

                GlobalFree( (HGLOBAL) hDIB );
#endif // OS2

                // set resolution of bitmap if neccessary
                if ( ( nXRes != -1 ) && ( nYRes != - 1 ) && ( nWidth != - 1 ) && ( nHeight != - 1 ) )
                {
                    const MapMode aMapMode( MAP_100TH_INCH, Point(), Fraction( 100, nXRes ), Fraction( 100, nYRes ) );
                    aBitmap.SetPrefMapMode( aMapMode );
                    aBitmap.SetPrefSize( Size( nWidth, nHeight ) );
                }

                nCurState = 7;
            }
            break;

            default:
            break;
        }
    }
}

// -----------------------------------------------------------------------------

void ImpTwain::ImplFallback( ULONG nEvent )
{
    Application::PostUserEvent( LINK( this, ImpTwain, ImplFallbackHdl ), (void*) nEvent );
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpTwain, ImplFallbackHdl, void*, pData )
{
    const ULONG nEvent = (ULONG) pData;
    BOOL        bFallback = TRUE;

    switch( nCurState )
    {
        case( 7 ):
        case( 6 ):
        {
            TW_PENDINGXFERS aXfers;

            if( PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &aXfers ) == TWRC_SUCCESS )
            {
                if( aXfers.Count != 0 )
                    PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, &aXfers );
            }

            nCurState = 5;
        }
        break;

        case( 5 ):
        {
            TW_USERINTERFACE aUI = { TRUE, TRUE, hTwainWnd };

            PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, &aUI );
            nCurState = 4;
        }
        break;

        case( 4 ):
        {
            PFUNC( &aAppIdent, NULL, DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, &aSrcIdent );
            nCurState = 3;
        }
        break;

        case( 3 ):
        {
            PFUNC( &aAppIdent, NULL, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &hTwainWnd );
            nCurState = 2;
        }
        break;

        case( 2 ):
        {
            delete pMod;
            pMod = NULL;
            nCurState = 1;
        }
        break;

        default:
        {
            if( nEvent != TWAIN_EVENT_NONE )
                aNotifyLink.Call( (void*) nEvent );

            bFallback = FALSE;
        }
        break;
    }

    if( bFallback )
        ImplFallback( nEvent );

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImpTwain, ImplDestroyHdl, void*, p )
{
#ifdef OS2

    if( hWndTwain )
        WinDestroyWindow( hWndTwain );

    // unset hook

#else

    if( hTwainWnd )
        DestroyWindow( hTwainWnd );

    if( hTwainHook )
        UnhookWindowsHookEx( hTwainHook );

#endif

    delete this;
    pImpTwainInstance = NULL;

    return 0L;
}
