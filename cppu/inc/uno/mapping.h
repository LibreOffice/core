/*************************************************************************
 *
 *  $RCSfile: mapping.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:52 $
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
#ifndef _UNO_MAPPING_H_
#define _UNO_MAPPING_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

typedef struct _typelib_InterfaceTypeDescription typelib_InterfaceTypeDescription;
typedef struct _uno_Mapping uno_Mapping;
typedef struct _uno_Environment uno_Environment;

/** Function pointer declaration to map an interface from one environment to another.
    <br>
    @param pMapping         mapping
    @param ppOut            [inout] destination interface; existing interfaces are released
    @param pInterface       source interface
    @param pInterfaceTypeDescr type description of the interface
*/
typedef void (SAL_CALL * uno_MapInterfaceFunc)(
    uno_Mapping * pMapping,
    void ** ppOut, void * pInterface,
    typelib_InterfaceTypeDescription * pInterfaceTypeDescr );


#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
#endif

/** This is the binary specification of a mapping.
    <br>
*/
typedef struct _uno_Mapping
{
    /** Acquires mapping
        <br>
        @param pMapping     mapping
    */
    void (SAL_CALL * acquire)( uno_Mapping * pMapping );
    /** Releases mapping.
        The last release may unload bridges.
        <br>
        @param pMapping     mapping
    */
    void (SAL_CALL * release)( uno_Mapping * pMapping );

    /** mapping function<br>
    */
    uno_MapInterfaceFunc mapInterface;
} uno_Mapping;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** Gets an interface mapping from one environment to another.
    <br>
    @param ppMapping    [inout] mapping; existing mapping will be released
    @param pFrom        source environment
    @param pTo          destination environment
                        (interfaces resulting in mapInterface() call can be used
                        in this language environment)
    @param pAddPurpose  additional purpose of mapping (e.g., protocolling); defaults to 0 (none)
*/
SAL_DLLEXPORT void SAL_CALL uno_getMapping(
    uno_Mapping ** ppMapping,
    uno_Environment * pFrom,
    uno_Environment * pTo,
    rtl_uString * pAddPurpose );

/** Callback function pointer declaration to get a mapping.
    <br>
    @param ppMapping    inout mapping
    @param pFrom        source environment
    @param pTo          destination environment
    @param pAddPurpose  additional purpose
*/
typedef void (SAL_CALL * uno_getMappingFunc)(
    uno_Mapping ** ppMapping,
    uno_Environment * pFrom,
    uno_Environment * pTo,
    rtl_uString * pAddPurpose );

/** Registers a callback being called each time a mapping is demanded.
    <br>
    @param pCallback    callback function
*/
SAL_DLLEXPORT void SAL_CALL uno_registerMappingCallback(
    uno_getMappingFunc pCallback );

/** Revokes a mapping callback registration.
    <br>
    @param pCallback    callback function
*/
SAL_DLLEXPORT void SAL_CALL uno_revokeMappingCallback(
    uno_getMappingFunc pCallback );

/** Function pointer declaration to free a mapping.
    <br>
    @param pMapping     mapping to be freed
*/
typedef void (SAL_CALL * uno_freeMappingFunc)( uno_Mapping * pMapping );

/** Registers a mapping.<br>
    A mapping registers itself on first acquire and revokes itself on last release.
    The given freeMapping function is called by the runtime to cleanup
    any resources.
    <br>
    @param ppMapping    inout mapping to be registered
    @param freeMapping  called by runtime to delete mapping
    @param pFrom        source environment
    @param pTo          destination environment
    @param pAddPurpose  additional purpose string; defaults to 0
*/
SAL_DLLEXPORT void SAL_CALL uno_registerMapping(
    uno_Mapping ** ppMapping, uno_freeMappingFunc freeMapping,
    uno_Environment * pFrom, uno_Environment * pTo, rtl_uString * pAddPurpose );

/** Revokes a mapping.<br>
    A mapping registers itself on first acquire and revokes itself on last release.
    <br>
    @param pMapping     mapping to be revoked
*/
SAL_DLLEXPORT void SAL_CALL uno_revokeMapping(
    uno_Mapping * pMapping );

/** Gets an interface mapping from one language environment to another by
    corresponding environment type names.
    <br>
    @param ppMapping    [inout] mapping; existing mapping will be released
    @param pFrom        source environment type name
    @param pTo          destination environment type name
                        (interfaces resulting in mapInterface() call can be used
                        in this language environment)
    @param pAddPurpose  additional purpose of mapping (e.g., protocolling); defaults to 0 (none)
*/
SAL_DLLEXPORT void SAL_CALL uno_getMappingByName(
    uno_Mapping ** ppMapping,
    rtl_uString * pFrom,
    rtl_uString * pTo,
    rtl_uString * pAddPurpose );

/* symbol exported by each language binding library */
#define UNO_EXT_GETMAPPING "uno_ext_getMapping"

/** Function pointer declaration to get a mapping from a loaded bridge.<br>
    Bridges export a function called <b>uno_ext_getMapping</b> of this signature.
    <br>
    @param ppMapping    [inout] mapping; existing mapping will be released
    @pFrom              source environment
    @pTo                destination environment
*/
typedef void (SAL_CALL * uno_ext_getMappingFunc)(
    uno_Mapping ** ppMapping,
    uno_Environment * pFrom,
    uno_Environment * pTo );

#ifdef __cplusplus
}
#endif

#endif
