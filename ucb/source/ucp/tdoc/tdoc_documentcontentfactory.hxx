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
            com::sun::star::frame::XTransientDocumentsDocumentContentFactory,
            com::sun::star::lang::XServiceInfo >
{
public:
    explicit DocumentContentFactory( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~DocumentContentFactory();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XTransientDocumentsDocumentContentFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    createDocumentContent( const ::com::sun::star::uno::Reference<
                                com::sun::star::frame::XModel >& Model )
        throw ( com::sun::star::lang::IllegalArgumentException,
                com::sun::star::uno::RuntimeException, std::exception ) override;

    // Non-UNO interfaces
    static OUString
    getImplementationName_Static();
    static com::sun::star::uno::Sequence< OUString >
    getSupportedServiceNames_Static();

    static com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleServiceFactory >
    createServiceFactory( const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & rxServiceMgr );
private:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
};

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCUMENTCONTENTFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
