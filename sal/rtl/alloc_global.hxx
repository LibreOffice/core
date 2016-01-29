/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SAL_RTL_ALLOC_GLOBAL_HXX
#define INCLUDED_SAL_RTL_ALLOC_GLOBAL_HXX

#include "sal/types.h"

typedef void *SAL_CALL (rtl_AllocateMemory) (sal_Size n);
typedef void  SAL_CALL (rtl_FreeMemory)     (void * p);

// String allocation function pointers

extern rtl_AllocateMemory *rtl_StringAllocateMemory;
extern rtl_FreeMemory     *rtl_StringFreeMemory;

extern void SAL_CALL rtl_string_alloc_init();
extern void SAL_CALL rtl_string_alloc_fini();

#endif // INCLUDED_SAL_RTL_ALLOC_GLOBAL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
