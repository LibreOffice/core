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

#ifndef _MANIFEST_READER_HXX
#define _MANIFEST_READER_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star {
    namespace lang { class XSingleServiceFactory; }
    namespace uno { class XComponentContext; }
} } }

class ManifestReader: public ::cppu::WeakImplHelper2
<
    ::com::sun::star::packages::manifest::XManifestReader,
    ::com::sun::star::lang::XServiceInfo
>
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
public:
    ManifestReader( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~ManifestReader();

    // XManifestReader
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL readManifestSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rStream )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Component constructor
    static OUString static_getImplementationName();
    static ::com::sun::star::uno::Sequence < OUString > static_getSupportedServiceNames();
    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > createServiceFactory( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
