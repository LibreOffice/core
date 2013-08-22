/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SFX_PROGRESS_HXX
#define _SFX_PROGRESS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/solar.h>
#include <tools/link.hxx>
#include <rtl/ustring.hxx>

namespace rtl {
    class OUString;
};
class SfxObjectShell;
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
                                         const rtl::OUString& rText,
                                         sal_uIntPtr nRange, sal_Bool bAllDocs = sal_False,
                                         sal_Bool bWait = sal_True );
    virtual                 ~SfxProgress();

    virtual void            SetText( const OUString& rText );
    sal_Bool                    SetStateText( sal_uIntPtr nVal, const rtl::OUString &rVal, sal_uIntPtr nNewRange = 0 );
    virtual sal_Bool            SetState( sal_uIntPtr nVal, sal_uIntPtr nNewRange = 0 );
    sal_uIntPtr                 GetState() const { return nVal; }

    void                    Resume();
    void                    Suspend();
    sal_Bool                    IsSuspended() const { return bSuspended; }

    void                    UnLock();
    void                    Reschedule();

    void                    Stop();

    static SfxProgress*     GetActiveProgress( SfxObjectShell *pDocSh = 0 );
    static void             EnterLock();
    static void             LeaveLock();

    DECL_DLLPRIVATE_STATIC_LINK( SfxProgress, SetStateHdl, PlugInLoadStatus* );
    DECL_DLLPRIVATE_STATIC_LINK( SfxProgress, DefaultBindingProgress, SvProgressArg* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
