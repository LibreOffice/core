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
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TypeConverter_Impl_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & )
    throw( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Sequence< OUString > tcv_getSupportedServiceNames();
    OUString tcv_getImplementationName();

    namespace ExternalUriReferenceTranslator
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriReferenceFactory
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >  const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTexpand
    {
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((::com::sun::star::uno::Exception));
    OUString SAL_CALL getImplementationName();
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTscript
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((com::sun::star::uno::Exception));
    OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace VndSunStarPkgUrlReferenceFactory
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames();
    }
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
