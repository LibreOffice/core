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
#ifndef INCLUDED_CPPUHELPER_SHLIB_HXX
#define INCLUDED_CPPUHELPER_SHLIB_HXX

#include "cppuhelper/cppuhelperdllapi.h"
#include "com/sun/star/uno/Reference.h"
#include "rtl/ustring.hxx"

namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }
namespace com { namespace sun { namespace star { namespace registry { class XRegistryKey; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XInterface; } } } }

namespace cppu
{

/** Loads a shared library component and gets the factory out of it.  You can give either a
    fully qualified libname or single lib name.  The libname need not be pre/postfixed
    (e.g. xxx.dll).

    @deprecated
    This should never have been put into the URE interface.  Do not call it from
    client code.

    @param uri URI of the library
    @param rPath deprecated, must be empty
    @param rImplName implementation to be retrieved from the library
    @param xMgr service manager to be provided to the component
    @param xKey deprecated, must be null
    @return
    factory instance (css::lang::XSingleComponentFactory or legacy
    css::lang::XSingleServiceFactory)
*/
CPPUHELPER_DLLPUBLIC css::uno::Reference< css::uno::XInterface >
SAL_CALL loadSharedLibComponentFactory(
    ::rtl::OUString const & uri,
    ::rtl::OUString const & rPath,
    ::rtl::OUString const & rImplName,
    css::uno::Reference< css::lang::XMultiServiceFactory > const & xMgr,
    css::uno::Reference< css::registry::XRegistryKey > const & xKey );

/** Invokes component_writeInfo() function of specified component library.  You can give either
    a fully qualified libname or single lib name. The libname need not be pre/postfixed
    (e.g. xxx.dll).

    @deprecated
    This should never have been put into the URE interface.  Do not call it from
    client code.  Also, this functionality is not needed for passively
    registered components, only for actively registered legacy ones.

    @param uri URI of the library
    @param rPath deprecated, must be empty
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
*/
CPPUHELPER_DLLPUBLIC void
SAL_CALL writeSharedLibComponentInfo(
    ::rtl::OUString const & uri,
    ::rtl::OUString const & rPath,
    css::uno::Reference< css::lang::XMultiServiceFactory > const & xMgr,
    css::uno::Reference< css::registry::XRegistryKey > const & xKey );

} // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
