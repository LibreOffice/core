/*************************************************************************
 *
 *  $RCSfile: sound.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-14 13:15:49 $
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

#define _SV_SOUND_CXX

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef REMOTE_APPSERVER
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#else
#include <rmwindow.hxx>
#include <rmsound.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif

#pragma hdrstop

// ----------------------
// - SalSound-Callback  -
// ----------------------

#ifndef REMOTE_APPSERVER

void SalSoundProc( void* pInst, SoundNotification eNotification, ULONG nError )
{
    ( (Sound*) pInst )->ImplNotify( eNotification, nError );
}

#endif

// ---------
// - Sound -
// ---------

Sound::Sound( Window* pWindow ) :
            mpWindow        ( pWindow ),
            mpSoundData     ( NULL ),
            mnDataLen       ( 0UL ),
            mnSoundLen      ( 0UL ),
            mnStartTime     ( 0UL ),
            mnPlayTime      ( SOUND_PLAYALL ),
            mnErrorCode     ( 0UL ),
            meNotification  ( SOUND_NOTIFY_SUCCESS ),
            mbPlaying       ( FALSE ),
            mbLoopMode      ( FALSE )
{
#ifndef REMOTE_APPSERVER

    mpSound = new SalSound;
    mpSound->Create();

    if( mpSound->IsValid() )
        mpSound->SetNotifyProc( this, SalSoundProc );

#else

    mpSound = new RMSound;
    mpSound->Create( this );

#endif
}

// -----------------------------------------------------------------------

Sound::~Sound()
{
    if( mpSoundData )
        SvMemFree( mpSoundData );

    delete mpSound;
}

// -----------------------------------------------------------------------

void Sound::ImplNotify( SoundNotification eNotification, ULONG nError )
{
    meNotification = eNotification;
    mbPlaying = FALSE;

    if( SOUND_NOTIFY_ERROR == meNotification )
        mnErrorCode = nError;

    Notify();

    if( maNotifyHdl.IsSet() )
        maNotifyHdl.Call( this );
}

// -----------------------------------------------------------------------

void Sound::Notify()
{
}

// -----------------------------------------------------------------------

BOOL Sound::SetSoundName( const XubString& rSoundName )
{
    BOOL bRet;

    if( !rSoundName.Len() )
    {
        mnDataLen = 0UL;
        mnSoundLen = 0UL;
        mnStartTime = 0UL;
        mnPlayTime = SOUND_PLAYALL;
        mnErrorCode = 0UL;
        meNotification = SOUND_NOTIFY_SUCCESS;
        mbPlaying = FALSE;
        mbLoopMode = FALSE;
        bRet = TRUE;

#ifdef REMOTE_APPSERVER
        if( mpSoundData )
        {
            SvMemFree( mpSoundData );
            mpSoundData = NULL;
        }

        // if IFace is init., destroy it and create a new one
        if( maSoundName.Len() )
        {
            delete mpSound;
            mpSound = new RMSound;
            mpSound->Create( this );
        }
#else
        mpSound->Init( NULL, rSoundName, mnSoundLen );
#endif
    }
    else if( mpSound->IsValid() )
    {
        INetURLObject   aSoundURL( rSoundName );
        String          aSoundName, aTmp;

        if( aSoundURL.GetProtocol() != INET_PROT_NOT_VALID )
            ::utl::LocalFileHelper::ConvertURLToPhysicalName( aSoundURL.GetMainURL(), aSoundName );
        else if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rSoundName, aTmp ) )
            aSoundName = rSoundName;
        else
        {
            DBG_ERROR( "invalid sound file name" );
            aSoundName = String();
        }

        bRet = mpSound->Init( NULL, aSoundName, mnSoundLen );
    }
    else
        bRet = FALSE;

    maSoundName = rSoundName;

    // if sound could not be initialized, but we've gotten _no_
    // notification ==> create common error notification
    if( !bRet && !mnErrorCode )
        ImplNotify( SOUND_NOTIFY_ERROR, SOUNDERR_GENERAL_ERROR );

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Sound::SetSoundData( const BYTE* pSoundData, ULONG nDataLen )
{
    BOOL bRet;

    if( mpSoundData )
        SvMemFree( mpSoundData );

    mpSoundData = (BYTE*) SvMemAlloc( mnDataLen = nDataLen );
    HMEMCPY( mpSoundData, pSoundData, nDataLen );

    if( mpSound->IsValid() )
        bRet = mpSound->Init( NULL, mpSoundData, mnDataLen, mnSoundLen );
    else
        bRet = FALSE;

    // if sound could not be initialized, but we've gotten _no_
    // notification ==> create common error notification
    if( !bRet && !mnErrorCode )
        ImplNotify( SOUND_NOTIFY_ERROR, SOUNDERR_GENERAL_ERROR );

    return bRet;
}

// -----------------------------------------------------------------------

void Sound::SetStartTime( ULONG nStartTime )
{
    mnStartTime = nStartTime;

#ifdef REMOTE_APPSERVER
    mpSound->SetStartTime( nStartTime );
#endif
}

// -----------------------------------------------------------------------

void Sound::SetPlayTime( ULONG nPlayTime )
{
    mnPlayTime = nPlayTime;

#ifdef REMOTE_APPSERVER
    mpSound->SetStartTime( nPlayTime );
#endif
}

// -----------------------------------------------------------------------

void Sound::SetLoopMode( BOOL bLoop )
{
    mbLoopMode = bLoop;

#ifdef REMOTE_APPSERVER
    mpSound->SetStartTime( bLoop );
#endif
}

// -----------------------------------------------------------------------

void Sound::ClearError()
{
    mnErrorCode = 0;

#ifdef REMOTE_APPSERVER
    mpSound->ClearError();
#endif
}

// -----------------------------------------------------------------------

void Sound::Play()
{
    BOOL bRet;

    if( mpSound->IsValid() && !mnErrorCode )
    {
#ifndef REMOTE_APPSERVER
        mpSound->Play( mnStartTime, mnPlayTime, mbLoopMode );
#else
        mpSound->Play();
#endif

        mbPlaying = TRUE;
    }
    else
        bRet = FALSE;
}

// -----------------------------------------------------------------------

void Sound::Stop()
{
    mbPlaying = FALSE;

    if( mpSound->IsValid() )
        mpSound->Stop();
}

// -----------------------------------------------------------------------

void Sound::Pause()
{
    mbPlaying = FALSE;

    if( mpSound->IsValid() )
        mpSound->Pause();
}

// -----------------------------------------------------------------------

void Sound::Beep( SoundType eType, Window* pWindow )
{
    if( !pWindow )
        ImplGetDefaultWindow()->ImplGetFrame()->Beep( eType );
    else
        pWindow->ImplGetFrame()->Beep( eType );
}

// -----------------------------------------------------------------------

void Sound::SetSoundPath( const XubString& rSoundPath )
{
}

// -----------------------------------------------------------------------

const XubString& Sound::GetSoundPath()
{
    return ImplGetSVEmptyStr();
}
