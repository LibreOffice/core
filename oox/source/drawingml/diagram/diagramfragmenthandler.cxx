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

#include <osl/diagnose.h>

#include "oox/drawingml/diagram/diagramfragmenthandler.hxx"
#include "oox/drawingml/diagram/datamodelcontext.hxx"
#include "diagramdefinitioncontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace oox { namespace drawingml {

DiagramDataFragmentHandler::DiagramDataFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramDataPtr pDataPtr )
    throw( )
    : FragmentHandler( rFilter, rFragmentPath )
    , mpDataPtr( pDataPtr )
{
}

DiagramDataFragmentHandler::~DiagramDataFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramDataFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramDataFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                    const Reference< XFastAttributeList >& )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DGM_TOKEN( dataModel ):
        xRet.set( new DataModelContext( *this, mpDataPtr ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

///////////////////

DiagramLayoutFragmentHandler::DiagramLayoutFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramLayoutPtr pDataPtr )
    throw( )
    : FragmentHandler( rFilter, rFragmentPath )
    , mpDataPtr( pDataPtr )
{
}

DiagramLayoutFragmentHandler::~DiagramLayoutFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramLayoutFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramLayoutFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                      const Reference< XFastAttributeList >& xAttribs )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DGM_TOKEN( layoutDef ):
        xRet.set( new DiagramDefinitionContext( *this, xAttribs, mpDataPtr ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

///////////////////////

DiagramQStylesFragmentHandler::DiagramQStylesFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramQStylesPtr pDataPtr )
    throw( )
    : FragmentHandler( rFilter, rFragmentPath )
    , mpDataPtr( pDataPtr )
{
}

DiagramQStylesFragmentHandler::~DiagramQStylesFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramQStylesFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramQStylesFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                    const Reference< XFastAttributeList >& )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DGM_TOKEN( styleDef ):
        // TODO
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

/////////////////////

DiagramColorsFragmentHandler::DiagramColorsFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramColorsPtr pDataPtr )
    throw( )
    : FragmentHandler( rFilter, rFragmentPath )
    , mpDataPtr( pDataPtr )
{
}

DiagramColorsFragmentHandler::~DiagramColorsFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramColorsFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{

}


Reference< XFastContextHandler > SAL_CALL
DiagramColorsFragmentHandler::createFastChildContext( ::sal_Int32 aElement,
                                                    const Reference< XFastAttributeList >& )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DGM_TOKEN( colorsDef ):
        // TODO
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}




} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
