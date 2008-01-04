/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saxnamespacefilter.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:24:09 $
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
#include "precompiled_framework.hxx"

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <stdio.h>

#include <xml/saxnamespacefilter.hxx>
#include <xml/attributelist.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

const OUString aXMLAttributeNamespace( RTL_CONSTASCII_USTRINGPARAM( "xmlns" ));
const OUString aXMLAttributeType( RTL_CONSTASCII_USTRINGPARAM( "CDATA" ));

namespace framework{


SaxNamespaceFilter::SaxNamespaceFilter( Reference< XDocumentHandler >& rSax1DocumentHandler ) :
    ThreadHelpBase( &Application::GetSolarMutex() ), OWeakObject(),
     m_xLocator( 0 ),
     xDocumentHandler( rSax1DocumentHandler ),
     m_nDepth( 0 )
{
}

SaxNamespaceFilter::~SaxNamespaceFilter()
{
}

Any SAL_CALL SaxNamespaceFilter::queryInterface( const Type & rType ) throw( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( XDocumentHandler*, this ));
    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

// XDocumentHandler
void SAL_CALL SaxNamespaceFilter::startDocument(void)
    throw ( SAXException, RuntimeException )
{
}

void SAL_CALL SaxNamespaceFilter::endDocument(void)
    throw(  SAXException, RuntimeException )
{
}

void SAL_CALL SaxNamespaceFilter::startElement(
    const rtl::OUString& rName, const Reference< XAttributeList > &xAttribs )
    throw(  SAXException, RuntimeException )
{
    XMLNamespaces aXMLNamespaces;
    if ( m_aNamespaceStack.size() > 0 )
        aXMLNamespaces = m_aNamespaceStack.top();

    AttributeListImpl* pNewList = new AttributeListImpl();

    // examine all namespaces for this level
    ::std::vector< sal_Int16 > aAttributeIndexes;
    {
        for ( sal_Int16 i=0; i< xAttribs->getLength(); i++ )
        {
            OUString aName = xAttribs->getNameByIndex( i );
            if ( aName.compareTo( aXMLAttributeNamespace, aXMLAttributeNamespace.getLength() ) == 0 )
                aXMLNamespaces.addNamespace( aName, xAttribs->getValueByIndex( i ));
            else
                aAttributeIndexes.push_back( i );
        }
    }

    // current namespaces for this level
    m_aNamespaceStack.push( aXMLNamespaces );

    try
    {
        // apply namespaces to all remaing attributes
        for ( ::std::vector< sal_Int16 >::const_iterator i(
                  aAttributeIndexes.begin());
              i != aAttributeIndexes.end(); ++i )
        {
            OUString aAttributeName          = xAttribs->getNameByIndex( *i );
            OUString aValue                  = xAttribs->getValueByIndex( *i );
            OUString aNamespaceAttributeName = aXMLNamespaces.applyNSToAttributeName( aAttributeName );
            pNewList->addAttribute( aNamespaceAttributeName, aXMLAttributeType, aValue );
        }
    }
    catch ( SAXException& e )
    {
        e.Message = OUString( getErrorLineString() + e.Message );
        throw e;
    }

    OUString aNamespaceElementName;

    try
    {
         aNamespaceElementName = aXMLNamespaces.applyNSToElementName( rName );
    }
    catch ( SAXException& e )
    {
        e.Message = OUString( getErrorLineString() + e.Message );
        throw e;
    }

    xDocumentHandler->startElement( aNamespaceElementName, pNewList );
}

void SAL_CALL SaxNamespaceFilter::endElement(const rtl::OUString& aName)
    throw(  SAXException, RuntimeException )
{
    XMLNamespaces& aXMLNamespaces = m_aNamespaceStack.top();
    OUString aNamespaceElementName;

    try
    {
        aNamespaceElementName = aXMLNamespaces.applyNSToElementName( aName );
    }
    catch ( SAXException& e )
    {
        e.Message = OUString( getErrorLineString() + e.Message );
        throw e;
    }

    xDocumentHandler->endElement( aNamespaceElementName );
    m_aNamespaceStack.pop();
}

void SAL_CALL SaxNamespaceFilter::characters(const rtl::OUString& aChars)
    throw(  SAXException, RuntimeException )
{
    xDocumentHandler->characters( aChars );
}

void SAL_CALL SaxNamespaceFilter::ignorableWhitespace(const rtl::OUString& aWhitespaces)
    throw(  SAXException, RuntimeException )
{
    xDocumentHandler->ignorableWhitespace( aWhitespaces );
}

void SAL_CALL SaxNamespaceFilter::processingInstruction(
    const rtl::OUString& aTarget, const rtl::OUString& aData)
    throw(  SAXException, RuntimeException )
{
    xDocumentHandler->processingInstruction( aTarget, aData );
}

void SAL_CALL SaxNamespaceFilter::setDocumentLocator(
    const Reference< XLocator > &xLocator)
    throw(  SAXException, RuntimeException )
{
    m_xLocator = xLocator;
    xDocumentHandler->setDocumentLocator( xLocator );
}

OUString SaxNamespaceFilter::getErrorLineString()
{
    char buffer[32];

    if ( m_xLocator.is() )
    {
        snprintf( buffer, sizeof(buffer), "Line: %ld - ", static_cast<long>( m_xLocator->getLineNumber() ));
        return OUString::createFromAscii( buffer );
    }
    else
        return OUString();
}

} // namespace
