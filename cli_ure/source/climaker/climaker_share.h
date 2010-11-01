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

#using <mscorlib.dll>
#using <cli_basetypes.dll>

#include <vcclr.h>

#include "osl/diagnose.h"
#include "com/sun/star/reflection/XConstantTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XCompoundTypeDescription.hpp"
#include "com/sun/star/reflection/XServiceTypeDescription2.hpp"
#include "com/sun/star/reflection/XSingletonTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


namespace css = ::com::sun::star;

namespace climaker
{

//------------------------------------------------------------------------------
extern bool g_verbose;

__gc struct Constants
{
    static const ::System::String * sUnoVoid = S"void";
    static const ::System::String * sUnoType = S"type";
    static const ::System::String * sUnoAny =  S"any";
    static const ::System::String * sUnoBool = S"boolean";
    static const ::System::String * sUnoByte = S"byte";
    static const ::System::String * sUnoChar = S"char";
    static const ::System::String * sUnoShort = S"short";
    static const ::System::String * sUnoUShort = S"unsigned short";
    static const ::System::String * sUnoLong = S"long";
    static const ::System::String * sUnoULong = S"unsigned long";
    static const ::System::String * sUnoHyper = S"hyper";
    static const ::System::String * sUnoUHyper = S"unsigned hyper";
    static const ::System::String * sUnoString = S"string";
    static const ::System::String * sUnoFloat = S"float";
    static const ::System::String * sUnoDouble = S"double";
    static const ::System::String * sUnoXInterface = S"com.sun.star.uno.XInterface";
    static const ::System::String * sBrackets = S"[]";

    static const System::String* sObject = S"System.Object";
    static const System::String* sType = S"System.Type";
    static const System::String* sUnoidl = S"unoidl.";
    static const System::String* sVoid = S"System.Void";
    static const System::String* sAny = S"uno.Any";
    static const System::String* sBoolean = S"System.Boolean";
    static const System::String* sChar = S"System.Char";
    static const System::String* sByte = S"System.Byte";
    static const System::String* sInt16 = S"System.Int16";
    static const System::String* sUInt16 = S"System.UInt16";
    static const System::String* sInt32 = S"System.Int32";
    static const System::String* sUInt32 = S"System.UInt32";
    static const System::String* sInt64 = S"System.Int64";
    static const System::String* sUInt64 = S"System.UInt64";
    static const System::String* sString = S"System.String";
    static const System::String* sSingle = S"System.Single";
    static const System::String* sDouble = S"System.Double";
    static const System::String* sComma = new System::String(S",");

};

//------------------------------------------------------------------------------
inline ::System::String * ustring_to_String( ::rtl::OUString const & ustr )
{
    return new ::System::String( ustr.getStr(), 0, ustr.getLength() );
}

//------------------------------------------------------------------------------
inline ::rtl::OUString String_to_ustring( ::System::String * str )
{
    OSL_ASSERT( sizeof (wchar_t) == sizeof (sal_Unicode) );
    wchar_t const __pin * chars = PtrToStringChars( str );
    return ::rtl::OUString( chars, str->get_Length() );
}

/* If the argument type is a typedef for an interface then the interface
   type description is returned, otherwise an exeption is thrown.
*/
css::uno::Reference< css::reflection::XInterfaceTypeDescription2 >
resolveInterfaceTypedef(const css::uno::Reference<css::reflection::XTypeDescription>& type);

const ::System::Reflection::MethodAttributes c_ctor_method_attr =
(::System::Reflection::MethodAttributes)
    (::System::Reflection::MethodAttributes::Public |
     ::System::Reflection::MethodAttributes::HideBySig |
     ::System::Reflection::MethodAttributes::SpecialName |
     ::System::Reflection::MethodAttributes::RTSpecialName
     /* | xxx todo: ??? compiler does not know Instance ???
        ::System::Reflection::MethodAttributes::Instance*/);

//==============================================================================
__gc class TypeEmitter : public ::System::IDisposable
{
    ::System::Reflection::Emit::ModuleBuilder * m_module_builder;
    ::System::Reflection::Assembly * m_extra_assemblies __gc [];

    ::System::Reflection::MethodInfo * m_method_info_Type_GetTypeFromHandle;

    ::System::Type * m_type_Exception;
    ::System::Type * get_type_Exception();
    ::System::Type * m_type_RuntimeException;
    ::System::Type * get_type_RuntimeException();

    ::System::Reflection::Emit::CustomAttributeBuilder* get_service_exception_attribute(
        const css::uno::Reference<css::reflection::XServiceConstructorDescription> & ctorDesc);
    ::System::Reflection::Emit::CustomAttributeBuilder* get_iface_method_exception_attribute(
        const css::uno::Reference< css::reflection::XInterfaceMethodTypeDescription >& xMethod );
    ::System::Reflection::Emit::CustomAttributeBuilder* get_exception_attribute(
        const css::uno::Sequence<css::uno::Reference<
        css::reflection::XCompoundTypeDescription > >& seq_exceptionsTd );
/* Creates ::System::Type object for UNO exceptions. The UNO exceptions are
       obtained by
       com::sun::star::reflection::XServiceConstructorDescription::getExceptions
       In a first step the respective CLI types are created. Then it is examined
       if a Type represents a super class of another class. If so the Type of the
       derived class is discarded. For example there are a uno RuntimeException and
       a DeploymentException which inherits RuntimeException. Then only the cli Type
       of the RuntimeException is returned.
       The purpose of this function is to provide exceptions for which catch blocks
       are generated in the service constructor code.

       It is always an instance of an ArrayList returned, even if the sequence argument
       does not contain elements.
    */
    ::System::Collections::ArrayList * get_service_ctor_method_exceptions_reduced(
        const css::uno::Sequence<
        css::uno::Reference<css::reflection::XCompoundTypeDescription> > & seqExceptionsTd);


    __gc class iface_entry
    {
    public:
        css::reflection::XInterfaceTypeDescription2 * m_xType;
        ::System::Reflection::Emit::TypeBuilder * m_type_builder;
    };
    ::System::Collections::Hashtable * m_incomplete_ifaces;
    ::System::Type * complete_iface_type( iface_entry * entry );

    __gc class struct_entry
    {
    public:
         css::reflection::XCompoundTypeDescription * m_xType;
        ::System::Reflection::Emit::TypeBuilder * m_type_builder;
        ::System::Type * m_base_type;

        ::System::String * m_member_names __gc [];
        ::System::Type * m_param_types __gc [];
        ::System::Reflection::ConstructorInfo * m_default_ctor;
        ::System::Reflection::ConstructorInfo * m_ctor;
    };
    ::System::Collections::Hashtable * m_incomplete_structs;
    ::System::Type * complete_struct_type( struct_entry * entry );

    /*  returns the type for the name. If it is a struct then it may
        complete the struct if not already done. This also refers to its
        base types.

        @param sName
            the full name of the type.
        @return the type object for sName. Not necessarily a struct.
    */
    ::System::Type * get_complete_struct( ::System::String * sName);

    __gc class service_entry
    {
    public:
        ::System::Reflection::Emit::TypeBuilder * m_type_builder;
        css::reflection::XServiceTypeDescription2 * m_xType;
    };
    ::System::Collections::Hashtable * m_incomplete_services;
    ::System::Type * complete_service_type(service_entry * entry);

    __gc class singleton_entry
    {
    public:
        ::System::Reflection::Emit::TypeBuilder * m_type_builder;
        css::reflection::XSingletonTypeDescription2 * m_xType;
    };


    ::System::Collections::Hashtable * m_incomplete_singletons;
    ::System::Type * complete_singleton_type(singleton_entry * entry);


    ::System::Collections::Hashtable * m_generated_structs;

    ::System::Type * get_type(
        ::System::String * cli_name, bool throw_exc );
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XConstantTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XConstantsTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XEnumTypeDescription > const & xType );
    /* returns the type for a struct or exception. In case of a polymorphic struct it may
        return a ::uno::PolymorphicType (cli_basetypes.dll) only if the struct is already
        complete.
    */
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XCompoundTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XInterfaceTypeDescription2 > const & xType );
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XSingletonTypeDescription2 > const & xType );

    /*
      May return NULL if the service description is an obsolete. See
      description of
      com.sun.star.reflection.XServiceTypeDescription2.isSingleInterfaceBased
     */
    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XServiceTypeDescription2 > const & xType );
public:
    TypeEmitter(
        ::System::Reflection::Emit::ModuleBuilder * module_builder,
        ::System::Reflection::Assembly * assemblies __gc [] );
    // must be called to finish up uncompleted types
    void Dispose();

    ::System::Reflection::Assembly * type_resolve(
        ::System::Object * sender, ::System::ResolveEventArgs * args );

    ::System::Type * get_type(
        css::uno::Reference<
        css::reflection::XTypeDescription > const & xType );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
