/*************************************************************************
 *
 *  $RCSfile: salsound.cxx,v $
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

#define _SV_SALSOUND_CXX

// ------------
// - Includes -
// ------------

#include <string.h>

#define INCL_DOSMODULEMGR
#include <tools/svpm.h>

#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

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
#define MCIFNC ( (SALMCIPROC) SalSound::mpMCIFnc )

// -----------------
// - Statics init. -
// -----------------

HMODULE     SalSound::mhMCILib = 0;
ULONG       SalSound::mnSoundState = SOUND_STATE_UNLOADED;
void*       SalSound::mpMCIFnc = NULL;

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
            ( (SalSound*) WinQueryWindowULong( hWnd, 0 ) )->ImplNotify( eNotification, 0 );
    }

    return WinDefWindowProc( hWnd, nMsg, nPar1, nPar2 );
}

// ------------
// - SalSound -
// ------------

SalSound::SalSound() :
    mpProc          ( NULL ),
    mhSoundWnd      ( 0 ),
    mnDeviceId      ( 0 ),
    mbLoop          ( FALSE ),
    mbPaused        ( FALSE )
{
}

// ------------------------------------------------------------------------

SalSound::~SalSound()
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

void SalSound::ImplSetError( ULONG nMciErr )
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

void SalSound::ImplNotify( SoundNotification eNotification, ULONG nError )
{
    if( mbLoop && ( SOUND_NOTIFY_SUCCESS == eNotification ) && !nError )
        Play( mnStartTime, mnPlayLen, TRUE );

    if( mpProc && mpInst )
        mpProc( mpInst, eNotification, nError );
}

// ------------------------------------------------------------------------

BOOL SalSound::Create()
{
    SalData*    pData = GetSalData();
    BOOL        bRet = FALSE;

    if( SOUND_STATE_UNLOADED == SalSound::mnSoundState )
    {
        if( DosLoadModule( 0, 0, SOUND_LIBNAME, &SalSound::mhMCILib ) == 0  &&
            DosQueryProcAddr( SalSound::mhMCILib, 0, SOUND_PROCNAME, (PFN*) &SalSound::mpMCIFnc ) == 0 )
        {
            char* pszClassName = "SoundWin";
            PFNWP pWindowProc = SoundWndProc;

            WinRegisterClass( pData->mhAB, pszClassName, pWindowProc, 0L, 4 );
            SalSound::mnSoundState = SOUND_STATE_VALID;
            bRet = TRUE;
        }
        else
        {
            if( SalSound::mhMCILib )
                DosFreeModule( SalSound::mhMCILib );

            SalSound::mnSoundState = SOUND_STATE_INVALID;
            ImplNotify( SOUND_NOTIFY_ERROR, SOUNDERR_CANNOT_LOAD_DRIVER );
        }
    }
    else if( SOUND_STATE_VALID == SalSound::mnSoundState )
        bRet = TRUE;

    if( bRet )
    {
        mhSoundWnd = WinCreateWindow( HWND_OBJECT, "SoundWin", "Sound", 0, 0, 0, 0, 0, HWND_DESKTOP, HWND_BOTTOM, 0, 0, 0 );
        WinSetWindowULong( mhSoundWnd, 0, (ULONG) this );
    }
    else
        mhSoundWnd = 0;

    return bRet;
}

// ------------------------------------------------------------------------

void SalSound::Release()
{
    if( SalSound::mhMCILib )
        DosFreeModule( SalSound::mhMCILib );

    SalSound::mnSoundState = SOUND_STATE_UNLOADED;
}

// ------------------------------------------------------------------------

BOOL SalSound::Init( SalFrame* pFrame, const String& rSoundName, ULONG& rSoundLen )
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
        aOpenParams.pszElementName = (char*) rSoundName.GetStr();
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

BOOL SalSound::Init( SalFrame* pFrame, const BYTE* pSound, ULONG nDataLen, ULONG& rSoundLen )
{
    rSoundLen = 0UL;
    ImplSetError( MCIERR_FILE_NOT_FOUND );

    return FALSE;
}

// ------------------------------------------------------------------------

void SalSound::Play( ULONG nStartTime, ULONG nPlayLen, BOOL bLoop )
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

void SalSound::Stop()
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

void SalSound::Pause()
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
