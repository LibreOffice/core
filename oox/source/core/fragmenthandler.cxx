/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fragmenthandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:50 $
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

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

namespace oox {
namespace core {

// ============================================================================

FragmentHandler::FragmentHandler( const XmlFilterRef& rxFilter, const OUString& rFragmentPath ) :
    mxFilter( rxFilter ),
    mxRelations( rxFilter->importRelations( rFragmentPath ) ),
    maFragmentPath( rFragmentPath ),
    mpParser( 0 )
{
}

FragmentHandler::FragmentHandler( const XmlFilterRef& rxFilter, const OUString& rFragmentPath, RelationsRef xRelations ) :
    mxFilter( rxFilter ),
    mxRelations( xRelations ),
    maFragmentPath( rFragmentPath ),
    mpParser( 0 )
{
}

OUString FragmentHandler::getFragmentPathFromTarget( const OUString& rTarget ) const
{
    return Relations::getFragmentPathFromTarget( maFragmentPath, rTarget );
}

OUString FragmentHandler::getFragmentPathFromRelId( const OUString& rRelId ) const
{
    return mxRelations->getFragmentPathFromRelId( maFragmentPath, rRelId );
}

OUString FragmentHandler::getFragmentPathFromType( const OUString& rType ) const
{
    return mxRelations->getFragmentPathFromType( maFragmentPath, rType );
}

void FragmentHandler::setRecordParser( RecordParser& rParser )
{
    mpParser = &rParser;
}

RecordParser& FragmentHandler::getRecordParser()
{
    OSL_ENSURE( mpParser, "FragmentHandler::getRecordParser - not in binary import mode" );
    return *mpParser;
}

// com.sun.star.xml.sax.XFastDocumentHandler interface ------------------------

void FragmentHandler::startDocument(  ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::endDocument(  ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::setDocumentLocator( const Reference< XLocator >& xLocator ) throw (SAXException, RuntimeException)
{
    mxLocator = xLocator;
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

void FragmentHandler::startFastElement( ::sal_Int32, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::startUnknownElement( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::endFastElement( ::sal_Int32 ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::endUnknownElement( const OUString&, const OUString& ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > FragmentHandler::createFastChildContext( ::sal_Int32, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    return Reference< XFastContextHandler >();
}

Reference< XFastContextHandler > FragmentHandler::createUnknownChildContext( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    return Reference< XFastContextHandler >();
}

void FragmentHandler::characters( const OUString& ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::ignorableWhitespace( const OUString& ) throw (SAXException, RuntimeException)
{
}

void FragmentHandler::processingInstruction( const OUString&, const OUString& ) throw (SAXException, RuntimeException)
{
}

// ============================================================================

} // namespace core
} // namespace oox

