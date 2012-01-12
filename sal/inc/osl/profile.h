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



#ifndef _OSL_PROFILE_H_
#define _OSL_PROFILE_H_

#include <sal/types.h>
#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef sal_uInt32 oslProfileOption;

#define osl_Profile_DEFAULT     0x0000
#define osl_Profile_SYSTEM      0x0001    /* use system depended functinality */
#define osl_Profile_READLOCK    0x0002    /* lock file for reading            */
#define osl_Profile_WRITELOCK   0x0004    /* lock file for writing            */
#define osl_Profile_FLUSHWRITE  0x0010    /* writing only with flush          */


typedef void* oslProfile;

/** Deprecated API.
    Open or create a configuration profile.
    @return 0 if the profile could not be created, otherwise a handle to the profile.
    @deprecated
*/
oslProfile SAL_CALL osl_openProfile(rtl_uString *strProfileName, oslProfileOption Options);

/** Deprecated API.
    Close the opened profile an flush all data to the disk.
    @param Profile handle to a opened profile.
    @deprecated
*/
sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile);


/** Deprecated API.
    @deprecated
*/
sal_Bool SAL_CALL osl_flushProfile(oslProfile Profile);
/** Deprecated API.
    @deprecated
*/
sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_Char* pszString, sal_uInt32 MaxLen,
                              const sal_Char* pszDefault);
/** Deprecated API.
    @deprecated
*/
sal_Bool SAL_CALL osl_readProfileBool(oslProfile Profile,
                            const sal_Char* pszSection, const sal_Char* pszEntry,
                            sal_Bool Default);
/** Deprecated API.
    @deprecated
*/
sal_uInt32 SAL_CALL osl_readProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Default);

/** Deprecated API.
    @deprecated
*/
sal_Bool SAL_CALL osl_writeProfileString(oslProfile Profile,
                               const sal_Char* pszSection, const sal_Char* pszEntry,
                               const sal_Char* pszString);
/** Deprecated API.
    @deprecated
*/
sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value);
/** Deprecated API.
    @deprecated
*/
sal_Bool SAL_CALL osl_writeProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Value);

/** Deprecated API.
    Acquire the mutex, block if already acquired by another thread.
    @param Profile handle to a opened profile.
    @return False if section or entry could not be found.
    @deprecated
*/
sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                               const sal_Char *pszSection, const sal_Char *pszEntry);

/** Deprecated API.
    Get all entries belonging to the specified section.
    @param Profile handle to a opened profile.
    @return Pointer to a array of pointers.
    @deprecated
*/
sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile, const sal_Char *pszSection,
                                    sal_Char* pszBuffer, sal_uInt32 MaxLen);

/** Deprecated API.
    Get all section entries
    @param Profile handle to a opened profile.
    @return Pointer to a array of pointers.
    @deprecated
*/
sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_PROFILE_H_ */

