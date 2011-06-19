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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <osl/mutex.hxx>
#include <rtl/process.h>
#include <rtl/ustring.hxx>

namespace {

rtl_uString ** g_ppCommandArgs = 0;
sal_uInt32     g_nCommandArgCount = 0;

struct ArgHolder
{
    ~ArgHolder();
};

ArgHolder::~ArgHolder()
{
    while (g_nCommandArgCount > 0)
        rtl_uString_release (g_ppCommandArgs[--g_nCommandArgCount]);

    rtl_freeMemory (g_ppCommandArgs);
    g_ppCommandArgs = 0;
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
            (rtl_uString**)rtl_allocateZeroMemory (n * sizeof(rtl_uString*));
        for (i = 0; i < n; i++)
        {
            rtl_uString * pArg = 0;
            osl_getCommandArg (i, &pArg);
            if (('-' == pArg->buffer[0] || '/' == pArg->buffer[0]) &&
                 'e' == pArg->buffer[1] &&
                 'n' == pArg->buffer[2] &&
                 'v' == pArg->buffer[3] &&
                 ':' == pArg->buffer[4] &&
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
    return (result);
}

sal_uInt32 SAL_CALL rtl_getAppCommandArgCount (void)
{
    init();
    return g_nCommandArgCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
