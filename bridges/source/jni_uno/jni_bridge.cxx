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
#include <memory>

#include "jni_bridge.h"
#include "jniunoenvironmentdata.hxx"

#include "jvmaccess/unovirtualmachine.hxx"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "uno/lbnames.h"

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
    uno_Interface ** ppUnoI = reinterpret_cast<uno_Interface **>(ppOut);
    jobject javaI = static_cast<jobject>(pIn);

    static_assert(sizeof (void *) == sizeof (jobject), "must be the same size");
    assert(ppUnoI != nullptr);
    assert(td != nullptr);

    if (nullptr == javaI)
    {
        if (nullptr != *ppUnoI)
        {
            uno_Interface * p = *ppUnoI;
            (*p->release)( p );
            *ppUnoI = nullptr;
        }
    }
    else
    {
        try
        {
            Bridge const * bridge =
                static_cast< Mapping const * >( mapping )->m_bridge;
            JNI_guarded_context jni(
                bridge->getJniInfo(),
                (static_cast<jni_uno::JniUnoEnvironmentData *>(
                    bridge->m_java_env->pContext)
                 ->machine));

            JNI_interface_type_info const * info =
                static_cast< JNI_interface_type_info const * >(
                    bridge->getJniInfo()->get_type_info(
                        jni, &td->aBase ) );
            uno_Interface * pUnoI = bridge->map_to_uno( jni, javaI, info );
            if (nullptr != *ppUnoI)
            {
                uno_Interface * p = *ppUnoI;
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
    jobject * ppJavaI = reinterpret_cast<jobject *>(ppOut);
    uno_Interface * pUnoI = static_cast<uno_Interface *>(pIn);

    static_assert(sizeof (void *) == sizeof (jobject), "must be the same size");
    assert(ppJavaI != nullptr);
    assert(td != nullptr);

    try
    {
        if (nullptr == pUnoI)
        {
            if (nullptr != *ppJavaI)
            {
                Bridge const * bridge =
                    static_cast< Mapping const * >( mapping )->m_bridge;
                JNI_guarded_context jni(
                    bridge->getJniInfo(),
                    (static_cast<jni_uno::JniUnoEnvironmentData *>(
                        bridge->m_java_env->pContext)
                     ->machine));
                jni->DeleteGlobalRef( *ppJavaI );
                *ppJavaI = nullptr;
            }
        }
        else
        {
            Bridge const * bridge =
                static_cast< Mapping const * >( mapping )->m_bridge;
            JNI_guarded_context jni(
                bridge->getJniInfo(),
                (static_cast<jni_uno::JniUnoEnvironmentData *>(
                    bridge->m_java_env->pContext)
                 ->machine));

            JNI_interface_type_info const * info =
                static_cast< JNI_interface_type_info const * >(
                    bridge->getJniInfo()->get_type_info(
                        jni, &td->aBase ) );
            jobject jlocal = bridge->map_to_java( jni, pUnoI, info );
            if (nullptr != *ppJavaI)
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


void Bridge::acquire() const
{
    if (1 == osl_atomic_increment( &m_ref ))
    {
        if (m_registered_java2uno)
        {
            uno_Mapping * mapping = const_cast< Mapping * >( &m_java2uno );
            uno_registerMapping(
                &mapping, Bridge_free,
                m_java_env, &m_uno_env->aBase, nullptr );
        }
        else
        {
            uno_Mapping * mapping = const_cast< Mapping * >( &m_uno2java );
            uno_registerMapping(
                &mapping, Bridge_free,
                &m_uno_env->aBase, m_java_env, nullptr );
        }
    }
}


void Bridge::release() const
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
    assert(m_java_env != nullptr);
    assert(m_uno_env != nullptr);

    // uno_initEnvironment (below) cannot report errors directly, so it clears
    // its pContext upon error to indirectly report errors from here:
    if (static_cast<jni_uno::JniUnoEnvironmentData *>(m_java_env->pContext)
        == nullptr)
    {
        throw BridgeRuntimeError("error during JNI-UNO's uno_initEnvironment");
    }

    (*m_uno_env->aBase.acquire)( &m_uno_env->aBase );
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


Bridge::~Bridge()
{
    (*m_java_env->release)( m_java_env );
    (*m_uno_env->aBase.release)( &m_uno_env->aBase );
}

JNI_info const * Bridge::getJniInfo() const {
    return static_cast<jni_uno::JniUnoEnvironmentData *>(m_java_env->pContext)
        ->info;
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
    if (m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "cannot get class java.lang.Object!" + get_stack_trace() );
    }
    JLocalAutoRef jo_Object( *this, jo_class );
    // method Object.toString()
    jmethodID method_Object_toString = m_env->GetMethodID(
        static_cast<jclass>(jo_Object.get()), "toString", "()Ljava/lang/String;" );
    if (m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "cannot get method id of java.lang.Object.toString()!" +
            get_stack_trace() );
    }
    assert(method_Object_toString != nullptr);

    JLocalAutoRef jo_descr(
        *this, m_env->CallObjectMethodA(
            jo_exc.get(), method_Object_toString, nullptr ) );
    if (m_env->ExceptionCheck()) // no chance at all
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "error examining java exception object!" +
            get_stack_trace() );
    }

    jsize len = m_env->GetStringLength( static_cast<jstring>(jo_descr.get()) );
    std::unique_ptr< rtl_mem > ustr_mem(
        rtl_mem::allocate(
            sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
    rtl_uString * ustr = reinterpret_cast<rtl_uString *>(ustr_mem.get());
    m_env->GetStringRegion( static_cast<jstring>(jo_descr.get()), 0, len, ustr->buffer );
    if (m_env->ExceptionCheck())
    {
        m_env->ExceptionClear();
        throw BridgeRuntimeError(
            "invalid java string object!" + get_stack_trace() );
    }
    ustr->refCount = 1;
    ustr->length = len;
    ustr->buffer[ len ] = '\0';
    OUString message( reinterpret_cast<rtl_uString *>(ustr_mem.release()), SAL_NO_ACQUIRE );

    throw BridgeRuntimeError( message + get_stack_trace( jo_exc.get() ) );
}


void JNI_context::getClassForName(
    jclass * classClass, jmethodID * methodForName) const
{
    jclass c = m_env->FindClass("java/lang/Class");
    if (c != nullptr) {
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
    jclass c = nullptr;
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
            static_cast<jclass>(jo_JNI_proxy.get()), "get_stack_trace",
            "(Ljava/lang/Throwable;)Ljava/lang/String;" );
        if (assert_no_exception() && (nullptr != method))
        {
            jvalue arg;
            arg.l = jo_exc;
            JLocalAutoRef jo_stack_trace(
                *this, m_env->CallStaticObjectMethodA(
                    static_cast<jclass>(jo_JNI_proxy.get()), method, &arg ) );
            if (assert_no_exception())
            {
                jsize len =
                    m_env->GetStringLength( static_cast<jstring>(jo_stack_trace.get()) );
                std::unique_ptr< rtl_mem > ustr_mem(
                    rtl_mem::allocate(
                        sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ) );
                rtl_uString * ustr = reinterpret_cast<rtl_uString *>(ustr_mem.get());
                m_env->GetStringRegion(
                    static_cast<jstring>(jo_stack_trace.get()), 0, len, ustr->buffer );
                if (assert_no_exception())
                {
                    ustr->refCount = 1;
                    ustr->length = len;
                    ustr->buffer[ len ] = '\0';
                    return OUString(
                        reinterpret_cast<rtl_uString *>(ustr_mem.release()), SAL_NO_ACQUIRE );
                }
            }
        }
    }
    return OUString();
}

}

using namespace ::jni_uno;

extern "C" {

void SAL_CALL java_env_dispose(uno_Environment * env) {
    auto * envData
        = static_cast<jni_uno::JniUnoEnvironmentData *>(env->pContext);
    if (envData != nullptr) {
        jobject async;
        {
            osl::MutexGuard g(envData->mutex);
            async = envData->asynchronousFinalizer;
            envData->asynchronousFinalizer = nullptr;
        }
        if (async != nullptr) {
            try {
                JNI_guarded_context jni(envData->info, envData->machine);
                jni->CallObjectMethodA(
                    async, envData->info->m_method_AsynchronousFinalizer_drain,
                    nullptr);
                jni.ensure_no_exception();
                jni->DeleteGlobalRef(async);
            } catch (const BridgeRuntimeError & e) {
                SAL_WARN(
                    "bridges",
                    "ignoring BridgeRuntimeError \"" << e.m_message << "\"");
            } catch (
                jvmaccess::VirtualMachine::AttachGuard::CreationException &)
            {
                SAL_WARN(
                    "bridges",
                    ("ignoring jvmaccess::VirtualMachine::AttachGuard"
                     "::CreationException"));
            }
        }
    }
}

void SAL_CALL java_env_disposing(uno_Environment * env) {
    java_env_dispose(env);
    delete static_cast<jni_uno::JniUnoEnvironmentData *>(env->pContext);
}

#ifdef DISABLE_DYNLOADING
#define uno_initEnvironment java_uno_initEnvironment
#endif


SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_initEnvironment( uno_Environment * java_env )
    SAL_THROW_EXTERN_C()
{
    try {
        // JavaComponentLoader::getJavaLoader (in
        // stoc/source/javaloader/javaloader.cxx) stores a
        // jvmaccess::UnoVirtualMachine pointer into java_env->pContext; replace
        // it here with either a pointer to a full JniUnoEnvironmentData upon
        // success, or with a null pointer upon failure (as this function cannot
        // directly report back failure, so it uses that way to indirectly
        // report failure later from within the Bridge ctor):
        rtl::Reference<jvmaccess::UnoVirtualMachine> vm(
            static_cast<jvmaccess::UnoVirtualMachine *>(java_env->pContext));
        java_env->pContext = nullptr;
        java_env->dispose = java_env_dispose;
        java_env->environmentDisposing = java_env_disposing;
        java_env->pExtEnv = nullptr; // no extended support
        std::unique_ptr<jni_uno::JniUnoEnvironmentData> envData(
            new jni_uno::JniUnoEnvironmentData(vm));
        {
            JNI_guarded_context jni(envData->info, envData->machine);
            JLocalAutoRef ref(
                jni,
                jni->NewObject(
                    envData->info->m_class_AsynchronousFinalizer,
                    envData->info->m_ctor_AsynchronousFinalizer));
            jni.ensure_no_exception();
            envData->asynchronousFinalizer = jni->NewGlobalRef(ref.get());
            jni.ensure_no_exception();
        }
        java_env->pContext = envData.release();
    } catch (const BridgeRuntimeError & e) {
        SAL_WARN("bridges", "BridgeRuntimeError \"" << e.m_message << "\"");
    } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {
        SAL_WARN(
            "bridges",
            "jvmaccess::VirtualMachine::AttachGuard::CreationException");
    }
}

#ifdef DISABLE_DYNLOADING
#define uno_ext_getMapping java_uno_ext_getMapping
#endif


SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
    SAL_THROW_EXTERN_C()
{
    assert(ppMapping != nullptr);
    assert(pFrom != nullptr);
    assert(pTo != nullptr);
    if (nullptr != *ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = nullptr;
    }

    static_assert(int(JNI_FALSE) == int(sal_False), "must be equal");
    static_assert(int(JNI_TRUE) == int(sal_True), "must be equal");
    static_assert(sizeof (jboolean) == sizeof (sal_Bool), "must be the same size");
    static_assert(sizeof (jchar) == sizeof (sal_Unicode), "must be the same size");
    static_assert(sizeof (jdouble) == sizeof (double), "must be the same size");
    static_assert(sizeof (jfloat) == sizeof (float), "must be the same size");
    static_assert(sizeof (jbyte) == sizeof (sal_Int8), "must be the same size");
    static_assert(sizeof (jshort) == sizeof (sal_Int16), "must be the same size");
    static_assert(sizeof (jint) == sizeof (sal_Int32), "must be the same size");
    static_assert(sizeof (jlong) == sizeof (sal_Int64), "must be the same size");

    OUString const & from_env_typename =
        OUString::unacquired( &pFrom->pTypeName );
    OUString const & to_env_typename =
        OUString::unacquired( &pTo->pTypeName );

    uno_Mapping * mapping = nullptr;

    try
    {
        if ( from_env_typename == UNO_LB_JAVA && to_env_typename == UNO_LB_UNO )
        {
            Bridge * bridge =
                new Bridge( pFrom, pTo->pExtEnv, true ); // ref count = 1
            mapping = &bridge->m_java2uno;
            uno_registerMapping(
                &mapping, Bridge_free,
                pFrom, &pTo->pExtEnv->aBase, nullptr );
            // coverity[leaked_storage]
        }
        else if ( from_env_typename == UNO_LB_UNO && to_env_typename == UNO_LB_JAVA )
        {
            Bridge * bridge =
                new Bridge( pTo, pFrom->pExtEnv, false ); // ref count = 1
            mapping = &bridge->m_uno2java;
            uno_registerMapping(
                &mapping, Bridge_free,
                &pFrom->pExtEnv->aBase, pTo, nullptr );
            // coverity[leaked_storage]
        }
    }
    catch (const BridgeRuntimeError & err)
    {
        SAL_WARN("bridges", "BridgeRuntimeError \"" << err.m_message << "\"");
    }

    *ppMapping = mapping;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
