/*************************************************************************
 *
 *  $RCSfile: climaker_emit.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2003-07-16 10:42:22 $
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

#include "climaker_share.h"

#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription.hpp"


using namespace ::System::Reflection;

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace climaker
{

//------------------------------------------------------------------------------
static inline ::System::String * to_cts_name(
    OUString const & uno_name )
{
    OUStringBuffer buf( 7 + uno_name.getLength() );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("unoidl.") );
    buf.append( uno_name );
    return ustring_to_String( buf.makeStringAndClear() );
}

//------------------------------------------------------------------------------
static inline ::System::Object * to_cli_constant( Any const & value )
{
    switch (value.getValueTypeClass())
    {
    case TypeClass_CHAR:
        return __box
            ((::System::Char) *reinterpret_cast< sal_Unicode const * >(
                value.getValue() ));
    case TypeClass_BOOLEAN:
        return __box
            ((::System::Boolean)
             sal_False != *reinterpret_cast< sal_Bool const * >(
                 value.getValue() ));
    case TypeClass_BYTE:
        return __box
            ((::System::Byte) *reinterpret_cast< sal_Int8 const * >(
                value.getValue() ));
    case TypeClass_SHORT:
        return __box
            ((::System::Int16) *reinterpret_cast< sal_Int16 const * >(
                value.getValue() ));
    case TypeClass_UNSIGNED_SHORT:
        return __box
            ((::System::UInt16) *reinterpret_cast< sal_uInt16 const * >(
                value.getValue() ));
    case TypeClass_LONG:
        return __box
            ((::System::Int32) *reinterpret_cast< sal_Int32 const * >(
                value.getValue() ));
    case TypeClass_UNSIGNED_LONG:
        return __box
            ((::System::UInt32) *reinterpret_cast< sal_uInt32 const * >(
                value.getValue() ));
    case TypeClass_HYPER:
        return __box
            ((::System::Int64) *reinterpret_cast< sal_Int64 const * >(
                value.getValue() ));
    case TypeClass_UNSIGNED_HYPER:
        return __box
            ((::System::UInt64) *reinterpret_cast< sal_uInt64 const * >(
                value.getValue() ));
    case TypeClass_FLOAT:
        return __box
            ((::System::Single) *reinterpret_cast< float const * >(
                value.getValue() ));
    case TypeClass_DOUBLE:
        return __box
            ((::System::Double) *reinterpret_cast< double const * >(
                value.getValue() ));
    default:
        throw RuntimeException(
            OUSTR("unexpected constant type ") +
            value.getValueType().getTypeName(),
            Reference< XInterface >() );
    }
}

//------------------------------------------------------------------------------
static inline void emit_ldarg( Emit::ILGenerator * code, ::System::Int32 index )
{
    switch (index)
    {
    case 0:
        code->Emit( Emit::OpCodes::Ldarg_0 );
        break;
    case 1:
        code->Emit( Emit::OpCodes::Ldarg_1 );
        break;
    case 2:
        code->Emit( Emit::OpCodes::Ldarg_2 );
        break;
    case 3:
        code->Emit( Emit::OpCodes::Ldarg_3 );
        break;
    default:
        if (index < 0x100)
            code->Emit( Emit::OpCodes::Ldarg_S, (::System::Byte) index );
        else if (index < 0x8000)
            code->Emit( Emit::OpCodes::Ldarg_S, (::System::Int16) index );
        else
            code->Emit( Emit::OpCodes::Ldarg, index );
        break;
    }
}

//______________________________________________________________________________
Assembly * TypeEmitter::type_resolve(
    ::System::Object * sender, ::System::ResolveEventArgs * args )
{
    ::System::String * cts_name = args->get_Name();
    ::System::Type * ret_type = m_module_builder->GetType(
        cts_name, false /* no exc */ );
    if (0 == ret_type)
    {
        iface_entry * entry = dynamic_cast< iface_entry * >(
            m_incomplete_ifaces->get_Item( cts_name ) );
        if (0 != entry)
            ret_type = entry->m_type_builder;
    }
    if (0 == ret_type)
    {
        sal_Int32 len = m_extra_assemblies->get_Length();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            ret_type = m_extra_assemblies[ pos ]->GetType(
                cts_name, false /* no exc */ );
            if (0 != ret_type)
            {
                if (g_verbose)
                {
                    ::System::Console::WriteLine(
                        "> resolving type {0} from {1}.",
                        cts_name, ret_type->get_Assembly()->get_FullName() );
                }
                break;
            }
        }
    }
    if (0 != ret_type)
        return ret_type->get_Assembly();
    return 0;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    ::System::String * cts_name, bool throw_exc )
{
    ::System::Type * ret_type = m_module_builder->GetType( cts_name, false );
    if (0 == ret_type)
    {
        iface_entry * entry = dynamic_cast< iface_entry * >(
            m_incomplete_ifaces->get_Item( cts_name ) );
        if (0 != entry)
            ret_type = entry->m_type_builder;
    }
    if (0 == ret_type)
    {
        // may call on type_resolve()
        return ::System::Type::GetType( cts_name, throw_exc );
    }
    else
    {
        return ret_type;
    }
}

//______________________________________________________________________________
void TypeEmitter::emit_Any_boxed_ctor(
    Emit::TypeBuilder * type_builder,
    Emit::FieldBuilder * field_Type, Emit::FieldBuilder * field_Value,
    ::System::Type * integral_type )
{
    ::System::Type * param_types __gc [] = new ::System::Type * __gc [ 1 ];
    param_types[ 0 ] = integral_type;
    Emit::ConstructorBuilder * ctor_builder =
        type_builder->DefineConstructor(
            c_ctor_method_attr, CallingConventions::Standard, param_types );
    ctor_builder->DefineParameter( 1, ParameterAttributes::In, S"Value" );
    Emit::ILGenerator * code = ctor_builder->GetILGenerator();
    code->Emit( Emit::OpCodes::Ldarg_0 );
    code->Emit( Emit::OpCodes::Ldtoken, integral_type );
    code->Emit( Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
    code->Emit( Emit::OpCodes::Stfld, field_Type );
    code->Emit( Emit::OpCodes::Ldarg_0 );
    code->Emit( Emit::OpCodes::Ldarg_1 );
    code->Emit( Emit::OpCodes::Box, integral_type );
    code->Emit( Emit::OpCodes::Stfld, field_Value );
    code->Emit( Emit::OpCodes::Ret );
}
//______________________________________________________________________________
::System::Type * TypeEmitter::get_type_Any()
{
    if (0 == m_type_Any)
    {
        m_type_Any = get_type( S"uno.Any", false /* no exc */ );
        if (0 == m_type_Any)
        {
            // define hardcoded type uno.Any
            Emit::TypeBuilder * type_builder =
                  m_module_builder->DefineType(
                      S"uno.Any",
                      (TypeAttributes) (TypeAttributes::Public |
                                        TypeAttributes::Sealed |
                                        TypeAttributes::SequentialLayout |
                                        TypeAttributes::BeforeFieldInit |
                                        TypeAttributes::AnsiClass),
                      __typeof (::System::ValueType) );
            Emit::FieldBuilder * field_Type = type_builder->DefineField(
                S"Type", __typeof (::System::Type), FieldAttributes::Public );
            Emit::FieldBuilder * field_Value = type_builder->DefineField(
                S"Value",
                __typeof (::System::Object), FieldAttributes::Public );
            Emit::FieldBuilder * field_VOID = type_builder->DefineField(
                S"VOID", type_builder,
                (FieldAttributes) (FieldAttributes::Public |
                                   FieldAttributes::Static |
                                   FieldAttributes::InitOnly) );
            // .ctor( System.Type, System.Object )
            ::System::Type * param_types __gc [] =
                  new ::System::Type * __gc [ 2 ];
            param_types[ 0 ] = __typeof (::System::Type);
            param_types[ 1 ] = __typeof (::System::Object);
            Emit::ConstructorBuilder * ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, S"Type" );
            ctor_builder->DefineParameter(
                2, ParameterAttributes::In, S"Value" );
            Emit::ILGenerator * code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_Type );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_2 );
            code->Emit( Emit::OpCodes::Stfld, field_Value );
            code->Emit( Emit::OpCodes::Ret );
            // .cctor
            // xxx todo: potential problem?
            // DefineTypeInitializer() does not apply the HideBySig attribute!
            code = type_builder->DefineTypeInitializer()->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldsflda, field_VOID );
            code->Emit( Emit::OpCodes::Ldtoken, __typeof (::System::Void) );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Ldnull );
            code->Emit( Emit::OpCodes::Call, ctor_builder );
            code->Emit( Emit::OpCodes::Ret );

            // integral .ctors
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Char) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Boolean) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Byte) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Int16) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::UInt16) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Int32) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::UInt32) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Int64) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::UInt64) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Single) );
            emit_Any_boxed_ctor(
                type_builder, field_Type, field_Value,
                __typeof (::System::Double) );

            // .ctor( System.Type )
            param_types = new ::System::Type * __gc [ 1 ];
            param_types[ 0 ] = __typeof (::System::Type);
            ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, S"Value" );
            code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldtoken, __typeof (::System::Type) );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Stfld, field_Type );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_Value );
            code->Emit( Emit::OpCodes::Ret );
            // .ctor( System.String )
            param_types[ 0 ] = __typeof (::System::String);
            ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, S"Value" );
            code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldtoken, __typeof (::System::String) );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Stfld, field_Type );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_Value );
            code->Emit( Emit::OpCodes::Ret );

            // public bool hasValue()
            Emit::MethodBuilder * method_builder =
                type_builder->DefineMethod(
                    S"hasValue",
                    (MethodAttributes) (MethodAttributes::Public |
                                        MethodAttributes::HideBySig
                                        /* | xxx todo: ??? compiler does not know Instance ???
                                           MethodAttributes::Instance*/),
                    __typeof (::System::Boolean),
                    new ::System::Type * __gc [ 0 ] );
            code = method_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_Type );
            code->Emit(
                Emit::OpCodes::Ldtoken, __typeof (::System::Void) );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Ceq );
            code->Emit( Emit::OpCodes::Ldc_I4_0 );
            code->Emit( Emit::OpCodes::Ceq );
            code->Emit( Emit::OpCodes::Ret );

            // public override String ToString()
            method_builder =
                type_builder->DefineMethod(
                    S"ToString",
                    (MethodAttributes) (MethodAttributes::Public |
                                        MethodAttributes::Virtual |
                                        MethodAttributes::HideBySig
                                        /* | xxx todo: ??? compiler does not know Instance ???
                                           MethodAttributes::Instance*/),
                    __typeof (::System::String),
                    new ::System::Type * __gc [ 0 ] );
            code = method_builder->GetILGenerator();
            Emit::LocalBuilder * local_var =
                code->DeclareLocal( __typeof (::System::Object * __gc []) );
            code->Emit( Emit::OpCodes::Ldc_I4_5 );
            code->Emit( Emit::OpCodes::Newarr, __typeof (::System::Object) );
            code->Emit( Emit::OpCodes::Stloc, local_var );
            code->Emit( Emit::OpCodes::Ldloc, local_var );
            code->Emit( Emit::OpCodes::Ldc_I4_0 );
            code->Emit( Emit::OpCodes::Ldstr, "uno.Any { Type=" );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_var );
            code->Emit( Emit::OpCodes::Ldc_I4_1 );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_Type );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_var );
            code->Emit( Emit::OpCodes::Ldc_I4_2 );
            code->Emit( Emit::OpCodes::Ldstr, ", Value=" );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_var );
            code->Emit( Emit::OpCodes::Ldc_I4_3 );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_Value );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_var );
            code->Emit( Emit::OpCodes::Ldc_I4_4 );
            code->Emit( Emit::OpCodes::Ldstr, " }" );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_var );
            param_types = new ::System::Type * __gc [ 1 ];
            param_types[ 0 ] = __typeof (::System::Object * __gc []);
            code->Emit( Emit::OpCodes::Call,
                        __typeof (::System::String)->GetMethod(
                            "Concat", param_types ) );
            code->Emit( Emit::OpCodes::Ret );

            if (g_verbose)
            {
                ::System::Console::WriteLine(
                    "> emitting integral type uno.Any" );
            }
            m_type_Any = type_builder->CreateType();
        }
        m_field_Any_VOID = m_type_Any->GetField( S"VOID" );
    }
    return m_type_Any;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type_Exception()
{
    if (0 == m_type_Exception)
    {
        m_type_Exception = get_type(
            S"unoidl.com.sun.star.uno.Exception", false /* no exc */ );
        if (0 == m_type_Exception)
        {
            // define hardcoded type unoidl.com.sun.star.uno.Exception
            Emit::TypeBuilder * type_builder =
                m_module_builder->DefineType(
                      S"unoidl.com.sun.star.uno.Exception",
                      (TypeAttributes) (TypeAttributes::Public |
                                        TypeAttributes::BeforeFieldInit |
                                        TypeAttributes::AnsiClass),
                      __typeof (::System::Exception) );
            Emit::FieldBuilder * field_Context = type_builder->DefineField(
                S"Context", __typeof (::System::Object),
                FieldAttributes::Public );
            // default .ctor
            type_builder->DefineDefaultConstructor( c_ctor_method_attr );
            // .ctor
            ::System::Type * param_types __gc [] =
                  new ::System::Type * __gc [ 2 ];
            param_types[ 0 ] = __typeof (::System::String);
            param_types[ 1 ] = __typeof (::System::Object);
            Emit::ConstructorBuilder * ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, S"Message" );
            ctor_builder->DefineParameter(
                2, ParameterAttributes::In, S"Context" );
            Emit::ILGenerator * code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            param_types = new ::System::Type * __gc [ 1 ];
            param_types[ 0 ] = __typeof (::System::String);
            code->Emit(
                Emit::OpCodes::Call,
                __typeof (::System::Exception)
                  ->GetConstructor( param_types ) );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_2 );
            code->Emit( Emit::OpCodes::Stfld, field_Context );
            code->Emit( Emit::OpCodes::Ret );

            if (g_verbose)
            {
                ::System::Console::WriteLine(
                    "> emitting exception type "
                    "unoidl.com.sun.star.uno.Exception" );
            }
            m_type_Exception = type_builder->CreateType();
        }
    }
    return m_type_Exception;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type_RuntimeException()
{
    if (0 == m_type_RuntimeException)
    {
        m_type_RuntimeException = get_type(
            S"unoidl.com.sun.star.uno.RuntimeException", false /* no exc */ );
        if (0 == m_type_RuntimeException)
        {
            // define hardcoded type unoidl.com.sun.star.uno.RuntimeException
            ::System::Type * type_Exception = get_type_Exception();
            Emit::TypeBuilder * type_builder =
                  m_module_builder->DefineType(
                      S"unoidl.com.sun.star.uno.RuntimeException",
                      (TypeAttributes) (TypeAttributes::Public |
                                        TypeAttributes::BeforeFieldInit |
                                        TypeAttributes::AnsiClass),
                      type_Exception );
            // default .ctor
            type_builder->DefineDefaultConstructor( c_ctor_method_attr );
            // .ctor
            ::System::Type * param_types __gc [] =
                  new ::System::Type * __gc [ 2 ];
            param_types[ 0 ] = __typeof (::System::String);
            param_types[ 1 ] = __typeof (::System::Object);
            Emit::ConstructorBuilder * ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, S"Message" );
            ctor_builder->DefineParameter(
                2, ParameterAttributes::In, S"Context" );
            Emit::ILGenerator * code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Ldarg_2 );
            code->Emit(
                Emit::OpCodes::Call,
                type_Exception->GetConstructor( param_types ) );
            code->Emit( Emit::OpCodes::Ret );

            if (g_verbose)
            {
                ::System::Console::WriteLine(
                    "> emitting exception type "
                    "unoidl.com.sun.star.uno.RuntimeException" );
            }
            m_type_RuntimeException = type_builder->CreateType();
        }
    }
    return m_type_RuntimeException;
}

//______________________________________________________________________________
ConstructorInfo * TypeEmitter::get_ctor_uno_MethodAttribute()
{
    if (0 == m_ctor_uno_MethodAttribute)
    {
        ::System::Type * type_MethodAttribute =
              get_type( S"uno.MethodAttribute", false /* no exc */ );
        if (0 == type_MethodAttribute)
        {
            // define hardcoded type uno.MethodAttribute
            Emit::TypeBuilder * type_builder =
                m_module_builder->DefineType(
                    S"uno.MethodAttribute",
                    (TypeAttributes) (TypeAttributes::Public |
                                      TypeAttributes::Sealed |
                                      TypeAttributes::BeforeFieldInit |
                                      TypeAttributes::AnsiClass),
                    __typeof (::System::Attribute) );
            Emit::FieldBuilder * field_m_raises =
                type_builder->DefineField(
                    S"m_raises", __typeof (::System::Type * __gc []),
                    FieldAttributes::Private );
            Emit::FieldBuilder * field_m_oneway =
                type_builder->DefineField(
                    S"m_oneway", __typeof (bool), FieldAttributes::Private );
            Emit::FieldBuilder * field_m_attribute_method =
                type_builder->DefineField(
                    S"m_attribute_method", __typeof (bool),
                    FieldAttributes::Private );

            // apply System.AttributeUsage attribute
            ::System::Type * params __gc [] = new ::System::Type * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::AttributeTargets);
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 1 ];
            args[ 0 ] = __box (::System::AttributeTargets::Method);
            PropertyInfo * props __gc [] = new PropertyInfo * __gc [ 1 ];
            props[ 0 ] = __typeof (::System::AttributeUsageAttribute)
                ->GetProperty( S"Inherited" );
            ::System::Object * props_args __gc [] =
                  new ::System::Object * __gc [ 1 ];
            props_args[ 0 ] = __box (false);
            type_builder->SetCustomAttribute(
                new Emit::CustomAttributeBuilder(
                    __typeof (::System::AttributeUsageAttribute)
                      ->GetConstructor( params ), args,
                    props, props_args ) );

            // default .ctor
            Emit::ConstructorBuilder * ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    new ::System::Type * __gc [ 0 ] );
            Emit::ILGenerator * code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            // protected System.Attribute ctor
            ConstructorInfo * base_ctor =
                __typeof (::System::Attribute) ->GetConstructor(
                    (BindingFlags) (BindingFlags::Instance |
                                    BindingFlags::NonPublic),
                    0, new ::System::Type * __gc [ 0 ], 0 );
            code->Emit( Emit::OpCodes::Call, base_ctor );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldnull );
            code->Emit( Emit::OpCodes::Stfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldc_I4_0 );
            code->Emit( Emit::OpCodes::Stfld, field_m_oneway );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldc_I4_0 );
            code->Emit( Emit::OpCodes::Stfld, field_m_attribute_method );
            code->Emit( Emit::OpCodes::Ret );

            // .ctor
            ::System::Type * param_types __gc [] =
                  new ::System::Type * __gc [ 3 ];
            param_types[ 0 ] = __typeof (::System::Type * __gc []);
            param_types[ 1 ] = __typeof (bool);
            param_types[ 2 ] = __typeof (bool);
            ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, S"Raises" );
            ctor_builder->DefineParameter(
                2, ParameterAttributes::In, S"OneWay" );
            ctor_builder->DefineParameter(
                3, ParameterAttributes::In, S"AttributeMethod" );
            code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Call, base_ctor );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_2 );
            code->Emit( Emit::OpCodes::Stfld, field_m_oneway );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_3 );
            code->Emit( Emit::OpCodes::Stfld, field_m_attribute_method );
            code->Emit( Emit::OpCodes::Ret );

            // property Raises
            const MethodAttributes c_property_method_attr = (MethodAttributes)
                (MethodAttributes::Public |
                 MethodAttributes::HideBySig |
                 MethodAttributes::SpecialName |
                 MethodAttributes::Instance);
            ::System::Type * no_params __gc [] =
                  new ::System::Type * __gc [ 0 ];
            Emit::PropertyBuilder * property_builder =
                type_builder->DefineProperty(
                    S"Raises", PropertyAttributes::None,
                    __typeof (::System::Type * __gc []), no_params );
            // getter
            Emit::MethodBuilder * method_builder =
                type_builder->DefineMethod(
                    S"get_Raises", c_property_method_attr,
                    __typeof (::System::Type * __gc []), no_params );
            code = method_builder->GetILGenerator();
            Emit::LocalBuilder * local_ar0 =
                code->DeclareLocal( __typeof (::System::Type * __gc []) );
            Emit::LocalBuilder * local_ar1 =
                code->DeclareLocal( __typeof (::System::Type * __gc []) );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_raises );
            Emit::Label label_else = code->DefineLabel();
            code->Emit( Emit::OpCodes::Brtrue_S, label_else );
            code->Emit( Emit::OpCodes::Ldc_I4_1 );
            code->Emit( Emit::OpCodes::Newarr, __typeof (::System::Type) );
            code->Emit( Emit::OpCodes::Stloc, local_ar1 );
            code->Emit( Emit::OpCodes::Ldloc, local_ar1 );
            code->Emit( Emit::OpCodes::Ldc_I4_0 );
            code->Emit( Emit::OpCodes::Ldtoken, get_type_RuntimeException() );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_ar1 );
            code->Emit( Emit::OpCodes::Ret );
            code->MarkLabel( label_else );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ldlen );
            code->Emit( Emit::OpCodes::Conv_I4 );
            code->Emit( Emit::OpCodes::Ldc_I4_1 );
            code->Emit( Emit::OpCodes::Add );
            code->Emit( Emit::OpCodes::Conv_Ovf_U4 );
            code->Emit( Emit::OpCodes::Newarr, __typeof (::System::Type) );
            code->Emit( Emit::OpCodes::Stloc, local_ar0 );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ldloc, local_ar0 );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ldlen );
            code->Emit( Emit::OpCodes::Conv_I4 );

            param_types = new ::System::Type * __gc [ 3 ];
            param_types[ 0 ] = __typeof (::System::Array);
            param_types[ 1 ] = __typeof (::System::Array);
            param_types[ 2 ] = __typeof (::System::Int32);
            code->Emit(
                Emit::OpCodes::Call,
                __typeof (::System::Array)->GetMethod( "Copy", param_types ) );
            code->Emit( Emit::OpCodes::Ldloc, local_ar0 );
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ldlen );
            code->Emit( Emit::OpCodes::Conv_I4 );
            code->Emit( Emit::OpCodes::Ldtoken, get_type_RuntimeException() );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Stelem_Ref );
            code->Emit( Emit::OpCodes::Ldloc, local_ar0 );
            code->Emit( Emit::OpCodes::Ret );
            property_builder->SetGetMethod( method_builder );
            // setter
            params = new ::System::Type * __gc [ 1 ];
            params[ 0 ] = __typeof (::System::Type * __gc []);
            method_builder =
                type_builder->DefineMethod(
                    S"set_Raises", c_property_method_attr,
                    __typeof (void), params );
            method_builder->DefineParameter(
                1, ParameterAttributes::In, S"value" );
            code = method_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_m_raises );
            code->Emit( Emit::OpCodes::Ret );
            property_builder->SetSetMethod( method_builder );

            // property OneWay
            params[ 0 ] = __typeof (bool);
            property_builder =
                type_builder->DefineProperty(
                    S"OneWay", PropertyAttributes::None,
                    __typeof (bool), no_params );
            // getter
            method_builder =
                type_builder->DefineMethod(
                    S"get_OneWay", c_property_method_attr,
                    __typeof (bool), no_params );
            code = method_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_oneway );
            code->Emit( Emit::OpCodes::Ret );
            property_builder->SetGetMethod( method_builder );
            // setter
            method_builder =
                type_builder->DefineMethod(
                    S"set_OneWay", c_property_method_attr,
                    __typeof (void), params );
            method_builder->DefineParameter(
                1, ParameterAttributes::In, S"value" );
            code = method_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_m_oneway );
            code->Emit( Emit::OpCodes::Ret );
            property_builder->SetSetMethod( method_builder );

            // property AttributeMethod
            params[ 0 ] = __typeof (bool);
            property_builder =
                type_builder->DefineProperty(
                    S"AttributeMethod", PropertyAttributes::None,
                    __typeof (bool), no_params );
            // getter
            method_builder =
                type_builder->DefineMethod(
                    S"get_AttributeMethod", c_property_method_attr,
                    __typeof (bool), no_params );
            code = method_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldfld, field_m_attribute_method );
            code->Emit( Emit::OpCodes::Ret );
            property_builder->SetGetMethod( method_builder );
            // setter
            method_builder =
                type_builder->DefineMethod(
                    S"set_AttributeMethod", c_property_method_attr,
                    __typeof (void), params );
            method_builder->DefineParameter(
                1, ParameterAttributes::In, S"value" );
            code = method_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            code->Emit( Emit::OpCodes::Stfld, field_m_attribute_method );
            code->Emit( Emit::OpCodes::Ret );
            property_builder->SetSetMethod( method_builder );

            if (g_verbose)
            {
                ::System::Console::WriteLine(
                    "> emitting attribute type uno.MethodAttribute" );
            }
            type_MethodAttribute = type_builder->CreateType();
        }
        // retrieve default .ctor
        ::System::Type * param_types __gc [] = new ::System::Type * __gc [ 0 ];
        m_default_ctor_uno_MethodAttribute =
            type_MethodAttribute->GetConstructor( param_types );
        OSL_ASSERT( 0 != m_default_ctor_uno_MethodAttribute );
        // retrieve .ctor
        param_types = new ::System::Type * __gc [ 3 ];
        param_types[ 0 ] = __typeof (::System::Type * __gc []);
        param_types[ 1 ] = __typeof (bool);
        param_types[ 2 ] = __typeof (bool);
        m_ctor_uno_MethodAttribute =
            type_MethodAttribute->GetConstructor( param_types );
        OSL_ASSERT( 0 != m_ctor_uno_MethodAttribute );
    }
    return m_ctor_uno_MethodAttribute;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    Reference< reflection::XConstantTypeDescription > const & xType )
{
    ::System::String * cts_name = to_cts_name( xType->getName() );
    ::System::Type * ret_type = get_type( cts_name, false /* no exc */ );
    if (0 == ret_type)
    {
        Reference< reflection::XConstantTypeDescription > xConstant(
            xType, UNO_QUERY_THROW );
        ::System::Object * constant =
              to_cli_constant( xConstant->getConstantValue() );
        Emit::TypeBuilder * type_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::Sealed |
                                  TypeAttributes::BeforeFieldInit |
                                  TypeAttributes::AnsiClass) );

        Emit::FieldBuilder * field_builder = type_builder->DefineField(
            cts_name->Substring( cts_name->LastIndexOf( '.' ) +1 ),
            constant->GetType(),
            (FieldAttributes) (FieldAttributes::Public |
                               FieldAttributes::Static |
                               FieldAttributes::Literal) );
        field_builder->SetConstant( constant );

        if (g_verbose)
        {
            ::System::Console::WriteLine(
                "> emitting constant type {0}", cts_name );
        }
        ret_type = type_builder->CreateType();
    }
    return ret_type;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    Reference< reflection::XConstantsTypeDescription > const & xType )
{
    ::System::String * cts_name = to_cts_name( xType->getName() );
    ::System::Type * ret_type = get_type( cts_name, false /* no exc */ );
    if (0 == ret_type)
    {
        Emit::TypeBuilder * type_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::Sealed |
                                  TypeAttributes::BeforeFieldInit |
                                  TypeAttributes::AnsiClass) );

        Sequence< Reference<
            reflection::XConstantTypeDescription > > seq_constants(
                xType->getConstants() );
        Reference< reflection::XConstantTypeDescription > const * constants =
            seq_constants.getConstArray();
        sal_Int32 constants_length = seq_constants.getLength();
        for ( sal_Int32 constants_pos = 0;
              constants_pos < constants_length; ++constants_pos )
        {
            Reference<
                reflection::XConstantTypeDescription > const & xConstant =
                constants[ constants_pos ];
            ::System::Object * constant =
                  to_cli_constant( xConstant->getConstantValue() );
            ::System::String * uno_name =
                  ustring_to_String( xConstant->getName() );
            Emit::FieldBuilder * field_builder = type_builder->DefineField(
                uno_name->Substring( uno_name->LastIndexOf( '.' ) +1 ),
                constant->GetType(),
                (FieldAttributes) (FieldAttributes::Public |
                                   FieldAttributes::Static |
                                   FieldAttributes::Literal) );
            field_builder->SetConstant( constant );
        }

        if (g_verbose)
        {
            ::System::Console::WriteLine(
                "> emitting constants group type {0}", cts_name );
        }
        ret_type = type_builder->CreateType();
    }
    return ret_type;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    Reference< reflection::XEnumTypeDescription > const & xType )
{
    ::System::String * cts_name = to_cts_name( xType->getName() );
    ::System::Type * ret_type = get_type( cts_name, false /* no exc */ );
    if (0 == ret_type)
    {
//         Emit::EnumBuilder * enum_builder =
//             m_module_builder->DefineEnum(
//                 cts_name,
//                 (TypeAttributes) (TypeAttributes::Public |
// //                                   TypeAttributes::Sealed |
//                                   TypeAttributes::AnsiClass),
//                 __typeof (::System::Int32) );
        // workaround enum builder bug
        Emit::TypeBuilder * enum_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::Sealed),
                __typeof (::System::Enum) );
        enum_builder->DefineField(
            S"value__", __typeof (::System::Int32),
            (FieldAttributes) (FieldAttributes::Private |
                               FieldAttributes::SpecialName |
                               FieldAttributes::RTSpecialName) );
        Sequence< OUString > seq_enum_names( xType->getEnumNames() );
        Sequence< sal_Int32 > seq_enum_values( xType->getEnumValues() );
        sal_Int32 enum_length = seq_enum_names.getLength();
        OSL_ASSERT( enum_length == seq_enum_values.getLength() );
        OUString const * enum_names = seq_enum_names.getConstArray();
        sal_Int32 const * enum_values = seq_enum_values.getConstArray();
        for ( sal_Int32 enum_pos = 0; enum_pos < enum_length; ++enum_pos )
        {
//             enum_builder->DefineLiteral(
//                 ustring_to_String( enum_names[ enum_pos ] ),
//                 __box ((::System::Int32) enum_values[ enum_pos ]) );
            Emit::FieldBuilder * field_builder =
                enum_builder->DefineField(
                    ustring_to_String( enum_names[ enum_pos ] ),
                    enum_builder,
                    (FieldAttributes) (FieldAttributes::Public |
                                       FieldAttributes::Static |
                                       FieldAttributes::Literal) );
            field_builder->SetConstant(
                __box ((::System::Int32) enum_values[ enum_pos ]) );
        }

        if (g_verbose)
        {
            ::System::Console::WriteLine(
                "> emitting enum type {0}", cts_name );
        }
        ret_type = enum_builder->CreateType();
    }
    return ret_type;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    Reference< reflection::XCompoundTypeDescription > const & xType )
{
    OUString uno_name( xType->getName() );
    if (TypeClass_EXCEPTION == xType->getTypeClass())
    {
        if (uno_name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                       "com.sun.star.uno.Exception") ))
        {
            return get_type_Exception();
        }
        if (uno_name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                       "com.sun.star.uno.RuntimeException") ))
        {
            return get_type_RuntimeException();
        }
    }

    ::System::String * cts_name = to_cts_name( uno_name );
    ::System::Type * ret_type = get_type( cts_name, false /* no exc */ );
    if (0 == ret_type)
    {
        Reference< reflection::XCompoundTypeDescription > xBaseType(
            xType->getBaseType(), UNO_QUERY );
        ::System::Type * base_type = (xBaseType.is()
                                      ? get_type( xBaseType )
                                      : __typeof (::System::Object));
        Emit::TypeBuilder * type_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::BeforeFieldInit |
                                  TypeAttributes::AnsiClass),
                base_type );

        // optional: lookup base type whether generated entry of this session
        struct_entry * base_type_entry = 0;
        if (0 != base_type)
        {
            base_type_entry =
                dynamic_cast< struct_entry * >(
                    m_generated_structs->get_Item(
                        base_type->get_FullName() ) );
        }

        // members
        Sequence< Reference< reflection::XTypeDescription > > seq_members(
            xType->getMemberTypes() );
        Sequence< OUString > seq_member_names( xType->getMemberNames() );
        sal_Int32 members_length = seq_members.getLength();
        OSL_ASSERT( seq_member_names.getLength() == members_length );

        sal_Int32 all_members_length = 0;
        sal_Int32 member_pos;

        // collect base types; wrong order
        ::System::Collections::ArrayList * base_types_list =
              new ::System::Collections::ArrayList( 3 /* initial capacity */ );
        for ( ::System::Type * base_type_pos = base_type;
              ! base_type_pos->Equals( __typeof (::System::Object) );
              base_type_pos = base_type_pos->get_BaseType() )
        {
            base_types_list->Add( base_type_pos );
            if (base_type_pos->Equals( __typeof (::System::Exception) ))
            {
                // special Message member
                all_members_length += 1;
                break; // don't include System.Exception base classes
            }
            else
            {
                all_members_length +=
                    base_type_pos->GetFields(
                        (BindingFlags) (BindingFlags::Instance |
                                        BindingFlags::Public |
                                        BindingFlags::DeclaredOnly) )
                      ->get_Length();
            }
        }

        // create all_members arrays; right order
        ::System::String * all_member_names __gc [] =
              new ::System::String * __gc [
                  all_members_length + members_length ];
        ::System::Type * all_param_types __gc [] =
              new ::System::Type * __gc [
                  all_members_length + members_length ];
        member_pos = 0;
        for ( sal_Int32 pos = base_types_list->get_Count(); pos--; )
        {
            ::System::Type * base_type = __try_cast< ::System::Type * >(
                base_types_list->get_Item( pos ) );
            if (base_type->Equals( __typeof (::System::Exception) ))
            {
                all_member_names[ member_pos ] = S"Message";
                all_param_types[ member_pos ] = __typeof (::System::String);
                ++member_pos;
            }
            else
            {
                ::System::String * base_type_name = base_type->get_FullName();
                struct_entry * entry =
                    dynamic_cast< struct_entry * >(
                        m_generated_structs->get_Item( base_type_name ) );
                if (0 == entry)
                {
                    // complete type
                    FieldInfo * fields __gc [] =
                        base_type->GetFields(
                            (BindingFlags) (BindingFlags::Instance |
                                            BindingFlags::Public |
                                            BindingFlags::DeclaredOnly) );
                    sal_Int32 len = fields->get_Length();
                    for ( sal_Int32 pos = 0; pos < len; ++pos )
                    {
                        FieldInfo * field = fields[ pos ];
                        all_member_names[ member_pos ] = field->get_Name();
                        all_param_types[ member_pos ] = field->get_FieldType();
                        ++member_pos;
                    }
                }
                else // generated during this session:
                     // members may be incomplete ifaces
                {
                    sal_Int32 len = entry->m_member_names->get_Length();
                    for ( sal_Int32 pos = 0; pos < len; ++pos )
                    {
                        all_member_names[ member_pos ] =
                            entry->m_member_names[ pos ];
                        all_param_types[ member_pos ] =
                            entry->m_param_types[ pos ];
                        ++member_pos;
                    }
                }
            }
        }
        OSL_ASSERT( all_members_length == member_pos );

        // build up entry
        struct_entry * entry = new struct_entry();
        entry->m_member_names = new ::System::String * __gc [ members_length ];
        entry->m_param_types = new ::System::Type * __gc [ members_length ];

        // add members
        FieldInfo * members __gc [] = new FieldInfo * __gc [ members_length ];
        Reference< reflection::XTypeDescription > const * pseq_members =
            seq_members.getConstArray();
        OUString const * pseq_member_names =
            seq_member_names.getConstArray();
        for ( member_pos = 0; member_pos < members_length; ++member_pos )
        {
            ::System::String * field_name =
                  ustring_to_String( pseq_member_names[ member_pos ] );
            ::System::Type * field_type =
                  get_type( pseq_members[ member_pos ] );
            members[ member_pos ] =
                type_builder->DefineField(
                    field_name, field_type, FieldAttributes::Public );
            // add to all_members
            all_member_names[ all_members_length + member_pos ] = field_name;
            all_param_types[ all_members_length + member_pos ] = field_type;
            // add to entry
            entry->m_member_names[ member_pos ] = field_name;
            entry->m_param_types[ member_pos ] = field_type;
        }
        all_members_length += members_length;

        // default .ctor
        Emit::ConstructorBuilder * ctor_builder =
            type_builder->DefineConstructor(
                c_ctor_method_attr, CallingConventions::Standard,
                new ::System::Type * __gc [ 0 ] );
        Emit::ILGenerator * code = ctor_builder->GetILGenerator();
        code->Emit( Emit::OpCodes::Ldarg_0 );
        code->Emit(
            Emit::OpCodes::Call,
            0 == base_type_entry
            ? base_type->GetConstructor( new ::System::Type * __gc [ 0 ] )
            : base_type_entry->m_default_ctor );
        // default initialize members
        for ( member_pos = 0; member_pos < members_length; ++member_pos )
        {
            FieldInfo * field = members[ member_pos ];
            ::System::Type * field_type = field->get_FieldType();
            // default initialize:
            // string, type, enum, sequence, struct, exception, any
            if (field_type->Equals( __typeof (::System::String) ))
            {
                code->Emit( Emit::OpCodes::Ldarg_0 );
                code->Emit( Emit::OpCodes::Ldstr, S"" );
                code->Emit( Emit::OpCodes::Stfld, field );
            }
            else if (field_type->Equals( __typeof (::System::Type) ))
            {
                code->Emit( Emit::OpCodes::Ldarg_0 );
                code->Emit(
                    Emit::OpCodes::Ldtoken, __typeof (::System::Void) );
                code->Emit(
                    Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
                code->Emit( Emit::OpCodes::Stfld, field );
            }
            else if (field_type->get_IsArray())
            {
                code->Emit( Emit::OpCodes::Ldarg_0 );
                code->Emit( Emit::OpCodes::Ldc_I4_0 );
                code->Emit(
                    Emit::OpCodes::Newarr, field_type->GetElementType() );
                code->Emit( Emit::OpCodes::Stfld, field );
            }
            else if (field_type->get_IsValueType())
            {
                if (field_type->get_FullName()->Equals( S"uno.Any" ))
                {
                    code->Emit( Emit::OpCodes::Ldarg_0 );
                    code->Emit( Emit::OpCodes::Ldsfld, m_field_Any_VOID );
                    code->Emit( Emit::OpCodes::Stfld, field );
                }
            }
            else if (field_type->get_IsClass())
            {
                /* may be XInterface */
                if (! field_type->Equals( __typeof (::System::Object) ))
                {
                    // struct, exception
                    code->Emit( Emit::OpCodes::Ldarg_0 );
                    code->Emit(
                        Emit::OpCodes::Newobj,
                        field_type->GetConstructor(
                            new ::System::Type * __gc [ 0 ] ) );
                    code->Emit( Emit::OpCodes::Stfld, field );
                }
            }
        }
        code->Emit( Emit::OpCodes::Ret );
        entry->m_default_ctor = ctor_builder;

        // parameterized .ctor including all base members
        ctor_builder = type_builder->DefineConstructor(
            c_ctor_method_attr, CallingConventions::Standard, all_param_types );
        for ( member_pos = 0; member_pos < all_members_length; ++member_pos )
        {
            ctor_builder->DefineParameter(
                member_pos +1 /* starts with 1 */, ParameterAttributes::In,
                all_member_names[ member_pos ] );
        }
        code = ctor_builder->GetILGenerator();
        // call base .ctor
        code->Emit( Emit::OpCodes::Ldarg_0 ); // push this
        sal_Int32 base_members_length = all_members_length - members_length;
        ::System::Type * param_types __gc [] =
              new ::System::Type * __gc [ base_members_length ];
        for ( member_pos = 0; member_pos < base_members_length; ++member_pos )
        {
            emit_ldarg( code, member_pos +1 );
            param_types[ member_pos ] = all_param_types[ member_pos ];
        }
        code->Emit(
            Emit::OpCodes::Call,
            0 == base_type_entry
            ? base_type->GetConstructor( param_types )
            : base_type_entry->m_ctor );
        // initialize members
        for ( member_pos = 0; member_pos < members_length; ++member_pos )
        {
            code->Emit( Emit::OpCodes::Ldarg_0 ); // push this
            emit_ldarg( code, member_pos + base_members_length +1 );
            code->Emit( Emit::OpCodes::Stfld, members[ member_pos ] );
        }
        code->Emit( Emit::OpCodes::Ret );
        entry->m_ctor = ctor_builder;

        if (g_verbose)
        {
            ::System::Console::WriteLine(
                "> emitting {0} type {1}",
                TypeClass_STRUCT == xType->getTypeClass()
                ? S"struct"
                : S"exception",
                cts_name );
        }
        // new entry
        m_generated_structs->Add( cts_name, entry );
        ret_type = type_builder->CreateType();
    }
    return ret_type;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    Reference< reflection::XInterfaceTypeDescription > const & xType )
{
    OUString uno_name( xType->getName() );
    if (uno_name.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.uno.XInterface") ))
    {
        return __typeof (::System::Object);
    }

    ::System::String * cts_name = to_cts_name( xType->getName() );
    ::System::Type * ret_type = get_type( cts_name, false /* no exc */ );
    if (0 == ret_type)
    {
        Emit::TypeBuilder * type_builder;

        TypeAttributes attr = (TypeAttributes) (TypeAttributes::Public |
                                                TypeAttributes::Interface |
                                                TypeAttributes::Abstract |
                                                TypeAttributes::AnsiClass);

        Reference< reflection::XInterfaceTypeDescription > xBaseType(
            xType->getBaseType(), UNO_QUERY );
        if (xBaseType.is() &&
            !xBaseType->getName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.uno.XInterface") ))
        {
            ::System::Type * base_interfaces __gc [] =
                  new ::System::Type * __gc [ 1 ];
            base_interfaces[ 0 ] = get_type( xBaseType );
            type_builder = m_module_builder->DefineType(
                cts_name, attr, 0, base_interfaces );
        }
        else
        {
            if (! xBaseType.is())
            {
                ::System::Console::WriteLine(
                    "warning: IDL interface {0} is not derived from "
                    "com.sun.star.uno.XInterface!",
                    ustring_to_String( uno_name ) );
            }
            type_builder = m_module_builder->DefineType( cts_name, attr );
        }

        // insert to be completed
        iface_entry * entry = new iface_entry();
        xType->acquire();
        entry->m_xType = xType.get();
        entry->m_type_builder = type_builder;
        m_incomplete_ifaces->Add( cts_name, entry );

        // type is incomplete
        ret_type = type_builder;
    }
    return ret_type;
}

//______________________________________________________________________________
::System::Type * TypeEmitter::complete_iface_type( iface_entry * entry )
{
    Emit::TypeBuilder * type_builder = entry->m_type_builder;
    reflection::XInterfaceTypeDescription * xType = entry->m_xType;

    Reference< reflection::XTypeDescription > xBaseType( xType->getBaseType() );
    if (xBaseType.is())
    {
        ::System::String * basetype_name = to_cts_name( xBaseType->getName() );
        iface_entry * base_entry = dynamic_cast< iface_entry * >(
            m_incomplete_ifaces->get_Item( basetype_name ) );
        if (0 != base_entry)
        {
            // complete uncompleted base type first
            complete_iface_type( base_entry );
        }
    }

    ConstructorInfo * ctor_MethodAttribute = get_ctor_uno_MethodAttribute();
    Emit::CustomAttributeBuilder * default_MethodAttribute_builder =
        new Emit::CustomAttributeBuilder(
            m_default_ctor_uno_MethodAttribute,
            new ::System::Object * __gc [ 0 ] );

    Sequence<
        Reference< reflection::XInterfaceMemberTypeDescription > > seq_members(
            xType->getMembers() );
    Reference< reflection::XInterfaceMemberTypeDescription > const * members =
        seq_members.getConstArray();
    sal_Int32 members_length = seq_members.getLength();
    for ( sal_Int32 members_pos = 0;
          members_pos < members_length; ++members_pos )
    {
        Reference<
            reflection::XInterfaceMemberTypeDescription > const & xMember =
            members[ members_pos ];
        Sequence< Reference< reflection::XTypeDescription > > seq_exceptions;
        Emit::MethodBuilder * method_builder;

        const MethodAttributes c_method_attr = (MethodAttributes)
            (MethodAttributes::Public |
             MethodAttributes::Abstract |
             MethodAttributes::Virtual |
             MethodAttributes::NewSlot |
             MethodAttributes::HideBySig |
             MethodAttributes::Instance);

        if (TypeClass_INTERFACE_METHOD == xMember->getTypeClass())
        {
            Reference< reflection::XInterfaceMethodTypeDescription > xMethod(
                xMember, UNO_QUERY_THROW );

            Sequence<
                Reference< reflection::XMethodParameter > > seq_parameters(
                    xMethod->getParameters() );
            sal_Int32 params_length = seq_parameters.getLength();
            ::System::Type * param_types __gc [] =
                  new ::System::Type * __gc [ params_length ];
            Reference< reflection::XMethodParameter > const * parameters =
                seq_parameters.getConstArray();
            // first determine all types
            sal_Int32 params_pos = 0;
            for ( ; params_pos < params_length; ++params_pos )
            {
                Reference< reflection::XMethodParameter > const & xParam =
                    parameters[ params_pos ];
                ::System::Type * param_type = get_type( xParam->getType() );
                ::System::String * param_type_name = param_type->get_FullName();
                if (xParam->isOut())
                {
                    param_type = get_type(
                        ::System::String::Concat(
                            param_type_name, S"&" ), true );
                }
                param_types[ xParam->getPosition() ] = param_type;
            }
            // create method
            method_builder = type_builder->DefineMethod(
                ustring_to_String( xMethod->getMemberName() ),
                c_method_attr, get_type( xMethod->getReturnType() ),
                param_types );
            // then define parameter infos
            params_pos = 0;
            for ( ; params_pos < params_length; ++params_pos )
            {
                Reference< reflection::XMethodParameter > const & xParam =
                    parameters[ params_pos ];
                long param_flags = 0;
                if (xParam->isIn())
                    param_flags |= ParameterAttributes::In;
                if (xParam->isOut())
                    param_flags |= ParameterAttributes::Out;
                OSL_ASSERT( 0 != param_flags );
                method_builder->DefineParameter(
                    xParam->getPosition() +1 /* starts with 1 */,
                    (ParameterAttributes) param_flags,
                    ustring_to_String( xParam->getName() ) );
            }
            seq_exceptions = xMethod->getExceptions();

            // uno method attribute
            Reference< reflection::XTypeDescription > const * exceptions =
                seq_exceptions.getConstArray();
            sal_Int32 exc_length = seq_exceptions.getLength();
            if (0 == exc_length) // opt
            {
                method_builder->SetCustomAttribute(
                    default_MethodAttribute_builder );
            }
            else
            {
                ::System::Type * exception_types __gc [] =
                      new ::System::Type * __gc [ exc_length ];
                for ( sal_Int32 exc_pos = 0; exc_pos < exc_length; ++exc_pos )
                {
                    Reference< reflection::XTypeDescription > const & xExc =
                        exceptions[ exc_pos ];
                    exception_types[ exc_pos ] = get_type( xExc );
                }
                ::System::Object * args __gc [] =
                      new ::System::Object * __gc [ 3 ];
                args[ 0 ] = exception_types;
                args[ 1 ] = __box (sal_False != xMethod->isOneway());
                args[ 2 ] = __box (false);
                method_builder->SetCustomAttribute(
                    new Emit::CustomAttributeBuilder(
                        ctor_MethodAttribute, args ) );
            }
        }
        else // attribute
        {
            OSL_ASSERT(
                TypeClass_INTERFACE_ATTRIBUTE == xMember->getTypeClass() );
            Reference<
                reflection::XInterfaceAttributeTypeDescription > xAttribute(
                    xMember, UNO_QUERY_THROW );

            const MethodAttributes c_property_method_attr = (MethodAttributes)
                (c_method_attr | MethodAttributes::SpecialName);

            ::System::Type * attribute_type = get_type( xAttribute->getType() );
            ::System::Type * parameters __gc [] =
                  new ::System::Type * __gc [ 0 ];

            Emit::PropertyBuilder * property_builder =
                type_builder->DefineProperty(
                    ustring_to_String( xAttribute->getMemberName() ),
                    PropertyAttributes::None,
                    attribute_type, parameters );

            // getter
            Emit::MethodBuilder * method_builder =
                type_builder->DefineMethod(
                    ustring_to_String( OUSTR("get_") +
                                       xAttribute->getMemberName() ),
                    c_property_method_attr, attribute_type, parameters );

            // exception spec
            ::System::Object * args __gc [] = new ::System::Object * __gc [ 3 ];
            args[ 0 ] = 0; // implies com.sun.star.uno.RuntimeException
            args[ 1 ] = __box (false);
            args[ 2 ] = __box (true); // is AttributeMethod
            Emit::CustomAttributeBuilder * attribute_builder =
                new Emit::CustomAttributeBuilder( ctor_MethodAttribute, args );

            method_builder->SetCustomAttribute( attribute_builder );
            property_builder->SetGetMethod( method_builder );

            if (! xAttribute->isReadOnly())
            {
                // setter
                parameters = new ::System::Type * __gc [ 1 ];
                parameters[ 0 ] = attribute_type;
                method_builder =
                    type_builder->DefineMethod(
                        ustring_to_String( OUSTR("set_") +
                                           xAttribute->getMemberName() ),
                        c_property_method_attr, 0, parameters );
                // define parameter info
                method_builder->DefineParameter(
                    1 /* starts with 1 */, ParameterAttributes::In, S"value" );
                method_builder->SetCustomAttribute( attribute_builder );
                property_builder->SetSetMethod( method_builder );
            }
        }
    }

    // remove from incomplete types map
    ::System::String * cts_name = type_builder->get_FullName();
    m_incomplete_ifaces->Remove( cts_name );
    xType->release();

    if (g_verbose)
    {
        ::System::Console::WriteLine(
            "> emitting interface type {0}", cts_name );
    }
    return type_builder->CreateType();
}

//______________________________________________________________________________
::System::Type * TypeEmitter::get_type(
    Reference< reflection::XTypeDescription > const & xType )
{
    switch (xType->getTypeClass())
    {
    case TypeClass_VOID:
        return __typeof (::System::Void);
    case TypeClass_CHAR:
        return __typeof (::System::Char);
    case TypeClass_BOOLEAN:
        return __typeof (::System::Boolean);
    case TypeClass_BYTE:
        return __typeof (::System::Byte);
    case TypeClass_SHORT:
        return __typeof (::System::Int16);
    case TypeClass_UNSIGNED_SHORT:
        return __typeof (::System::UInt16);
    case TypeClass_LONG:
        return __typeof (::System::Int32);
    case TypeClass_UNSIGNED_LONG:
        return __typeof (::System::UInt32);
    case TypeClass_HYPER:
        return __typeof (::System::Int64);
    case TypeClass_UNSIGNED_HYPER:
        return __typeof (::System::UInt64);
    case TypeClass_FLOAT:
        return __typeof (::System::Single);
    case TypeClass_DOUBLE:
        return __typeof (::System::Double);
    case TypeClass_STRING:
        return __typeof (::System::String);
    case TypeClass_TYPE:
        return __typeof (::System::Type);
    case TypeClass_ANY:
        return get_type_Any();
    case TypeClass_ENUM:
        return get_type( Reference< reflection::XEnumTypeDescription >(
                             xType, UNO_QUERY_THROW ) );
    case TypeClass_TYPEDEF:
        // unwind typedefs
        return get_type(
            Reference< reflection::XIndirectTypeDescription >(
                xType, UNO_QUERY_THROW )->getReferencedType() );
    case TypeClass_STRUCT:
    case TypeClass_EXCEPTION:
        return get_type(
            Reference< reflection::XCompoundTypeDescription >(
                xType, UNO_QUERY_THROW ) );
    case TypeClass_SEQUENCE:
    {
        ::System::Type * element_type = get_type(
            Reference< reflection::XIndirectTypeDescription >(
                xType, UNO_QUERY_THROW )->getReferencedType() );
        return get_type(
            ::System::String::Concat(
                element_type->get_FullName(), S"[]" ), true );
    }
    case TypeClass_INTERFACE:
        return get_type(
            Reference< reflection::XInterfaceTypeDescription >(
                xType, UNO_QUERY_THROW ) );
    case TypeClass_CONSTANT:
        return get_type(
            Reference< reflection::XConstantTypeDescription >(
                xType, UNO_QUERY_THROW ) );
    case TypeClass_CONSTANTS:
        return get_type(
            Reference< reflection::XConstantsTypeDescription >(
                xType, UNO_QUERY_THROW ) );
    case TypeClass_SERVICE:
    case TypeClass_MODULE:
    case TypeClass_SINGLETON:
        // ignore these
        return 0;
    default:
        throw RuntimeException(
            OUSTR("unexpected type ") + xType->getName(),
            Reference< XInterface >() );
    }
}

//______________________________________________________________________________
void TypeEmitter::Dispose()
{
    while (true)
    {
        ::System::Collections::IDictionaryEnumerator * enumerator =
              m_incomplete_ifaces->GetEnumerator();
        if (! enumerator->MoveNext())
            break;
        complete_iface_type(
            __try_cast< iface_entry * >( enumerator->get_Value() ) );
    }
    m_incomplete_ifaces = 0;
}
//______________________________________________________________________________
TypeEmitter::TypeEmitter(
    ::System::Reflection::Emit::ModuleBuilder * module_builder,
    ::System::Reflection::Assembly * extra_assemblies __gc [] )
    : m_module_builder( module_builder ),
      m_extra_assemblies( extra_assemblies ),
      m_method_info_Type_GetTypeFromHandle( 0 ),
      m_type_Exception( 0 ),
      m_type_RuntimeException( 0 ),
      m_type_Any( 0 ),
      m_field_Any_VOID( 0 ),
      m_ctor_uno_MethodAttribute( 0 ),
      m_default_ctor_uno_MethodAttribute( 0 ),
      m_incomplete_ifaces( new ::System::Collections::Hashtable() ),
      m_generated_structs( new ::System::Collections::Hashtable() )
{
    ::System::Type * param_types __gc [] = new ::System::Type * __gc [ 1 ];
    param_types[ 0 ] = __typeof (::System::RuntimeTypeHandle);
    m_method_info_Type_GetTypeFromHandle =
        __typeof (::System::Type)
          ->GetMethod( "GetTypeFromHandle", param_types );
}

}
