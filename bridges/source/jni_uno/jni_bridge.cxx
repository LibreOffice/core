/*************************************************************************
 *
 *  $RCSfile: jni_bridge.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-09-26 14:37:01 $
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

#include "jni_bridge.h"

#include <rtl/unload.h>
#include <rtl/strbuf.hxx>
#include <uno/lbnames.h>


using namespace ::rtl;

namespace jni_bridge
{

//==================================================================================================
rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

//__________________________________________________________________________________________________
static void SAL_CALL jni_Bridge_free( uno_Mapping * mapping ) SAL_THROW( () )
{
    delete static_cast< jni_Mapping * >( mapping )->m_bridge;
}
//__________________________________________________________________________________________________
void jni_Bridge::acquire() const SAL_THROW( () )
{
    if (1 == osl_incrementInterlockedCount( &m_ref ))
    {
        if (m_registered_java2uno)
        {
            uno_Mapping * mapping = const_cast< jni_Mapping * >( &m_java2uno );
            ::uno_registerMapping(
                &mapping, jni_Bridge_free,
                m_java_env, (uno_Environment *)m_uno_env, 0 );
        }
        else
        {
            uno_Mapping * mapping = const_cast< jni_Mapping * >( &m_uno2java );
            ::uno_registerMapping(
                &mapping, jni_Bridge_free,
                (uno_Environment *)m_uno_env, m_java_env, 0 );
        }
    }
}
//__________________________________________________________________________________________________
void jni_Bridge::release() const SAL_THROW( () )
{
    if (! osl_decrementInterlockedCount( &m_ref ))
    {
        ::uno_revokeMapping(
            m_registered_java2uno
            ? const_cast< jni_Mapping * >( &m_java2uno )
            : const_cast< jni_Mapping * >( &m_uno2java ) );
    }
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL jni_Mapping_acquire( uno_Mapping * mapping ) SAL_THROW( () )
{
    static_cast< jni_Mapping * >( mapping )->m_bridge->acquire();
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL jni_Mapping_release( uno_Mapping * mapping ) SAL_THROW( () )
{
    static_cast< jni_Mapping * >( mapping )->m_bridge->release();
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL jni_Mapping_java2uno(
    uno_Mapping * mapping, uno_Interface ** ppUnoI,
    jobject javaI, typelib_InterfaceTypeDescription * td )
    SAL_THROW( () )
{
    OSL_ASSERT( sizeof (void *) == sizeof (jobject) );
    OSL_ENSURE( ppUnoI && td, "### null ptr!" );

    if (0 != *ppUnoI)
    {
        uno_Interface * pUnoI = *(uno_Interface **)ppUnoI;
        (*pUnoI->release)( pUnoI );
        *ppUnoI = 0;
    }
    if (0 != javaI)
    {
        try
        {
            jni_Bridge const * that = static_cast< jni_Mapping * >( mapping )->m_bridge;
            JNI_attach attach( that->m_java_env );
            JNI_type_info const * info = that->m_jni_class_data->get_type_info(
                (typelib_TypeDescription *)td, attach );
            *ppUnoI = that->map_java2uno( attach, javaI, info );
        }
#ifdef _DEBUG
        catch (BridgeRuntimeError & err)
        {
            OString cstr( OUStringToOString( err.m_message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, cstr.getStr() );
        }
#endif
        catch (...)
        {
            OSL_ENSURE( 0, "### error mapping java to uno!" );
        }
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL jni_Mapping_uno2java(
    uno_Mapping * mapping, jobject * ppJavaI,
    uno_Interface * pUnoI, typelib_InterfaceTypeDescription * td )
    SAL_THROW( () )
{
    OSL_ASSERT( sizeof (void *) == sizeof (jobject) );
    OSL_ENSURE( ppJavaI && td, "### null ptr!" );

    try
    {
        jni_Bridge * that = static_cast< jni_Mapping * >( mapping )->m_bridge;
        JNI_attach attach( that->m_java_env );

        if (0 != *ppJavaI)
        {
            attach->DeleteLocalRef( *ppJavaI );
            *ppJavaI = 0;
        }
        if (0 != pUnoI)
        {
            JNI_type_info const * info = that->m_jni_class_data->get_type_info(
                (typelib_TypeDescription *)td, attach );
            *ppJavaI = that->map_uno2java( attach, pUnoI, info );
        }
    }
#ifdef _DEBUG
    catch (BridgeRuntimeError & err)
    {
        OString cstr_msg( OUStringToOString( err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, cstr_msg.getStr() );
    }
#endif
    catch (...)
    {
        OSL_ENSURE( 0, "### error mapping uno to java!" );
    }
}

//__________________________________________________________________________________________________
jni_Bridge::jni_Bridge(
    uno_Environment * java_env, uno_ExtEnvironment * uno_env, bool register_java2uno )
    SAL_THROW( () )
        : m_ref( 1 ),
          m_java_env( java_env ),
          m_uno_env( uno_env ),
          m_registered_java2uno( register_java2uno )
{
    OSL_ASSERT( 0 != m_java_env && 0 != m_uno_env );
    (*m_java_env->acquire)( m_java_env );
    (*((uno_Environment *)m_uno_env)->acquire)( (uno_Environment *)m_uno_env );

    m_jvm_context = (JavaVMContext *)java_env->pContext;
    m_jni_class_data = (JNI_class_data *)m_jvm_context->m_extra;

    // java2uno
    m_java2uno.acquire = jni_Mapping_acquire;
    m_java2uno.release = jni_Mapping_release;
    m_java2uno.mapInterface = (uno_MapInterfaceFunc)jni_Mapping_java2uno;
    m_java2uno.m_bridge = this;
    // uno2java
    m_uno2java.acquire = jni_Mapping_acquire;
    m_uno2java.release = jni_Mapping_release;
    m_uno2java.mapInterface = (uno_MapInterfaceFunc)jni_Mapping_uno2java;
    m_uno2java.m_bridge = this;

    (*g_moduleCount.modCnt.acquire)( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
jni_Bridge::~jni_Bridge()
    SAL_THROW( () )
{
    (*((uno_Environment *)m_uno_env)->release)( (uno_Environment *)m_uno_env );
    (*m_java_env->release)( m_java_env );

    (*g_moduleCount.modCnt.release)( &g_moduleCount.modCnt );
}

//##################################################################################################

//--------------------------------------------------------------------------------------------------
static void SAL_CALL java_env_disposing( uno_Environment * java_env )
    SAL_THROW( () )
{
    JavaVMContext * pVMContext = reinterpret_cast< JavaVMContext * >( java_env->pContext );
    delete reinterpret_cast< JNI_class_data * >( pVMContext->m_extra );
    java_env->pContext = 0;
    delete pVMContext;
}

}

using namespace ::jni_bridge;

extern "C"
{
//##################################################################################################
void SAL_CALL uno_initEnvironment( uno_Environment * java_env )
    SAL_THROW_EXTERN_C()
{
    java_env->environmentDisposing = java_env_disposing;
    java_env->pExtEnv = 0; // no extended support
    OSL_ASSERT( 0 != java_env->pContext );
    try
    {
        ((JavaVMContext *)java_env->pContext)->m_extra = new JNI_class_data( java_env );
    }
#ifdef _DEBUG
    catch (BridgeRuntimeError & err)
    {
        OStringBuffer buf( 64 );
        buf.append( RTL_CONSTASCII_STRINGPARAM("### error initializing (jni) java environment: ") );
        buf.append( OUStringToOString( err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OString msg( buf.makeStringAndClear() );
        OSL_ENSURE( 0, msg.getStr() );
    }
#endif
    catch (...)
    {
        OSL_ENSURE( 0, "### error initializing (jni) java environment!" );
    }
}
//##################################################################################################
void SAL_CALL uno_ext_getMapping(
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

    if (from_env_typename.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_JAVA) ) &&
        to_env_typename.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ))
    {
        jni_Bridge * bridge = new jni_Bridge(
            pFrom, pTo->pExtEnv, true ); // ref count = 1
        mapping = &bridge->m_java2uno;
        uno_registerMapping(
            &mapping, jni_Bridge_free,
            pFrom, (uno_Environment *)pTo->pExtEnv, 0 );
    }
    else if (from_env_typename.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ) &&
             to_env_typename.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_JAVA) ))
    {
        jni_Bridge * bridge = new jni_Bridge(
            pTo, pFrom->pExtEnv, false ); // ref count = 1
        mapping = &bridge->m_uno2java;
        uno_registerMapping(
            &mapping, jni_Bridge_free,
            (uno_Environment *)pFrom->pExtEnv, pTo, 0 );
    }

    *ppMapping = mapping;
}
//##################################################################################################
sal_Bool SAL_CALL component_canUnload( TimeValue * pTime )
    SAL_THROW_EXTERN_C()
{
    return (*g_moduleCount.canUnload)( &g_moduleCount, pTime );
}
}
