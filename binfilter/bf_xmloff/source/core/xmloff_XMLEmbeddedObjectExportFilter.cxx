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

#include "XMLEmbeddedObjectExportFilter.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


XMLEmbeddedObjectExportFilter::XMLEmbeddedObjectExportFilter(
        const Reference< XDocumentHandler > & rHandler ) throw() :
    xHandler( rHandler ),
    xExtHandler( rHandler, UNO_QUERY )
{
}

XMLEmbeddedObjectExportFilter::~XMLEmbeddedObjectExportFilter () throw()
{
}


void SAL_CALL XMLEmbeddedObjectExportFilter::startDocument( void )
    throw( SAXException, RuntimeException )
{
    // do nothing, filter this
}

void SAL_CALL XMLEmbeddedObjectExportFilter::endDocument( void )
    throw( SAXException, RuntimeException)
{
    // do nothing, filter this
}

void SAL_CALL XMLEmbeddedObjectExportFilter::startElement(
        const OUString& rName,
        const Reference< XAttributeList >& xAttrList )
    throw(SAXException, RuntimeException)
{
    xHandler->startElement( rName, xAttrList );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::endElement( const OUString& rName )
    throw(SAXException, RuntimeException)
{
    xHandler->endElement( rName );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::characters( const OUString& rChars )
    throw(SAXException, RuntimeException)
{
    xHandler->characters( rChars );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::ignorableWhitespace(
        const OUString& rWhitespaces )
    throw(SAXException, RuntimeException)
{
    xHandler->ignorableWhitespace( rWhitespaces );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::processingInstruction(
        const OUString& rTarget,
        const OUString& rData )
    throw(SAXException, RuntimeException)
{
    xHandler->processingInstruction( rTarget, rData );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::setDocumentLocator(
        const Reference< XLocator >& rLocator )
    throw(SAXException, RuntimeException)
{
    xHandler->setDocumentLocator( rLocator );
}

// XExtendedDocumentHandler
void SAL_CALL XMLEmbeddedObjectExportFilter::startCDATA( void )
    throw(SAXException, RuntimeException)
{
    if( xExtHandler.is() )
        xExtHandler->startCDATA();
}

void SAL_CALL XMLEmbeddedObjectExportFilter::endCDATA( void )
    throw(RuntimeException)
{
    if( xExtHandler.is() )
        xExtHandler->endCDATA();
}

void SAL_CALL XMLEmbeddedObjectExportFilter::comment( const OUString& rComment )
    throw(SAXException, RuntimeException)
{
    if( xExtHandler.is() )
        xExtHandler->comment( rComment );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::allowLineBreak( void )
    throw(SAXException, RuntimeException)
{
    if( xExtHandler.is() )
        xExtHandler->allowLineBreak();
}

void SAL_CALL XMLEmbeddedObjectExportFilter::unknown( const OUString& rString )
    throw(SAXException, RuntimeException)
{
    if( xExtHandler.is() )
        xExtHandler->unknown( rString );
}

// XInitialize
void SAL_CALL XMLEmbeddedObjectExportFilter::initialize(
        const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
    const sal_Int32 nAnyCount = aArguments.getLength();
    const Any* pAny = aArguments.getConstArray();

    for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
    {
        if( pAny->getValueType() ==
                ::getCppuType((const Reference< XDocumentHandler >*)0))
        {
            *pAny >>= xHandler;
            *pAny >>= xExtHandler;
        }
    }
}

// XServiceInfo
OUString SAL_CALL XMLEmbeddedObjectExportFilter::getImplementationName()
    throw(RuntimeException)
{
    OUString aStr;
    return aStr;
}

sal_Bool SAL_CALL XMLEmbeddedObjectExportFilter::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return sal_False;
}

Sequence< OUString > SAL_CALL XMLEmbeddedObjectExportFilter::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Sequence< OUString > aSeq;
    return aSeq;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
