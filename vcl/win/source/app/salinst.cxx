/*************************************************************************
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:50:30 $
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
#include <tools/svwin.h>
#ifdef WNT
#include <process.h>
#endif

#define _SV_SALINST_CXX

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALIDS_HRC
#include <salids.hrc>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_SALTIMER_H
#include <saltimer.h>
#endif
#ifndef _SV_SALSOUND_H
#include <salsound.h>
#endif
#ifndef _SV_SALATYPE_HXX
#include <salatype.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif
#ifndef _SV_SALOGL_H
#include <salogl.h>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_SALIMESTATUS_HXX
#include <salimestatus.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW       0x00020000
#endif

// =======================================================================

void SalAbort( const XubString& rErrorText )
{
    ImplFreeSalGDI();

    if ( !rErrorText.Len() )
        FatalAppExit( 0, "Application Error" );
    else
    {
        ByteString aErrorText( ImplSalGetWinAnsiString( rErrorText ) );
        FatalAppExit( 0, aErrorText.GetBuffer() );
    }
}

// =======================================================================

LRESULT CALLBACK SalComWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

// =======================================================================

class SalYieldMutex : public vos::OMutex
{
public: // for ImplSalYield()
    WinSalInstance*             mpInstData;
    ULONG                       mnCount;
    DWORD                       mnThreadId;

public:
                                SalYieldMutex( WinSalInstance* pInstData );

    virtual void SAL_CALL       acquire();
    virtual void SAL_CALL       release();
    virtual sal_Bool SAL_CALL   tryToAcquire();

    ULONG                       GetAcquireCount( ULONG nThreadId );
};

// -----------------------------------------------------------------------

SalYieldMutex::SalYieldMutex( WinSalInstance* pInstData )
{
    mpInstData  = pInstData;
    mnCount     = 0;
    mnThreadId  = 0;
}

// -----------------------------------------------------------------------

void SAL_CALL SalYieldMutex::acquire()
{
    OMutex::acquire();
    mnCount++;
    mnThreadId = GetCurrentThreadId();
}

// -----------------------------------------------------------------------

void SAL_CALL SalYieldMutex::release()
{
    DWORD nThreadId = GetCurrentThreadId();
    if ( mnThreadId != nThreadId )
        OMutex::release();
    else
    {
        SalData* pSalData = GetSalData();
        if ( pSalData->mnAppThreadId != nThreadId )
        {
            if ( mnCount == 1 )
            {
                // If we don't call these message, the Output from the
                // Java clients doesn't come in the right order
                GdiFlush();

                mpInstData->mpSalWaitMutex->acquire();
                if ( mpInstData->mnYieldWaitCount )
                    ImplPostMessage( mpInstData->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
                mnThreadId = 0;
                mnCount--;
                OMutex::release();
                mpInstData->mpSalWaitMutex->release();
            }
            else
            {
                mnCount--;
                OMutex::release();
            }
        }
        else
        {
            if ( mnCount == 1 )
                mnThreadId = 0;
            mnCount--;
            OMutex::release();
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SAL_CALL SalYieldMutex::tryToAcquire()
{
    if( OMutex::tryToAcquire() )
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
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    // If we are the main thread, then we must wait with wait, because
    // in if we don't reschedule, then we create deadlocks if a Windows
    // Function is called from another thread. If we arn't the main thread,
    // than we call qcquire directly.
    DWORD nThreadId = GetCurrentThreadId();
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
                    MSG aTmpMsg;
                    ImplGetMessage( &aTmpMsg, pInst->mhComWnd, SAL_MSG_RELEASEWAITYIELD, SAL_MSG_RELEASEWAITYIELD );
                    pInst->mnYieldWaitCount--;
                    if ( pInst->mnYieldWaitCount )
                        ImplPostMessage( pInst->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
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
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexRelease()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
    {
        GdiFlush();
        pInst->mpSalYieldMutex->release();
    }
}

// -----------------------------------------------------------------------

ULONG ImplSalReleaseYieldMutex()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
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
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
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
    DWORD       nCurThreadId = GetCurrentThreadId();
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

static void InitSalShlData()
{
    aSalShlData.mnVKAdd         = LOWORD( VkKeyScan( '+' ) );
    aSalShlData.mnVKSubtract    = LOWORD( VkKeyScan( '-' ) );
    aSalShlData.mnVKMultiply    = LOWORD( VkKeyScan( '*' ) );
    aSalShlData.mnVKDivide      = LOWORD( VkKeyScan( '/' ) );
    aSalShlData.mnVKPoint       = LOWORD( VkKeyScan( '.' ) );
    aSalShlData.mnVKComma       = LOWORD( VkKeyScan( ',' ) );
    aSalShlData.mnVKLess        = LOWORD( VkKeyScan( '<' ) );
    aSalShlData.mnVKGreater     = LOWORD( VkKeyScan( '>' ) );
    aSalShlData.mnVKEqual       = LOWORD( VkKeyScan( '=' ) );
}

// =======================================================================

void InitSalData()
{
    SalData* pSalData = new SalData;
    memset( pSalData, 0, sizeof( SalData ) );
    SetSalData( pSalData );
    CoInitialize(0);
}

// -----------------------------------------------------------------------

void DeInitSalData()
{
    CoUninitialize();
    SalData* pSalData = GetSalData();
    delete pSalData;
    SetSalData( NULL );
}

void InitSalMain()
{
    // remember data, copied from WinMain
    SalData* pData = GetAppSalData();
    if ( pData )    // Im AppServer NULL
    {
        STARTUPINFO aSI;
        aSI.cb = sizeof( aSI );
        GetStartupInfo( &aSI );
        pData->mhInst                   = GetModuleHandle( NULL );
        pData->mhPrevInst               = NULL;
        pData->mnCmdShow                = aSI.wShowWindow;
    }
}

void DeInitSalMain()
{
}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance()
{
    SalData* pSalData = GetSalData();

    // determine the windows version
    aSalShlData.mbW40 = 0;
    aSalShlData.mbWNT = 0;
    aSalShlData.mbWXP = 0;
    WORD nVer = (WORD)GetVersion();
    aSalShlData.mnVersion = (((WORD)LOBYTE(nVer)) * 100) + HIBYTE(nVer);
    if ( aSalShlData.mnVersion >= W95_VERSION )
        aSalShlData.mbW40 = 1;
    OSVERSIONINFO aVerInfo;
    aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
    if ( GetVersionEx( &aVerInfo ) )
    {
        if ( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            aSalShlData.mbWNT = 1;
            // Windows XP ?
            if ( aVerInfo.dwMajorVersion > 5 ||
               ( aVerInfo.dwMajorVersion == 5 && aVerInfo.dwMinorVersion >= 1 ) )
                aSalShlData.mbWXP = 1;
        }
    }

    pSalData->mnAppThreadId = GetCurrentThreadId();

    // register frame class
    if ( !pSalData->mhPrevInst )
    {
        if ( aSalShlData.mbWNT )
        {
            WNDCLASSEXW aWndClassEx;
            aWndClassEx.cbSize          = sizeof( aWndClassEx );
            aWndClassEx.style           = CS_OWNDC;
            aWndClassEx.lpfnWndProc     = SalFrameWndProcW;
            aWndClassEx.cbClsExtra      = 0;
            aWndClassEx.cbWndExtra      = SAL_FRAME_WNDEXTRA;
            aWndClassEx.hInstance       = pSalData->mhInst;
            aWndClassEx.hCursor         = 0;
            aWndClassEx.hbrBackground   = 0;
            aWndClassEx.lpszMenuName    = 0;
            aWndClassEx.lpszClassName   = SAL_FRAME_CLASSNAMEW;
            ImplLoadSalIcon( SAL_RESID_ICON_DEFAULT, aWndClassEx.hIcon, aWndClassEx.hIconSm );
            if ( !RegisterClassExW( &aWndClassEx ) )
                return NULL;

            aWndClassEx.hIcon           = 0;
            aWndClassEx.hIconSm         = 0;
            aWndClassEx.style          |= CS_SAVEBITS;
            aWndClassEx.lpszClassName   = SAL_SUBFRAME_CLASSNAMEW;
            if ( !RegisterClassExW( &aWndClassEx ) )
                return NULL;

            // shadow effect for popups on XP
            if( aSalShlData.mbWXP )
                aWndClassEx.style       |= CS_DROPSHADOW;
            aWndClassEx.lpszClassName   = SAL_TMPSUBFRAME_CLASSNAMEW;
            if ( !RegisterClassExW( &aWndClassEx ) )
                return NULL;

            aWndClassEx.style           = 0;
            aWndClassEx.lpfnWndProc     = SalComWndProcW;
            aWndClassEx.cbWndExtra      = 0;
            aWndClassEx.lpszClassName   = SAL_COM_CLASSNAMEW;
            if ( !RegisterClassExW( &aWndClassEx ) )
                return NULL;
        }
        else
        {
            WNDCLASSEXA aWndClassEx;
            aWndClassEx.cbSize          = sizeof( aWndClassEx );
            aWndClassEx.style           = CS_OWNDC;
            aWndClassEx.lpfnWndProc     = SalFrameWndProcA;
            aWndClassEx.cbClsExtra      = 0;
            aWndClassEx.cbWndExtra      = SAL_FRAME_WNDEXTRA;
            aWndClassEx.hInstance       = pSalData->mhInst;
            aWndClassEx.hCursor         = 0;
            aWndClassEx.hbrBackground   = 0;
            aWndClassEx.lpszMenuName    = 0;
            aWndClassEx.lpszClassName   = SAL_FRAME_CLASSNAMEA;
            ImplLoadSalIcon( SAL_RESID_ICON_DEFAULT, aWndClassEx.hIcon, aWndClassEx.hIconSm );
            if ( !RegisterClassExA( &aWndClassEx ) )
                return NULL;

            aWndClassEx.hIcon           = 0;
            aWndClassEx.hIconSm         = 0;
            aWndClassEx.style          |= CS_SAVEBITS;
            aWndClassEx.lpszClassName   = SAL_SUBFRAME_CLASSNAMEA;
            if ( !RegisterClassExA( &aWndClassEx ) )
                return NULL;

            aWndClassEx.style           = 0;
            aWndClassEx.lpfnWndProc     = SalComWndProcA;
            aWndClassEx.cbWndExtra      = 0;
            aWndClassEx.lpszClassName   = SAL_COM_CLASSNAMEA;
            if ( !RegisterClassExA( &aWndClassEx ) )
                return NULL;
        }
    }

    HWND hComWnd;
    if ( aSalShlData.mbWNT )
    {
        hComWnd = CreateWindowExW( WS_EX_TOOLWINDOW, SAL_COM_CLASSNAMEW,
                                   L"", WS_POPUP, 0, 0, 0, 0, 0, 0,
                                   pSalData->mhInst, NULL );
    }
    else
    {
        hComWnd = CreateWindowExA( WS_EX_TOOLWINDOW, SAL_COM_CLASSNAMEA,
                                   "", WS_POPUP, 0, 0, 0, 0, 0, 0,
                                   pSalData->mhInst, NULL );
    }
    if ( !hComWnd )
        return NULL;

    WinSalInstance* pInst = new WinSalInstance;

    // init shl data
    InitSalShlData();

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance   = pInst;
    pInst->mhInst    = pSalData->mhInst;
    pInst->mhComWnd  = hComWnd;

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

    // reset instance
    if ( pSalData->mpFirstInstance == pInst )
        pSalData->mpFirstInstance = NULL;

    delete pInst;
}

// -----------------------------------------------------------------------

WinSalInstance::WinSalInstance()
{
    mhComWnd                 = 0;
    mpSalYieldMutex          = new SalYieldMutex( this );
    mpSalWaitMutex           = new vos::OMutex;
    mnYieldWaitCount         = 0;
    mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

WinSalInstance::~WinSalInstance()
{
    WinSalOpenGL::Release();
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    delete mpSalWaitMutex;
    DestroyWindow( mhComWnd );
}

// -----------------------------------------------------------------------

vos::IMutex* WinSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

ULONG WinSalInstance::ReleaseYieldMutex()
{
    return ImplSalReleaseYieldMutex();
}

// -----------------------------------------------------------------------

void WinSalInstance::AcquireYieldMutex( ULONG nCount )
{
    ImplSalAcquireYieldMutex( nCount );
}

// -----------------------------------------------------------------------

static void ImplSalDispatchMessage( MSG* pMsg )
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstObject )
    {
        if ( ImplSalPreDispatchMsg( pMsg ) )
            return;
    }
    LRESULT lResult = ImplDispatchMessage( pMsg );
    if ( pSalData->mpFirstObject )
        ImplSalPostDispatchMsg( pMsg, lResult );
}

// -----------------------------------------------------------------------

void ImplSalYield( BOOL bWait )
{
    MSG aMsg;

    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( bWait )
    {
        if ( ImplGetMessage( &aMsg, 0, 0, 0 ) )
        {
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
        }
    }
    else
    {
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
        }
    }
}

// -----------------------------------------------------------------------

void WinSalInstance::Yield( BOOL bWait )
{
    SalYieldMutex*  pYieldMutex = mpSalYieldMutex;
    SalData*        pSalData = GetSalData();
    DWORD           nCurThreadId = GetCurrentThreadId();
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
        //ImplSendMessage( mhComWnd, SAL_MSG_THREADYIELD, (WPARAM)bWait, (LPARAM)0 );
        Sleep(1);
        n = nCount;
        while ( n )
        {
            pYieldMutex->acquire();
            n--;
        }
    }
    else
    {
        ImplSalYield( bWait );

        n = nCount;
        while ( n )
        {
            ImplSalYieldMutexAcquireWithWait();
            n--;
        }
    }
}

// -----------------------------------------------------------------------

LRESULT CALLBACK SalComWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT nRet = 0;

    switch ( nMsg )
    {
        case SAL_MSG_PRINTABORTJOB:
            ImplSalPrinterAbortJobAsync( (HDC)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_THREADYIELD:
            // #97739#: we should never reach this point
            // other threads do not actively yield anymore, they just sleep (see SalInstance::Yield)
            ImplSalYield( (BOOL)wParam );
            rDef = FALSE;
            break;
        // If we get this message, because another GetMessage() call
        // has recieved this message, we must post this message to
        // us again, because in the other case we wait forever.
        case SAL_MSG_RELEASEWAITYIELD:
            {
            WinSalInstance* pInst = GetSalData()->mpFirstInstance;
            if ( pInst && pInst->mnYieldWaitCount )
                ImplPostMessage( hWnd, SAL_MSG_RELEASEWAITYIELD, wParam, lParam );
            }
            rDef = FALSE;
            break;
        case SAL_MSG_STARTTIMER:
            ImplSalStartTimer( (ULONG) lParam, FALSE );
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEFRAME:
            nRet = (LRESULT)ImplSalCreateFrame( GetSalData()->mpFirstInstance, (HWND)lParam, (ULONG)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATEHWND:
            nRet = (LRESULT)ImplSalReCreateHWND( (HWND)wParam, (HWND)lParam, FALSE );
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATECHILDHWND:
            nRet = (LRESULT)ImplSalReCreateHWND( (HWND)wParam, (HWND)lParam, TRUE );
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYFRAME:
            delete (SalFrame*)lParam;
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEOBJECT:
            nRet = (LRESULT)ImplSalCreateObject( GetSalData()->mpFirstInstance, (WinSalFrame*)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYOBJECT:
            delete (SalObject*)lParam;
            rDef = FALSE;
            break;
        case SAL_MSG_CREATESOUND:
            nRet = ((WinSalSound*)lParam)->ImplCreate();
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYSOUND:
            ((WinSalSound*)lParam)->ImplDestroy();
            rDef = FALSE;
            break;
        case SAL_MSG_GETDC:
            nRet = (LRESULT)GetDCEx( (HWND)wParam, 0, DCX_CACHE );
            rDef = FALSE;
            break;
        case SAL_MSG_RELEASEDC:
            ReleaseDC( (HWND)wParam, (HDC)lParam );
            rDef = FALSE;
            break;
    }

    return nRet;
}

LRESULT CALLBACK SalComWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalComWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
    {
        if ( !ImplHandleGlobalMsg( hWnd, nMsg, wParam, lParam, nRet ) )
            nRet = DefWindowProcA( hWnd, nMsg, wParam, lParam );
    }
    return nRet;
}

LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalComWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
    {
        if ( !ImplHandleGlobalMsg( hWnd, nMsg, wParam, lParam, nRet ) )
            nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    }
    return nRet;
}

// -----------------------------------------------------------------------
// #108919#: ignore timer messages when called during time-out handler
BOOL ImplVerifyTimerMessage( const MSG& rMsg )
{
    if( rMsg.message == WM_TIMER )
        return !GetSalData()->mbInTimerProc;
    else
        return TRUE;    // arbitrary messages
}

bool WinSalInstance::AnyInput( USHORT nType )
{
    MSG aMsg;

    if ( (nType & (INPUT_ANY)) == (INPUT_ANY) )
    {
        // #108919#: If called from the time-out handler, timer messages are ignored (see ImplVerifyTimerMessage)
        // Thus we have to check for user input messages first to be able to detect them between the timer messages.
        // Otherwise we might not detect user input and the system will become unresponsive.
        if( AnyInput( INPUT_MOUSE ) || AnyInput( INPUT_KEYBOARD ) || AnyInput( INPUT_PAINT ) )
            return true;
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
            return ImplVerifyTimerMessage( aMsg );
    }
    else
    {
        if ( nType & INPUT_MOUSE )
        {
            // Test for mouse input
            if ( ImplPeekMessage( &aMsg, 0, WM_MOUSEFIRST, WM_MOUSELAST,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & INPUT_KEYBOARD )
        {
            // Test for key input
            if ( ImplPeekMessage( &aMsg, 0, WM_KEYDOWN, WM_KEYDOWN,
                                  PM_NOREMOVE | PM_NOYIELD ) )
            {
                if ( (aMsg.wParam == VK_SHIFT)   ||
                     (aMsg.wParam == VK_CONTROL) ||
                     (aMsg.wParam == VK_MENU) )
                    return false;
                else
                    return true;
            }
        }

        if ( nType & INPUT_PAINT )
        {
            // Test for paint input
            if ( ImplPeekMessage( &aMsg, 0, WM_PAINT, WM_PAINT,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & INPUT_TIMER )
        {
            // Test for timer input
            if ( ImplPeekMessage( &aMsg, 0, WM_TIMER, WM_TIMER,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return ImplVerifyTimerMessage( aMsg );

        }

        if ( nType & INPUT_OTHER )
        {
            // Test for any input
            // to have timer messages handled correctly, just check for INPUT_ANY, see above (#108919#)
            return AnyInput( INPUT_ANY );
            //if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
            //    return ImplVerifyTimerMessage( aMsg );
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void SalTimer::Start( ULONG nMS )
{
    // Um auf Main-Thread umzuschalten
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
            ImplPostMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
        else
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

// -----------------------------------------------------------------------

SalFrame* WinSalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    return (SalFrame*)ImplSendMessage( mhComWnd, SAL_MSG_CREATEFRAME, nSalFrameStyle, (LPARAM)pSystemParentData->hWnd );
}

// -----------------------------------------------------------------------

SalFrame* WinSalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    HWND hWndParent;
    if ( pParent )
        hWndParent = static_cast<WinSalFrame*>(pParent)->mhWnd;
    else
        hWndParent = 0;
    return (SalFrame*)ImplSendMessage( mhComWnd, SAL_MSG_CREATEFRAME, nSalFrameStyle, (LPARAM)hWndParent );
}

// -----------------------------------------------------------------------

void WinSalInstance::DestroyFrame( SalFrame* pFrame )
{
    ImplSendMessage( mhComWnd, SAL_MSG_DESTROYFRAME, 0, (LPARAM)pFrame );
}

// -----------------------------------------------------------------------

SalObject* WinSalInstance::CreateObject( SalFrame* pParent )
{
    // Um auf Main-Thread umzuschalten
    return (SalObject*)ImplSendMessage( mhComWnd, SAL_MSG_CREATEOBJECT, 0, (LPARAM)static_cast<WinSalFrame*>(pParent) );
}

// -----------------------------------------------------------------------

void WinSalInstance::DestroyObject( SalObject* pObject )
{
    ImplSendMessage( mhComWnd, SAL_MSG_DESTROYOBJECT, 0, (LPARAM)pObject );
}

// -----------------------------------------------------------------------

void* WinSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return "";
}

// -----------------------------------------------------------------------

SalTimer* WinSalInstance::CreateSalTimer()
{
    return new WinSalTimer();
}

// -----------------------------------------------------------------------

SalSound* WinSalInstance::CreateSalSound()
{
    return new WinSalSound();
}

// -----------------------------------------------------------------------

SalOpenGL* WinSalInstance::CreateSalOpenGL( SalGraphics* pGraphics )
{
    return new WinSalOpenGL( pGraphics );
}

// -----------------------------------------------------------------------

SalBitmap* WinSalInstance::CreateSalBitmap()
{
    return new WinSalBitmap();
}

class WinImeStatus : public SalI18NImeStatus
{
  public:
    WinImeStatus() {}
    virtual ~WinImeStatus() {}

    // asks whether there is a status window available
    // to toggle into menubar
    virtual bool canToggle() { return false; }
    virtual void toggle() {}
};

SalI18NImeStatus* WinSalInstance::CreateI18NImeStatus()
{
    return new WinImeStatus();
}
