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
#using "cli_ure.dll"
#using "cli_uretypes.dll"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
