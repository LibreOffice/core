/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: clrschemecontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:51 $
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

#include "oox/drawingml/clrschemecontext.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

static void setClrMap( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes,
            oox::drawingml::ClrMap& rClrMap, sal_Int32 nToken )
{
    if ( xAttributes->hasAttribute( nToken ) )
    {
        sal_Int32 nMappedToken = xAttributes->getOptionalValueToken( nToken, 0 );
        rClrMap.setColorMap( nToken, nMappedToken );
    }
}

clrMapContext::clrMapContext( const FragmentHandlerRef& xHandler,
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes, oox::drawingml::ClrMap& rClrMap )
: Context( xHandler )
{
    setClrMap( xAttributes, rClrMap, XML_bg1 );
    setClrMap( xAttributes, rClrMap, XML_tx1 );
    setClrMap( xAttributes, rClrMap, XML_bg2 );
    setClrMap( xAttributes, rClrMap, XML_tx2 );
    setClrMap( xAttributes, rClrMap, XML_accent1 );
    setClrMap( xAttributes, rClrMap, XML_accent2 );
    setClrMap( xAttributes, rClrMap, XML_accent3 );
    setClrMap( xAttributes, rClrMap, XML_accent4 );
    setClrMap( xAttributes, rClrMap, XML_accent5 );
    setClrMap( xAttributes, rClrMap, XML_accent6 );
    setClrMap( xAttributes, rClrMap, XML_hlink );
    setClrMap( xAttributes, rClrMap, XML_folHlink );
}

Reference< XFastContextHandler > clrMapContext::createFastChildContext( sal_Int32 /* aElementToken */, const Reference< XFastAttributeList >& /* xAttribs */ )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    return xRet;
}

clrSchemeContext::clrSchemeContext( const ::oox::core::FragmentHandlerRef& xHandler, const oox::drawingml::ClrSchemePtr pClrSchemePtr )
: Context( xHandler )
, mpClrSchemePtr( pClrSchemePtr )
{
}

void clrSchemeContext::startFastElement( sal_Int32 /* aElementToken */, const Reference< XFastAttributeList >& /* xAttribs */ ) throw (SAXException, RuntimeException)
{
}

void clrSchemeContext::endFastElement( sal_Int32 aElementToken ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_dk1:
        case NMSP_DRAWINGML|XML_lt1:
        case NMSP_DRAWINGML|XML_dk2:
        case NMSP_DRAWINGML|XML_lt2:
        case NMSP_DRAWINGML|XML_accent1:
        case NMSP_DRAWINGML|XML_accent2:
        case NMSP_DRAWINGML|XML_accent3:
        case NMSP_DRAWINGML|XML_accent4:
        case NMSP_DRAWINGML|XML_accent5:
        case NMSP_DRAWINGML|XML_accent6:
        case NMSP_DRAWINGML|XML_hlink:
        case NMSP_DRAWINGML|XML_folHlink:
        {
            mpClrSchemePtr->setColor( aElementToken & 0xffff, maColor.getColor( *getHandler()->getFilter().get() ) );
            break;
        }
    }
}

Reference< XFastContextHandler > clrSchemeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& /* xAttribs */ ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_scrgbClr:   // CT_ScRgbColor
        case NMSP_DRAWINGML|XML_srgbClr:    // CT_SRgbColor
        case NMSP_DRAWINGML|XML_hslClr: // CT_HslColor
        case NMSP_DRAWINGML|XML_sysClr: // CT_SystemColor
//      case NMSP_DRAWINGML|XML_schemeClr:  // CT_SchemeColor
        case NMSP_DRAWINGML|XML_prstClr:    // CT_PresetColor
        {
            xRet.set( new colorChoiceContext( getHandler(), maColor ) );
            break;
        }
    }
    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }
