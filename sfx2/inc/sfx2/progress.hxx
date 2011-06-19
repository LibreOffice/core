/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    DECL_DLLPRIVATE_STATIC_LINK( SfxProgress, SetStateHdl, PlugInLoadStatus* );
    DECL_DLLPRIVATE_STATIC_LINK( SfxProgress, DefaultBindingProgress, SvProgressArg* );
    SAL_DLLPRIVATE bool  StatusBarManagerGone_Impl(SfxStatusBarManager*pStb);
    SAL_DLLPRIVATE const String& GetStateText_Impl() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
