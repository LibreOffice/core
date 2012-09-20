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
#ifndef _UNO_ENVIRONMENT_H_
#define _UNO_ENVIRONMENT_H_

#include <cppu/cppudllapi.h>
#include <rtl/ustring.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _uno_ExtEnvironment;
struct _typelib_InterfaceTypeDescription;

#if defined( SAL_W32)
#pragma pack(push, 8)
#endif

/** The binary specification of an UNO environment.
*/
typedef struct _uno_Environment
{
    /** reserved for future use (0 if not used)
    */
    void *              pReserved;

    /** type name of environment
    */
    rtl_uString *       pTypeName;

    /** free context pointer to be used for specific classes of environments (e.g., a jvm pointer)
    */
    void *              pContext;

    /** pointer to extended environment (interface registration functionality), if supported
    */
    struct _uno_ExtEnvironment * pExtEnv;

    /** Acquires this environment.

        @param pEnv this environment
    */
    void (SAL_CALL * acquire)( struct _uno_Environment * pEnv );

    /** Releases this environment; last release of environment will revoke the environment from
        runtime.

        @param pEnv this environment
    */
    void (SAL_CALL * release)( struct _uno_Environment * pEnv );

    /** Acquires this environment weakly.  You can only harden a weakly held environment if it
        is still acquired hard (acquire()).

        @param pEnv this environment
    */
    void (SAL_CALL * acquireWeak)( struct _uno_Environment * pEnv );

    /** Releases this environment weakly in correspondence to acquireWeak().

        @param pEnv this environment
    */
    void (SAL_CALL * releaseWeak)( struct _uno_Environment * pEnv );

    /** Makes hard reference out of weak referenced environment. You can only harden a weakly
        held environment if it is still acquired hard (acquire()).

        @param ppHardEnv inout hard referenced environment (has to be released via release())
        @param pEnv environment (may be weak referenced)
    */
    void (SAL_CALL * harden)(
        struct _uno_Environment ** ppHardEnv,
        struct _uno_Environment * pEnv );

    /** Call this function to EXPLICITLY dispose this environment (e.g., release all
        interfaces). You may want to call this function before shutting down due to a runtime error.

        @param pEnv this environment
    */
    void (SAL_CALL * dispose)( struct _uno_Environment * pEnv );

    /* ===== the following part will be late initialized by a matching bridge ===== *
     * ===== and is NOT for public use.                                       ===== */

    /** CALLBACK function pointer: Disposing callback function pointer that can be set to get
                                   signalled before the environment is destroyed.

        @param pEnv environment that is being disposed
    */
    void (SAL_CALL * environmentDisposing)( struct _uno_Environment * pEnv );
} uno_Environment;

/** Generic function pointer declaration to free a proxy object if it is not needed by the
    environment anymore.
    Any proxy object must register itself on first acquire() call and revoke itself on last
    release() call. This can happen several times because the environment caches proxy objects
    until the environment explicitly frees the proxy object calling this function.

    @param pEnv environment
    @param pProxy proxy pointer
*/
typedef void (SAL_CALL * uno_freeProxyFunc)( struct _uno_ExtEnvironment * pEnv, void * pProxy );

/** Generic function pointer declaration to allocate memory. Used with getRegisteredInterfaces().

    @param nBytes amount of memory in bytes
    @return pointer to allocated memory
*/
typedef void * (SAL_CALL * uno_memAlloc)( sal_Size nBytes );

/** The binary specification of an UNO environment supporting interface registration.
*/
typedef struct _uno_ExtEnvironment
{
    /** inherits all members of an uno_Environment
    */
    uno_Environment aBase;

    /** Registers an interface of this environment.

        @param pEnv         this environment
        @param ppInterface  inout parameter of interface to be registered
        @param pOId         object id of interface
        @param pTypeDescr   type description of interface
    */
    void (SAL_CALL * registerInterface)(
        struct _uno_ExtEnvironment * pEnv,
        void ** ppInterface,
        rtl_uString * pOId,
        struct _typelib_InterfaceTypeDescription * pTypeDescr );

    /** Registers a proxy interface of this environment that can be reanimated and is freed
        explicitly by this environment.

        @param pEnv         this environment
        @param ppInterface  inout parameter of interface to be registered
        @param freeProxy    function to free proxy object
        @param pOId         object id of interface
        @param pTypeDescr   type description of interface
    */
    void (SAL_CALL * registerProxyInterface)(
        struct _uno_ExtEnvironment * pEnv,
        void ** ppProxy,
        uno_freeProxyFunc freeProxy,
        rtl_uString * pOId,
        struct _typelib_InterfaceTypeDescription * pTypeDescr );

    /** Revokes an interface from this environment. You have to revoke any interface that has
        been registered via this method.

        @param pEnv         this environment
        @param pInterface   interface to be revoked
    */
    void (SAL_CALL * revokeInterface)(
        struct _uno_ExtEnvironment * pEnv,
        void * pInterface );

    /** Provides the object id of a given interface.

        @param ppOut        inout oid
        @param pInterface   interface of object
    */
    void (SAL_CALL * getObjectIdentifier)(
        struct _uno_ExtEnvironment * pEnv,
        rtl_uString ** ppOId,
        void * pInterface );

    /** Retrieves an interface identified by its object id and type from this environment.
        Interfaces are retrieved in the same order as they are registered.

        @param pEnv         this environment
        @param ppInterface  inout parameter for the registered interface; (0) if none was found
        @param pOId         object id of interface to be retrieved
        @param pTypeDescr   type description of interface to be retrieved
    */
    void (SAL_CALL * getRegisteredInterface)(
        struct _uno_ExtEnvironment * pEnv,
        void ** ppInterface,
        rtl_uString * pOId,
        struct _typelib_InterfaceTypeDescription * pTypeDescr );

    /** Returns all currently registered interfaces of this environment. The memory block
        allocated might be slightly larger than (*pnLen * sizeof(void *)).

        @param pEnv         this environment
        @param pppInterfaces out param; pointer to array of interface pointers
        @param pnLen        out param; length of array
        @param memAlloc     function for allocating memory that is passed back
    */
    void (SAL_CALL * getRegisteredInterfaces)(
        struct _uno_ExtEnvironment * pEnv,
        void *** pppInterfaces,
        sal_Int32 * pnLen,
        uno_memAlloc memAlloc );

    /* ===== the following part will be late initialized by a matching bridge ===== */

    /** Computes an object id of the given interface; is called by the environment implementation.

        @param pEnv         corresponding environment
        @param ppOId        out param: computed id
        @param pInterface   an interface
    */
    void (SAL_CALL * computeObjectIdentifier)(
        struct _uno_ExtEnvironment * pEnv,
        rtl_uString ** ppOId, void * pInterface );

    /** Function to acquire an interface.

        @param pEnv         corresponding environment
        @param pInterface   an interface
    */
    void (SAL_CALL * acquireInterface)(
        struct _uno_ExtEnvironment * pEnv,
        void * pInterface );

    /** Function to release an interface.

        @param pEnv         corresponding environment
        @param pInterface   an interface
    */
    void (SAL_CALL * releaseInterface)(
        struct _uno_ExtEnvironment * pEnv,
        void * pInterface );

} uno_ExtEnvironment;

#if defined( SAL_W32)
#pragma pack(pop)
#endif

/** Function exported by some bridge library providing acquireInterface(), releaseInterface();
    may set a disposing callback.

    @param pEnv environment to be initialized
*/
typedef void (SAL_CALL * uno_initEnvironmentFunc)( uno_Environment * pEnv );
#define UNO_INIT_ENVIRONMENT "uno_initEnvironment"

#ifdef DISABLE_DYNLOADING
/* We link statically and have just the C++ environment */
void SAL_CALL CPPU_ENV_uno_initEnvironment( uno_Environment * Env )
     SAL_THROW_EXTERN_C();
#ifdef SOLAR_JAVA
/* We also have the Java environment */
void SAL_CALL java_uno_initEnvironment( uno_Environment * Env )
     SAL_THROW_EXTERN_C();
#endif
#endif

/** Gets a specific environment. If the specified environment does not exist, then a default one
    is created and registered. The environment revokes itself on last release() call.

    @param ppEnv        inout parameter of environment; given environment will be released
    @param pEnvDcp      descriptor of environment
    @param pContext     some context pointer (e.g., to distinguish java vm; set 0 if not needed)
*/
CPPU_DLLPUBLIC void SAL_CALL uno_getEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvDcp, void * pContext )
    SAL_THROW_EXTERN_C();

/** Gets all specified environments. Caller has to release returned environments and free allocated
    memory.

    @param pppEnvs      out param; pointer to array of environments
    @param pnLen        out param; length of array
    @param memAlloc     function for allocating memory that is passed back
    @param pEnvDcp      descriptor of environments; 0 defaults to all
*/
CPPU_DLLPUBLIC void SAL_CALL uno_getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    rtl_uString * pEnvDcp )
    SAL_THROW_EXTERN_C();

/** Creates an environment. The new environment is anonymous (NOT publicly registered/ accessible).

    @param ppEnv        out parameter of environment; given environment will be released
    @param pEnvDcp      descriptor of environment
    @param pContext     context pointer (e.g., to distinguish java vm); set 0 if not needed
*/
CPPU_DLLPUBLIC void SAL_CALL uno_createEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvDcp, void * pContext )
    SAL_THROW_EXTERN_C();

/** Dumps out environment information, i.e. registered interfaces.

    @param stream       output stream (FILE *)
    @param pEnv         environment to be dumped
    @param pFilter      if not null, filters output
*/
CPPU_DLLPUBLIC void SAL_CALL uno_dumpEnvironment(
    void * stream, uno_Environment * pEnv, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C();
/** Dumps out environment information, i.e. registered interfaces.

    @param stream       output stream (FILE *)
    @param pEnvDcp      descritpro of environment to be dumped
    @param pFilter      if not null, filters output
*/
CPPU_DLLPUBLIC void SAL_CALL uno_dumpEnvironmentByName(
    void * stream, rtl_uString * pEnvDcp, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C();



/** Returns the current Environment.
    In case no Environment has explicitly been entered, a purpose free
    default environment gets returned (e.g. the "uno" or "gcc3" Environment).

    @param ppEnv      inout parameter; a given environment will be released
    @param pTypeName  the optional type of the environment, falls back to "uno"
    @since UDK 3.2.7
*/
CPPU_DLLPUBLIC void SAL_CALL uno_getCurrentEnvironment(uno_Environment ** ppEnv, rtl_uString * pTypeName)
    SAL_THROW_EXTERN_C();

/** Typedef for variable argument function.
 */
typedef void SAL_CALL uno_EnvCallee(va_list * pParam);

/** Invoke the passed function in the given environment.

    @param pEnv     the target environment
    @param pCallee  the function to call
    @param pParam   the parameter pointer passed to the function
    @since UDK 3.2.7
 */
CPPU_DLLPUBLIC void SAL_CALL uno_Environment_invoke_v(uno_Environment * pEnv, uno_EnvCallee * pCallee, va_list * pParam)
    SAL_THROW_EXTERN_C();

/** Invoke the passed function in the given environment.

    @param pEnv     the target environment
    @param pCallee  the function to call
    @param ...      the parameters passed to the function
    @since UDK 3.2.7
*/
CPPU_DLLPUBLIC void SAL_CALL uno_Environment_invoke (uno_Environment * pEnv, uno_EnvCallee * pCallee, ...)
    SAL_THROW_EXTERN_C();

/** Enter an environment explicitly.

    @param pEnv    the environment to enter; NULL leaves all environments
    @since UDK 3.2.7
*/
CPPU_DLLPUBLIC void SAL_CALL uno_Environment_enter(uno_Environment * pEnv)
    SAL_THROW_EXTERN_C();

/** Check if a particular environment is currently valid, so
    that objects of that environment might be called.

    @param pEnv                    the environment
    @param pReason                 the reason, if it is not valid
    @return                        1 == valid, 0 == invalid
    @since UDK 3.2.7
*/
CPPU_DLLPUBLIC int SAL_CALL uno_Environment_isValid(uno_Environment * pEnv, rtl_uString ** pReason)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
