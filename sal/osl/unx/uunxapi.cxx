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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

 #ifndef _OSL_UUNXAPI_H_
 #include "uunxapi.h"
 #endif

 #ifndef __OSL_SYSTEM_H__
 #include "system.h"
 #endif

 #ifndef _LIMITS_H
 #include <limits.h>
 #endif

 #ifndef _RTL_USTRING_HXX_
 #include <rtl/ustring.hxx>
 #endif

 #ifndef _OSL_THREAD_H_
 #include <osl/thread.h>
 #endif

 //###########################
 inline rtl::OString OUStringToOString(const rtl_uString* s)
 {
    return rtl::OUStringToOString(
        rtl::OUString(const_cast<rtl_uString*>(s)),
        osl_getThreadTextEncoding());
 }

 //###########################
#ifdef MACOSX
/*
 * Helper function for resolving Mac native alias files (not the same as unix alias files)
 * and to return the resolved alias as rtl::OString
 */
 inline rtl::OString macxp_resolveAliasAndConvert(const rtl_uString* s)
 {
  rtl::OString p = OUStringToOString(s);
  sal_Char path[PATH_MAX];
  if (p.getLength() < PATH_MAX)
    {
      strcpy(path, p.getStr());
      macxp_resolveAlias(path, PATH_MAX);
      p = rtl::OString(path);
    }
  return p;
 }
#endif /* MACOSX */

 //###########################
 //access_u
 int access_u(const rtl_uString* pustrPath, int mode)
 {
#ifndef MACOSX // not MACOSX
    return access(OUStringToOString(pustrPath).getStr(), mode);
#else
    return access(macxp_resolveAliasAndConvert(pustrPath).getStr(), mode);
#endif
 }

 //#########################
 //realpath_u
 sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
 {
#ifndef MACOSX // not MACOSX
        rtl::OString fn = OUStringToOString(pustrFileName);
#else
    rtl::OString fn = macxp_resolveAliasAndConvert(pustrFileName);
#endif
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
#ifndef MACOSX  // not MACOSX
    return lstat(OUStringToOString(pustrPath).getStr(), buf);
#else
    return lstat(macxp_resolveAliasAndConvert(pustrPath).getStr(), buf);
#endif
 }

 //#########################
 // @see mkdir
 int mkdir_u(const rtl_uString* path, mode_t mode)
 {
    return mkdir(OUStringToOString(path).getStr(), mode);
 }

