/*************************************************************************
 *
 *  $RCSfile: jni_info.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2002-10-29 10:55:07 $
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

#include "jni_bridge.h"

#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>


using namespace ::std;
using namespace ::osl;
using namespace ::rtl;

namespace jni_bridge
{

//__________________________________________________________________________________________________
JNI_type_info::JNI_type_info(
    JNI_attach const & attach, typelib_InterfaceTypeDescription * td )
    : m_td( (typelib_TypeDescription *)td ),
      m_methods( 0 ),
      m_fields( 0 )
{
    m_td.makeComplete();
    if (! m_td.get()->bComplete)
    {
        throw BridgeRuntimeError(
            OUSTR("cannot make type incomplete: ") +
            *reinterpret_cast< OUString const * >( &m_td.get()->pTypeName ) );
    }
    td = (typelib_InterfaceTypeDescription *)m_td.get();

    OUString const & uno_name = *reinterpret_cast< OUString const * >(
        &((typelib_TypeDescription *)td)->pTypeName );

    OString java_name(
        OUStringToOString( uno_name.replace( '.', '/' ), RTL_TEXTENCODING_ASCII_US ) );
    JLocalAutoRef jo_class( attach, find_class( attach, java_name.getStr() ) );

    JNI_info const * jni_info = attach.get_jni_info();

    // retrieve info for base type
    typelib_TypeDescription * base_td = (typelib_TypeDescription *)td->pBaseTypeDescription;
    m_base = (0 == base_td ? 0 : jni_info->get_type_info( attach, base_td ));

    // if ! XInterface
    if (! typelib_typedescriptionreference_equals(
            ((typelib_TypeDescription *)td)->pWeakRef, jni_info->m_XInterface.get()->pWeakRef ))
    {
        try
        {
            // retrieve method ids for all direct members
            m_methods = new jmethodID[ td->nMapFunctionIndexToMemberIndex ];
            sal_Int32 nMethodIndex = 0;
            typelib_TypeDescriptionReference ** ppMembers = td->ppMembers;
            sal_Int32 nMembers = td->nMembers;

            for ( sal_Int32 nPos = 0; nPos < nMembers; ++nPos )
            {
                TypeDescr member_td( ppMembers[ nPos ] );

                OStringBuffer sig_buf( 64 );

                if (typelib_TypeClass_INTERFACE_METHOD == member_td.get()->eTypeClass) // method
                {
                    typelib_InterfaceMethodTypeDescription * method_td =
                        (typelib_InterfaceMethodTypeDescription *)member_td.get();

                    sig_buf.append( '(' );
                    for ( sal_Int32 nPos = 0; nPos < method_td->nParams; ++nPos )
                    {
                        typelib_MethodParameter const & param = method_td->pParams[ nPos ];
                        if (param.bOut)
                            sig_buf.append( '[' );
                        jni_info->append_sig( &sig_buf, param.pTypeRef );
                    }
                    sig_buf.append( ')' );
                    jni_info->append_sig( &sig_buf, method_td->pReturnTypeRef );

                    OString method_signature( sig_buf.makeStringAndClear() );
                    OString method_name(
                        OUStringToOString(
                            *reinterpret_cast< OUString const * >( &method_td->aBase.pMemberName ),
                            RTL_TEXTENCODING_ASCII_US ) );

                    m_methods[ nMethodIndex ] = attach->GetMethodID(
                        (jclass)jo_class.get(), method_name.getStr(), method_signature.getStr() );
                    attach.ensure_no_exception();
                    OSL_ASSERT( 0 != m_methods[ nMethodIndex ] );
                    ++nMethodIndex;
                }
                else // attribute
                {
                    OSL_ASSERT(
                        typelib_TypeClass_INTERFACE_ATTRIBUTE == member_td.get()->eTypeClass );
                    typelib_InterfaceAttributeTypeDescription * attribute_td =
                        (typelib_InterfaceAttributeTypeDescription *)member_td.get();

                    // type sig
                    jni_info->append_sig( &sig_buf, attribute_td->pAttributeTypeRef );
                    OString type_sig( sig_buf.makeStringAndClear() );
                    sig_buf.ensureCapacity( 64 );
                    // member name
                    OUString const & member_name = *reinterpret_cast< OUString const * >(
                        &attribute_td->aBase.pMemberName );

                    // getter
                    sig_buf.append( RTL_CONSTASCII_STRINGPARAM("()") );
                    sig_buf.append( type_sig );
                    OString method_signature( sig_buf.makeStringAndClear() );
                    OUStringBuffer name_buf( 3 + member_name.getLength() );
                    name_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("get") );
                    name_buf.append( member_name );
                    OString method_name(
                        OUStringToOString(
                            name_buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
                    m_methods[ nMethodIndex ] = attach->GetMethodID(
                        (jclass)jo_class.get(), method_name.getStr(), method_signature.getStr() );
                    attach.ensure_no_exception();
                    OSL_ASSERT( 0 != m_methods[ nMethodIndex ] );
                    ++nMethodIndex;
                    if (! attribute_td->bReadOnly)
                    {
                        // setter
                        sig_buf.ensureCapacity( 64 );
                        sig_buf.append( '(' );
                        sig_buf.append( type_sig );
                        sig_buf.append( RTL_CONSTASCII_STRINGPARAM(")V") );
                        method_signature = sig_buf.makeStringAndClear();
                        name_buf.ensureCapacity( 3 + member_name.getLength() );
                        name_buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("set") );
                        name_buf.append( member_name );
                        method_name = OUStringToOString(
                            name_buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US );
                        m_methods[ nMethodIndex ] = attach->GetMethodID(
                            (jclass)jo_class.get(), method_name.getStr(),
                            method_signature.getStr() );
                        attach.ensure_no_exception();
                        OSL_ASSERT( 0 != m_methods[ nMethodIndex ] );
                        ++nMethodIndex;
                    }
                }
            }
        }
        catch (...)
        {
            delete [] m_methods;
            throw;
        }
    }

    JLocalAutoRef jo_type( attach, create_type( attach, (jclass)jo_class.get() ) );

    m_class = (jclass)attach->NewGlobalRef( jo_class.get() );
    m_jo_type = attach->NewGlobalRef( jo_type.get() );
}
//__________________________________________________________________________________________________
JNI_type_info::JNI_type_info(
    JNI_attach const & attach, typelib_CompoundTypeDescription * td )
    : m_td( (typelib_TypeDescription *)td ),
      m_methods( 0 ),
      m_fields( 0 )
{
    m_td.makeComplete();
    if (! m_td.get()->bComplete)
    {
        throw BridgeRuntimeError(
            OUSTR("cannot make type incomplete: ") +
            *reinterpret_cast< OUString const * >( &m_td.get()->pTypeName ) );
    }
    td = (typelib_CompoundTypeDescription *)m_td.get();

    OUString const & uno_name = *reinterpret_cast< OUString const * >(
        &((typelib_TypeDescription *)td)->pTypeName );

    OString java_name(
        OUStringToOString( uno_name.replace( '.', '/' ), RTL_TEXTENCODING_ASCII_US ) );
    JLocalAutoRef jo_class( attach, find_class( attach, java_name.getStr() ) );

    JNI_info const * jni_info = attach.get_jni_info();

    // retrieve ctor
    m_ctor = attach->GetMethodID(
        (jclass)jo_class.get(), "<init>",
        (typelib_TypeClass_EXCEPTION == m_td.get()->eTypeClass ? "(Ljava/lang/String;)V" : "()V") );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor );

    // retrieve info for base type
    typelib_TypeDescription * base_td = (typelib_TypeDescription *)td->pBaseTypeDescription;
    m_base = (0 == base_td ? 0 : jni_info->get_type_info( attach, base_td ));

    try
    {
        if (typelib_typedescriptionreference_equals(
                ((typelib_TypeDescription *)td)->pWeakRef,
                jni_info->m_Exception.getTypeLibType() ) ||
            typelib_typedescriptionreference_equals(
                ((typelib_TypeDescription *)td)->pWeakRef,
                jni_info->m_RuntimeException.getTypeLibType() ))
        {
            m_fields = new jfieldID[ 2 ];
            m_fields[ 0 ] = 0; // special Throwable.getMessage()
            // field Context
            m_fields[ 1 ] = attach->GetFieldID(
                (jclass)jo_class.get(), "Context", "Ljava/lang/Object;" );
            attach.ensure_no_exception();
            OSL_ASSERT( 0 != m_fields[ 1 ] );
        }
        else
        {
            // retrieve field ids for all direct members
            sal_Int32 nMembers = td->nMembers;
            m_fields = new jfieldID[ nMembers ];

            for ( sal_Int32 nPos = 0; nPos < nMembers; ++nPos )
            {
                OStringBuffer sig_buf( 32 );
                jni_info->append_sig( &sig_buf, td->ppTypeRefs[ nPos ] );
                OString sig( sig_buf.makeStringAndClear() );

                OString member_name(
                    OUStringToOString(
                        *reinterpret_cast< OUString const * >( &td->ppMemberNames[ nPos ] ),
                        RTL_TEXTENCODING_ASCII_US ) );

                m_fields[ nPos ] = attach->GetFieldID(
                    (jclass)jo_class.get(), member_name.getStr(), sig.getStr() );
                attach.ensure_no_exception();
                OSL_ASSERT( 0 != m_fields[ nPos ] );
            }
        }
    }
    catch (...)
    {
        delete [] m_fields;
        throw;
    }

    m_class = (jclass)attach->NewGlobalRef( jo_class.get() );
    m_jo_type = 0;
}
//__________________________________________________________________________________________________
void JNI_type_info::_delete( JNI_attach const & attach, JNI_type_info * that ) SAL_THROW( () )
{
    delete [] that->m_fields;
    delete [] that->m_methods;
    attach->DeleteGlobalRef( that->m_class );
    delete that;
}

//##################################################################################################

//__________________________________________________________________________________________________
JNI_type_info const * JNI_info::get_type_info(
    JNI_attach const & attach, typelib_TypeDescription * td ) const
{
    JNI_type_info * info;

    OUString const & uno_name = *reinterpret_cast< OUString const * >( &td->pTypeName );

    ClearableMutexGuard guard( m_mutex );
    t_str2type::const_iterator iFind( m_type_map.find( uno_name ) );
    if (m_type_map.end() == iFind)
    {
        guard.clear();

        JNI_type_info * new_info;
        switch (td->eTypeClass)
        {
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            new_info = new JNI_type_info( attach, (typelib_CompoundTypeDescription *)td );
            break;
        case typelib_TypeClass_INTERFACE:
            new_info = new JNI_type_info( attach, (typelib_InterfaceTypeDescription *)td );
            break;
        default:
            throw BridgeRuntimeError(
                OUSTR("unsupported: type info for ") +
                *reinterpret_cast< OUString const * >( &td->pTypeName ) );
        }

        // look again
        ClearableMutexGuard guard( m_mutex );
        JNI_type_info_holder & holder = m_type_map[ uno_name ];
        if (0 == holder.m_info) // new insertion
        {
            holder.m_info = new_info;
            guard.clear();
            info = new_info;
        }
        else // inserted in the meantime
        {
            info = holder.m_info;
            guard.clear();
            JNI_type_info::_delete( attach, new_info );
        }
    }
    else
    {
        info = iFind->second.m_info;
    }

    return info;
}
//__________________________________________________________________________________________________
JNI_info::JNI_info( uno_Environment * java_env )
    : m_java_env( java_env ), // unacquired pointer to owner
      m_XInterface(
          ::getCppuType(
              (::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > const *)0 ) ),
      m_Exception(
          ::getCppuType(
              (::com::sun::star::uno::Exception const *)0 ) ),
      m_RuntimeException(
          ::getCppuType(
              (::com::sun::star::uno::RuntimeException const *)0 ) ),
      m_class_JNI_proxy( 0 )
{
    JNI_attach attach( java_env );

    // class lookup
    JLocalAutoRef jo_Object(
        attach, find_class( attach, "java/lang/Object" ) );
    JLocalAutoRef jo_Class(
        attach, find_class( attach, "java/lang/Class" ) );
    JLocalAutoRef jo_Throwable(
        attach, find_class( attach, "java/lang/Throwable" ) );
    JLocalAutoRef jo_Character(
        attach, find_class( attach, "java/lang/Character" ) );
    JLocalAutoRef jo_Boolean(
        attach, find_class( attach, "java/lang/Boolean" ) );
    JLocalAutoRef jo_Byte(
        attach, find_class( attach, "java/lang/Byte" ) );
    JLocalAutoRef jo_Short(
        attach, find_class( attach, "java/lang/Short" ) );
    JLocalAutoRef jo_Integer(
        attach, find_class( attach, "java/lang/Integer" ) );
    JLocalAutoRef jo_Long(
        attach, find_class( attach, "java/lang/Long" ) );
    JLocalAutoRef jo_Float(
        attach, find_class( attach, "java/lang/Float" ) );
    JLocalAutoRef jo_Double(
        attach, find_class( attach, "java/lang/Double" ) );
    JLocalAutoRef jo_String(
        attach, find_class( attach, "java/lang/String" ) );
    JLocalAutoRef jo_RuntimeException(
        attach, find_class( attach, "com/sun/star/uno/RuntimeException" ) );
    JLocalAutoRef jo_UnoRuntime(
        attach, find_class( attach, "com/sun/star/uno/UnoRuntime" ) );
    JLocalAutoRef jo_Any(
        attach, find_class( attach, "com/sun/star/uno/Any" ) );
    JLocalAutoRef jo_Enum(
        attach, find_class( attach, "com/sun/star/uno/Enum" ) );
    JLocalAutoRef jo_Type(
        attach, find_class( attach, "com/sun/star/uno/Type" ) );
    JLocalAutoRef jo_TypeClass(
        attach, find_class( attach, "com/sun/star/uno/TypeClass" ) );
    JLocalAutoRef jo_IEnvironment(
        attach, find_class( attach, "com/sun/star/uno/IEnvironment" ) );
    JLocalAutoRef jo_JNI_proxy(
        attach, find_class( attach, "com/sun/star/bridges/jni_uno/JNI_proxy" ) );

    // method Object.getClass()
    m_method_Object_getClass = attach->GetMethodID(
        (jclass)jo_Object.get(), "getClass", "()Ljava/lang/Class;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Object_getClass );
    // method Object.equals()
    m_method_Object_equals = attach->GetMethodID(
        (jclass)jo_Object.get(), "equals", "(Ljava/lang/Object;)Z" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Object_equals );
    // method Object.toString()
    m_method_Object_toString = attach->GetMethodID(
        (jclass)jo_Object.get(), "toString", "()Ljava/lang/String;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Object_toString );

    // method Class.getName()
    m_method_Class_getName = attach->GetMethodID(
        (jclass)jo_Class.get(), "getName", "()Ljava/lang/String;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Class_getName );

    // method Throwable.getMessage()
    m_method_Throwable_getMessage = attach->GetMethodID(
        (jclass)jo_Throwable.get(), "getMessage", "()Ljava/lang/String;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Throwable_getMessage );

    // method Character.charValue()
    m_method_Character_charValue = attach->GetMethodID(
        (jclass)jo_Character.get(), "charValue", "()C" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Character_charValue );
    // method Boolean.booleanValue()
    m_method_Boolean_booleanValue = attach->GetMethodID(
        (jclass)jo_Boolean.get(), "booleanValue", "()Z" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Boolean_booleanValue );
    // method Byte.byteValue()
    m_method_Byte_byteValue = attach->GetMethodID(
        (jclass)jo_Byte.get(), "byteValue", "()B" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Byte_byteValue );
    // method Short.shortValue()
    m_method_Short_shortValue = attach->GetMethodID(
        (jclass)jo_Short.get(), "shortValue", "()S" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Short_shortValue );
    // method Integer.intValue()
    m_method_Integer_intValue = attach->GetMethodID(
        (jclass)jo_Integer.get(), "intValue", "()I" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Integer_intValue );
    // method Long.longValue()
    m_method_Long_longValue = attach->GetMethodID(
        (jclass)jo_Long.get(), "longValue", "()J" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Long_longValue );
    // method Float.floatValue()
    m_method_Float_floatValue = attach->GetMethodID(
        (jclass)jo_Float.get(), "floatValue", "()F" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Float_floatValue );
    // method Double.doubleValue()
    m_method_Double_doubleValue = attach->GetMethodID(
        (jclass)jo_Double.get(), "doubleValue", "()D" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Double_doubleValue );

    // ctor Character( char )
    m_ctor_Character_with_char = attach->GetMethodID(
        (jclass)jo_Character.get(), "<init>", "(C)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Character_with_char );
    // ctor Boolean( boolean )
    m_ctor_Boolean_with_boolean = attach->GetMethodID(
        (jclass)jo_Boolean.get(), "<init>", "(Z)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Boolean_with_boolean );
    // ctor Byte( byte )
    m_ctor_Byte_with_byte = attach->GetMethodID(
        (jclass)jo_Byte.get(), "<init>", "(B)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Byte_with_byte );
    // ctor Short( short )
    m_ctor_Short_with_short = attach->GetMethodID(
        (jclass)jo_Short.get(), "<init>", "(S)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Short_with_short );
    // ctor Integer( int )
    m_ctor_Integer_with_int = attach->GetMethodID(
        (jclass)jo_Integer.get(), "<init>", "(I)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Integer_with_int );
    // ctor Long( long )
    m_ctor_Long_with_long = attach->GetMethodID(
        (jclass)jo_Long.get(), "<init>", "(J)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Long_with_long );
    // ctor Float( float )
    m_ctor_Float_with_float = attach->GetMethodID(
        (jclass)jo_Float.get(), "<init>", "(F)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Float_with_float );
    // ctor Double( double )
    m_ctor_Double_with_double = attach->GetMethodID(
        (jclass)jo_Double.get(), "<init>", "(D)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Double_with_double );

    // static method UnoRuntime.generateOid()
    m_method_UnoRuntime_generateOid = attach->GetStaticMethodID(
        (jclass)jo_UnoRuntime.get(), "generateOid", "(Ljava/lang/Object;)Ljava/lang/String;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_UnoRuntime_generateOid );
    // static method UnoRuntime.queryInterface()
    m_method_UnoRuntime_queryInterface = attach->GetStaticMethodID(
        (jclass)jo_UnoRuntime.get(),
        "queryInterface", "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)Ljava/lang/Object;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_UnoRuntime_queryInterface );

    // field Enum.m_value
    m_field_Enum_m_value = attach->GetFieldID(
        (jclass)jo_Enum.get(), "m_value", "I" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Enum_m_value );

    // static method TypeClass.fromInt()
    m_method_TypeClass_fromInt = attach->GetStaticMethodID(
        (jclass)jo_TypeClass.get(), "fromInt", "(I)Lcom/sun/star/uno/TypeClass;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_TypeClass_fromInt );

    // ctor Type( Class )
    m_ctor_Type_with_Class = attach->GetMethodID(
        (jclass)jo_Type.get(), "<init>", "(Ljava/lang/Class;)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Type_with_Class );
    // ctor Type( String, TypeClass )
    m_ctor_Type_with_Name_TypeClass = attach->GetMethodID(
        (jclass)jo_Type.get(), "<init>", "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Type_with_Name_TypeClass );
    // field Type._typeName
    m_field_Type__typeName = attach->GetFieldID(
        (jclass)jo_Type.get(), "_typeName", "Ljava/lang/String;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Type__typeName );

    // ctor Any( Type, Object )
    m_ctor_Any_with_Type_Object = attach->GetMethodID(
        (jclass)jo_Any.get(), "<init>", "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Any_with_Type_Object );

    // field Any._type
    m_field_Any__type = attach->GetFieldID(
        (jclass)jo_Any.get(), "_type", "Lcom/sun/star/uno/Type;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Any__type );
    // field Any._object
    m_field_Any__object = attach->GetFieldID(
        (jclass)jo_Any.get(), "_object", "Ljava/lang/Object;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Any__object );

    // method IEnvironment.getRegisteredInterface()
    m_method_IEnvironment_getRegisteredInterface = attach->GetMethodID(
        (jclass)jo_IEnvironment.get(), "getRegisteredInterface",
        "(Ljava/lang/String;Lcom/sun/star/uno/Type;)Ljava/lang/Object;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_IEnvironment_getRegisteredInterface );
    // method IEnvironment.registerInterface()
    m_method_IEnvironment_registerInterface = attach->GetMethodID(
        (jclass)jo_IEnvironment.get(), "registerInterface",
        "(Ljava/lang/Object;[Ljava/lang/String;Lcom/sun/star/uno/Type;)Ljava/lang/Object;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_IEnvironment_registerInterface );
    // method IEnvironment.revokeInterface()
    m_method_IEnvironment_revokeInterface = attach->GetMethodID(
        (jclass)jo_IEnvironment.get(), "revokeInterface",
        "(Ljava/lang/String;Lcom/sun/star/uno/Type;)V" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_IEnvironment_revokeInterface );

    // static method JNI_proxy.create()
    m_method_JNI_proxy_create = attach->GetStaticMethodID(
        (jclass)jo_JNI_proxy.get(), "create",
        "(JJJLcom/sun/star/uno/Type;Ljava/lang/String;)Ljava/lang/Object;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_JNI_proxy_create );
    // field JNI_proxy.m_receiver_handle
    m_field_JNI_proxy_m_receiver_handle = attach->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_receiver_handle", "J" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_receiver_handle );
    // field JNI_proxy.m_td_handle
    m_field_JNI_proxy_m_td_handle = attach->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_td_handle", "J" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_td_handle );
    // field JNI_proxy.m_type
    m_field_JNI_proxy_m_type = attach->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_type", "Lcom/sun/star/uno/Type;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_type );
    // field JNI_proxy.m_oid
    m_field_JNI_proxy_m_oid = attach->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_oid", "Ljava/lang/String;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_oid );

    // get java env
    JLocalAutoRef jo_java( attach, ustring_to_jstring( attach, java_env->pTypeName ) );
    jvalue args[ 2 ];
    args[ 0 ].l = jo_java.get();
    args[ 1 ].l = 0;
    jmethodID method_getEnvironment = attach->GetStaticMethodID(
        (jclass)jo_UnoRuntime.get(), "getEnvironment",
        "(Ljava/lang/String;Ljava/lang/Object;)Lcom/sun/star/uno/IEnvironment;" );
    attach.ensure_no_exception();
    OSL_ASSERT( 0 != method_getEnvironment );
    JLocalAutoRef jo_java_env(
        attach, attach->CallStaticObjectMethodA(
            (jclass)jo_UnoRuntime.get(), method_getEnvironment, args ) );

    // make global refs
    m_class_Any = (jclass)attach->NewGlobalRef( jo_Any.get() );
    m_class_Type = (jclass)attach->NewGlobalRef( jo_Type.get() );
    m_class_TypeClass = (jclass)attach->NewGlobalRef( jo_TypeClass.get() );
    m_class_UnoRuntime = (jclass)attach->NewGlobalRef( jo_UnoRuntime.get() );
    m_class_RuntimeException = (jclass)attach->NewGlobalRef( jo_RuntimeException.get() );
    m_class_JNI_proxy = (jclass)attach->NewGlobalRef( jo_JNI_proxy.get() );

    m_class_Character = (jclass)attach->NewGlobalRef( jo_Character.get() );
    m_class_Boolean = (jclass)attach->NewGlobalRef( jo_Boolean.get() );
    m_class_Byte = (jclass)attach->NewGlobalRef( jo_Byte.get() );
    m_class_Short = (jclass)attach->NewGlobalRef( jo_Short.get() );
    m_class_Integer = (jclass)attach->NewGlobalRef( jo_Integer.get() );
    m_class_Long = (jclass)attach->NewGlobalRef( jo_Long.get() );
    m_class_Float = (jclass)attach->NewGlobalRef( jo_Float.get() );
    m_class_Double = (jclass)attach->NewGlobalRef( jo_Double.get() );
    m_class_String = (jclass)attach->NewGlobalRef( jo_String.get() );
    m_class_Object = (jclass)attach->NewGlobalRef( jo_Object.get() );

    m_object_java_env = attach->NewGlobalRef( jo_java_env.get() );
}
//__________________________________________________________________________________________________
JNI_info::~JNI_info() SAL_THROW( () )
{
    JNI_attach attach( m_java_env );

    t_str2type::const_iterator iPos( m_type_map.begin() );
    t_str2type::const_iterator const iEnd( m_type_map.begin() );
    for ( ; iPos != iEnd; ++iPos )
    {
        JNI_type_info::_delete( attach, iPos->second.m_info );
    }

    // free global refs
    attach->DeleteGlobalRef( m_object_java_env );

    attach->DeleteGlobalRef( m_class_Object );
    attach->DeleteGlobalRef( m_class_String );
    attach->DeleteGlobalRef( m_class_Double );
    attach->DeleteGlobalRef( m_class_Float );
    attach->DeleteGlobalRef( m_class_Long );
    attach->DeleteGlobalRef( m_class_Integer );
    attach->DeleteGlobalRef( m_class_Short );
    attach->DeleteGlobalRef( m_class_Byte );
    attach->DeleteGlobalRef( m_class_Boolean );
    attach->DeleteGlobalRef( m_class_Character );

    attach->DeleteGlobalRef( m_class_JNI_proxy );
    attach->DeleteGlobalRef( m_class_RuntimeException );
    attach->DeleteGlobalRef( m_class_UnoRuntime );
    attach->DeleteGlobalRef( m_class_TypeClass );
    attach->DeleteGlobalRef( m_class_Type );
    attach->DeleteGlobalRef( m_class_Any );
}

}
