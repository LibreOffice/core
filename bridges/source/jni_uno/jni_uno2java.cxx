/*************************************************************************
 *
 *  $RCSfile: jni_uno2java.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dbo $ $Date: 2002-11-22 11:54:01 $
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

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif

#ifdef DEBUG
#include "rtl/ustrbuf.hxx"
#endif

#include "jni_bridge.h"


using namespace ::std;
using namespace ::rtl;

namespace
{
extern "C"
{
//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_free(
    uno_ExtEnvironment * env, void * proxy )
    SAL_THROW_EXTERN_C();
//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_acquire( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C();
//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_release( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C();
//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args[], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C();
}
}

namespace jni_bridge
{

//__________________________________________________________________________________________________
void jni_Bridge::handle_java_exc(
    JNI_attach const & attach, JLocalAutoRef const & jo_exc, uno_Any * uno_exc ) const
{
    OSL_ASSERT( jo_exc.is() );
    if (! jo_exc.is())
    {
        throw BridgeRuntimeError(
            OUSTR("java exception occured, but no java exception available!?") );
    }

    OUString exc_name( get_class_name( attach, jo_exc.get() ) );
    ::com::sun::star::uno::TypeDescription td( exc_name.pData );
    if (!td.is() || (typelib_TypeClass_EXCEPTION != td.get()->eTypeClass))
    {
        // call toString()
        JLocalAutoRef jo_descr(
            attach, attach->CallObjectMethodA(
                jo_exc.get(), m_jni_info->m_method_Object_toString, 0 ) );
        attach.ensure_no_exception();
        OUString descr( jstring_to_oustring( attach, (jstring)jo_descr.get() ) );
        throw BridgeRuntimeError( OUSTR("non-UNO exception occurred: ") + descr );
    }

    auto_ptr< rtl_mem > uno_data( rtl_mem::allocate( td.get()->nSize ) );
    jvalue val;
    val.l = jo_exc.get();
    map_to_uno(
        attach, uno_data.get(), val, td.get()->pWeakRef, 0,
        false /* no assign */, false /* no out param */ );

    typelib_typedescriptionreference_acquire( td.get()->pWeakRef );
    uno_exc->pType = td.get()->pWeakRef;
    uno_exc->pData = uno_data.release();
#ifdef DEBUG
    OString cstr_msg(
        OUStringToOString(
            OUString(uno_exc->pType->pTypeName) + OUSTR(": ") +
            *(OUString const *)uno_exc->pData, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( cstr_msg.getStr() );
#endif
}
//__________________________________________________________________________________________________
void jni_Bridge::call_java(
    jobject javaI, JNI_type_info const * info, sal_Int32 function_pos,
    typelib_TypeDescriptionReference * return_type /* 0 indicates void return */,
    typelib_MethodParameter * params, sal_Int32 nParams,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const
{
    // determine exact interface type info
    while (0 != info->m_base)
    {
        typelib_InterfaceTypeDescription * base_td =
            (typelib_InterfaceTypeDescription *)info->m_base->m_td.get();
        sal_Int32 base_functions = base_td->nMapFunctionIndexToMemberIndex;
        if (function_pos >= base_functions)
        {
            function_pos -= base_functions;
            break;
        }
        info = info->m_base;
    }

    JNI_attach attach( m_java_env );

    // prepare java args, save param td
    jvalue * java_args = (jvalue *)alloca( sizeof (jvalue) * nParams );

    sal_Int32 nPos;
    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        typelib_MethodParameter const & param = params[ nPos ];
        try
        {
            java_args[ nPos ].l = 0; // build up array[ 1 ]
            map_to_java(
                attach, &java_args[ nPos ],
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
                if (param.bOut || typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
                    attach->DeleteLocalRef( java_args[ n ].l );
            }
            throw;
        }
    }

    // complex return value
    JLocalAutoRef java_ret;

    jmethodID method_id = info->m_methods[ function_pos ];
    if (0 == return_type) // indicates void return
    {
        attach->CallVoidMethodA( javaI, method_id, java_args );
    }
    else
    {
        switch (return_type->eTypeClass)
        {
        case typelib_TypeClass_VOID:
            attach->CallVoidMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_CHAR:
            *(sal_Unicode *)uno_ret = attach->CallCharMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_BOOLEAN:
            *(sal_Bool *)uno_ret = attach->CallBooleanMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_BYTE:
            *(sal_Int8 *)uno_ret = attach->CallByteMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int16 *)uno_ret = attach->CallShortMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_Int32 *)uno_ret = attach->CallIntMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            *(sal_Int64 *)uno_ret = attach->CallLongMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_FLOAT:
            *(float *)uno_ret = attach->CallFloatMethodA( javaI, method_id, java_args );
            break;
        case typelib_TypeClass_DOUBLE:
            *(double *)uno_ret = attach->CallDoubleMethodA( javaI, method_id, java_args );
            break;
        default:
            java_ret.reset( attach, attach->CallObjectMethodA( javaI, method_id, java_args ) );
            break;
        }
    }

    if (attach->ExceptionCheck())
    {
        JLocalAutoRef jo_exc( attach, attach->ExceptionOccurred() );
        attach->ExceptionClear();

        // release temp java local refs
        for ( nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = params[ nPos ];
            if (param.bOut || typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
                attach->DeleteLocalRef( java_args[ nPos ].l );
        }

        handle_java_exc( attach, jo_exc, *uno_exc );
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
                        attach, uno_args[ nPos ], java_args[ nPos ], param.pTypeRef, 0,
                        sal_False != param.bIn /* assign if inout */, true /* out param */ );
                    // out array
                    attach->DeleteLocalRef( java_args[ nPos ].l );
                }
                catch (...)
                {
                    // cleanup uno pure out
                    for ( sal_Int32 n = 0; n < nPos; ++n )
                    {
                        typelib_MethodParameter const & param = params[ n ];
                        if (! param.bIn)
                            uno_type_destructData( uno_args[ n ], param.pTypeRef, 0 );
                    }
                    // cleanup java temp local refs
                    for ( ; nPos < nParams; ++nPos )
                    {
                        typelib_MethodParameter const & param = params[ nPos ];
                        if (param.bOut || typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
                            attach->DeleteLocalRef( java_args[ nPos ].l );
                    }
                    throw;
                }
            }
            else // pure temp in param
            {
                if (typelib_TypeClass_DOUBLE < param.pTypeRef->eTypeClass)
                    attach->DeleteLocalRef( java_args[ nPos ].l );
            }
        }

        // return value
        if ((0 != return_type) &&
            (typelib_TypeClass_DOUBLE < return_type->eTypeClass)) // complex
        {
            jvalue val;
            val.l = java_ret.get();
            map_to_uno(
                attach, uno_ret, val, return_type, 0,
                false /* no assign */, false /* no out param */ );
        } // else already set integral uno return value

        // no exception occured
        *uno_exc = 0;
    }
}

//==== a uno proxy wrapping a java interface =======================================================
struct jni_unoInterfaceProxy : public uno_Interface
{
    mutable oslInterlockedCount         m_ref;
    jni_Bridge const *                  m_bridge;

    // mapping information
    OUString                            m_oid;
    jobject                             m_javaI;
    jstring                             m_jo_oid;
    JNI_type_info const *               m_type_info;

    inline void acquire() const SAL_THROW( () );
    inline void release() const SAL_THROW( () );

    // ctor
    inline jni_unoInterfaceProxy(
        JNI_attach const & attach, jni_Bridge const * bridge,
        jobject javaI, jstring jo_oid, OUString const & oid, JNI_type_info const * info )
        SAL_THROW( () );
};
//__________________________________________________________________________________________________
inline jni_unoInterfaceProxy::jni_unoInterfaceProxy(
    JNI_attach const & attach, jni_Bridge const * bridge,
    jobject javaI, jstring jo_oid, OUString const & oid, JNI_type_info const * info )
    SAL_THROW( () )
    : m_ref( 1 ),
      m_oid( oid ),
      m_type_info( info )
{
    bridge->acquire();
    m_bridge = bridge;
    m_javaI = attach->NewGlobalRef( javaI );
    m_jo_oid = (jstring)attach->NewGlobalRef( jo_oid );

    // uno_Interface
    uno_Interface::acquire = jni_unoInterfaceProxy_acquire;
    uno_Interface::release = jni_unoInterfaceProxy_release;
    uno_Interface::pDispatcher = jni_unoInterfaceProxy_dispatch;
}
//__________________________________________________________________________________________________
inline void jni_unoInterfaceProxy::acquire() const SAL_THROW( () )
{
    if (1 == osl_incrementInterlockedCount( &m_ref ))
    {
        // rebirth of proxy zombie
        void * that = const_cast< jni_unoInterfaceProxy * >( this );
        // register at uno env
        (*m_bridge->m_uno_env->registerProxyInterface)(
            m_bridge->m_uno_env, &that,
            jni_unoInterfaceProxy_free, m_oid.pData,
            (typelib_InterfaceTypeDescription *)m_type_info->m_td.get() );
#ifdef DEBUG
        OSL_ASSERT( this == (void const * const)that );
#endif
    }
}
//__________________________________________________________________________________________________
inline void jni_unoInterfaceProxy::release() const SAL_THROW( () )
{
    if (0 == osl_decrementInterlockedCount( &m_ref ))
    {
        // revoke from uno env on last release
        (*m_bridge->m_uno_env->revokeInterface)(
            m_bridge->m_uno_env, const_cast< jni_unoInterfaceProxy * >( this ) );
    }
}

//##################################################################################################

//__________________________________________________________________________________________________
uno_Interface * jni_Bridge::map_java2uno(
    JNI_attach const & attach,
    jobject javaI, JNI_type_info const * info ) const
{
    JLocalAutoRef jo_oid( attach, compute_oid( attach, javaI ) );
    OUString oid( jstring_to_oustring( attach, (jstring)jo_oid.get() ) );

    uno_Interface * pUnoI = 0;
    (*m_uno_env->getRegisteredInterface)(
        m_uno_env, (void **)&pUnoI,
        oid.pData, (typelib_InterfaceTypeDescription *)info->m_td.get() );

    if (0 == pUnoI) // no existing interface, register new proxy
    {
        JLocalAutoRef jo_iface(
            attach, m_jni_info->java_env_registerInterface(
                attach, javaI, (jstring)jo_oid.get(), info->m_jo_type ) );

        // refcount initially 1
        pUnoI = new jni_unoInterfaceProxy(
            attach, const_cast< jni_Bridge * >( this ),
            jo_iface.get(), (jstring)jo_oid.get(), oid, info );

        (*m_uno_env->registerProxyInterface)(
            m_uno_env, (void **)&pUnoI,
            jni_unoInterfaceProxy_free,
            oid.pData, (typelib_InterfaceTypeDescription *)info->m_td.get() );
    }
    return pUnoI;
}

}

using namespace ::jni_bridge;

namespace
{
extern "C"
{

//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_free( uno_ExtEnvironment * env, void * proxy )
    SAL_THROW_EXTERN_C()
{
    jni_unoInterfaceProxy const * that = reinterpret_cast< jni_unoInterfaceProxy const * >( proxy );
    OSL_ASSERT( env == that->m_bridge->m_uno_env );
#ifdef DEBUG
    OString cstr_msg(
        OUStringToOString(
            OUSTR("freeing binary uno proxy: ") + that->m_oid, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( cstr_msg.getStr() );
#endif

    try
    {
        JNI_attach attach( that->m_bridge->m_java_env );
        try
        {
            that->m_bridge->m_jni_info->java_env_revokeInterface(
                attach, that->m_jo_oid, that->m_type_info->m_jo_type );
        }
        catch (BridgeRuntimeError & err)
        {
#ifdef _DEBUG
            OString cstr( OUStringToOString( err.m_message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, cstr.getStr() );
#endif
        }
        attach->DeleteGlobalRef( that->m_javaI );
        attach->DeleteGlobalRef( that->m_jo_oid );
    }
    catch (BridgeRuntimeError & err)
    {
#ifdef _DEBUG
        OString cstr( OUStringToOString( err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, cstr.getStr() );
#endif
    }

    that->m_bridge->release();
#ifdef DEBUG
    *(int *)that = 0xdeadcafe;
#endif
    delete that;
}
//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_acquire( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C()
{
    jni_unoInterfaceProxy const * that = static_cast< jni_unoInterfaceProxy const * >( pUnoI );
    that->acquire();
}
//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_release( uno_Interface * pUnoI )
    SAL_THROW_EXTERN_C()
{
    jni_unoInterfaceProxy * that = static_cast< jni_unoInterfaceProxy * >( pUnoI );
    that->release();
}

//--------------------------------------------------------------------------------------------------
void SAL_CALL jni_unoInterfaceProxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C()
{
    jni_unoInterfaceProxy const * that = static_cast< jni_unoInterfaceProxy const * >( pUnoI );
#ifdef DEBUG
    OUStringBuffer trace_buf( 64 );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("uno->java call: ") );
    trace_buf.append( *reinterpret_cast< OUString const * >( &member_td->pTypeName ) );
    trace_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" on oid ") );
    trace_buf.append( that->m_oid );
    OString cstr_msg(
        OUStringToOString( trace_buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( cstr_msg.getStr() );
#endif

    try
    {
        jni_Bridge const * bridge = that->m_bridge;

        switch (member_td->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            sal_Int32 member_pos = ((typelib_InterfaceMemberTypeDescription *)member_td)->nPosition;
            typelib_InterfaceTypeDescription * iface_td =
                (typelib_InterfaceTypeDescription *)that->m_type_info->m_td.get();
            OSL_ENSURE(
                member_pos < iface_td->nAllMembers,
                "### member pos out of range!" );
            sal_Int32 function_pos = iface_td->pMapMemberIndexToFunctionIndex[ member_pos ];
            OSL_ENSURE(
                function_pos < iface_td->nMapFunctionIndexToMemberIndex,
                "### illegal function index!" );

            if (uno_ret) // is getter method
            {
                bridge->call_java(
                    that->m_javaI, that->m_type_info, function_pos,
                    ((typelib_InterfaceAttributeTypeDescription *)member_td)->pAttributeTypeRef,
                    0, 0, // no params
                    uno_ret, 0, uno_exc );
            }
            else // is setter method
            {
                typelib_MethodParameter param;
                param.pTypeRef =
                    ((typelib_InterfaceAttributeTypeDescription *)member_td)->pAttributeTypeRef;
                param.bIn = sal_True;
                param.bOut = sal_False;

                bridge->call_java(
                    that->m_javaI, that->m_type_info, function_pos +1, // get, then set method
                    0 /* indicates void return */, &param, 1,
                    0, uno_args, uno_exc );
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            sal_Int32 member_pos = ((typelib_InterfaceMemberTypeDescription *)member_td)->nPosition;
            typelib_InterfaceTypeDescription * iface_td =
                (typelib_InterfaceTypeDescription *)that->m_type_info->m_td.get();
            OSL_ENSURE(
                member_pos < iface_td->nAllMembers,
                "### member pos out of range!" );
            sal_Int32 function_pos =
                iface_td->pMapMemberIndexToFunctionIndex[ member_pos ];
            OSL_ENSURE(
                function_pos < iface_td->nMapFunctionIndexToMemberIndex,
                "### illegal function index!" );

            switch (function_pos)
            {
            case 0: // queryInterface()
            {
                TypeDescr demanded_td(
                    *reinterpret_cast< typelib_TypeDescriptionReference ** >( uno_args[ 0 ] ) );
                if (typelib_TypeClass_INTERFACE != demanded_td.get()->eTypeClass)
                {
                    throw BridgeRuntimeError(
                        OUSTR("queryInterface() call demands an INTERFACE type!") );
                }

                uno_Interface * pInterface = 0;
                (*bridge->m_uno_env->getRegisteredInterface)(
                    bridge->m_uno_env,
                    (void **)&pInterface, that->m_oid.pData,
                    (typelib_InterfaceTypeDescription *)demanded_td.get() );

                if (0 == pInterface)
                {
                    JNI_info const * jni_info = bridge->m_jni_info;
                    JNI_attach attach( bridge->m_java_env );

                    JNI_type_info const * info =
                        jni_info->get_type_info( attach, demanded_td.get() );

                    jvalue args[ 2 ];
                    args[ 0 ].l = info->m_jo_type;
                    args[ 1 ].l = that->m_javaI;

                    JLocalAutoRef jo_ret(
                        attach, attach->CallStaticObjectMethodA(
                            jni_info->m_class_UnoRuntime,
                            jni_info->m_method_UnoRuntime_queryInterface, args ) );

                    if (attach->ExceptionCheck())
                    {
                        JLocalAutoRef jo_exc( attach, attach->ExceptionOccurred() );
                        attach->ExceptionClear();
                        bridge->handle_java_exc( attach, jo_exc, *uno_exc );
                    }
                    else
                    {
                        if (jo_ret.is())
                        {
#ifdef _DEBUG
                            JLocalAutoRef jo_oid( attach, compute_oid( attach, jo_ret.get() ) );
                            OUString oid( jstring_to_oustring( attach, (jstring)jo_oid.get() ) );
                            OSL_ENSURE( oid.equals( that->m_oid ), "### different oids!" );
#endif
                            JLocalAutoRef jo_iface(
                                attach, jni_info->java_env_registerInterface(
                                    attach, jo_ret.get(), that->m_jo_oid, info->m_jo_type ) );

                            // refcount initially 1
                            uno_Interface * pUnoI = new jni_unoInterfaceProxy(
                                attach, bridge, jo_iface.get(),
                                that->m_jo_oid, that->m_oid, info );

                            (*bridge->m_uno_env->registerProxyInterface)(
                                bridge->m_uno_env, (void **)&pUnoI,
                                (uno_freeProxyFunc)jni_unoInterfaceProxy_free,
                                that->m_oid.pData,
                                (typelib_InterfaceTypeDescription *)info->m_td.get() );

                            uno_any_construct(
                                (uno_Any *)uno_ret, &pUnoI, demanded_td.get(), 0 );
                            (*pUnoI->release)( pUnoI );
                        }
                        else // object does not support demanded interface
                        {
                            uno_any_construct( (uno_Any *)uno_ret, 0, 0, 0 );
                        }
                        // no excetpion occured
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
                jni_unoInterfaceProxy_acquire( const_cast< jni_unoInterfaceProxy * >( that ) );
                *uno_exc = 0;
                break;
            case 2: // release this proxy
                jni_unoInterfaceProxy_release( const_cast< jni_unoInterfaceProxy * >( that ) );
                *uno_exc = 0;
                break;
            default: // arbitrary method call
            {
                typelib_InterfaceMethodTypeDescription * method_td =
                    (typelib_InterfaceMethodTypeDescription *)member_td;
                bridge->call_java(
                    that->m_javaI, that->m_type_info, function_pos,
                    method_td->pReturnTypeRef, method_td->pParams, method_td->nParams,
                    uno_ret, uno_args, uno_exc );
                break;
            }
            }
            break;
        }
        default:
        {
            throw BridgeRuntimeError( OUSTR("illegal member type description!") );
        }
        }
    }
    catch (BridgeRuntimeError & err)
    {
        // binary identical struct
        ::com::sun::star::uno::RuntimeException exc(
            OUSTR("[jni_uno bridge error] ") + err.m_message,
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
        ::com::sun::star::uno::Type const & exc_type = ::getCppuType( &exc );
        uno_type_any_construct( *uno_exc, &exc, exc_type.getTypeLibType(), 0 );
#ifdef _DEBUG
        OString cstr_msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( cstr_msg.getStr() );
#endif
    }
}

}
}
