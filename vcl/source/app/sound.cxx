/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sound.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:49:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif



// ----------------------
// - SalSound-Callback  -
// ----------------------

void SalSoundProc( void* pInst, SoundNotification eNotification, ULONG nError )
{
    ( (Sound*) pInst )->ImplNotify( eNotification, nError );
}

// ---------
// - Sound -
// ---------

Sound::Sound( Window* pWindow ) :
            mpWindow        ( pWindow ),
            mnDataLen       ( 0UL ),
            mnSoundLen      ( 0UL ),
            mnStartTime     ( 0UL ),
            mnPlayTime      ( SOUND_PLAYALL ),
            mnErrorCode     ( 0UL ),
            meNotification  ( SOUND_NOTIFY_SUCCESS ),
            mbPlaying       ( FALSE ),
            mbLoopMode      ( FALSE )
{
    mpSound = ImplGetSVData()->mpDefInst->CreateSalSound();

    if( mpSound->IsValid() )
        mpSound->SetNotifyProc( this, SalSoundProc );
}

// -----------------------------------------------------------------------

Sound::~Sound()
{
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
    BOOL bRet( FALSE );

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

        mpSound->Init( rSoundName, mnSoundLen );
    }
    else if( mpSound->IsValid() )
    {
        INetURLObject   aSoundURL( rSoundName );
        String          aSoundName, aTmp;
        BOOL            bValidName( FALSE );

        // #106654# Accept only local sound files
        if( aSoundURL.GetProtocol() == INET_PROT_FILE )
        {
            utl::LocalFileHelper::ConvertURLToPhysicalName( aSoundURL.GetMainURL( INetURLObject::NO_DECODE ), aSoundName );
            bValidName = TRUE;
        }
        else if( aSoundURL.GetProtocol() == INET_PROT_NOT_VALID &&
                 ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rSoundName, aTmp ) )
        {
            aSoundName = rSoundName;
            bValidName = TRUE;
        }
        else
        {
            // no valid sound file name
            aSoundName = String();

            // #106654# Don't set bRet to true for invalid sound file
            // names, but init with empty string, anyway
            mpSound->Init( aSoundName, mnSoundLen );
        }

        if( bValidName )
            bRet = mpSound->Init( aSoundName, mnSoundLen );
    }

    maSoundName = rSoundName;

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
}

// -----------------------------------------------------------------------

void Sound::SetPlayTime( ULONG nPlayTime )
{
    mnPlayTime = nPlayTime;
}

// -----------------------------------------------------------------------

void Sound::SetLoopMode( BOOL bLoop )
{
    mbLoopMode = bLoop;
}

// -----------------------------------------------------------------------

void Sound::ClearError()
{
    mnErrorCode = 0;
}

// -----------------------------------------------------------------------

void Sound::Play()
{
    BOOL bRet;

    if( mpSound->IsValid() && !mnErrorCode )
    {
        mpSound->Play( mnStartTime, mnPlayTime, mbLoopMode );
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
    {
        Window* pDefWindow = ImplGetDefaultWindow();
        pDefWindow->ImplGetFrame()->Beep( eType );
    }
    else
        pWindow->ImplGetFrame()->Beep( eType );
}

// -----------------------------------------------------------------------

void Sound::SetSoundPath( const XubString& )
{
}

// -----------------------------------------------------------------------

const XubString& Sound::GetSoundPath()
{
    return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

BOOL Sound::IsSoundFile( const XubString& rSoundPath )
{
    BOOL bRet = FALSE;

    if( rSoundPath.Len() )
    {
        INetURLObject   aSoundURL( rSoundPath );
        String          aSoundName;

        if( aSoundURL.GetProtocol() != INET_PROT_NOT_VALID )
            aSoundName = aSoundURL.GetMainURL( INetURLObject::NO_DECODE );
        else if( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( rSoundPath, aSoundName ) )
            aSoundName.Erase();

        if( aSoundName.Len() )
        {
            SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aSoundName, STREAM_READ );

            if( pIStm )
            {
                sal_Char aData[ 12 ];

                if( ( pIStm->Read( aData, 12 ) == 12 ) && !pIStm->GetError() )
                {
                    // check for WAV
                    bRet = ( aData[ 0 ] == 'R' && aData[ 1 ] == 'I' && aData[ 2 ] == 'F' && aData[ 3 ] == 'F' &&
                             aData[ 8 ] == 'W' && aData[ 9 ] == 'A' && aData[ 10 ] == 'V' && aData[ 11 ] == 'E' );
                }

                delete pIStm;
            }

            if( !bRet )
            {
                // check it the hard way
                Sound aTestSound;
                bRet = aTestSound.SetSoundName( rSoundPath );
            }
        }
    }

    return bRet;
}
