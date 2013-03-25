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

#ifndef _RTL_STRING_UTILS_CONST_H_
#define _RTL_STRING_UTILS_CONST_H_

#include <sal/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

static const sal_Int32 kErrCompareAStringToUString     = -2;
static const sal_Int32 kErrCompareNAStringToUString    = -3;
static const sal_Int32 kErrCompareAStringToRTLUString  = -4;
static const sal_Int32 kErrCompareNAStringToRTLUString = -5;
static const sal_Int32 kErrAStringToByteStringCompare  = -6;
static const sal_Int32 kErrAStringToByteStringNCompare = -7;
static const sal_Int32 kErrCompareAStringToString      = -8;
static const sal_Int32 kErrCompareNAStringToString     = -9;

#ifdef __cplusplus
}
#endif

#endif /* _RTL_STRING_UTILS_CONST_H_ */



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
