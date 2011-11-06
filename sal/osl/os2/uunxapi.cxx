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



 #ifndef _LIMITS_H
 #include <limits.h>
 #endif

 #ifndef _RTL_USTRING_HXX_
 #include <rtl/ustring.hxx>
 #endif

 #ifndef _OSL_THREAD_H_
 #include <osl/thread.h>
 #endif

 #ifndef _OSL_UUNXAPI_HXX_
 #include "uunxapi.hxx"
 #endif

 //###########################
 //access_u
 int access_u(const rtl_uString* pustrPath, int mode)
 {
    return access(OUStringToOString(pustrPath).getStr(), mode);
 }

 //#########################
 //realpath_u
 sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
 {
     rtl::OString fn = rtl::OUStringToOString(
        rtl::OUString(const_cast<rtl_uString*>(pustrFileName)),
        osl_getThreadTextEncoding());

    char  rp[PATH_MAX];
    bool  bRet = realpath(fn.getStr(), rp);

    if (bRet)
    {
        rtl::OUString resolved = rtl::OStringToOUString(
            rtl::OString(static_cast<sal_Char*>(rp)),
            osl_getThreadTextEncoding());

        rtl_uString_assign(ppustrResolvedName, resolved.pData);
    }
    return bRet;
 }

 //#########################
 //lstat_u
  int lstat_u(const rtl_uString* pustrPath, struct stat* buf)
 {
    return lstat(OUStringToOString(pustrPath).getStr(), buf);
 }

 //#########################
 // @see mkdir
 int mkdir_u(const rtl_uString* path, mode_t mode)
 {
    return mkdir(OUStringToOString(path).getStr(), mode);
 }

