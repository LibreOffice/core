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

 /* @see lstat */
 int lstat_u(const rtl_uString* pustrPath, struct stat* buf);

 /* @see mkdir */
 int mkdir_u(const rtl_uString* path, mode_t mode);

 #ifdef __cplusplus
 }
 #endif


 #endif /* _OSL_UUNXAPI_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
