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

#include "XMLBasicExportFilter.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//.........................................................................
namespace binfilter
{
//.........................................................................

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

//.........................................................................
}	// namespace binfilter
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
