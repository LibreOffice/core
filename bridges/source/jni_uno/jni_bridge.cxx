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

#include <sal/config.h>

#include <cassert>

#include "jni_bridge.h"

#include <boost/static_assert.hpp>
#include "jvmaccess/unovirtualmachine.hxx"
#include "o3tl/heap_ptr.hxx"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "uno/lbnames.h"

using namespace ::rtl;
using namespace ::osl;
using namespace ::jni_uno;

namespace
{
extern "C"
{


void SAL_CALL Mapping_acquire( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping const * that = static_cast< Mapping const * >( mapping );
    that->m_bridge->acquire();
}


void SAL_CALL Mapping_release( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping const * that = static_cast< Mapping const * >( mapping );
    that->m_bridge->release();
}


void SAL_CALL Mapping_map_to_uno(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    uno_Interface ** ppUnoI = (uno_Interface **)ppOut;
    jobject javaI = (jobject) pIn;

    BOOST_STATIC_ASSERT( sizeof (void *) == sizeof (jobject) );
    assert(ppUnoI != 0);
    assert(td != 0);

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
            SAL_WARN(
                "bridges",
                "ingoring BridgeRuntimeError \"" << err.m_message << "\"");
        }
        catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
            SAL_WARN("bridges", "attaching current thread to java failed");
        }
    }
}


void SAL_CALL Mapping_map_to_java(
    uno_Mapping * mapping, void ** ppOut,
    void * pIn, typelib_InterfaceTypeDescription * td )
    SAL_THROW_EXTERN_C()
{
    jobject * ppJavaI = (jobject *) ppOut;
    uno_Interface * pUnoI = (uno_Interface *)pIn;

    BOOST_STATIC_ASSERT( sizeof (void *) == sizeof (jobject) );
    assert(ppJavaI != 0);
    assert(td != 0);

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
        SAL_WARN(
            "bridges",
            "ingoring BridgeRuntimeError \"" << err.m_message << "\"");
    }
    catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        SAL_WARN("bridges", "attaching current thread to java failed");
    }
}


void SAL_CALL Bridge_free( uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    Mapping * that = static_cast< Mapping * >( mapping );
    delete that->m_bridge;
}

}

}

namespace jni_uno
{


void Bridge::acquire() const SAL_THROW(())
{
    if (1 == osl_atomic_increment( &m_ref ))
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


void Bridge::release() const SAL_THROW(())
{
    if (! osl_atomic_decrement( &m_ref ))
    {
        uno_revokeMapping(
            m_registered_java2uno
            ? const_cast< Mapping * >( &m_java2uno )
            : const_cast< Mapping * >( &m_uno2java ) );
    }
}


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

    assert(m_java_env != 0);
    assert(m_uno_env != 0);
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
}


Bridge::~Bridge() SAL_THROW(())
{
    (*m_java_env->release)( m_java_env );
    (*((uno_Environment *)m_uno_env)->release)( (uno_Environment *)m_uno_env );
}



void JNI_context::java_exc_occurred() const
{
    // !don't rely on JNI_info!

    JLocalAutoRef jo_exc( *this, m_env->ExceptionOccurred() );
    m_env->ExceptionClear();
    assert(jo_exc.is());
    if (! jo_exc.is())
    {
        throw BridgeRuntimeError(
            "java exception occurred, but not available!?" +
            get_stack_trace() );
    }

    // call toString(); don't rely on m_jni_info
    jclass jo_class = m_env->FindClass( "java/lang/Object" );
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "cannot get class java.lang.Object!" + get_stack_trace() );
    }
    JLocalAutoRef jo_Object( *this, jo_class );
    // method Object.toString()
    jmethodID method_Object_toString = m_env->GetMethodID(
        (jclass) jo_Object.get(), "toString", "()Ljava/lang/String;" );
    if (JNI_FALSE != m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "cannot get method id of java.lang.Object.toString()!" +
            get_stack_trace() );
    }
    assert(method_Object_toString != 0);

    JLocalAutoRef jo_descr(
        *this, m_env->CallObjectMethodA(
            jo_exc.get(), method_Object_toString, 0 ) );
    if (m_env->ExceptionCheck()) // no chance at all
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "error examining java exception object!" +
            get_stack_trace() );
    }

    jsize len = m_env->GetStringLength( (jstring) jo_descr.get() );
    o3tl::heap_ptr< rtl_mem > ustr_mem(
        rtl_mem::allocate(
            sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
    rtl_uString * ustr = (rtl_uString *)ustr_mem.get();
    m_env->GetStringRegion( (jstring) jo_descr.get(), 0, len, ustr->buffer );
    if (m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "invalid java string object!" + get_stack_trace() );
    }
    ustr->refCount = 1;
    ustr->length = len;
    ustr->buffer[ len ] = '\0';
    OUString message( (rtl_uString *)ustr_mem.release(), SAL_NO_ACQUIRE );

    throw BridgeRuntimeError( message + get_stack_trace( jo_exc.get() ) );
}


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
                o3tl::heap_ptr< rtl_mem > ustr_mem(
                    rtl_mem::allocate(
                        sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
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


SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_initEnvironment( uno_Environment * java_env )
    SAL_THROW_EXTERN_C()
{
    java_env->environmentDisposing = java_env_disposing;
    java_env->pExtEnv = 0; // no extended support
    assert(java_env->pContext != 0);

    ::jvmaccess::UnoVirtualMachine * machine =
          reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
              java_env->pContext );
    machine->acquire();
}

#ifdef DISABLE_DYNLOADING
#define uno_ext_getMapping java_uno_ext_getMapping
#endif


SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
    SAL_THROW_EXTERN_C()
{
    assert(ppMapping != 0);
    assert(pFrom != 0);
    assert(pTo != 0);
    if (0 != *ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
    }

    BOOST_STATIC_ASSERT( JNI_FALSE == sal_False );
    BOOST_STATIC_ASSERT( JNI_TRUE == sal_True );
    BOOST_STATIC_ASSERT( sizeof (jboolean) == sizeof (sal_Bool) );
    BOOST_STATIC_ASSERT( sizeof (jchar) == sizeof (sal_Unicode) );
    BOOST_STATIC_ASSERT( sizeof (jdouble) == sizeof (double) );
    BOOST_STATIC_ASSERT( sizeof (jfloat) == sizeof (float) );
    BOOST_STATIC_ASSERT( sizeof (jbyte) == sizeof (sal_Int8) );
    BOOST_STATIC_ASSERT( sizeof (jshort) == sizeof (sal_Int16) );
    BOOST_STATIC_ASSERT( sizeof (jint) == sizeof (sal_Int32) );
    BOOST_STATIC_ASSERT( sizeof (jlong) == sizeof (sal_Int64) );

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
        SAL_WARN(
            "bridges",
            "ingoring BridgeRuntimeError \"" << err.m_message << "\"");
    }
    catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        SAL_WARN("bridges", "attaching current thread to java failed");
    }

    *ppMapping = mapping;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
