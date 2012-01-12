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



#ifndef _PATH_HELPER_H_
#define _PATH_HELPER_H_

#include <sal/types.h>
#include <rtl/ustring.h>
#include <osl/file.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************
 osl_systemPathEnsureSeparator
 Adds a trailing path separator to the given system path if not
 already there and if the path is not the root path or a logical
 drive alone
 ******************************************************************/

void osl_systemPathEnsureSeparator(/*inout*/ rtl_uString** ppustrPath);

/*******************************************************************
 osl_systemPathRemoveSeparator
 Removes the last separator from the given system path if any and
 if the path is not the root path '\'
 ******************************************************************/

void SAL_CALL osl_systemPathRemoveSeparator(/*inout*/ rtl_uString** ppustrPath);

/*******************************************************************
 osl_is_logical_drive_pattern
 Returns whether a given path is only a logical drive pattern or not.
 A logical drive pattern is something like "a:\", "c:\".
 No logical drive pattern is something like "c:\test"
 ******************************************************************/

sal_Int32 osl_systemPathIsLogicalDrivePattern(/*in*/ const rtl_uString* pustrPath);

#ifdef __cplusplus
}
#endif

#endif
