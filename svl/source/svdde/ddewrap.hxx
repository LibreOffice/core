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

#ifndef _SVTOOLS_SVDDE_DDEWRAP_HXX_
#define _SVTOOLS_SVDDE_DDEWRAP_HXX_

#define DECLARE_WAPI_FUNC(func) \
    extern func##_PROC lpfn##func;

typedef HSZ (WINAPI *DdeCreateStringHandleW_PROC)( DWORD idInst, LPCWSTR pszString, int iCodePage );
typedef DWORD (WINAPI *DdeQueryStringW_PROC)( DWORD idInst, HSZ hsz, LPWSTR pszString, DWORD cchMax, int iCodePage );
typedef UINT (WINAPI *DdeInitializeW_PROC)( LPDWORD pidInst, PFNCALLBACK pfnCallback, DWORD afCmd, DWORD ulRes );


DECLARE_WAPI_FUNC( DdeCreateStringHandleW );
DECLARE_WAPI_FUNC( DdeQueryStringW );
DECLARE_WAPI_FUNC( DdeInitializeW );


#ifndef _SVTOOLS_SVDDE_DDEWRAP_CXX_
#define DdeCreateStringHandleW  lpfnDdeCreateStringHandleW
#define DdeQueryStringW         lpfnDdeQueryStringW
#define DdeInitializeW          lpfnDdeInitializeW
#endif


#endif
