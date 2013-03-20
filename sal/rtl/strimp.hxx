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

#pragma once
#if 1

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
#  define RTL_LOG_STRING_NEW(s)
#  define RTL_LOG_STRING_DELETE(s)

#endif /* INCLUDED_RTL_SOURCE_STRIMP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
