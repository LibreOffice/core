/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

void acquireKey(RegKeyHandle);
void releaseKey(RegKeyHandle);
bool isKeyReadOnly(RegKeyHandle);
RegError getKeyName(RegKeyHandle, rtl_uString**);
RegError createKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError openKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError openSubKeys(
    RegKeyHandle, rtl_uString*, RegKeyHandle**, sal_uInt32*);
RegError closeSubKeys(RegKeyHandle*, sal_uInt32);
RegError deleteKey(RegKeyHandle, rtl_uString*);
RegError closeKey(RegKeyHandle);
RegError setValue(
    RegKeyHandle, rtl_uString*, RegValueType, RegValue, sal_uInt32);
RegError setLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32 const *, sal_uInt32);
RegError setStringListValue(
    RegKeyHandle, rtl_uString*, char**, sal_uInt32);
RegError setUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
RegError getValueInfo(
    RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
RegError getValue(RegKeyHandle, rtl_uString*, RegValue);
RegError getLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
RegError getStringListValue(
    RegKeyHandle, rtl_uString*, char***, sal_uInt32*);
RegError getUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
RegError freeValueList(RegValueType, RegValue, sal_uInt32);
RegError getResolvedKeyName(
    RegKeyHandle, rtl_uString*, bool, rtl_uString**);
RegError getKeyNames(
    RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
RegError freeKeyNames(rtl_uString**, sal_uInt32);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
