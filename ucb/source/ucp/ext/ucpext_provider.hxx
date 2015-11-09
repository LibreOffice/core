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

#ifndef INCLUDED_UCB_SOURCE_UCP_EXT_UCPEXT_PROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_EXT_UCPEXT_PROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>


namespace ucb { namespace ucp { namespace ext
{



    //= ContentProvider

    typedef ::ucbhelper::ContentProviderImplHelper  ContentProvider_Base;
    class ContentProvider : public ContentProvider_Base
    {
    public:
        explicit ContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~ContentProvider();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static() throw (css::uno::RuntimeException);
        static css::uno::Reference< css::uno::XInterface > SAL_CALL Create( const css::uno::Reference< css::uno::XComponentContext >& i_rContext );

        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) throw (css::ucb::IllegalIdentifierException, css::uno::RuntimeException, std::exception) override;

    public:
        static OUString getRootURL();
        static OUString getArtificialNodeContentType();
    };


} } }   // namespace ucb::ucp::ext


#endif // INCLUDED_UCB_SOURCE_UCP_EXT_UCPEXT_PROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
