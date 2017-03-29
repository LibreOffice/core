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

#using "cli_ure.dll"
#using "cli_uretypes.dll"

#include "rtl/ustring.hxx"
#include <uno/lbnames.h>
#include "uno/mapping.hxx"

#include <vcclr.h>


namespace uno
{
namespace util
{


inline ::System::String ^ ustring_to_String( OUString const & ustr )
{
    return gcnew ::System::String( ustr.getStr(), 0, ustr.getLength() );
}

inline OUString String_to_ustring( ::System::String ^ str )
{
    OSL_ASSERT( sizeof (wchar_t) == sizeof (sal_Unicode) );
    pin_ptr<wchar_t const> chars = PtrToStringChars( str );
    return OUString( chars, str->Length );
}

template< typename T >
inline ::System::Object ^ to_cli(
    css::uno::Reference< T > const & x )
{
    css::uno::Mapping mapping(
        CPPU_CURRENT_LANGUAGE_BINDING_NAME, UNO_LB_CLI );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
    {
        throw css::uno::RuntimeException(
            "cannot get mapping from C++ to CLI!",
            css::uno::Reference<
              css::uno::XInterface >() );
    }

    intptr_t intptr =
        reinterpret_cast< intptr_t >(
            mapping.mapInterface( x.get(), cppu::UnoType<decltype(x)>::get() ) );
    ::System::Runtime::InteropServices::GCHandle ^ handle = ::System::Runtime::InteropServices::GCHandle::FromIntPtr(::System::IntPtr(intptr));
    ::System::Object ^ ret = handle->Target;
    handle->Free();
    return ret;
}

template< typename T >
inline void to_uno(
    css::uno::Reference< T > * pRet, ::System::Object ^ x )
{
    css::uno::Mapping mapping(
        UNO_LB_CLI, CPPU_CURRENT_LANGUAGE_BINDING_NAME );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
    {
        throw css::uno::RuntimeException(
            "cannot get mapping from CLI to C++!",
            css::uno::Reference<
              css::uno::XInterface >() );
    }

    ::System::Runtime::InteropServices::GCHandle handle(
        ::System::Runtime::InteropServices::GCHandle::Alloc( x ) );
    T * ret = 0;
    mapping.mapInterface(
        reinterpret_cast< void ** >( &ret ),
        reinterpret_cast< void * >(
            ::System::Runtime::InteropServices::GCHandle::op_Explicit( handle )
#if defined _WIN64
            .ToInt64()
#elif defined _WIN32
            .ToInt32()
#else
#error ERROR: either _WIN64 or _WIN32 must be defined
            ERROR: either _WIN64 or _WIN32 must be defined
#endif
            ),
        cppu::UnoType<T>::get() );
    handle.Free();
    pRet->set( ret, SAL_NO_ACQUIRE /* takeover ownership */ );
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
