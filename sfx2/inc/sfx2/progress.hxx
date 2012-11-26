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


#ifndef _SFX_PROGRESS_HXX
#define _SFX_PROGRESS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/solar.h>
#include <tools/link.hxx>

class String;
class SfxObjectShell;
class SfxStatusBarManager;
class INetHint;
class SvDispatch;
struct SfxProgress_Impl;
struct PlugInLoadStatus;
struct SvProgressArg;

//=======================================================================

class SFX2_DLLPUBLIC SfxProgress
{
    SfxProgress_Impl*       pImp;
    sal_uIntPtr                 nVal;
    sal_Bool                    bSuspended;

public:
                            SfxProgress( SfxObjectShell* pObjSh,
                                         const String& rText,
                                         sal_uIntPtr nRange, sal_Bool bAllDocs = sal_False,
                                         sal_Bool bWait = sal_True );
    virtual                 ~SfxProgress();

    virtual void            SetText( const String& rText );
    sal_Bool                    SetStateText( sal_uIntPtr nVal, const String &rVal, sal_uIntPtr nNewRange = 0 );
    virtual sal_Bool            SetState( sal_uIntPtr nVal, sal_uIntPtr nNewRange = 0 );
    sal_uIntPtr                 GetState() const { return nVal; }

    void                    Resume();
    void                    Suspend();
    sal_Bool                    IsSuspended() const { return bSuspended; }

    void                    Lock();
    void                    UnLock();
    void                    Reschedule();

    void                    Stop();

    void                    SetWaitMode( sal_Bool bWait );
    sal_Bool                    GetWaitMode() const;

    static SfxProgress*     GetActiveProgress( SfxObjectShell *pDocSh = 0 );
    static void             EnterLock();
    static void             LeaveLock();

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_STATIC_LINK( SfxProgress, SetStateHdl, PlugInLoadStatus* );
    DECL_DLLPRIVATE_STATIC_LINK( SfxProgress, DefaultBindingProgress, SvProgressArg* );
    SAL_DLLPRIVATE bool StatusBarManagerGone_Impl(SfxStatusBarManager*pStb);
    SAL_DLLPRIVATE const String& GetStateText_Impl() const;
//#endif
};

#endif

