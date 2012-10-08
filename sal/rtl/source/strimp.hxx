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

#ifndef INCLUDED_RTL_SOURCE_STRIMP_HXX
#define INCLUDED_RTL_SOURCE_STRIMP_HXX

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
