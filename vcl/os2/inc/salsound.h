/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salsound.h,v $
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

#ifndef _SV_SALSOUND_H
#define _SV_SALSOUND_H

#include <tools/gen.hxx>
#include <sv.h>
#include <tools/string.hxx>
#include <vcl/salframe.hxx>
#ifndef _SV_SALSTYPE_HXX
#include <vcl/salstype.hxx>
#endif
#include <vcl/salsound.hxx>

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
