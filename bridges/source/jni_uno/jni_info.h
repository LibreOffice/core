/*************************************************************************
 *
 *  $RCSfile: jni_info.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:50:34 $
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

#if ! defined INCLUDED_JNI_INFO_H
#define INCLUDED_JNI_INFO_H

#include <hash_map>

#include "jni_base.h"

#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"

#include "uno/environment.h"
#include "typelib/typedescription.hxx"

#include "com/sun/star/uno/Type.hxx"


namespace jni_uno
{

//------------------------------------------------------------------------------
inline bool type_equals(
    typelib_TypeDescriptionReference * type1,
    typelib_TypeDescriptionReference * type2 )
{
    if (type1 == type2)
        return true;
    ::rtl::OUString const & name1 =
          ::rtl::OUString::unacquired( &type1->pTypeName );
    ::rtl::OUString const & name2 =
          ::rtl::OUString::unacquired( &type2->pTypeName );
    return ((type1->eTypeClass == type2->eTypeClass) && name1.equals( name2 ));
}

//------------------------------------------------------------------------------
inline bool is_XInterface( typelib_TypeDescriptionReference * type )
{
    return ((typelib_TypeClass_INTERFACE == type->eTypeClass) &&
            ::rtl::OUString::unacquired( &type->pTypeName ).equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.uno.XInterface") ));
}

//==============================================================================
struct JNI_type_info
{
    JNI_type_info const *                       m_base;
    ::com::sun::star::uno::TypeDescription      m_td;
    jclass                                      m_class;

    virtual void destroy( JNIEnv * jni_env ) = 0;
protected:
    inline void destruct( JNIEnv * jni_env )
        { jni_env->DeleteGlobalRef( m_class ); }
    inline ~JNI_type_info() {}
    explicit JNI_type_info(
        JNI_context const & jni, typelib_TypeDescription * td );
};

//==============================================================================
struct JNI_interface_type_info : public JNI_type_info
{
    jobject                                     m_proxy_ctor; // proxy ctor
    jobject                                     m_type;
    // sorted via typelib function index
    jmethodID *                                 m_methods;

    virtual void destroy( JNIEnv * jni_env );
    explicit JNI_interface_type_info(
        JNI_context const & jni, typelib_TypeDescription * td );
};

//==============================================================================
struct JNI_compound_type_info : public JNI_type_info
{
    // ctor( msg ) for exceptions
    jmethodID                                   m_exc_ctor;
    // sorted via typelib member index
    jfieldID *                                  m_fields;

    virtual void destroy( JNIEnv * jni_env );
    explicit JNI_compound_type_info(
        JNI_context const & jni, typelib_TypeDescription * td );
};

//==============================================================================
struct JNI_type_info_holder
{
    JNI_type_info * m_info;
    inline JNI_type_info_holder()
        : m_info( 0 )
        {}
};

typedef ::std::hash_map<
    ::rtl::OUString, JNI_type_info_holder, ::rtl::OUStringHash > t_str2type;

//==============================================================================
class JNI_info
{
    mutable ::osl::Mutex        m_mutex;
    mutable t_str2type          m_type_map;

public:
    //
    jobject                     m_object_java_env;
    jobject                     m_object_Any_VOID;
    jobject                     m_object_Type_UNSIGNED_SHORT;
    jobject                     m_object_Type_UNSIGNED_LONG;
    jobject                     m_object_Type_UNSIGNED_HYPER;

    //
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

    //
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

    //
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

    //
    jmethodID                   m_method_JNI_proxy_get_proxy_ctor;
    jmethodID                   m_method_JNI_proxy_create;
    jfieldID                    m_field_JNI_proxy_m_receiver_handle;
    jfieldID                    m_field_JNI_proxy_m_td_handle;
    jfieldID                    m_field_JNI_proxy_m_type;
    jfieldID                    m_field_JNI_proxy_m_oid;

    //
    ::com::sun::star::uno::TypeDescription m_XInterface_queryInterface_td;
    ::com::sun::star::uno::Type const & m_Exception_type;
    ::com::sun::star::uno::Type const & m_RuntimeException_type;
    ::com::sun::star::uno::Type const & m_void_type;
    //
    JNI_interface_type_info const * m_XInterface_type_info;

    //
    JNI_type_info const * get_type_info(
        JNI_context const & jni,
        typelib_TypeDescription * type ) const;
    JNI_type_info const * get_type_info(
        JNI_context const & jni,
        typelib_TypeDescriptionReference * type ) const;
    JNI_type_info const * get_type_info(
        JNI_context const & jni,
        ::rtl::OUString const & uno_name ) const;
    //
    inline static void append_sig(
        ::rtl::OStringBuffer * buf, typelib_TypeDescriptionReference * type,
        bool use_Object_for_type_XInterface = true );

    // get this
    static JNI_info const * get_jni_info( JNIEnv * jni_env );
    inline void destroy( JNIEnv * jni_env );

private:
    JNI_type_info const * create_type_info(
        JNI_context const & jni, typelib_TypeDescription * td ) const;

    void destruct( JNIEnv * jni_env );

    explicit JNI_info( JNIEnv * jni_env );
    inline ~JNI_info() {}
};

//______________________________________________________________________________
inline void JNI_info::destroy( JNIEnv * jni_env )
{
    destruct( jni_env );
    delete this;
}

//______________________________________________________________________________
inline void JNI_info::append_sig(
    ::rtl::OStringBuffer * buf, typelib_TypeDescriptionReference * type,
    bool use_Object_for_type_XInterface )
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
        buf->append( RTL_CONSTASCII_STRINGPARAM("Ljava/lang/String;") );
        break;
    case typelib_TypeClass_TYPE:
        buf->append( RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/Type;") );
        break;
    case typelib_TypeClass_ANY:
        buf->append( RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;") );
        break;
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        ::rtl::OUString const & uno_name =
              ::rtl::OUString::unacquired( &type->pTypeName );
        buf->append( 'L' );
        buf->append(
            ::rtl::OUStringToOString(
                uno_name.replace( '.', '/' ), RTL_TEXTENCODING_JAVA_UTF8 ) );
        buf->append( ';' );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        buf->append( '[' );
        TypeDescr td( type );
        append_sig(
            buf, ((typelib_IndirectTypeDescription *)td.get())->pType,
            use_Object_for_type_XInterface );
        break;
    }
    case typelib_TypeClass_INTERFACE:
        if (use_Object_for_type_XInterface && is_XInterface( type ))
        {
            buf->append( RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;") );
        }
        else
        {
            ::rtl::OUString const & uno_name =
                  ::rtl::OUString::unacquired( &type->pTypeName );
            buf->append( 'L' );
            buf->append(
                ::rtl::OUStringToOString(
                    uno_name.replace( '.', '/' ),
                    RTL_TEXTENCODING_JAVA_UTF8 ) );
            buf->append( ';' );
        }
        break;
    default:
        throw BridgeRuntimeError(
            OUSTR("unsupported type: ") +
            ::rtl::OUString::unacquired( &type->pTypeName ) );
    }
}

}

#endif
