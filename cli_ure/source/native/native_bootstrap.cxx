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

// Use UNICODE Windows and C API.
#define _UNICODE
#define UNICODE

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>

#include "native_share.h"

#include "rtl/bootstrap.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/bootstrap.hxx"
#include <stdio.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace uno
{
namespace util
{

/** Bootstrapping native UNO.

    Bootstrapping requires the existence of many libraries which are contained
    in an URE installation. To find and load these libraries the Windows
    registry keys HKEY_CURRENT_USER\Software\LibreOffice\Layer\URE\1
    and HKEY_LOCAL_MACHINE\Software\LibreOffice\Layer\URE\1 are examined.
    These contain a named value UREINSTALLLOCATION which holds a path to the URE
    installation folder.
*/
public ref class Bootstrap sealed
{
    inline Bootstrap() {}

public:

    /** Bootstraps the initial component context from a native UNO installation.

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext ^
        defaultBootstrap_InitialComponentContext();

    /** Bootstraps the initial component context from a native UNO installation.

        @param ini_file
               a file URL of an ini file, e.g. uno.ini/unorc. (The ini file must
               reside next to the cppuhelper library)
        @param bootstrap_parameters
               bootstrap parameters (maybe null)

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext ^
        defaultBootstrap_InitialComponentContext(
            ::System::String ^ ini_file,
            ::System::Collections::IDictionaryEnumerator ^
              bootstrap_parameters );

    /** Bootstraps the initial component context from a native UNO installation.

    @see cppuhelper/bootstrap.hxx:bootstrap()
     */
    static ::unoidl::com::sun::star::uno::XComponentContext ^
    bootstrap();
};


::unoidl::com::sun::star::uno::XComponentContext ^
Bootstrap::defaultBootstrap_InitialComponentContext(
    ::System::String ^ ini_file,
    ::System::Collections::IDictionaryEnumerator ^ bootstrap_parameters )
{
    if (nullptr != bootstrap_parameters)
    {
        bootstrap_parameters->Reset();
        while (bootstrap_parameters->MoveNext())
        {
            OUString key(
                String_to_ustring( safe_cast< ::System::String ^ >(
                                       bootstrap_parameters->Key ) ) );
            OUString value(
                String_to_ustring( safe_cast< ::System::String ^ >(
                                       bootstrap_parameters->Value ) ) );

            ::rtl::Bootstrap::set( key, value );
        }
    }

    // bootstrap native uno
    Reference< XComponentContext > xContext;
    if (nullptr == ini_file)
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext();
    }
    else
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext(
            String_to_ustring( safe_cast< ::System::String ^ >( ini_file ) ) );
    }

    return safe_cast< ::unoidl::com::sun::star::uno::XComponentContext ^ >(
        to_cli( xContext ) );
}


::unoidl::com::sun::star::uno::XComponentContext ^
Bootstrap::defaultBootstrap_InitialComponentContext()
{
    return defaultBootstrap_InitialComponentContext( nullptr, nullptr );
}

::unoidl::com::sun::star::uno::XComponentContext ^ Bootstrap::bootstrap()
{
    Reference<XComponentContext> xContext = ::cppu::bootstrap();
    return safe_cast< ::unoidl::com::sun::star::uno::XComponentContext ^ >(
        to_cli( xContext ) );

}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
