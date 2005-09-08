/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: file_url.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:56:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
