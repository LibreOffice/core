/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sound.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:11:10 $
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

#ifndef _SV_SOUND_HXX
#define _SV_SOUND_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SNDSTYLE_HXX
#include <vcl/sndstyle.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _SV_SALSTYPE_HXX
#include <vcl/salstype.hxx>
#endif

class Window;
class Timer;
class SalSound;
class RMSound;

// ---------
// - Sound -
// ---------

class VCL_DLLPUBLIC Sound
{
private:

    SalSound*           mpSound;
    XubString           maSoundName;
    Window*             mpWindow;
    BYTE*               mpSoundData_NOTUSED;
    ULONG               mnDataLen;
    ULONG               mnSoundLen;
    ULONG               mnStartTime;
    ULONG               mnPlayTime;
    ULONG               mnErrorCode;
    SoundNotification   meNotification;
    BOOL                mbPlaying;
    BOOL                mbLoopMode;
    Link                maNotifyHdl;
    void*               mpExtraData;

private:

                        Sound( const Sound& rSound );
    Sound&              operator=( const Sound& rSound );

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE void ImplNotify( SoundNotification eNotification, ULONG nError );
//#endif // __PRIVATE

public:
                        Sound( Window* pWindow = NULL );
    virtual             ~Sound();

    virtual void        Notify();

    void                SetWindow( Window* pWindow ) { mpWindow = pWindow; }
    Window*             GetWindow() const { return mpWindow; }
    BOOL                SetSoundName( const XubString& rSoundName );
    const XubString&    GetSoundName() const { return maSoundName; }
    ULONG               GetSoundLen() const { return mnSoundLen; }

    void                Play();
    void                Stop();
    void                Pause();
    BOOL                IsPlaying() const { return mbPlaying; }

    void                SetStartTime( ULONG nStartTime = 0 );
    ULONG               GetStartTime() const { return mnStartTime; }
    void                SetPlayTime( ULONG nPlayTime = SOUND_PLAYALL );
    ULONG               GetPlayTime() const { return mnPlayTime; }
    void                SetLoopMode( BOOL bLoop );
    BOOL                IsLoopMode() const { return mbLoopMode; }

    void                ClearError();
    ULONG               GetLastError() const { return mnErrorCode; }
    SoundNotification   GetNotification() const { return meNotification; }

    void                SetNotifyHdl( const Link& rLink ) { maNotifyHdl = rLink; }
    const Link&         GetNotifyHdl() const { return maNotifyHdl; }

    static void         Beep( SoundType eType = SOUND_DEFAULT, Window* pWindow = NULL );

    static void         SetSoundPath( const XubString& rSoundPath );
    static const XubString& GetSoundPath();

    static BOOL         IsSoundFile( const XubString& rSoundPath );
};

#endif // _SV_SOUND_HXX

