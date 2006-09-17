/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLBasicExportFilter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:19:49 $
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

#ifndef _XMLOFF_XMLBASICEXPORTFILTER_HXX
#include "XMLBasicExportFilter.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


// =============================================================================
// XMLBasicExportFilter
// =============================================================================

XMLBasicExportFilter::XMLBasicExportFilter( const Reference< xml::sax::XDocumentHandler >& rxHandler )
    :m_xHandler( rxHandler )
{
}

// -----------------------------------------------------------------------------

XMLBasicExportFilter::~XMLBasicExportFilter()
{
}

// -----------------------------------------------------------------------------
// XDocumentHandler
// -----------------------------------------------------------------------------

void XMLBasicExportFilter::startDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // do nothing, filter this
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // do nothing, filter this
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::startElement( const ::rtl::OUString& aName,
        const Reference< xml::sax::XAttributeList >& xAttribs )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->startElement( aName, xAttribs );
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::endElement( const ::rtl::OUString& aName )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->endElement( aName );
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::characters( const ::rtl::OUString& aChars )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->characters( aChars );
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->ignorableWhitespace( aWhitespaces );
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::processingInstruction( const ::rtl::OUString& aTarget,
        const ::rtl::OUString& aData )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->processingInstruction( aTarget, aData );
}

// -----------------------------------------------------------------------------

void XMLBasicExportFilter::setDocumentLocator( const Reference< xml::sax::XLocator >& xLocator )
    throw (xml::sax::SAXException, RuntimeException)
{
    if ( m_xHandler.is() )
        m_xHandler->setDocumentLocator( xLocator );
}

// -----------------------------------------------------------------------------
