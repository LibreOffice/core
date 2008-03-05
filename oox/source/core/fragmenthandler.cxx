/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:13:25 $
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

#include "oox/core/fragmenthandler.hxx"
#include "oox/core/xmlfilterbase.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::com::sun::star::xml::sax::XLocator;

namespace oox {
namespace core {

// ============================================================================

FragmentBaseData::FragmentBaseData( XmlFilterBase& rFilter, const OUString& rFragmentPath, RelationsRef xRelations ) :
    mrFilter( rFilter ),
    maFragmentPath( rFragmentPath ),
    mxRelations( xRelations )
{
}

// ============================================================================

FragmentHandler::FragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath ) :
    FragmentHandlerImplBase( FragmentBaseDataRef( new FragmentBaseData( rFilter, rFragmentPath, rFilter.importRelations( rFragmentPath ) ) ) )
{
}

FragmentHandler::FragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, RelationsRef xRelations ) :
    FragmentHandlerImplBase( FragmentBaseDataRef( new FragmentBaseData( rFilter, rFragmentPath, xRelations ) ) )
{
}

FragmentHandler::~FragmentHandler()
{
}

// com.sun.star.xml.sax.XFastDocumentHandler interface ------------------------

void FragmentHandler::startDocument() throw( SAXException, RuntimeException )
{
}

void FragmentHandler::endDocument() throw( SAXException, RuntimeException )
{
}

void FragmentHandler::setDocumentLocator( const Reference< XLocator >& rxLocator ) throw( SAXException, RuntimeException )
{
    implSetLocator( rxLocator );
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

void FragmentHandler::startFastElement( sal_Int32, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
}

void FragmentHandler::startUnknownElement( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
}

void FragmentHandler::endFastElement( sal_Int32 ) throw( SAXException, RuntimeException )
{
}

void FragmentHandler::endUnknownElement( const OUString&, const OUString& ) throw( SAXException, RuntimeException )
{
}

Reference< XFastContextHandler > FragmentHandler::createFastChildContext( sal_Int32, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
    return 0;
}

Reference< XFastContextHandler > FragmentHandler::createUnknownChildContext( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
    return 0;
}

void FragmentHandler::characters( const OUString& ) throw( SAXException, RuntimeException )
{
}

void FragmentHandler::ignorableWhitespace( const OUString& ) throw( SAXException, RuntimeException )
{
}

void FragmentHandler::processingInstruction( const OUString&, const OUString& ) throw( SAXException, RuntimeException )
{
}

// binary records -------------------------------------------------------------

const RecordInfo* FragmentHandler::getRecordInfos() const
{
    // default: no support for binary records
    return 0;
}

// ============================================================================

} // namespace core
} // namespace oox

