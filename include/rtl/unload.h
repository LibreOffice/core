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
#ifndef INCLUDED_RTL_UNLOAD_H
#define INCLUDED_RTL_UNLOAD_H

#include "sal/config.h"

#include "osl/interlck.h"
#include "osl/time.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

/** @file
    Backwards-compatibility remainders of a removed library unloading feature.
*/

#ifdef __cplusplus
extern "C"
{
#endif

/** Backwards-compatibility remainder of a removed library unloading feature.

    @deprecated Do not use.
*/
typedef struct SAL_DLLPUBLIC_RTTI _rtl_ModuleCount
{
    void ( SAL_CALL * acquire ) ( struct _rtl_ModuleCount * that );
    void ( SAL_CALL * release ) ( struct _rtl_ModuleCount * that );
}rtl_ModuleCount;

/** Backwards-compatibility remainder of a removed library unloading feature.

    @deprecated Do not use.
*/
#define MODULE_COUNT_INIT \
{ {rtl_moduleCount_acquire,rtl_moduleCount_release}, rtl_moduleCount_canUnload, 0, {0, 0}}

/** Backwards-compatibility remainder of a removed library unloading feature.

    @deprecated Do not use.
*/
typedef struct _rtl_StandardModuleCount
{
    rtl_ModuleCount modCnt;
     sal_Bool ( *canUnload ) ( struct _rtl_StandardModuleCount* a, TimeValue* libUnused);
    oslInterlockedCount counter;
    TimeValue unusedSince;
} rtl_StandardModuleCount;

/** Backwards-compatibility remainder of a removed library unloading feature.

    @deprecated Do not use.
*/
SAL_DLLPUBLIC void rtl_moduleCount_acquire(rtl_ModuleCount * that ) SAL_COLD;

/** Backwards-compatibility remainder of a removed library unloading feature.

    @deprecated Do not use.
*/
SAL_DLLPUBLIC void rtl_moduleCount_release( rtl_ModuleCount * that ) SAL_COLD;

/** Backwards-compatibility remainder of a removed library unloading feature.

    @deprecated Do not use.
*/
SAL_DLLPUBLIC sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue* libUnused) SAL_COLD;

#ifdef __cplusplus
}
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
