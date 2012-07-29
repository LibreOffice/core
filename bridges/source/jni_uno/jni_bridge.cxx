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


#include "jni_bridge.h"

#include "jvmaccess/unovirtualmachine.hxx"
#include "rtl/ref.hxx"
#include "rtl/unload.h"
#include "rtl/strbuf.hxx"
#include "uno/lbnames.h"


using namespace ::std;
using namespace ::rtl;
using namespace ::osl;
using namespace ::jni_uno;

namespace
{
extern "C"
{

//------------------------------------------------------------------------------
void SAL_CALL Mapping_acquire( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping const * that = static_cast< Mapping const * >( mapping );
    that->m_bridge->acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL Mapping_release( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping const * that = static_cast< Mapping const * >( mapping );
    that->m_bridge->release();
}

//------------------------------------------------------------------------------
void SAL_CALL Mapping_map_to_uno(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    uno_Interface ** ppUnoI = (uno_Interface **)ppOut;
    jobject javaI = (jobject) pIn;

    OSL_ASSERT( sizeof (void *) == sizeof (jobject) );
    OSL_ENSURE( ppUnoI && td, "### null ptr!" );

    if (0 == javaI)
    {
        if (0 != *ppUnoI)
        {
            uno_Interface * p = *(uno_Interface **)ppUnoI;
            (*p->release)( p );
            *ppUnoI = 0;
        }
    }
    else
    {
        try
        {
            Bridge const * bridge =
                static_cast< Mapping const * >( mapping )->m_bridge;
            JNI_guarded_context jni(
                bridge->m_jni_info,
                reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
                    bridge->m_java_env->pContext ) );

            JNI_interface_type_info const * info =
                static_cast< JNI_interface_type_info const * >(
                    bridge->m_jni_info->get_type_info(
                        jni, (typelib_TypeDescription *)td ) );
            uno_Interface * pUnoI = bridge->map_to_uno( jni, javaI, info );
            if (0 != *ppUnoI)
            {
                uno_Interface * p = *(uno_Interface **)ppUnoI;
                (*p->release)( p );
            }
            *ppUnoI = pUnoI;
        }
        catch (const BridgeRuntimeError & err)
        {
#if OSL_DEBUG_LEVEL > 0
            OString cstr_msg(
                OUStringToOString(
                    OUSTR("[jni_uno bridge error] ") + err.m_message,
                    RTL_TEXTENCODING_ASCII_US ) );
            OSL_FAIL( cstr_msg.getStr() );
#else
            (void) err; // unused
#endif
        }
        catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            OSL_FAIL(
                "[jni_uno bridge error] attaching current thread "
                "to java failed!" );
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL Mapping_map_to_java(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    jobject * ppJavaI = (jobject *) ppOut;
    uno_Interface * pUnoI = (uno_Interface *)pIn;

    OSL_ASSERT( sizeof (void *) == sizeof (jobject) );
    OSL_ENSURE( ppJavaI && td, "### null ptr!" );

    try
    {
        if (0 == pUnoI)
        {
            if (0 != *ppJavaI)
            {
                Bridge const * bridge =
                    static_cast< Mapping const * >( mapping )->m_bridge;
                JNI_guarded_context jni(
                    bridge->m_jni_info,
                    reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
                        bridge->m_java_env->pContext ) );
                jni->DeleteGlobalRef( *ppJavaI );
                *ppJavaI = 0;
            }
        }
        else
        {
            Bridge const * bridge =
                static_cast< Mapping const * >( mapping )->m_bridge;
            JNI_guarded_context jni(
                bridge->m_jni_info,
                reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
                    bridge->m_java_env->pContext ) );

            JNI_interface_type_info const * info =
                static_cast< JNI_interface_type_info const * >(
                    bridge->m_jni_info->get_type_info(
                        jni, (typelib_TypeDescription *)td ) );
            jobject jlocal = bridge->map_to_java( jni, pUnoI, info );
            if (0 != *ppJavaI)
                jni->DeleteGlobalRef( *ppJavaI );
            *ppJavaI = jni->NewGlobalRef( jlocal );
            jni->DeleteLocalRef( jlocal );
        }
    }
    catch (const BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg(
            OUStringToOString(
                OUSTR("[jni_uno bridge error] ") + err.m_message,
                RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr_msg.getStr() );
#else
            (void) err; // unused
#endif
    }
    catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        OSL_FAIL(
            "[jni_uno bridge error] attaching current thread to java failed!" );
    }
}

//______________________________________________________________________________
void SAL_CALL Bridge_free( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping * that = static_cast< Mapping * >( mapping );
    delete that->m_bridge;
}

}

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

}

namespace jni_uno
{

//______________________________________________________________________________
void Bridge::acquire() const SAL_THROW(())
{
    if (1 == osl_incrementInterlockedCount( &m_ref ))
    {
        if (m_registered_java2uno)
        {
            uno_Mapping * mapping = const_cast< Mapping * >( &m_java2uno );
            uno_registerMapping(
                &mapping, Bridge_free,
                m_java_env, (uno_Environment *)m_uno_env, 0 );
        }
        else
        {
            uno_Mapping * mapping = const_cast< Mapping * >( &m_uno2java );
            uno_registerMapping(
                &mapping, Bridge_free,
                (uno_Environment *)m_uno_env, m_java_env, 0 );
        }
    }
}

//______________________________________________________________________________
void Bridge::release() const SAL_THROW(())
{
    if (! osl_decrementInterlockedCount( &m_ref ))
    {
        uno_revokeMapping(
            m_registered_java2uno
            ? const_cast< Mapping * >( &m_java2uno )
            : const_cast< Mapping * >( &m_uno2java ) );
    }
}

//______________________________________________________________________________
Bridge::Bridge(
    uno_Environment * java_env, uno_ExtEnvironment * uno_env,
    bool registered_java2uno )
    : m_ref( 1 ),
      m_uno_env( uno_env ),
      m_java_env( java_env ),
      m_registered_java2uno( registered_java2uno )
{
    // bootstrapping bridge jni_info
    m_jni_info = JNI_info::get_jni_info(
        reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
            m_java_env->pContext ) );

    OSL_ASSERT( 0 != m_java_env && 0 != m_uno_env );
    (*((uno_Environment *)m_uno_env)->acquire)( (uno_Environment *)m_uno_env );
    (*m_java_env->acquire)( m_java_env );

    // java2uno
    m_java2uno.acquire = Mapping_acquire;
    m_java2uno.release = Mapping_release;
    m_java2uno.mapInterface = Mapping_map_to_uno;
    m_java2uno.m_bridge = this;
    // uno2java
    m_uno2java.acquire = Mapping_acquire;
    m_uno2java.release = Mapping_release;
    m_uno2java.mapInterface = Mapping_map_to_java;
    m_uno2java.m_bridge = this;

    (*g_moduleCount.modCnt.acquire)( &g_moduleCount.modCnt );
}

//______________________________________________________________________________
Bridge::~Bridge() SAL_THROW(())
{
    (*m_java_env->release)( m_java_env );
    (*((uno_Environment *)m_uno_env)->release)( (uno_Environment *)m_uno_env );

    (*g_moduleCount.modCnt.release)( &g_moduleCount.modCnt );
}


//______________________________________________________________________________
void JNI_context::java_exc_occurred() const
{
    // !don't rely on JNI_info!

    JLocalAutoRef jo_exc( *this, m_env->ExceptionOccurred() );
    m_env->ExceptionClear();
    OSL_ASSERT( jo_exc.is() );
    if (! jo_exc.is())
    {
        throw BridgeRuntimeError(
            OUSTR("java exception occurred, but not available!?") +
            get_stack_trace() );
    }

    // call toString(); don't rely on m_jni_info
    jclass jo_class = m_env->FindClass( "java/lang/Object" );
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            OUSTR("cannot get class java.lang.Object!") + get_stack_trace() );
    }
    JLocalAutoRef jo_Object( *this, jo_class );
    // method Object.toString()
    jmethodID method_Object_toString = m_env->GetMethodID(
        (jclass) jo_Object.get(), "toString", "()Ljava/lang/String;" );
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            OUSTR("cannot get method id of java.lang.Object.toString()!") +
            get_stack_trace() );
    }
    OSL_ASSERT( 0 != method_Object_toString );

    JLocalAutoRef jo_descr(
        *this, m_env->CallObjectMethodA(
            jo_exc.get(), method_Object_toString, 0 ) );
    if (m_env->ExceptionCheck()) // no chance at all
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            OUSTR("error examining java exception object!") +
            get_stack_trace() );
    }

    jsize len = m_env->GetStringLength( (jstring) jo_descr.get() );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr< rtl_mem > ustr_mem(
        rtl_mem::allocate(
            sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    rtl_uString * ustr = (rtl_uString *)ustr_mem.get();
    m_env->GetStringRegion( (jstring) jo_descr.get(), 0, len, ustr->buffer );
    if (m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            OUSTR("invalid java string object!") + get_stack_trace() );
    }
    ustr->refCount = 1;
    ustr->length = len;
    ustr->buffer[ len ] = '\0';
    OUString message( (rtl_uString *)ustr_mem.release(), SAL_NO_ACQUIRE );

    throw BridgeRuntimeError( message + get_stack_trace( jo_exc.get() ) );
}

//______________________________________________________________________________
void JNI_context::getClassForName(
    jclass * classClass, jmethodID * methodForName) const
{
    jclass c = m_env->FindClass("java/lang/Class");
    if (c != 0) {
        *methodForName = m_env->GetStaticMethodID(
            c, "forName",
            "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
    }
    *classClass = c;
}

//______________________________________________________________________________
jclass JNI_context::findClass(
    char const * name, jclass classClass, jmethodID methodForName,
    bool inException) const
{
    jclass c = 0;
    JLocalAutoRef s(*this, m_env->NewStringUTF(name));
    if (s.is()) {
        jvalue a[3];
        a[0].l = s.get();
        a[1].z = JNI_FALSE;
        a[2].l = m_class_loader;
        c = static_cast< jclass >(
            m_env->CallStaticObjectMethodA(classClass, methodForName, a));
    }
    if (!inException) {
        ensure_no_exception();
    }
    return c;
}

//______________________________________________________________________________
OUString JNI_context::get_stack_trace( jobject jo_exc ) const
{
    JLocalAutoRef jo_JNI_proxy(
        *this,
        find_class( *this, "com.sun.star.bridges.jni_uno.JNI_proxy", true ) );
    if (assert_no_exception())
    {
        // static method JNI_proxy.get_stack_trace()
        jmethodID method = m_env->GetStaticMethodID(
            (jclass) jo_JNI_proxy.get(), "get_stack_trace",
            "(Ljava/lang/Throwable;)Ljava/lang/String;" );
        if (assert_no_exception() && (0 != method))
        {
            jvalue arg;
            arg.l = jo_exc;
            JLocalAutoRef jo_stack_trace(
                *this, m_env->CallStaticObjectMethodA(
                    (jclass) jo_JNI_proxy.get(), method, &arg ) );
            if (assert_no_exception())
            {
                jsize len =
                    m_env->GetStringLength( (jstring) jo_stack_trace.get() );
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                auto_ptr< rtl_mem > ustr_mem(
                    rtl_mem::allocate(
                        sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
                SAL_WNODEPRECATED_DECLARATIONS_POP
                rtl_uString * ustr = (rtl_uString *)ustr_mem.get();
                m_env->GetStringRegion(
                    (jstring) jo_stack_trace.get(), 0, len, ustr->buffer );
                if (assert_no_exception())
                {
                    ustr->refCount = 1;
                    ustr->length = len;
                    ustr->buffer[ len ] = '\0';
                    return OUString(
                        (rtl_uString *)ustr_mem.release(), SAL_NO_ACQUIRE );
                }
            }
        }
    }
    return OUString();
}

}

using namespace ::jni_uno;

extern "C"
{
namespace
{

//------------------------------------------------------------------------------
void SAL_CALL java_env_disposing( uno_Environment * java_env )
    SAL_THROW_EXTERN_C()
{
    ::jvmaccess::UnoVirtualMachine * machine =
          reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
              java_env->pContext );
    java_env->pContext = 0;
    machine->release();
}
}

#ifdef DISABLE_DYNLOADING
#define uno_initEnvironment java_uno_initEnvironment
#endif

//------------------------------------------------------------------------------
SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_initEnvironment( uno_Environment * java_env )
    SAL_THROW_EXTERN_C()
{
    java_env->environmentDisposing = java_env_disposing;
    java_env->pExtEnv = 0; // no extended support
    OSL_ASSERT( 0 != java_env->pContext );

    ::jvmaccess::UnoVirtualMachine * machine =
          reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
              java_env->pContext );
    machine->acquire();
}

#ifdef DISABLE_DYNLOADING
#define uno_ext_getMapping java_uno_ext_getMapping
#endif

//------------------------------------------------------------------------------
SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( 0 != ppMapping && 0 != pFrom && 0 != pTo );
    if (0 != *ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
    }

    OSL_ASSERT( JNI_FALSE == sal_False );
    OSL_ASSERT( JNI_TRUE == sal_True );
    OSL_ASSERT( sizeof (jboolean) == sizeof (sal_Bool) );
    OSL_ASSERT( sizeof (jchar) == sizeof (sal_Unicode) );
    OSL_ASSERT( sizeof (jdouble) == sizeof (double) );
    OSL_ASSERT( sizeof (jfloat) == sizeof (float) );
    OSL_ASSERT( sizeof (jbyte) == sizeof (sal_Int8) );
    OSL_ASSERT( sizeof (jshort) == sizeof (sal_Int16) );
    OSL_ASSERT( sizeof (jint) == sizeof (sal_Int32) );
    OSL_ASSERT( sizeof (jlong) == sizeof (sal_Int64) );
    if ((JNI_FALSE == sal_False) &&
        (JNI_TRUE == sal_True) &&
        (sizeof (jboolean) == sizeof (sal_Bool)) &&
        (sizeof (jchar) == sizeof (sal_Unicode)) &&
        (sizeof (jdouble) == sizeof (double)) &&
        (sizeof (jfloat) == sizeof (float)) &&
        (sizeof (jbyte) == sizeof (sal_Int8)) &&
        (sizeof (jshort) == sizeof (sal_Int16)) &&
        (sizeof (jint) == sizeof (sal_Int32)) &&
        (sizeof (jlong) == sizeof (sal_Int64)))
    {
        OUString const & from_env_typename =
            OUString::unacquired( &pFrom->pTypeName );
        OUString const & to_env_typename =
            OUString::unacquired( &pTo->pTypeName );

        uno_Mapping * mapping = 0;

        try
        {
            if ( from_env_typename == UNO_LB_JAVA && to_env_typename == UNO_LB_UNO )
            {
                Bridge * bridge =
                    new Bridge( pFrom, pTo->pExtEnv, true ); // ref count = 1
                mapping = &bridge->m_java2uno;
                uno_registerMapping(
                    &mapping, Bridge_free,
                    pFrom, (uno_Environment *)pTo->pExtEnv, 0 );
            }
            else if ( from_env_typename == UNO_LB_UNO && to_env_typename == UNO_LB_JAVA )
            {
                Bridge * bridge =
                    new Bridge( pTo, pFrom->pExtEnv, false ); // ref count = 1
                mapping = &bridge->m_uno2java;
                uno_registerMapping(
                    &mapping, Bridge_free,
                    (uno_Environment *)pFrom->pExtEnv, pTo, 0 );
            }
        }
        catch (const BridgeRuntimeError & err)
        {
#if OSL_DEBUG_LEVEL > 0
            OString cstr_msg(
                OUStringToOString(
                    OUSTR("[jni_uno bridge error] ") + err.m_message,
                    RTL_TEXTENCODING_ASCII_US ) );
            OSL_FAIL( cstr_msg.getStr() );
#else
            (void) err; // unused
#endif
        }
        catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            OSL_FAIL(
                "[jni_uno bridge error] attaching current thread "
                "to java failed!" );
        }

        *ppMapping = mapping;
    }
}

#ifndef DISABLE_DYNLOADING

//------------------------------------------------------------------------------
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue * pTime )
    SAL_THROW_EXTERN_C()
{
    return (*g_moduleCount.canUnload)( &g_moduleCount, pTime );
}

#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
