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

#ifndef UCB_UCPEXT_CONTENT_PROVIDER_HXX
#define UCB_UCPEXT_CONTENT_PROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>


namespace ucb { namespace ucp { namespace ext
{



    //= ContentProvider

    typedef ::ucbhelper::ContentProviderImplHelper  ContentProvider_Base;
    class ContentProvider : public ContentProvider_Base
    {
    public:
        ContentProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~ContentProvider();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static() throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext );

        // XContentProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > SAL_CALL queryContent( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier >& Identifier ) throw (::com::sun::star::ucb::IllegalIdentifierException, ::com::sun::star::uno::RuntimeException);

    public:
        static OUString getRootURL();
        static OUString getArtificialNodeContentType();
    };


} } }   // namespace ucb::ucp::ext


#endif  // UCB_UCPEXT_CONTENT_PROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
