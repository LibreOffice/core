/*************************************************************************
 *
 *  $RCSfile: profile.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-16 13:10:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_PROFILE_H_
#define _OSL_PROFILE_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_H
#     include <rtl/ustring.h>
#endif

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

/** Open or create a configuration profile.
    @return 0 if the profile could not be created, otherwise a handle to the profile.
*/
oslProfile SAL_CALL osl_openProfile(rtl_uString *strProfileName, oslProfileOption Options);

/** Close the opened profile an flush all data to the disk.
    @param Profile handle to a opened profile.
*/
sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile);


sal_Bool SAL_CALL osl_flushProfile(oslProfile Profile);


sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_Char* pszString, sal_uInt32 MaxLen,
                              const sal_Char* pszDefault);
sal_Bool SAL_CALL osl_readProfileBool(oslProfile Profile,
                            const sal_Char* pszSection, const sal_Char* pszEntry,
                            sal_Bool Default);
sal_uInt32 SAL_CALL osl_readProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Default);

sal_Bool SAL_CALL osl_writeProfileString(oslProfile Profile,
                               const sal_Char* pszSection, const sal_Char* pszEntry,
                               const sal_Char* pszString);
sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value);
sal_Bool SAL_CALL osl_writeProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Value);

/** Acquire the mutex, block if already acquired by another thread.
    @param Profile handle to a opened profile.
    @return False if section or entry could not be found.
*/
sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                               const sal_Char *pszSection, const sal_Char *pszEntry);

/** Get all entries belonging to the specified section.
    @param Profile handle to a opened profile.
    @return Pointer to a array of pointers.
*/
sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile, const sal_Char *pszSection,
                                    sal_Char* pszBuffer, sal_uInt32 MaxLen);

/** Get all section entries
    @param Profile handle to a opened profile.
    @return Pointer to a array of pointers.
*/
sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen);

sal_Bool SAL_CALL osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName);


#ifdef __cplusplus
}
#endif

#endif  /* _OSL_PROFILE_H_ */

