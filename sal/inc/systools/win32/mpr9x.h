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



#pragma once

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the winuser.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

#ifdef WNetGetConnectionW
#undef WNetGetConnectionW
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define MPR9X_API extern

//------------------------------------------------------------------------
// declare function pointers to the appropriate user32 functions
//------------------------------------------------------------------------

MPR9X_API DWORD (WINAPI *lpfnWNetGetConnectionW)(
    LPCWSTR lpLocalName,    // pointer to local name
    LPWSTR  lpRemoteName,   // pointer to buffer for remote name
    LPDWORD lpnLength       // pointer to buffer size, in characters
);

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define WNetGetConnectionW          lpfnWNetGetConnectionW

#ifdef __cplusplus
}
#endif
