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

#include <sal/config.h>
#include <registry/regtype.h>
#include <rtl/ustring.h>
#include <sal/types.h>

extern "C" {

void REGISTRY_CALLTYPE acquireKey(RegKeyHandle);
void REGISTRY_CALLTYPE releaseKey(RegKeyHandle);
sal_Bool REGISTRY_CALLTYPE isKeyReadOnly(RegKeyHandle);
RegError REGISTRY_CALLTYPE getKeyName(RegKeyHandle, rtl_uString**);
RegError REGISTRY_CALLTYPE createKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openSubKeys(
    RegKeyHandle, rtl_uString*, RegKeyHandle**, sal_uInt32*);
RegError REGISTRY_CALLTYPE closeSubKeys(RegKeyHandle*, sal_uInt32);
RegError REGISTRY_CALLTYPE deleteKey(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE closeKey(RegKeyHandle);
RegError REGISTRY_CALLTYPE setValue(
    RegKeyHandle, rtl_uString*, RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE setLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32 const *, sal_uInt32);
RegError REGISTRY_CALLTYPE setStringListValue(
    RegKeyHandle, rtl_uString*, char**, sal_uInt32);
RegError REGISTRY_CALLTYPE setUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
RegError REGISTRY_CALLTYPE getValueInfo(
    RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
RegError REGISTRY_CALLTYPE getValue(RegKeyHandle, rtl_uString*, RegValue);
RegError REGISTRY_CALLTYPE getLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
RegError REGISTRY_CALLTYPE getStringListValue(
    RegKeyHandle, rtl_uString*, char***, sal_uInt32*);
RegError REGISTRY_CALLTYPE getUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeValueList(RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE getResolvedKeyName(
    RegKeyHandle, rtl_uString*, sal_Bool, rtl_uString**);
RegError REGISTRY_CALLTYPE getKeyNames(
    RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeKeyNames(rtl_uString**, sal_uInt32);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
