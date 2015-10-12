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

#ifndef INCLUDED_BRIDGES_SOURCE_JNI_UNO_JNI_INFO_H
#define INCLUDED_BRIDGES_SOURCE_JNI_UNO_JNI_INFO_H

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include "jni_base.h"

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"

#include "uno/environment.h"
#include "typelib/typedescription.hxx"

#include "com/sun/star/uno/Type.hxx"

namespace jvmaccess { class UnoVirtualMachine; }

namespace jni_uno
{

inline bool type_equals(
    typelib_TypeDescriptionReference * type1,
    typelib_TypeDescriptionReference * type2 )
{
    if (type1 == type2)
        return true;
    OUString const & name1 =
          OUString::unacquired( &type1->pTypeName );
    OUString const & name2 =
          OUString::unacquired( &type2->pTypeName );
    return ((type1->eTypeClass == type2->eTypeClass) && name1.equals( name2 ));
}

inline bool is_XInterface( typelib_TypeDescriptionReference * type )
{
    return ((typelib_TypeClass_INTERFACE == type->eTypeClass) &&
            OUString::unacquired( &type->pTypeName ) == "com.sun.star.uno.XInterface");
}

struct JNI_type_info: private boost::noncopyable
{
    ::com::sun::star::uno::TypeDescription      m_td;
    jclass                                      m_class;

    virtual void destroy( JNIEnv * jni_env ) = 0;
protected:
    inline void destruct( JNIEnv * jni_env )
        { jni_env->DeleteGlobalRef( m_class ); }
    virtual inline ~JNI_type_info() {}
    explicit JNI_type_info(
        JNI_context const & jni, typelib_TypeDescription * td );
};

struct JNI_interface_type_info : public JNI_type_info
{
    jobject                                     m_proxy_ctor; // proxy ctor
    jobject                                     m_type;
    // sorted via typelib function index
    jmethodID *                                 m_methods;

    virtual void destroy( JNIEnv * jni_env ) override;
    explicit JNI_interface_type_info(
        JNI_context const & jni, typelib_TypeDescription * td );

private:
    virtual ~JNI_interface_type_info() {}
};

struct JNI_compound_type_info : public JNI_type_info
{
    JNI_type_info const *                       m_base;
    // ctor( msg ) for exceptions
    jmethodID                                   m_exc_ctor;
    // sorted via typelib member index
    jfieldID *                                  m_fields;

    virtual void destroy( JNIEnv * jni_env ) override;
    explicit JNI_compound_type_info(
        JNI_context const & jni, typelib_TypeDescription * td );

private:
    virtual ~JNI_compound_type_info() {}
};

struct JNI_type_info_holder: private boost::noncopyable
{
    JNI_type_info * m_info;
    inline JNI_type_info_holder()
        : m_info( 0 )
        {}
};

typedef ::boost::unordered_map<
    OUString, JNI_type_info_holder, OUStringHash > t_str2type;

class JNI_info: private boost::noncopyable
{
    mutable ::osl::Mutex        m_mutex;
    mutable t_str2type          m_type_map;

public:
    // These two are needed very early by find_class from within the ctor:
    jclass                      m_class_Class;
    jmethodID                   m_method_Class_forName;

    jobject                     m_object_java_env;
    jobject                     m_object_Any_VOID;
    jobject                     m_object_Type_UNSIGNED_SHORT;
    jobject                     m_object_Type_UNSIGNED_LONG;
    jobject                     m_object_Type_UNSIGNED_HYPER;

    jclass                      m_class_Object;
    jclass                      m_class_Character;
    jclass                      m_class_Boolean;
    jclass                      m_class_Byte;
    jclass                      m_class_Short;
    jclass                      m_class_Integer;
    jclass                      m_class_Long;
    jclass                      m_class_Float;
    jclass                      m_class_Double;
    jclass                      m_class_String;

    jclass                      m_class_UnoRuntime;
    jclass                      m_class_RuntimeException;
    jclass                      m_class_Any;
    jclass                      m_class_Type;
    jclass                      m_class_TypeClass;
    jclass                      m_class_JNI_proxy;
    jclass                      m_class_AsynchronousFinalizer;

    jmethodID                   m_method_Object_toString;
    jmethodID                   m_method_Class_getName;
    jmethodID                   m_method_Throwable_getMessage;
    jmethodID                   m_ctor_Character_with_char;
    jmethodID                   m_ctor_Boolean_with_boolean;
    jmethodID                   m_ctor_Byte_with_byte;
    jmethodID                   m_ctor_Short_with_short;
    jmethodID                   m_ctor_Integer_with_int;
    jmethodID                   m_ctor_Long_with_long;
    jmethodID                   m_ctor_Float_with_float;
    jmethodID                   m_ctor_Double_with_double;
    jmethodID                   m_method_Boolean_booleanValue;
    jmethodID                   m_method_Byte_byteValue;
    jmethodID                   m_method_Character_charValue;
    jmethodID                   m_method_Double_doubleValue;
    jmethodID                   m_method_Float_floatValue;
    jmethodID                   m_method_Integer_intValue;
    jmethodID                   m_method_Long_longValue;
    jmethodID                   m_method_Short_shortValue;

    jmethodID                   m_method_IEnvironment_getRegisteredInterface;
    jmethodID                   m_method_IEnvironment_registerInterface;
    jmethodID                   m_method_UnoRuntime_generateOid;
    jmethodID                   m_method_UnoRuntime_queryInterface;
    jmethodID                   m_ctor_Any_with_Type_Object;
    jfieldID                    m_field_Any__type;
    jfieldID                    m_field_Any__object;
    jmethodID                   m_ctor_Type_with_Class;
    jmethodID                   m_ctor_Type_with_Name_TypeClass;
    jfieldID                    m_field_Type__typeName;
    jmethodID                   m_method_TypeClass_fromInt;
    jfieldID                    m_field_Enum_m_value;

    jmethodID                   m_method_JNI_proxy_get_proxy_ctor;
    jmethodID                   m_method_JNI_proxy_create;
    jfieldID                    m_field_JNI_proxy_m_receiver_handle;
    jfieldID                    m_field_JNI_proxy_m_td_handle;
    jfieldID                    m_field_JNI_proxy_m_type;
    jfieldID                    m_field_JNI_proxy_m_oid;

    jmethodID                   m_ctor_AsynchronousFinalizer;
    jmethodID                   m_method_AsynchronousFinalizer_drain;

    ::com::sun::star::uno::TypeDescription m_XInterface_queryInterface_td;
    ::com::sun::star::uno::Type const & m_Exception_type;
    ::com::sun::star::uno::Type const & m_RuntimeException_type;
    ::com::sun::star::uno::Type const & m_void_type;
    JNI_interface_type_info const * m_XInterface_type_info;

    JNI_type_info const * get_type_info(
        JNI_context const & jni,
        typelib_TypeDescription * type ) const;
    JNI_type_info const * get_type_info(
        JNI_context const & jni,
        typelib_TypeDescriptionReference * type ) const;
    JNI_type_info const * get_type_info(
        JNI_context const & jni,
        OUString const & uno_name ) const;
    inline static void append_sig(
        OStringBuffer * buf, typelib_TypeDescriptionReference * type,
        bool use_Object_for_type_XInterface = true, bool use_slashes = true );

    // get this
    static JNI_info const * get_jni_info(
        rtl::Reference< jvmaccess::UnoVirtualMachine > const & uno_vm );
    inline void destroy( JNIEnv * jni_env );

private:
    JNI_type_info const * create_type_info(
        JNI_context const & jni, typelib_TypeDescription * td ) const;

    void destruct( JNIEnv * jni_env );

    JNI_info( JNIEnv * jni_env, jobject class_loader,
              jclass classClass, jmethodID methodForName );
    inline ~JNI_info() {}
};

inline void JNI_info::destroy( JNIEnv * jni_env )
{
    destruct( jni_env );
    delete this;
}

inline void JNI_info::append_sig(
    OStringBuffer * buf, typelib_TypeDescriptionReference * type,
    bool use_Object_for_type_XInterface, bool use_slashes )
{
    switch (type->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        buf->append( 'V' );
        break;
    case typelib_TypeClass_CHAR:
        buf->append( 'C' );
        break;
    case typelib_TypeClass_BOOLEAN:
        buf->append( 'Z' );
        break;
    case typelib_TypeClass_BYTE:
        buf->append( 'B' );
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        buf->append( 'S' );
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        buf->append( 'I' );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        buf->append( 'J' );
        break;
    case typelib_TypeClass_FLOAT:
        buf->append( 'F' );
        break;
    case typelib_TypeClass_DOUBLE:
        buf->append( 'D' );
        break;
    case typelib_TypeClass_STRING:
        if ( use_slashes ) {
            buf->append( "Ljava/lang/String;" );
        } else {
            buf->append( "Ljava.lang.String;" );
        }
        break;
    case typelib_TypeClass_TYPE:
        if ( use_slashes ) {
            buf->append( "Lcom/sun/star/uno/Type;" );
        } else {
            buf->append( "Lcom.sun.star.uno.Type;" );
        }
        break;
    case typelib_TypeClass_ANY:
        if ( use_slashes ) {
            buf->append( "Ljava/lang/Object;" );
        } else {
            buf->append( "Ljava.lang.Object;" );
        }
        break;
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        OUString const & uno_name =
              OUString::unacquired( &type->pTypeName );
        buf->append( 'L' );
        // Erase type arguments of instantiated polymorphic struct types:
        sal_Int32 i = uno_name.indexOf( '<' );
        if ( i < 0 ) {
            buf->append(
                OUStringToOString(
                    use_slashes ? uno_name.replace( '.', '/' ) : uno_name,
                    RTL_TEXTENCODING_JAVA_UTF8 ) );
        } else {
            OUString s( uno_name.copy( 0, i ) );
            buf->append(
                OUStringToOString(
                    use_slashes ? s.replace( '.', '/' ) : s,
                    RTL_TEXTENCODING_JAVA_UTF8 ) );
        }
        buf->append( ';' );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        buf->append( '[' );
        TypeDescr td( type );
        append_sig(
            buf, reinterpret_cast<typelib_IndirectTypeDescription *>(td.get())->pType,
            use_Object_for_type_XInterface, use_slashes );
        break;
    }
    case typelib_TypeClass_INTERFACE:
        if (use_Object_for_type_XInterface && is_XInterface( type ))
        {
            if ( use_slashes ) {
                buf->append( "Ljava/lang/Object;" );
            } else {
                buf->append( "Ljava.lang.Object;" );
            }
        }
        else
        {
            OUString const & uno_name =
                  OUString::unacquired( &type->pTypeName );
            buf->append( 'L' );
            buf->append(
                OUStringToOString(
                    use_slashes ? uno_name.replace( '.', '/' ) : uno_name,
                    RTL_TEXTENCODING_JAVA_UTF8 ) );
            buf->append( ';' );
        }
        break;
    default:
        throw BridgeRuntimeError(
            "unsupported type: " +
            OUString::unacquired( &type->pTypeName ) );
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
