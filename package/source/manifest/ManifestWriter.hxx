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

#ifndef INCLUDED_PACKAGE_SOURCE_MANIFEST_MANIFESTWRITER_HXX
#define INCLUDED_PACKAGE_SOURCE_MANIFEST_MANIFESTWRITER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star {
    namespace lang { class XSingleServiceFactory; }
    namespace uno { class XComponentContext;  }
} } }

class ManifestWriter: public ::cppu::WeakImplHelper
<
    ::com::sun::star::packages::manifest::XManifestWriter,
    ::com::sun::star::lang::XServiceInfo
>
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
public:
    ManifestWriter( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~ManifestWriter();

    // XManifestWriter
    virtual void SAL_CALL writeManifestSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rStream, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& rSequence )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // Component constructor
    static OUString static_getImplementationName();
    static ::com::sun::star::uno::Sequence < OUString > static_getSupportedServiceNames();
    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > createServiceFactory( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
