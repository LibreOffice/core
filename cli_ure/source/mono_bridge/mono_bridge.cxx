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

#include <string.h>

#include "uno/dispatcher.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include "osl/diagnose.h"
#include "rtl/unload.h"
#include "rtl/ustring.hxx"
#include "glib/gtypes.h"

#include "uno/mapping.hxx"

extern "C" {
#include "mono/metadata/appdomain.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/debug-helpers.h"
#include "mono/metadata/object.h"
#include "mono/metadata/threads.h"
}

#include "mono_bridge.h"

#ifndef MONO_PUBLIC_KEY_TOKEN_LENGTH
#define MONO_PUBLIC_KEY_TOKEN_LENGTH 17
struct _MonoAssemblyName {
    const char *name;
    const char *culture;
    const char *hash_value;
    const guint8* public_key;
    guchar public_key_token [MONO_PUBLIC_KEY_TOKEN_LENGTH];
    guint32 hash_alg;
    guint32 hash_len;
    guint32 flags;
    guint16 major, minor, build, revision;
};
#endif

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

using namespace ::rtl;
using namespace ::mono_uno;

extern "C" {

void SAL_CALL Mapping_acquire( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    static_cast< Mapping const * >( mapping )->m_bridge->acquire();
}

void SAL_CALL Mapping_release( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    static_cast< Mapping const * >( mapping )->m_bridge->release();
}

void SAL_CALL Mapping_map_to_uno(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    uno_Interface ** ppUnoI = (uno_Interface **)ppOut;
    void * monoI = pIn;

// FIXME do this here? OSL_ASSERT( sizeof (void *) >= sizeof (guint32))
    OSL_ENSURE( ppUnoI && td, "### null ptr!" );

    if (0 != *ppUnoI)
    {
        uno_Interface * pUnoI = *ppUnoI;
        (*pUnoI->release)( pUnoI );
        *ppUnoI = 0;
    }

    try
    {
        Bridge const *bridge =
            static_cast< Mapping const * >( mapping )->m_bridge;
        // FIXME any wrapper necessary around mono calls? cf. JNI_guarded_context
        uno_Interface * pUnoI = bridge->map_to_uno(
            monoI, (typelib_TypeDescription *)td );
        *ppUnoI = pUnoI;
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg(
            OUStringToOString(
                OUSTR("[mono_uno bridge error] ") + err.m_message,
                RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, cstr_msg.getStr() );
#endif
    }
}

void SAL_CALL Mapping_map_to_mono(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    void ** pMonoI = ppOut;
    uno_Interface * pUnoI = (uno_Interface *)pIn;

    OSL_ENSURE( ppOut && td, "### null ptr!" );

    if (0 != *pMonoI)
    {
        // FIXME JNI bridge has guarded_context here
        // FIXME: do the right thing in the managed bridge
        // mono_gchandle_free( *pMonoI );
    }

    try
    {
        if (0 != pUnoI)
        {
            Bridge const * bridge =
                static_cast< Mapping const *>( mapping )->m_bridge;
            // FIXME guarded context
             *ppOut = (void *)bridge->map_to_mono(
                 pUnoI, (typelib_TypeDescription *)td );
             OSL_ASSERT( ppOut && *ppOut );
        }
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL >= 1
        rtl::OString cstr_msg(
            rtl::OUStringToOString(
                OUSTR("[mono_uno bridge error] ") + err.m_message,
                RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, cstr_msg.getStr() );
#endif
    }
}

void SAL_CALL Bridge_free( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping * that = static_cast< Mapping * >( mapping );
    delete that->m_bridge;
}

} // extern "C"

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

namespace mono_uno
{

void Bridge::acquire() const SAL_THROW( () )
{
    if (1 == osl_incrementInterlockedCount( &m_ref ))
    {
        if (m_registered_mono2uno)
        {
            uno_Mapping * mapping = const_cast< Mapping * >( &m_mono2uno );
            uno_registerMapping(
                &mapping, Bridge_free,
                m_mono_env, (uno_Environment *)m_uno_env, 0 );
        }
        else
        {
            uno_Mapping * mapping = const_cast< Mapping * >( &m_uno2mono );
            uno_registerMapping(
                &mapping, Bridge_free,
                (uno_Environment *)m_uno_env, m_mono_env, 0 );
        }
    }
}

void Bridge::release() const SAL_THROW( () )
{
    if (! osl_decrementInterlockedCount( &m_ref ))
    {
        uno_revokeMapping(
            m_registered_mono2uno
            ? const_cast< Mapping * >( &m_mono2uno )
            : const_cast< Mapping * >( &m_uno2mono ) );
    }
}

MonoAssembly *
DoLoad (MonoDomain * /* domain */, char * /* fullname */)
{
        MonoAssemblyName aname;
        MonoImageOpenStatus status = MONO_IMAGE_OK;
        MonoAssembly *ass;

        memset (&aname, 0, sizeof (aname));
        aname.culture = "";
        strncpy ((char *)aname.public_key_token, "ce2cb7e279207b9e", MONO_PUBLIC_KEY_TOKEN_LENGTH);
        aname.name = "cli_uno_bridge";
        aname.major=1;
        aname.minor=0;
        aname.build=0;
        aname.revision=0;

        ass = mono_assembly_load (&aname, NULL, &status);
        if (status != MONO_IMAGE_OK)
                return NULL;
        return ass;
}

Bridge::Bridge(
    uno_Environment * mono_env, uno_ExtEnvironment * uno_env,
    bool registered_mono2uno )
    : m_ref( 1 ),
      m_uno_env( uno_env ),
      m_mono_env( mono_env ),
      m_registered_mono2uno( registered_mono2uno )
{
    MonoDomain * pDomain = mono_get_root_domain();
    // FIXME where is this freed?
    MonoAssembly * pAssembly = DoLoad (pDomain, "cli_uno_bridge, Version=1.0.0.0, Culture=neutral, PublicKeyToken=ce2cb7e279207b9e");
    // FIXME and this, is this needed later?
    MonoClass * pClass = mono_class_from_name (
        (MonoImage *)mono_assembly_get_image( pAssembly ), "com.sun.star.bridges.mono_uno", "Bridge" );
    OSL_ASSERT( 0 != pClass );
    /* FIXME add args to method description string */
    MonoMethodDesc * pMethodDesc = mono_method_desc_new( ":.ctor", FALSE );
    MonoMethod * pCtor = mono_method_desc_search_in_class( pMethodDesc, pClass );
    mono_method_desc_free( pMethodDesc );
    OSL_ASSERT( 0 != pCtor );

    pMethodDesc = mono_method_desc_new( "Bridge:MapManagedToUno", FALSE );
    m_mapManagedToUnoMethod = mono_method_desc_search_in_class( pMethodDesc, pClass );
    mono_method_desc_free( pMethodDesc );
    OSL_ASSERT( 0 != m_mapManagedToUnoMethod );

    pMethodDesc = mono_method_desc_new( "Bridge:MapUnoToManaged", FALSE );
    m_mapUnoToManagedMethod = mono_method_desc_search_in_class( pMethodDesc, pClass );
    mono_method_desc_free( pMethodDesc );
    OSL_ASSERT( 0 != m_mapUnoToManagedMethod );

    gpointer pParams[1];
    pParams[0] = &uno_env;
    m_managedBridge = mono_object_new( pDomain, pClass );
    mono_uno::runtime_invoke( pCtor, m_managedBridge, pParams, NULL,
                              mono_object_get_domain( m_managedBridge ) );

    OSL_ASSERT( 0 != m_mono_env && 0 != m_uno_env );
    (*((uno_Environment *)m_uno_env)->acquire)( (uno_Environment *)m_uno_env );
    (*m_mono_env->acquire)( m_mono_env );

    // mono2uno mapping
    m_mono2uno.acquire = Mapping_acquire;
    m_mono2uno.release = Mapping_release;
    m_mono2uno.mapInterface = Mapping_map_to_uno;
    m_mono2uno.m_bridge = this;
    // uno2mono mapping
    m_uno2mono.acquire = Mapping_acquire;
    m_uno2mono.release = Mapping_release;
    m_uno2mono.mapInterface = Mapping_map_to_mono;
    m_uno2mono.m_bridge = this;

    (*g_moduleCount.modCnt.acquire)( &g_moduleCount.modCnt );
}

Bridge::~Bridge() SAL_THROW( () )
{
    (*m_mono_env->release)( m_mono_env );
    (*((uno_Environment *)m_uno_env)->release)( (uno_Environment *)m_uno_env );
    // FIXME release managed bridge

    (*g_moduleCount.modCnt.release)( &g_moduleCount.modCnt );
}

void * Bridge::map_to_mono(
    uno_Interface *pUnoI, typelib_TypeDescription * pTD ) const
{
    gpointer pMonoParams[2];

    pMonoParams[0] = &pUnoI;
    pMonoParams[1] = pTD;

    return
        mono_uno::runtime_invoke( m_mapUnoToManagedMethod,
                                  m_managedBridge, pMonoParams, NULL,
                                  mono_object_get_domain( m_managedBridge ) );
}

uno_Interface * Bridge::map_to_uno(
    void * pMonoI, typelib_TypeDescription * pTD ) const
{
    gpointer pMonoParams[2];
    uno_Interface ** ppResult;

    pMonoParams[0] = pMonoI;
    pMonoParams[1] = pTD;

    ppResult = (uno_Interface **)mono_object_unbox(
        mono_uno::runtime_invoke( m_mapManagedToUnoMethod,
                                  m_managedBridge, pMonoParams, NULL,
                                  mono_object_get_domain( m_managedBridge ) ) );

    return *ppResult;
}

} // namespace mono_uno

extern "C" {

// void SAL_CALL mono_environmentDisposing( uno_Environment * mono_env )
// 	SAL_THROW_EXTERN_C()
// {
// }

void SAL_CALL uno_initEnvironment( uno_Environment * mono_env )
    SAL_THROW_EXTERN_C()
{
    // mono_env->environmentDisposing = mono_environmentDisposing;
    mono_env->pExtEnv = 0; /* no extended support */
}

void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment *pFrom, uno_Environment *pTo )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( 0 != ppMapping && 0 != pFrom && 0 != pTo );
    if (0 != *ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
    }

    /* FIXME check that mono's and sal's types have matching sizes
     * like jni_bridge does? */

    OUString const & from_env_typename = OUString::unacquired( &pFrom->pTypeName );
    OUString const & to_env_typename = OUString::unacquired( &pTo->pTypeName );

    uno_Mapping * mapping = 0;

    try
    {
        if (from_env_typename.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( UNO_LB_CLI )) &&
            to_env_typename.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( UNO_LB_UNO)))
        {
            Bridge * bridge =
                new Bridge( pFrom, pTo->pExtEnv, true ); // ref count = 1
            mapping = &bridge->m_mono2uno;
            uno_registerMapping(
                &mapping, Bridge_free, pFrom,
                (uno_Environment *)pTo->pExtEnv, 0);
        }
        else if (from_env_typename.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM( UNO_LB_UNO)) &&
                 to_env_typename.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM( UNO_LB_CLI)))
        {
            Bridge * bridge =
                new Bridge( pTo, pFrom->pExtEnv, false ); // ref count = 1
            mapping = &bridge->m_uno2mono;
            uno_registerMapping(
                &mapping, Bridge_free,
                (uno_Environment *)pFrom->pExtEnv, pTo, 0);
        }
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL >= 1
        OString cstr_msg(
                OUStringToOString(
                    OUSTR("[mono_uno bridge error] ") + err.m_message,
                    RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, cstr_msg.getStr() );
#endif
    }

    *ppMapping = mapping;
}

sal_Bool SAL_CALL component_canUnload( TimeValue * pTime )
    SAL_THROW_EXTERN_C()
{
    return (*g_moduleCount.canUnload)( &g_moduleCount, pTime );
}

} // extern "C"

MonoObject*
mono_uno::runtime_invoke (MonoMethod *method, void *obj, void **params,
                          MonoObject **exc, MonoDomain *domain)
{
    mono_thread_attach( domain );
    return mono_runtime_invoke( method, obj, params, exc );
}
