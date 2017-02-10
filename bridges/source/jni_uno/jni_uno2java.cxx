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

#include <sal/alloca.h>

#include "com/sun/star/uno/RuntimeException.hpp"

#include "rtl/ustrbuf.hxx"

#include "jni_bridge.h"
#include "jniunoenvironmentdata.hxx"

namespace
{
extern "C"
{


void SAL_CALL UNO_proxy_free( uno_ExtEnvironment * env, void * proxy )
    SAL_THROW_EXTERN_C();


void SAL_CALL UNO_proxy_acquire( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C();


void SAL_CALL UNO_proxy_release( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C();


void SAL_CALL UNO_proxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args[], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C();
}
}

namespace jni_uno
{


void Bridge::handle_java_exc(
    JNI_context const & jni,
    JLocalAutoRef const & jo_exc, uno_Any * uno_exc ) const
{
    assert( jo_exc.is() );
    if (! jo_exc.is())
    {
        throw BridgeRuntimeError(
            "java exception occurred, but no java exception available!?" +
            jni.get_stack_trace() );
    }

    JLocalAutoRef jo_class( jni, jni->GetObjectClass( jo_exc.get() ) );
    JLocalAutoRef jo_class_name(
        jni, jni->CallObjectMethodA(
            jo_class.get(), getJniInfo()->m_method_Class_getName, nullptr ) );
    jni.ensure_no_exception();
    OUString exc_name(
        jstring_to_oustring( jni, static_cast<jstring>(jo_class_name.get()) ) );

    ::com::sun::star::uno::TypeDescription td( exc_name.pData );
    if (!td.is() || (typelib_TypeClass_EXCEPTION != td.get()->eTypeClass))
    {
        // call toString()
        JLocalAutoRef jo_descr(
            jni, jni->CallObjectMethodA(
                jo_exc.get(), getJniInfo()->m_method_Object_toString, nullptr ) );
        jni.ensure_no_exception();
        throw BridgeRuntimeError(
            "non-UNO exception occurred: "
            + jstring_to_oustring( jni, static_cast<jstring>(jo_descr.get()) )
            + jni.get_stack_trace( jo_exc.get() ) );
    }

    std::unique_ptr< rtl_mem > uno_data( rtl_mem::allocate( td.get()->nSize ) );
    jvalue val;
    val.l = jo_exc.get();
    map_to_uno(
        jni, uno_data.get(), val, td.get()->pWeakRef, nullptr,
        false /* no assign */, false /* no out param */ );

#if OSL_DEBUG_LEVEL > 0
    // patch Message, append stack trace
    reinterpret_cast< ::com::sun::star::uno::Exception * >(
        uno_data.get() )->Message += jni.get_stack_trace( jo_exc.get() );
#endif

    typelib_typedescriptionreference_acquire( td.get()->pWeakRef );
    uno_exc->pType = td.get()->pWeakRef;
    uno_exc->pData = uno_data.release();

    SAL_INFO(
        "bridges",
        "exception occurred uno->java: [" << exc_name << "] "
        << (static_cast<css::uno::Exception const *>(uno_exc->pData)
            ->Message));
}


void Bridge::call_java(
    jobject javaI, typelib_InterfaceTypeDescription * iface_td,
    sal_Int32 local_member_index, sal_Int32 function_pos_offset,
    typelib_TypeDescriptionReference * return_type,
    typelib_MethodParameter * params, sal_Int32 nParams,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const
{
    assert( function_pos_offset == 0 || function_pos_offset == 1 );

    JNI_guarded_context jni(
        getJniInfo(),
        static_cast<JniUnoEnvironmentData *>(m_java_env->pContext)->machine);

    // assure fully initialized iface_td:
    ::com::sun::star::uno::TypeDescription iface_holder;
    if (! iface_td->aBase.bComplete) {
        iface_holder = ::com::sun::star::uno::TypeDescription(
            reinterpret_cast<typelib_TypeDescription *>(iface_td) );
        iface_holder.makeComplete();
        if (! iface_holder.get()->bComplete) {
            throw BridgeRuntimeError(
                "cannot make type complete: "
                + OUString::unacquired(&iface_holder.get()->pTypeName)
                + jni.get_stack_trace() );
        }
        iface_td = reinterpret_cast<typelib_InterfaceTypeDescription *>(
            iface_holder.get() );
        assert( iface_td->aBase.eTypeClass == typelib_TypeClass_INTERFACE );
    }

    // prepare java args, save param td
    jvalue * java_args = static_cast<jvalue *>(alloca( sizeof (jvalue) * nParams ));

    sal_Int32 nPos;
    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        try
        {
            typelib_MethodParameter const & param = params[ nPos ];
            java_args[ nPos ].l = nullptr; // if out: build up array[ 1 ]
            map_to_java(
                jni, &java_args[ nPos ],
                uno_args[ nPos ],
                param.pTypeRef, nullptr,
                param.bIn /* convert uno value */,
                param.bOut /* build up array[ 1 ] */ );
        }
        catch (...)
        {
            // cleanup
            for ( sal_Int32 n = 0; n < nPos; ++n )
            {
                typelib_MethodParameter const & param = params[ n ];
                if (param.bOut ||
                    typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
                {
                    jni->DeleteLocalRef( java_args[ n ].l );
                }
            }
            throw;
        }
    }

    sal_Int32 base_members = iface_td->nAllMembers - iface_td->nMembers;
    assert( base_members < iface_td->nAllMembers );
    sal_Int32 base_members_function_pos =
        iface_td->pMapMemberIndexToFunctionIndex[ base_members ];
    sal_Int32 member_pos = base_members + local_member_index;
    SAL_WARN_IF(
        member_pos >= iface_td->nAllMembers, "bridges",
        "member pos out of range");
    sal_Int32 function_pos =
        iface_td->pMapMemberIndexToFunctionIndex[ member_pos ]
        + function_pos_offset;
    SAL_WARN_IF(
        (function_pos < base_members_function_pos
         || function_pos >= iface_td->nMapFunctionIndexToMemberIndex),
        "bridges", "illegal function index");
    function_pos -= base_members_function_pos;

    JNI_interface_type_info const * info =
        static_cast< JNI_interface_type_info const * >(
            getJniInfo()->get_type_info( jni, &iface_td->aBase ) );
    jmethodID method_id = info->m_methods[ function_pos ];

#if OSL_DEBUG_LEVEL > 0
    OUStringBuffer trace_buf( 128 );
    trace_buf.append( "calling " );
    JLocalAutoRef jo_method(
        jni, jni->ToReflectedMethod( info->m_class, method_id, JNI_FALSE ) );
    jni.ensure_no_exception();
    JLocalAutoRef jo_descr(
        jni, jni->CallObjectMethodA(
            jo_method.get(), getJniInfo()->m_method_Object_toString, nullptr ) );
    jni.ensure_no_exception();
    trace_buf.append( jstring_to_oustring( jni, static_cast<jstring>(jo_descr.get()) ) );
    trace_buf.append( " on " );
    jo_descr.reset(
        jni->CallObjectMethodA(
            javaI, getJniInfo()->m_method_Object_toString, nullptr ) );
    jni.ensure_no_exception();
    trace_buf.append( jstring_to_oustring( jni, static_cast<jstring>(jo_descr.get()) ) );
    trace_buf.append( " (" );
    JLocalAutoRef jo_class( jni, jni->GetObjectClass( javaI ) );
    jo_descr.reset(
        jni->CallObjectMethodA(
            jo_class.get(), getJniInfo()->m_method_Object_toString, nullptr ) );
    jni.ensure_no_exception();
    trace_buf.append( jstring_to_oustring( jni, static_cast<jstring>(jo_descr.get()) ) );
    trace_buf.append( ")" );
    SAL_INFO("bridges", trace_buf.makeStringAndClear());
#endif

    // complex return value
    JLocalAutoRef java_ret( jni );

    switch (return_type->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        jni->CallVoidMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_CHAR:
        *static_cast<sal_Unicode *>(uno_ret) =
            jni->CallCharMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_BOOLEAN:
        *static_cast<sal_Bool *>(uno_ret) =
            jni->CallBooleanMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_BYTE:
        *static_cast<sal_Int8 *>(uno_ret) =
            jni->CallByteMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *static_cast<sal_Int16 *>(uno_ret) =
            jni->CallShortMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *static_cast<sal_Int32 *>(uno_ret) =
            jni->CallIntMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *static_cast<sal_Int64 *>(uno_ret) =
            jni->CallLongMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_FLOAT:
        *static_cast<float *>(uno_ret) =
            jni->CallFloatMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_DOUBLE:
        *static_cast<double *>(uno_ret) =
            jni->CallDoubleMethodA( javaI, method_id, java_args );
        break;
    default:
        java_ret.reset(
            jni->CallObjectMethodA( javaI, method_id, java_args ) );
        break;
    }

    if (jni->ExceptionCheck())
    {
        JLocalAutoRef jo_exc( jni, jni->ExceptionOccurred() );
        jni->ExceptionClear();

        // release temp java local refs
        for ( nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = params[ nPos ];
            if (param.bOut ||
                typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
            {
                jni->DeleteLocalRef( java_args[ nPos ].l );
            }
        }

        handle_java_exc( jni, jo_exc, *uno_exc );
    }
    else // no exception
    {
        for ( nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = params[ nPos ];
            if (param.bOut)
            {
                try
                {
                    map_to_uno(
                        jni, uno_args[ nPos ],
                        java_args[ nPos ], param.pTypeRef, nullptr,
                        param.bIn /* assign if inout */,
                        true /* out param */ );
                }
                catch (...)
                {
                    // cleanup uno pure out
                    for ( sal_Int32 n = 0; n < nPos; ++n )
                    {
                        typelib_MethodParameter const & p = params[ n ];
                        if (! p.bIn)
                        {
                            uno_type_destructData(
                                uno_args[ n ], p.pTypeRef, nullptr );
                        }
                    }
                    // cleanup java temp local refs
                    for ( ; nPos < nParams; ++nPos )
                    {
                        typelib_MethodParameter const & p = params[ nPos ];
                        if (p.bOut ||
                            typelib_TypeClass_DOUBLE <
                              p.pTypeRef->eTypeClass)
                        {
                            jni->DeleteLocalRef( java_args[ nPos ].l );
                        }
                    }
                    throw;
                }
                jni->DeleteLocalRef( java_args[ nPos ].l );
            }
            else // pure temp in param
            {
                if (typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
                    jni->DeleteLocalRef( java_args[ nPos ].l );
            }
        }

        // return value
        if (typelib_TypeClass_DOUBLE < return_type->eTypeClass)
        {
            try
            {
                jvalue val;
                val.l = java_ret.get();
                map_to_uno(
                    jni, uno_ret, val, return_type, nullptr,
                    false /* no assign */, false /* no out param */ );
            }
            catch (...)
            {
                // cleanup uno pure out
                for ( sal_Int32 i = 0; i < nParams; ++i )
                {
                    typelib_MethodParameter const & param = params[ i ];
                    if (! param.bIn)
                    {
                        uno_type_destructData(
                            uno_args[ i ], param.pTypeRef, nullptr );
                    }
                }
                throw;
            }
        } // else: already set integral uno return value

        // no exception occurred
        *uno_exc = nullptr;
    }
}

// an UNO proxy wrapping a Java interface
struct UNO_proxy : public uno_Interface
{
    mutable oslInterlockedCount         m_ref;
    Bridge const *                      m_bridge;

    // mapping information
    jobject                             m_javaI;
    jstring                             m_jo_oid;
    OUString                            m_oid;
    JNI_interface_type_info const *     m_type_info;

    inline void acquire() const;
    inline void release() const;

    // ctor
    inline UNO_proxy(
        JNI_context const & jni, Bridge const * bridge,
        jobject javaI, jstring jo_oid, OUString const & oid,
        JNI_interface_type_info const * info );
};


inline UNO_proxy::UNO_proxy(
    JNI_context const & jni, Bridge const * bridge,
    jobject javaI, jstring jo_oid, OUString const & oid,
    JNI_interface_type_info const * info )
    : m_ref( 1 ),
      m_oid( oid ),
      m_type_info( info )
{
    JNI_info const * jni_info = bridge->getJniInfo();
    JLocalAutoRef jo_string_array(
        jni, jni->NewObjectArray( 1, jni_info->m_class_String, jo_oid ) );
    jni.ensure_no_exception();
    jvalue args[ 3 ];
    args[ 0 ].l = javaI;
    args[ 1 ].l = jo_string_array.get();
    args[ 2 ].l = info->m_type;
    jobject jo_iface = jni->CallObjectMethodA(
        jni_info->m_object_java_env,
        jni_info->m_method_IEnvironment_registerInterface, args );
    jni.ensure_no_exception();

    m_javaI = jni->NewGlobalRef( jo_iface );
    m_jo_oid = static_cast<jstring>(jni->NewGlobalRef( jo_oid ));
    bridge->acquire();
    m_bridge = bridge;

    // uno_Interface
    uno_Interface::acquire = UNO_proxy_acquire;
    uno_Interface::release = UNO_proxy_release;
    uno_Interface::pDispatcher = UNO_proxy_dispatch;
}


inline void UNO_proxy::acquire() const
{
    if (1 == osl_atomic_increment( &m_ref ))
    {
        // rebirth of proxy zombie
        void * that = const_cast< UNO_proxy * >( this );
        // register at uno env
        (*m_bridge->m_uno_env->registerProxyInterface)(
            m_bridge->m_uno_env, &that,
            UNO_proxy_free, m_oid.pData,
            reinterpret_cast<typelib_InterfaceTypeDescription *>(m_type_info->m_td.get()) );
        assert( this == that );
    }
}


inline void UNO_proxy::release() const
{
    if (0 == osl_atomic_decrement( &m_ref ))
    {
        // revoke from uno env on last release
        (*m_bridge->m_uno_env->revokeInterface)(
            m_bridge->m_uno_env, const_cast< UNO_proxy * >( this ) );
    }
}


uno_Interface * Bridge::map_to_uno(
    JNI_context const & jni,
    jobject javaI, JNI_interface_type_info const * info ) const
{
    JLocalAutoRef jo_oid( jni, compute_oid( jni, javaI ) );
    OUString oid( jstring_to_oustring( jni, static_cast<jstring>(jo_oid.get()) ) );

    uno_Interface * pUnoI = nullptr;
    (*m_uno_env->getRegisteredInterface)(
        m_uno_env, reinterpret_cast<void **>(&pUnoI),
        oid.pData, reinterpret_cast<typelib_InterfaceTypeDescription *>(info->m_td.get()) );

    if (nullptr == pUnoI) // no existing interface, register new proxy
    {
        // refcount initially 1
        pUnoI = new UNO_proxy(
            jni, this,
            javaI, static_cast<jstring>(jo_oid.get()), oid, info );

        (*m_uno_env->registerProxyInterface)(
            m_uno_env, reinterpret_cast<void **>(&pUnoI),
            UNO_proxy_free,
            oid.pData, reinterpret_cast<typelib_InterfaceTypeDescription *>(info->m_td.get()) );
    }
    return pUnoI;
}

}

using namespace ::jni_uno;

namespace
{
extern "C"
{


void SAL_CALL UNO_proxy_free( uno_ExtEnvironment * env, void * proxy )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy * that = static_cast< UNO_proxy * >( proxy );
    Bridge const * bridge = that->m_bridge;

    assert(env == bridge->m_uno_env); (void) env;
    SAL_INFO("bridges", "freeing binary uno proxy: " << that->m_oid);

    try
    {
        JNI_guarded_context jni(
            bridge->getJniInfo(),
            (static_cast<JniUnoEnvironmentData *>(bridge->m_java_env->pContext)
             ->machine));

        jni->DeleteGlobalRef( that->m_javaI );
        jni->DeleteGlobalRef( that->m_jo_oid );
    }
    catch (BridgeRuntimeError & err)
    {
        SAL_WARN(
            "bridges",
            "ignoring BridgeRuntimeError \"" << err.m_message << "\"");
    }
    catch (::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        SAL_WARN("bridges", "attaching current thread to java failed");
    }

    bridge->release();
#if OSL_DEBUG_LEVEL > 0
    *reinterpret_cast<int *>(that) = 0xdeadcafe;
#endif
    delete that;
}


void SAL_CALL UNO_proxy_acquire( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = static_cast< UNO_proxy const * >( pUnoI );
    that->acquire();
}


void SAL_CALL UNO_proxy_release( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = static_cast< UNO_proxy const * >( pUnoI );
    that->release();
}


void SAL_CALL UNO_proxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = static_cast< UNO_proxy const * >( pUnoI );
    Bridge const * bridge = that->m_bridge;

    SAL_INFO(
        "bridges",
        "uno->java call: " << OUString::unacquired(&member_td->pTypeName)
            << " on oid " << that->m_oid);

    try
    {
        switch (member_td->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_InterfaceAttributeTypeDescription const * attrib_td =
                reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription const * >(
                    member_td );
            com::sun::star::uno::TypeDescription attrib_holder;
            while ( attrib_td->pBaseRef != nullptr ) {
                attrib_holder = com::sun::star::uno::TypeDescription(
                    attrib_td->pBaseRef );
                assert(
                    attrib_holder.get()->eTypeClass
                    == typelib_TypeClass_INTERFACE_ATTRIBUTE );
                attrib_td = reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        attrib_holder.get() );
            }
            typelib_InterfaceTypeDescription * iface_td = attrib_td->pInterface;

            if (nullptr == uno_ret) // is setter method
            {
                typelib_MethodParameter param;
                param.pTypeRef = attrib_td->pAttributeTypeRef;
                param.bIn = true;
                param.bOut = false;

                bridge->call_java(
                    that->m_javaI, iface_td,
                    attrib_td->nIndex, 1, // get, then set method
                    bridge->getJniInfo()->m_void_type.getTypeLibType(),
                    &param, 1,
                    nullptr, uno_args, uno_exc );
            }
            else // is getter method
            {
                bridge->call_java(
                    that->m_javaI, iface_td, attrib_td->nIndex, 0,
                    attrib_td->pAttributeTypeRef,
                    nullptr, 0, // no params
                    uno_ret, nullptr, uno_exc );
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription const * method_td =
                reinterpret_cast<
                typelib_InterfaceMethodTypeDescription const * >(
                    member_td );
            com::sun::star::uno::TypeDescription method_holder;
            while ( method_td->pBaseRef != nullptr ) {
                method_holder = com::sun::star::uno::TypeDescription(
                    method_td->pBaseRef );
                assert(
                    method_holder.get()->eTypeClass
                    == typelib_TypeClass_INTERFACE_METHOD );
                method_td = reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(
                        method_holder.get() );
            }
            typelib_InterfaceTypeDescription * iface_td = method_td->pInterface;

            switch ( method_td->aBase.nPosition )
            {
            case 0: // queryInterface()
            {
                TypeDescr demanded_td(
                    *static_cast< typelib_TypeDescriptionReference ** >(
                        uno_args[ 0 ] ) );
                if (typelib_TypeClass_INTERFACE !=
                      demanded_td.get()->eTypeClass)
                {
                    throw BridgeRuntimeError(
                        "queryInterface() call demands an INTERFACE type!" );
                }

                uno_Interface * pInterface = nullptr;
                (*bridge->m_uno_env->getRegisteredInterface)(
                    bridge->m_uno_env,
                    reinterpret_cast<void **>(&pInterface), that->m_oid.pData,
                    reinterpret_cast<typelib_InterfaceTypeDescription *>(demanded_td.get()) );

                if (nullptr == pInterface)
                {
                    JNI_info const * jni_info = bridge->getJniInfo();
                    JNI_guarded_context jni(
                        jni_info,
                        (static_cast<JniUnoEnvironmentData *>(
                            bridge->m_java_env->pContext)
                         ->machine));

                    JNI_interface_type_info const * info =
                        static_cast< JNI_interface_type_info const * >(
                            jni_info->get_type_info( jni, demanded_td.get() ) );

                    jvalue args[ 2 ];
                    args[ 0 ].l = info->m_type;
                    args[ 1 ].l = that->m_javaI;

                    JLocalAutoRef jo_ret(
                        jni, jni->CallStaticObjectMethodA(
                            jni_info->m_class_UnoRuntime,
                            jni_info->m_method_UnoRuntime_queryInterface,
                            args ) );

                    if (jni->ExceptionCheck())
                    {
                        JLocalAutoRef jo_exc( jni, jni->ExceptionOccurred() );
                        jni->ExceptionClear();
                        bridge->handle_java_exc( jni, jo_exc, *uno_exc );
                    }
                    else
                    {
                        if (jo_ret.is())
                        {
                            SAL_WARN_IF(
                                (jstring_to_oustring(
                                    jni,
                                    static_cast<jstring>(
                                        JLocalAutoRef(
                                            jni, compute_oid(jni, jo_ret.get()))
                                        .get()))
                                 != that->m_oid),
                                "bridges", "different oids");
                            // refcount initially 1
                            uno_Interface * pUnoI2 = new UNO_proxy(
                                jni, bridge, jo_ret.get(),
                                that->m_jo_oid, that->m_oid, info );

                            (*bridge->m_uno_env->registerProxyInterface)(
                                bridge->m_uno_env,
                                reinterpret_cast<void **>(&pUnoI2),
                                UNO_proxy_free, that->m_oid.pData,
                                reinterpret_cast<
                                  typelib_InterfaceTypeDescription * >(
                                      info->m_td.get() ) );

                            uno_any_construct(
                                static_cast<uno_Any *>(uno_ret), &pUnoI2,
                                demanded_td.get(), nullptr );
                            (*pUnoI2->release)( pUnoI2 );
                        }
                        else // object does not support demanded interface
                        {
                            uno_any_construct(
                                static_cast< uno_Any * >( uno_ret ),
                                nullptr, nullptr, nullptr );
                        }
                        // no exception occurred
                        *uno_exc = nullptr;
                    }
                }
                else
                {
                    uno_any_construct(
                        static_cast< uno_Any * >( uno_ret ),
                        &pInterface, demanded_td.get(), nullptr );
                    (*pInterface->release)( pInterface );
                    *uno_exc = nullptr;
                }
                break;
            }
            case 1: // acquire this proxy
                that->acquire();
                *uno_exc = nullptr;
                break;
            case 2: // release this proxy
                that->release();
                *uno_exc = nullptr;
                break;
            default: // arbitrary method call
                bridge->call_java(
                    that->m_javaI, iface_td, method_td->nIndex, 0,
                    method_td->pReturnTypeRef,
                    method_td->pParams, method_td->nParams,
                    uno_ret, uno_args, uno_exc );
                break;
            }
            break;
        }
        default:
        {
            throw BridgeRuntimeError(
                "illegal member type description!" );
        }
        }
    }
    catch (BridgeRuntimeError & err)
    {
        OUStringBuffer buf( 128 );
        buf.append( "[jni_uno bridge error] UNO calling Java method " );
        if (typelib_TypeClass_INTERFACE_METHOD == member_td->eTypeClass ||
            typelib_TypeClass_INTERFACE_ATTRIBUTE == member_td->eTypeClass)
        {
            buf.append( OUString::unacquired(
                            &reinterpret_cast<
                            typelib_InterfaceMemberTypeDescription const * >(
                                member_td )->pMemberName ) );
        }
        buf.append( ": " );
        buf.append( err.m_message );
        // binary identical struct
        ::com::sun::star::uno::RuntimeException exc(
            buf.makeStringAndClear(),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
        ::com::sun::star::uno::Type const & exc_type = cppu::UnoType<decltype(exc)>::get();
        uno_type_any_construct( *uno_exc, &exc, exc_type.getTypeLibType(), nullptr );
        SAL_INFO("bridges", exc.Message);
    }
    catch (::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        // binary identical struct
        ::com::sun::star::uno::RuntimeException exc(
            "[jni_uno bridge error] attaching current thread to java failed!",
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
        ::com::sun::star::uno::Type const & exc_type = cppu::UnoType<decltype(exc)>::get();
        uno_type_any_construct( *uno_exc, &exc, exc_type.getTypeLibType(), nullptr );
        SAL_WARN("bridges", exc.Message);
    }
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
