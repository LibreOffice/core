/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLBasicExportFilter.hxx"

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
