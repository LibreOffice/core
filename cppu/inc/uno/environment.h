/*************************************************************************
 *
 *  $RCSfile: environment.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2001-08-21 09:17:07 $
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
#ifndef _UNO_ENVIRONMENT_H_
#define _UNO_ENVIRONMENT_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct _uno_ExtEnvironment;
struct _typelib_InterfaceTypeDescription;

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
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
typedef void * (SAL_CALL * uno_memAlloc)( sal_uInt32 nBytes );

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

    /* ===== the following part will be late initialized by a matching bridge ===== *

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

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

/** Function exported by some bridge library providing acquireInterface(), releaseInterface();
    may set a disposing callback.

    @param pEnv environment to be initialized
*/
typedef void (SAL_CALL * uno_initEnvironmentFunc)( uno_Environment * pEnv );
#define UNO_INIT_ENVIRONMENT "uno_initEnvironment"

/** Gets a specific environment. If the specified environment does not exist, then a default one
    is created and registered. The environment revokes itself on last release() call.

    @param ppEnv        inout parameter of environment; given environment will be released
    @param pEnvTypeName type name of environment
    @param pContext     some context pointer (e.g., to distinguish java vm; set 0 if not needed)
*/
void SAL_CALL uno_getEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvTypeName, void * pContext )
    SAL_THROW_EXTERN_C();

/** Gets all specified environments. Caller has to release returned environments and free allocated
    memory.

    @param pppEnvs      out param; pointer to array of environments
    @param pnLen        out param; length of array
    @param memAlloc     function for allocating memory that is passed back
    @param pEnvTypeName type name of environments; 0 defaults to all
*/
void SAL_CALL uno_getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    rtl_uString * pEnvTypeName )
    SAL_THROW_EXTERN_C();

/** Creates an environment. The new environment is anonymous (NOT publicly registered/ accessible).

    @param ppEnv        out parameter of environment; given environment will be released
    @param pEnvTypeName name of environment
    @param pContext     context pointer (e.g., to distinguish java vm); set 0 if not needed
*/
void SAL_CALL uno_createEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvTypeName, void * pContext )
    SAL_THROW_EXTERN_C();

/** Dumps out environment information, i.e. registered interfaces.

    @param stream       output stream (FILE *)
    @param pEnv         environment to be dumped
    @param pFilter      if not null, filters output
*/
void SAL_CALL uno_dumpEnvironment(
    void * stream, uno_Environment * pEnv, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C();
/** Dumps out environment information, i.e. registered interfaces.

    @param stream       output stream (FILE *)
    @param pEnvTypeName type name of environment to be dumped
    @param pFilter      if not null, filters output
*/
void SAL_CALL uno_dumpEnvironmentByName(
    void * stream, rtl_uString * pEnvTypeName, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
