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

#include "climaker_share.h"

#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp"
#include <vector>

using namespace ::System::Reflection;

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace climaker
{
System::String^ mapUnoPolymorphicName(System::String^ unoName);
//------------------------------------------------------------------------------
static inline ::System::String ^ to_cts_name(
    OUString const & uno_name )
{
    OUStringBuffer buf( 7 + uno_name.getLength() );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("unoidl.") );
    buf.append( uno_name );
    return ustring_to_String( buf.makeStringAndClear() );
}

//------------------------------------------------------------------------------
static inline ::System::Object ^ to_cli_constant( Any const & value )
{
    switch (value.getValueTypeClass())
    {
    case TypeClass_CHAR:
        return ((::System::Char) *reinterpret_cast< sal_Unicode const * >(
                value.getValue() ));
    case TypeClass_BOOLEAN:
        return ((::System::Boolean)
             sal_False != *reinterpret_cast< sal_Bool const * >(
                 value.getValue() ));
    case TypeClass_BYTE:
        return ((::System::Byte) *reinterpret_cast< sal_Int8 const * >(
                value.getValue() ));
    case TypeClass_SHORT:
        return ((::System::Int16) *reinterpret_cast< sal_Int16 const * >(
                value.getValue() ));
    case TypeClass_UNSIGNED_SHORT:
        return ((::System::UInt16) *reinterpret_cast< sal_uInt16 const * >(
                value.getValue() ));
    case TypeClass_LONG:
        return ((::System::Int32) *reinterpret_cast< sal_Int32 const * >(
                value.getValue() ));
    case TypeClass_UNSIGNED_LONG:
        return ((::System::UInt32) *reinterpret_cast< sal_uInt32 const * >(
                value.getValue() ));
    case TypeClass_HYPER:
        return ((::System::Int64) *reinterpret_cast< sal_Int64 const * >(
                value.getValue() ));
    case TypeClass_UNSIGNED_HYPER:
        return ((::System::UInt64) *reinterpret_cast< sal_uInt64 const * >(
                value.getValue() ));
    case TypeClass_FLOAT:
        return ((::System::Single) *reinterpret_cast< float const * >(
                value.getValue() ));
    case TypeClass_DOUBLE:
        return ((::System::Double) *reinterpret_cast< double const * >(
                value.getValue() ));
    default:
        throw RuntimeException(
            OUSTR("unexpected constant type ") +
            value.getValueType().getTypeName(),
            Reference< XInterface >() );
    }
}

//------------------------------------------------------------------------------
static inline void emit_ldarg( Emit::ILGenerator ^ code, ::System::Int32 index )
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

void polymorphicStructNameToStructName(::System::String ^* sPolyName)
{
    if ((*sPolyName)->EndsWith(">") == false)
        return;

    int index = (*sPolyName)->IndexOf('<');
    OSL_ASSERT(index != -1);
    *sPolyName = (*sPolyName)->Substring(0, index);
}


System::String^ mapUnoTypeName(System::String ^ typeName)
{
    ::System::Text::StringBuilder^ buf= gcnew System::Text::StringBuilder();
    ::System::String ^ sUnoName = ::System::String::Copy(typeName);
    //determine if the type is a sequence and its dimensions
    int dims= 0;
    if (typeName->StartsWith("["))//if (usUnoName[0] == '[')
    {
        int index= 1;
        while (true)
        {
            if (typeName[index++] == ']')//if (usUnoName[index++] == ']')
                dims++;
            if (typeName[index++] != '[')//usUnoName[index++] != '[')
                break;
        }
        sUnoName = sUnoName->Substring(index - 1);//usUnoName = usUnoName.copy(index - 1);
    }
    if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoBool)))
        buf->Append(const_cast<System::String^>(Constants::sBoolean));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoChar)))
        buf->Append(const_cast<System::String^>(Constants::sChar));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoByte)))
        buf->Append(const_cast<System::String^>(Constants::sByte));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoShort)))
        buf->Append(const_cast<System::String^>(Constants::sInt16));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoUShort)))
        buf->Append(const_cast<System::String^>(Constants::sUInt16));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoLong)))
        buf->Append(const_cast<System::String^>(Constants::sInt32));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoULong)))
        buf->Append(const_cast<System::String^>(Constants::sUInt32));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoHyper)))
        buf->Append(const_cast<System::String^>(Constants::sInt64));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoUHyper)))
        buf->Append(const_cast<System::String^>(Constants::sUInt64));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoFloat)))
        buf->Append(const_cast<System::String^>(Constants::sSingle));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoDouble)))
        buf->Append(const_cast<System::String^>(Constants::sDouble));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoString)))
        buf->Append(const_cast<System::String^>(Constants::sString));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoVoid)))
        buf->Append(const_cast<System::String^>(Constants::sVoid));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoType)))
        buf->Append(const_cast<System::String^>(Constants::sType));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoXInterface)))
        buf->Append(const_cast<System::String^>(Constants::sObject));
    else if (sUnoName->Equals(const_cast<System::String^>(Constants::sUnoAny)))
    {
        buf->Append(const_cast<System::String^>(Constants::sAny));
    }
    else
    {
        //put "unoidl." at the beginning
        buf->Append(const_cast<System::String^>(Constants::sUnoidl));
        buf->Append(mapUnoPolymorphicName(sUnoName));
    }
    // apend []
    for (;dims--;)
        buf->Append(const_cast<System::String^>(Constants::sBrackets));

    return buf->ToString();
}


/** For example, there is a uno type
    com.sun.star.Foo<char, long>.
    The values in the type list
    are uno types and are replaced by cli types, such as System.Char,
    System.Int32, etc.

    Strings can be as complicated as this
    test.MyStruct<char,test.MyStruct<long, []string>>
 */
System::String^ mapUnoPolymorphicName(System::String^ unoName)
{
    int index = unoName->IndexOf('<');
    if (index == -1)
        return unoName;

    System::Text::StringBuilder ^ builder =
        gcnew System::Text::StringBuilder(unoName->Substring(0, index +1 ));

    //Find the first occurrence of ','
    //If the parameter is a polymorphic struct then we neede to ignore everything
    //between the brackets because it can also contain commas
    //get the type list within < and >
    int endIndex = unoName->Length - 1;
    index++;
    int cur = index;
    int countParams = 0;
    while (cur <= endIndex)
    {
        System::Char c = unoName[cur];
        if (c == ',' || c == '>')
        {
            //insert a comma if needed
            if (countParams != 0)
                builder->Append(",");
            countParams++;
            System::String ^ sParam = unoName->Substring(index, cur - index);
            //skip the comma
            cur++;
            //the the index to the beginning of the next param
            index = cur;
            builder->Append(mapUnoTypeName(sParam));
        }
        else if (c == '<')
        {
            cur++;
            //continue until the matching '>'
            int numNested = 0;
            for (;;cur++)
            {
                System::Char curChar = unoName[cur];
                if (curChar == '<')
                {
                    numNested ++;
                }
                else if (curChar == '>')
                {
                    if (numNested > 0)
                        numNested--;
                    else
                        break;
                }
            }
        }
        cur++;
    }

    builder->Append((System::Char) '>');
    return builder->ToString();
}



//______________________________________________________________________________
Assembly ^ TypeEmitter::type_resolve(
    ::System::Object ^, ::System::ResolveEventArgs ^ args )
{
    ::System::String ^ cts_name = args->Name;
    ::System::Type ^ ret_type;

    iface_entry ^ entry = dynamic_cast< iface_entry ^ >(m_incomplete_ifaces[cts_name] );
    if (nullptr != entry)
        ret_type = entry->m_type_builder;

    if (nullptr == ret_type)
    {
        sal_Int32 len = m_extra_assemblies->Length;
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            ret_type = m_extra_assemblies[ pos ]->GetType(
                cts_name, false /* no exc */ );
            if (nullptr != ret_type)
            {
                if (g_verbose)
                {
                    ::System::Console::WriteLine(
                        "> resolving type {0} from {1}.",
                        cts_name, ret_type->Assembly->FullName );
                }
                break;
            }
        }
    }
    if (nullptr != ret_type)
        return ret_type->Assembly;
    return nullptr;
}

//______________________________________________________________________________
::System::Type ^ TypeEmitter::get_type(
    ::System::String ^ cts_name, bool throw_exc )
{
    ::System::Type ^ ret_type = m_module_builder->GetType( cts_name, false );
    //We get the type from the ModuleBuilder even if the type is not complete
    //but have been defined.
    //if (ret_type == 0)
    //{
    //    iface_entry * entry = dynamic_cast< iface_entry * >(
    //        m_incomplete_ifaces->get_Item( cts_name ) );
    //    if (0 != entry)
    //        ret_type = entry->m_type_builder;
    //}
        //try the cli_basetypes assembly
    if (ret_type == nullptr)
    {
        ::System::Text::StringBuilder ^ builder = gcnew ::System::Text::StringBuilder(cts_name);
        builder->Append(",cli_basetypes");
        ret_type = ::System::Type::GetType(builder->ToString());
    }

    if (ret_type == nullptr)
    {
        try
        {
            // may call on type_resolve()
            return ::System::Type::GetType( cts_name, throw_exc );
        }
        catch (::System::Exception^ exc)
        {
            //If the type is not found one may have forgotten to specify assemblies with
            //additional types
            ::System::Text::StringBuilder ^ sb = gcnew ::System::Text::StringBuilder();
            sb->Append(gcnew ::System::String("\nThe type "));
            sb->Append(cts_name);
            sb->Append(gcnew ::System::String(" \n could not be found. Did you forget to " \
                "specify an additional assembly with the --reference option?\n"));
            if (throw_exc)
                throw gcnew ::System::Exception(sb->ToString(), exc);
        }
    }
    else
    {
        return ret_type;
    }
}

//______________________________________________________________________________
::System::Type ^ TypeEmitter::get_type_Exception()
{
    if (nullptr == m_type_Exception)
    {
        m_type_Exception = get_type(
            "unoidl.com.sun.star.uno.Exception", false /* no exc */ );
        if (nullptr == m_type_Exception)
        {
            // define hardcoded type unoidl.com.sun.star.uno.Exception
            Emit::TypeBuilder ^ type_builder =
                m_module_builder->DefineType(
                      "unoidl.com.sun.star.uno.Exception",
                      (TypeAttributes) (TypeAttributes::Public |
                                        TypeAttributes::BeforeFieldInit |
                                        TypeAttributes::AnsiClass),
                      (::System::Exception::typeid) );
            Emit::FieldBuilder ^ field_Context = type_builder->DefineField(
                "Context", (::System::Object::typeid),
                FieldAttributes::Public );
            // default .ctor
            type_builder->DefineDefaultConstructor( c_ctor_method_attr );
            // .ctor
            array< ::System::Type^>^ param_types =
                  gcnew array< ::System::Type^>(2);
            param_types[ 0 ] = ::System::String::typeid;
            param_types[ 1 ] = ::System::Object::typeid;
            Emit::ConstructorBuilder ^ ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, "Message" );
            ctor_builder->DefineParameter(
                2, ParameterAttributes::In, "Context" );
            Emit::ILGenerator ^ code = ctor_builder->GetILGenerator();
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldarg_1 );
            param_types = gcnew array< ::System::Type^>(1);
            param_types[ 0 ] = ::System::String::typeid;
            code->Emit(
                Emit::OpCodes::Call,
                (::System::Exception::typeid)
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
::System::Type ^ TypeEmitter::get_type_RuntimeException()
{
    if (nullptr == m_type_RuntimeException)
    {
        m_type_RuntimeException = get_type(
            "unoidl.com.sun.star.uno.RuntimeException", false /* no exc */ );
        if (nullptr == m_type_RuntimeException)
        {
            // define hardcoded type unoidl.com.sun.star.uno.RuntimeException
            ::System::Type ^ type_Exception = get_type_Exception();
            Emit::TypeBuilder ^ type_builder =
                  m_module_builder->DefineType(
                      "unoidl.com.sun.star.uno.RuntimeException",
                      (TypeAttributes) (TypeAttributes::Public |
                                        TypeAttributes::BeforeFieldInit |
                                        TypeAttributes::AnsiClass),
                      type_Exception );
            // default .ctor
            type_builder->DefineDefaultConstructor( c_ctor_method_attr );
            // .ctor
            array< ::System::Type^>^ param_types =
                  gcnew array< ::System::Type^>(2);
            param_types[ 0 ] = ::System::String::typeid;
            param_types[ 1 ] = ::System::Object::typeid;
            Emit::ConstructorBuilder ^ ctor_builder =
                type_builder->DefineConstructor(
                    c_ctor_method_attr, CallingConventions::Standard,
                    param_types );
            ctor_builder->DefineParameter(
                1, ParameterAttributes::In, "Message" );
            ctor_builder->DefineParameter(
                2, ParameterAttributes::In, "Context" );
            Emit::ILGenerator ^ code = ctor_builder->GetILGenerator();
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
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XConstantTypeDescription > const & xType )
{
    ::System::String ^ cts_name = to_cts_name( xType->getName() );
    ::System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (nullptr == ret_type)
    {
        Reference< reflection::XConstantTypeDescription > xConstant(
            xType, UNO_QUERY_THROW );
        ::System::Object ^ constant =
              to_cli_constant( xConstant->getConstantValue() );
        Emit::TypeBuilder ^ type_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::Sealed |
                                  TypeAttributes::BeforeFieldInit |
                                  TypeAttributes::AnsiClass) );

        Emit::FieldBuilder ^ field_builder = type_builder->DefineField(
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
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XConstantsTypeDescription > const & xType )
{
    ::System::String ^ cts_name = to_cts_name( xType->getName() );
    ::System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (nullptr == ret_type)
    {
        Emit::TypeBuilder ^ type_builder =
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
            ::System::Object ^ constant =
                  to_cli_constant( xConstant->getConstantValue() );
            ::System::String ^ uno_name =
                  ustring_to_String( xConstant->getName() );
            Emit::FieldBuilder ^ field_builder = type_builder->DefineField(
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
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XEnumTypeDescription > const & xType )
{
    ::System::String ^ cts_name = to_cts_name( xType->getName() );
    ::System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (nullptr == ret_type)
    {
//         Emit::EnumBuilder * enum_builder =
//             m_module_builder->DefineEnum(
//                 cts_name,
//                 (TypeAttributes) (TypeAttributes::Public |
// //                                   TypeAttributes::Sealed |
//                                   TypeAttributes::AnsiClass),
//                 __typeof (::System::Int32) );
        // workaround enum builder bug
        Emit::TypeBuilder ^ enum_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::Sealed),
                ::System::Enum::typeid );
        enum_builder->DefineField(
            "value__", ::System::Int32::typeid,
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
            Emit::FieldBuilder ^ field_builder =
                enum_builder->DefineField(
                    ustring_to_String( enum_names[ enum_pos ] ),
                    enum_builder,
                    (FieldAttributes) (FieldAttributes::Public |
                                       FieldAttributes::Static |
                                       FieldAttributes::Literal) );
            field_builder->SetConstant(
                ((::System::Int32) enum_values[ enum_pos ]) );
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
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XCompoundTypeDescription > const & xType )
{
    OUString uno_name( xType->getName() );
    if (TypeClass_EXCEPTION == xType->getTypeClass())
    {
        if ( uno_name == "com.sun.star.uno.Exception" )
        {
            return get_type_Exception();
        }
        if ( uno_name == "com.sun.star.uno.RuntimeException" )
        {
            return get_type_RuntimeException();
        }
    }
    ::System::String ^ cts_name = to_cts_name( uno_name );
    // if the struct is an instantiated polymorpic struct then we create the simple struct name
    // For example:
    // void func ([in] PolyStruct<boolean> arg);
    //PolyStruct<boolean> will be converted to PolyStruct
    polymorphicStructNameToStructName( & cts_name);

    ::System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (nullptr == ret_type)
    {
        Reference< reflection::XCompoundTypeDescription > xBaseType(
            xType->getBaseType(), UNO_QUERY );
        ::System::Type ^ base_type = (xBaseType.is()
                                      ? get_type( xBaseType )
                                      : ::System::Object::typeid);
        Emit::TypeBuilder ^ type_builder =
            m_module_builder->DefineType(
                cts_name,
                (TypeAttributes) (TypeAttributes::Public |
                                  TypeAttributes::BeforeFieldInit |
                                  TypeAttributes::AnsiClass),
                base_type );


         // insert to be completed
        struct_entry ^ entry = gcnew struct_entry();
        xType->acquire();
        entry->m_xType = xType.get();
        entry->m_type_builder = type_builder;
        entry->m_base_type = base_type;
        m_incomplete_structs->Add( cts_name, entry );

        // type is incomplete
        ret_type = type_builder;
    }

    //In case of an instantiated polymorphic struct we want to return a
    //uno.PolymorphicType (inherits Type) rather then Type. This is neaded for constructing
    //the service code. We can only do that if the struct is completed.
    if (m_generated_structs[cts_name])
    {
        Reference< reflection::XStructTypeDescription> xStructTypeDesc(
            xType, UNO_QUERY);

        if (xStructTypeDesc.is())
        {
            Sequence< Reference< reflection::XTypeDescription > > seqTypeArgs = xStructTypeDesc->getTypeArguments();
            sal_Int32 numTypes = seqTypeArgs.getLength();
            if (numTypes > 0)
            {
                //it is an instantiated polymorphic struct
                ::System::String ^ sCliName = mapUnoTypeName(ustring_to_String(xType->getName()));
                ret_type = ::uno::PolymorphicType::GetType(ret_type, sCliName);
            }
        }
    }
    return ret_type;
}

//______________________________________________________________________________
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XInterfaceTypeDescription2 > const & xType )
{
    OUString uno_name( xType->getName() );
    if ( uno_name == "com.sun.star.uno.XInterface" )
    {
        return ::System::Object::typeid;
    }

    ::System::String ^ cts_name = to_cts_name( xType->getName() );
    ::System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (nullptr == ret_type)
    {
        Emit::TypeBuilder ^ type_builder;

        TypeAttributes attr = (TypeAttributes) (TypeAttributes::Public |
                                                TypeAttributes::Interface |
                                                TypeAttributes::Abstract |
                                                TypeAttributes::AnsiClass);

        std::vector<Reference<reflection::XInterfaceTypeDescription2> > vecBaseTypes;
        Sequence<Reference< reflection::XTypeDescription > > seqBaseTypes =
            xType->getBaseTypes();
        if (seqBaseTypes.getLength() > 0)
        {
            for (int i = 0; i < seqBaseTypes.getLength(); i++)
            {
                Reference<reflection::XInterfaceTypeDescription2> xIfaceTd =
                    resolveInterfaceTypedef(seqBaseTypes[i]);

                if ( xIfaceTd->getName() != "com.sun.star.uno.XInterface" )
                {
                    vecBaseTypes.push_back(xIfaceTd);
                }
            }

            array< ::System::Type^>^ base_interfaces =
                  gcnew array< ::System::Type^>( vecBaseTypes.size() );

            typedef std::vector<Reference<reflection::XInterfaceTypeDescription2> >::const_iterator it;
            int index = 0;
            for (it i = vecBaseTypes.begin(); i != vecBaseTypes.end(); ++i, ++index)
                base_interfaces[ index ] = get_type( *i );
            type_builder = m_module_builder->DefineType(
                cts_name, attr, nullptr, base_interfaces );
        }
        else
        {
            ::System::Console::WriteLine(
                "warning: IDL interface {0} is not derived from "
                "com.sun.star.uno.XInterface!",
                ustring_to_String( uno_name ) );

            type_builder = m_module_builder->DefineType( cts_name, attr );
        }

        // insert to be completed
        iface_entry ^ entry = gcnew iface_entry();
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
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XServiceTypeDescription2 > const & xType )
{
    if (xType->isSingleInterfaceBased() == sal_False)
        return nullptr;

    System::String ^ cts_name = to_cts_name( xType->getName() );
    System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (ret_type != nullptr)
        return ret_type;

    TypeAttributes attr = (TypeAttributes) (TypeAttributes::Public |
                                            TypeAttributes::Sealed |
                                            TypeAttributes::BeforeFieldInit |
                                            TypeAttributes::AnsiClass);

    Emit::TypeBuilder ^ type_builder = m_module_builder->DefineType(
        cts_name, attr);

    // insert to be completed
    service_entry ^ entry = gcnew service_entry();
    xType->acquire();
    entry->m_xType = xType.get();
    entry->m_type_builder = type_builder;
    m_incomplete_services->Add(cts_name,entry );

    return type_builder;
}

::System::Type ^ TypeEmitter::get_type(
    Reference<reflection::XSingletonTypeDescription2 > const & xType )
{
    if (xType->isInterfaceBased() == sal_False)
        return nullptr;

    ::System::String^ cts_name = to_cts_name( xType->getName() );
    ::System::Type ^ ret_type = get_type( cts_name, false /* no exc */ );
    if (ret_type != nullptr)
        return ret_type;

    TypeAttributes attr = static_cast<TypeAttributes>(
        TypeAttributes::Public |
        TypeAttributes::Sealed |
        TypeAttributes::BeforeFieldInit |
        TypeAttributes::AnsiClass);

    Emit::TypeBuilder ^ type_builder = m_module_builder->DefineType(
        cts_name, attr);

    // insert to be completed
    singleton_entry ^ entry = gcnew singleton_entry();
    xType->acquire();
    entry->m_xType = xType.get();
    entry->m_type_builder = type_builder;
    m_incomplete_singletons->Add(cts_name,entry );

    return type_builder;

}

//______________________________________________________________________________
::System::Type ^ TypeEmitter::complete_iface_type( iface_entry ^ entry )
{
    Emit::TypeBuilder ^ type_builder = entry->m_type_builder;
    reflection::XInterfaceTypeDescription2 * xType = entry->m_xType;

    Sequence<Reference< reflection::XTypeDescription > > seqBaseTypes( xType->getBaseTypes() );
    if (seqBaseTypes.getLength() > 0)
    {
        for (int i = 0; i < seqBaseTypes.getLength(); i++)
        {
            //make sure we get the interface rather then a typedef
            Reference<reflection::XInterfaceTypeDescription2> aBaseType =
                resolveInterfaceTypedef( seqBaseTypes[i]);

            if ( aBaseType->getName() != "com.sun.star.uno.XInterface" )
            {
                ::System::String ^ basetype_name = to_cts_name( aBaseType->getName() );
                iface_entry ^ base_entry = dynamic_cast< iface_entry ^ >(
                    m_incomplete_ifaces[basetype_name] );
                if (nullptr != base_entry)
                {
                // complete uncompleted base type first
                    complete_iface_type( base_entry );
                }
            }
        }
    }

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
        Emit::MethodBuilder ^ method_builder;

        MethodAttributes c_method_attr = (MethodAttributes)
            (MethodAttributes::Public |
             MethodAttributes::Abstract |
             MethodAttributes::Virtual |
             MethodAttributes::NewSlot |
             MethodAttributes::HideBySig);
//#if defined(_MSC_VER) && (_MSC_VER < 1400)
//             MethodAttributes::Instance);
//#else
//       Instance);
//#endif

        if (TypeClass_INTERFACE_METHOD == xMember->getTypeClass())
        {
            Reference< reflection::XInterfaceMethodTypeDescription > xMethod(
                xMember, UNO_QUERY_THROW );

            Sequence<
                Reference< reflection::XMethodParameter > > seq_parameters(
                    xMethod->getParameters() );
            sal_Int32 params_length = seq_parameters.getLength();
            array< ::System::Type^>^ param_types =
                  gcnew array< ::System::Type^>( params_length );
            Reference< reflection::XMethodParameter > const * parameters =
                seq_parameters.getConstArray();
            // first determine all types
            //Make the first param type as return type
            sal_Int32 params_pos = 0;
            for ( ; params_pos < params_length; ++params_pos )
            {
                Reference< reflection::XMethodParameter > const & xParam =
                    parameters[ params_pos ];
                ::System::Type ^ param_type = get_type( xParam->getType() );
                ::System::String ^ param_type_name = param_type->FullName;
                if (xParam->isOut())
                {
                    param_type = get_type(
                        ::System::String::Concat(
                            param_type_name, "&" ), true );
                }
                param_types[ xParam->getPosition() ] = param_type;
            }


            // create method
//             if (tb)
//                 method_builder = type_builder->DefineMethod(
//                 ustring_to_String( xMethod->getMemberName() ),
//                 c_method_attr, tb,
//                 param_types );
//             else
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
                    param_flags |= (long)ParameterAttributes::In;
                if (xParam->isOut())
                    param_flags |= (long)ParameterAttributes::Out;
                OSL_ASSERT( 0 != param_flags );
                method_builder->DefineParameter(
                    xParam->getPosition() +1 /* starts with 1 */,
                    (ParameterAttributes) param_flags,
                    ustring_to_String( xParam->getName() ) );
            }
            //Apply attribute TypeParametersAttribute to return value if it
            //is a parameterized Type. Currently only structs can have parameters.
            Reference<reflection::XStructTypeDescription> xReturnStruct(
                xMethod->getReturnType(), UNO_QUERY);

            if (xReturnStruct.is())
            {
                Sequence<Reference<reflection::XTypeDescription> > seq_type_args =
                    xReturnStruct->getTypeArguments();
                if (seq_type_args.getLength() != 0)
                {
                    //get th ctor of the attribute
                    array< ::System::Type^>^ arCtor = {::System::Type::GetType("System.Type[]")};
                    //Get the arguments for the attribute's ctor
                    Reference<reflection::XTypeDescription> const * arXTypeArgs =
                        seq_type_args.getConstArray();
                    int numTypes = seq_type_args.getLength();
                    array< ::System::Type^>^ arCtsTypes = gcnew array< ::System::Type^>(numTypes);
                    for (int i = 0; i < numTypes; i++)
                        arCtsTypes[i] = get_type(arXTypeArgs[i]);
                    array< ::System::Object^>^ arArgs = {arCtsTypes};

                    Emit::CustomAttributeBuilder ^ attrBuilder =
                        gcnew Emit::CustomAttributeBuilder(
                            ::uno::TypeArgumentsAttribute::typeid
                            ->GetConstructor( arCtor),
                            arArgs);

                    method_builder->SetCustomAttribute(attrBuilder);
                }
            }

            //define UNO exception attribute (exceptions)--------------------------------------
            Emit::CustomAttributeBuilder^ attrBuilder =
                get_iface_method_exception_attribute(xMethod);
            if (attrBuilder != nullptr)
                method_builder->SetCustomAttribute(attrBuilder);

            // oneway attribute
            if (xMethod->isOneway())
            {
                array< ::System::Type^>^ arCtorOneway = gcnew array< ::System::Type^>(0);
                array< ::System::Object^>^ arArgs = gcnew array< ::System::Object^>(0);
                Emit::CustomAttributeBuilder ^ attrBuilder =
                    gcnew Emit::CustomAttributeBuilder(
                        ::uno::OnewayAttribute::typeid->GetConstructor( arCtorOneway),
                        arArgs);
                method_builder->SetCustomAttribute(attrBuilder);
            }
        }
        else // attribute
        {
            OSL_ASSERT(
                TypeClass_INTERFACE_ATTRIBUTE == xMember->getTypeClass() );
            Reference<
                reflection::XInterfaceAttributeTypeDescription2 > xAttribute(
                    xMember, UNO_QUERY_THROW );

            MethodAttributes c_property_method_attr = (MethodAttributes)
                (c_method_attr | MethodAttributes::SpecialName);

            ::System::Type ^ attribute_type = get_type( xAttribute->getType() );
            array< ::System::Type^>^ parameters  =
                  gcnew array< ::System::Type^> ( 0 );

            Emit::PropertyBuilder ^ property_builder =
                type_builder->DefineProperty(
                    ustring_to_String( xAttribute->getMemberName() ),
                    PropertyAttributes::None,
                    attribute_type, parameters );

            //set BoundAttribute, if necessary
            if (xAttribute->isBound())
            {
                ConstructorInfo ^ ctorBoundAttr =
                    ::uno::BoundAttribute::typeid->GetConstructor(
                        gcnew array<System::Type^>(0));
                Emit::CustomAttributeBuilder ^ attrBuilderBound =
                    gcnew Emit::CustomAttributeBuilder(
                        ctorBoundAttr, gcnew array< ::System::Object^>(0));
                property_builder->SetCustomAttribute(attrBuilderBound);
            }

            // getter
            Emit::MethodBuilder ^ method_builder =
                type_builder->DefineMethod(
                    ustring_to_String( OUSTR("get_") +
                                       xAttribute->getMemberName() ),
                    c_property_method_attr, attribute_type, parameters );

           //define UNO exception attribute (exceptions)--------------------------------------
            Emit::CustomAttributeBuilder^ attrBuilder =
                get_exception_attribute(xAttribute->getGetExceptions());
            if (attrBuilder != nullptr)
                method_builder->SetCustomAttribute(attrBuilder);

            property_builder->SetGetMethod( method_builder );

            if (! xAttribute->isReadOnly())
            {
                // setter
                parameters = gcnew array< ::System::Type^> ( 1 );
                parameters[ 0 ] = attribute_type;
                method_builder =
                    type_builder->DefineMethod(
                        ustring_to_String( OUSTR("set_") +
                                           xAttribute->getMemberName() ),
                        c_property_method_attr, nullptr, parameters );
                // define parameter info
                method_builder->DefineParameter(
                    1 /* starts with 1 */, ParameterAttributes::In, "value" );
                //define UNO exception attribute (exceptions)--------------------------------------
                Emit::CustomAttributeBuilder^ attrBuilder =
                    get_exception_attribute(xAttribute->getSetExceptions());
                if (attrBuilder != nullptr)
                    method_builder->SetCustomAttribute(attrBuilder);

                property_builder->SetSetMethod( method_builder );
            }
        }
    }

    // remove from incomplete types map
    ::System::String ^ cts_name = type_builder->FullName;
    m_incomplete_ifaces->Remove( cts_name );
    xType->release();

    if (g_verbose)
    {
        ::System::Console::WriteLine(
            "> emitting interface type {0}", cts_name );
    }
    return type_builder->CreateType();
}

::System::Type ^ TypeEmitter::complete_struct_type( struct_entry ^ entry )
{
     OSL_ASSERT(entry);
    ::System::String ^ cts_name = entry->m_type_builder->FullName;

    //Polymorphic struct, define uno.TypeParametersAttribute
    //A polymorphic struct cannot have a basetype.
    //When we create the template of the struct then we have no exact types
    //and the name does not contain a parameter list
    Sequence< OUString > seq_type_parameters;
    Reference< reflection::XStructTypeDescription> xStructTypeDesc(
        entry->m_xType, UNO_QUERY);
    if (xStructTypeDesc.is())
    {
        seq_type_parameters = xStructTypeDesc->getTypeParameters();
        int numTypes = 0;
        if ((numTypes = seq_type_parameters.getLength()) > 0)
        {
            array< ::System::Object^>^ aArg = gcnew array< ::System::Object^>(numTypes);
            for (int i = 0; i < numTypes; i++)
                aArg[i] = ustring_to_String(seq_type_parameters.getConstArray()[i]);
            array< ::System::Object^>^ args = {aArg};

            array< ::System::Type^>^ arTypesCtor =
            {::System::Type::GetType("System.String[]")};
            Emit::CustomAttributeBuilder ^ attrBuilder =
                gcnew Emit::CustomAttributeBuilder(
                ::uno::TypeParametersAttribute::typeid->GetConstructor(arTypesCtor),
                args);
            entry->m_type_builder->SetCustomAttribute(attrBuilder);
        }
    }

    // optional: lookup base type whether generated entry of this session
    struct_entry ^ base_type_entry = nullptr;
    if (nullptr != entry->m_base_type)
    {
        //ToDo maybe get from incomplete structs
        base_type_entry =
            dynamic_cast< struct_entry ^ >(
                m_generated_structs[
                    entry->m_base_type->FullName ] );
    }

        // members
    Sequence< Reference< reflection::XTypeDescription > > seq_members(
        entry->m_xType->getMemberTypes() );
    Sequence< OUString > seq_member_names( entry->m_xType->getMemberNames() );
    sal_Int32 members_length = seq_members.getLength();
    OSL_ASSERT( seq_member_names.getLength() == members_length );
    //check if we have a XTypeDescription for every member. If not then the user may
    //have forgotten to specify additional rdbs with the --extra option.
    Reference< reflection::XTypeDescription > const * pseq_members =
            seq_members.getConstArray();
    OUString const * pseq_member_names =
        seq_member_names.getConstArray();
    for (int i = 0; i < members_length; i++)
    {
        const OUString sType(entry->m_xType->getName());
        const OUString sMemberName(pseq_member_names[i]);
        if ( ! pseq_members[i].is())
            throw RuntimeException(OUSTR("Missing type description . Check if you need to " \
            "specify additional RDBs with the --extra option. Type missing for: ") +  sType +
            OUSTR("::") + sMemberName,0);
    }

    sal_Int32 all_members_length = 0;
    sal_Int32 member_pos;
    sal_Int32 type_param_pos = 0;

    // collect base types; wrong order
    ::System::Collections::ArrayList ^ base_types_list =
            gcnew ::System::Collections::ArrayList( 3 /* initial capacity */ );
    for (::System::Type ^ base_type_pos = entry->m_base_type;
        ! base_type_pos->Equals( ::System::Object::typeid );
        base_type_pos = base_type_pos->BaseType )
    {
        base_types_list->Add( base_type_pos );
        if (base_type_pos->Equals( ::System::Exception::typeid ))
        {
            // special Message member
            all_members_length += 1;
            break; // don't include System.Exception base classes
        }
        else
        {
            //ensure the base type is complete. Otherwise GetFields won't work
            get_complete_struct(base_type_pos->FullName);
            all_members_length +=
                base_type_pos->GetFields(
                (BindingFlags) (BindingFlags::Instance |
                BindingFlags::Public |
                BindingFlags::DeclaredOnly) )
                ->Length;
        }
    }

    // create all_members arrays; right order
    array< ::System::String^>^ all_member_names =
        gcnew array< ::System::String^> (all_members_length + members_length );
    array< ::System::Type^>^ all_param_types =
        gcnew array< ::System::Type^> (all_members_length + members_length );
    member_pos = 0;
    for ( sal_Int32 pos = base_types_list->Count; pos--; )
    {
        ::System::Type ^ base_type = safe_cast< ::System::Type ^ >(
            base_types_list[pos] );
        if (base_type->Equals( ::System::Exception::typeid ))
        {
            all_member_names[ member_pos ] = "Message";
            all_param_types[ member_pos ] = ::System::String::typeid;
            ++member_pos;
        }
        else
        {
            ::System::String ^ base_type_name = base_type->FullName;

            //ToDo m_generated_structs?
            struct_entry ^ entry =
                dynamic_cast< struct_entry ^ >(
                m_generated_structs[base_type_name] );
            if (nullptr == entry)
            {
                // complete type
                array<FieldInfo^>^ fields =
                    base_type->GetFields(
                    (BindingFlags) (BindingFlags::Instance |
                    BindingFlags::Public |
                    BindingFlags::DeclaredOnly) );
                sal_Int32 len = fields->Length;
                for ( sal_Int32 pos = 0; pos < len; ++pos )
                {
                    FieldInfo ^ field = fields[ pos ];
                    all_member_names[ member_pos ] = field->Name;
                    all_param_types[ member_pos ] = field->FieldType;
                    ++member_pos;
                }
            }
            else // generated during this session:
                // members may be incomplete ifaces
            {
                sal_Int32 len = entry->m_member_names->Length;
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
//    struct_entry * entry = new struct_entry();
    entry->m_member_names = gcnew array< ::System::String^> ( members_length );
    entry->m_param_types = gcnew array< ::System::Type^> ( members_length );

    // add members
    array<Emit::FieldBuilder^>^ members = gcnew array<Emit::FieldBuilder^> ( members_length );
    //Reference< reflection::XTypeDescription > const * pseq_members =
    //    seq_members.getConstArray();
    //OUString const * pseq_member_names =
    //    seq_member_names.getConstArray();

    int curParamIndex = 0; //count the fields which have parameterized types
    for ( member_pos = 0; member_pos < members_length; ++member_pos )
    {
        ::System::String ^ field_name =
            ustring_to_String( pseq_member_names[ member_pos ] );
        ::System::Type ^ field_type;
        //Special handling of struct parameter types
        bool bParameterizedType = false;
        if (pseq_members[ member_pos ]->getTypeClass() == TypeClass_UNKNOWN)
        {
            bParameterizedType = true;
            if (type_param_pos < seq_type_parameters.getLength())
            {
                field_type = ::System::Object::typeid;
                type_param_pos++;
            }
            else
            {
                throw RuntimeException(
                    OUSTR("unexpected member type in ") + entry->m_xType->getName(),
                    Reference< XInterface >() );
            }
        }
        else
        {
            field_type =
                get_type( pseq_members[ member_pos ] );
        }
        members[ member_pos ] =
            entry->m_type_builder->DefineField(
            field_name, field_type, FieldAttributes::Public );

        //parameterized type (polymorphic struct) ?
        if (bParameterizedType && xStructTypeDesc.is())
        {
            //get the name
            OSL_ASSERT(seq_type_parameters.getLength() > curParamIndex);
            ::System::String^ sTypeName = ustring_to_String(
                seq_type_parameters.getConstArray()[curParamIndex++]);
            array< ::System::Object^>^ args = {sTypeName};
            //set ParameterizedTypeAttribute
            array< ::System::Type^>^ arCtorTypes = {::System::String::typeid};

            Emit::CustomAttributeBuilder ^ attrBuilder =
                gcnew Emit::CustomAttributeBuilder(
                ::uno::ParameterizedTypeAttribute::typeid
                ->GetConstructor(arCtorTypes),
                args);

            members[member_pos]->SetCustomAttribute(attrBuilder);
        }
        // add to all_members
        all_member_names[ all_members_length + member_pos ] = field_name;
        all_param_types[ all_members_length + member_pos ] = field_type;
        // add to entry
        entry->m_member_names[ member_pos ] = field_name;
        entry->m_param_types[ member_pos ] = field_type;
    }
    all_members_length += members_length;

    // default .ctor
    Emit::ConstructorBuilder ^ ctor_builder =
        entry->m_type_builder->DefineConstructor(
        c_ctor_method_attr, CallingConventions::Standard,
        gcnew array< ::System::Type^> ( 0 ) );
    Emit::ILGenerator ^ code = ctor_builder->GetILGenerator();
    code->Emit( Emit::OpCodes::Ldarg_0 );
    code->Emit(
        Emit::OpCodes::Call,
        nullptr == base_type_entry
        ? entry->m_base_type->GetConstructor( gcnew array< ::System::Type^> ( 0 ) )
        : base_type_entry->m_default_ctor );
    // default initialize members
    for ( member_pos = 0; member_pos < members_length; ++member_pos )
    {
        FieldInfo ^ field = members[ member_pos ];
        ::System::Type ^ field_type = field->FieldType;
        //            ::System::Type * new_field_type = m_module_builder->GetType(field_type->FullName, false);
        // default initialize:
        // string, type, enum, sequence, struct, exception, any
        if (field_type->Equals( ::System::String::typeid ))
        {
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldstr, "" );
            code->Emit( Emit::OpCodes::Stfld, field );
        }
        else if (field_type->Equals( ::System::Type::typeid ))
        {
            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit(
                Emit::OpCodes::Ldtoken, ::System::Void::typeid );
            code->Emit(
                Emit::OpCodes::Call, m_method_info_Type_GetTypeFromHandle );
            code->Emit( Emit::OpCodes::Stfld, field );
        }
        else if (field_type->IsArray)
        {
            //Find the value type. In case of sequence<sequence< ... > > find the actual value type
            ::System::Type ^ value = field_type;
            while ((value = value->GetElementType())->IsArray);
            //If the value type is a struct then make sure it is fully created.
            get_complete_struct(value->FullName);

            code->Emit( Emit::OpCodes::Ldarg_0 );
            code->Emit( Emit::OpCodes::Ldc_I4_0 );
            code->Emit(
                Emit::OpCodes::Newarr, field_type->GetElementType() );
            code->Emit( Emit::OpCodes::Stfld, field );
        }
        else if (field_type->IsValueType)
        {
            if (field_type->FullName->Equals( "uno.Any" ))
            {
                code->Emit( Emit::OpCodes::Ldarg_0 );
                code->Emit( Emit::OpCodes::Ldsfld, ::uno::Any::typeid->GetField("VOID"));
                code->Emit( Emit::OpCodes::Stfld, field );
            }
        }
        else if (field_type->IsClass)
        {
            /* may be XInterface */
            if (! field_type->Equals( ::System::Object::typeid ))
            {
                // struct, exception
                //make sure the struct is already complete.
                get_complete_struct(field_type->FullName);
                code->Emit( Emit::OpCodes::Ldarg_0 );
                code->Emit(
                    Emit::OpCodes::Newobj,
                    //GetConstructor requies that the member types of the object which is to be constructed are already known.
                    field_type->GetConstructor(
                    gcnew array< ::System::Type^> ( 0 ) ) );
                code->Emit( Emit::OpCodes::Stfld, field );
            }
        }
    }
    code->Emit( Emit::OpCodes::Ret );
    entry->m_default_ctor = ctor_builder;

    // parameterized .ctor including all base members
    ctor_builder = entry->m_type_builder->DefineConstructor(
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
    array< ::System::Type^>^ param_types  =
        gcnew array< ::System::Type^> ( base_members_length );
    for ( member_pos = 0; member_pos < base_members_length; ++member_pos )
    {
        emit_ldarg( code, member_pos +1 );
        param_types[ member_pos ] = all_param_types[ member_pos ];
    }
    code->Emit(
        Emit::OpCodes::Call,
        nullptr == base_type_entry
        ? entry->m_base_type->GetConstructor( param_types )
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
            TypeClass_STRUCT == entry->m_xType->getTypeClass()
            ? "struct"
            : "exception",
            cts_name);
    }
    // new entry
    m_generated_structs->Add(cts_name, entry );
    ::System::Type ^ ret_type = entry->m_type_builder->CreateType();

    // remove from incomplete types map
    m_incomplete_structs->Remove( cts_name );
    entry->m_xType->release();

    if (g_verbose)
    {
        ::System::Console::WriteLine(
            "> emitting struct type {0}", cts_name);
    }
    return ret_type;
}

//Examples of generated code
//      public static XWeak constructor1(XComponentContext ctx)
//      {
//          XMultiComponentFactory factory = ctx.getServiceManager();
//          if (factory == null)
//              throw new com.sun.star.uno.DeploymentException("bla", null);
//          return (XWeak) factory.createInstanceWithContext("service_specifier", ctx);
//      }
//      public static XWeak constructor2(XComponentContext ctx, int a, int b, Any c)
//      {
//          XMultiComponentFactory factory = ctx.getServiceManager();
//          if (factory == null)
//              throw new com.sun.star.uno.DeploymentException("bla", null);
//          Any[] arAny = new Any[3];
//          arAny[0] = new Any(typeof(int), a);
//          arAny[1] = new Any(typeof(int), b);
//          arAny[2] = new Any(c.Type, c.Value);
//          return (XWeak) factory.createInstanceWithArgumentsAndContext("service_specifier", arAny, ctx);
//      }
// Notice that a any parameter is NOT wrapped by another any. Instead the new any is created with the type and value
// of the parameter.

//      public static XWeak constructor3(XComponentContext ctx, params Any[] c)
//      {
//          XMultiComponentFactory factory = ctx.getServiceManager();
//          if (factory == null)
//              throw new com.sun.star.uno.DeploymentException("bla", null);
//          return (XWeak) factory.createInstanceWithArgumentsAndContext("service_specifier", c, ctx);
//      }
::System::Type ^ TypeEmitter::complete_service_type(service_entry ^ entry)
{
    Emit::TypeBuilder ^ type_builder = entry->m_type_builder;
    reflection::XServiceTypeDescription2 * xServiceType = entry->m_xType;

    //Create the private default constructor
    Emit::ConstructorBuilder^ ctor_builder =
        type_builder->DefineConstructor(
            (MethodAttributes) (MethodAttributes::Private |
                                MethodAttributes::HideBySig |
                                MethodAttributes::SpecialName |
                                MethodAttributes::RTSpecialName),
            CallingConventions::Standard, nullptr);

    Emit::ILGenerator^ ilGen = ctor_builder->GetILGenerator();
    ilGen->Emit( Emit::OpCodes::Ldarg_0 ); // push this
    ilGen->Emit(
            Emit::OpCodes::Call,
            type_builder->BaseType->GetConstructor(gcnew array< ::System::Type^>(0)));
    ilGen->Emit( Emit::OpCodes::Ret );


    //Create the service constructors.
    //obtain the interface which makes up this service, it is the return
    //type of the constructor functions
    Reference<reflection::XInterfaceTypeDescription2> xIfaceType(
        xServiceType->getInterface(), UNO_QUERY);
    if (xIfaceType.is () == sal_False)
        xIfaceType = resolveInterfaceTypedef(xServiceType->getInterface());
    System::Type ^ retType = get_type(xIfaceType);

    //Create the ConstructorInfo for a DeploymentException
    ::System::Type ^ typeDeploymentExc =
          get_type("unoidl.com.sun.star.uno.DeploymentException", true);

    array< ::System::Type^>^ arTypeCtor = {::System::String::typeid,
                                         ::System::Object::typeid};
    ::System::Reflection::ConstructorInfo ^ ctorDeploymentException  =
          typeDeploymentExc->GetConstructor(arTypeCtor);

    Sequence<Reference<reflection::XServiceConstructorDescription> >  seqCtors =
        xServiceType->getConstructors();

    ::System::Type ^ type_uno_exception = get_type("unoidl.com.sun.star.uno.Exception", true);

    for (int i = seqCtors.getLength() - 1; i >= 0; i--)
    {
        bool bParameterArray = false;
        ::System::Type ^ typeAny = ::uno::Any::typeid;
        const Reference<reflection::XServiceConstructorDescription> & ctorDes =
            seqCtors[i];
        //obtain the parameter types
        Sequence<Reference<reflection::XParameter> > seqParams =
            ctorDes->getParameters();
        Reference<reflection::XParameter> const * arXParams = seqParams.getConstArray();
        sal_Int32 cParams = seqParams.getLength();
        array< ::System::Type^>^ arTypeParameters = gcnew array< ::System::Type^> (cParams + 1);
        arTypeParameters[0] = get_type("unoidl.com.sun.star.uno.XComponentContext", true);
        for (int iparam = 0; iparam != cParams; iparam++)
        {
            if (arXParams[iparam]->isRestParameter())
                arTypeParameters[iparam + 1] = array< ::uno::Any>::typeid;
            else
                arTypeParameters[iparam + 1] = get_type(arXParams[iparam]->getType());
        }
        //The array arTypeParameters can contain:
        //System.Type and uno.PolymorphicType.
        //Passing PolymorphicType to MethodBuilder.DefineMethod will cause a problem.
        //The exception will read something like no on information for parameter # d
        //Maybe we need no override another Type method in PolymorphicType ...
        //Until we have figured this out, we will create another array of System.Type which
        //we pass on to DefineMethod.
        array< ::System::Type^>^ arParamTypes = gcnew array< ::System::Type^> (cParams + 1);
//        arParamTypes[0] = get_type("unoidl.com.sun.star.uno.XComponentContext", true);
        for (int i = 0; i < cParams + 1; i++)
        {
            ::uno::PolymorphicType ^ pT = dynamic_cast< ::uno::PolymorphicType ^ >(arTypeParameters[i]);
            if (pT)
                arParamTypes[i] = pT->OriginalType;
            else
                arParamTypes[i] = arTypeParameters[i];
        }
        //define method
        System::String ^ ctorName;
        if (ctorDes->isDefaultConstructor())
            ctorName = gcnew ::System::String("create");
        else
            ctorName = ustring_to_String(ctorDes->getName());
        Emit::MethodBuilder^ method_builder = type_builder->DefineMethod(
            ctorName,
            static_cast<MethodAttributes>(MethodAttributes::Public | MethodAttributes::HideBySig |
                                          MethodAttributes::Static),
            retType,
//            arTypeParameters);
            arParamTypes);

        //define UNO exception attribute (exceptions)--------------------------------------
        Emit::CustomAttributeBuilder^ attrBuilder = get_service_exception_attribute(ctorDes);
        if (attrBuilder != nullptr)
            method_builder->SetCustomAttribute(attrBuilder);

        //-------------------------------------------------------------
        //define parameter attributes (paramarray), names etc.
        //The first parameter is the XComponentContext, which cannot be obtained
        //from reflection.
        //The context is not part of the idl description
        method_builder->DefineParameter(
            1, ParameterAttributes::In, "the_context");

        array<Emit::ParameterBuilder^>^ arParameterBuilder =
            gcnew array<Emit::ParameterBuilder^> (cParams);
        for (int iparam = 0; iparam != cParams; iparam++)
        {
            Reference<reflection::XParameter> const & aParam = arXParams[iparam];
            ::System::String ^ sParamName = ustring_to_String(aParam->getName());

            arParameterBuilder[iparam] = method_builder->DefineParameter(
                iparam + 2, ParameterAttributes::In, sParamName);

            if (aParam->isRestParameter())
            {
                bParameterArray = true;
                //set the ParameterArrayAttribute
                ::System::Reflection::ConstructorInfo^ ctor_info =
                    System::ParamArrayAttribute::typeid->GetConstructor(
                        gcnew array< ::System::Type^>(0));
                Emit::CustomAttributeBuilder ^ attr_builder =
                    gcnew Emit::CustomAttributeBuilder(ctor_info, gcnew array< ::System::Object^>(0));
                arParameterBuilder[iparam]->SetCustomAttribute(attr_builder);
                break;
            }
        }

        Emit::ILGenerator ^ ilGen = method_builder->GetILGenerator();

        //Define locals ---------------------------------
        //XMultiComponentFactory
        Emit::LocalBuilder^ local_factory =
            ilGen->DeclareLocal(
                get_type("unoidl.com.sun.star.lang.XMultiComponentFactory", true));

        //The return type
        Emit::LocalBuilder^ local_return_val =
            ilGen->DeclareLocal(retType);

        //Obtain the XMultiComponentFactory and throw an exception if we do not get one
        ilGen->Emit(Emit::OpCodes::Ldarg_0);

        ::System::Reflection::MethodInfo ^ methodGetServiceManager = get_type(
            "unoidl.com.sun.star.uno.XComponentContext", true)
                ->GetMethod("getServiceManager");
        ilGen->Emit(Emit::OpCodes::Callvirt, methodGetServiceManager);
        ilGen->Emit(Emit::OpCodes::Stloc, local_factory);
        ilGen->Emit(Emit::OpCodes::Ldloc, local_factory);
        Emit::Label label1 = ilGen->DefineLabel();
        ilGen->Emit(Emit::OpCodes::Brtrue, label1);
        //The string for the exception
        ::System::Text::StringBuilder ^ strbuilder = gcnew ::System::Text::StringBuilder(256);
        strbuilder->Append("The service ");
        strbuilder->Append(to_cts_name(xServiceType->getName()));
        strbuilder->Append(" could not be created. The context failed to supply the service manager.");

        ilGen->Emit(Emit::OpCodes::Ldstr, strbuilder->ToString());
        ilGen->Emit(Emit::OpCodes::Ldarg_0);
        ilGen->Emit(Emit::OpCodes::Newobj, ctorDeploymentException);
        ilGen->Emit(Emit::OpCodes::Throw);
        ilGen->MarkLabel(label1);

        //We create a try/ catch around the createInstanceWithContext, etc. functions
        //There are 3 cases
        //1. function do not specify exceptions. Then RuntimeExceptions are retrhown and other
        //   exceptions produce a DeploymentException.
        //2. function specify  Exception. Then all exceptions fly through
        //3. function specifies exceptions but no Exception. Then these are rethrown
        //   and other exceptions, except RuntimeException, produce a deployment exception.
        //In case there are no parameters we call
        //XMultiComponentFactory.createInstanceWithContext

        ::System::Collections::ArrayList ^ arExceptionTypes =
              get_service_ctor_method_exceptions_reduced(ctorDes->getExceptions());
        if (arExceptionTypes->Contains(
                type_uno_exception) == false)
        {
            ilGen->BeginExceptionBlock();
        }
        if (cParams == 0)
        {
            ilGen->Emit(Emit::OpCodes::Ldloc, local_factory);
            ilGen->Emit(Emit::OpCodes::Ldstr, ustring_to_String(xServiceType->getName()));
            ilGen->Emit(Emit::OpCodes::Ldarg_0);

            ::System::Reflection::MethodInfo ^ methodCreate =
                    local_factory->LocalType->GetMethod("createInstanceWithContext");
            ilGen->Emit(Emit::OpCodes::Callvirt, methodCreate);
        }
        else if(bParameterArray)
        {
            //Service constructor with parameter array
            ilGen->Emit(Emit::OpCodes::Ldloc, local_factory);
            ilGen->Emit(Emit::OpCodes::Ldstr, ustring_to_String(xServiceType->getName()));
            ilGen->Emit(Emit::OpCodes::Ldarg_1);
            ilGen->Emit(Emit::OpCodes::Ldarg_0);
            ::System::Reflection::MethodInfo ^ methodCreate =
                    local_factory->LocalType->GetMethod("createInstanceWithArgumentsAndContext");
            ilGen->Emit(Emit::OpCodes::Callvirt, methodCreate);
        }
        else
        {
            // Any param1, Any param2, etc.
            // For each parameter,except the component context, and parameter array
            // and Any is created.
            array<Emit::LocalBuilder^>^ arLocalAny = gcnew array<Emit::LocalBuilder^> (cParams);

            for (int iParam = 0; iParam < cParams; iParam ++)
            {
                arLocalAny[iParam] = ilGen->DeclareLocal(typeAny);
            }

            //Any[]. This array is filled with the created Anys which contain the parameters
            //and the values contained in the parameter array
            Emit::LocalBuilder ^ local_anyParams =
                ilGen->DeclareLocal(array< ::uno::Any>::typeid);

            //Create the Any for every argument, except for the parameter array
            //arLocalAny contains the LocalBuilder for all these parameters.
            //we call the ctor Any(Type, Object)
            //If the parameter is an Any then the Any is created with Any(param.Type, param.Value);
            array< ::System::Type^>^ arTypesCtorAny = {::System::Type::typeid,
                                                    ::System::Object::typeid};
            ::System::Reflection::ConstructorInfo ^ ctorAny =
                typeAny->GetConstructor( arTypesCtorAny);
            ::System::Reflection::MethodInfo ^ methodAnyGetType =
                typeAny->GetProperty("Type")->GetGetMethod();
            ::System::Reflection::MethodInfo ^ methodAnyGetValue =
                typeAny->GetProperty("Value")->GetGetMethod();
            for (int i = 0; i < arLocalAny->Length; i ++)
            {
                //check if the parameter is a polymorphic struct
                ::uno::PolymorphicType ^polyType = dynamic_cast< ::uno::PolymorphicType^ >(arTypeParameters[i+1]);
                //arTypeParameters[i+1] = polyType->OriginalType;
                if (polyType)
                {
                    //It is a polymorphic struct
                    //Load the uninitialized local Any on which we will call the ctor
                    ilGen->Emit(Emit::OpCodes::Ldloca, arLocalAny[i]);
                    // Call PolymorphicType PolymorphicType::GetType(Type t, String polyName)
                    // Prepare the first parameter
                    ilGen->Emit(Emit::OpCodes::Ldtoken, polyType->OriginalType);
                    array< ::System::Type^>^ arTypeParams = {::System::RuntimeTypeHandle::typeid};
                    ilGen->Emit(Emit::OpCodes::Call,
                                ::System::Type::typeid->GetMethod(
                                    "GetTypeFromHandle", arTypeParams));
                    // Prepare the second parameter
                    ilGen->Emit(Emit::OpCodes::Ldstr, polyType->PolymorphicName);
                    // Make the actual call
                    array< ::System::Type^>^ arTypeParam_GetType = {
                        ::System::Type::typeid, ::System::String::typeid };
                    ilGen->Emit(Emit::OpCodes::Call,
                    ::uno::PolymorphicType::typeid->GetMethod(gcnew System::String("GetType"),
                        arTypeParam_GetType));

                    //Stack is: localAny, PolymorphicType
                    //Call Any::Any(Type, Object)
                    //Prepare the second parameter for the any ctor
                    ilGen->Emit(Emit::OpCodes::Ldarg, i + 1);
                    // if the parameter is a value type then we need to box it, because
                    // the Any ctor takes an Object
                    if (arTypeParameters[i+1]->IsValueType)
                        ilGen->Emit(Emit::OpCodes::Box, arTypeParameters[i+1]);
                    ilGen->Emit(Emit::OpCodes::Call, ctorAny);
                }
                else if (arTypeParameters[i+1] == typeAny)
                {
                    //Create the call new Any(param.Type,param,Value)
                    //Stack must be Any,Type,Value
                    //First load the Any which is to be constructed
                    ilGen->Emit(Emit::OpCodes::Ldloca, arLocalAny[i]);
                    //Load the Type, which is obtained by calling param.Type
                    ilGen->Emit(Emit::OpCodes::Ldarga, i + 1);
                    ilGen->Emit(Emit::OpCodes::Call, methodAnyGetType);
                    //Load the Value, which is obtained by calling param.Value
                    ilGen->Emit(Emit::OpCodes::Ldarga, i + 1);
                    ilGen->Emit(Emit::OpCodes::Call, methodAnyGetValue);
                    //Call the Any ctor.
                    ilGen->Emit(Emit::OpCodes::Call, ctorAny);
                }
                else
                {
                    ilGen->Emit(Emit::OpCodes::Ldloca, arLocalAny[i]);
                    ilGen->Emit(Emit::OpCodes::Ldtoken, arTypeParameters[i+1]);

                    array< ::System::Type^>^ arTypeParams = {::System::RuntimeTypeHandle::typeid};
                    ilGen->Emit(Emit::OpCodes::Call,
                                ::System::Type::typeid->GetMethod(
                                    "GetTypeFromHandle", arTypeParams));
                    ilGen->Emit(Emit::OpCodes::Ldarg, i + 1);
                    // if the parameter is a value type then we need to box it, because
                    // the Any ctor takes an Object
                    if (arTypeParameters[i+1]->IsValueType)
                        ilGen->Emit(Emit::OpCodes::Box, arTypeParameters[i+1]);
                    ilGen->Emit(Emit::OpCodes::Call, ctorAny);
                }
            }

            //Create the Any[] that is passed to the
            //createInstanceWithContext[AndArguments] function
            ilGen->Emit(Emit::OpCodes::Ldc_I4, arLocalAny->Length);
            ilGen->Emit(Emit::OpCodes::Newarr, typeAny);
            ilGen->Emit(Emit::OpCodes::Stloc, local_anyParams);

            //Assign all anys created from the parameters
            //array to the Any[]
            for (int i = 0; i < arLocalAny->Length; i++)
            {
                ilGen->Emit(Emit::OpCodes::Ldloc, local_anyParams);
                ilGen->Emit(Emit::OpCodes::Ldc_I4, i);
                ilGen->Emit(Emit::OpCodes::Ldelema, typeAny);
                ilGen->Emit(Emit::OpCodes::Ldloc, arLocalAny[i]);
                ilGen->Emit(Emit::OpCodes::Stobj, typeAny);
            }
            // call createInstanceWithContextAndArguments
            ilGen->Emit(Emit::OpCodes::Ldloc, local_factory);
            ilGen->Emit(Emit::OpCodes::Ldstr, ustring_to_String(xServiceType->getName()));
            ilGen->Emit(Emit::OpCodes::Ldloc, local_anyParams);
            ilGen->Emit(Emit::OpCodes::Ldarg_0);
            ::System::Reflection::MethodInfo ^ methodCreate =
                    local_factory->LocalType->GetMethod("createInstanceWithArgumentsAndContext");
            ilGen->Emit(Emit::OpCodes::Callvirt, methodCreate);
        }
        //cast the object returned by the functions createInstanceWithContext or
        //createInstanceWithArgumentsAndContext to the interface type
        ilGen->Emit(Emit::OpCodes::Castclass, retType);
        ilGen->Emit(Emit::OpCodes::Stloc, local_return_val);

        //catch exceptions thrown by createInstanceWithArgumentsAndContext and createInstanceWithContext
        if (arExceptionTypes->Contains(type_uno_exception) == false)
        {
            // catch (unoidl.com.sun.star.uno.RuntimeException) {throw;}
            ilGen->BeginCatchBlock(get_type("unoidl.com.sun.star.uno.RuntimeException", true));
            ilGen->Emit(Emit::OpCodes::Pop);
            ilGen->Emit(Emit::OpCodes::Rethrow);

            //catch and rethrow all other defined Exceptions
            for (int i = 0; i < arExceptionTypes->Count; i++)
            {
                ::System::Type ^ excType = safe_cast< ::System::Type^ >(
                    arExceptionTypes[i]);
                if (excType->IsInstanceOfType(
                        get_type("unoidl.com.sun.star.uno.RuntimeException", true)))
                {// we have a catch for RuntimeException already defined
                    continue;
                }

                //catch Exception and rethrow
                ilGen->BeginCatchBlock(excType);
                ilGen->Emit(Emit::OpCodes::Pop);
                ilGen->Emit(Emit::OpCodes::Rethrow);
            }
            //catch (unoidl.com.sun.star.uno.Exception) {throw DeploymentException...}
            ilGen->BeginCatchBlock(type_uno_exception);

            //Define the local variabe that keeps the exception
             Emit::LocalBuilder ^ local_exception = ilGen->DeclareLocal(
                 type_uno_exception);

             //Store the exception
             ilGen->Emit(Emit::OpCodes::Stloc, local_exception);

            //prepare the construction of the exception
             strbuilder = gcnew ::System::Text::StringBuilder(256);
             strbuilder->Append("The context (com.sun.star.uno.XComponentContext) failed to supply the service ");
             strbuilder->Append(to_cts_name(xServiceType->getName()));
             strbuilder->Append(": ");

             ilGen->Emit(Emit::OpCodes::Ldstr, strbuilder->ToString());

            //add to the string the Exception.Message
            ilGen->Emit(Emit::OpCodes::Ldloc, local_exception);
            ilGen->Emit(Emit::OpCodes::Callvirt,
                        type_uno_exception->GetProperty("Message")->GetGetMethod());
            array< ::System::Type^>^ arConcatParams = {System::String::typeid,
                                                  System::String::typeid};
            ilGen->Emit(Emit::OpCodes::Call,
                        System::String::typeid->GetMethod("Concat", arConcatParams));
            //load contex argument
            ilGen->Emit(Emit::OpCodes::Ldarg_0);
            ilGen->Emit(Emit::OpCodes::Newobj, ctorDeploymentException);
            ilGen->Emit(Emit::OpCodes::Throw);//Exception(typeDeploymentExc);

            ilGen->EndExceptionBlock();
        }


        //Check if the service instance was create and throw a exception if not.
        Emit::Label label_service_created = ilGen->DefineLabel();
        ilGen->Emit(Emit::OpCodes::Ldloc, local_return_val);
        ilGen->Emit(Emit::OpCodes::Brtrue_S, label_service_created);

        strbuilder = gcnew ::System::Text::StringBuilder(256);
        strbuilder->Append("The context (com.sun.star.uno.XComponentContext) failed to supply the service ");
        strbuilder->Append(to_cts_name(xServiceType->getName()));
        strbuilder->Append(".");
        ilGen->Emit(Emit::OpCodes::Ldstr, strbuilder->ToString());
        ilGen->Emit(Emit::OpCodes::Ldarg_0);
        ilGen->Emit(Emit::OpCodes::Newobj, ctorDeploymentException);
        ilGen->Emit(Emit::OpCodes::Throw);//Exception(typeDeploymentExc);

        ilGen->MarkLabel(label_service_created);
        ilGen->Emit(Emit::OpCodes::Ldloc, local_return_val);
        ilGen->Emit(Emit::OpCodes::Ret);

    }
    // remove from incomplete types map
    ::System::String ^ cts_name = type_builder->FullName;
    m_incomplete_services->Remove( cts_name );
    xServiceType->release();
    if (g_verbose)
    {
        ::System::Console::WriteLine(
            "> emitting service type {0}", cts_name );
    }
    return type_builder->CreateType();
}


Emit::CustomAttributeBuilder^ TypeEmitter::get_service_exception_attribute(
    const Reference<reflection::XServiceConstructorDescription> & ctorDes  )
{
    return get_exception_attribute(ctorDes->getExceptions());
}

Emit::CustomAttributeBuilder^ TypeEmitter::get_iface_method_exception_attribute(
    const Reference< reflection::XInterfaceMethodTypeDescription >& xMethod )
{

    const Sequence<Reference<reflection::XTypeDescription> > seqTD = xMethod->getExceptions();
    int len = seqTD.getLength();
    Sequence<Reference<reflection::XCompoundTypeDescription> > seqCTD(len);
    Reference<reflection::XCompoundTypeDescription> * arCTD = seqCTD.getArray();
    for (int i = 0; i < len; i++)
        arCTD[i] = Reference<reflection::XCompoundTypeDescription>(seqTD[i], UNO_QUERY_THROW);
    return get_exception_attribute(seqCTD);
}

Emit::CustomAttributeBuilder^ TypeEmitter::get_exception_attribute(

    const Sequence<Reference< reflection::XCompoundTypeDescription > >& seq_exceptionsTd )
{
    Emit::CustomAttributeBuilder ^ attr_builder = nullptr;

    Reference< reflection::XCompoundTypeDescription > const * exceptions =
        seq_exceptionsTd.getConstArray();

    array< ::System::Type^>^ arTypesCtor = {::System::Type::GetType("System.Type[]")};
    ConstructorInfo ^ ctor_ExceptionAttribute =
        ::uno::ExceptionAttribute::typeid->GetConstructor(arTypesCtor);

    sal_Int32 exc_length = seq_exceptionsTd.getLength();
    if (exc_length != 0) // opt
    {
        array< ::System::Type^>^ exception_types =
              gcnew array< ::System::Type^> ( exc_length );
        for ( sal_Int32 exc_pos = 0; exc_pos < exc_length; ++exc_pos )
        {
            Reference< reflection::XCompoundTypeDescription > const & xExc =
                exceptions[ exc_pos ];
            exception_types[ exc_pos ] = get_type( xExc );
        }
        array< ::System::Object^>^ args = {exception_types};
        attr_builder = gcnew Emit::CustomAttributeBuilder(
            ctor_ExceptionAttribute, args );
    }
    return attr_builder;
}


::System::Type ^ TypeEmitter::complete_singleton_type(singleton_entry ^ entry)
{
    Emit::TypeBuilder ^ type_builder = entry->m_type_builder;
    reflection::XSingletonTypeDescription2 * xSingletonType = entry->m_xType;
    ::System::String^ sSingletonName = to_cts_name(xSingletonType->getName());

    //Create the private default constructor
    Emit::ConstructorBuilder^ ctor_builder =
        type_builder->DefineConstructor(
            static_cast<MethodAttributes>(MethodAttributes::Private |
                                          MethodAttributes::HideBySig |
                                          MethodAttributes::SpecialName |
                                          MethodAttributes::RTSpecialName),
            CallingConventions::Standard, nullptr);

    Emit::ILGenerator^ ilGen = ctor_builder->GetILGenerator();
    ilGen->Emit( Emit::OpCodes::Ldarg_0 ); // push this
    ilGen->Emit(
            Emit::OpCodes::Call,
            type_builder->BaseType->GetConstructor(gcnew array< ::System::Type^>(0)));
    ilGen->Emit( Emit::OpCodes::Ret );


    //obtain the interface which makes up this service, it is the return
    //type of the constructor functions
    Reference<reflection::XInterfaceTypeDescription2> xIfaceType(
        xSingletonType->getInterface(), UNO_QUERY);
    if (xIfaceType.is () == sal_False)
        xIfaceType = resolveInterfaceTypedef(xSingletonType->getInterface());
    System::Type ^ retType = get_type(xIfaceType);

    //define method
    array< ::System::Type^>^ arTypeParameters = {get_type("unoidl.com.sun.star.uno.XComponentContext", true)};
    Emit::MethodBuilder^ method_builder = type_builder->DefineMethod(
        gcnew System::String("get"),
        static_cast<MethodAttributes>(MethodAttributes::Public | MethodAttributes::HideBySig |
                                      MethodAttributes::Static),
        retType,
        arTypeParameters);


//         method_builder->SetCustomAttribute(get_service_ctor_method_attribute(ctorDes));

    //The first parameter is the XComponentContext, which cannot be obtained
    //from reflection.
    //The context is not part of the idl description
    method_builder->DefineParameter(1, ParameterAttributes::In, "the_context");


    ilGen = method_builder->GetILGenerator();
    //Define locals ---------------------------------
    // Any, returned by XComponentContext.getValueByName
    Emit::LocalBuilder^ local_any =
        ilGen->DeclareLocal(::uno::Any::typeid);

    //Call XContext::getValueByName
    ilGen->Emit(Emit::OpCodes::Ldarg_0);
    // build the singleton name : /singleton/unoidl.com.sun.star.XXX
    ::System::Text::StringBuilder^ sBuilder =
          gcnew ::System::Text::StringBuilder("/singletons/");
    sBuilder->Append(sSingletonName);
    ilGen->Emit(Emit::OpCodes::Ldstr, sBuilder->ToString());

    ::System::Reflection::MethodInfo ^ methodGetValueByName =
          get_type("unoidl.com.sun.star.uno.XComponentContext", true)->GetMethod("getValueByName");
    ilGen->Emit(Emit::OpCodes::Callvirt, methodGetValueByName);
    ilGen->Emit(Emit::OpCodes::Stloc_0);

    //Contains the returned Any a value?
    ilGen->Emit(Emit::OpCodes::Ldloca_S, local_any);
    ::System::Reflection::MethodInfo ^ methodHasValue =
          ::uno::Any::typeid->GetMethod("hasValue");
    ilGen->Emit(Emit::OpCodes::Call, methodHasValue);

    //If not, then throw an DeploymentException
    Emit::Label label_singleton_exists = ilGen->DefineLabel();
    ilGen->Emit(Emit::OpCodes::Brtrue_S, label_singleton_exists);
    sBuilder = gcnew ::System::Text::StringBuilder(
        "Component context fails to supply singleton ");
    sBuilder->Append(sSingletonName);
    sBuilder->Append(" of type ");
    sBuilder->Append(retType->FullName);
    sBuilder->Append(".");
    ilGen->Emit(Emit::OpCodes::Ldstr, sBuilder->ToString());
    ilGen->Emit(Emit::OpCodes::Ldarg_0);
    array< ::System::Type^>^ arTypesCtorDeploymentException = {
        ::System::String::typeid, ::System::Object::typeid};
    ilGen->Emit(Emit::OpCodes::Newobj,
                get_type("unoidl.com.sun.star.uno.DeploymentException",true)
                ->GetConstructor(arTypesCtorDeploymentException));
    ilGen->Emit(Emit::OpCodes::Throw);
    ilGen->MarkLabel(label_singleton_exists);

    //Cast the singleton contained in the Any to the expected interface and return it.
    ilGen->Emit(Emit::OpCodes::Ldloca_S, local_any);
    ilGen->Emit(Emit::OpCodes::Call,  ::uno::Any::typeid->GetProperty("Value")->GetGetMethod());
    ilGen->Emit(Emit::OpCodes::Castclass, retType);
    ilGen->Emit(Emit::OpCodes::Ret);

    // remove from incomplete types map
    ::System::String ^ cts_name = type_builder->FullName;
    m_incomplete_singletons->Remove( cts_name );
    xSingletonType->release();
    if (g_verbose)
    {
        ::System::Console::WriteLine(
            "> emitting singleton type {0}", cts_name );
    }
    return type_builder->CreateType();
}


//______________________________________________________________________________
::System::Type ^ TypeEmitter::get_type(
    Reference< reflection::XTypeDescription > const & xType )
{
    switch (xType->getTypeClass())
    {
    case TypeClass_VOID:
        return ::System::Void::typeid;
    case TypeClass_CHAR:
        return ::System::Char::typeid;
    case TypeClass_BOOLEAN:
        return ::System::Boolean::typeid;
    case TypeClass_BYTE:
        return ::System::Byte::typeid;
    case TypeClass_SHORT:
        return ::System::Int16::typeid;
    case TypeClass_UNSIGNED_SHORT:
        return ::System::UInt16::typeid;
    case TypeClass_LONG:
        return ::System::Int32::typeid;
    case TypeClass_UNSIGNED_LONG:
        return ::System::UInt32::typeid;
    case TypeClass_HYPER:
        return ::System::Int64::typeid;
    case TypeClass_UNSIGNED_HYPER:
        return ::System::UInt64::typeid;
    case TypeClass_FLOAT:
        return ::System::Single::typeid;
    case TypeClass_DOUBLE:
        return ::System::Double::typeid;
    case TypeClass_STRING:
        return ::System::String::typeid;
    case TypeClass_TYPE:
        return ::System::Type::typeid;
    case TypeClass_ANY:
        return ::uno::Any::typeid;
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
        ::System::Type ^ element_type = get_type(
            Reference< reflection::XIndirectTypeDescription >(
                xType, UNO_QUERY_THROW )->getReferencedType() );
        ::System::Type ^ retType = get_type(
            ::System::String::Concat(
                element_type->FullName, "[]" ), true );

        ::uno::PolymorphicType ^ pt = dynamic_cast< ::uno::PolymorphicType ^ >(element_type);
        if (pt)
        {
            ::System::String ^ sName = ::System::String::Concat(pt->PolymorphicName, "[]");
            retType = ::uno::PolymorphicType::GetType(retType, sName);
        }
        return retType;
    }
    case TypeClass_INTERFACE:
        return get_type(
            Reference< reflection::XInterfaceTypeDescription2 >(
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
        return get_type(
            Reference< reflection::XServiceTypeDescription2 >(
                xType, UNO_QUERY_THROW) );
    case TypeClass_SINGLETON:
        return get_type(
            Reference< reflection::XSingletonTypeDescription2 >(
                xType, UNO_QUERY_THROW) );
    case TypeClass_MODULE:
        // ignore these
        return nullptr;
    default:
        throw RuntimeException(
            OUSTR("unexpected type ") + xType->getName(),
            Reference< XInterface >() );
    }
}

//______________________________________________________________________________
::System::Type ^ TypeEmitter::get_complete_struct( ::System::String ^ sName)
{
    struct_entry ^ pStruct = safe_cast< struct_entry ^>(
        m_incomplete_structs[sName]);
    if (pStruct)
    {
        complete_struct_type(pStruct);
    }
    //get_type will asked the module builder for the type or otherwise all known assemblies.
    return get_type(sName, true);
}
TypeEmitter::~TypeEmitter()
{
    while (true)
    {
        ::System::Collections::IDictionaryEnumerator ^ enumerator =
              m_incomplete_ifaces->GetEnumerator();
        if (! enumerator->MoveNext())
            break;
        complete_iface_type(
            safe_cast< iface_entry ^ >( enumerator->Value ) );
    }

    while (true)
    {
        ::System::Collections::IDictionaryEnumerator ^ enumerator =
              m_incomplete_structs->GetEnumerator();
        if (! enumerator->MoveNext())
            break;
        complete_struct_type(
            safe_cast< struct_entry ^ >( enumerator->Value ) );
    }


    while (true)
    {
        ::System::Collections::IDictionaryEnumerator ^ enumerator =
              m_incomplete_services->GetEnumerator();
        if (! enumerator->MoveNext())
            break;
        complete_service_type(
            safe_cast< service_entry ^ >( enumerator->Value ) );
    }

    while (true)
    {
        ::System::Collections::IDictionaryEnumerator ^ enumerator =
              m_incomplete_singletons->GetEnumerator();
        if (! enumerator->MoveNext())
            break;
        complete_singleton_type(
            safe_cast< singleton_entry ^ >( enumerator->Value ) );
    }
}
//______________________________________________________________________________
TypeEmitter::TypeEmitter(
    ::System::Reflection::Emit::ModuleBuilder ^ module_builder,
    array< ::System::Reflection::Assembly^>^ extra_assemblies )
    : m_module_builder( module_builder ),
      m_extra_assemblies( extra_assemblies ),
      m_method_info_Type_GetTypeFromHandle( nullptr ),
      m_type_Exception( nullptr ),
      m_type_RuntimeException( nullptr ),
      m_incomplete_ifaces( gcnew ::System::Collections::Hashtable() ),
      m_incomplete_structs( gcnew ::System::Collections::Hashtable() ),
      m_incomplete_services(gcnew ::System::Collections::Hashtable() ),
      m_incomplete_singletons(gcnew ::System::Collections::Hashtable() ),
      m_generated_structs( gcnew ::System::Collections::Hashtable() )
{
    array< ::System::Type^>^ param_types = gcnew array< ::System::Type^> ( 1 );
    param_types[ 0 ] = ::System::RuntimeTypeHandle::typeid;
    m_method_info_Type_GetTypeFromHandle =
        ::System::Type::typeid
          ->GetMethod( "GetTypeFromHandle", param_types );
}

::System::Collections::ArrayList ^ TypeEmitter::get_service_ctor_method_exceptions_reduced(
    const Sequence<Reference<reflection::XCompoundTypeDescription> > & seqExceptionsTd)
{
    if (seqExceptionsTd.getLength() == 0)
        return gcnew ::System::Collections::ArrayList();

    ::System::Collections::ArrayList ^ arTypes = gcnew ::System::Collections::ArrayList();
    for (int i = 0; i < seqExceptionsTd.getLength(); i++)
        arTypes->Add(get_type(to_cts_name(seqExceptionsTd[i]->getName()), true));

    int start = 0;
    while (true)
    {
        bool bRemove = false;
        for (int i = start; i < arTypes->Count; i++)
        {
            ::System::Type ^ t = safe_cast< ::System::Type^ >(arTypes[i]);
            for (int j = 0; j < arTypes->Count; j++)
            {
                if (t->IsSubclassOf(safe_cast< ::System::Type^ >(arTypes[j])))
                {
                    arTypes->RemoveAt(i);
                    bRemove = true;
                    break;
                }
            }
            if (bRemove)
                break;
            start++;
        }

        if (bRemove == false)
            break;
    }
    return arTypes;
}


css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
resolveInterfaceTypedef(
    const css::uno::Reference<css::reflection::XTypeDescription>& type)
{
    Reference<reflection::XInterfaceTypeDescription2>
        xIfaceTd(type, UNO_QUERY);

    if (xIfaceTd.is())
        return xIfaceTd;

    Reference<reflection::XIndirectTypeDescription> xIndTd(
        type, UNO_QUERY);
    if (xIndTd.is() == sal_False)
        throw css::uno::Exception(
            OUSTR("resolveInterfaceTypedef was called with an invalid argument"), 0);

    return resolveInterfaceTypedef(xIndTd->getReferencedType());
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
