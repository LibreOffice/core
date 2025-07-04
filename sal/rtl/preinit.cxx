/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <stdlib.h>
#include <string.h>

#include <rtl/alloc.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

#include <setallowedpaths.hxx>

#include "strimp.hxx"

void SAL_CALL rtl_alloc_preInit(sal_uInt8 mode) SAL_THROW_EXTERN_C()
{
    switch (mode)
    {
        case 0:
            alloc_preInit(false);
            break;
        case 1:
            alloc_preInit(true);
            break;
#if defined UNX
        case 2:
        {
            const char* pAllowedPaths = getenv("SAL_ALLOWED_PATHS");
            if (pAllowedPaths)
                setAllowedPaths(
                    OUString(pAllowedPaths, strlen(pAllowedPaths), RTL_TEXTENCODING_UTF8));
            break;
        }
#endif
        default:
            SAL_WARN("sal.rtl", "Unknown rtl_alloc_preInit mode " << unsigned(mode));
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
