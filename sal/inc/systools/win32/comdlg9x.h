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

#ifndef _COMMDLG_H_
#include <commdlg.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the shlobj.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

#ifdef GetOpenFileNameW
#undef GetOpenFileNameW
#endif

#ifdef GetSaveFileNameW
#undef GetSaveFileNameW
#endif

//------------------------------------------------------------------------
// set the compiler directives for the function pointer we declare below
// if we build sal or sal will be used as static library we define extern
// else sal exports the function pointers from a dll and we use __declspec
//------------------------------------------------------------------------

#define COMDLG9X_API extern

//------------------------------------------------------------------------
// declare function pointers to the appropriate comdlg functions
//------------------------------------------------------------------------

COMDLG9X_API BOOL ( WINAPI * lpfnGetOpenFileNameW ) ( LPOPENFILENAMEW lpofn );
COMDLG9X_API BOOL ( WINAPI * lpfnGetSaveFileNameW ) ( LPOPENFILENAMEW lpofn );

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define GetOpenFileNameW    lpfnGetOpenFileNameW
#define GetSaveFileNameW    lpfnGetSaveFileNameW

#ifdef __cplusplus
}
#endif
