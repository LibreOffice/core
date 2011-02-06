/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#define INCL_DOSMISC
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS

#include <string.h>
#include <svpm.h>
#include <process.h>

#define _SV_SALINST_CXX

#include <vcl/solarmutex.hxx>
#include <tools/debug.hxx>

#include <salids.hrc>
#include <vcl/salatype.hxx>
#include <saldata.hxx>
#include <salinst.h>
#include <salframe.h>
#include <salobj.h>
#include <saltimer.h>
#include <salbmp.h>
#include <vcl/salimestatus.hxx>
#include <vcl/timer.hxx>
#include <tools/solarmutex.hxx>

// =======================================================================

void SalAbort( const XubString& rErrorText )
{
    ImplFreeSalGDI();

    if( !rErrorText.Len() )
        fprintf( stderr, "Application Error " );
    else
        fprintf( stderr, "%s ",
            ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
    abort();
}

// =======================================================================

ULONG GetCurrentThreadId()
{
    PTIB pptib = NULL;
    PPIB pppib = NULL;

    DosGetInfoBlocks( &pptib, &pppib );
    return pptib->tib_ptib2->tib2_ultid;
}

// =======================================================================

MRESULT EXPENTRY SalComWndProc( HWND hWnd, ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );

// =======================================================================

class SalYieldMutex : public vcl::SolarMutexObject
{
public:
    Os2SalInstance*         mpInstData;
    ULONG                   mnCount;
    ULONG                   mnThreadId;

public:
                            SalYieldMutex( Os2SalInstance* pInstData );

    virtual void SAL_CALL       acquire();
    virtual void SAL_CALL       release();
    virtual sal_Bool SAL_CALL   tryToAcquire();

    ULONG                   GetAcquireCount( ULONG nThreadId );
};

// -----------------------------------------------------------------------

SalYieldMutex::SalYieldMutex( Os2SalInstance* pInstData )
{
    mpInstData  = pInstData;
    mnCount     = 0;
    mnThreadId  = 0;
}

// -----------------------------------------------------------------------

void SalYieldMutex::acquire()
{
    SolarMutexObject::acquire();
    mnCount++;
    mnThreadId = GetCurrentThreadId();
}

// -----------------------------------------------------------------------

void SalYieldMutex::release()
{
    ULONG nThreadId = GetCurrentThreadId();
    if ( mnThreadId != nThreadId )
        SolarMutexObject::release();
    else
    {
        SalData* pSalData = GetSalData();
        if ( pSalData->mnAppThreadId != nThreadId )
        {
            if ( mnCount == 1 )
            {
                mpInstData->mpSalWaitMutex->acquire();
                if ( mpInstData->mnYieldWaitCount )
                    WinPostMsg( mpInstData->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
                mnThreadId = 0;
                mnCount--;
                SolarMutexObject::release();
                mpInstData->mpSalWaitMutex->release();
            }
            else
            {
                mnCount--;
                SolarMutexObject::release();
            }
        }
        else
        {
            if ( mnCount == 1 )
                mnThreadId = 0;
            mnCount--;
            SolarMutexObject::release();
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SalYieldMutex::tryToAcquire()
{
    if ( SolarMutexObject::tryToAcquire() )
    {
        mnCount++;
        mnThreadId = GetCurrentThreadId();
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

ULONG SalYieldMutex::GetAcquireCount( ULONG nThreadId )
{
    if ( nThreadId == mnThreadId )
        return mnCount;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquireWithWait()
{
    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    // If we are the main thread, then we must wait with wait, because
    // in if we don't reschedule, then we create deadlocks if a Windows
    // Function is called from another thread. If we arn't the main thread,
    // than we call qcquire directly.
    ULONG nThreadId = GetCurrentThreadId();
    SalData* pSalData = GetSalData();
    if ( pSalData->mnAppThreadId == nThreadId )
    {
        // Wenn wir den Mutex nicht bekommen, muessen wir solange
        // warten, bis wir Ihn bekommen
        BOOL bAcquire = FALSE;
        do
        {
            if ( pInst->mpSalYieldMutex->tryToAcquire() )
                bAcquire = TRUE;
            else
            {
                pInst->mpSalWaitMutex->acquire();
                if ( pInst->mpSalYieldMutex->tryToAcquire() )
                {
                    bAcquire = TRUE;
                    pInst->mpSalWaitMutex->release();
                }
                else
                {
                    pInst->mnYieldWaitCount++;
                    pInst->mpSalWaitMutex->release();
                    QMSG aTmpMsg;
                    WinGetMsg( pSalData->mhAB, &aTmpMsg, pInst->mhComWnd, SAL_MSG_RELEASEWAITYIELD, SAL_MSG_RELEASEWAITYIELD );
                    pInst->mnYieldWaitCount--;
                    if ( pInst->mnYieldWaitCount )
                        WinPostMsg( pInst->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0 , 0 );
                }
            }
        }
        while ( !bAcquire );
    }
    else
        pInst->mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

BOOL ImplSalYieldMutexTryToAcquire()
{
    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquire()
{
    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexRelease()
{
    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->release();
}

// -----------------------------------------------------------------------

ULONG ImplSalReleaseYieldMutex()
{
    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return 0;

    SalYieldMutex*  pYieldMutex = pInst->mpSalYieldMutex;
    ULONG           nCount = pYieldMutex->GetAcquireCount( GetCurrentThreadId() );
    ULONG           n = nCount;
    while ( n )
    {
        pYieldMutex->release();
        n--;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void ImplSalAcquireYieldMutex( ULONG nCount )
{
    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    SalYieldMutex*  pYieldMutex = pInst->mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

// -----------------------------------------------------------------------

#ifdef DBG_UTIL

void ImplDbgTestSolarMutex()
{
    SalData*    pSalData = GetSalData();
    ULONG       nCurThreadId = GetCurrentThreadId();
    if ( pSalData->mnAppThreadId != nCurThreadId )
    {
        if ( pSalData->mpFirstInstance )
        {
            SalYieldMutex* pYieldMutex = pSalData->mpFirstInstance->mpSalYieldMutex;
            if ( pYieldMutex->mnThreadId != nCurThreadId )
            {
                DBG_ERROR( "SolarMutex not locked, and not thread save code in VCL is called from outside of the main thread" );
            }
        }
    }
    else
    {
        if ( pSalData->mpFirstInstance )
        {
            SalYieldMutex* pYieldMutex = pSalData->mpFirstInstance->mpSalYieldMutex;
            if ( pYieldMutex->mnThreadId != nCurThreadId )
            {
                DBG_ERROR( "SolarMutex not locked in the main thread" );
            }
        }
    }
}

#endif

// =======================================================================

void InitSalData()
{
    SalData* pSalData = new SalData;
    memset( pSalData, 0, sizeof( SalData ) );
    SetSalData( pSalData );
}

// -----------------------------------------------------------------------

void DeInitSalData()
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFontMetrics )
        delete pSalData->mpFontMetrics;
    delete pSalData;
    SetSalData( NULL );
}

// -----------------------------------------------------------------------

void InitSalMain()
{
    PPIB pib;
    PTIB tib;
    HAB hAB;
    HMQ hMQ;
    SalData* pData = GetAppSalData();
#if OSL_DEBUG_LEVEL > 0
printf("InitSalMain\n");
#endif

    // morph application to PM
    DosGetInfoBlocks(&tib, &pib);
    // Change flag from VIO to PM:
    if (pib->pib_ultype==2) pib->pib_ultype = 3;

    // create anchor block
    hAB = WinInitialize( 0 );
    if ( !hAB )
        return;

    // create message queue
    hMQ = WinCreateMsgQueue( hAB, 60 );
    if ( !hMQ )
    {
        WinTerminate( hAB );
        return;
    }

    if ( pData ) // Im AppServer NULL
    {
        // Ankerblock und Messagequeue merken
        pData->mhAB = hAB;
        pData->mhMQ = hMQ;
    }

}

void DeInitSalMain()
{
#if OSL_DEBUG_LEVEL > 0
printf("DeInitSalMain\n");
#endif

    SalData* pData = GetAppSalData();
    // destroy message queue and anchor block
    WinDestroyMsgQueue( pData->mhMQ );
    WinTerminate( pData->mhAB );

}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance()
{
    SalData*        pSalData = GetSalData();

    // determine the os2 version
    ULONG nMayor;
    ULONG nMinor;
    DosQuerySysInfo( QSV_VERSION_MAJOR, QSV_VERSION_MAJOR, &nMayor, sizeof( nMayor ) );
    DosQuerySysInfo( QSV_VERSION_MINOR, QSV_VERSION_MINOR, &nMinor, sizeof( nMinor ) );
    aSalShlData.mnVersion = (USHORT)(nMayor*10 + nMinor);

    pSalData->mnAppThreadId = GetCurrentThreadId();

    // register frame class
    if ( !WinRegisterClass( pSalData->mhAB, (PSZ)SAL_FRAME_CLASSNAME,
                            (PFNWP)SalFrameWndProc, CS_MOVENOTIFY /* 17/08 CS_HITTEST | CS_MOVENOTIFY */,
                            SAL_FRAME_WNDEXTRA ) )
    {
        return NULL;
    }
    // register subframe class
    if ( !WinRegisterClass( pSalData->mhAB, (PSZ)SAL_SUBFRAME_CLASSNAME,
                            (PFNWP)SalFrameWndProc, CS_SAVEBITS| CS_MOVENOTIFY,
                            SAL_FRAME_WNDEXTRA ) )
    {
        return NULL;
    }
    // register object class
    if ( !WinRegisterClass( pSalData->mhAB, (PSZ)SAL_COM_CLASSNAME,
                            (PFNWP)SalComWndProc, 0, 0 ))
    {
        return NULL;
    }

    HWND hComWnd = WinCreateWindow( HWND_OBJECT, (PCSZ)SAL_COM_CLASSNAME,
                                    (PCSZ)"", 0, 0, 0, 0, 0,
                                    HWND_OBJECT, HWND_TOP,
                                    222, NULL, NULL);
    if ( !hComWnd )
        return NULL;

#if OSL_DEBUG_LEVEL > 1
    debug_printf("CreateSalInstance hComWnd %x\n", hComWnd);
#endif
    Os2SalInstance* pInst = new Os2SalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance   = pInst;
    pInst->mhAB     = pSalData->mhAB;
    pInst->mhMQ     = pSalData->mhMQ;
    pInst->mnArgc   = pSalData->mnArgc;
    pInst->mpArgv   = pSalData->mpArgv;
    pInst->mhComWnd = hComWnd;

    // AppIcon ermitteln
    ImplLoadSalIcon( SAL_RESID_ICON_DEFAULT, pInst->mhAppIcon);

    // init static GDI Data
    ImplInitSalGDI();

    return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
    SalData* pSalData = GetSalData();

    //  (only one instance in this version !!!)
    ImplFreeSalGDI();

#ifdef ENABLE_IME
    // IME-Daten freigeben
    if ( pSalData->mpIMEData )
        ImplReleaseSALIMEData();
#endif

    // reset instance
    if ( pSalData->mpFirstInstance == pInst )
        pSalData->mpFirstInstance = NULL;

    delete pInst;
}

// -----------------------------------------------------------------------

Os2SalInstance::Os2SalInstance()
{
    mhComWnd                = 0;
    mpSalYieldMutex         = new SalYieldMutex( this );
    mpSalWaitMutex          = new osl::Mutex;
    mnYieldWaitCount         = 0;
    mpSalYieldMutex->acquire();
    ::tools::SolarMutex::SetSolarMutex( mpSalYieldMutex );
}

// -----------------------------------------------------------------------

Os2SalInstance::~Os2SalInstance()
{
    ::tools::SolarMutex::SetSolarMutex( 0 );
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    delete mpSalWaitMutex;
    WinDestroyWindow( mhComWnd);
}

// -----------------------------------------------------------------------

osl::SolarMutex* Os2SalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}
// -----------------------------------------------------------------------

ULONG Os2SalInstance::ReleaseYieldMutex()
{
    return ImplSalReleaseYieldMutex();
}

// -----------------------------------------------------------------------

void Os2SalInstance::AcquireYieldMutex( ULONG nCount )
{
    ImplSalAcquireYieldMutex( nCount );
}

// -----------------------------------------------------------------------

static void ImplSalYield( BOOL bWait, BOOL bHandleAllCurrentEvents )
{
    QMSG            aMsg;
    bool bWasMsg = false, bOneEvent = false;
    bool bQuit = false;

    Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
    int nMaxEvents = bHandleAllCurrentEvents ? 100 : 1;
    do
    {
        if ( WinPeekMsg( pInst->mhAB, &aMsg, 0, 0, 0, PM_REMOVE ) )
        {
            WinDispatchMsg( pInst->mhAB, &aMsg );
            bOneEvent = bWasMsg = true;
            if (aMsg.msg == WM_QUIT)
                bQuit = true;
        }
        else
            bOneEvent = false;
    } while( --nMaxEvents && bOneEvent );

    if ( bWait && ! bWasMsg )
    {
        if ( WinGetMsg( pInst->mhAB, &aMsg, 0, 0, 0 ) )
            WinDispatchMsg( pInst->mhAB, &aMsg );
        else
            bQuit = true;
    }

    if (bQuit)
    {
        ImplSalYieldMutexAcquireWithWait();
        Os2SalFrame*   pFrame = GetSalData()->mpFirstFrame;
        if ( pFrame )
        {
            if (pFrame->CallCallback( SALEVENT_SHUTDOWN, 0 ))
                WinCancelShutdown( pFrame->mhAB, FALSE );
        }
        ImplSalYieldMutexRelease();
    }

}

// -----------------------------------------------------------------------

void Os2SalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    SalYieldMutex*  pYieldMutex = mpSalYieldMutex;
    SalData*        pSalData = GetSalData();
    ULONG           nCurThreadId = GetCurrentThreadId();
    ULONG           nCount = pYieldMutex->GetAcquireCount( nCurThreadId );
    ULONG           n = nCount;
    while ( n )
    {
        pYieldMutex->release();
        n--;
    }
    if ( pSalData->mnAppThreadId != nCurThreadId )
    {
        // #97739# A SendMessage call blocks until the called thread (here: the main thread)
        // returns. During a yield however, messages are processed in the main thread that might
        // result in a new message loop due to opening a dialog. Thus, SendMessage would not
        // return which will block this thread!
        // Solution: just give up the time slice and hope that messages are processed
        // by the main thread anyway (where all windows are created)
        // If the mainthread is not currently handling messages, then our SendMessage would
        // also do nothing, so this seems to be reasonable.

        // #i18883# only sleep if potential deadlock scenario, ie, when a dialog is open
        if( ImplGetSVData()->maAppData.mnModalMode )
            DosSleep(1);
        else
            WinSendMsg( mhComWnd, SAL_MSG_THREADYIELD, (MPARAM)bWait, (MPARAM)bHandleAllCurrentEvents );

        n = nCount;
        while ( n )
        {
            pYieldMutex->acquire();
            n--;
        }
    }
    else
    {
        ImplSalYield( bWait, bHandleAllCurrentEvents );

        n = nCount;
        while ( n )
        {
            ImplSalYieldMutexAcquireWithWait();
            n--;
        }
    }
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalComWndProc( HWND hWnd, ULONG nMsg,
                                MPARAM nMP1, MPARAM nMP2 )
{
    //debug_printf( "SalComWndProc hWnd 0x%x nMsg %d\n", hWnd, nMsg);

    switch ( nMsg )
    {
        case SAL_MSG_PRINTABORTJOB:
            //ImplSalPrinterAbortJobAsync( (HDC)wParam );
            break;
        case SAL_MSG_THREADYIELD:
            ImplSalYield( (bool)nMP1, (bool) nMP2);
            return 0;
        // If we get this message, because another GetMessage() call
        // has recieved this message, we must post this message to
        // us again, because in the other case we wait forever.
        case SAL_MSG_RELEASEWAITYIELD:
            {
            Os2SalInstance* pInst = GetSalData()->mpFirstInstance;
            if ( pInst && pInst->mnYieldWaitCount )
                WinPostMsg( hWnd, SAL_MSG_RELEASEWAITYIELD, nMP1, nMP2 );
            }
            return 0;
        case SAL_MSG_STARTTIMER:
            ImplSalStartTimer( (ULONG)nMP2, FALSE);
            return 0;
        case SAL_MSG_CREATEFRAME:
            return (MRESULT)ImplSalCreateFrame( GetSalData()->mpFirstInstance, (HWND)nMP2, (ULONG)nMP1 );
        case SAL_MSG_DESTROYFRAME:
            delete (SalFrame*)nMP2;
            return 0;
        case SAL_MSG_DESTROYHWND:
            //We only destroy the native window here. We do NOT destroy the SalFrame contained
            //in the structure (GetWindowPtr()).
            if (WinDestroyWindow((HWND)nMP2) == 0)
            {
                OSL_ENSURE(0, "DestroyWindow failed!");
                //Failure: We remove the SalFrame from the window structure. So we avoid that
                // the window structure may contain an invalid pointer, once the SalFrame is deleted.
               SetWindowPtr((HWND)nMP2, 0);
            }
            return 0;
        case SAL_MSG_CREATEOBJECT:
            return (MRESULT)ImplSalCreateObject( GetSalData()->mpFirstInstance, (Os2SalFrame*)(ULONG)nMP2 );
        case SAL_MSG_DESTROYOBJECT:
            delete (SalObject*)nMP2;
            return 0;
        case SAL_MSG_CREATESOUND:
            //return (MRESULT)((Os2SalSound*)nMP2)->ImplCreate();
            return 0;
        case SAL_MSG_DESTROYSOUND:
            //((Os2SalSound*)nMP2)->ImplDestroy();
            return 0;
        case SAL_MSG_POSTTIMER:
            SalTimerProc( 0, 0, SALTIMERPROC_RECURSIVE, (ULONG)nMP2 );
            break;
        case WM_TIMER:
            SalTimerProc( hWnd, 0, 0, 0 );
            break;
    }

    return WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );
}

// -----------------------------------------------------------------------

bool Os2SalInstance::AnyInput( USHORT nType )
{
    SalData* pSalData = GetSalData();
    QMSG    aQMSG;

    if ( (nType & (INPUT_ANY)) == INPUT_ANY )
    {
        // Any Input
        if ( WinPeekMsg( pSalData->mhAB, &aQMSG, 0, 0, 0, PM_NOREMOVE ) )
            return TRUE;
    }
    else
    {
        if ( nType & INPUT_MOUSE )
        {
            // Test auf Mouseinput
            if ( WinPeekMsg( pSalData->mhAB, &aQMSG, 0,
                             WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE ) )
                return TRUE;
        }

        if ( nType & INPUT_KEYBOARD )
        {
            // Test auf Keyinput
            if ( WinPeekMsg( pSalData->mhAB, &aQMSG, 0,
                             WM_CHAR, WM_CHAR, PM_NOREMOVE ) )
                return !(SHORT1FROMMP( aQMSG.mp1 ) & KC_KEYUP);
        }

        if ( nType & INPUT_PAINT )
        {
            // Test auf Paintinput
            if ( WinPeekMsg( pSalData->mhAB, &aQMSG, 0,
                             WM_PAINT, WM_PAINT, PM_NOREMOVE ) )
                return TRUE;
        }

        if ( nType & INPUT_TIMER )
        {
            // Test auf Timerinput
            if ( WinPeekMsg( pSalData->mhAB, &aQMSG, 0,
                             WM_TIMER, WM_TIMER, PM_NOREMOVE ) )
                return TRUE;
        }

        if ( nType & INPUT_OTHER )
        {
            // Test auf sonstigen Input
            if ( WinPeekMsg( pSalData->mhAB, &aQMSG, 0, 0, 0, PM_NOREMOVE ) )
                return TRUE;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

SalFrame* Os2SalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    return (SalFrame*)WinSendMsg( mhComWnd, SAL_MSG_CREATEFRAME, (MPARAM)nSalFrameStyle, (MPARAM)pSystemParentData->hWnd );
}

// -----------------------------------------------------------------------

SalFrame* Os2SalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    HWND mhWndClient;
//31/05/06 YD use client as owner(parent) so positioning will not need to
// take care of borders and captions
    if ( pParent )
        mhWndClient = static_cast<Os2SalFrame*>(pParent)->mhWndClient;
    else
        mhWndClient = 0;
    return (SalFrame*)WinSendMsg( mhComWnd, SAL_MSG_CREATEFRAME, (MPARAM)nSalFrameStyle, (MPARAM)mhWndClient );
}


// -----------------------------------------------------------------------

void Os2SalInstance::DestroyFrame( SalFrame* pFrame )
{
    WinSendMsg( mhComWnd, SAL_MSG_DESTROYFRAME, 0, (MPARAM)pFrame );
}

// -----------------------------------------------------------------------

SalObject* Os2SalInstance::CreateObject( SalFrame* pParent,
                                         SystemWindowData* /*pWindowData*/, // SystemWindowData meaningless on Windows
                                         BOOL /*bShow*/ )
{
    // Um auf Main-Thread umzuschalten
    return (SalObject*)WinSendMsg( mhComWnd, SAL_MSG_CREATEOBJECT, 0, (MPARAM)pParent );
}


// -----------------------------------------------------------------------

void Os2SalInstance::DestroyObject( SalObject* pObject )
{
    WinSendMsg( mhComWnd, SAL_MSG_DESTROYOBJECT, 0, (MPARAM)pObject );
}

// -----------------------------------------------------------------------

void* Os2SalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return (void*) "";
}

void Os2SalInstance::AddToRecentDocumentList(const rtl::OUString& /*rFileUrl*/, const rtl::OUString& /*rMimeType*/)
{
}

// -----------------------------------------------------------------------

SalTimer* Os2SalInstance::CreateSalTimer()
{
    return new Os2SalTimer();
}

// -----------------------------------------------------------------------

SalBitmap* Os2SalInstance::CreateSalBitmap()
{
    return new Os2SalBitmap();
}

// -----------------------------------------------------------------------

class Os2ImeStatus : public SalI18NImeStatus
{
  public:
    Os2ImeStatus() {}
    virtual ~Os2ImeStatus() {}

    // asks whether there is a status window available
    // to toggle into menubar
    virtual bool canToggle() { return false; }
    virtual void toggle() {}
};

SalI18NImeStatus* Os2SalInstance::CreateI18NImeStatus()
{
    return new Os2ImeStatus();
}

// -----------------------------------------------------------------------

const ::rtl::OUString& SalGetDesktopEnvironment()
{
    static ::rtl::OUString aDesktopEnvironment( RTL_CONSTASCII_USTRINGPARAM( "OS/2" ) );
    return aDesktopEnvironment;
}

SalSession* Os2SalInstance::CreateSalSession()
{
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
