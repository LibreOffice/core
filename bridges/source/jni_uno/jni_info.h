/*************************************************************************
 *
 *  $RCSfile: jni_info.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-10-28 18:20:26 $
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
#ifndef _JNI_INFO_H_
#define _JNI_INFO_H_

#include <jni.h>
#include <hash_map>

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

#include <uno/environment.h>
#include <typelib/typedescription.hxx>

#include <com/sun/star/uno/Type.hxx>


namespace jni_bridge
{

class JNI_attach;
class JNI_info;
//==================================================================================================
struct JNI_type_info
{
    ::com::sun::star::uno::TypeDescription      m_td;
    jclass                                      m_class;
    JNI_type_info const *                       m_base;
    jobject                                     m_jo_type; // is unused (0) for compound types
    // sorted via typelib function index
    jmethodID *                                 m_methods;
    // sorted via typelib member index
    jfieldID *                                  m_fields;
    jmethodID                                   m_ctor;

    JNI_type_info(
        JNI_attach const & attach, typelib_InterfaceTypeDescription * td );
    JNI_type_info(
        JNI_attach const & attach, typelib_CompoundTypeDescription * td );

    static void _delete( JNI_attach const & attach, JNI_type_info * that ) SAL_THROW( () );

//     // returns unacquired ref of demanded member
//     typelib_TypeDescriptionReference * get_member(
//         ::rtl::OUString const & fully_qualified_member_name ) const;
};
//==================================================================================================
struct JNI_type_info_holder
{
    JNI_type_info * m_info;
    inline JNI_type_info_holder() SAL_THROW( () )
        : m_info( 0 )
        {}
};
//==================================================================================================
typedef ::std::hash_map< ::rtl::OUString, JNI_type_info_holder, ::rtl::OUStringHash > t_str2type;

//==================================================================================================
class JNI_info
{
    uno_Environment *           m_java_env;
    mutable ::osl::Mutex        m_mutex;
    mutable t_str2type          m_type_map;

public:
    //
    ::com::sun::star::uno::TypeDescription m_XInterface;
    ::com::sun::star::uno::Type const & m_Exception;
    ::com::sun::star::uno::Type const & m_RuntimeException;

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
    jmethodID                   m_method_Object_getClass;
    jmethodID                   m_method_Object_equals;
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

    jmethodID                   m_method_JNI_proxy_create;
    jfieldID                    m_field_JNI_proxy_m_receiver_handle;
    jfieldID                    m_field_JNI_proxy_m_td_handle;
    jfieldID                    m_field_JNI_proxy_m_type;
    jfieldID                    m_field_JNI_proxy_m_oid;

    jmethodID                   m_method_IEnvironment_getRegisteredInterface;
    jmethodID                   m_method_IEnvironment_registerInterface;
    jmethodID                   m_method_IEnvironment_revokeInterface;
    //
    jobject                     m_object_java_env;

    // xxx todo: opt inline?
    jobject java_env_getRegisteredInterface(
        JNI_attach const & attach, jstring oid, jobject type ) const;
    jobject java_env_registerInterface(
        JNI_attach const & attach, jobject javaI, jstring oid, jobject type ) const;
    void java_env_revokeInterface(
        JNI_attach const & attach, jstring oid, jobject type ) const;

    //
    JNI_info( uno_Environment * java_env );
    ~JNI_info() SAL_THROW( () );

    //
    JNI_type_info const * get_type_info(
        JNI_attach const & attach, typelib_TypeDescription * td ) const;
    //
    void append_sig(
        ::rtl::OStringBuffer * buf, typelib_TypeDescriptionReference * type ) const;
};

}

#endif
