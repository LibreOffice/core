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

#include "jni_helper.h"

#include <com/sun/star/uno/RuntimeException.hpp>

#include <jvmaccess/unovirtualmachine.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include <uno/lbnames.h>


namespace jni_uno
{


JNI_type_info::JNI_type_info(
    JNI_context const & jni, typelib_TypeDescription * td )
    : m_td( td ),
      m_class( nullptr )
{
    m_td.makeComplete();
    if (! m_td.get()->bComplete)
    {
        throw BridgeRuntimeError(
            "cannot make type complete: "
            + OUString::unacquired( &m_td.get()->pTypeName )
            + jni.get_stack_trace() );
    }
}


void JNI_interface_type_info::destroy( JNIEnv * jni_env )
{
    JNI_type_info::destruct( jni_env );
    jni_env->DeleteGlobalRef( m_proxy_ctor );
    jni_env->DeleteGlobalRef( m_type );
    m_methods.reset();
    delete this;
}


JNI_interface_type_info::JNI_interface_type_info(
    JNI_context const & jni, typelib_TypeDescription * td_ )
    : JNI_type_info( jni, td_ )
{
    assert( m_td.get()->eTypeClass == typelib_TypeClass_INTERFACE );

    OUString const & uno_name = OUString::unacquired( &m_td.get()->pTypeName );
    JNI_info const * jni_info = jni.get_info();

    JLocalAutoRef jo_class(
        jni,
        find_class(
            jni,
            ( OUStringToOString( uno_name, RTL_TEXTENCODING_JAVA_UTF8 ).
              getStr() ) ) );
    JLocalAutoRef jo_type( jni, create_type( jni, static_cast<jclass>(jo_class.get()) ) );

    // get proxy ctor
    jvalue arg;
    arg.l = jo_class.get();
    JLocalAutoRef jo_proxy_ctor(
        jni, jni->CallStaticObjectMethodA(
            jni_info->m_class_JNI_proxy,
            jni_info->m_method_JNI_proxy_get_proxy_ctor, &arg ) );

    if (is_XInterface( m_td.get()->pWeakRef ))
    {
        m_methods = nullptr; // no methods
    }
    else
    {
        // retrieve method ids for all direct members
        try
        {
            typelib_InterfaceTypeDescription * td =
                reinterpret_cast< typelib_InterfaceTypeDescription * >(
                    m_td.get() );
            // coverity[ctor_dtor_leak] - on purpose
            m_methods.reset(new jmethodID[ td->nMapFunctionIndexToMemberIndex ]);
            sal_Int32 nMethodIndex = 0;
            typelib_TypeDescriptionReference ** ppMembers = td->ppMembers;
            sal_Int32 nMembers = td->nMembers;

            for ( sal_Int32 nPos = 0; nPos < nMembers; ++nPos )
            {
                TypeDescr member_td( ppMembers[ nPos ] );

                OStringBuffer sig_buf( 64 );

                if (member_td.get()->eTypeClass ==
                      typelib_TypeClass_INTERFACE_METHOD) // method
                {
                    typelib_InterfaceMethodTypeDescription * method_td =
                        reinterpret_cast<
                          typelib_InterfaceMethodTypeDescription * >(
                              member_td.get() );

                    sig_buf.append( '(' );
                    for ( sal_Int32 i = 0; i < method_td->nParams; ++i )
                    {
                        typelib_MethodParameter const & param =
                            method_td->pParams[ i ];
                        if (param.bOut)
                            sig_buf.append( '[' );
                        JNI_info::append_sig( &sig_buf, param.pTypeRef );
                    }
                    sig_buf.append( ')' );
                    JNI_info::append_sig( &sig_buf, method_td->pReturnTypeRef );

                    OString method_signature( sig_buf.makeStringAndClear() );
                    OString method_name(
                        OUStringToOString( OUString::unacquired(
                                               &method_td->aBase.pMemberName ),
                                           RTL_TEXTENCODING_JAVA_UTF8 ) );

                    m_methods[ nMethodIndex ] = jni->GetMethodID(
                        static_cast<jclass>(jo_class.get()), method_name.getStr(),
                        method_signature.getStr() );
                    jni.ensure_no_exception();
                    assert( m_methods[ nMethodIndex ] != nullptr );
                    ++nMethodIndex;
                }
                else // attribute
                {
                    assert(
                        member_td.get()->eTypeClass ==
                          typelib_TypeClass_INTERFACE_ATTRIBUTE );
                    typelib_InterfaceAttributeTypeDescription * attribute_td =
                        reinterpret_cast<
                          typelib_InterfaceAttributeTypeDescription * >(
                              member_td.get() );

                    // type sig
                    JNI_info::append_sig(
                        &sig_buf, attribute_td->pAttributeTypeRef );
                    OString type_sig( sig_buf.makeStringAndClear() );
                    sig_buf.ensureCapacity( 64 );
                    // member name
                    OUString const & member_name =
                        OUString::unacquired(
                            &attribute_td->aBase.pMemberName );

                    // getter
                    sig_buf.append( "()" + type_sig );
                    OString method_signature( sig_buf.makeStringAndClear() );
                    OString method_name(
                        OUStringToOString(
                            rtl::Concat2View("get" + member_name),
                            RTL_TEXTENCODING_JAVA_UTF8 ) );
                    m_methods[ nMethodIndex ] = jni->GetMethodID(
                        static_cast<jclass>(jo_class.get()), method_name.getStr(),
                        method_signature.getStr() );
                    jni.ensure_no_exception();
                    assert( m_methods[ nMethodIndex ] != nullptr );
                    ++nMethodIndex;
                    if (! attribute_td->bReadOnly)
                    {
                        // setter
                        method_signature = "(" + type_sig + ")V";
                        method_name = OUStringToOString(
                            rtl::Concat2View("set" + member_name),
                            RTL_TEXTENCODING_JAVA_UTF8 );
                        m_methods[ nMethodIndex ] = jni->GetMethodID(
                            static_cast<jclass>(jo_class.get()), method_name.getStr(),
                            method_signature.getStr() );
                        jni.ensure_no_exception();
                        assert( m_methods[ nMethodIndex ] != nullptr );
                        ++nMethodIndex;
                    }
                }
            }
        }
        catch (...)
        {
            m_methods.reset();
            throw;
        }
    }
    m_class = static_cast<jclass>(jni->NewGlobalRef( jo_class.get() ));
    m_type = jni->NewGlobalRef( jo_type.get() );
    m_proxy_ctor = jni->NewGlobalRef( jo_proxy_ctor.get() );
}


void JNI_compound_type_info::destroy( JNIEnv * jni_env )
{
    JNI_type_info::destruct( jni_env );
    m_fields.reset();
    delete this;
}


JNI_compound_type_info::JNI_compound_type_info(
    JNI_context const & jni, typelib_TypeDescription * td_ )
    : JNI_type_info( jni, td_ ),
      m_exc_ctor( nullptr )
{
    assert( m_td.get()->eTypeClass == typelib_TypeClass_STRUCT ||
                m_td.get()->eTypeClass == typelib_TypeClass_EXCEPTION );
    typelib_CompoundTypeDescription * td =
        reinterpret_cast< typelib_CompoundTypeDescription * >( m_td.get() );

    OUString const & uno_name =
        OUString::unacquired( &td->aBase.pTypeName );

    // Erase type arguments of instantiated polymorphic struct types:
    std::u16string_view nucleus;
    sal_Int32 i = uno_name.indexOf( '<' );
    if ( i < 0 ) {
        nucleus = uno_name;
    } else {
        nucleus = uno_name.subView( 0, i );
    }
    JLocalAutoRef jo_class(
        jni,
        find_class(
            jni,
            OUStringToOString(
                nucleus, RTL_TEXTENCODING_JAVA_UTF8 ).getStr() ) );

    JNI_info const * jni_info = jni.get_info();

    if (m_td.get()->eTypeClass == typelib_TypeClass_EXCEPTION)
    {
        // retrieve exc ctor( msg )
        m_exc_ctor = jni->GetMethodID(
            static_cast<jclass>(jo_class.get()), "<init>", "(Ljava/lang/String;)V" );
        jni.ensure_no_exception();
        assert( m_exc_ctor != nullptr );
    }

    // retrieve info for base type
    typelib_TypeDescription * base_td =
        type_equals(
            td->aBase.pWeakRef,
            jni_info->m_RuntimeException_type.getTypeLibType())
        ? nullptr
        : reinterpret_cast< typelib_TypeDescription * >(
            td->pBaseTypeDescription );
    m_base = (base_td == nullptr ? nullptr : jni_info->get_type_info( jni, base_td ));

    try
    {
        if (type_equals(
                td->aBase.pWeakRef,
                jni_info->m_Exception_type.getTypeLibType() ) ||
            type_equals(
                td->aBase.pWeakRef,
                jni_info->m_RuntimeException_type.getTypeLibType() ))
        {
            // coverity[ctor_dtor_leak] - on purpose
            m_fields.reset(new jfieldID[ 2 ]);
            m_fields[ 0 ] = nullptr; // special Throwable.getMessage()
            // field Context
            m_fields[ 1 ] = jni->GetFieldID(
                static_cast<jclass>(jo_class.get()), "Context", "Ljava/lang/Object;" );
            jni.ensure_no_exception();
            assert( m_fields[ 1 ] != nullptr );
        }
        else
        {
            // retrieve field ids for all direct members
            sal_Int32 nMembers = td->nMembers;
            m_fields.reset(new jfieldID[ nMembers ]);

            for ( sal_Int32 nPos = 0; nPos < nMembers; ++nPos )
            {
                OString sig;
                if (td->aBase.eTypeClass == typelib_TypeClass_STRUCT
                    && reinterpret_cast< typelib_StructTypeDescription * >(
                        td)->pParameterizedTypes != nullptr
                    && reinterpret_cast< typelib_StructTypeDescription * >(
                        td)->pParameterizedTypes[nPos])
                {
                    sig = "Ljava/lang/Object;"_ostr;
                } else {
                    OStringBuffer sig_buf( 32 );
                    JNI_info::append_sig( &sig_buf, td->ppTypeRefs[ nPos ] );
                    sig = sig_buf.makeStringAndClear();
                }

                OString member_name(
                    OUStringToOString(
                        OUString::unacquired( &td->ppMemberNames[ nPos ] ),
                        RTL_TEXTENCODING_JAVA_UTF8 ) );

                m_fields[ nPos ] = jni->GetFieldID(
                    static_cast<jclass>(jo_class.get()), member_name.getStr(),
                    sig.getStr() );
                jni.ensure_no_exception();
                assert( m_fields[ nPos ] != nullptr );
            }
        }
    }
    catch (...)
    {
        m_fields.reset();
        throw;
    }

    m_class = static_cast<jclass>(jni->NewGlobalRef( jo_class.get() ));
}


JNI_type_info const * JNI_info::create_type_info(
    JNI_context const & jni, typelib_TypeDescription * td ) const
{
    OUString const & uno_name = OUString::unacquired( &td->pTypeName );

    JNI_type_info * new_info;
    switch (td->eTypeClass)
    {
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        new_info = new JNI_compound_type_info( jni, td );
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        new_info = new JNI_interface_type_info( jni, td );
        break;
    }
    default:
    {
        throw BridgeRuntimeError(
            "type info not supported for " + uno_name + jni.get_stack_trace() );
    }
    }

    // look up
    JNI_type_info * info;
    std::unique_lock guard( m_mutex );
    JNI_type_info_holder & holder = m_type_map[ uno_name ];
    if (holder.m_info == nullptr) // new insertion
    {
        holder.m_info = new_info;
        guard.unlock();
        info = new_info;
    }
    else // inserted in the meantime
    {
        info = holder.m_info;
        guard.unlock();
        new_info->destroy( jni.get_jni_env() );
    }
    return info;
}


JNI_type_info const * JNI_info::get_type_info(
    JNI_context const & jni, typelib_TypeDescription * td ) const
{
    if (is_XInterface( td->pWeakRef ))
    {
        return m_XInterface_type_info;
    }

    OUString const & uno_name = OUString::unacquired( &td->pTypeName );
    JNI_type_info const * info;
    std::unique_lock guard( m_mutex );

    t_str2type::const_iterator iFind( m_type_map.find( uno_name ) );
    if (iFind == m_type_map.end())
    {
        guard.unlock();
        info = create_type_info( jni, td );
    }
    else
    {
        info = iFind->second.m_info;
    }

    return info;
}


JNI_type_info const * JNI_info::get_type_info(
    JNI_context const & jni, typelib_TypeDescriptionReference * type ) const
{
    if (is_XInterface( type ))
    {
        return m_XInterface_type_info;
    }

    OUString const & uno_name = OUString::unacquired( &type->pTypeName );
    JNI_type_info const * info;
    std::unique_lock guard( m_mutex );
    t_str2type::const_iterator iFind( m_type_map.find( uno_name ) );
    if (iFind == m_type_map.end())
    {
        guard.unlock();
        TypeDescr td( type );
        info = create_type_info( jni, td.get() );
    }
    else
    {
        info = iFind->second.m_info;
    }

    return info;
}


JNI_type_info const * JNI_info::get_type_info(
    JNI_context const & jni, OUString const & uno_name ) const
{
    if ( uno_name == "com.sun.star.uno.XInterface" )
    {
        return m_XInterface_type_info;
    }

    JNI_type_info const * info;
    std::unique_lock guard( m_mutex );
    t_str2type::const_iterator iFind( m_type_map.find( uno_name ) );
    if (iFind == m_type_map.end())
    {
        guard.unlock();
        css::uno::TypeDescription td( uno_name );
        if (! td.is())
        {
            throw BridgeRuntimeError(
                "UNO type not found: " + uno_name + jni.get_stack_trace() );
        }
        info = create_type_info( jni, td.get() );
    }
    else
    {
        info = iFind->second.m_info;
    }

    return info;
}


JNI_info::JNI_info(
    JNIEnv * jni_env, jobject class_loader, jclass classClass,
    jmethodID methodForName )
    : m_class_Class( classClass ),
      m_method_Class_forName( methodForName ),
      m_class_JNI_proxy( nullptr ),
      m_XInterface_queryInterface_td(
        (reinterpret_cast< typelib_InterfaceTypeDescription * >(
            css::uno::TypeDescription(
                cppu::UnoType<css::uno::XInterface>::get())
            .get())->ppMembers[ 0 ] ) ),
      m_Exception_type(cppu::UnoType<css::uno::Exception>::get()),
      m_RuntimeException_type(cppu::UnoType<css::uno::RuntimeException>::get()),
      m_void_type(cppu::UnoType<void>::get()),
      m_XInterface_type_info( nullptr )
{
    JNI_context jni( this, jni_env, class_loader ); // !no proper jni_info!

    // class lookup
    JLocalAutoRef jo_Object(
        jni, find_class( jni, "java.lang.Object" ) );
    JLocalAutoRef jo_Class(
        jni, find_class( jni, "java.lang.Class" ) );
    JLocalAutoRef jo_Throwable(
        jni, find_class( jni, "java.lang.Throwable" ) );
    JLocalAutoRef jo_Character(
        jni, find_class( jni, "java.lang.Character" ) );
    JLocalAutoRef jo_Boolean(
        jni, find_class( jni, "java.lang.Boolean" ) );
    JLocalAutoRef jo_Byte(
        jni, find_class( jni, "java.lang.Byte" ) );
    JLocalAutoRef jo_Short(
        jni, find_class( jni, "java.lang.Short" ) );
    JLocalAutoRef jo_Integer(
        jni, find_class( jni, "java.lang.Integer" ) );
    JLocalAutoRef jo_Long(
        jni, find_class( jni, "java.lang.Long" ) );
    JLocalAutoRef jo_Float(
        jni, find_class( jni, "java.lang.Float" ) );
    JLocalAutoRef jo_Double(
        jni, find_class( jni, "java.lang.Double" ) );
    JLocalAutoRef jo_String(
        jni, find_class( jni, "java.lang.String" ) );
    JLocalAutoRef jo_RuntimeException(
        jni, find_class( jni, "com.sun.star.uno.RuntimeException" ) );
    JLocalAutoRef jo_UnoRuntime(
        jni, find_class( jni, "com.sun.star.uno.UnoRuntime" ) );
    JLocalAutoRef jo_Any(
        jni, find_class( jni, "com.sun.star.uno.Any" ) );
    JLocalAutoRef jo_Enum(
        jni, find_class( jni, "com.sun.star.uno.Enum" ) );
    JLocalAutoRef jo_Type(
        jni, find_class( jni, "com.sun.star.uno.Type" ) );
    JLocalAutoRef jo_TypeClass(
        jni, find_class( jni, "com.sun.star.uno.TypeClass" ) );
    JLocalAutoRef jo_IEnvironment(
        jni, find_class( jni, "com.sun.star.uno.IEnvironment" ) );
    JLocalAutoRef jo_JNI_proxy(
        jni, find_class( jni, "com.sun.star.bridges.jni_uno.JNI_proxy" ) );
    JLocalAutoRef jo_AsynchronousFinalizer(
        jni, find_class( jni, "com.sun.star.lib.util.AsynchronousFinalizer" ) );

    // method Object.toString()
    m_method_Object_toString = jni->GetMethodID(
        static_cast<jclass>(jo_Object.get()), "toString", "()Ljava/lang/String;" );
    jni.ensure_no_exception();
    assert( m_method_Object_toString != nullptr );
    // method Class.getName()
    m_method_Class_getName = jni->GetMethodID(
        static_cast<jclass>(jo_Class.get()), "getName", "()Ljava/lang/String;" );
    jni.ensure_no_exception();
    assert( m_method_Class_getName != nullptr );

    // method Throwable.getMessage()
    m_method_Throwable_getMessage = jni->GetMethodID(
        static_cast<jclass>(jo_Throwable.get()), "getMessage", "()Ljava/lang/String;" );
    jni.ensure_no_exception();
    assert( m_method_Throwable_getMessage != nullptr );

    // method Character.charValue()
    m_method_Character_charValue = jni->GetMethodID(
        static_cast<jclass>(jo_Character.get()), "charValue", "()C" );
    jni.ensure_no_exception();
    assert( m_method_Character_charValue != nullptr );
    // method Boolean.booleanValue()
    m_method_Boolean_booleanValue = jni->GetMethodID(
        static_cast<jclass>(jo_Boolean.get()), "booleanValue", "()Z" );
    jni.ensure_no_exception();
    assert( m_method_Boolean_booleanValue != nullptr );
    // method Byte.byteValue()
    m_method_Byte_byteValue = jni->GetMethodID(
        static_cast<jclass>(jo_Byte.get()), "byteValue", "()B" );
    jni.ensure_no_exception();
    assert( m_method_Byte_byteValue != nullptr );
    // method Short.shortValue()
    m_method_Short_shortValue = jni->GetMethodID(
        static_cast<jclass>(jo_Short.get()), "shortValue", "()S" );
    jni.ensure_no_exception();
    assert( m_method_Short_shortValue != nullptr );
    // method Integer.intValue()
    m_method_Integer_intValue = jni->GetMethodID(
        static_cast<jclass>(jo_Integer.get()), "intValue", "()I" );
    jni.ensure_no_exception();
    assert( m_method_Integer_intValue != nullptr );
    // method Long.longValue()
    m_method_Long_longValue = jni->GetMethodID(
        static_cast<jclass>(jo_Long.get()), "longValue", "()J" );
    jni.ensure_no_exception();
    assert( m_method_Long_longValue != nullptr );
    // method Float.floatValue()
    m_method_Float_floatValue = jni->GetMethodID(
        static_cast<jclass>(jo_Float.get()), "floatValue", "()F" );
    jni.ensure_no_exception();
    assert( m_method_Float_floatValue != nullptr );
    // method Double.doubleValue()
    m_method_Double_doubleValue = jni->GetMethodID(
        static_cast<jclass>(jo_Double.get()), "doubleValue", "()D" );
    jni.ensure_no_exception();
    assert( m_method_Double_doubleValue != nullptr );

    // ctor Character( char )
    m_ctor_Character_with_char = jni->GetMethodID(
        static_cast<jclass>(jo_Character.get()), "<init>", "(C)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Character_with_char != nullptr );
    // ctor Boolean( boolean )
    m_ctor_Boolean_with_boolean = jni->GetMethodID(
        static_cast<jclass>(jo_Boolean.get()), "<init>", "(Z)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Boolean_with_boolean != nullptr );
    // ctor Byte( byte )
    m_ctor_Byte_with_byte = jni->GetMethodID(
        static_cast<jclass>(jo_Byte.get()), "<init>", "(B)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Byte_with_byte != nullptr );
    // ctor Short( short )
    m_ctor_Short_with_short = jni->GetMethodID(
        static_cast<jclass>(jo_Short.get()), "<init>", "(S)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Short_with_short != nullptr );
    // ctor Integer( int )
    m_ctor_Integer_with_int = jni->GetMethodID(
        static_cast<jclass>(jo_Integer.get()), "<init>", "(I)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Integer_with_int != nullptr );
    // ctor Long( long )
    m_ctor_Long_with_long = jni->GetMethodID(
        static_cast<jclass>(jo_Long.get()), "<init>", "(J)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Long_with_long != nullptr );
    // ctor Float( float )
    m_ctor_Float_with_float = jni->GetMethodID(
        static_cast<jclass>(jo_Float.get()), "<init>", "(F)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Float_with_float != nullptr );
    // ctor Double( double )
    m_ctor_Double_with_double = jni->GetMethodID(
        static_cast<jclass>(jo_Double.get()), "<init>", "(D)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Double_with_double != nullptr );

    // static method UnoRuntime.generateOid()
    m_method_UnoRuntime_generateOid = jni->GetStaticMethodID(
        static_cast<jclass>(jo_UnoRuntime.get()),
        "generateOid", "(Ljava/lang/Object;)Ljava/lang/String;" );
    jni.ensure_no_exception();
    assert( m_method_UnoRuntime_generateOid != nullptr );
    // static method UnoRuntime.queryInterface()
    m_method_UnoRuntime_queryInterface = jni->GetStaticMethodID(
        static_cast<jclass>(jo_UnoRuntime.get()),
        "queryInterface",
        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    assert( m_method_UnoRuntime_queryInterface != nullptr );

    // field Enum.m_value
    m_field_Enum_m_value = jni->GetFieldID(
        static_cast<jclass>(jo_Enum.get()), "m_value", "I" );
    jni.ensure_no_exception();
    assert( m_field_Enum_m_value != nullptr );

    // static method TypeClass.fromInt()
    m_method_TypeClass_fromInt = jni->GetStaticMethodID(
        static_cast<jclass>(jo_TypeClass.get()),
        "fromInt", "(I)Lcom/sun/star/uno/TypeClass;" );
    jni.ensure_no_exception();
    assert( m_method_TypeClass_fromInt != nullptr );

    // ctor Type( Class )
    m_ctor_Type_with_Class = jni->GetMethodID(
        static_cast<jclass>(jo_Type.get()), "<init>", "(Ljava/lang/Class;)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Type_with_Class != nullptr );
    // ctor Type( String, TypeClass )
    m_ctor_Type_with_Name_TypeClass = jni->GetMethodID(
        static_cast<jclass>(jo_Type.get()),
        "<init>", "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Type_with_Name_TypeClass != nullptr );
    // field Type._typeName
    m_field_Type_typeName = jni->GetFieldID(
        static_cast<jclass>(jo_Type.get()), "_typeName", "Ljava/lang/String;" );
    jni.ensure_no_exception();
    assert( m_field_Type_typeName != nullptr );

    // ctor Any( Type, Object )
    m_ctor_Any_with_Type_Object = jni->GetMethodID(
        static_cast<jclass>(jo_Any.get()),
        "<init>", "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V" );
    jni.ensure_no_exception();
    assert( m_ctor_Any_with_Type_Object != nullptr );

    // field Any._type
    m_field_Any_type = jni->GetFieldID(
        static_cast<jclass>(jo_Any.get()), "_type", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    assert( m_field_Any_type != nullptr );
    // field Any._object
    m_field_Any_object = jni->GetFieldID(
        static_cast<jclass>(jo_Any.get()), "_object", "Ljava/lang/Object;" );
    jni.ensure_no_exception();
    assert( m_field_Any_object != nullptr );

    // method IEnvironment.getRegisteredInterface()
    m_method_IEnvironment_getRegisteredInterface = jni->GetMethodID(
        static_cast<jclass>(jo_IEnvironment.get()),
        "getRegisteredInterface",
        "(Ljava/lang/String;Lcom/sun/star/uno/Type;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    assert( m_method_IEnvironment_getRegisteredInterface != nullptr );
    // method IEnvironment.registerInterface()
    m_method_IEnvironment_registerInterface = jni->GetMethodID(
        static_cast<jclass>(jo_IEnvironment.get()), "registerInterface",
        "(Ljava/lang/Object;[Ljava/lang/String;Lcom/sun/star/uno/Type;)"
        "Ljava/lang/Object;" );
    jni.ensure_no_exception();
    assert( m_method_IEnvironment_registerInterface != nullptr );

    // static method JNI_proxy.get_proxy_ctor()
    m_method_JNI_proxy_get_proxy_ctor = jni->GetStaticMethodID(
        static_cast<jclass>(jo_JNI_proxy.get()), "get_proxy_ctor",
        "(Ljava/lang/Class;)Ljava/lang/reflect/Constructor;" );
    jni.ensure_no_exception();
    assert( m_method_JNI_proxy_get_proxy_ctor != nullptr );
    // static method JNI_proxy.create()
    m_method_JNI_proxy_create = jni->GetStaticMethodID(
        static_cast<jclass>(jo_JNI_proxy.get()), "create",
        "(JLcom/sun/star/uno/IEnvironment;JJLcom/sun/star/uno/Type;Ljava/lang"
        "/String;Ljava/lang/reflect/Constructor;"
        "Lcom/sun/star/lib/util/AsynchronousFinalizer;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    assert( m_method_JNI_proxy_create != nullptr );
    // field JNI_proxy.m_receiver_handle
    m_field_JNI_proxy_m_receiver_handle = jni->GetFieldID(
        static_cast<jclass>(jo_JNI_proxy.get()), "m_receiver_handle", "J" );
    jni.ensure_no_exception();
    assert( m_field_JNI_proxy_m_receiver_handle != nullptr );
    // field JNI_proxy.m_td_handle
    m_field_JNI_proxy_m_td_handle = jni->GetFieldID(
        static_cast<jclass>(jo_JNI_proxy.get()), "m_td_handle", "J" );
    jni.ensure_no_exception();
    assert( m_field_JNI_proxy_m_td_handle != nullptr );
    // field JNI_proxy.m_type
    m_field_JNI_proxy_m_type = jni->GetFieldID(
        static_cast<jclass>(jo_JNI_proxy.get()), "m_type", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    assert( m_field_JNI_proxy_m_type != nullptr );
    // field JNI_proxy.m_oid
    m_field_JNI_proxy_m_oid = jni->GetFieldID(
        static_cast<jclass>(jo_JNI_proxy.get()), "m_oid", "Ljava/lang/String;" );
    jni.ensure_no_exception();
    assert( m_field_JNI_proxy_m_oid != nullptr );

    // ctor AsynchronousFinalizer
    m_ctor_AsynchronousFinalizer = jni->GetMethodID(
        static_cast<jclass>(jo_AsynchronousFinalizer.get()), "<init>", "()V" );
    jni.ensure_no_exception();
    assert( m_ctor_AsynchronousFinalizer != nullptr );
    // method AsynchronousFinalizer.drain()
    m_method_AsynchronousFinalizer_drain = jni->GetMethodID(
        static_cast<jclass>(jo_AsynchronousFinalizer.get()), "drain", "()V" );
    jni.ensure_no_exception();
    assert( m_method_AsynchronousFinalizer_drain != nullptr );

    // get java env
    OUString java_env_type_name( u"" UNO_LB_JAVA ""_ustr );
    JLocalAutoRef jo_java(
        jni, ustring_to_jstring( jni, java_env_type_name.pData ) );
    jvalue args[ 2 ];
    args[ 0 ].l = jo_java.get();
    args[ 1 ].l = nullptr;
    jmethodID method_getEnvironment = jni->GetStaticMethodID(
        static_cast<jclass>(jo_UnoRuntime.get()), "getEnvironment",
        "(Ljava/lang/String;Ljava/lang/Object;)"
        "Lcom/sun/star/uno/IEnvironment;" );
    jni.ensure_no_exception();
    assert( method_getEnvironment != nullptr );
    JLocalAutoRef jo_java_env(
        jni, jni->CallStaticObjectMethodA(
            static_cast<jclass>(jo_UnoRuntime.get()), method_getEnvironment, args ) );

    // get com.sun.star.uno.Any.VOID
    jfieldID field_Any_VOID = jni->GetStaticFieldID(
        static_cast<jclass>(jo_Any.get()), "VOID", "Lcom/sun/star/uno/Any;" );
    jni.ensure_no_exception();
    assert( field_Any_VOID != nullptr );
    JLocalAutoRef jo_Any_VOID(
        jni, jni->GetStaticObjectField(
            static_cast<jclass>(jo_Any.get()), field_Any_VOID ) );
    // get com.sun.star.uno.Type.UNSIGNED_SHORT
    jfieldID field_Type_UNSIGNED_SHORT = jni->GetStaticFieldID(
        static_cast<jclass>(jo_Type.get()), "UNSIGNED_SHORT", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    assert( field_Type_UNSIGNED_SHORT != nullptr );
    JLocalAutoRef jo_Type_UNSIGNED_SHORT(
        jni, jni->GetStaticObjectField(
            static_cast<jclass>(jo_Type.get()), field_Type_UNSIGNED_SHORT ) );
    // get com.sun.star.uno.Type.UNSIGNED_LONG
    jfieldID field_Type_UNSIGNED_LONG = jni->GetStaticFieldID(
        static_cast<jclass>(jo_Type.get()), "UNSIGNED_LONG", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    assert( field_Type_UNSIGNED_LONG != nullptr );
    JLocalAutoRef jo_Type_UNSIGNED_LONG(
        jni, jni->GetStaticObjectField(
            static_cast<jclass>(jo_Type.get()), field_Type_UNSIGNED_LONG ) );
    // get com.sun.star.uno.Type.UNSIGNED_HYPER
    jfieldID field_Type_UNSIGNED_HYPER = jni->GetStaticFieldID(
        static_cast<jclass>(jo_Type.get()), "UNSIGNED_HYPER", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    assert( field_Type_UNSIGNED_HYPER != nullptr );
    JLocalAutoRef jo_Type_UNSIGNED_HYPER(
        jni, jni->GetStaticObjectField(
            static_cast<jclass>(jo_Type.get()), field_Type_UNSIGNED_HYPER ) );

    // make global refs
    m_class_UnoRuntime =
        static_cast<jclass>(jni->NewGlobalRef( jo_UnoRuntime.get() ));
    m_class_RuntimeException =
        static_cast<jclass>(jni->NewGlobalRef( jo_RuntimeException.get() ));
    m_class_Any =
        static_cast<jclass>(jni->NewGlobalRef( jo_Any.get() ));
    m_class_Type =
        static_cast<jclass>(jni->NewGlobalRef( jo_Type.get() ));
    m_class_TypeClass =
        static_cast<jclass>(jni->NewGlobalRef( jo_TypeClass.get() ));
    m_class_JNI_proxy =
        static_cast<jclass>(jni->NewGlobalRef( jo_JNI_proxy.get() ));
    m_class_AsynchronousFinalizer =
        static_cast<jclass>(jni->NewGlobalRef( jo_AsynchronousFinalizer.get() ));

    m_class_Character =
        static_cast<jclass>(jni->NewGlobalRef( jo_Character.get() ));
    m_class_Boolean =
        static_cast<jclass>(jni->NewGlobalRef( jo_Boolean.get() ));
    m_class_Byte =
        static_cast<jclass>(jni->NewGlobalRef( jo_Byte.get() ));
    m_class_Short =
        static_cast<jclass>(jni->NewGlobalRef( jo_Short.get() ));
    m_class_Integer =
        static_cast<jclass>(jni->NewGlobalRef( jo_Integer.get() ));
    m_class_Long =
        static_cast<jclass>(jni->NewGlobalRef( jo_Long.get() ));
    m_class_Float =
        static_cast<jclass>(jni->NewGlobalRef( jo_Float.get() ));
    m_class_Double =
        static_cast<jclass>(jni->NewGlobalRef( jo_Double.get() ));
    m_class_String =
        static_cast<jclass>(jni->NewGlobalRef( jo_String.get() ));
    m_class_Object =
        static_cast<jclass>(jni->NewGlobalRef( jo_Object.get() ));
    m_class_Class =
        static_cast<jclass>(jni->NewGlobalRef( m_class_Class ));

    m_object_Any_VOID =
        jni->NewGlobalRef( jo_Any_VOID.get() );
    m_object_Type_UNSIGNED_SHORT =
        jni->NewGlobalRef( jo_Type_UNSIGNED_SHORT.get() );
    m_object_Type_UNSIGNED_LONG =
        jni->NewGlobalRef( jo_Type_UNSIGNED_LONG.get() );
    m_object_Type_UNSIGNED_HYPER =
        jni->NewGlobalRef( jo_Type_UNSIGNED_HYPER.get() );
    m_object_java_env = jni->NewGlobalRef( jo_java_env.get() );

    try
    {
        css::uno::TypeDescription XInterface_td(
            cppu::UnoType<css::uno::XInterface>::get());
        // coverity[ctor_dtor_leak] - on purpose
        m_XInterface_type_info =
            new JNI_interface_type_info( jni, XInterface_td.get() );
    }
    catch (...)
    {
        destruct( jni_env );
        throw;
    }
}


void JNI_info::destruct( JNIEnv * jni_env )
{
    for (auto & i: m_type_map)
    {
        i.second.m_info->destroy( jni_env );
    }
    if (m_XInterface_type_info != nullptr)
    {
        const_cast< JNI_interface_type_info * >(
            m_XInterface_type_info )->destroy( jni_env );
    }

    // free global refs
    jni_env->DeleteGlobalRef( m_object_java_env );
    jni_env->DeleteGlobalRef( m_object_Any_VOID );
    jni_env->DeleteGlobalRef( m_object_Type_UNSIGNED_SHORT );
    jni_env->DeleteGlobalRef( m_object_Type_UNSIGNED_LONG );
    jni_env->DeleteGlobalRef( m_object_Type_UNSIGNED_HYPER );

    jni_env->DeleteGlobalRef( m_class_Class );
    jni_env->DeleteGlobalRef( m_class_Object );
    jni_env->DeleteGlobalRef( m_class_String );
    jni_env->DeleteGlobalRef( m_class_Double );
    jni_env->DeleteGlobalRef( m_class_Float );
    jni_env->DeleteGlobalRef( m_class_Long );
    jni_env->DeleteGlobalRef( m_class_Integer );
    jni_env->DeleteGlobalRef( m_class_Short );
    jni_env->DeleteGlobalRef( m_class_Byte );
    jni_env->DeleteGlobalRef( m_class_Boolean );
    jni_env->DeleteGlobalRef( m_class_Character );

    jni_env->DeleteGlobalRef( m_class_AsynchronousFinalizer );
    jni_env->DeleteGlobalRef( m_class_JNI_proxy );
    jni_env->DeleteGlobalRef( m_class_RuntimeException );
    jni_env->DeleteGlobalRef( m_class_UnoRuntime );
    jni_env->DeleteGlobalRef( m_class_TypeClass );
    jni_env->DeleteGlobalRef( m_class_Type );
    jni_env->DeleteGlobalRef( m_class_Any );
}


JNI_info const * JNI_info::get_jni_info(
    rtl::Reference< jvmaccess::UnoVirtualMachine > const & uno_vm )
{
    // !!!no JNI_info available at JNI_context!!!
    ::jvmaccess::VirtualMachine::AttachGuard guard(
        uno_vm->getVirtualMachine() );
    JNIEnv * jni_env = guard.getEnvironment();
    JNI_context jni(
        nullptr, jni_env, static_cast< jobject >(uno_vm->getClassLoader()) );

    jclass jo_class;
    jmethodID jo_forName;
    jni.getClassForName( &jo_class, &jo_forName );
    jni.ensure_no_exception();
    JLocalAutoRef jo_JNI_info_holder(
        jni,
        jni.findClass(
            "com.sun.star.bridges.jni_uno.JNI_info_holder", jo_class,
            jo_forName, false ) );
    // field JNI_info_holder.m_jni_info_handle
    jfieldID field_s_jni_info_handle =
        jni->GetStaticFieldID(
            static_cast<jclass>(jo_JNI_info_holder.get()), "s_jni_info_handle", "J" );
    jni.ensure_no_exception();
    assert( field_s_jni_info_handle != nullptr );

    JNI_info const * jni_info =
        reinterpret_cast< JNI_info const * >(
            jni->GetStaticLongField(
                static_cast<jclass>(jo_JNI_info_holder.get()), field_s_jni_info_handle ) );
    if (jni_info == nullptr) // un-initialized?
    {
        JNI_info * new_info = new JNI_info(
            jni_env, static_cast< jobject >(uno_vm->getClassLoader()), jo_class,
            jo_forName );

        osl::ClearableMutexGuard g( osl::Mutex::getGlobalMutex() );
        jni_info =
            reinterpret_cast< JNI_info const * >(
                jni->GetStaticLongField(
                    static_cast<jclass>(jo_JNI_info_holder.get()),
                    field_s_jni_info_handle ) );
        if (jni_info == nullptr) // still un-initialized?
        {
            jni->SetStaticLongField(
                static_cast<jclass>(jo_JNI_info_holder.get()), field_s_jni_info_handle,
                reinterpret_cast< jlong >( new_info ) );
            jni_info = new_info;
        }
        else
        {
            g.clear();
            new_info->destroy( jni_env );
        }
    }

    return jni_info;
}

}

extern "C"
{


SAL_JNI_EXPORT void
JNICALL Java_com_sun_star_bridges_jni_1uno_JNI_1info_1holder_finalize__J(
    JNIEnv * jni_env, SAL_UNUSED_PARAMETER jobject, jlong jni_info_handle )
    SAL_THROW_EXTERN_C()
{
    ::jni_uno::JNI_info * jni_info =
          reinterpret_cast< ::jni_uno::JNI_info * >( jni_info_handle );
    jni_info->destroy( jni_env );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
