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
#include <cppuhelper/implbase4.hxx>

#include "exporter.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::rtl::OUString;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::task::XStatusIndicator;

namespace pwp {

// -----------------------------------------------------------------------------

class PlaceWareExportFilter : public cppu::WeakImplHelper4
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XExporter,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
    Reference< XComponent > mxDoc;
    Reference< XMultiServiceFactory > mxMSF;

public:
    PlaceWareExportFilter( const Reference< XMultiServiceFactory > &rxMSF);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) throw(RuntimeException);
    virtual void SAL_CALL cancel( ) throw (RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException);
};

// -----------------------------------------------------------------------------

PlaceWareExportFilter::PlaceWareExportFilter(const Reference< XMultiServiceFactory > &rxMSF)
:   mxMSF( rxMSF )
{
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PlaceWareExportFilter::filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sURL;
    Reference < XInterface > xInteractionHandler;
    Reference < XOutputStream > xOutputStream;
    Reference < XStatusIndicator > xStatusIndicator;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "OutputStream" ) ) )
        {
            pValue[i].Value >>= xOutputStream;
        }
        else if( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
        {
            pValue[i].Value >>= sURL;
        }
        else if( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InteractionHandler" ) ) )
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
        OSL_ASSERT ( 0 );
        return sal_False;
    }

    PlaceWareExporter aExporter( mxMSF );
    return aExporter.doExport( mxDoc, xOutputStream, sURL, xInteractionHandler, xStatusIndicator );
}

// -----------------------------------------------------------------------------

void SAL_CALL PlaceWareExportFilter::cancel(  )
    throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

// XExporter
void SAL_CALL PlaceWareExportFilter::setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDoc = xDoc;
}

// -----------------------------------------------------------------------------

// XInitialization
void SAL_CALL PlaceWareExportFilter::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& /* aArguments */ )
    throw (Exception, RuntimeException)
{
}

// -----------------------------------------------------------------------------

OUString PlaceWareExportFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Impress.PlaceWareExportFilter" ) );
}

// -----------------------------------------------------------------------------

#define SERVICE_NAME "com.sun.star.document.ExportFilter"

sal_Bool SAL_CALL PlaceWareExportFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName == SERVICE_NAME;
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL PlaceWareExportFilter_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL PlaceWareExportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new PlaceWareExportFilter( rSMgr );
}

// -----------------------------------------------------------------------------

// XServiceInfo
OUString SAL_CALL PlaceWareExportFilter::getImplementationName(  )
    throw (RuntimeException)
{
    return PlaceWareExportFilter_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL PlaceWareExportFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return PlaceWareExportFilter_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL PlaceWareExportFilter::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return PlaceWareExportFilter_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
