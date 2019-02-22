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
#include <memory>

namespace rtl {
    class OUString;
};
class SfxObjectShell;
struct SfxProgress_Impl;
struct PlugInLoadStatus;
struct SvProgressArg;

class SFX2_DLLPUBLIC SfxProgress
{
    std::unique_ptr< SfxProgress_Impl >       pImpl;
    sal_uInt32              nVal;
    bool                    bSuspended;

public:
                            SfxProgress( SfxObjectShell* pObjSh,
                                         const OUString& rText,
                                         sal_uInt32 nRange,
                                         bool bWait = true);
    virtual                 ~SfxProgress();

    void                    SetState( sal_uInt32 nVal, sal_uInt32 nNewRange = 0 );
    sal_uInt32              GetState() const { return nVal; }

    void                    Resume();
    void                    Suspend();
    bool                    IsSuspended() const { return bSuspended; }

    static void             Reschedule();

    void                    Stop();

    static SfxProgress*     GetActiveProgress( SfxObjectShell const *pDocSh = nullptr );
    static void             EnterLock();
    static void             LeaveLock();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
