/*************************************************************************
 *
 *  $RCSfile: scanwin.cxx,v $
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

#include <math.h>
#include <tools/svwin.h>
#include <tools/stream.hxx>
#include <vos/mutex.hxx>
#include <vos/module.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/salbtype.hxx>
#include "scanner.hxx"
#include "twain/twain.h"

// -----------
// - Defines -
// -----------

#define TWAIN_SELECT            0x00000001UL
#define TWAIN_ACQUIRE           0x00000002UL
#define TWAIN_TERMINATE         0xFFFFFFFFUL

#define TWAIN_EVENT_NONE        0x00000000UL
#define TWAIN_EVENT_QUIT        0x00000001UL
#define TWAIN_EVENT_SCANNING    0x00000002UL
#define TWAIN_EVENT_XFER        0x00000004UL

#define PFUNC                   (*pDSM)
#define PTWAINMSG               MSG*
#define FIXTODOUBLE( nFix )     ((double)nFix.Whole+(double)nFix.Frac/65536.)
#define FIXTOLONG( nFix )       ((long)floor(FIXTODOUBLE(nFix)+0.5))

#if defined WIN
#define TWAIN_LIBNAME           "TWAIN.DLL"
#define TWAIN_FUNCNAME          "DSM_Entry"
#elif defined WNT
#define TWAIN_LIBNAME           "TWAIN_32.DLL"
#define TWAIN_FUNCNAME          "DSM_Entry"
#endif

// --------------
// - TwainState -
// --------------

enum TwainState
{
    TWAIN_STATE_NONE = 0,
    TWAIN_STATE_SCANNING = 1,
    TWAIN_STATE_DONE = 2,
    TWAIN_STATE_CANCELED = 3
};

// ------------
// - ImpTwain -
// ------------

class ImpTwain
{
    ScannerManager*             mpMgr;
    TW_IDENTITY                 aAppIdent;
    TW_IDENTITY                 aSrcIdent;
    Link                        aNotifyLink;
    DSMENTRYPROC                pDSM;
    NAMESPACE_VOS( OModule )*   pMod;
    ULONG                       nCurState;

    void                        ImplCreate();
    void                        ImplOpenSourceManager();
    void                        ImplOpenSource();
    BOOL                        ImplEnableSource();
    void                        ImplXfer();
    void                        ImplFallback( ULONG nEvent );

                                DECL_LINK( ImplFallbackHdl, void* );
                                DECL_LINK( ImplDestroyHdl, void* );

public:

    HWND                        hTwainWnd;
    HHOOK                       hTwainHook;

    BOOL                        ImplHandleMsg( void* pMsg );

public:

                                ImpTwain( ScannerManager* pMgr, const Link& rNotifyLink );
                                ~ImpTwain();

    void                        Destroy();

    BOOL                        SelectSource();
    BOOL                        InitXfer();
};

// ---------
// - Procs -
// ---------

static ImpTwain* pImpTwainInstance = NULL;

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

// -----------------------------------------------------------------------------

ImpTwain::ImpTwain( ScannerManager* pMgr, const Link& rNotifyLink ) :
            mpMgr       ( pMgr ),
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

    HWND        hParentWnd = HWND_DESKTOP;
    WNDCLASS    aWc = { 0, &TwainWndProc, 0, sizeof( WNDCLASS ), GetModuleHandle( NULL ),
                        NULL, NULL, NULL, NULL, "TwainClass" };

    RegisterClass( &aWc );
    hTwainWnd = CreateWindowEx( WS_EX_TOPMOST, aWc.lpszClassName, "TWAIN", 0, 0, 0, 0, 0, hParentWnd, NULL, aWc.hInstance, 0 );
    hTwainHook = SetWindowsHookEx( WH_GETMESSAGE, &TwainMsgProc, NULL, GetCurrentThreadId() );
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

void ImpTwain::ImplOpenSourceManager()
{
    if( 1 == nCurState )
    {
        pMod = new ::vos::OModule( ::rtl::OUString() );

        if( pMod->load( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( TWAIN_LIBNAME ) ) ) )
        {
            nCurState = 2;

            if( ( ( pDSM = (DSMENTRYPROC) pMod->getSymbol( String( RTL_CONSTASCII_USTRINGPARAM( TWAIN_FUNCNAME ) ) ) ) != NULL ) &&
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
            TW_CAPABILITY   aCap = { CAP_XFERCOUNT, TWON_ONEVALUE, GlobalAlloc( GHND, sizeof( TW_ONEVALUE ) ) };
            TW_ONEVALUE*    pVal = (TW_ONEVALUE*) GlobalLock( aCap.hContainer );

            pVal->ItemType = TWTY_INT16, pVal->Item = 1;
            GlobalUnlock( aCap.hContainer );
            PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &aCap );
            GlobalFree( aCap.hContainer );
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

                    if( mpMgr && mpMgr->GetData() )
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
        long            nWidth, nHeight, nXRes, nYRes;

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
                if( mpMgr && hDIB )
                {
                    if( ( nXRes != -1 ) && ( nYRes != - 1 ) && ( nWidth != - 1 ) && ( nHeight != - 1 ) )
                    {
                        // set resolution of bitmap
                        BITMAPINFOHEADER*   pBIH = (BITMAPINFOHEADER*) GlobalLock( (HGLOBAL) hDIB );
                        static const double fFactor = 100.0 / 2.54;

                        pBIH->biXPelsPerMeter = FRound( fFactor * nXRes );
                        pBIH->biYPelsPerMeter = FRound( fFactor * nYRes );

                        GlobalUnlock( (HGLOBAL) hDIB );
                    }

                    mpMgr->SetData( (void*)(long) hDIB );
                }
                else
                    GlobalFree( (HGLOBAL) hDIB );

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
    if( hTwainWnd )
        DestroyWindow( hTwainWnd );

    if( hTwainHook )
        UnhookWindowsHookEx( hTwainHook );

    delete this;
    pImpTwainInstance = NULL;

    return 0L;
}

// ---------
// - Twain -
// ---------

class Twain
{
    ScannerManager*                             mpMgr;
    REF( com::sun::star::lang::XEventListener ) mxListener;
    ImpTwain*                                   mpImpTwain;
    TwainState                                  meState;

                                                DECL_LINK( ImpNotifyHdl, ImpTwain* );

public:

                                                Twain();
                                                ~Twain();

    BOOL                                        SelectSource();
    BOOL                                        PerformTransfer( ScannerManager* pMgr,
                                                                 const REF( com::sun::star::lang::XEventListener )& rxListener );

    TwainState                                  GetState() const { return meState; }
};

// ------------------------------------------------------------------------

Twain::Twain() :
        mpMgr       ( NULL ),
        mpImpTwain  ( NULL ),
        meState     ( TWAIN_STATE_NONE )
{
}

// ------------------------------------------------------------------------

Twain::~Twain()
{
    if( mpImpTwain )
        mpImpTwain->Destroy();
}

// ------------------------------------------------------------------------

BOOL Twain::SelectSource()
{
    BOOL bRet;

    if( !mpImpTwain )
    {
        meState = TWAIN_STATE_NONE;
        mpImpTwain = new ImpTwain( mpMgr, LINK( this, Twain, ImpNotifyHdl ) );
        bRet = mpImpTwain->SelectSource();
    }
    else
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Twain::PerformTransfer( ScannerManager* pMgr, const REF( com::sun::star::lang::XEventListener )& rxListener )
{
    BOOL bRet;

    if( pMgr && !mpImpTwain )
    {
        mpMgr = pMgr;
        mxListener = rxListener;
        meState = TWAIN_STATE_NONE;
        mpImpTwain = new ImpTwain( mpMgr, LINK( this, Twain, ImpNotifyHdl ) );
        bRet = mpImpTwain->InitXfer();
    }
    else
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------------

IMPL_LINK( Twain, ImpNotifyHdl, ImpTwain*, nEvent )
{
    switch( (ULONG)(void*) nEvent )
    {
        case( TWAIN_EVENT_SCANNING ):
            meState = TWAIN_STATE_SCANNING;
        break;

        case( TWAIN_EVENT_QUIT ):
        {
            if( meState != TWAIN_STATE_DONE )
                meState = TWAIN_STATE_CANCELED;

            mpImpTwain->Destroy();
            mpImpTwain = NULL;

            if( mpMgr && mxListener.is() )
                mxListener->disposing( com::sun::star::lang::EventObject( REF( XInterface )( static_cast< OWeakObject* >( mpMgr ) ) ) );

            mxListener = REF( com::sun::star::lang::XEventListener )();
        }
        break;

        case( TWAIN_EVENT_XFER ):
        {
            if( mpImpTwain )
            {
                meState = ( ( mpMgr && mpMgr->GetData() ) ? TWAIN_STATE_DONE : TWAIN_STATE_CANCELED );

                mpImpTwain->Destroy();
                mpImpTwain = NULL;

                if( mpMgr && mxListener.is() )
                    mxListener->disposing( com::sun::star::lang::EventObject( REF( XInterface )( static_cast< OWeakObject* >( mpMgr ) ) ) );
            }

            mxListener = REF( com::sun::star::lang::XEventListener )();
        }
        break;

        default:
        break;
    }

    return 0L;
}

// -----------
// - statics -
// -----------

static Twain aTwain;

// ------------------
// - ScannerManager -
// ------------------

void ScannerManager::DestroyData()
{
    if( mpData )
    {
        GlobalFree( (HGLOBAL)(long) mpData );
        mpData = NULL;
    }
}

// -----------------------------------------------------------------------------

AWT::Size ScannerManager::getSize()
{
    AWT::Size   aRet;
    HGLOBAL     hDIB = (HGLOBAL)(long) mpData;

    if( hDIB )
    {
        BITMAPINFOHEADER* pBIH = (BITMAPINFOHEADER*) GlobalLock( hDIB );

        if( pBIH )
        {
            aRet.Width = pBIH->biWidth;
            aRet.Height = pBIH->biHeight;
        }
        else
            aRet.Width = aRet.Height = 0;

        GlobalUnlock( hDIB );
    }
    else
        aRet.Width = aRet.Height = 0;

    return aRet;
}

// -----------------------------------------------------------------------------

SEQ( sal_Int8 ) ScannerManager::getDIB()
{
    SEQ( sal_Int8 ) aRet;

    if( mpData )
    {
        HGLOBAL             hDIB = (HGLOBAL)(long) mpData;
        const sal_uInt32    nDIBSize = GlobalSize( hDIB );
        sal_uInt32          nSize = sizeof( BITMAPFILEHEADER) + nDIBSize;
        BITMAPINFOHEADER*   pBIH = (BITMAPINFOHEADER*) GlobalLock( hDIB );

        if( pBIH )
        {
            sal_uInt32  nColEntries;

            switch( pBIH->biBitCount )
            {
                case( 1 ):
                case( 4 ):
                case( 8 ):
                    nColEntries = pBIH->biClrUsed ? pBIH->biClrUsed : ( 1 << pBIH->biBitCount );
                break;

                case( 24 ):
                    nColEntries = pBIH->biClrUsed ? pBIH->biClrUsed : 0;
                break;

                case( 16 ):
                case( 32 ):
                {
                    nColEntries = pBIH->biClrUsed;

                    if( pBIH->biCompression == BI_BITFIELDS )
                        nColEntries += 3;
                }
                break;

                default:
                    nColEntries = 0;
                break;
            }

            aRet = SEQ( sal_Int8 )( sizeof( BITMAPFILEHEADER ) + nDIBSize );

            sal_Int8*       pBuf = aRet.getArray();
            SvMemoryStream* pMemStm = new SvMemoryStream( (char*) pBuf, sizeof( BITMAPFILEHEADER ), STREAM_WRITE );

            *pMemStm << 'B' << 'M' << (sal_uInt32) 0 << (sal_uInt32) 0;
            *pMemStm << (sal_uInt32) ( sizeof( BITMAPFILEHEADER ) + pBIH->biSize + ( nColEntries * sizeof( RGBQUAD ) ) );

            delete pMemStm;
            HMEMCPY( pBuf + sizeof( BITMAPFILEHEADER ), pBIH, nDIBSize );
        }

        GlobalUnlock( hDIB );
        DestroyData();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

SEQ( ScannerContext ) SAL_CALL ScannerManager::getAvailableScanners()
{
    vos::OGuard             aGuard( maProtector );
    SEQ( ScannerContext )   aRet( 1 );

    aRet.getArray()[0].ScannerName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TWAIN" ) );
    aRet.getArray()[0].InternalData = 0;

    return aRet;
}

// -----------------------------------------------------------------------------

BOOL SAL_CALL ScannerManager::configureScanner( ScannerContext& rContext ) throw( ScannerException )
{
    vos::OGuard             aGuard( maProtector );
    REF( XScannerManager )  xThis( this );

    if( rContext.InternalData != 0 || rContext.ScannerName != ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TWAIN" ) ) )
        throw ScannerException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Scanner does not exist" ) ), xThis, ScanError_InvalidContext );

    DestroyData();

    return aTwain.SelectSource();
}

// -----------------------------------------------------------------------------

void SAL_CALL ScannerManager::startScan( const ScannerContext& rContext, const REF( com::sun::star::lang::XEventListener )& rxListener ) throw( ScannerException )
{
    vos::OGuard             aGuard( maProtector );
    REF( XScannerManager )  xThis( this );

    if( rContext.InternalData != 0 || rContext.ScannerName != ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TWAIN" ) ) )
        throw ScannerException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Scanner does not exist" ) ), xThis, ScanError_InvalidContext );

    DestroyData();

    aTwain.PerformTransfer( this, rxListener );
}

// -----------------------------------------------------------------------------

ScanError SAL_CALL ScannerManager::getError( const ScannerContext& rContext ) throw( ScannerException )
{
    vos::OGuard             aGuard( maProtector );
    ScanError               eError;
    REF( XScannerManager )  xThis( this );

    if( rContext.InternalData != 0 || rContext.ScannerName != ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TWAIN" ) ) )
        throw ScannerException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Scanner does not exist" ) ), xThis, ScanError_InvalidContext );

    if( aTwain.GetState() == TWAIN_STATE_CANCELED )
        eError = ScanError_ScanCanceled;
    else
        eError = ScanError_ScanErrorNone;

    return eError;
}

// -----------------------------------------------------------------------------

REF( AWT::XBitmap ) SAL_CALL ScannerManager::getBitmap( const ScannerContext& rContext ) throw( ScannerException )
{
    vos::OGuard aGuard( maProtector );
    return REF( AWT::XBitmap )( this );
}
