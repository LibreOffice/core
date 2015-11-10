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
#ifndef INCLUDED_SFX2_PROGRESS_HXX
#define INCLUDED_SFX2_PROGRESS_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <tools/link.hxx>

namespace rtl {
    class OUString;
};
class SfxObjectShell;
struct SfxProgress_Impl;
struct PlugInLoadStatus;
struct SvProgressArg;

class SFX2_DLLPUBLIC SfxProgress
{
    SfxProgress_Impl*       pImp;
    sal_uIntPtr             nVal;
    bool                    bSuspended;

public:
                            SfxProgress( SfxObjectShell* pObjSh,
                                         const rtl::OUString& rText,
                                         sal_uIntPtr nRange, bool bAllDocs = false,
                                         bool bWait = true );
    virtual                 ~SfxProgress();

    bool                    SetStateText( sal_uIntPtr nVal, const rtl::OUString &rVal, sal_uIntPtr nNewRange = 0 );
    bool                    SetState( sal_uIntPtr nVal, sal_uIntPtr nNewRange = 0 );
    sal_uIntPtr             GetState() const { return nVal; }

    void                    Resume();
    void                    Suspend();
    bool                    IsSuspended() const { return bSuspended; }

    void                    UnLock();
    void                    Reschedule();

    void                    Stop();

    static SfxProgress*     GetActiveProgress( SfxObjectShell *pDocSh = nullptr );
    static void             EnterLock();
    static void             LeaveLock();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
