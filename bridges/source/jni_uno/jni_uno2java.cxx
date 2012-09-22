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


#include <sal/alloca.h>

#include "com/sun/star/uno/RuntimeException.hpp"

#include "rtl/ustrbuf.hxx"

#include "jni_bridge.h"


using namespace ::std;
using namespace ::rtl;

namespace
{
extern "C"
{

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_free( uno_ExtEnvironment * env, void * proxy )
    SAL_THROW_EXTERN_C();

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_acquire( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C();

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_release( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C();

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args[], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C();
}
}

namespace jni_uno
{

//______________________________________________________________________________
void Bridge::handle_java_exc(
    JNI_context const & jni,
    JLocalAutoRef const & jo_exc, uno_Any * uno_exc ) const
{
    OSL_ASSERT( jo_exc.is() );
    if (! jo_exc.is())
    {
        throw BridgeRuntimeError(
            OUSTR("java exception occurred, but no java exception available!?") +
            jni.get_stack_trace() );
    }

    JLocalAutoRef jo_class( jni, jni->GetObjectClass( jo_exc.get() ) );
    JLocalAutoRef jo_class_name(
        jni, jni->CallObjectMethodA(
            jo_class.get(), m_jni_info->m_method_Class_getName, 0 ) );
    jni.ensure_no_exception();
    OUString exc_name(
        jstring_to_oustring( jni, (jstring) jo_class_name.get() ) );

    ::com::sun::star::uno::TypeDescription td( exc_name.pData );
    if (!td.is() || (typelib_TypeClass_EXCEPTION != td.get()->eTypeClass))
    {
        // call toString()
        JLocalAutoRef jo_descr(
            jni, jni->CallObjectMethodA(
                jo_exc.get(), m_jni_info->m_method_Object_toString, 0 ) );
        jni.ensure_no_exception();
        OUStringBuffer buf( 128 );
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("non-UNO exception occurred: ") );
        buf.append( jstring_to_oustring( jni, (jstring) jo_descr.get() ) );
        buf.append( jni.get_stack_trace( jo_exc.get() ) );
        throw BridgeRuntimeError( buf.makeStringAndClear() );
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr< rtl_mem > uno_data( rtl_mem::allocate( td.get()->nSize ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    jvalue val;
    val.l = jo_exc.get();
    map_to_uno(
        jni, uno_data.get(), val, td.get()->pWeakRef, 0,
        false /* no assign */, false /* no out param */ );

#if OSL_DEBUG_LEVEL > 0
    // patch Message, append stack trace
    reinterpret_cast< ::com::sun::star::uno::Exception * >(
        uno_data.get() )->Message += jni.get_stack_trace( jo_exc.get() );
#endif

    typelib_typedescriptionreference_acquire( td.get()->pWeakRef );
    uno_exc->pType = td.get()->pWeakRef;
    uno_exc->pData = uno_data.release();

#if OSL_DEBUG_LEVEL > 1
    OUStringBuffer trace_buf( 128 );
    trace_buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM("exception occurred uno->java: [") );
    trace_buf.append( exc_name );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] ") );
    trace_buf.append(
        reinterpret_cast< ::com::sun::star::uno::Exception const * >(
            uno_exc->pData )->Message );
    OString cstr_trace(
        OUStringToOString(
            trace_buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "%s", cstr_trace.getStr() );
#endif
}

//______________________________________________________________________________
void Bridge::call_java(
    jobject javaI, typelib_InterfaceTypeDescription * iface_td,
    sal_Int32 local_member_index, sal_Int32 function_pos_offset,
    typelib_TypeDescriptionReference * return_type,
    typelib_MethodParameter * params, sal_Int32 nParams,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const
{
    OSL_ASSERT( function_pos_offset == 0 || function_pos_offset == 1 );

    JNI_guarded_context jni(
        m_jni_info, reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
            m_java_env->pContext ) );

    // assure fully initialized iface_td:
    ::com::sun::star::uno::TypeDescription iface_holder;
    if (! iface_td->aBase.bComplete) {
        iface_holder = ::com::sun::star::uno::TypeDescription(
            reinterpret_cast<typelib_TypeDescription *>(iface_td) );
        iface_holder.makeComplete();
        if (! iface_holder.get()->bComplete) {
            OUStringBuffer buf;
            buf.appendAscii(
                RTL_CONSTASCII_STRINGPARAM("cannot make type complete: ") );
            buf.append( OUString::unacquired(&iface_holder.get()->pTypeName) );
            buf.append( jni.get_stack_trace() );
            throw BridgeRuntimeError( buf.makeStringAndClear() );
        }
        iface_td = reinterpret_cast<typelib_InterfaceTypeDescription *>(
            iface_holder.get() );
        OSL_ASSERT( iface_td->aBase.eTypeClass == typelib_TypeClass_INTERFACE );
    }

    // prepare java args, save param td
#ifdef BROKEN_ALLOCA
    jvalue * java_args = (jvalue *) malloc( sizeof (jvalue) * nParams );
#else
    jvalue * java_args = (jvalue *) alloca( sizeof (jvalue) * nParams );
#endif

    sal_Int32 nPos;
    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        try
        {
            typelib_MethodParameter const & param = params[ nPos ];
            java_args[ nPos ].l = 0; // if out: build up array[ 1 ]
            map_to_java(
                jni, &java_args[ nPos ],
                uno_args[ nPos ],
                param.pTypeRef, 0,
                sal_False != param.bIn /* convert uno value */,
                sal_False != param.bOut /* build up array[ 1 ] */ );
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
#ifdef BROKEN_ALLOCA
        free( java_args );
#endif
            throw;
        }
    }

    sal_Int32 base_members = iface_td->nAllMembers - iface_td->nMembers;
    OSL_ASSERT( base_members < iface_td->nAllMembers );
    sal_Int32 base_members_function_pos =
        iface_td->pMapMemberIndexToFunctionIndex[ base_members ];
    sal_Int32 member_pos = base_members + local_member_index;
    OSL_ENSURE(
        member_pos < iface_td->nAllMembers, "### member pos out of range!" );
    sal_Int32 function_pos =
        iface_td->pMapMemberIndexToFunctionIndex[ member_pos ]
        + function_pos_offset;
    OSL_ENSURE(
        function_pos >= base_members_function_pos
        && function_pos < iface_td->nMapFunctionIndexToMemberIndex,
        "### illegal function index!" );
    function_pos -= base_members_function_pos;

    JNI_interface_type_info const * info =
        static_cast< JNI_interface_type_info const * >(
            m_jni_info->get_type_info( jni, &iface_td->aBase ) );
    jmethodID method_id = info->m_methods[ function_pos ];

#if OSL_DEBUG_LEVEL > 1
    OUStringBuffer trace_buf( 128 );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("calling ") );
    JLocalAutoRef jo_method(
        jni, jni->ToReflectedMethod( info->m_class, method_id, JNI_FALSE ) );
    jni.ensure_no_exception();
    JLocalAutoRef jo_descr(
        jni, jni->CallObjectMethodA(
            jo_method.get(), m_jni_info->m_method_Object_toString, 0 ) );
    jni.ensure_no_exception();
    trace_buf.append( jstring_to_oustring( jni, (jstring) jo_descr.get() ) );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" on ") );
    jo_descr.reset(
        jni->CallObjectMethodA(
            javaI, m_jni_info->m_method_Object_toString, 0 ) );
    jni.ensure_no_exception();
    trace_buf.append( jstring_to_oustring( jni, (jstring) jo_descr.get() ) );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (") );
    JLocalAutoRef jo_class( jni, jni->GetObjectClass( javaI ) );
    jo_descr.reset(
        jni->CallObjectMethodA(
            jo_class.get(), m_jni_info->m_method_Object_toString, 0 ) );
    jni.ensure_no_exception();
    trace_buf.append( jstring_to_oustring( jni, (jstring) jo_descr.get() ) );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(")") );
    OString cstr_trace(
        OUStringToOString(
            trace_buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "%s", cstr_trace.getStr() );
#endif

    // complex return value
    JLocalAutoRef java_ret( jni );

    switch (return_type->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        jni->CallVoidMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_CHAR:
        *(sal_Unicode *)uno_ret =
            jni->CallCharMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_BOOLEAN:
        *(sal_Bool *)uno_ret =
            jni->CallBooleanMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_BYTE:
        *(sal_Int8 *)uno_ret =
            jni->CallByteMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *(sal_Int16 *)uno_ret =
            jni->CallShortMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *(sal_Int32 *)uno_ret =
            jni->CallIntMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *(sal_Int64 *)uno_ret =
            jni->CallLongMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_FLOAT:
        *(float *)uno_ret =
            jni->CallFloatMethodA( javaI, method_id, java_args );
        break;
    case typelib_TypeClass_DOUBLE:
        *(double *)uno_ret =
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
                        java_args[ nPos ], param.pTypeRef, 0,
                        sal_False != param.bIn /* assign if inout */,
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
                                uno_args[ n ], p.pTypeRef, 0 );
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
#ifdef BROKEN_ALLOCA
            free( java_args );
#endif
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
                    jni, uno_ret, val, return_type, 0,
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
                            uno_args[ i ], param.pTypeRef, 0 );
                    }
                }
#ifdef BROKEN_ALLOCA
        free( java_args );
#endif
                throw;
            }
        } // else: already set integral uno return value

        // no exception occurred
        *uno_exc = 0;
    }
#ifdef BROKEN_ALLOCA
    free( java_args );
#endif
}

//==== a uno proxy wrapping a java interface ===================================
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

//______________________________________________________________________________
inline UNO_proxy::UNO_proxy(
    JNI_context const & jni, Bridge const * bridge,
    jobject javaI, jstring jo_oid, OUString const & oid,
    JNI_interface_type_info const * info )
    : m_ref( 1 ),
      m_oid( oid ),
      m_type_info( info )
{
    JNI_info const * jni_info = bridge->m_jni_info;
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
    m_jo_oid = (jstring) jni->NewGlobalRef( jo_oid );
    bridge->acquire();
    m_bridge = bridge;

    // uno_Interface
    uno_Interface::acquire = UNO_proxy_acquire;
    uno_Interface::release = UNO_proxy_release;
    uno_Interface::pDispatcher = UNO_proxy_dispatch;
}

//______________________________________________________________________________
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
            (typelib_InterfaceTypeDescription *)m_type_info->m_td.get() );
#if OSL_DEBUG_LEVEL > 1
        OSL_ASSERT( this == (void const * const)that );
#endif
    }
}

//______________________________________________________________________________
inline void UNO_proxy::release() const
{
    if (0 == osl_atomic_decrement( &m_ref ))
    {
        // revoke from uno env on last release
        (*m_bridge->m_uno_env->revokeInterface)(
            m_bridge->m_uno_env, const_cast< UNO_proxy * >( this ) );
    }
}


//______________________________________________________________________________
uno_Interface * Bridge::map_to_uno(
    JNI_context const & jni,
    jobject javaI, JNI_interface_type_info const * info ) const
{
    JLocalAutoRef jo_oid( jni, compute_oid( jni, javaI ) );
    OUString oid( jstring_to_oustring( jni, (jstring) jo_oid.get() ) );

    uno_Interface * pUnoI = 0;
    (*m_uno_env->getRegisteredInterface)(
        m_uno_env, (void **)&pUnoI,
        oid.pData, (typelib_InterfaceTypeDescription *)info->m_td.get() );

    if (0 == pUnoI) // no existing interface, register new proxy
    {
        // refcount initially 1
        pUnoI = new UNO_proxy(
            jni, const_cast< Bridge * >( this ),
            javaI, (jstring) jo_oid.get(), oid, info );

        (*m_uno_env->registerProxyInterface)(
            m_uno_env, (void **)&pUnoI,
            UNO_proxy_free,
            oid.pData, (typelib_InterfaceTypeDescription *)info->m_td.get() );
    }
    return pUnoI;
}

}

using namespace ::jni_uno;

namespace
{
extern "C"
{

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_free( uno_ExtEnvironment * env, void * proxy )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = reinterpret_cast< UNO_proxy const * >( proxy );
    Bridge const * bridge = that->m_bridge;

    if ( env != bridge->m_uno_env ) {
        OSL_ASSERT(false);
    }
#if OSL_DEBUG_LEVEL > 1
    OString cstr_msg(
        OUStringToOString(
            OUSTR("freeing binary uno proxy: ") + that->m_oid,
            RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "%s", cstr_msg.getStr() );
#endif

    try
    {
        JNI_guarded_context jni(
            bridge->m_jni_info,
            reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
                bridge->m_java_env->pContext ) );

        jni->DeleteGlobalRef( that->m_javaI );
        jni->DeleteGlobalRef( that->m_jo_oid );
    }
    catch (BridgeRuntimeError & err)
    {
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg2(
            OUStringToOString( err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr_msg2.getStr() );
#else
        (void) err; // unused
#endif
    }
    catch (::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        OSL_FAIL(
            "[jni_uno bridge error] attaching current thread to java failed!" );
    }

    bridge->release();
#if OSL_DEBUG_LEVEL > 1
    *(int *)that = 0xdeadcafe;
#endif
    delete that;
}

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_acquire( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = static_cast< UNO_proxy const * >( pUnoI );
    that->acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_release( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = static_cast< UNO_proxy const * >( pUnoI );
    that->release();
}

//------------------------------------------------------------------------------
void SAL_CALL UNO_proxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C()
{
    UNO_proxy const * that = static_cast< UNO_proxy const * >( pUnoI );
    Bridge const * bridge = that->m_bridge;

#if OSL_DEBUG_LEVEL > 1
    OUStringBuffer trace_buf( 64 );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("uno->java call: ") );
    trace_buf.append( OUString::unacquired( &member_td->pTypeName ) );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" on oid ") );
    trace_buf.append( that->m_oid );
    OString cstr_msg(
        OUStringToOString(
            trace_buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "%s", cstr_msg.getStr() );
#endif

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
            while ( attrib_td->pBaseRef != 0 ) {
                attrib_holder = com::sun::star::uno::TypeDescription(
                    attrib_td->pBaseRef );
                OSL_ASSERT(
                    attrib_holder.get()->eTypeClass
                    == typelib_TypeClass_INTERFACE_ATTRIBUTE );
                attrib_td = reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        attrib_holder.get() );
            }
            typelib_InterfaceTypeDescription * iface_td = attrib_td->pInterface;

            if (0 == uno_ret) // is setter method
            {
                typelib_MethodParameter param;
                param.pTypeRef = attrib_td->pAttributeTypeRef;
                param.bIn = sal_True;
                param.bOut = sal_False;

                bridge->call_java(
                    that->m_javaI, iface_td,
                    attrib_td->nIndex, 1, // get, then set method
                    bridge->m_jni_info->m_void_type.getTypeLibType(),
                    &param, 1,
                    0, uno_args, uno_exc );
            }
            else // is getter method
            {
                bridge->call_java(
                    that->m_javaI, iface_td, attrib_td->nIndex, 0,
                    attrib_td->pAttributeTypeRef,
                    0, 0, // no params
                    uno_ret, 0, uno_exc );
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
            while ( method_td->pBaseRef != 0 ) {
                method_holder = com::sun::star::uno::TypeDescription(
                    method_td->pBaseRef );
                OSL_ASSERT(
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
                    *reinterpret_cast< typelib_TypeDescriptionReference ** >(
                        uno_args[ 0 ] ) );
                if (typelib_TypeClass_INTERFACE !=
                      demanded_td.get()->eTypeClass)
                {
                    throw BridgeRuntimeError(
                        OUSTR("queryInterface() call demands "
                              "an INTERFACE type!") );
                }

                uno_Interface * pInterface = 0;
                (*bridge->m_uno_env->getRegisteredInterface)(
                    bridge->m_uno_env,
                    (void **) &pInterface, that->m_oid.pData,
                    (typelib_InterfaceTypeDescription *)demanded_td.get() );

                if (0 == pInterface)
                {
                    JNI_info const * jni_info = bridge->m_jni_info;
                    JNI_guarded_context jni(
                        jni_info,
                        reinterpret_cast< ::jvmaccess::UnoVirtualMachine * >(
                            bridge->m_java_env->pContext ) );

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
#if OSL_DEBUG_LEVEL > 0
                            JLocalAutoRef jo_oid(
                                jni, compute_oid( jni, jo_ret.get() ) );
                            OUString oid( jstring_to_oustring(
                                              jni, (jstring) jo_oid.get() ) );
                            OSL_ENSURE(
                                oid.equals( that->m_oid ),
                                "### different oids!" );
#endif
                            // refcount initially 1
                            uno_Interface * pUnoI2 = new UNO_proxy(
                                jni, bridge, jo_ret.get(),
                                that->m_jo_oid, that->m_oid, info );

                            (*bridge->m_uno_env->registerProxyInterface)(
                                bridge->m_uno_env,
                                (void **) &pUnoI2,
                                UNO_proxy_free, that->m_oid.pData,
                                reinterpret_cast<
                                  typelib_InterfaceTypeDescription * >(
                                      info->m_td.get() ) );

                            uno_any_construct(
                                (uno_Any *)uno_ret, &pUnoI2,
                                demanded_td.get(), 0 );
                            (*pUnoI2->release)( pUnoI2 );
                        }
                        else // object does not support demanded interface
                        {
                            uno_any_construct(
                                reinterpret_cast< uno_Any * >( uno_ret ),
                                0, 0, 0 );
                        }
                        // no exception occurred
                        *uno_exc = 0;
                    }
                }
                else
                {
                    uno_any_construct(
                        reinterpret_cast< uno_Any * >( uno_ret ),
                        &pInterface, demanded_td.get(), 0 );
                    (*pInterface->release)( pInterface );
                    *uno_exc = 0;
                }
                break;
            }
            case 1: // acquire this proxy
                that->acquire();
                *uno_exc = 0;
                break;
            case 2: // release this proxy
                that->release();
                *uno_exc = 0;
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
                OUSTR("illegal member type description!") );
        }
        }
    }
    catch (BridgeRuntimeError & err)
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM(
                "[jni_uno bridge error] UNO calling Java method ") );
        if (typelib_TypeClass_INTERFACE_METHOD == member_td->eTypeClass ||
            typelib_TypeClass_INTERFACE_ATTRIBUTE == member_td->eTypeClass)
        {
            buf.append( OUString::unacquired(
                            &reinterpret_cast<
                            typelib_InterfaceMemberTypeDescription const * >(
                                member_td )->pMemberName ) );
        }
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(": ") );
        buf.append( err.m_message );
        // binary identical struct
        ::com::sun::star::uno::RuntimeException exc(
            buf.makeStringAndClear(),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
        ::com::sun::star::uno::Type const & exc_type = ::getCppuType( &exc );
        uno_type_any_construct( *uno_exc, &exc, exc_type.getTypeLibType(), 0 );
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg2(
            OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "%s", cstr_msg2.getStr() );
#endif
    }
    catch (::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        // binary identical struct
        ::com::sun::star::uno::RuntimeException exc(
            OUSTR("[jni_uno bridge error] attaching current thread "
                  "to java failed!"),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
        ::com::sun::star::uno::Type const & exc_type = ::getCppuType( &exc );
        uno_type_any_construct( *uno_exc, &exc, exc_type.getTypeLibType(), 0 );
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg2(
            OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr_msg2.getStr() );
#endif
    }
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
