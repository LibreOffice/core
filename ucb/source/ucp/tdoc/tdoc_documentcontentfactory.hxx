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

#pragma once
#if 1

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp"

#include "cppuhelper/implbase2.hxx"

namespace tdoc_ucp {

class DocumentContentFactory :
        public cppu::WeakImplHelper2<
            com::sun::star::frame::XTransientDocumentsDocumentContentFactory,
            com::sun::star::lang::XServiceInfo >
{
public:
    DocumentContentFactory( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~DocumentContentFactory();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // XTransientDocumentsDocumentContentFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    createDocumentContent( const ::com::sun::star::uno::Reference<
                                com::sun::star::frame::XModel >& Model )
        throw ( com::sun::star::lang::IllegalArgumentException,
                com::sun::star::uno::RuntimeException );

    // Non-UNO interfaces
    static rtl::OUString
    getImplementationName_Static();
    static com::sun::star::uno::Sequence< rtl::OUString >
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

#endif /* !INCLUDED_TDOC_DOCUMENTCONTENTFACTORY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
