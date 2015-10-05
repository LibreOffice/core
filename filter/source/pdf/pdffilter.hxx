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

#ifndef INCLUDED_FILTER_SOURCE_PDF_PDFFILTER_HXX
#define INCLUDED_FILTER_SOURCE_PDF_PDFFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <osl/diagnose.h>
#include <rtl/process.h>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;


// - PDFFilter -


class PDFFilter : public cppu::WeakImplHelper < XFilter,
                                                 XExporter,
                                                 XInitialization,
                                                 XServiceInfo >
{
private:

    Reference< XComponentContext >      mxContext;
    Reference< XComponent >             mxSrcDoc;

    bool                            implExport( const Sequence< PropertyValue >& rDescriptor );

protected:

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancel( ) throw (RuntimeException, std::exception) SAL_OVERRIDE;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException, std::exception) SAL_OVERRIDE;

public:

    explicit    PDFFilter( const Reference< XComponentContext >& rxContext );
    virtual     ~PDFFilter();
};



OUString PDFFilter_getImplementationName ()
    throw ( RuntimeException );



bool SAL_CALL PDFFilter_supportsService( const OUString& ServiceName )
    throw ( RuntimeException );



Sequence< OUString > SAL_CALL PDFFilter_getSupportedServiceNames(  )
    throw ( RuntimeException );



Reference< XInterface >
SAL_CALL PDFFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw ( Exception );

#endif // INCLUDED_FILTER_SOURCE_PDF_PDFFILTER_HXX










/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
