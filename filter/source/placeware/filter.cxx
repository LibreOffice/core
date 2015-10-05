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

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>

#include "exporter.hxx"
#include "filter.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::com::sun::star::lang::XComponent;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::task::XStatusIndicator;

namespace pwp {



class PlaceWareExportFilter : public cppu::WeakImplHelper
<
    css::document::XFilter,
    css::document::XExporter,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{
    Reference< XComponent > mxDoc;
    Reference< XComponentContext > mxContext;

public:
    explicit PlaceWareExportFilter( const Reference< XComponentContext > &rxContext);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancel( ) throw (RuntimeException, std::exception) SAL_OVERRIDE;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException, std::exception) SAL_OVERRIDE;
};



PlaceWareExportFilter::PlaceWareExportFilter(const Reference< XComponentContext > &rxContext)
:   mxContext( rxContext )
{
}



sal_Bool SAL_CALL PlaceWareExportFilter::filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException, std::exception)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sURL;
    Reference < XInterface > xInteractionHandler;
    Reference < XOutputStream > xOutputStream;
    Reference < XStatusIndicator > xStatusIndicator;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "OutputStream" )
        {
            pValue[i].Value >>= xOutputStream;
        }
        else if( pValue[i].Name == "URL" )
        {
            pValue[i].Value >>= sURL;
        }
        else if( pValue[i].Name == "InteractionHandler" )
        {
            pValue[i].Value >>= xInteractionHandler;
        }
        else if ( pValue[i].Name == "StatusIndicator" )
        {
            pValue[i].Value >>= xStatusIndicator;
        }
    }
    if ( !xOutputStream.is() )
    {
        OSL_ASSERT ( false );
        return sal_False;
    }

    PlaceWareExporter aExporter( mxContext );
    return aExporter.doExport( mxDoc, xOutputStream, sURL, xInteractionHandler, xStatusIndicator );
}



void SAL_CALL PlaceWareExportFilter::cancel(  )
    throw (RuntimeException, std::exception)
{
}



// XExporter
void SAL_CALL PlaceWareExportFilter::setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
    throw (css::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    mxDoc = xDoc;
}



// XInitialization
void SAL_CALL PlaceWareExportFilter::initialize( const css::uno::Sequence< css::uno::Any >& /* aArguments */ )
    throw (Exception, RuntimeException, std::exception)
{
}

OUString PlaceWareExportFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.Impress.PlaceWareExportFilter" );
}

Sequence< OUString > SAL_CALL PlaceWareExportFilter_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ExportFilter";
    return aRet;
}

Reference< XInterface > SAL_CALL PlaceWareExportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new PlaceWareExportFilter( comphelper::getComponentContext(rSMgr) ));
}

// XServiceInfo
OUString SAL_CALL PlaceWareExportFilter::getImplementationName(  )
    throw (RuntimeException, std::exception)
{
    return PlaceWareExportFilter_getImplementationName();
}

sal_Bool SAL_CALL PlaceWareExportFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SAL_CALL PlaceWareExportFilter::getSupportedServiceNames(  )
    throw (RuntimeException, std::exception)
{
    return PlaceWareExportFilter_getSupportedServiceNames();
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
