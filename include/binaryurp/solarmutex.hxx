/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <sal/macros.h>
#include <sal/types.h>
#include <binaryurp/binaryurpdllapi.h>

namespace binaryurp
{
BINARYURP_DLLPUBLIC void SetMutexFunctions(sal_uInt32 (*ReleaseSolarMutexFuncPtr)(),
                                           void (*AcquireSolarMutexFuncPtr)(sal_uInt32));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
