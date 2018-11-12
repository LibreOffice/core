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

#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <rtl/process.h>
#include <rtl/ustring.hxx>

namespace {

rtl_uString ** g_ppCommandArgs = nullptr;
sal_uInt32     g_nCommandArgCount = 0;

struct ArgHolder
{
    ~ArgHolder();
};

ArgHolder::~ArgHolder()
{
    while (g_nCommandArgCount > 0)
        rtl_uString_release (g_ppCommandArgs[--g_nCommandArgCount]);

    free (g_ppCommandArgs);
    g_ppCommandArgs = nullptr;
}

// The destructor of this static ArgHolder is "activated" by the assignments to
// g_ppCommandArgs and g_nCommandArgCount in init():
ArgHolder argHolder;

void init()
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    if (!g_ppCommandArgs)
    {
        sal_Int32 i, n = osl_getCommandArgCount();

        g_ppCommandArgs =
            static_cast<rtl_uString**>(rtl_allocateZeroMemory (n * sizeof(rtl_uString*)));
        for (i = 0; i < n; i++)
        {
            rtl_uString * pArg = nullptr;
            osl_getCommandArg (i, &pArg);
            if ((pArg->buffer[0] == '-' || pArg->buffer[0] == '/') &&
                 pArg->buffer[1] == 'e' &&
                 pArg->buffer[2] == 'n' &&
                 pArg->buffer[3] == 'v' &&
                 pArg->buffer[4] == ':' &&
                rtl_ustr_indexOfChar (&(pArg->buffer[5]), '=') >= 0 )
            {
                // ignore.
                rtl_uString_release (pArg);
            }
            else
            {
                // assign.
                g_ppCommandArgs[g_nCommandArgCount++] = pArg;
            }
        }
    }
}

}

oslProcessError SAL_CALL rtl_getAppCommandArg (
    sal_uInt32 nArg, rtl_uString **ppCommandArg)
{
    init();
    oslProcessError result = osl_Process_E_NotFound;
    if( nArg < g_nCommandArgCount )
    {
        rtl_uString_assign( ppCommandArg, g_ppCommandArgs[nArg] );
        result = osl_Process_E_None;
    }
    return result;
}

sal_uInt32 SAL_CALL rtl_getAppCommandArgCount()
{
    init();
    return g_nCommandArgCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
