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

#ifndef INCLUDED_STOC_INC_STOCSERVICES_HXX
#define INCLUDED_STOC_INC_STOCSERVICES_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
    class XComponentContext;
} } } }

namespace stoc_services
{
    // typeconv
    css::uno::Reference< css::uno::XInterface > SAL_CALL TypeConverter_Impl_CreateInstance(
        const css::uno::Reference< css::uno::XComponentContext > & )
    throw( css::uno::RuntimeException );
    css::uno::Sequence< OUString > tcv_getSupportedServiceNames();
    OUString tcv_getImplementationName();

    namespace ExternalUriReferenceTranslator
    {
    css::uno::Reference< css::uno::XInterface > SAL_CALL create(
        css::uno::Reference< css::uno::XComponentContext > const & context)
        SAL_THROW((css::uno::Exception));
    OUString SAL_CALL getImplementationName();
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriReferenceFactory
    {
    css::uno::Reference< css::uno::XInterface > SAL_CALL create(
        css::uno::Reference< css::uno::XComponentContext >  const & context)
        SAL_THROW((css::uno::Exception));
    OUString SAL_CALL getImplementationName();
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTexpand
    {
    css::uno::Reference< css::uno::XInterface > SAL_CALL create(
        css::uno::Reference< css::uno::XComponentContext > const &)
        SAL_THROW((css::uno::Exception));
    OUString SAL_CALL getImplementationName();
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTscript
    {
    css::uno::Reference< css::uno::XInterface > SAL_CALL create(
        css::uno::Reference< css::uno::XComponentContext > const &)
        SAL_THROW((css::uno::Exception));
    OUString SAL_CALL getImplementationName();
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace VndSunStarPkgUrlReferenceFactory
    {
    css::uno::Reference< css::uno::XInterface > SAL_CALL create(
        css::uno::Reference< css::uno::XComponentContext > const & context)
        SAL_THROW((css::uno::Exception));
    OUString SAL_CALL getImplementationName();
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }
} // namespace

#endif // INCLUDED_STOC_INC_STOCSERVICES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
