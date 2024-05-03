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

#include <sal/config.h>

#include <cstdio>
#include <cstdlib>

#include <sal/types.h>
#include <rtl/alloc.h>
#include <rtl/random.h>
#include <oslrandom.h>

namespace {

struct RandomPool_Impl
{
};

}

rtlRandomPool SAL_CALL rtl_random_createPool() SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl = static_cast< RandomPool_Impl* >(rtl_allocateZeroMemory(sizeof(RandomPool_Impl)));
    return static_cast< rtlRandomPool >(pImpl);
}

void SAL_CALL rtl_random_destroyPool(rtlRandomPool Pool) SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl = static_cast< RandomPool_Impl* >(Pool);
    if (pImpl)
    {
        rtl_freeZeroMemory (pImpl, sizeof(RandomPool_Impl));
    }
}

rtlRandomError SAL_CALL rtl_random_addBytes(
    rtlRandomPool Pool, const void *Buffer, sal_Size /*Bytes*/) SAL_THROW_EXTERN_C()
{
    RandomPool_Impl *pImpl = static_cast< RandomPool_Impl* >(Pool);
    const sal_uInt8 *pBuffer = static_cast< const sal_uInt8* >(Buffer);

    if (!pImpl || !pBuffer)
        return rtl_Random_E_Argument;

    return rtl_Random_E_None;
}

rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool, void *Buffer, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    sal_uInt8 *pBuffer = static_cast< sal_uInt8* >(Buffer);

    if (!pBuffer)
        return rtl_Random_E_Argument;

    if (!osl_get_system_random_data(static_cast<char*>(Buffer), Bytes))
    {
        ::std::fprintf(stderr, "rtl_random_getBytes(): cannot read random device, aborting.\n");
        ::std::abort();
    }
    return rtl_Random_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
