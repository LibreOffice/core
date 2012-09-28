/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _OSL_PROFILE_H_
#define _OSL_PROFILE_H_

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

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
SAL_DLLPUBLIC oslProfile SAL_CALL osl_openProfile(
        rtl_uString *strProfileName, oslProfileOption Options);

/** Deprecated API.
    Close the opened profile an flush all data to the disk.
    @param Profile handle to a opened profile.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_closeProfile(
        oslProfile Profile);


/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_flushProfile(
        oslProfile Profile);
/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_readProfileString(
                              oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_Char* pszString, sal_uInt32 MaxLen,
                              const sal_Char* pszDefault);
/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_readProfileBool(
                            oslProfile Profile,
                            const sal_Char* pszSection, const sal_Char* pszEntry,
                            sal_Bool Default);
/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_readProfileIdent(
                              oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Default);

/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_writeProfileString(
                               oslProfile Profile,
                               const sal_Char* pszSection, const sal_Char* pszEntry,
                               const sal_Char* pszString);
/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_writeProfileBool(
                             oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value);
/** Deprecated API.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_writeProfileIdent(
                              oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Value);

/** Deprecated API.
    Acquire the mutex, block if already acquired by another thread.
    @return False if section or entry could not be found.
    @deprecated
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_removeProfileEntry(
                           oslProfile Profile,
                           const sal_Char *pszSection, const sal_Char *pszEntry);

/** Deprecated API.
    Get all entries belonging to the specified section.
    @return Pointer to a array of pointers.
    @deprecated
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_getProfileSectionEntries(
                            oslProfile Profile, const sal_Char *pszSection,
                            sal_Char* pszBuffer, sal_uInt32 MaxLen);

/** Deprecated API.
    Get all section entries
    @return Pointer to a array of pointers.
    @deprecated
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_getProfileSections(
        oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_PROFILE_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
