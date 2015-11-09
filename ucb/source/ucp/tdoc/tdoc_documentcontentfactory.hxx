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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCUMENTCONTENTFACTORY_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCUMENTCONTENTFACTORY_HXX

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp"

#include <cppuhelper/implbase.hxx>

namespace tdoc_ucp {

class DocumentContentFactory :
        public cppu::WeakImplHelper<
            css::frame::XTransientDocumentsDocumentContentFactory,
            css::lang::XServiceInfo >
{
public:
    explicit DocumentContentFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~DocumentContentFactory();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XTransientDocumentsDocumentContentFactory
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    createDocumentContent( const css::uno::Reference< css::frame::XModel >& Model )
        throw ( css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception ) override;

    // Non-UNO interfaces
    static OUString
    getImplementationName_Static();
    static css::uno::Sequence< OUString >
    getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference< css::lang::XMultiServiceFactory > & rxServiceMgr );
private:
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMgr;
};

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCUMENTCONTENTFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
