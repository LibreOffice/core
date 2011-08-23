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
    ContextHandlerImplBase(),
    mxBaseData( rParent.mxBaseData )
{
}

ContextHandler::ContextHandler( const FragmentBaseDataRef& rxBaseData ) :
    mxBaseData( rxBaseData )
{
}

ContextHandler::~ContextHandler()
{
}

XmlFilterBase& ContextHandler::getFilter() const
{
    return mxBaseData->mrFilter;
}

const Relations& ContextHandler::getRelations() const
{
    return *mxBaseData->mxRelations;
}

const OUString& ContextHandler::getFragmentPath() const
{
    return mxBaseData->maFragmentPath;
}

OUString ContextHandler::getFragmentPathFromRelation( const Relation& rRelation ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromRelation( rRelation );
}

OUString ContextHandler::getFragmentPathFromRelId( const OUString& rRelId ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromRelId( rRelId );
}

OUString ContextHandler::getFragmentPathFromFirstType( const OUString& rType ) const
{
    return mxBaseData->mxRelations->getFragmentPathFromFirstType( rType );
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

