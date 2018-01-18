/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_UNO_MAPPING_H
#define INCLUDED_UNO_MAPPING_H

#include "cppu/cppudllapi.h"
#include "rtl/ustring.h"


#ifdef __cplusplus
extern "C"
{
#endif

struct _typelib_InterfaceTypeDescription;
struct _uno_Mapping;
struct _uno_Environment;

/**
   Function pointer declaration to acquire a UNO mapping.
*/
typedef void (SAL_CALL * uno_AcquireMappingFunc)(struct _uno_Mapping *);

/**
   Function pointer declaration to release a UNO mapping.
*/
typedef void (SAL_CALL * uno_ReleaseMappingFunc)(struct _uno_Mapping *);

/** Function pointer declaration to map an interface from one environment to another.

    @param pMapping         mapping
    @param ppOut            [inout] destination interface; existing interfaces are released
    @param pInterface       source interface
    @param pInterfaceTypeDescr type description of the interface
*/
typedef void (SAL_CALL * uno_MapInterfaceFunc)(
    struct _uno_Mapping * pMapping,
    void ** ppOut, void * pInterface,
    struct _typelib_InterfaceTypeDescription * pInterfaceTypeDescr );


#if defined( _WIN32)
#pragma pack(push, 8)
#endif

/** This is the binary specification of a mapping.
*/
typedef struct SAL_DLLPUBLIC_RTTI _uno_Mapping
{
    /** Acquires mapping
    */
    uno_AcquireMappingFunc acquire;

    /** Releases mapping. The last release may unload bridges.
    */
    uno_ReleaseMappingFunc release;

    /** mapping function
    */
    uno_MapInterfaceFunc mapInterface;
} uno_Mapping;

#if defined( _WIN32)
#pragma pack(pop)
#endif

/** Gets an interface mapping from one environment to another.

    @param ppMapping    [inout] mapping; existing mapping will be released
    @param pFrom        source environment
    @param pTo          destination environment
                        (interfaces resulting in mapInterface() call can be used
                        in this language environment)
    @param pAddPurpose  additional purpose of mapping (e.g., protocolling); defaults to 0 (none)
*/
CPPU_DLLPUBLIC void SAL_CALL uno_getMapping(
    struct _uno_Mapping ** ppMapping,
    struct _uno_Environment * pFrom,
    struct _uno_Environment * pTo,
    rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C();

/** Callback function pointer declaration to get a mapping.

    @param ppMapping    inout mapping
    @param pFrom        source environment
    @param pTo          destination environment
    @param pAddPurpose  additional purpose
*/
typedef void (SAL_CALL * uno_getMappingFunc)(
    struct _uno_Mapping ** ppMapping,
    struct _uno_Environment * pFrom,
    struct _uno_Environment * pTo,
    rtl_uString * pAddPurpose );

/** Registers a callback being called each time a mapping is demanded.

    @param pCallback    callback function
*/
CPPU_DLLPUBLIC void SAL_CALL uno_registerMappingCallback(
    uno_getMappingFunc pCallback )
    SAL_THROW_EXTERN_C();

/** Revokes a mapping callback registration.

    @param pCallback    callback function
*/
CPPU_DLLPUBLIC void SAL_CALL uno_revokeMappingCallback(
    uno_getMappingFunc pCallback )
    SAL_THROW_EXTERN_C();

/** Function pointer declaration to free a mapping.

    @param pMapping     mapping to be freed
*/
typedef void (SAL_CALL * uno_freeMappingFunc)( struct _uno_Mapping * pMapping );

/** Registers a mapping. A mapping registers itself on first acquire and revokes itself on last
    release. The given freeMapping function is called by the runtime to cleanup any resources.

    @param ppMapping    inout mapping to be registered
    @param freeMapping  called by runtime to delete mapping
    @param pFrom        source environment
    @param pTo          destination environment
    @param pAddPurpose  additional purpose string; defaults to 0
*/
CPPU_DLLPUBLIC void SAL_CALL uno_registerMapping(
    struct _uno_Mapping ** ppMapping, uno_freeMappingFunc freeMapping,
    struct _uno_Environment * pFrom, struct _uno_Environment * pTo, rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C();

/** Revokes a mapping. A mapping registers itself on first acquire and revokes itself on last
    release.

    @param pMapping     mapping to be revoked
*/
CPPU_DLLPUBLIC void SAL_CALL uno_revokeMapping(
    struct _uno_Mapping * pMapping )
    SAL_THROW_EXTERN_C();

/** Gets an interface mapping from one language environment to another by corresponding environment
    type names.

    @param ppMapping    [inout] mapping; existing mapping will be released
    @param pFrom        source environment type name
    @param pTo          destination environment type name
                        (interfaces resulting in mapInterface() call can be used
                        in this language environment)
    @param pAddPurpose  additional purpose of mapping (e.g., protocolling); defaults to 0 (none)
*/
CPPU_DLLPUBLIC void SAL_CALL uno_getMappingByName(
    struct _uno_Mapping ** ppMapping,
    rtl_uString * pFrom,
    rtl_uString * pTo,
    rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C();

/* symbol exported by each language binding library */
#define UNO_EXT_GETMAPPING "uno_ext_getMapping"

/** Function pointer declaration to get a mapping from a loaded bridge. Bridges export a function
    called uno_ext_getMapping() of this signature.

    @param[in,out] ppMapping  mapping; existing mapping will be released
    @param[in] pFrom              source environment
    @param[in] pTo                destination environment
*/
typedef void (SAL_CALL * uno_ext_getMappingFunc)(
    struct _uno_Mapping ** ppMapping,
    struct _uno_Environment * pFrom,
    struct _uno_Environment * pTo );

#ifdef DISABLE_DYNLOADING
/* Static linking, this is the uno_ext_getMapping function in the C++/UNO bridge */
void SAL_CALL CPPU_ENV_uno_ext_getMapping(
    struct _uno_Mapping ** ppMapping,
    struct _uno_Environment * pFrom,
    struct _uno_Environment * pTo )
    SAL_THROW_EXTERN_C();

/* This is the uno_ext_getMapping function in the Java/UNO bridge */
void SAL_CALL java_uno_ext_getMapping(
    struct _uno_Mapping ** ppMapping,
    struct _uno_Environment * pFrom,
    struct _uno_Environment * pTo )
    SAL_THROW_EXTERN_C();

#endif

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
