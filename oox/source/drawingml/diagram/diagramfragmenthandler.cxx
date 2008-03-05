/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagramfragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:38:51 $
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

#include <osl/diagnose.h>

#include "oox/drawingml/diagram/diagramfragmenthandler.hxx"
#include "oox/drawingml/diagram/datamodelcontext.hxx"
#include "oox/core/namespaces.hxx"
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
    case NMSP_DIAGRAM|XML_dataModel:
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
    case NMSP_DIAGRAM|XML_layoutDef:
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
    case NMSP_DIAGRAM|XML_styleDef:
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
    case NMSP_DIAGRAM|XML_colorsDef:
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
