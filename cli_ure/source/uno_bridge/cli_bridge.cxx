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

#include <vcclr.h>
//ToDo: remove when build with .NET 2
#pragma warning(push, 1)
#include <windows.h>
#include "uno/environment.hxx"
#pragma warning(pop)
#include "rtl/unload.h"
#include "uno/lbnames.h"
#include "uno/mapping.hxx"
#include "typelib/typedescription.hxx"
#include "rtl/ustring.hxx"

#include "cli_bridge.h"
#include "cli_proxy.h"
#if defined(_MSC_VER) && (_MSC_VER < 1400)
#include <_vcclrit.h>
#endif

namespace sri= System::Runtime::InteropServices;
using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

namespace cli_uno
{

extern "C"
{
void SAL_CALL Mapping_acquire( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping const * that = static_cast< Mapping const * >( mapping );
     that->m_bridge->acquire();
}
//--------------------------------------------------------------------------------------------------
void SAL_CALL Mapping_release( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping const * that = static_cast< Mapping const * >( mapping );
    that->m_bridge->release();
}


//--------------------------------------------------------------------------------------------------
void SAL_CALL Mapping_cli2uno(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    uno_Interface ** ppUnoI = (uno_Interface **)ppOut;
    intptr_t  cliI = (intptr_t)pIn;

    OSL_ENSURE( ppUnoI && td, "### null ptr!" );

     if (0 != *ppUnoI)
     {
         uno_Interface * pUnoI = *(uno_Interface **)ppUnoI;
         (*pUnoI->release)( pUnoI );
         *ppUnoI = 0;
     }
    try
    {
        Mapping const * that = static_cast< Mapping const * >( mapping );
        Bridge * bridge = that->m_bridge;

        if (0 != cliI)
        {
            System::Object^ cliObj= sri::GCHandle::FromIntPtr(IntPtr(cliI)).Target;
            (*ppOut)= bridge->map_cli2uno(cliObj, (typelib_TypeDescription*) td);
        }
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL >= 1
        OString cstr_msg(
            OUStringToOString(
                OUSTR("[cli_uno bridge error] ") + err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr_msg.getStr() );
#else
        (void) err; // unused
#endif
    }
}
//--------------------------------------------------------------------------------------------------
void SAL_CALL Mapping_uno2cli(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    try
    {
        OSL_ENSURE( td && ppOut, "### null ptr!" );
        OSL_ENSURE( (sizeof(System::Char) == sizeof(sal_Unicode))
                    && (sizeof(System::Boolean) == sizeof(sal_Bool))
                    && (sizeof(System::SByte) == sizeof(sal_Int8))
                    && (sizeof(System::Int16) == sizeof(sal_Int16))
                    && (sizeof(System::UInt16) == sizeof(sal_uInt16))
                    && (sizeof(System::Int32) == sizeof(sal_Int32))
                    && (sizeof(System::UInt32) == sizeof(sal_uInt32))
                    && (sizeof(System::Int64) == sizeof(sal_Int64))
                    && (sizeof(System::UInt64) == sizeof(sal_uInt64))
                    && (sizeof(System::Single) == sizeof(float))
                    && (sizeof(System::Double) == sizeof(double)),
                    "[cli_uno bridge] incompatible .NET data types");
        intptr_t * ppDNetI = (intptr_t *)ppOut;
        uno_Interface * pUnoI = (uno_Interface *)pIn;

        Mapping const * that = static_cast< Mapping const * >( mapping );
        Bridge  * bridge = that->m_bridge;

        if (0 != *ppDNetI)
        {
            sri::GCHandle::FromIntPtr(IntPtr(ppDNetI)).Free();
        }

        if (0 != pUnoI)
        {
            System::Object^ cliI=  bridge->map_uno2cli(pUnoI, td);
            intptr_t ptr= NULL;
            if(cliI)
            {
                ptr= sri::GCHandle::ToIntPtr(sri::GCHandle::Alloc(cliI))
#ifdef _WIN32
                    .ToInt32();
#else /* defined(_WIN64) */                 .ToInt64();
#endif
            }
            (*ppOut)= reinterpret_cast<void*>(ptr);
        }
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL >= 1
        rtl::OString cstr_msg(
            rtl::OUStringToOString(
                OUSTR("[cli_uno bridge error] ") + err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr_msg.getStr() );
#else
        (void) err; // unused
#endif
    }
}

//__________________________________________________________________________________________________
void SAL_CALL Bridge_free( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping * that = static_cast< Mapping * >( mapping );
    delete that->m_bridge;
}

} //extern C
} //namespace

namespace cli_uno
{

//__________________________________________________________________________________________________
/** ToDo
    I doubt that the the case that the ref count raises from 0 to 1
    can occur.  uno_ext_getMapping returns an acquired mapping. Every time
    that function is called then a new mapping is created. Following the
    rules of ref counted objects, then if the ref count is null noone has
    a reference to the object anymore. Hence noone can call acquire. If someone
    calls acquire then they must have kept an unacquired pointer which is
    illegal.
 */
void Bridge::acquire()  const SAL_THROW(())
{
    if (1 == osl_atomic_increment( &m_ref ))
    {
        if (m_registered_cli2uno)
        {
            uno_Mapping * mapping = const_cast<Mapping*>(&m_cli2uno);
            uno_registerMapping(
                & const_cast<uno_Mapping*>(mapping), Bridge_free, m_uno_cli_env, (uno_Environment *)m_uno_env, 0 );
        }
        else
        {
            uno_Mapping * mapping = const_cast<Mapping*>(&m_uno2cli);
            uno_registerMapping(
                &mapping, Bridge_free, (uno_Environment *)m_uno_env, m_uno_cli_env, 0 );
        }
    }
}
//__________________________________________________________________________________________________
void Bridge::release() const  SAL_THROW(())
{
    if (! osl_atomic_decrement( &m_ref ))
    {
        uno_revokeMapping(
            m_registered_cli2uno
            ?  const_cast<Mapping*>(&m_cli2uno)
            :  const_cast<Mapping*>(&m_uno2cli)  );
   }
}
//__________________________________________________________________________________________________
Bridge::Bridge(
    uno_Environment * uno_cli_env, uno_ExtEnvironment * uno_env,
    bool registered_cli2uno )
    : m_ref( 1 ),
      m_uno_env( uno_env ),
      m_uno_cli_env( uno_cli_env ),
      m_registered_cli2uno( registered_cli2uno )
{
    OSL_ASSERT( 0 != m_uno_cli_env && 0 != m_uno_env );
    (*((uno_Environment *)m_uno_env)->acquire)( (uno_Environment *)m_uno_env );
    (*m_uno_cli_env->acquire)( m_uno_cli_env );

    // cli2uno
    m_cli2uno.acquire = Mapping_acquire;
    m_cli2uno.release = Mapping_release;
    m_cli2uno.mapInterface = Mapping_cli2uno;
    m_cli2uno.m_bridge = this;
    // uno2cli
    m_uno2cli.acquire = Mapping_acquire;
    m_uno2cli.release = Mapping_release;
    m_uno2cli.mapInterface = Mapping_uno2cli;
    m_uno2cli.m_bridge = this;

}

//__________________________________________________________________________________________________
Bridge::~Bridge() SAL_THROW(())
{
    //System::GC::Collect();
    (*m_uno_cli_env->release)( m_uno_cli_env );
    (*((uno_Environment *)m_uno_env)->release)( (uno_Environment *)m_uno_env );
}



} //namespace cli_uno

extern "C"
{

namespace cli_uno
{
//--------------------------------------------------------------------------------------------------
void SAL_CALL cli_env_disposing( uno_Environment * uno_cli_env )
    SAL_THROW_EXTERN_C()
{
    uno_cli_env->pContext = 0;
}

//##################################################################################################
SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_initEnvironment( uno_Environment * uno_cli_env )
    SAL_THROW_EXTERN_C()
{
    //ToDo: remove when compiled with .NET 2
#if defined(_MSC_VER) && (_MSC_VER < 1400)
    __crt_dll_initialize();
#endif

    uno_cli_env->environmentDisposing= cli_env_disposing;
    uno_cli_env->pExtEnv = 0;
    //Set the console to print Trace messages
#if OSL_DEBUG_LEVEL >= 1
    System::Diagnostics::Trace::Listeners->
            Add( gcnew System::Diagnostics::TextWriterTraceListener(System::Console::Out));
#endif
    OSL_ASSERT( 0 == uno_cli_env->pContext );

    // We let the Cli_environment leak, since there is no good point where we could destruct it.
    //dispose is not used because we would have then also synchronize the calls to proxies. If the
    //Cli_environment is disposed, we must prevent all calls, otherwise we may crash at points
    //where g_cli_env is accessed.
    //When we compile the bridge with .NET 2 then we can again hold g_cli_env as a static gcroot
    //member in a unmanaged class, such as Bridge.
    CliEnvHolder::g_cli_env = gcnew Cli_environment();
}
//##################################################################################################
SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( 0 != ppMapping && 0 != pFrom && 0 != pTo );
    if (*ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
    }


    OUString const & from_env_typename = *reinterpret_cast< OUString const * >(
        &pFrom->pTypeName );
    OUString const & to_env_typename = *reinterpret_cast< OUString const * >(
        &pTo->pTypeName );

    uno_Mapping * mapping = 0;

    try
    {
        if ( from_env_typename == UNO_LB_CLI && to_env_typename == UNO_LB_UNO )
        {
            Bridge * bridge = new Bridge( pFrom, pTo->pExtEnv, true ); // ref count = 1
            mapping = &bridge->m_cli2uno;
            uno_registerMapping(
                &mapping, Bridge_free, pFrom, (uno_Environment *)pTo->pExtEnv, 0 );
        }
        else if ( from_env_typename == UNO_LB_UNO && to_env_typename == UNO_LB_CLI )
        {
            Bridge * bridge = new Bridge( pTo, pFrom->pExtEnv, false ); // ref count = 1
            mapping = &bridge->m_uno2cli;
            uno_registerMapping(
                &mapping, Bridge_free, (uno_Environment *)pFrom->pExtEnv, pTo, 0 );
        }
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL >= 1
        OString cstr_msg(
            OUStringToOString(
                OUSTR("[cli_uno bridge error] ") + err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr_msg.getStr() );
#else
        (void) err; // unused
#endif
    }
    *ppMapping = mapping;
}


//##################################################################################################
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue * )
    SAL_THROW_EXTERN_C()
{
    return true;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
