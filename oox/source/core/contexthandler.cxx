/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexthandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:12:18 $
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

#include "oox/core/contexthandler.hxx"
#include "oox/core/fragmenthandler.hxx"

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

ContextHandler::ContextHandler( ContextHandler& rParent ) :
    ContextHandlerImplBase( rParent ),
    mxBaseData( rParent.mxBaseData ),
    mpParentHandler( &rParent )
{
}

ContextHandler::ContextHandler( const FragmentBaseDataRef& rxBaseData ) :
    mxBaseData( rxBaseData ),
    mpParentHandler( 0 )
{
}

ContextHandler::~ContextHandler()
{
}

XmlFilterBase& ContextHandler::getFilter() const
{
    return mxBaseData->mrFilter;
}

ContextHandler* ContextHandler::getParentHandler() const
{
    return mpParentHandler;
}

const Relations& ContextHandler::getRelations() const
{
    return *mxBaseData->mxRelations;
}

const OUString& ContextHandler::getFragmentPath() const
{
    return mxBaseData->maFragmentPath;
}

OUString ContextHandler::getFragmentPathFromTarget( const OUString& rTarget ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromTarget( rTarget );
}

OUString ContextHandler::getFragmentPathFromRelId( const OUString& rRelId ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromRelId( rRelId );
}

OUString ContextHandler::getFragmentPathFromType( const OUString& rType ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromType( rType );
}

void ContextHandler::implSetLocator( const Reference< XLocator >& rxLocator )
{
    mxBaseData->mxLocator = rxLocator;
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

void ContextHandler::startFastElement( sal_Int32, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
}

void ContextHandler::startUnknownElement( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
}

void ContextHandler::endFastElement( sal_Int32 ) throw( SAXException, RuntimeException )
{
}

void ContextHandler::endUnknownElement( const OUString&, const OUString& ) throw( SAXException, RuntimeException )
{
}

Reference< XFastContextHandler > ContextHandler::createFastChildContext( sal_Int32, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
    return 0;
}

Reference< XFastContextHandler > ContextHandler::createUnknownChildContext( const OUString&, const OUString&, const Reference< XFastAttributeList >& ) throw( SAXException, RuntimeException )
{
    return 0;
}

void ContextHandler::characters( const OUString& ) throw( SAXException, RuntimeException )
{
}

void ContextHandler::ignorableWhitespace( const OUString& ) throw( SAXException, RuntimeException )
{
}

void ContextHandler::processingInstruction( const OUString&, const OUString& ) throw( SAXException, RuntimeException )
{
}

// record context interface ---------------------------------------------------

ContextHandlerRef ContextHandler::createRecordContext( sal_Int32, RecordInputStream& )
{
    return 0;
}

void ContextHandler::startRecord( sal_Int32, RecordInputStream& )
{
}

void ContextHandler::endRecord( sal_Int32 )
{
}

// ============================================================================

} // namespace core
} // namespace oox

