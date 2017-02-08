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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>

#include <prewin.h>
#include <postwin.h>
#include <math.h>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <osl/mutex.hxx>
#include <osl/module.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/sysdata.hxx>
#include "scanner.hxx"

#if defined _MSC_VER
#pragma warning (push,1)
#pragma warning (disable:4668)
#endif
#include "twain/twain.h"
#if defined _MSC_VER
#pragma warning (pop)
#endif

using namespace ::com::sun::star;

#define TWAIN_EVENT_NONE        0x00000000UL
#define TWAIN_EVENT_QUIT        0x00000001UL
#define TWAIN_EVENT_SCANNING    0x00000002UL
#define TWAIN_EVENT_XFER        0x00000004UL

#define PFUNC                   (*pDSM)
#define PTWAINMSG               MSG*
#define FIXTODOUBLE( nFix )     ((double)nFix.Whole+(double)nFix.Frac/65536.)
#define FIXTOLONG( nFix )       ((long)floor(FIXTODOUBLE(nFix)+0.5))
#define TWAIN_FUNCNAME          "DSM_Entry"

#if defined(TWH_64BIT)
#    define TWAIN_LIBNAME "TWAINDSM.DLL"
#else
#    define TWAIN_LIBNAME "TWAIN_32.DLL"
#endif

enum TwainState
{
    TWAIN_STATE_NONE = 0,
    TWAIN_STATE_SCANNING = 1,
    TWAIN_STATE_DONE = 2,
    TWAIN_STATE_CANCELED = 3
};

class ImpTwain : public ::cppu::WeakImplHelper< util::XCloseListener >
{
    friend LRESULT CALLBACK TwainMsgProc( int nCode, WPARAM wParam, LPARAM lParam );

    uno::Reference< uno::XInterface >           mxSelfRef;
    uno::Reference< scanner::XScannerManager >  mxMgr;
    ScannerManager&                             mrMgr;
    TW_IDENTITY                                 aAppIdent;
    TW_IDENTITY                                 aSrcIdent;
    Link<unsigned long,void>                    aNotifyLink;
    DSMENTRYPROC                                pDSM;
    osl::Module*                                pMod;
    ULONG_PTR                                   nCurState;
    HWND                                        hTwainWnd;
    HHOOK                                       hTwainHook;
    bool                                        mbCloseFrameOnExit;

    bool                                        ImplHandleMsg( void* pMsg );
    void                                        ImplOpenSourceManager();
    void                                        ImplOpenSource();
    bool                                        ImplEnableSource();
    void                                        ImplXfer();
    void                                        ImplFallback( ULONG_PTR nEvent );
    void                                        ImplDeregisterCloseListener();
    void                                        ImplRegisterCloseListener();

                                                DECL_LINK( ImplFallbackHdl, void*, void );
                                                DECL_LINK( ImplDestroyHdl, void*, void );

    // from util::XCloseListener
    virtual void SAL_CALL queryClosing( const lang::EventObject& Source, sal_Bool GetsOwnership ) override;
    virtual void SAL_CALL notifyClosing( const lang::EventObject& Source ) override;

    // from lang::XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

public:

                                                ImpTwain( ScannerManager& rMgr, const Link<unsigned long,void>& rNotifyLink );
                                                ~ImpTwain() override;

    void                                        Destroy();

    bool                                        SelectSource();
    bool                                        InitXfer();
};

static ImpTwain* pImpTwainInstance = nullptr;

LRESULT CALLBACK TwainWndProc( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    return DefWindowProc( hWnd, nMsg, nPar1, nPar2 );
}

LRESULT CALLBACK TwainMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    MSG* pMsg = reinterpret_cast<MSG*>(lParam);

    if( ( nCode < 0 ) || ( pImpTwainInstance->hTwainWnd != pMsg->hwnd ) || !pImpTwainInstance->ImplHandleMsg( reinterpret_cast<void*>(lParam) ) )
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

namespace {

uno::Reference< frame::XFrame > ImplGetActiveFrame()
{
    try
    {
        // query desktop instance
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( ::comphelper::getProcessComponentContext() );

        uno::Reference< frame::XFrame > xActiveFrame = xDesktop->getActiveFrame();

        if( xActiveFrame.is() )
        {
            return xActiveFrame;
        }
    }
    catch( const uno::Exception& )
    {
    }

    OSL_FAIL("ImpTwain::ImplGetActiveFrame: Could not determine active frame!");
    return uno::Reference< frame::XFrame >();
}

uno::Reference< util::XCloseBroadcaster > ImplGetActiveFrameCloseBroadcaster()
{
    try
    {
        return uno::Reference< util::XCloseBroadcaster >( ImplGetActiveFrame(), uno::UNO_QUERY );
    }
    catch( const uno::Exception& )
    {
    }

    OSL_FAIL("ImpTwain::ImplGetActiveFrameCloseBroadcaster: Could determine close broadcaster on active frame!");
    return uno::Reference< util::XCloseBroadcaster >();
}

void ImplSendCloseEvent()
{
    try
    {
        uno::Reference< util::XCloseable > xCloseable( ImplGetActiveFrame(), uno::UNO_QUERY );

        if( xCloseable.is() )
            xCloseable->close( true );
    }
    catch( const uno::Exception& )
    {
    }

    OSL_FAIL("ImpTwain::ImplSendCloseEvent: Could not send required close broadcast!");
}

}

// #107835# hold reference to ScannerManager, to prevent premature death
ImpTwain::ImpTwain( ScannerManager& rMgr, const Link<unsigned long,void>& rNotifyLink ) :
            mxMgr( uno::Reference< scanner::XScannerManager >( static_cast< OWeakObject* >( &rMgr ), uno::UNO_QUERY) ),
            mrMgr( rMgr ),
            aNotifyLink( rNotifyLink ),
            pDSM( nullptr ),
            pMod( nullptr ),
            nCurState( 1 ),
            hTwainWnd( nullptr ),
            hTwainHook( nullptr ),
            mbCloseFrameOnExit( false )
{
    // setup TWAIN window
    pImpTwainInstance = this;

    aAppIdent.Id = 0;
    aAppIdent.Version.MajorNum = 1;
    aAppIdent.Version.MinorNum = 0;
    aAppIdent.Version.Language = TWLG_USA;
    aAppIdent.Version.Country = TWCY_USA;
    aAppIdent.ProtocolMajor = TWON_PROTOCOLMAJOR;
    aAppIdent.ProtocolMinor = TWON_PROTOCOLMINOR;
    aAppIdent.SupportedGroups = DG_IMAGE | DG_CONTROL;
    strncpy( aAppIdent.Version.Info, "8.0", 32 );
    aAppIdent.Version.Info[32] = aAppIdent.Version.Info[33] = 0;
    strncpy( aAppIdent.Manufacturer, "Sun Microsystems", 32 );
    aAppIdent.Manufacturer[32] = aAppIdent.Manufacturer[33] = 0;
    strncpy( aAppIdent.ProductFamily,"Office", 32 );
    aAppIdent.ProductFamily[32] = aAppIdent.ProductFamily[33] = 0;
    strncpy( aAppIdent.ProductName, "Office", 32 );
    aAppIdent.ProductName[32] = aAppIdent.ProductName[33] = 0;

    WNDCLASS aWc = { 0, &TwainWndProc, 0, sizeof( WNDCLASS ), GetModuleHandle( nullptr ), nullptr, nullptr, nullptr, nullptr, "TwainClass" };
    RegisterClass( &aWc );

    hTwainWnd = CreateWindowEx( WS_EX_TOPMOST, aWc.lpszClassName, "TWAIN", 0, 0, 0, 0, 0, HWND_DESKTOP, nullptr, aWc.hInstance, nullptr );
    hTwainHook = SetWindowsHookEx( WH_GETMESSAGE, &TwainMsgProc, nullptr, GetCurrentThreadId() );

    // block destruction until ImplDestroyHdl is called
    mxSelfRef = static_cast< ::cppu::OWeakObject* >( this );
}

ImpTwain::~ImpTwain()
{
    // are we responsible for application shutdown?
    if( mbCloseFrameOnExit )
        ImplSendCloseEvent();
}

void ImpTwain::Destroy()
{
    ImplFallback( TWAIN_EVENT_NONE );
    Application::PostUserEvent( LINK( this, ImpTwain, ImplDestroyHdl ) );
}

bool ImpTwain::SelectSource()
{
    TW_UINT16 nRet = TWRC_FAILURE;

    ImplOpenSourceManager();

    if( 3 == nCurState )
    {
        TW_IDENTITY aIdent;

        aIdent.Id = 0;
        aIdent.ProductName[ 0 ] = '\0';
        aNotifyLink.Call( TWAIN_EVENT_SCANNING );
        nRet = PFUNC( &aAppIdent, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &aIdent );
    }

    ImplFallback( TWAIN_EVENT_QUIT );

    return( TWRC_SUCCESS == nRet );
}

bool ImpTwain::InitXfer()
{
    bool bRet = false;

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

void ImpTwain::ImplOpenSourceManager()
{
    if( 1 == nCurState )
    {
        pMod = new ::osl::Module( OUString() );

        if( pMod->load( TWAIN_LIBNAME ) )
        {
            nCurState = 2;

            pDSM = reinterpret_cast<DSMENTRYPROC>(pMod->getSymbol(TWAIN_FUNCNAME));
            if (pDSM &&
                ( PFUNC( &aAppIdent, nullptr, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, &hTwainWnd ) == TWRC_SUCCESS ) )
            {
                nCurState = 3;
            }
        }
        else
        {
            delete pMod;
            pMod = nullptr;
        }
    }
}

void ImpTwain::ImplOpenSource()
{
    if( 3 == nCurState )
    {
        if( ( PFUNC( &aAppIdent, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &aSrcIdent ) == TWRC_SUCCESS ) &&
            ( PFUNC( &aAppIdent, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &aSrcIdent ) == TWRC_SUCCESS ) )
        {
            TW_CAPABILITY   aCap = { CAP_XFERCOUNT, TWON_ONEVALUE, GlobalAlloc( GHND, sizeof( TW_ONEVALUE ) ) };
            TW_ONEVALUE*    pVal = static_cast<TW_ONEVALUE*>(GlobalLock( aCap.hContainer ));

            pVal->ItemType = TWTY_INT16;
            pVal->Item = 1;
            GlobalUnlock( aCap.hContainer );
            PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &aCap );
            GlobalFree( aCap.hContainer );
            nCurState = 4;
        }
    }
}

bool ImpTwain::ImplEnableSource()
{
    bool bRet = false;

    if( 4 == nCurState )
    {
        TW_USERINTERFACE aUI = { true, true, hTwainWnd };

        aNotifyLink.Call( TWAIN_EVENT_SCANNING );
        nCurState = 5;

        // register as vetoable close listener, to prevent application to die under us
        ImplRegisterCloseListener();

        if( PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, &aUI ) == TWRC_SUCCESS )
        {
            bRet = true;
        }
        else
        {
            nCurState = 4;

            // deregister as vetoable close listener, dialog failed
            ImplDeregisterCloseListener();
        }
    }

    return bRet;
}

bool ImpTwain::ImplHandleMsg( void* pMsg )
{
    TW_UINT16   nRet;
    PTWAINMSG   pMess = static_cast<PTWAINMSG>(pMsg);
    TW_EVENT    aEvt = { pMess, MSG_NULL };

    if (pDSM)
        nRet = PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &aEvt );
    else
        nRet = TWRC_NOTDSEVENT;

    if( aEvt.TWMessage != MSG_NULL )
    {
        switch( aEvt.TWMessage )
        {
            case MSG_XFERREADY:
            {
                ULONG_PTR nEvent = TWAIN_EVENT_QUIT;

                if( 5 == nCurState )
                {
                    nCurState = 6;
                    ImplXfer();

                    if( mrMgr.GetData() )
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
            case TWRC_CANCEL:
                nCurState = 7;
            break;

            case TWRC_XFERDONE:
            {
                if( hDIB )
                {
                    if( ( nXRes != -1 ) && ( nYRes != - 1 ) && ( nWidth != - 1 ) && ( nHeight != - 1 ) )
                    {
                        // set resolution of bitmap
                        BITMAPINFOHEADER*   pBIH = static_cast<BITMAPINFOHEADER*>(GlobalLock( reinterpret_cast<HGLOBAL>((sal_IntPtr) hDIB) ));
                        static const double fFactor = 100.0 / 2.54;

                        pBIH->biXPelsPerMeter = FRound( fFactor * nXRes );
                        pBIH->biYPelsPerMeter = FRound( fFactor * nYRes );

                        GlobalUnlock( reinterpret_cast<HGLOBAL>((sal_IntPtr) hDIB) );
                    }

                    mrMgr.SetData( reinterpret_cast<void*>((sal_IntPtr) hDIB) );
                }
                else
                    GlobalFree( reinterpret_cast<HGLOBAL>((sal_IntPtr) hDIB) );

                nCurState = 7;
            }
            break;

            default:
            break;
        }
    }
}

void ImpTwain::ImplFallback( ULONG_PTR nEvent )
{
    Application::PostUserEvent( LINK( this, ImpTwain, ImplFallbackHdl ), reinterpret_cast<void*>(nEvent) );
}

IMPL_LINK( ImpTwain, ImplFallbackHdl, void*, pData, void )
{
    const sal_uIntPtr nEvent = reinterpret_cast<sal_uIntPtr>(pData);
    bool        bFallback = true;

    switch( nCurState )
    {
        case 7:
        case 6:
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

        case 5:
        {
            TW_USERINTERFACE aUI = { true, true, hTwainWnd };

            PFUNC( &aAppIdent, &aSrcIdent, DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, &aUI );
            nCurState = 4;

            // deregister as vetoable close listener
            ImplDeregisterCloseListener();
        }
        break;

        case 4:
        {
            PFUNC( &aAppIdent, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, &aSrcIdent );
            nCurState = 3;
        }
        break;

        case 3:
        {
            PFUNC( &aAppIdent, nullptr, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &hTwainWnd );
            nCurState = 2;
        }
        break;

        case 2:
        {
            delete pMod;
            pMod = nullptr;
            nCurState = 1;
        }
        break;

        default:
        {
            if( nEvent != TWAIN_EVENT_NONE )
                aNotifyLink.Call( nEvent );

            bFallback = false;
        }
        break;
    }

    if( bFallback )
        ImplFallback( nEvent );
}

IMPL_LINK_NOARG( ImpTwain, ImplDestroyHdl, void*, void )
{
    if( hTwainWnd )
        DestroyWindow( hTwainWnd );

    if( hTwainHook )
        UnhookWindowsHookEx( hTwainHook );

    // permit destruction of ourselves (normally, refcount
    // should drop to zero exactly here)
    mxSelfRef = nullptr;
    pImpTwainInstance = nullptr;
}

void ImpTwain::ImplRegisterCloseListener()
{
    try
    {
        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( ImplGetActiveFrameCloseBroadcaster() );

        if( xCloseBroadcaster.is() )
        {
            xCloseBroadcaster->addCloseListener(this);
            return; // successfully registered as a close listener
        }
        else
        {
            // interface unknown. don't register, then
            OSL_FAIL("ImpTwain::ImplRegisterCloseListener: XFrame has no XCloseBroadcaster!");
            return;
        }
    }
    catch( const uno::Exception& )
    {
    }

    OSL_FAIL("ImpTwain::ImplRegisterCloseListener: Could not register as close listener!");
}

void ImpTwain::ImplDeregisterCloseListener()
{
    try
    {
        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster(
            ImplGetActiveFrameCloseBroadcaster() );

        if( xCloseBroadcaster.is() )
        {
            xCloseBroadcaster->removeCloseListener(this);
            return; // successfully deregistered as a close listener
        }
        else
        {
            // interface unknown. don't deregister, then
            OSL_FAIL("ImpTwain::ImplDeregisterCloseListener: XFrame has no XCloseBroadcaster!");
            return;
        }
    }
    catch( const uno::Exception& )
    {
    }

    OSL_FAIL("ImpTwain::ImplDeregisterCloseListener: Could not deregister as close listener!");
}

void SAL_CALL ImpTwain::queryClosing( const lang::EventObject& /*Source*/, sal_Bool GetsOwnership )
{
    // shall we re-send the close query later on?
    mbCloseFrameOnExit = GetsOwnership;

    // the sole purpose of this listener is to forbid closing of the listened-at frame
    throw util::CloseVetoException();
}

void SAL_CALL ImpTwain::notifyClosing( const lang::EventObject& /*Source*/ )
{
    // should not happen
    OSL_FAIL("ImpTwain::notifyClosing called, but we vetoed the closing before!");
}

void SAL_CALL ImpTwain::disposing( const lang::EventObject& /*Source*/ )
{
    // we're not holding any references to the frame, thus noop
}

class Twain
{
    uno::Reference< lang::XEventListener >      mxListener;
    uno::Reference< scanner::XScannerManager >  mxMgr;
    const ScannerManager*                       mpCurMgr;
    ImpTwain*                                   mpImpTwain;
    TwainState                                  meState;

    DECL_LINK( ImpNotifyHdl, unsigned long, void );

public:

                                    Twain();
                                    ~Twain();

    bool                            SelectSource( ScannerManager& rMgr );
    bool                            PerformTransfer( ScannerManager& rMgr, const uno::Reference< lang::XEventListener >& rxListener );

    TwainState                      GetState() const { return meState; }
};

Twain::Twain() :
        mpCurMgr( nullptr ),
        mpImpTwain( nullptr ),
        meState( TWAIN_STATE_NONE )
{
}

Twain::~Twain()
{
    if( mpImpTwain )
        mpImpTwain->Destroy();
}

bool Twain::SelectSource( ScannerManager& rMgr )
{
    bool bRet;

    if( !mpImpTwain )
    {
        // hold reference to ScannerManager, to prevent premature death
        mxMgr.set( static_cast< OWeakObject* >( const_cast< ScannerManager* >( mpCurMgr = &rMgr ) ),
                   uno::UNO_QUERY );

        meState = TWAIN_STATE_NONE;
        mpImpTwain = new ImpTwain( rMgr, LINK( this, Twain, ImpNotifyHdl ) );
        bRet = mpImpTwain->SelectSource();
    }
    else
        bRet = false;

    return bRet;
}

bool Twain::PerformTransfer( ScannerManager& rMgr, const uno::Reference< lang::XEventListener >& rxListener )
{
    bool bRet;

    if( !mpImpTwain )
    {
        // hold reference to ScannerManager, to prevent premature death
        mxMgr.set( static_cast< OWeakObject* >( const_cast< ScannerManager* >( mpCurMgr = &rMgr ) ),
                   uno::UNO_QUERY );

        mxListener = rxListener;
        meState = TWAIN_STATE_NONE;
        mpImpTwain = new ImpTwain( rMgr, LINK( this, Twain, ImpNotifyHdl ) );
        bRet = mpImpTwain->InitXfer();
    }
    else
        bRet = false;

    return bRet;
}

IMPL_LINK( Twain, ImpNotifyHdl, unsigned long, nEvent, void )
{
    switch( nEvent )
    {
        case TWAIN_EVENT_SCANNING:
            meState = TWAIN_STATE_SCANNING;
        break;

        case TWAIN_EVENT_QUIT:
        {
            if( meState != TWAIN_STATE_DONE )
                meState = TWAIN_STATE_CANCELED;

            if( mpImpTwain )
            {
                mpImpTwain->Destroy();
                mpImpTwain = nullptr;
                mpCurMgr = nullptr;
            }

            if( mxListener.is() )
                mxListener->disposing( lang::EventObject( mxMgr ) );

            mxListener = nullptr;
        }
        break;

        case TWAIN_EVENT_XFER:
        {
            if( mpImpTwain )
            {
                meState = ( mpCurMgr->GetData() ? TWAIN_STATE_DONE : TWAIN_STATE_CANCELED );

                mpImpTwain->Destroy();
                mpImpTwain = nullptr;
                mpCurMgr = nullptr;

                if( mxListener.is() )
                    mxListener->disposing( lang::EventObject( mxMgr ) );
            }

            mxListener = nullptr;
        }
        break;

        default:
        break;
    }
}

static Twain aTwain;

void ScannerManager::AcquireData()
{
}

void ScannerManager::ReleaseData()
{
    if( mpData )
    {
        GlobalFree( static_cast<HGLOBAL>(mpData) );
        mpData = nullptr;
    }
}

awt::Size ScannerManager::getSize()
{
    awt::Size   aRet;
    HGLOBAL     hDIB = static_cast<HGLOBAL>(mpData);

    if( hDIB )
    {
        BITMAPINFOHEADER* pBIH = static_cast<BITMAPINFOHEADER*>(GlobalLock( hDIB ));

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

uno::Sequence< sal_Int8 > ScannerManager::getDIB()
{
    uno::Sequence< sal_Int8 > aRet;

    if( mpData )
    {
        HGLOBAL             hDIB = static_cast<HGLOBAL>(mpData);
        const sal_uInt32    nDIBSize = GlobalSize( hDIB );
        BITMAPINFOHEADER*   pBIH = static_cast<BITMAPINFOHEADER*>(GlobalLock( hDIB ));

        if( pBIH )
        {
            sal_uInt32  nColEntries;

            switch( pBIH->biBitCount )
            {
                case 1:
                case 4:
                case 8:
                    nColEntries = pBIH->biClrUsed ? pBIH->biClrUsed : ( 1 << pBIH->biBitCount );
                break;

                case 24:
                    nColEntries = pBIH->biClrUsed ? pBIH->biClrUsed : 0;
                break;

                case 16:
                case 32:
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

            aRet = uno::Sequence< sal_Int8 >( sizeof( BITMAPFILEHEADER ) + nDIBSize );

            sal_Int8*       pBuf = aRet.getArray();
            SvMemoryStream* pMemStm = new SvMemoryStream( pBuf, sizeof( BITMAPFILEHEADER ), StreamMode::WRITE );

            pMemStm->WriteChar( 'B' ).WriteChar( 'M' ).WriteUInt32( 0 ).WriteUInt32( 0 );
            pMemStm->WriteUInt32( sizeof( BITMAPFILEHEADER ) + pBIH->biSize + ( nColEntries * sizeof( RGBQUAD ) ) );

            delete pMemStm;
            memcpy( pBuf + sizeof( BITMAPFILEHEADER ), pBIH, nDIBSize );
        }

        GlobalUnlock( hDIB );
        ReleaseData();
    }

    return aRet;
}

uno::Sequence< ScannerContext > SAL_CALL ScannerManager::getAvailableScanners()
{
    osl::MutexGuard aGuard( maProtector );
    uno::Sequence< ScannerContext >   aRet( 1 );

    aRet.getArray()[0].ScannerName = "TWAIN" ;
    aRet.getArray()[0].InternalData = 0;

    return aRet;
}

sal_Bool SAL_CALL ScannerManager::configureScannerAndScan( ScannerContext& rContext, const uno::Reference< lang::XEventListener >& )
{
    osl::MutexGuard aGuard( maProtector );
    uno::Reference< XScannerManager >   xThis( this );

    if( rContext.InternalData != 0 || rContext.ScannerName != "TWAIN" )
        throw ScannerException("Scanner does not exist", xThis, ScanError_InvalidContext );

    ReleaseData();

    return aTwain.SelectSource( *this );
}

void SAL_CALL ScannerManager::startScan( const ScannerContext& rContext, const uno::Reference< lang::XEventListener >& rxListener )
{
    osl::MutexGuard aGuard( maProtector );
    uno::Reference< XScannerManager >   xThis( this );

    if( rContext.InternalData != 0 || rContext.ScannerName != "TWAIN" )
        throw ScannerException("Scanner does not exist", xThis, ScanError_InvalidContext );

    ReleaseData();
    aTwain.PerformTransfer( *this, rxListener );
}

ScanError SAL_CALL ScannerManager::getError( const ScannerContext& rContext )
{
    osl::MutexGuard aGuard( maProtector );
    uno::Reference< XScannerManager >   xThis( this );

    if( rContext.InternalData != 0 || rContext.ScannerName != "TWAIN" )
        throw ScannerException("Scanner does not exist", xThis, ScanError_InvalidContext );

    return( ( aTwain.GetState() == TWAIN_STATE_CANCELED ) ? ScanError_ScanCanceled : ScanError_ScanErrorNone );
}

uno::Reference< awt::XBitmap > SAL_CALL ScannerManager::getBitmap( const ScannerContext& /*rContext*/ )
{
    osl::MutexGuard aGuard( maProtector );
    return uno::Reference< awt::XBitmap >( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
