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

#ifndef INCLUDED_SAL_OSL_W32_PATH_HELPER_H
#define INCLUDED_SAL_OSL_W32_PATH_HELPER_H

#include <sal/types.h>
#include <rtl/ustring.h>
#include <osl/file.h>
#include <osl/diagnose.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************
 osl_systemPathEnsureSeparator
 Adds a trailing path separator to the given system path if not
 already there and if the path is not the root path or a logical
 drive alone
 ******************************************************************/

void osl_systemPathEnsureSeparator(/*inout*/ rtl_uString** ppustrPath);

/*******************************************************************
 osl_systemPathRemoveSeparator
 Removes the last separator from the given system path if any and
 if the path is not the root path '\'
 ******************************************************************/

void SAL_CALL osl_systemPathRemoveSeparator(/*inout*/ rtl_uString** ppustrPath);

/*******************************************************************
 osl_is_logical_drive_pattern
 Returns whether a given path is only a logical drive pattern or not.
 A logical drive pattern is something like "a:\", "c:\".
 No logical drive pattern is something like "c:\test"
 ******************************************************************/

sal_Int32 osl_systemPathIsLogicalDrivePattern(/*in*/ const rtl_uString* pustrPath);

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
