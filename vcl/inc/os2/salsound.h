/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    virtual void        Play( ULONG nStartTime, ULONG nPlayTime, bool bLoop );
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
