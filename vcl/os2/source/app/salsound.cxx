/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salsound.cxx,v $
 * $Revision: 1.5 $
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

#define _SV_SALSOUND_CXX

// ------------
// - Includes -
// ------------

#include <string.h>

#define INCL_DOSMODULEMGR
#include <svpm.h>
#include <saldata.hxx>
#include <salinst.h>
#include <salsound.h>

#define INCL_MCIOS2
#include <os2me.h>

// ---------
// - Names -
// ---------

#define SOUND_LIBNAME   "MDM"
#define SOUND_PROCNAME  "mciSendCommand"

// ------------
// - Fnc cast -
// ------------

typedef ULONG (_cdecl *SALMCIPROC)( USHORT nDeviceId, USHORT nMessage, ULONG nFlags, void* pParam, USHORT );
#define MCIFNC ( (SALMCIPROC) Os2SalSound::mpMCIFnc )

// -----------------
// - Statics init. -
// -----------------

HMODULE     Os2SalSound::mhMCILib = 0;
ULONG       Os2SalSound::mnSoundState = SOUND_STATE_UNLOADED;
void*       Os2SalSound::mpMCIFnc = NULL;

// -------------------
// - Window-Callback -
// -------------------

MRESULT EXPENTRY SoundWndProc( HWND hWnd, ULONG nMsg, MPARAM nPar1, MPARAM nPar2 )
{
    if( MM_MCINOTIFY == nMsg )
    {
        USHORT              nNotify = (USHORT)(ULONG) nPar1;
        SoundNotification   eNotification;
        BOOL                bNotify = TRUE;

        switch( nNotify )
        {
            case( MCI_NOTIFY_SUCCESSFUL ):
                eNotification = SOUND_NOTIFY_SUCCESS;
            break;

            case( MCI_NOTIFY_ABORTED ):
                eNotification = SOUND_NOTIFY_ABORTED;
            break;

            case( MCI_NOTIFY_SUPERSEDED ):
                bNotify = FALSE;
            break;

            default:
                eNotification = SOUND_NOTIFY_ERROR;
            break;
        }

        if( bNotify )
            ( (Os2SalSound*) WinQueryWindowULong( hWnd, 0 ) )->ImplNotify( eNotification, 0 );
    }

    return WinDefWindowProc( hWnd, nMsg, nPar1, nPar2 );
}

// ------------
// - Os2SalSound -
// ------------

Os2SalSound::Os2SalSound() :
    mpProc          ( NULL ),
    mhSoundWnd      ( 0 ),
    mnDeviceId      ( 0 ),
    mbLoop          ( FALSE ),
    mbPaused        ( FALSE )
{
}

// ------------------------------------------------------------------------

Os2SalSound::~Os2SalSound()
{
    Stop();

    if( mnDeviceId )
    {
        MCI_GENERIC_PARMS aGenericParams;
        memset( &aGenericParams, 0, sizeof( aGenericParams ) );
        aGenericParams.hwndCallback = mhSoundWnd;
        MCIFNC( mnDeviceId, MCI_CLOSE, MCI_WAIT, &aGenericParams, 0 );
    }

    if( mhSoundWnd )
        WinDestroyWindow( mhSoundWnd );
}

// ------------------------------------------------------------------------

void Os2SalSound::ImplSetError( ULONG nMciErr )
{
    struct aMapper { DWORD nMci; ULONG nSv; };

    ULONG           nError = SOUNDERR_GENERAL_ERROR;
    static aMapper  aErrArr[] =
    {
        { 0, SOUNDERR_SUCCESS },
        { MCIERR_CANNOT_LOAD_DRIVER, SOUNDERR_CANNOT_LOAD_DRIVER },
        { MCIERR_DEVICE_LOCKED, SOUNDERR_DEVICE_LOCKED },
        { MCIERR_DEVICE_NOT_READY, SOUNDERR_DEVICE_NOT_READY },
        { MCIERR_DEVICE_TYPE_REQUIRED, SOUNDERR_DEVICE_NOT_FOUND },
        { MCIERR_DRIVER, SOUNDERR_CANNOT_LOAD_DRIVER },
        { MCIERR_DRIVER_INTERNAL, SOUNDERR_CANNOT_LOAD_DRIVER  },
        { MCIERR_EXTENSION_NOT_FOUND, SOUNDERR_SOUND_NOT_FOUND },
        { MCIERR_FILE_NOT_FOUND, SOUNDERR_SOUND_NOT_FOUND },
        { MCIERR_HARDWARE, SOUNDERR_HARDWARE_ERROR },
        { MCIERR_INVALID_DEVICE_NAME, SOUNDERR_DEVICE_NOT_FOUND },
        { MCIERR_OUT_OF_MEMORY, SOUNDERR_OUT_OF_MEMORY },
        { MCIERR_UNSUPPORTED_FUNCTION, SOUNDERR_UNSUPPORTED_FUNCTION }
    };

    for( USHORT n=0; n < (sizeof( aErrArr ) / sizeof( aMapper ) ); n++ )
    {
        if( aErrArr[ n ].nMci == nMciErr )
        {
            nError = aErrArr[ n ].nSv;
            break;
        }
    }

    if( nError )
        ImplNotify( SOUND_NOTIFY_ERROR, nError );
}

// ------------------------------------------------------------------------

void Os2SalSound::ImplNotify( SoundNotification eNotification, ULONG nError )
{
    if( mbLoop && ( SOUND_NOTIFY_SUCCESS == eNotification ) && !nError )
        Play( mnStartTime, mnPlayLen, TRUE );

    if( mpProc && mpInst )
        mpProc( mpInst, eNotification, nError );
}

// ------------------------------------------------------------------------

BOOL Os2SalSound::Create()
{
    SalData*    pData = GetSalData();
    BOOL        bRet = FALSE;

    if( SOUND_STATE_UNLOADED == Os2SalSound::mnSoundState )
    {
        if( DosLoadModule( 0, 0, (PCSZ)SOUND_LIBNAME, &Os2SalSound::mhMCILib ) == 0  &&
            DosQueryProcAddr( Os2SalSound::mhMCILib, 0, (PCSZ)SOUND_PROCNAME, (PFN*) &Os2SalSound::mpMCIFnc ) == 0 )
        {
            char* pszClassName = "SoundWin";
            PFNWP pWindowProc = SoundWndProc;

            WinRegisterClass( pData->mhAB, (PCSZ)pszClassName, pWindowProc, 0L, 4 );
            Os2SalSound::mnSoundState = SOUND_STATE_VALID;
            bRet = TRUE;
        }
        else
        {
            if( Os2SalSound::mhMCILib )
                DosFreeModule( Os2SalSound::mhMCILib );

            Os2SalSound::mnSoundState = SOUND_STATE_INVALID;
            ImplNotify( SOUND_NOTIFY_ERROR, SOUNDERR_CANNOT_LOAD_DRIVER );
        }
    }
    else if( SOUND_STATE_VALID == Os2SalSound::mnSoundState )
        bRet = TRUE;

    if( bRet )
    {
        mhSoundWnd = WinCreateWindow( HWND_OBJECT, (PCSZ)"SoundWin", (PCSZ)"Sound", 0, 0, 0, 0, 0, HWND_DESKTOP, HWND_BOTTOM, 0, 0, 0 );
        WinSetWindowULong( mhSoundWnd, 0, (ULONG) this );
    }
    else
        mhSoundWnd = 0;

    return bRet;
}

// ------------------------------------------------------------------------

void Os2SalSound::Release()
{
    if( Os2SalSound::mhMCILib )
        DosFreeModule( Os2SalSound::mhMCILib );

    Os2SalSound::mnSoundState = SOUND_STATE_UNLOADED;
}

// ------------------------------------------------------------------------

bool Os2SalSound::IsValid()
{
  return (Os2SalSound::mnSoundState == SOUND_STATE_VALID);
}

// ------------------------------------------------------------------------

bool Os2SalSound::Init( const XubString& rSoundName, ULONG& rSoundLen )
{
    MCI_OPEN_PARMS  aOpenParams;
    ULONG           nMciErr = 0;

    rSoundLen = 0;

    // clear old device
    if( mnDeviceId )
    {
        Stop();

        MCI_GENERIC_PARMS aGenericParams;
        memset( &aGenericParams, 0, sizeof( aGenericParams ) );
        aGenericParams.hwndCallback = mhSoundWnd;
        nMciErr = MCIFNC( mnDeviceId, MCI_CLOSE, MCI_WAIT, &aGenericParams, 0 );
        mnDeviceId = 0;
    }

    if( rSoundName.Len() )
    {
        // init new device with sound name
        memset( &aOpenParams, 0, sizeof( aOpenParams ) );
        aOpenParams.pszElementName = (PSZ) rSoundName.GetBuffer();
        aOpenParams.hwndCallback = mhSoundWnd;
        nMciErr = MCIFNC( 0, MCI_OPEN, MCI_WAIT | MCI_OPEN_ELEMENT, &aOpenParams, 0 );

        if( !nMciErr )
        {
            // set time format
            MCI_SET_PARMS aSetParams;
            memset( &aSetParams, 0, sizeof( aSetParams ) );
            mnDeviceId = aOpenParams.usDeviceID;
            aSetParams.ulTimeFormat = MCI_FORMAT_MILLISECONDS;
            nMciErr = MCIFNC( mnDeviceId, MCI_SET, MCI_WAIT | MCI_SET_TIME_FORMAT, &aSetParams, 0 );

            if( !nMciErr )
            {
                // get length of sound
                MCI_STATUS_PARMS aStatus;
                memset( &aStatus, 0, sizeof( aStatus ) );
                aStatus.ulItem = MCI_STATUS_LENGTH;
                MCIFNC( mnDeviceId, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, &aStatus, 0 );
                rSoundLen = (ULONG) aStatus.ulReturn;
            }
        }
    }

    if( nMciErr )
        ImplSetError( nMciErr );

    return( nMciErr ? FALSE : TRUE );
}

// ------------------------------------------------------------------------
#if 0
BOOL Os2SalSound::Init( SalFrame* pFrame, const BYTE* pSound, ULONG nDataLen, ULONG& rSoundLen )
{
    rSoundLen = 0UL;
    ImplSetError( MCIERR_FILE_NOT_FOUND );

    return FALSE;
}
#endif

// ------------------------------------------------------------------------

void Os2SalSound::Play( ULONG nStartTime, ULONG nPlayLen, bool bLoop )
{
    if( mnDeviceId )
    {
        ULONG nMciErr = 0;

        if( !mbPaused )
        {
            MCI_SEEK_PARMS aSeekParams;
            memset( &aSeekParams, 0, sizeof( aSeekParams ) );
            aSeekParams.hwndCallback = mhSoundWnd;
            aSeekParams.ulTo = 0;
            nMciErr = MCIFNC( mnDeviceId, MCI_SEEK,MCI_WAIT | MCI_TO, &aSeekParams, 0 );
        }

        mnStartTime = nStartTime;
        mnPlayLen = nPlayLen;
        mbLoop = bLoop;
        mbPaused = FALSE;

        if( !nMciErr )
        {
            MCI_PLAY_PARMS  aPlayParams;
            DWORD           nFlags = MCI_NOTIFY;

            memset( &aPlayParams, 0, sizeof( aPlayParams ) );
            aPlayParams.hwndCallback = mhSoundWnd;

            if( nStartTime )
            {
                aPlayParams.ulFrom = nStartTime;
                nFlags |= MCI_FROM;
            }

            if( nPlayLen != SOUND_PLAYALL )
            {
                aPlayParams.ulTo = nStartTime + nPlayLen;
                nFlags |= MCI_TO;
            }

            nMciErr = MCIFNC( mnDeviceId, MCI_PLAY, nFlags, &aPlayParams, 0 );

            if( !nMciErr )
                mbPaused = FALSE;
        }

        if( nMciErr )
            ImplSetError( nMciErr );
    }
    else
        ImplSetError( MCIERR_EXTENSION_NOT_FOUND );
}

// ------------------------------------------------------------------------

void Os2SalSound::Stop()
{
    if( mnDeviceId )
    {
        MCI_GENERIC_PARMS aGenericParams;
        memset( &aGenericParams, 0, sizeof( aGenericParams ) );
        aGenericParams.hwndCallback = mhSoundWnd;
        mbLoop = mbPaused = FALSE;
        MCIFNC( mnDeviceId, MCI_STOP, MCI_WAIT, &aGenericParams, 0 );
    }
}

// ------------------------------------------------------------------------

void Os2SalSound::Pause()
{
    if( mnDeviceId )
    {
        MCI_GENERIC_PARMS aGenericParams;
        memset( &aGenericParams, 0, sizeof( aGenericParams ) );
        aGenericParams.hwndCallback = mhSoundWnd;
        mbPaused = TRUE;
        MCIFNC( mnDeviceId, MCI_PAUSE, MCI_WAIT, &aGenericParams, 0 );
    }
}

// ------------------------------------------------------------------------

void Os2SalSound::Continue()
{
    Play( 0, 0, mbLoop );
}

// ------------------------------------------------------------------------

bool Os2SalSound::IsLoopMode() const
{
    return mbLoop;
}

// ------------------------------------------------------------------------

bool Os2SalSound::IsPlaying() const
{
    return !mbPaused;
}

// ------------------------------------------------------------------------

bool Os2SalSound::IsPaused() const
{
    return mbPaused;
}
