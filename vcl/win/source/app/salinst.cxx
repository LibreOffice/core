/*************************************************************************
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 10:03:56 $
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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif
#ifndef _SV_SALATYPE_HXX
#include <salatype.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <timer.hxx>
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

class SalYieldMutex : public NAMESPACE_VOS(OMutex)
{
public: // for ImplSalYield()
    SalInstanceData*            mpInstData;
    ULONG                       mnCount;
    DWORD                       mnThreadId;

public:
                                SalYieldMutex( SalInstanceData* pInstData );

    virtual void SAL_CALL       acquire();
    virtual void SAL_CALL       release();
    virtual sal_Bool SAL_CALL   tryToAcquire();

    ULONG                       GetAcquireCount( ULONG nThreadId );
};

// -----------------------------------------------------------------------

SalYieldMutex::SalYieldMutex( SalInstanceData* pInstData )
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
        // If we don't call these message, the Output from the
        // Java clients doesn't come in the right order
        GdiFlush();

        SalData* pSalData = GetSalData();
        if ( pSalData->mnAppThreadId != nThreadId )
        {
            if ( mnCount == 1 )
            {
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
    SalInstance* pInst = GetSalData()->mpFirstInstance;
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
            if ( pInst->maInstData.mpSalYieldMutex->tryToAcquire() )
                bAcquire = TRUE;
            else
            {
                pInst->maInstData.mpSalWaitMutex->acquire();
                if ( pInst->maInstData.mpSalYieldMutex->tryToAcquire() )
                {
                    bAcquire = TRUE;
                    pInst->maInstData.mpSalWaitMutex->release();
                }
                else
                {
                    pInst->maInstData.mnYieldWaitCount++;
                    pInst->maInstData.mpSalWaitMutex->release();
                    MSG aTmpMsg;
                    ImplGetMessage( &aTmpMsg, pInst->maInstData.mhComWnd, SAL_MSG_RELEASEWAITYIELD, SAL_MSG_RELEASEWAITYIELD );
                    pInst->maInstData.mnYieldWaitCount--;
                    if ( pInst->maInstData.mnYieldWaitCount )
                        ImplPostMessage( pInst->maInstData.mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
                }
            }
        }
        while ( !bAcquire );
    }
    else
        pInst->maInstData.mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

BOOL ImplSalYieldMutexTryToAcquire()
{
    SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->maInstData.mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquire()
{
    SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->maInstData.mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexRelease()
{
    SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->maInstData.mpSalYieldMutex->release();
}

// -----------------------------------------------------------------------

ULONG ImplSalReleaseYieldMutex()
{
    SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return 0;

    SalYieldMutex*  pYieldMutex = pInst->maInstData.mpSalYieldMutex;
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
    SalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    SalYieldMutex*  pYieldMutex = pInst->maInstData.mpSalYieldMutex;
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
            SalYieldMutex* pYieldMutex = pSalData->mpFirstInstance->maInstData.mpSalYieldMutex;
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
            SalYieldMutex* pYieldMutex = pSalData->mpFirstInstance->maInstData.mpSalYieldMutex;
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
}

void DeInitSalMain()
{
}

// -----------------------------------------------------------------------

void SetFilterCallback( void* pCallback, void* pInst )
{
    SalData* pSalData = GetSalData();

    pSalData->mpFirstInstance->maInstData.mpFilterCallback = pCallback;
    pSalData->mpFirstInstance->maInstData.mpFilterInst = pInst;
}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance()
{
    SalData* pSalData = GetSalData();

    // determine the windows version
    WORD nVer = (WORD)GetVersion();
    aSalShlData.mnVersion = (((WORD)LOBYTE(nVer)) * 100) + HIBYTE(nVer);
    if ( aSalShlData.mnVersion >= W95_VERSION )
        aSalShlData.mbW40 = 1;
    OSVERSIONINFO aVerInfo;
    aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
    if ( GetVersionEx( &aVerInfo ) )
    {
        if ( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
            aSalShlData.mbWNT = 1;
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
            aWndClassEx.style          |= CS_SAVEBITS;
            aWndClassEx.lpszClassName   = SAL_FRAME_CLASSNAME_SBW;
            if ( !RegisterClassExW( &aWndClassEx ) )
                return NULL;

            aWndClassEx.style           = 0;
            aWndClassEx.lpfnWndProc     = SalComWndProcW;
            aWndClassEx.cbWndExtra      = 0;
            aWndClassEx.hIcon           = 0;
            aWndClassEx.hIconSm         = 0;
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
            aWndClassEx.style          |= CS_SAVEBITS;
            aWndClassEx.lpszClassName   = SAL_FRAME_CLASSNAME_SBA;
            if ( !RegisterClassExA( &aWndClassEx ) )
                return NULL;

            aWndClassEx.style           = 0;
            aWndClassEx.lpfnWndProc     = SalComWndProcA;
            aWndClassEx.cbWndExtra      = 0;
            aWndClassEx.hIcon           = 0;
            aWndClassEx.hIconSm         = 0;
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

    SalInstance* pInst = new SalInstance;

    // init shl data
    InitSalShlData();

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance   = pInst;
    pInst->maInstData.mhInst    = pSalData->mhInst;
    pInst->maInstData.mhComWnd  = hComWnd;

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

SalInstance::SalInstance()
{
    maInstData.mhComWnd                 = 0;
    maInstData.mpFilterCallback         = NULL;
    maInstData.mpFilterInst             = NULL;
    maInstData.mpSalYieldMutex          = new SalYieldMutex( &maInstData );
    maInstData.mpSalWaitMutex           = new NAMESPACE_VOS(OMutex);
    maInstData.mnYieldWaitCount         = 0;
    maInstData.mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

SalInstance::~SalInstance()
{
    maInstData.mpSalYieldMutex->release();
    delete maInstData.mpSalYieldMutex;
    delete maInstData.mpSalWaitMutex;
    DestroyWindow( maInstData.mhComWnd );
}

// -----------------------------------------------------------------------

#ifdef _VOS_NO_NAMESPACE
IMutex* SalInstance::GetYieldMutex()
#else
vos::IMutex* SalInstance::GetYieldMutex()
#endif
{
    return maInstData.mpSalYieldMutex;
}

// -----------------------------------------------------------------------

ULONG SalInstance::ReleaseYieldMutex()
{
    return ImplSalReleaseYieldMutex();
}

// -----------------------------------------------------------------------

void SalInstance::AcquireYieldMutex( ULONG nCount )
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

void SalInstance::Yield( BOOL bWait )
{
    SalYieldMutex*  pYieldMutex = maInstData.mpSalYieldMutex;
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
        ImplSendMessage( maInstData.mhComWnd, SAL_MSG_THREADYIELD, (WPARAM)bWait, (LPARAM)0 );
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
            ImplSalYield( (BOOL)wParam );
            rDef = FALSE;
            break;
        // If we get this message, because another GetMessage() call
        // has recieved this message, we must post this message to
        // us again, because in the other case we wait forever.
        case SAL_MSG_RELEASEWAITYIELD:
            {
            SalInstance* pInst = GetSalData()->mpFirstInstance;
            if ( pInst && pInst->maInstData.mnYieldWaitCount )
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
        case SAL_MSG_DESTROYFRAME:
            delete (SalFrame*)lParam;
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEOBJECT:
            nRet = (LRESULT)ImplSalCreateObject( GetSalData()->mpFirstInstance, (SalFrame*)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYOBJECT:
            delete (SalObject*)lParam;
            rDef = FALSE;
            break;
        case SAL_MSG_CREATESOUND:
            nRet = ((SalSound*)lParam)->ImplCreate();
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYSOUND:
            ((SalSound*)lParam)->ImplDestroy();
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

BOOL SalInstance::AnyInput( USHORT nType )
{
    MSG aMsg;

    if ( (nType & (INPUT_ANY)) == (INPUT_ANY) )
    {
        // Any Input
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
            return TRUE;
    }
    else
    {
        if ( nType & INPUT_MOUSE )
        {
            // Test auf Mouseinput
            if ( ImplPeekMessage( &aMsg, 0, WM_MOUSEFIRST, WM_MOUSELAST,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return TRUE;
        }

        if ( nType & INPUT_KEYBOARD )
        {
            // Test auf Keyinput
            if ( ImplPeekMessage( &aMsg, 0, WM_KEYDOWN, WM_KEYDOWN,
                                  PM_NOREMOVE | PM_NOYIELD ) )
            {
                if ( (aMsg.wParam == VK_SHIFT)   ||
                     (aMsg.wParam == VK_CONTROL) ||
                     (aMsg.wParam == VK_MENU) )
                    return FALSE;
                else
                    return TRUE;
            }
        }

        if ( nType & INPUT_PAINT )
        {
            // Test auf Paintinput
            if ( ImplPeekMessage( &aMsg, 0, WM_PAINT, WM_PAINT,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return TRUE;
        }

        if ( nType & INPUT_TIMER )
        {
            // Test auf Timerinput
            if ( ImplPeekMessage( &aMsg, 0, WM_TIMER, WM_TIMER,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return TRUE;
        }

        if ( nType & INPUT_OTHER )
        {
            // Test auf sonstigen Input
            if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
                return TRUE;
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
            ImplPostMessage( pSalData->mpFirstInstance->maInstData.mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
        else
            ImplSendMessage( pSalData->mpFirstInstance->maInstData.mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    return (SalFrame*)ImplSendMessage( maInstData.mhComWnd, SAL_MSG_CREATEFRAME, nSalFrameStyle, (LPARAM)pSystemParentData->hWnd );
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    HWND hWndParent;
    if ( pParent )
        hWndParent = pParent->maFrameData.mhWnd;
    else
        hWndParent = 0;
    return (SalFrame*)ImplSendMessage( maInstData.mhComWnd, SAL_MSG_CREATEFRAME, nSalFrameStyle, (LPARAM)hWndParent );
}

// -----------------------------------------------------------------------

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
    ImplSendMessage( maInstData.mhComWnd, SAL_MSG_DESTROYFRAME, 0, (LPARAM)pFrame );
}

// -----------------------------------------------------------------------

SalObject* SalInstance::CreateObject( SalFrame* pParent )
{
    // Um auf Main-Thread umzuschalten
    return (SalObject*)ImplSendMessage( maInstData.mhComWnd, SAL_MSG_CREATEOBJECT, 0, (LPARAM)pParent );
}

// -----------------------------------------------------------------------

void SalInstance::DestroyObject( SalObject* pObject )
{
    ImplSendMessage( maInstData.mhComWnd, SAL_MSG_DESTROYOBJECT, 0, (LPARAM)pObject );
}

// -----------------------------------------------------------------------

void SalInstance::SetEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void SalInstance::SetErrorEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void* SalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return "";
}
