/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLEmbeddedObjectExportFilter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:28:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX
#include "XMLEmbeddedObjectExportFilter.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


XMLEmbeddedObjectExportFilter::XMLEmbeddedObjectExportFilter() throw()
{
}

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

sal_Bool SAL_CALL XMLEmbeddedObjectExportFilter::supportsService( const OUString& )
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
