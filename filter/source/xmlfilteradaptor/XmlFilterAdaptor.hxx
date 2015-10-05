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

#ifndef INCLUDED_FILTER_SOURCE_XMLFILTERADAPTOR_XMLFILTERADAPTOR_HXX
#define INCLUDED_FILTER_SOURCE_XMLFILTERADAPTOR_XMLFILTERADAPTOR_HXX


#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>



enum FilterType
{
    FILTER_IMPORT,
    FILTER_EXPORT
};

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */

class XmlFilterAdaptor : public cppu::WeakImplHelper
<
    css::document::XFilter,
    css::document::XExporter,
    css::document::XImporter,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{

protected:

    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::lang::XComponent > mxDoc;
    OUString msFilterName;
    css::uno::Sequence< OUString > msUserData;
    OUString msTemplateName;
    FilterType meType;

    bool SAL_CALL exportImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
        throw (css::uno::RuntimeException);

    bool SAL_CALL importImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
        throw (css::uno::RuntimeException);



public:

    explicit XmlFilterAdaptor( const css::uno::Reference< css::uno::XComponentContext > & rxContext)
        : mxContext(rxContext)
        , meType(FILTER_IMPORT)
    {
    }

    virtual ~XmlFilterAdaptor() {}



    // XFilter

    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL cancel(  )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;



    // XExporter

    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;



    // XImporter

    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )

        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;



    // XInitialization

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )

        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;



    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  )

        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )

        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )

        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};



OUString XmlFilterAdaptor_getImplementationName()
    throw ( css::uno::RuntimeException );



bool SAL_CALL XmlFilterAdaptor_supportsService( const OUString& ServiceName )
    throw ( css::uno::RuntimeException );



css::uno::Sequence< OUString > SAL_CALL XmlFilterAdaptor_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );



css::uno::Reference< css::uno::XInterface >

SAL_CALL XmlFilterAdaptor_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)

    throw ( css::uno::Exception );



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
