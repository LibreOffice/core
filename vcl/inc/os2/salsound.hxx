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



#ifndef _SV_SALSOUND_HXX
#define _SV_SALSOUND_HXX

#include <tools/gen.hxx>
#include <sv.h>
#include <tools/string.hxx>
#include <salframe.hxx>
#ifndef _SV_SALSTYPE_HXX
#include <salstype.hxx>
#endif
#include <salsound.hxx>

// ------------
// - SalSound -
// ------------

class SalSound
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

                        SalSound();
                        ~SalSound();

    BOOL                Create();
    static void         Release();
    static BOOL         IsValid() { return( SOUND_STATE_VALID == SalSound::mnSoundState ); }

    BOOL                Init( SalFrame* pFrame, const String& rSoundName, ULONG& rSoundLen );
    BOOL                Init( SalFrame* pFrame, const BYTE* pSound, ULONG nDataLen, ULONG& rSoundLen );
    void                Play( ULONG nStartTime, ULONG nPlayTime, BOOL bLoop );
    void                Stop();
    void                Pause();

    void                SetNotifyProc( void* pInst, SALSOUNDPROC pProc )
                            { mpInst = pInst; mpProc = pProc; }
};

#endif // _SV_SALSOUND_HXX
