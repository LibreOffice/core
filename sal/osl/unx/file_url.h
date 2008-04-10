/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: file_url.h,v $
 * $Revision: 1.3 $
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

 /***************************************************
  * Internal header file, declares all functions
  * that are not part of the offical API and are
  * not defined in the osl header files
  **************************************************/

 #ifndef _FILE_URL_H_
 #define _FILE_URL_H_

 #ifndef _FILE_H_
 #include <osl/file.h>
 #endif

 #ifdef __cplusplus
 extern "C"
 {
 #endif

 /**************************************************
  * _osl_getSystemPathFromFileURL
  *************************************************/

 #define FURL_ALLOW_RELATIVE sal_True
 #define FURL_DENY_RELATIVE  sal_False

 oslFileError osl_getSystemPathFromFileURL_Ex(rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath, sal_Bool bAllowRelative);

 /**************************************************
  * FileURLToPath
  *************************************************/

 oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL);


 #ifdef __cplusplus
 }
 #endif


 #endif /* #define _FILE_URL_H_ */
