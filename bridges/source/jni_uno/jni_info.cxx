/*************************************************************************
 *
 *  $RCSfile: jni_info.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2002-12-06 10:26:04 $
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

#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"


using namespace ::std;
using namespace ::osl;
using namespace ::rtl;

namespace jni_uno
{

//__________________________________________________________________________________________________
JNI_type_info::JNI_type_info(
    JNI_context const & jni, typelib_InterfaceTypeDescription * td )
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
    JLocalAutoRef jo_class( jni, find_class( jni, java_name.getStr() ) );

    JNI_info const * jni_info = jni.get_info();

    // retrieve info for base type
    typelib_TypeDescription * base_td = (typelib_TypeDescription *)td->pBaseTypeDescription;
    m_base = (0 == base_td ? 0 : jni_info->get_type_info( jni, base_td ));

    // if ! XInterface
    if (! typelib_typedescriptionreference_equals(
            ((typelib_TypeDescription *)td)->pWeakRef, jni_info->m_XInterface_td.get()->pWeakRef ))
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

                    m_methods[ nMethodIndex ] = jni->GetMethodID(
                        (jclass)jo_class.get(), method_name.getStr(), method_signature.getStr() );
                    jni.ensure_no_exception();
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
                    m_methods[ nMethodIndex ] = jni->GetMethodID(
                        (jclass)jo_class.get(), method_name.getStr(), method_signature.getStr() );
                    jni.ensure_no_exception();
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
                        m_methods[ nMethodIndex ] = jni->GetMethodID(
                            (jclass)jo_class.get(), method_name.getStr(),
                            method_signature.getStr() );
                        jni.ensure_no_exception();
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

    JLocalAutoRef jo_type( jni, create_type( jni, (jclass)jo_class.get() ) );

    m_class = (jclass)jni->NewGlobalRef( jo_class.get() );
    m_jo_type = jni->NewGlobalRef( jo_type.get() );
}
//__________________________________________________________________________________________________
JNI_type_info::JNI_type_info(
    JNI_context const & jni, typelib_CompoundTypeDescription * td )
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
    JLocalAutoRef jo_class( jni, find_class( jni, java_name.getStr() ) );

    JNI_info const * jni_info = jni.get_info();

    // retrieve ctor
    m_ctor = jni->GetMethodID(
        (jclass)jo_class.get(), "<init>",
        (typelib_TypeClass_EXCEPTION == m_td.get()->eTypeClass ? "(Ljava/lang/String;)V" : "()V") );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor );

    // retrieve info for base type
    typelib_TypeDescription * base_td = (typelib_TypeDescription *)td->pBaseTypeDescription;
    m_base = (0 == base_td ? 0 : jni_info->get_type_info( jni, base_td ));

    try
    {
        if (typelib_typedescriptionreference_equals(
                ((typelib_TypeDescription *)td)->pWeakRef,
                jni_info->m_Exception_type.getTypeLibType() ) ||
            typelib_typedescriptionreference_equals(
                ((typelib_TypeDescription *)td)->pWeakRef,
                jni_info->m_RuntimeException_type.getTypeLibType() ))
        {
            m_fields = new jfieldID[ 2 ];
            m_fields[ 0 ] = 0; // special Throwable.getMessage()
            // field Context
            m_fields[ 1 ] = jni->GetFieldID(
                (jclass)jo_class.get(), "Context", "Ljava/lang/Object;" );
            jni.ensure_no_exception();
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

                m_fields[ nPos ] = jni->GetFieldID(
                    (jclass)jo_class.get(), member_name.getStr(), sig.getStr() );
                jni.ensure_no_exception();
                OSL_ASSERT( 0 != m_fields[ nPos ] );
            }
        }
    }
    catch (...)
    {
        delete [] m_fields;
        throw;
    }

    m_class = (jclass)jni->NewGlobalRef( jo_class.get() );
    m_jo_type = 0;
}
//__________________________________________________________________________________________________
void JNI_type_info::_delete( JNI_context const & jni, JNI_type_info * that ) SAL_THROW( () )
{
    delete [] that->m_fields;
    delete [] that->m_methods;
    jni->DeleteGlobalRef( that->m_class );
    delete that;
}

//##################################################################################################

//__________________________________________________________________________________________________
JNI_type_info const * JNI_info::get_type_info(
    JNI_context const & jni, typelib_TypeDescription * td ) const
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
            new_info = new JNI_type_info( jni, (typelib_CompoundTypeDescription *)td );
            break;
        case typelib_TypeClass_INTERFACE:
            new_info = new JNI_type_info( jni, (typelib_InterfaceTypeDescription *)td );
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
            JNI_type_info::_delete( jni, new_info );
        }
    }
    else
    {
        info = iFind->second.m_info;
    }

    return info;
}
//__________________________________________________________________________________________________
JNI_type_info const * JNI_info::get_type_info(
    JNI_context const & jni, OUString const & uno_name ) const
{
    JNI_type_info * info;

    ClearableMutexGuard guard( m_mutex );
    t_str2type::const_iterator iFind( m_type_map.find( uno_name ) );
    if (m_type_map.end() == iFind)
    {
        guard.clear();

        ::com::sun::star::uno::TypeDescription td( uno_name );
        if (! td.is())
            throw BridgeRuntimeError( OUSTR("UNO type not found: ") + uno_name );

        JNI_type_info * new_info;
        switch (td.get()->eTypeClass)
        {
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            new_info = new JNI_type_info( jni, (typelib_CompoundTypeDescription *)td.get() );
            break;
        case typelib_TypeClass_INTERFACE:
            new_info = new JNI_type_info( jni, (typelib_InterfaceTypeDescription *)td.get() );
            break;
        default:
            throw BridgeRuntimeError( OUSTR("unsupported: type info for ") + uno_name );
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
            JNI_type_info::_delete( jni, new_info );
        }
    }
    else
    {
        info = iFind->second.m_info;
    }

    return info;
}
//__________________________________________________________________________________________________
JNI_info::JNI_info( Bridge const * bridge )
    : m_bridge( bridge ), // unacquired pointer to owner
      m_XInterface_td(
          ::getCppuType(
              (::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > const *)0 ) ),
      m_XInterface_queryInterface_td(
          ((typelib_InterfaceTypeDescription *)m_XInterface_td.get())->ppMembers[ 0 ] ),
      m_Exception_type(
          ::getCppuType(
              (::com::sun::star::uno::Exception const *)0 ) ),
      m_RuntimeException_type(
          ::getCppuType(
              (::com::sun::star::uno::RuntimeException const *)0 ) ),
      m_class_JNI_proxy( 0 )
{
    JNI_guarded_context jni(
        this, reinterpret_cast< ::jvmaccess::VirtualMachine * >( m_bridge->m_java_env->pContext ) );

    // class lookup
    JLocalAutoRef jo_Object(
        jni, find_class( jni, "java/lang/Object" ) );
    JLocalAutoRef jo_Class(
        jni, find_class( jni, "java/lang/Class" ) );
    JLocalAutoRef jo_Throwable(
        jni, find_class( jni, "java/lang/Throwable" ) );
    JLocalAutoRef jo_Character(
        jni, find_class( jni, "java/lang/Character" ) );
    JLocalAutoRef jo_Boolean(
        jni, find_class( jni, "java/lang/Boolean" ) );
    JLocalAutoRef jo_Byte(
        jni, find_class( jni, "java/lang/Byte" ) );
    JLocalAutoRef jo_Short(
        jni, find_class( jni, "java/lang/Short" ) );
    JLocalAutoRef jo_Integer(
        jni, find_class( jni, "java/lang/Integer" ) );
    JLocalAutoRef jo_Long(
        jni, find_class( jni, "java/lang/Long" ) );
    JLocalAutoRef jo_Float(
        jni, find_class( jni, "java/lang/Float" ) );
    JLocalAutoRef jo_Double(
        jni, find_class( jni, "java/lang/Double" ) );
    JLocalAutoRef jo_String(
        jni, find_class( jni, "java/lang/String" ) );
    JLocalAutoRef jo_RuntimeException(
        jni, find_class( jni, "com/sun/star/uno/RuntimeException" ) );
    JLocalAutoRef jo_UnoRuntime(
        jni, find_class( jni, "com/sun/star/uno/UnoRuntime" ) );
    JLocalAutoRef jo_Any(
        jni, find_class( jni, "com/sun/star/uno/Any" ) );
    JLocalAutoRef jo_Enum(
        jni, find_class( jni, "com/sun/star/uno/Enum" ) );
    JLocalAutoRef jo_Type(
        jni, find_class( jni, "com/sun/star/uno/Type" ) );
    JLocalAutoRef jo_TypeClass(
        jni, find_class( jni, "com/sun/star/uno/TypeClass" ) );
    JLocalAutoRef jo_IEnvironment(
        jni, find_class( jni, "com/sun/star/uno/IEnvironment" ) );
    JLocalAutoRef jo_TypedProxy(
        jni, find_class( jni, "com/sun/star/lib/uno/TypedProxy" ) );
    JLocalAutoRef jo_JNI_proxy(
        jni, find_class( jni, "com/sun/star/bridges/jni_uno/JNI_proxy" ) );

    // method Object.getClass()
    m_method_Object_getClass = jni->GetMethodID(
        (jclass)jo_Object.get(), "getClass", "()Ljava/lang/Class;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Object_getClass );
    // method Object.equals()
    m_method_Object_equals = jni->GetMethodID(
        (jclass)jo_Object.get(), "equals", "(Ljava/lang/Object;)Z" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Object_equals );
    // method Object.toString()
    m_method_Object_toString = jni->GetMethodID(
        (jclass)jo_Object.get(), "toString", "()Ljava/lang/String;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Object_toString );

    // method Class.getName()
    m_method_Class_getName = jni->GetMethodID(
        (jclass)jo_Class.get(), "getName", "()Ljava/lang/String;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Class_getName );

    // method Throwable.getMessage()
    m_method_Throwable_getMessage = jni->GetMethodID(
        (jclass)jo_Throwable.get(), "getMessage", "()Ljava/lang/String;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Throwable_getMessage );

    // method Character.charValue()
    m_method_Character_charValue = jni->GetMethodID(
        (jclass)jo_Character.get(), "charValue", "()C" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Character_charValue );
    // method Boolean.booleanValue()
    m_method_Boolean_booleanValue = jni->GetMethodID(
        (jclass)jo_Boolean.get(), "booleanValue", "()Z" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Boolean_booleanValue );
    // method Byte.byteValue()
    m_method_Byte_byteValue = jni->GetMethodID(
        (jclass)jo_Byte.get(), "byteValue", "()B" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Byte_byteValue );
    // method Short.shortValue()
    m_method_Short_shortValue = jni->GetMethodID(
        (jclass)jo_Short.get(), "shortValue", "()S" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Short_shortValue );
    // method Integer.intValue()
    m_method_Integer_intValue = jni->GetMethodID(
        (jclass)jo_Integer.get(), "intValue", "()I" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Integer_intValue );
    // method Long.longValue()
    m_method_Long_longValue = jni->GetMethodID(
        (jclass)jo_Long.get(), "longValue", "()J" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Long_longValue );
    // method Float.floatValue()
    m_method_Float_floatValue = jni->GetMethodID(
        (jclass)jo_Float.get(), "floatValue", "()F" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Float_floatValue );
    // method Double.doubleValue()
    m_method_Double_doubleValue = jni->GetMethodID(
        (jclass)jo_Double.get(), "doubleValue", "()D" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_Double_doubleValue );

    // ctor Character( char )
    m_ctor_Character_with_char = jni->GetMethodID(
        (jclass)jo_Character.get(), "<init>", "(C)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Character_with_char );
    // ctor Boolean( boolean )
    m_ctor_Boolean_with_boolean = jni->GetMethodID(
        (jclass)jo_Boolean.get(), "<init>", "(Z)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Boolean_with_boolean );
    // ctor Byte( byte )
    m_ctor_Byte_with_byte = jni->GetMethodID(
        (jclass)jo_Byte.get(), "<init>", "(B)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Byte_with_byte );
    // ctor Short( short )
    m_ctor_Short_with_short = jni->GetMethodID(
        (jclass)jo_Short.get(), "<init>", "(S)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Short_with_short );
    // ctor Integer( int )
    m_ctor_Integer_with_int = jni->GetMethodID(
        (jclass)jo_Integer.get(), "<init>", "(I)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Integer_with_int );
    // ctor Long( long )
    m_ctor_Long_with_long = jni->GetMethodID(
        (jclass)jo_Long.get(), "<init>", "(J)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Long_with_long );
    // ctor Float( float )
    m_ctor_Float_with_float = jni->GetMethodID(
        (jclass)jo_Float.get(), "<init>", "(F)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Float_with_float );
    // ctor Double( double )
    m_ctor_Double_with_double = jni->GetMethodID(
        (jclass)jo_Double.get(), "<init>", "(D)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Double_with_double );

    // static method UnoRuntime.generateOid()
    m_method_UnoRuntime_generateOid = jni->GetStaticMethodID(
        (jclass)jo_UnoRuntime.get(), "generateOid", "(Ljava/lang/Object;)Ljava/lang/String;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_UnoRuntime_generateOid );
    // static method UnoRuntime.queryInterface()
    m_method_UnoRuntime_queryInterface = jni->GetStaticMethodID(
        (jclass)jo_UnoRuntime.get(),
        "queryInterface", "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_UnoRuntime_queryInterface );

    // field Enum.m_value
    m_field_Enum_m_value = jni->GetFieldID(
        (jclass)jo_Enum.get(), "m_value", "I" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Enum_m_value );

    // static method TypeClass.fromInt()
    m_method_TypeClass_fromInt = jni->GetStaticMethodID(
        (jclass)jo_TypeClass.get(), "fromInt", "(I)Lcom/sun/star/uno/TypeClass;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_TypeClass_fromInt );

    // ctor Type( Class )
    m_ctor_Type_with_Class = jni->GetMethodID(
        (jclass)jo_Type.get(), "<init>", "(Ljava/lang/Class;)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Type_with_Class );
    // ctor Type( String, TypeClass )
    m_ctor_Type_with_Name_TypeClass = jni->GetMethodID(
        (jclass)jo_Type.get(), "<init>", "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Type_with_Name_TypeClass );
    // field Type._typeName
    m_field_Type__typeName = jni->GetFieldID(
        (jclass)jo_Type.get(), "_typeName", "Ljava/lang/String;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Type__typeName );

    // ctor Any( Type, Object )
    m_ctor_Any_with_Type_Object = jni->GetMethodID(
        (jclass)jo_Any.get(), "<init>", "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_ctor_Any_with_Type_Object );

    // field Any._type
    m_field_Any__type = jni->GetFieldID(
        (jclass)jo_Any.get(), "_type", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Any__type );
    // field Any._object
    m_field_Any__object = jni->GetFieldID(
        (jclass)jo_Any.get(), "_object", "Ljava/lang/Object;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_Any__object );

    // method IEnvironment.getRegisteredInterface()
    m_method_IEnvironment_getRegisteredInterface = jni->GetMethodID(
        (jclass)jo_IEnvironment.get(), "getRegisteredInterface",
        "(Ljava/lang/String;Lcom/sun/star/uno/Type;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_IEnvironment_getRegisteredInterface );
    // method IEnvironment.registerInterface()
    m_method_IEnvironment_registerInterface = jni->GetMethodID(
        (jclass)jo_IEnvironment.get(), "registerInterface",
        "(Ljava/lang/Object;[Ljava/lang/String;Lcom/sun/star/uno/Type;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_IEnvironment_registerInterface );
    // method IEnvironment.revokeInterface()
    m_method_IEnvironment_revokeInterface = jni->GetMethodID(
        (jclass)jo_IEnvironment.get(), "revokeInterface",
        "(Ljava/lang/String;Lcom/sun/star/uno/Type;)V" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_IEnvironment_revokeInterface );

    // method TypedProxy.getType()
    m_method_TypedProxy_getType = jni->GetMethodID(
        (jclass)jo_TypedProxy.get(), "getType",
        "()Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_TypedProxy_getType );

    // static method JNI_proxy.create()
    m_method_JNI_proxy_create = jni->GetStaticMethodID(
        (jclass)jo_JNI_proxy.get(), "create",
        "(JLcom/sun/star/uno/IEnvironment;JJLcom/sun/star/uno/Type;Ljava/lang/String;)Ljava/lang/Object;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_method_JNI_proxy_create );
    // field JNI_proxy.m_receiver_handle
    m_field_JNI_proxy_m_receiver_handle = jni->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_receiver_handle", "J" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_receiver_handle );
    // field JNI_proxy.m_td_handle
    m_field_JNI_proxy_m_td_handle = jni->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_td_handle", "J" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_td_handle );
    // field JNI_proxy.m_type
    m_field_JNI_proxy_m_type = jni->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_type", "Lcom/sun/star/uno/Type;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_type );
    // field JNI_proxy.m_oid
    m_field_JNI_proxy_m_oid = jni->GetFieldID(
        (jclass)jo_JNI_proxy.get(), "m_oid", "Ljava/lang/String;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != m_field_JNI_proxy_m_oid );

    // get java env
    JLocalAutoRef jo_java( jni, ustring_to_jstring( jni, m_bridge->m_java_env->pTypeName ) );
    jvalue args[ 2 ];
    args[ 0 ].l = jo_java.get();
    args[ 1 ].l = 0;
    jmethodID method_getEnvironment = jni->GetStaticMethodID(
        (jclass)jo_UnoRuntime.get(), "getEnvironment",
        "(Ljava/lang/String;Ljava/lang/Object;)Lcom/sun/star/uno/IEnvironment;" );
    jni.ensure_no_exception();
    OSL_ASSERT( 0 != method_getEnvironment );
    JLocalAutoRef jo_java_env(
        jni, jni->CallStaticObjectMethodA(
            (jclass)jo_UnoRuntime.get(), method_getEnvironment, args ) );

    // make global refs
    m_class_UnoRuntime = (jclass)jni->NewGlobalRef( jo_UnoRuntime.get() );
    m_class_RuntimeException = (jclass)jni->NewGlobalRef( jo_RuntimeException.get() );
    m_class_Any = (jclass)jni->NewGlobalRef( jo_Any.get() );
    m_class_Type = (jclass)jni->NewGlobalRef( jo_Type.get() );
    m_class_TypeClass = (jclass)jni->NewGlobalRef( jo_TypeClass.get() );
    m_class_TypedProxy = (jclass)jni->NewGlobalRef( jo_TypedProxy.get() );
    m_class_JNI_proxy = (jclass)jni->NewGlobalRef( jo_JNI_proxy.get() );

    m_class_Character = (jclass)jni->NewGlobalRef( jo_Character.get() );
    m_class_Boolean = (jclass)jni->NewGlobalRef( jo_Boolean.get() );
    m_class_Byte = (jclass)jni->NewGlobalRef( jo_Byte.get() );
    m_class_Short = (jclass)jni->NewGlobalRef( jo_Short.get() );
    m_class_Integer = (jclass)jni->NewGlobalRef( jo_Integer.get() );
    m_class_Long = (jclass)jni->NewGlobalRef( jo_Long.get() );
    m_class_Float = (jclass)jni->NewGlobalRef( jo_Float.get() );
    m_class_Double = (jclass)jni->NewGlobalRef( jo_Double.get() );
    m_class_String = (jclass)jni->NewGlobalRef( jo_String.get() );
    m_class_Object = (jclass)jni->NewGlobalRef( jo_Object.get() );

    m_object_java_env = jni->NewGlobalRef( jo_java_env.get() );
}
//__________________________________________________________________________________________________
JNI_info::~JNI_info() SAL_THROW( () )
{
    try
    {
        JNI_guarded_context jni(
            this,
            reinterpret_cast< ::jvmaccess::VirtualMachine * >( m_bridge->m_java_env->pContext ) );

        t_str2type::const_iterator iPos( m_type_map.begin() );
        t_str2type::const_iterator const iEnd( m_type_map.begin() );
        for ( ; iPos != iEnd; ++iPos )
        {
            JNI_type_info::_delete( jni, iPos->second.m_info );
        }

        // free global refs
        jni->DeleteGlobalRef( m_object_java_env );

        jni->DeleteGlobalRef( m_class_Object );
        jni->DeleteGlobalRef( m_class_String );
        jni->DeleteGlobalRef( m_class_Double );
        jni->DeleteGlobalRef( m_class_Float );
        jni->DeleteGlobalRef( m_class_Long );
        jni->DeleteGlobalRef( m_class_Integer );
        jni->DeleteGlobalRef( m_class_Short );
        jni->DeleteGlobalRef( m_class_Byte );
        jni->DeleteGlobalRef( m_class_Boolean );
        jni->DeleteGlobalRef( m_class_Character );

        jni->DeleteGlobalRef( m_class_JNI_proxy );
        jni->DeleteGlobalRef( m_class_RuntimeException );
        jni->DeleteGlobalRef( m_class_UnoRuntime );
        jni->DeleteGlobalRef( m_class_TypeClass );
        jni->DeleteGlobalRef( m_class_Type );
        jni->DeleteGlobalRef( m_class_Any );
    }
    catch (BridgeRuntimeError & err)
    {
#ifdef _DEBUG
        OString cstr_msg(
            OUStringToOString(
                OUSTR("[jni_uno bridge error] ") + err.m_message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, cstr_msg.getStr() );
#endif
    }
    catch (::jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        OSL_ENSURE( 0, "[jni_uno bridge error] attaching current thread to java failed!" );
    }
}

}
