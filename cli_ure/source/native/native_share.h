/*************************************************************************
 *
 *  $RCSfile: native_share.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2003-08-20 12:53:21 $
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
#using "cli_ure.dll"
#using "cli_types.dll"

#include "rtl/ustring.hxx"
#include "uno/mapping.hxx"

#include <vcclr.h>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


namespace uno
{
namespace util
{

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

template< typename T >
inline ::System::Object * to_cli(
    ::com::sun::star::uno::Reference< T > const & x )
{
    ::com::sun::star::uno::Mapping mapping(
        OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME), OUSTR(UNO_LB_CLI) );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
    {
        throw ::com::sun::star::uno::RuntimeException(
            OUSTR("cannot get mapping from C++ to CLI!"),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
    }

    intptr_t intptr =
        reinterpret_cast< intptr_t >(
            mapping.mapInterface( x.get(), ::getCppuType( &x ) ) );
    ::System::Runtime::InteropServices::GCHandle handle(
        ::System::Runtime::InteropServices::GCHandle::op_Explicit( intptr ) );
    ::System::Object * ret = handle.get_Target();
    handle.Free();
    return ret;
}

template< typename T >
inline void to_uno(
    ::com::sun::star::uno::Reference< T > * pRet, ::System::Object * x )
{
    ::com::sun::star::uno::Mapping mapping(
        OUSTR(UNO_LB_CLI), OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
    {
        throw ::com::sun::star::uno::RuntimeException(
            OUSTR("cannot get mapping from CLI to C++!"),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
    }

    ::System::Runtime::InteropServices::GCHandle handle(
        ::System::Runtime::InteropServices::GCHandle::Alloc( x ) );
    T * ret = 0;
    mapping.mapInterface(
        reinterpret_cast< void ** >( &ret ),
        reinterpret_cast< void * >(
            ::System::Runtime::InteropServices::GCHandle::op_Explicit( handle )
#if defined _WIN32
            .ToInt32()
#elif defined _WIN64
            .ToInt64()
#else
#error ERROR: either _WIN64 or _WIN32 must be defined
            ERROR: either _WIN64 or _WIN32 must be defined
#endif
            ),
        ::getCppuType( pRet ) );
    handle.Free();
    pRet->set( ret, SAL_NO_ACQUIRE /* takeover ownership */ );
}

}
}
