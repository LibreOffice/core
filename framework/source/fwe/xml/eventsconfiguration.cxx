/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <framework/eventsconfiguration.hxx>
#include <xml/eventsdocumenthandler.hxx>
#include <services.h>

#include <xml/saxnamespacefilter.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/debug.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;


namespace framework
{

static Reference< XParser > GetSaxParser(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    //Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    //return Reference< XParser >( xServiceManager->createInstance( SERVICENAME_SAXPARSER), UNO_QUERY);
    return Reference< XParser >( xServiceFactory->createInstance( SERVICENAME_SAXPARSER), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    //Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    //return Reference< XDocumentHandler >( xServiceManager->createInstance( SERVICENAME_SAXWRITER), UNO_QUERY) ;
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( SERVICENAME_SAXWRITER), UNO_QUERY) ;
}

// #110897#
sal_Bool EventsConfiguration::LoadEventsConfig(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    SvStream& rInStream, EventsConfig& aItems )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );
    Reference< XInputStream > xInputStream(
                                (::cppu::OWeakObject *)new utl::OInputStreamWrapper( rInStream ),
                                UNO_QUERY );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set events document handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadEventsDocumentHandler( aItems ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch( SAXException& )
    {
        return sal_False;
    }
    catch( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

// #110897#
sal_Bool EventsConfiguration::StoreEventsConfig(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    SvStream& rOutStream, const EventsConfig& aItems )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );

    Reference< XOutputStream > xOutputStream(
                                (::cppu::OWeakObject *)new utl::OOutputStreamWrapper( rOutStream ),
                                UNO_QUERY );

    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( xOutputStream );

    try
    {
        OWriteEventsDocumentHandler aWriteEventsDocumentHandler( aItems, xWriter );
        aWriteEventsDocumentHandler.WriteEventsDocument();
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch ( SAXException& )
    {
        return sal_False;
    }
    catch ( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
