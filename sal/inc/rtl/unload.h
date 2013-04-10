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
#ifndef _RTL_UNLOAD_H_
#define _RTL_UNLOAD_H_

#include "sal/config.h"

#include "osl/interlck.h"
#include "osl/module.h"
#include "osl/time.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

/** C-interface for a module reference counting
 */
#ifdef __cplusplus
extern "C"
{
#endif

/**
Pointers to <code>rtl_ModuleCount</code> are passed as arguments to the default factory creator
functions: <code>createSingleComponentFactory</code>, <code>createSingleFactory</code>,
<code>createOneInstanceFactory</code>.
The factory implementation is calling <code>rtl_ModuleCount.acquire</code> when it is being
constructed and it is calling <code>rtl_ModuleCount.release</code>. The implementations of
<code>acquire</code>
and <code>release</code> should influence the return value of <code>component_canUnload</code>
in a way that it
returns <code>sal_False</code> after <code>acquire</code> has been called. That is the module will not be unloaded
once a default factory has been created. A call to <code>release</code> may cause
<code>component_canUnload</code> to return <code>sal_False</code>, but only if there are
no object alive which
originated from the module. These objects are factory instances and the service instances
which have been created by these factories.
<p>
It is not necessary to synchronize <code>acquire</code> and <code>release</code> as a whole.
Simply sychronize the
access to a counter variable, e.g. the <code>rtl_moduleCount_release</code> implementation:
<pre>
extern "C" void rtl_moduleCount_acquire(rtl_ModuleCount * that )
{
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    osl_atomic_increment( &pMod->counter);
}
</pre>
The SAL library offers functions that can be used for <code>acquire</code> and <code>release</code>. See struct
<code>_rtl_StandardModuleCount</code>.
*/
typedef struct _rtl_ModuleCount
{
    void ( SAL_CALL * acquire ) ( struct _rtl_ModuleCount * that );
    void ( SAL_CALL * release ) ( struct _rtl_ModuleCount * that );
}rtl_ModuleCount;


#define MODULE_COUNT_INIT \
{ {rtl_moduleCount_acquire,rtl_moduleCount_release}, rtl_moduleCount_canUnload, 0, {0, 0}}

/**
This struct can be used to implement the unloading mechanism. To make a UNO library
unloadable create one global instance of this struct in the module. To initialize it one
uses the MODULE_COUNT_INIT macro.

<pre>rtl_StandardModuleCount globalModuleCount= MODULE_COUNT_INIT</pre>;
*/
typedef struct _rtl_StandardModuleCount
{
    rtl_ModuleCount modCnt;
     sal_Bool ( *canUnload ) ( struct _rtl_StandardModuleCount* a, TimeValue* libUnused);
    oslInterlockedCount counter;
    TimeValue unusedSince;
} rtl_StandardModuleCount;

/** Default implementation for <code>rtl_ModuleCount.acquire</code>. Use this function along with
<code>rtl_StandardModuleCount</code>.
*/
SAL_DLLPUBLIC void rtl_moduleCount_acquire(rtl_ModuleCount * that );
/** Default implementation for <code>rtl_ModuleCount.release</code>.
Use this function along with
<code>rtl_StandardModuleCount</code>.
*/
SAL_DLLPUBLIC void rtl_moduleCount_release( rtl_ModuleCount * that );

/** Default implementation for <code>component_canUnload</code>. Use this function along with
<code>rtl_StandardModuleCount</code>.
*/
SAL_DLLPUBLIC sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue* libUnused);


#ifdef __cplusplus
}
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
