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

#include <algorithm>
#include <cassert>

#include <sal/alloca.h>

#include "jni_bridge.h"
#include "jniunoenvironmentdata.hxx"

namespace jni_uno
{


jobject Bridge::map_to_java(
    JNI_context const & jni,
    uno_Interface * pUnoI, JNI_interface_type_info const * info ) const
{
    // get oid
    rtl_uString * pOid = nullptr;
    (*m_uno_env->getObjectIdentifier)( m_uno_env, &pOid, pUnoI );
    assert( nullptr != pOid );
    OUString oid( pOid, SAL_NO_ACQUIRE );

    // opt getRegisteredInterface()
    JLocalAutoRef jo_oid( jni, ustring_to_jstring( jni, oid.pData ) );
    jvalue args[ 2 ];
    args[ 0 ].l = jo_oid.get();
    args[ 1 ].l = info->m_type;
    jobject jo_iface = jni->CallObjectMethodA(
        getJniInfo()->m_object_java_env,
        getJniInfo()->m_method_IEnvironment_getRegisteredInterface, args );
    jni.ensure_no_exception();

    if (nullptr == jo_iface) // no registered iface
    {
        // register uno interface
        (*m_uno_env->registerInterface)(
            m_uno_env, reinterpret_cast< void ** >( &pUnoI ),
            oid.pData, reinterpret_cast<typelib_InterfaceTypeDescription *>(info->m_td.get()) );

        // create java and register java proxy
        jvalue args2[ 8 ];
        acquire();
        args2[ 0 ].j = reinterpret_cast< sal_Int64 >( this );
        (*pUnoI->acquire)( pUnoI );
        args2[ 1 ].l = getJniInfo()->m_object_java_env;
        args2[ 2 ].j = reinterpret_cast< sal_Int64 >( pUnoI );
        typelib_typedescription_acquire( info->m_td.get() );
        args2[ 3 ].j = reinterpret_cast< sal_Int64 >( info->m_td.get() );
        args2[ 4 ].l = info->m_type;
        args2[ 5 ].l = jo_oid.get();
        args2[ 6 ].l = info->m_proxy_ctor;
        auto * envData = static_cast<jni_uno::JniUnoEnvironmentData *>(
            m_java_env->pContext);
        {
            osl::MutexGuard g(envData->mutex);
            args2[ 7 ].l = envData->asynchronousFinalizer;
        }
        jo_iface = jni->CallStaticObjectMethodA(
            getJniInfo()->m_class_JNI_proxy,
            getJniInfo()->m_method_JNI_proxy_create, args2 );
        jni.ensure_no_exception();
    }

    assert( nullptr != jo_iface );
    return jo_iface;
}


void Bridge::handle_uno_exc( JNI_context const & jni, uno_Any * uno_exc ) const
{
    if (typelib_TypeClass_EXCEPTION == uno_exc->pType->eTypeClass)
    {
#if OSL_DEBUG_LEVEL > 0
        // append java stack trace to Message member
        static_cast< ::com::sun::star::uno::Exception * >(
            uno_exc->pData )->Message += jni.get_stack_trace();
#endif
        SAL_INFO(
            "bridges",
            "exception occurred java->uno: ["
                << OUString::unacquired(&uno_exc->pType->pTypeName) << "] "
                << static_cast<css::uno::Exception const *>(
                    uno_exc->pData)->Message);
        // signal exception
        jvalue java_exc;
        try
        {
            map_to_java(
                jni, &java_exc, uno_exc->pData, uno_exc->pType, nullptr,
                true /* in */, false /* no out */ );
        }
        catch (...)
        {
            uno_any_destruct( uno_exc, nullptr );
            throw;
        }
        uno_any_destruct( uno_exc, nullptr );

        JLocalAutoRef jo_exc( jni, java_exc.l );
        jint res = jni->Throw( static_cast<jthrowable>(jo_exc.get()) );
        if (0 != res)
        {
            // call toString()
            JLocalAutoRef jo_descr(
                jni, jni->CallObjectMethodA(
                    jo_exc.get(), getJniInfo()->m_method_Object_toString, nullptr ) );
            jni.ensure_no_exception();
            throw BridgeRuntimeError(
                "throwing java exception failed: "
                + jstring_to_oustring( jni, static_cast<jstring>(jo_descr.get()) )
                + jni.get_stack_trace() );
        }
    }
    else
    {
        OUString message(
            "thrown exception is no uno exception: " +
            OUString::unacquired( &uno_exc->pType->pTypeName ) +
            jni.get_stack_trace() );
        uno_any_destruct( uno_exc, nullptr );
        throw BridgeRuntimeError( message );
    }
}

union largest
{
    sal_Int64 n;
    double d;
    void * p;
    uno_Any a;
};


jobject Bridge::call_uno(
    JNI_context const & jni,
    uno_Interface * pUnoI, typelib_TypeDescription * member_td,
    typelib_TypeDescriptionReference * return_type,
    sal_Int32 nParams, typelib_MethodParameter const * pParams,
    jobjectArray jo_args /* may be 0 */ ) const
{
    // return mem
    sal_Int32 return_size;
    switch (return_type->eTypeClass) {
    case typelib_TypeClass_VOID:
        return_size = 0;
        break;

    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        return_size = std::max(
            TypeDescr(return_type).get()->nSize,
            static_cast< sal_Int32 >(sizeof (largest)));
        break;

    default:
        return_size = sizeof (largest);
        break;
    }

    char * mem = static_cast<char *>(alloca(
        (nParams * sizeof (void *)) +
        return_size + (nParams * sizeof (largest)) ));
    void ** uno_args = reinterpret_cast<void **>(mem);
    void * uno_ret = return_size == 0 ? nullptr : (mem + (nParams * sizeof (void *)));
    largest * uno_args_mem = reinterpret_cast<largest *>
        (mem + (nParams * sizeof (void *)) + return_size);

    assert( (0 == nParams) || (nParams == jni->GetArrayLength( jo_args )) );
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        typelib_MethodParameter const & param = pParams[ nPos ];
        typelib_TypeDescriptionReference * type = param.pTypeRef;

        uno_args[ nPos ] = &uno_args_mem[ nPos ];
        if (typelib_TypeClass_STRUCT == type->eTypeClass ||
            typelib_TypeClass_EXCEPTION == type->eTypeClass)
        {
            TypeDescr td( type );
            if (sal::static_int_cast< sal_uInt32 >(td.get()->nSize)
                > sizeof (largest))
                uno_args[ nPos ] = alloca( td.get()->nSize );
        }

        if (param.bIn)
        {
            try
            {
                JLocalAutoRef jo_arg(
                    jni, jni->GetObjectArrayElement( jo_args, nPos ) );
                jni.ensure_no_exception();
                jvalue java_arg;
                java_arg.l = jo_arg.get();
                map_to_uno(
                    jni, uno_args[ nPos ], java_arg, type, nullptr,
                    false /* no assign */, param.bOut,
                    true /* special wrapped integral types */ );
            }
            catch (...)
            {
                // cleanup uno in args
                for ( sal_Int32 n = 0; n < nPos; ++n )
                {
                    typelib_MethodParameter const & p = pParams[ n ];
                    if (p.bIn)
                    {
                        uno_type_destructData(
                            uno_args[ n ], p.pTypeRef, nullptr );
                    }
                }
                throw;
            }
        }
    }

    uno_Any uno_exc_holder;
    uno_Any * uno_exc = &uno_exc_holder;
    // call binary uno
    (*pUnoI->pDispatcher)( pUnoI, member_td, uno_ret, uno_args, &uno_exc );

    if (nullptr == uno_exc)
    {
        // convert out args; destruct uno args
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = pParams[ nPos ];
            typelib_TypeDescriptionReference * type = param.pTypeRef;
            if (param.bOut)
            {
                try
                {
                    // get out holder array[ 1 ]
                    JLocalAutoRef jo_out_holder(
                        jni, jni->GetObjectArrayElement( jo_args, nPos ) );
                    jni.ensure_no_exception();
                    jvalue java_arg;
                    java_arg.l = jo_out_holder.get();
                    map_to_java(
                        jni, &java_arg, uno_args[ nPos ], type, nullptr,
                        true /* in */, true /* out holder */ );
                }
                catch (...)
                {
                    // cleanup further uno args
                    for ( sal_Int32 n = nPos; n < nParams; ++n )
                    {
                        uno_type_destructData(
                            uno_args[ n ], pParams[ n ].pTypeRef, nullptr );
                    }
                    // cleanup uno return value
                    uno_type_destructData( uno_ret, return_type, nullptr );
                    throw;
                }
            }
            if (typelib_TypeClass_DOUBLE < type->eTypeClass &&
                typelib_TypeClass_ENUM != type->eTypeClass) // opt
            {
                uno_type_destructData( uno_args[ nPos ], type, nullptr );
            }
        }

        if (typelib_TypeClass_VOID != return_type->eTypeClass)
        {
            // convert uno return value
            jvalue java_ret;
            try
            {
                map_to_java(
                    jni, &java_ret, uno_ret, return_type, nullptr,
                    true /* in */, false /* no out */,
                    true /* special_wrapped_integral_types */ );
            }
            catch (...)
            {
                uno_type_destructData( uno_ret, return_type, nullptr );
                throw;
            }
            if (typelib_TypeClass_DOUBLE < return_type->eTypeClass &&
                typelib_TypeClass_ENUM != return_type->eTypeClass) // opt
            {
                uno_type_destructData( uno_ret, return_type, nullptr );
            }
            return java_ret.l;
        }
        return nullptr; // void return
    }
    else // exception occurred
    {
        // destruct uno in args
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = pParams[ nPos ];
            if (param.bIn)
                uno_type_destructData( uno_args[ nPos ], param.pTypeRef, nullptr );
        }

        handle_uno_exc( jni, uno_exc );
        return nullptr;
    }
}

}

using namespace ::jni_uno;

extern "C"
{


SAL_JNI_EXPORT jobject
JNICALL Java_com_sun_star_bridges_jni_1uno_JNI_1proxy_dispatch_1call(
    JNIEnv * jni_env, jobject jo_proxy, jlong bridge_handle, jstring jo_method,
    jobjectArray jo_args /* may be 0 */ )
{
    Bridge const * bridge = reinterpret_cast< Bridge const * >( bridge_handle );
    JNI_info const * jni_info = bridge->getJniInfo();
    JNI_context jni(
        jni_info, jni_env,
        static_cast< jobject >(
            static_cast<JniUnoEnvironmentData *>(bridge->m_java_env->pContext)
            ->machine->getClassLoader()));

    OUString method_name;

    try
    {
        method_name = jstring_to_oustring( jni, jo_method );
        SAL_INFO(
            "bridges",
            "java->uno call: " << method_name << " on oid "
            << jstring_to_oustring(
                jni,
                static_cast<jstring>(
                    JLocalAutoRef(
                        jni,
                        jni->GetObjectField(
                            jo_proxy, jni_info->m_field_JNI_proxy_m_oid))
                    .get())));
        // special IQueryInterface.queryInterface()
        if ( method_name == "queryInterface" )
        {
            // oid
            JLocalAutoRef jo_oid(
                jni, jni->GetObjectField(
                    jo_proxy, jni_info->m_field_JNI_proxy_m_oid ) );
            // type
            JLocalAutoRef jo_type(
                jni, jni->GetObjectArrayElement( jo_args, 0 ) );
            jni.ensure_no_exception();

            JLocalAutoRef jo_type_name(
                jni, jni->GetObjectField(
                    jo_type.get(), jni_info->m_field_Type_typeName ) );
            if (! jo_type_name.is())
            {
                throw BridgeRuntimeError(
                    "incomplete type object: no type name!" +
                    jni.get_stack_trace() );
            }
            OUString type_name(
                jstring_to_oustring( jni, static_cast<jstring>(jo_type_name.get()) ) );
            JNI_type_info const * info =
                jni_info->get_type_info( jni, type_name );
            if (typelib_TypeClass_INTERFACE != info->m_td.get()->eTypeClass)
            {
                throw BridgeRuntimeError(
                    "queryInterface() call demands an INTERFACE type!" );
            }
            JNI_interface_type_info const * iface_info =
                static_cast< JNI_interface_type_info const * >( info );

            // getRegisteredInterface() already tested in JNI_proxy:
            // perform queryInterface call on binary uno interface
            uno_Interface * pUnoI = reinterpret_cast< uno_Interface * >(
                jni->GetLongField(
                    jo_proxy, jni_info->m_field_JNI_proxy_m_receiver_handle ) );

            uno_Any uno_ret;
            void * uno_args[] = { &iface_info->m_td.get()->pWeakRef };
            uno_Any uno_exc_holder;
            uno_Any * uno_exc = &uno_exc_holder;
            // call binary uno
            (*pUnoI->pDispatcher)(
                pUnoI, jni_info->m_XInterface_queryInterface_td.get(),
                &uno_ret, uno_args, &uno_exc );
            if (nullptr == uno_exc)
            {
                jobject jo_ret = nullptr;
                if (typelib_TypeClass_INTERFACE == uno_ret.pType->eTypeClass)
                {
                    uno_Interface * pUnoRet =
                        static_cast<uno_Interface *>(uno_ret.pReserved);
                    if (nullptr != pUnoRet)
                    {
                        try
                        {
                            jo_ret =
                                bridge->map_to_java( jni, pUnoRet, iface_info );
                        }
                        catch (...)
                        {
                            uno_any_destruct( &uno_ret, nullptr );
                            throw;
                        }
                    }
                }
                uno_any_destruct( &uno_ret, nullptr );
                return jo_ret;
            }
            else
            {
                bridge->handle_uno_exc( jni, uno_exc );
                return nullptr;
            }
        }

        typelib_InterfaceTypeDescription * td =
            reinterpret_cast< typelib_InterfaceTypeDescription * >(
                jni->GetLongField(
                    jo_proxy, jni_info->m_field_JNI_proxy_m_td_handle ) );
        uno_Interface * pUnoI =
            reinterpret_cast< uno_Interface * >(
                jni->GetLongField(
                    jo_proxy, jni_info->m_field_JNI_proxy_m_receiver_handle ) );

        typelib_TypeDescriptionReference ** ppAllMembers = td->ppAllMembers;
        for ( sal_Int32 nPos = td->nAllMembers; nPos--; )
        {
            // try to avoid getting typedescription as long as possible,
            // because of a Mutex.acquire() in
            // typelib_typedescriptionreference_getDescription()
            typelib_TypeDescriptionReference * member_type =
                ppAllMembers[ nPos ];

            // check method_name against fully qualified type_name
            // of member_type; type_name is of the form
            //  <name> "::" <method_name> *(":@" <idx> "," <idx> ":" <name>)
            OUString const & type_name =
                OUString::unacquired( &member_type->pTypeName );
            sal_Int32 offset = type_name.indexOf( ':' ) + 2;
            assert(offset >= 2);
            assert(offset < type_name.getLength());
            assert(type_name[offset - 1] == ':' );
            sal_Int32 remainder = type_name.getLength() - offset;
            if (typelib_TypeClass_INTERFACE_METHOD == member_type->eTypeClass)
            {
                if ((method_name.getLength() == remainder
                     || (method_name.getLength() < remainder
                         && type_name[offset + method_name.getLength()] == ':'))
                    && type_name.match(method_name, offset))
                {
                    TypeDescr member_td( member_type );
                    typelib_InterfaceMethodTypeDescription * method_td =
                        reinterpret_cast<
                          typelib_InterfaceMethodTypeDescription * >(
                              member_td.get() );
                    return bridge->call_uno(
                        jni, pUnoI, member_td.get(),
                        method_td->pReturnTypeRef,
                        method_td->nParams, method_td->pParams,
                        jo_args );
                }
            }
            else // attribute
            {
                assert(
                    typelib_TypeClass_INTERFACE_ATTRIBUTE ==
                      member_type->eTypeClass );

                if (method_name.getLength() >= 3
                    && (method_name.getLength() - 3 == remainder
                        || (method_name.getLength() - 3 < remainder
                            && type_name[
                                offset + (method_name.getLength() - 3)] == ':'))
                    && method_name[1] == 'e' && method_name[2] == 't'
                    && rtl_ustr_compare_WithLength(
                        type_name.getStr() + offset,
                        method_name.getLength() - 3,
                        method_name.getStr() + 3,
                        method_name.getLength() - 3) == 0)
                {
                    if ('g' == method_name[ 0 ])
                    {
                        TypeDescr member_td( member_type );
                        typelib_InterfaceAttributeTypeDescription * attr_td =
                            reinterpret_cast<
                              typelib_InterfaceAttributeTypeDescription * >(
                                  member_td.get() );
                        return bridge->call_uno(
                            jni, pUnoI, member_td.get(),
                            attr_td->pAttributeTypeRef,
                            0, nullptr,
                            jo_args );
                    }
                    else if ('s' == method_name[ 0 ])
                    {
                        TypeDescr member_td( member_type );
                        typelib_InterfaceAttributeTypeDescription * attr_td =
                            reinterpret_cast<
                              typelib_InterfaceAttributeTypeDescription * >(
                                  member_td.get() );
                        if (! attr_td->bReadOnly)
                        {
                            typelib_MethodParameter param;
                            param.pTypeRef = attr_td->pAttributeTypeRef;
                            param.bIn = true;
                            param.bOut = false;
                            return bridge->call_uno(
                                jni, pUnoI, member_td.get(),
                                jni_info->m_void_type.getTypeLibType(),
                                1, &param,
                                jo_args );
                        }
                    }
                }
            }
        }
        // the thing that should not be... no method info found!
        throw BridgeRuntimeError(
            "calling undeclared function on interface "
            + OUString::unacquired(&td->aBase.pTypeName)
            + ": " + method_name + jni.get_stack_trace() );
    }
    catch (const BridgeRuntimeError & err)
    {
        SAL_WARN(
            "bridges",
            "Java calling UNO method " << method_name << ": " << err.m_message);
        // notify RuntimeException
        OString cstr_msg(
            "[jni_uno bridge error] Java calling UNO method "
            + OUStringToOString(method_name, RTL_TEXTENCODING_JAVA_UTF8) + ": "
            + OUStringToOString(err.m_message, RTL_TEXTENCODING_JAVA_UTF8));
        if (jni->ThrowNew(jni_info->m_class_RuntimeException, cstr_msg.getStr())
            != 0)
        {
            assert( false );
        }
        return nullptr;
    }
    catch (const ::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        SAL_WARN("bridges", "attaching current thread to java failed");
        OString cstr_msg(
            "[jni_uno bridge error] attaching current thread to java failed"
            + OUStringToOString(
                jni.get_stack_trace(), RTL_TEXTENCODING_JAVA_UTF8));
        if (jni->ThrowNew(jni_info->m_class_RuntimeException, cstr_msg.getStr())
            != 0)
        {
            assert( false );
        }
        return nullptr;
    }
}


SAL_JNI_EXPORT void
JNICALL Java_com_sun_star_bridges_jni_1uno_JNI_1proxy_finalize__J(
    JNIEnv * jni_env, jobject jo_proxy, jlong bridge_handle )
{
    Bridge const * bridge = reinterpret_cast< Bridge const * >( bridge_handle );
    JNI_info const * jni_info = bridge->getJniInfo();
    JNI_context jni(
        jni_info, jni_env,
        static_cast< jobject >(
            static_cast<JniUnoEnvironmentData *>(bridge->m_java_env->pContext)
            ->machine->getClassLoader()));

    uno_Interface * pUnoI = reinterpret_cast< uno_Interface * >(
        jni->GetLongField(
            jo_proxy, jni_info->m_field_JNI_proxy_m_receiver_handle ) );
    typelib_TypeDescription * td =
        reinterpret_cast< typelib_TypeDescription * >(
            jni->GetLongField(
                jo_proxy, jni_info->m_field_JNI_proxy_m_td_handle ) );
    SAL_INFO(
        "bridges",
        "freeing java uno proxy: "
            << jstring_to_oustring(
                jni,
                static_cast<jstring>(
                    JLocalAutoRef(
                        jni,
                        jni->GetObjectField(
                            jo_proxy, jni_info->m_field_JNI_proxy_m_oid))
                    .get())));
    // revoke from uno env; has already been revoked from java env
    (*bridge->m_uno_env->revokeInterface)( bridge->m_uno_env, pUnoI );
    // release receiver
    (*pUnoI->release)( pUnoI );
    // release typedescription handle
    typelib_typedescription_release( td );
    // release bridge handle
    bridge->release();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
