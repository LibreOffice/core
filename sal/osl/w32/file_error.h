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



#ifndef INCLUDED_OSL_FILE_ERROR_H
#define INCLUDED_OSL_FILE_ERROR_H

#include "osl/file.h"
#include "rtl/ustring.h"

#ifdef __cplusplus
extern "C" {
#endif

oslFileError oslTranslateFileError (/*DWORD*/ unsigned long dwError);

#if OSL_DEBUG_LEVEL > 0
void _osl_warnFile (const char * message, rtl_uString * ustrFile);
#define OSL_ENSURE_FILE( cond, msg, file ) ( (cond) ?  (void)0 : _osl_warnFile( msg, file ) )
#else
#define OSL_ENSURE_FILE( cond, msg, file ) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_OSL_FILE_ERROR_H */
