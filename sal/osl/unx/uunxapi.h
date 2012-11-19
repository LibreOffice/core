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

 #ifndef _OSL_UUNXAPI_H_
 #define _OSL_UUNXAPI_H_

 #ifndef _UNISTD_H
 #include <unistd.h>
 #endif

 #ifndef _STDLIB_H
 #include <stdlib.h>
 #endif

 #ifndef _TYPES_H
 #include <sys/types.h>
 #endif

 #ifndef _STAT_H
 #include <sys/stat.h>
 #endif

 #ifndef _RTL_USTRING_H_
 #include <rtl/ustring.h>
 #endif


 #ifdef __cplusplus
 extern "C"
 {
 #endif

 /* @see access */
 int access_u(const rtl_uString* pustrPath, int mode);

 /***********************************
  @descr
  The return value differs from the
  realpath function

  @returns sal_True on success else
  sal_False

  @see realpath
  **********************************/
 sal_Bool realpath_u(
     const rtl_uString* pustrFileName,
    rtl_uString** ppustrResolvedName);

 /* @see stat */
 int stat_c(const char *cpPath, struct stat* buf);

 /* @see lstat */
 int lstat_c(const char *cpPath, struct stat* buf);

 /* @see lstat */
 int lstat_u(const rtl_uString* pustrPath, struct stat* buf);

 /* @see mkdir */
 int mkdir_u(const rtl_uString* path, mode_t mode);

 #ifdef __cplusplus
 }
 #endif


 #endif /* _OSL_UUNXAPI_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
