/*************************************************************************
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

#define INCL_DOSMISC
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS

#include <string.h>
#include <tools/svpm.h>
#include <process.h>

#define _SV_SALINST_CXX

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
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
#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif

#define SVMODULENAME        "VCL" MAKE_NUMSTR( SUPD ) __DLLEXTENSION

// =======================================================================

void SalSetExceptionHandler( SALEXCEPTIONPROC pProc )
{
}

// -----------------------------------------------------------------------

void SalAbort( const XubString& rErrorText )
{
}

// -----------------------------------------------------------------------

ULONG ImplSalGetCurrentThreadId()
{
    PTIB pptib = NULL;
    PPIB pppib = NULL;

    DosGetInfoBlocks( &pptib, &pppib );
    return pptib->tib_ptib2->tib2_ultid;
}

// =======================================================================

class SalYieldMutex : public NAMESPACE_VOS(OMutex)
{
public:
    SalInstanceData*        mpInstData;
    ULONG                   mnCount;
    ULONG                   mnWaitCount;
    ULONG                   mnThreadId;

public:
                            SalYieldMutex( SalInstanceData* pInstData );

    virtual void            acquire();
    virtual void            release();
    virtual Boolean         tryToAcquire();

    ULONG                   GetAcquireCount( ULONG nThreadId );
};

// -----------------------------------------------------------------------

SalYieldMutex::SalYieldMutex( SalInstanceData* pInstData )
{
    mpInstData  = pInstData;
    mnCount     = 0;
    mnThreadId  = 0;
    mnWaitCount = 0;
}

// -----------------------------------------------------------------------

void SalYieldMutex::acquire()
{
    OMutex::acquire();
    mnCount++;
    mnThreadId = ImplSalGetCurrentThreadId();
}

// -----------------------------------------------------------------------

void SalYieldMutex::release()
{
    ULONG nThreadId = ImplSalGetCurrentThreadId();
    if ( mnThreadId != nThreadId )
        OMutex::release();
    else
    {
        SalData* pSalData = GetSalData();
        if ( pSalData->mnAppThreadId != nThreadId )
        {
            NAMESPACE_VOS(OGuard) aGuard( mpInstData->mpSalWaitMutex );
            if ( mnCount == 1 )
            {
                if ( mnWaitCount && WinPostMsg( mpInstData->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 ) )
                    mnWaitCount--;
                mnThreadId = 0;
            }
            mnCount--;
            OMutex::release();
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

Boolean SalYieldMutex::tryToAcquire()
{
    if ( OMutex::tryToAcquire() )
    {
        mnCount++;
        mnThreadId = ImplSalGetCurrentThreadId();
        return True;
    }
    else
        return False;
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

BOOL ImplSalYieldMutexTryToAcquire()
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
        return pSalData->mpFirstInstance->maInstData.mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquire()
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
        pSalData->mpFirstInstance->maInstData.mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexRelease()
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
        pSalData->mpFirstInstance->maInstData.mpSalYieldMutex->release();
}

// -----------------------------------------------------------------------

#ifdef DBG_UTIL

void ImplDbgTestSolarMutex()
{
    SalData*    pSalData = GetSalData();
    ULONG       nCurThreadId = ImplSalGetCurrentThreadId();
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

void InitSalSystemData()
{
}

// -----------------------------------------------------------------------

static void ImplSalYield( BOOL bWait, BOOL bMainThread, SalYieldMutex* pYieldMutex )
{
    QMSG            aMsg;
    QMSG            aTmpMsg;
    BOOL            bDispatch = FALSE;
    USHORT          nPostReleaseWait = 0;
    static QMSG*    pMsg = NULL;
    SalData*        pSalData = GetSalData();

    do
    {
        if ( !pMsg )
        {
            BOOL bQuit = FALSE;
            if ( bWait )
            {
                if ( WinGetMsg( pSalData->mhAB, &aMsg, 0, 0, 0 ) )
                    pMsg = &aMsg;
                else
                    bQuit = TRUE;
            }
            else
            {
                if ( WinPeekMsg( pSalData->mhAB, &aMsg, 0, 0, 0, PM_REMOVE ) )
                {
                    if ( aMsg.msg == WM_QUIT )
                        bQuit = TRUE;
                    else
                        pMsg = &aMsg;
                }
            }

            // ShutDown-Event ausloesen (ist immer dann der Fall,
            // wenn wir eine Quit-Message bekommen)
            if ( bQuit && pSalData->mpDefaultFrame )
            {
                SalFrame* pDefaultFrame = pSalData->mpDefaultFrame;
                if ( pDefaultFrame->maFrameData.mpProc( pDefaultFrame->maFrameData.mpInst, pDefaultFrame,
                                                        SALEVENT_SHUTDOWN, 0 ) )
                    WinCancelShutdown( pSalData->mhAB, FALSE );
            }

            // ReleaseWaitYield ignorieren wir, da diese fuer andere
            // Yield-Aufrufe gedacht sind
            if ( pMsg )
            {
                if ( pMsg->msg == SAL_MSG_RELEASEWAITYIELD )
                {
                    nPostReleaseWait++;
                    pMsg = NULL;
                    continue;
                }
            }
        }

        if ( pMsg )
        {
            // Darf ich die Message dispatchen
            pYieldMutex->mpInstData->mpSalWaitMutex->acquire();
            if ( pYieldMutex->tryToAcquire() )
            {
                pYieldMutex->mpInstData->mpSalWaitMutex->release();
                bDispatch = TRUE;
            }
            else
            {
                pYieldMutex->mnWaitCount++;
                pYieldMutex->mpInstData->mpSalWaitMutex->release();
                WinGetMsg( pSalData->mhAB, &aTmpMsg, pYieldMutex->mpInstData->mhComWnd, SAL_MSG_RELEASEWAITYIELD, SAL_MSG_RELEASEWAITYIELD );
                if ( !pMsg )
                    bDispatch = TRUE;
            }
        }
        else
            bDispatch = TRUE;
    }
    while( !bDispatch );

    if ( pMsg )
    {
        // acquire ist nicht notwendig, da dies schon in der oberen
        // Schleife bei tryToAcquire() gemacht wurde
        QMSG* pTmpMsg = pMsg;
        pMsg = NULL;
        WinDispatchMsg( pSalData->mhAB, pTmpMsg );
        pYieldMutex->release();
    }

    while ( nPostReleaseWait )
    {
        WinPostMsg( pYieldMutex->mpInstData->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
        nPostReleaseWait--;
    }
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalComWndProc( HWND hWnd, PM_ULONG nMsg,
                                MPARAM nMP1, MPARAM nMP2 )
{
    switch ( nMsg )
    {
        case SAL_MSG_STARTTIMER:
            ImplSalStartTimer();
            return 0;
        case SAL_MSG_CREATEFRAME:
            return (MRESULT)ImplSalCreateFrame( GetSalData()->mpFirstInstance, (SalFrame*)(ULONG)nMP2, (ULONG)nMP1 );
        case SAL_MSG_CREATEOBJECT:
            return (MRESULT)ImplSalCreateObject( GetSalData()->mpFirstInstance, (SalFrame*)(ULONG)nMP2 );
        case SAL_MSG_THREADYIELD:
            ImplSalYield( (BOOL)(LONG)nMP1, FALSE, (SalYieldMutex*)(LONG)nMP2 );
            return 0;
    }

    return WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );
}

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

void SetFilterCallback( void* pCallback, void* pInst )
{
    SalData* pSalData = GetSalData();
    pSalData->mpFirstInstance->maInstData.mpFilterCallback = pCallback;
    pSalData->mpFirstInstance->maInstData.mpFilterInst = pInst;
}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance()
{
    SalData*        pSalData = GetSalData();
    SalInstance*    pInst = new SalInstance;

    // determine Module-Handle for SVDLL
    DosQueryModuleHandle( (PSZ)SVMODULENAME, &aSalShlData.mhMod );

    // determine the os2 version
    PM_ULONG nMayor;
    PM_ULONG nMinor;
    DosQuerySysInfo( QSV_VERSION_MAJOR, QSV_VERSION_MAJOR, &nMayor, sizeof( nMayor ) );
    DosQuerySysInfo( QSV_VERSION_MINOR, QSV_VERSION_MINOR, &nMinor, sizeof( nMinor ) );
    aSalShlData.mnVersion = (USHORT)(nMayor*10 + nMinor);

    pSalData->mnAppThreadId = ImplSalGetCurrentThreadId();

    // register frame class
    if ( !WinRegisterClass( pSalData->mhAB, (PSZ)SAL_FRAME_CLASSNAME,
                            (PFNWP)SalFrameWndProc, CS_HITTEST | CS_MOVENOTIFY,
                            SAL_FRAME_WNDEXTRA ) )
    {
        delete pInst;
        return NULL;
    }

    // register frame class
    if ( !WinRegisterClass( pSalData->mhAB, (PSZ)SAL_COM_CLASSNAME,
                            (PFNWP)SalComWndProc, 0, 0 ))
    {
        delete pInst;
        return NULL;
    }

    HWND hComWnd = WinCreateWindow( HWND_OBJECT, (PSZ)SAL_COM_CLASSNAME,
                                    "", 0, 0, 0, 0, 0,
                                    HWND_OBJECT, HWND_TOP,
                                    222, NULL, NULL);

    // init system data
    InitSalSystemData();

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance   = pInst;
    pInst->maInstData.mhAB      = pSalData->mhAB;
    pInst->maInstData.mhMQ      = pSalData->mhMQ;
    pInst->maInstData.mnArgc    = pSalData->mnArgc;
    pInst->maInstData.mpArgv    = pSalData->mpArgv;
    pInst->maInstData.mhComWnd  = hComWnd;

    // AppIcon ermitteln
    pInst->maInstData.mhAppIcon = WinLoadPointer( HWND_DESKTOP, pSalData->mhAB, 1 );
    if ( !pInst->maInstData.mhAppIcon )
        pInst->maInstData.mhAppIcon = ImplLoadPointer( SAL_RESID_ICON_SD );

    return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
    SalData* pSalData = GetSalData();

    //  (only one instance in this version !!!)

#ifdef ENABLE_IME
    // IME-Daten freigeben
    if ( pSalData->mpIMEData )
        ImplReleaseSALIMEData();
#endif

    // Destroy Dummy Frame
    if ( pSalData->mpDummyFrame )
        pInst->DestroyFrame( pSalData->mpDummyFrame );

    // reset instance
    if ( pSalData->mpFirstInstance == pInst )
        pSalData->mpFirstInstance = NULL;

    delete pInst;
}

// -----------------------------------------------------------------------

SalInstance::SalInstance()
{
    maInstData.mpFilterCallback         = NULL;
    maInstData.mpFilterInst             = NULL;

    maInstData.mpSalWaitMutex           = new NAMESPACE_VOS(OMutex);
    maInstData.mpSalYieldMutex          = new SalYieldMutex( &maInstData );
    maInstData.mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

SalInstance::~SalInstance()
{
    maInstData.mpSalYieldMutex->release();
    delete maInstData.mpSalYieldMutex;
    delete maInstData.mpSalWaitMutex;
}

// -----------------------------------------------------------------------

BOOL SalInstance::AnyInput( USHORT nType )
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

void SalInstance::Yield( BOOL bWait )
{
    SalYieldMutex*  pYieldMutex = maInstData.mpSalYieldMutex;
    SalData*        pSalData = GetSalData();
    ULONG           nCurThreadId = ImplSalGetCurrentThreadId();
    ULONG           nCount = pYieldMutex->GetAcquireCount( nCurThreadId );
    ULONG           n = nCount;
    while ( n )
    {
        pYieldMutex->release();
        n--;
    }
    if ( pSalData->mnAppThreadId != nCurThreadId )
    {
        WinSendMsg( maInstData.mhComWnd, SAL_MSG_THREADYIELD,
                   (MPARAM)bWait, (MPARAM)(void*)pYieldMutex );

        n = nCount;
        while ( n )
        {
            pYieldMutex->acquire();
            n--;
        }
    }
    else
    {
        ImplSalYield( bWait, TRUE, pYieldMutex );

        n = nCount;
        while ( n )
        {
            // Wenn wir den Mutex nicht bekommen, muessen wir solange
            // warten, bis wir Ihn bekommen
            pYieldMutex->mpInstData->mpSalWaitMutex->acquire();
            if ( pYieldMutex->tryToAcquire() )
            {
                pYieldMutex->mpInstData->mpSalWaitMutex->release();
                n--;
            }
            else
            {
                pYieldMutex->mnWaitCount++;
                pYieldMutex->mpInstData->mpSalWaitMutex->release();
                QMSG aTmpMsg;
                WinGetMsg( pSalData->mhAB, &aTmpMsg, maInstData.mhComWnd, SAL_MSG_RELEASEWAITYIELD, SAL_MSG_RELEASEWAITYIELD );
            }
    }
}

// -----------------------------------------------------------------------

XubString SalInstance::GetFileName()
{
    String aFileName( maInstData.mpArgv[0] );
    return aFileName;
}

// -----------------------------------------------------------------------

USHORT SalInstance::GetCommandLineParamCount()
{
    return maInstData.mnArgc-1;
}

// -----------------------------------------------------------------------

XubString SalInstance::GetCommandLineParam( USHORT nParam )
{
    if ( nParam < maInstData.mnArgc-1 )
    {
        String aParam( maInstData.mpArgv[nParam+1] );
        return aParam;
    }
    else
        return ImplGetSVEmptyStr();
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
    SalYieldMutex*  pYieldMutex = maInstData.mpSalYieldMutex;
    ULONG           nCount = pYieldMutex->GetAcquireCount( ImplSalGetCurrentThreadId() );
    ULONG           n = nCount;
    while ( n )
    {
        pYieldMutex->release();
        n--;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void SalInstance::AcquireYieldMutex( ULONG nCount )
{
    SalYieldMutex*  pYieldMutex = maInstData.mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateFrame( SalFrame* pParent, USHORT nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    return (SalFrame*)WinSendMsg( maInstData.mhComWnd, SAL_MSG_CREATEFRAME, (MPARAM)nSalFrameStyle, (MPARAM)pParent );
}

// -----------------------------------------------------------------------

SalObject* SalInstance::CreateObject( SalFrame* pParent )
{
    // Um auf Main-Thread umzuschalten
    return (SalObject*)WinSendMsg( maInstData.mhComWnd, SAL_MSG_CREATEOBJECT, 0, (MPARAM)pParent );
}

// -----------------------------------------------------------------------

void SalTimer::Start( ULONG nMS )
{
    // Um auf Main-Thread umzuschalten
    SalData* pSalData = GetSalData();
    pSalData->mnNewTimerMS = nMS;
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != ImplSalGetCurrentThreadId() )
            WinPostMsg( pSalData->mpFirstInstance->maInstData.mhComWnd, SAL_MSG_STARTTIMER, 0, 0 );
        else
            WinSendMsg( pSalData->mpFirstInstance->maInstData.mhComWnd, SAL_MSG_STARTTIMER, 0, 0 );
    }
    else
        ImplSalStartTimer();
}
