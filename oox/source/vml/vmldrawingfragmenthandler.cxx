/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vmldrawingfragmenthandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:07 $
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

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "oox/core/context.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/vml/drawingfragmenthandler.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace vml {

//--------------------------------------------------------------------------------------------------------------

class BasicShapeContext : public oox::core::Context
{
public:
    BasicShapeContext( const FragmentHandlerRef& xHandler,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, const ShapePtr pShapePtr );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, RuntimeException);
private:
    ShapePtr mpShapePtr;
};
BasicShapeContext::BasicShapeContext( const FragmentHandlerRef& xHandler,
    sal_Int32 /* aElement */, const Reference< XFastAttributeList >& xAttribs, const ShapePtr pShapePtr )
: Context( xHandler )
, mpShapePtr( pShapePtr )
{
    mpShapePtr->msId = xAttribs->getOptionalValue( XML_id );
    mpShapePtr->msType = xAttribs->getOptionalValue( XML_type );
    rtl::OUString aShapeType( xAttribs->getOptionalValue( NMSP_OFFICE|XML_spt ) );
    if ( aShapeType.getLength() )
        mpShapePtr->mnShapeType = aShapeType.toInt32();
    rtl::OUString aCoordSize( xAttribs->getOptionalValue( XML_coordsize ) );
    if ( aCoordSize.getLength() )
    {
        sal_Int32 nIndex = 0;
        rtl::OUString aCoordWidth ( aCoordSize.getToken( 0, ',', nIndex ) );
        rtl::OUString aCoordHeight( aCoordSize.getToken( 0, ',', nIndex ) );
        if ( aCoordWidth.getLength() )
            mpShapePtr->mnCoordWidth = aCoordWidth.toInt32();
        if ( aCoordHeight.getLength() )
            mpShapePtr->mnCoordHeight = aCoordHeight.toInt32();
    }
    mpShapePtr->msPath = xAttribs->getOptionalValue( XML_path );
    mpShapePtr->mnStroked = xAttribs->getOptionalValueToken( XML_stroked, 0 );
    mpShapePtr->mnFilled = xAttribs->getOptionalValueToken( XML_filled, 0 );
}
Reference< XFastContextHandler > BasicShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_VML|XML_imagedata:
            {
                OUString aRelId( xAttribs->getOptionalValue( NMSP_OFFICE|XML_relid ) );
                mpShapePtr->msGraphicURL = getHandler()->getFragmentPathFromRelId( aRelId );
                mpShapePtr->msImageTitle = xAttribs->getOptionalValue( NMSP_OFFICE|XML_title );
            }
            break;
        default:
            break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

//--------------------------------------------------------------------------------------------------------------
// CT_Shapetype
class ShapeTypeContext : public BasicShapeContext
{
public:
    ShapeTypeContext( const FragmentHandlerRef& xHandler,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, const ShapePtr pShapePtr );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);

};
ShapeTypeContext::ShapeTypeContext( const FragmentHandlerRef& xHandler,
    sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, const ShapePtr pShapePtr )
: BasicShapeContext( xHandler, aElement, xAttribs, pShapePtr )
{
}
Reference< XFastContextHandler > ShapeTypeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
//  switch( aElementToken )
//  {
//      default:
            xRet = BasicShapeContext::createFastChildContext( aElementToken, xAttribs );
//      break;
//  }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

//--------------------------------------------------------------------------------------------------------------
// CT_Shape
class ShapeContext : public BasicShapeContext
{
public:
    ShapeContext( const FragmentHandlerRef& xHandler,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, const ShapePtr pShapePtr );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
};
ShapeContext::ShapeContext( const FragmentHandlerRef& xHandler,
    sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, const ShapePtr pShapePtr )
: BasicShapeContext( xHandler, aElement, xAttribs, pShapePtr )
{
}
Reference< XFastContextHandler > ShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
//  switch( aElementToken )
//  {
//      default:
            xRet = BasicShapeContext::createFastChildContext( aElementToken, xAttribs );
//      break;
//  }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

//--------------------------------------------------------------------------------------------------------------

DrawingFragmentHandler::DrawingFragmentHandler( const XmlFilterRef& xFilter, const OUString& rFragmentPath, const DrawingPtr pDrawingPtr )
    throw()
: FragmentHandler( xFilter, rFragmentPath )
, mpDrawingPtr( pDrawingPtr )
{
}
DrawingFragmentHandler::~DrawingFragmentHandler()
    throw()
{
}
Reference< XFastContextHandler > DrawingFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case XML_xml:
        break;
        case NMSP_OFFICE|XML_shapelayout:
        break;
        case NMSP_VML|XML_shapetype:
            {
                ShapePtr pShapePtr( new Shape );
                xRet = new ShapeTypeContext( this, aElementToken, xAttribs, pShapePtr );
                mpDrawingPtr->getShapeTypes().push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_shape:
            {
                ShapePtr pShapePtr( new Shape );
                xRet = new ShapeContext( this, aElementToken, xAttribs, pShapePtr );
                mpDrawingPtr->getShapes().push_back( pShapePtr );
            }
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

void SAL_CALL DrawingFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{
}

//--------------------------------------------------------------------------------------------------------------



} }

