/*************************************************************************
 *
 *  $RCSfile: climaker_share.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2003-03-28 10:17:38 $
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

#using <mscorlib.dll>
#include <vcclr.h>

#include "osl/diagnose.h"
#include "com/sun/star/reflection/XConstantTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription.hpp"
#include "com/sun/star/reflection/XCompoundTypeDescription.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


namespace css = ::com::sun::star;

namespace climaker
{

//--------------------------------------------------------------------------------------------------
extern bool g_verbose;

//--------------------------------------------------------------------------------------------------
inline ::System::String * ustring_to_String( ::rtl::OUString const & ustr )
{
    return new ::System::String( ustr.getStr(), 0, ustr.getLength() );
}
//--------------------------------------------------------------------------------------------------
inline ::rtl::OUString String_to_ustring( ::System::String * str )
{
    OSL_ASSERT( sizeof (wchar_t) == sizeof (sal_Unicode) );
    wchar_t const __pin * chars = PtrToStringChars( str );
    return ::rtl::OUString( chars, str->get_Length() );
}

//==================================================================================================
__gc class TypeEmitter : public ::System::IDisposable
{
    const ::System::Reflection::MethodAttributes s_ctor_attr;

    ::System::Reflection::Emit::ModuleBuilder * m_module_builder;
    ::System::Reflection::Assembly * m_extra_assemblies __gc [];

    ::System::Reflection::MethodInfo * m_method_info_Type_GetTypeFromHandle;

    ::System::Type * m_type_Exception;
    ::System::Type * get_type_Exception();
    ::System::Type * m_type_RuntimeException;
    ::System::Type * get_type_RuntimeException();
    ::System::Reflection::ConstructorInfo * m_ctor_uno_MethodAttribute;
    ::System::Reflection::ConstructorInfo * m_default_ctor_uno_MethodAttribute;
    ::System::Reflection::ConstructorInfo * get_ctor_uno_MethodAttribute();
    ::System::Type * m_type_Any;
    ::System::Type * m_type_Any_ref;
    ::System::Reflection::FieldInfo * m_field_Any_VOID;
    void emit_Any_boxed_ctor(
        ::System::Reflection::Emit::TypeBuilder * type_builder,
        ::System::Reflection::Emit::FieldBuilder * field_Type,
        ::System::Reflection::Emit::FieldBuilder * field_Value,
        ::System::Type * integral_type );
    ::System::Type * get_type_Any();

    __gc class iface_entry
    {
    public:
        css::reflection::XInterfaceTypeDescription * m_xType;
        ::System::Reflection::Emit::TypeBuilder * m_type_builder;
    };
    ::System::Collections::Hashtable * m_incomplete_ifaces;
    ::System::Type * complete_iface_type( iface_entry * entry );

    __gc class struct_entry
    {
    public:
        ::System::String * m_member_names __gc [];
        ::System::Type * m_param_types __gc [];
        ::System::Reflection::ConstructorInfo * m_default_ctor;
        ::System::Reflection::ConstructorInfo * m_ctor;
    };
    ::System::Collections::Hashtable * m_generated_structs;

    ::System::Type * get_type(
        ::System::String * cli_name, bool throw_exc );
    ::System::Type * get_type(
        css::uno::Reference< css::reflection::XConstantTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference< css::reflection::XConstantsTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference< css::reflection::XEnumTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference< css::reflection::XCompoundTypeDescription > const & xType );
    ::System::Type * get_type(
        css::uno::Reference< css::reflection::XInterfaceTypeDescription > const & xType );
public:
    TypeEmitter(
        ::System::Reflection::Emit::ModuleBuilder * module_builder,
        ::System::Reflection::Assembly * assemblies __gc [] );
    // must be called to finish up uncompleted types
    void Dispose();

    ::System::Reflection::Assembly * type_resolve(
        ::System::Object * sender, ::System::ResolveEventArgs * args );

    ::System::Type * get_type(
        css::uno::Reference< css::reflection::XTypeDescription > const & xType );
};

}
