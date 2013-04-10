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

#include <rtl/unload.h>

#include <rtl/ustring.hxx>
#include <rtl/instance.hxx>
#include <osl/mutex.hxx>

using osl::MutexGuard;

//----------------------------------------------------------------------------

#ifndef DISABLE_DYNLOADING

namespace
{
    class theUnloadingMutex : public rtl::Static<osl::Mutex, theUnloadingMutex>{};
}

static osl::Mutex& getUnloadingMutex()
{
    return theUnloadingMutex::get();
}

#endif

extern "C" void rtl_moduleCount_acquire(rtl_ModuleCount * that )
{
#ifdef DISABLE_DYNLOADING
    (void) that;
#else
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    osl_atomic_increment( &pMod->counter);
#endif
}

extern "C" void rtl_moduleCount_release( rtl_ModuleCount * that )
{
#ifdef DISABLE_DYNLOADING
    (void) that;
#else
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    OSL_ENSURE( pMod->counter >0 , "library counter incorrect" );
    osl_atomic_decrement( &pMod->counter);
    if( pMod->counter == 0)
    {
        MutexGuard guard( getUnloadingMutex());

        if( sal_False == osl_getSystemTime( &pMod->unusedSince) )
        {
            // set the time to 0 if we could not get the time
            pMod->unusedSince.Seconds= 0;
            pMod->unusedSince.Nanosec= 0;
        }
    }
#endif
}

extern "C" sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue * libUnused)
{
#ifdef DISABLE_DYNLOADING
    (void) that;
    (void) libUnused;
    return sal_False;
#else
    if (that->counter == 0)
    {
        MutexGuard guard( getUnloadingMutex());
        if (libUnused && (that->counter == 0))
        {
            memcpy(libUnused, &that->unusedSince, sizeof(TimeValue));
        }
    }
    return (that->counter == 0);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
