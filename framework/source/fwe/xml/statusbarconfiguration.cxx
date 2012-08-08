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


#include <framework/statusbarconfiguration.hxx>
#include <xml/statusbardocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>
#include <services.h>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;


namespace framework
{

static Reference< XParser > GetSaxParser(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XParser >( xServiceFactory->createInstance( SERVICENAME_SAXPARSER), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( SERVICENAME_SAXWRITER), UNO_QUERY) ;
}

sal_Bool StatusBarConfiguration::LoadStatusBar(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XInputStream >& xInputStream,
    const Reference< XIndexContainer >& rStatusbarConfiguration )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );

    // connect stream to input stream to the parser
    InputSource aInputSource;
    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set menudocument handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadStatusBarDocumentHandler( rStatusbarConfiguration ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return sal_True;
    }
    catch ( const RuntimeException& )
    {
        return sal_False;
    }
    catch( const SAXException& )
    {
        return sal_False;
    }
    catch( const ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

sal_Bool StatusBarConfiguration::StoreStatusBar(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XOutputStream >& xOutputStream,
    const Reference< XIndexAccess >& rStatusbarConfiguration )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );
    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( xOutputStream );

    try
    {
        OWriteStatusBarDocumentHandler aWriteStatusBarDocumentHandler( rStatusbarConfiguration, xWriter );
        aWriteStatusBarDocumentHandler.WriteStatusBarDocument();
        return sal_True;
    }
    catch ( const RuntimeException& )
    {
        return sal_False;
    }
    catch ( const SAXException& )
    {
        return sal_False;
    }
    catch ( const ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
