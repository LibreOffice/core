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

#ifndef INCLUDED_BACKTRACE_HXX
#define INCLUDED_BACKTRACE_HXX

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

/** print a stack backtrace of the caller function */
static inline void print_stacktrace( FILE *out = stderr, unsigned int max_frames = 80 )
{
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace( addrlist, sizeof(addrlist) / sizeof(void*) );

    if (addrlen == 0) {
	fprintf(out, "  <empty, possibly corrupt>\n");
	return;
    }

    // resolve addresses into strings
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // iterate over the returned symbol lines
    for (int i = 0; i < addrlen; i++)
    {
	    fprintf(out, "  %s\n", symbollist[i]);
    }

    free(symbollist);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
