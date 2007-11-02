/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsound.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:46:36 $
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

#ifndef _SV_SALSOUND_H
#define _SV_SALSOUND_H

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SALFRAME_HXX
#include <vcl/salframe.hxx>
#endif
#ifndef _SV_SALSTYPE_HXX
#include <vcl/salstype.hxx>
#endif
#ifndef _SV_SALSOUND_HXX
#include <vcl/salsound.hxx>
#endif

// ------------
// - SalSound -
// ------------

//class SalSound
class Os2SalSound : public SalSound
{
private:

    static HMODULE      mhMCILib;
    static ULONG        mnSoundState;
    static void*        mpMCIFnc;
    SALSOUNDPROC        mpProc;
    void*               mpInst;
    ULONG               mnStartTime;
    ULONG               mnPlayLen;
    HWND                mhSoundWnd;
    USHORT              mnDeviceId;
    BOOL                mbLoop;
    BOOL                mbPaused;

public:

    void                ImplSetError( ULONG nMciErr );
    void                ImplNotify( SoundNotification eNotification, ULONG nError );

public:

    BOOL                Create();
    //static BOOL           IsValid() { return( SOUND_STATE_VALID == Os2SalSound::mnSoundState ); }
    //BOOL              Init( SalFrame* pFrame, const String& rSoundName, ULONG& rSoundLen );
    //BOOL              Init( SalFrame* pFrame, const BYTE* pSound, ULONG nDataLen, ULONG& rSoundLen );

    void                SetNotifyProc( void* pInst, SALSOUNDPROC pProc )
                            { mpInst = pInst; mpProc = pProc; }
public:
    Os2SalSound();
    virtual ~Os2SalSound();

    virtual bool        IsValid();
    virtual bool        Init( const String& rSoundName, ULONG& rSoundLen );
       virtual void     Play( ULONG nStartTime, ULONG nPlayTime, bool bLoop );
    virtual void        Stop();
    virtual void        Pause();
    virtual void        Continue();
    virtual bool        IsLoopMode() const;
    virtual bool        IsPlaying() const;
    virtual bool        IsPaused() const;

    bool                ImplCreate();
    void                ImplDestroy();
    static void         Release();
};

#endif // _SV_SALSOUND_H
