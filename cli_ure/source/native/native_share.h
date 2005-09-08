/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: native_share.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:02:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
