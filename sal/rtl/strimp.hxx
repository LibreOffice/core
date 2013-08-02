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

#ifndef INCLUDED_RTL_SOURCE_STRIMP_HXX
#define INCLUDED_RTL_SOURCE_STRIMP_HXX

#include <config_probes.h>
#if USE_SDT_PROBES
#include <sys/sdt.h>
#endif

#include <osl/interlck.h>

#include "sal/types.h"

/* ======================================================================= */
/* Help functions for String and UString                                   */
/* ======================================================================= */

/*
 * refCount is opaqueincludes 2 bit-fields;
 * MSB:   'interned' - is stored in the intern hash
 * MSB-1: 'static'   - is a const / static string,
 *                     do no ref counting
 */
#define SAL_STRING_INTERN_FLAG 0x80000000
#define SAL_STRING_STATIC_FLAG 0x40000000
#define SAL_STRING_REFCOUNT(a) ((a) & 0x3fffffff)

#define SAL_STRING_IS_INTERN(a) ((a)->refCount & SAL_STRING_INTERN_FLAG)
#define SAL_STRING_IS_STATIC(a) ((a)->refCount & SAL_STRING_STATIC_FLAG)

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix );

sal_Bool rtl_ImplIsWhitespace( sal_Unicode c );

// string lifetime instrumentation / diagnostics
#if USE_SDT_PROBES
#  define PROBE_SNAME(n,b) n ## _ ## b
#  define PROBE_NAME(n,b) PROBE_SNAME(n,b)
#  define PROBE_NEW PROBE_NAME (new_string,RTL_LOG_STRING_BITS)
#  define PROBE_DEL PROBE_NAME (delete_string,RTL_LOG_STRING_BITS)
#  define RTL_LOG_STRING_NEW(s) \
    DTRACE_PROBE4(libreoffice, PROBE_NEW, s, \
                  (s)->refCount, (s)->length, (s)->buffer)
#  define RTL_LOG_STRING_DELETE(s) \
    DTRACE_PROBE4(libreoffice, PROBE_DEL, s, \
                  (s)->refCount, (s)->length, (s)->buffer)
#else
#  define RTL_LOG_STRING_NEW(s)
#  define RTL_LOG_STRING_DELETE(s)
#endif /* USE_SDT_PROBES */

#endif /* INCLUDED_RTL_SOURCE_STRIMP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
