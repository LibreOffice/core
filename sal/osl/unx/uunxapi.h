/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

