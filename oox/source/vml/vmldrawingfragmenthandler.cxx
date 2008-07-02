/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmldrawingfragmenthandler.cxx,v $
 * $Revision: 1.6 $
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

#include "oox/vml/drawingfragmenthandler.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/contexthandler.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace vml {

static sal_Int32 getMeasure( const rtl::OUString& rVal )
{
    double fVal = rVal.toDouble();
    const sal_Int32 nLen = rVal.getLength();
    if ( nLen >= 2 )
    {
        switch( static_cast< sal_Int8 >( rVal[ nLen - 1 ] ) )
        {
            case 'n' : fVal *= 2540; break;
            case 'm' : rVal[ nLen - 2 ] == 'm' ? fVal *= 100.0 : fVal *= 1000.0; break;
            case 't' : fVal *= 2540.0 / 72.0; break;
            case 'c' : fVal *= ( 2540.0 / 72.0 ) * 12.0; break;
        }
    }
    return static_cast< sal_Int32 >( fVal );
}

// AG_CoreAttributes
static void ApplyCoreAttributes( const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
{
    // AG_Id
    rShape.msId = xAttribs->getOptionalValue( XML_id );

    // AG_Style
    if ( xAttribs->hasAttribute( XML_style ) )
    {
        rtl::OUString sStyle( xAttribs->getOptionalValue( XML_style ) );
        sal_Int32 nIndex = 0;
        do
        {
            OUString aStyleToken( sStyle.getToken( 0, ';', nIndex ) );
            if ( aStyleToken.getLength() )
            {
                sal_Int32 nIndex2 = 0;
                OUString aName( aStyleToken.getToken( 0, ':', nIndex2 ) );
                OUString aVal ( aStyleToken.getToken( 0, ':', nIndex2 ) );
                if ( aName.getLength() && aVal.getLength() )
                {
                    static const ::rtl::OUString sPosition( RTL_CONSTASCII_USTRINGPARAM( "position" ) );
                    static const ::rtl::OUString sLeft( RTL_CONSTASCII_USTRINGPARAM( "left" ) );
                    static const ::rtl::OUString sTop( RTL_CONSTASCII_USTRINGPARAM( "top" ) );
                    static const ::rtl::OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "width" ) );
                    static const ::rtl::OUString sHeight( RTL_CONSTASCII_USTRINGPARAM( "height" ) );
                    static const ::rtl::OUString sMarginLeft( RTL_CONSTASCII_USTRINGPARAM( "margin-left" ) );
                    static const ::rtl::OUString sMarginTop( RTL_CONSTASCII_USTRINGPARAM( "margin-top" ) );
                    if ( aName == sPosition )
                        rShape.msPosition = aVal;
                    else if ( aName == sLeft )
                        rShape.maPosition.X = getMeasure( aVal );
                    else if ( aName == sTop )
                        rShape.maPosition.Y = getMeasure( aVal );
                    else if ( aName == sWidth )
                        rShape.maSize.Width = getMeasure( aVal );
                    else if ( aName == sHeight )
                        rShape.maSize.Height = getMeasure( aVal );
                    else if ( aName == sMarginLeft )
                        rShape.maPosition.X = getMeasure( aVal );
                    else if ( aName == sMarginTop )
                        rShape.maPosition.Y = getMeasure( aVal );
                }
            }
        }
        while ( nIndex >= 0 );
    }

    // href
    // target
    // class
    // title
    // alt

    // coordsize
    rtl::OUString aCoordSize( xAttribs->getOptionalValue( XML_coordsize ) );
    if ( aCoordSize.getLength() )
    {
        sal_Int32 nIndex = 0;
        rtl::OUString aCoordWidth ( aCoordSize.getToken( 0, ',', nIndex ) );
        rtl::OUString aCoordHeight( aCoordSize.getToken( 0, ',', nIndex ) );
        if ( aCoordWidth.getLength() )
            rShape.mnCoordWidth = aCoordWidth.toInt32();
        if ( aCoordHeight.getLength() )
            rShape.mnCoordHeight = aCoordHeight.toInt32();
    }

    // coordorigin
    // wrapcoords
    // print
}

// AG_ShapeAttributes
static void ApplyShapeAttributes( const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
{
    AttributeList aAttributeList( xAttribs );
    rShape.mnStroked = xAttribs->getOptionalValueToken( XML_stroked, 0 );
    if ( xAttribs->hasAttribute( XML_filled ) )
        rShape.moFilled = ::boost::optional< sal_Bool >( aAttributeList.getBool( XML_filled, sal_False ) );
    if ( xAttribs->hasAttribute( XML_fillcolor ) )
        rShape.moFillColor = ::boost::optional< rtl::OUString >( xAttribs->getOptionalValue( XML_fillcolor ) );
}

//--------------------------------------------------------------------------------------------------------------
// EG_ShapeElements
class BasicShapeContext : public ContextHandler
{
public:
    BasicShapeContext( ContextHandler& rParent,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, RuntimeException);
private:
    Shape& mrShape;
};

BasicShapeContext::BasicShapeContext( ContextHandler& rParent,
    sal_Int32 /* aElement */, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: ContextHandler( rParent )
, mrShape( rShape )
{
    mrShape.msType = xAttribs->getOptionalValue( XML_type );
    mrShape.msShapeType = xAttribs->getOptionalValue( NMSP_OFFICE|XML_spt );
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
                mrShape.msGraphicURL = getFragmentPathFromRelId( aRelId );
                mrShape.msImageTitle = xAttribs->getOptionalValue( NMSP_OFFICE|XML_title );
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
    ShapeTypeContext( ContextHandler& rParent,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
};

ShapeTypeContext::ShapeTypeContext( ContextHandler& rParent,
    sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
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
// CT_PolyLine
class PolyLineContext : public BasicShapeContext
{
public:
    PolyLineContext( ContextHandler& rParent,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
};

PolyLineContext::PolyLineContext( ContextHandler& rParent,
    sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
{
    ApplyShapeAttributes( xAttribs, rShape );
    ApplyCoreAttributes( xAttribs, rShape );

    rtl::OUString aPoints( xAttribs->getOptionalValue( XML_points ) );
    if ( aPoints.getLength() )
    {
        std::vector< awt::Point > vPoints;
        sal_Int32 nIndex = 0;
        do
        {
            OUString aX( aPoints.getToken( 0, ',', nIndex ) );
            OUString aY( aPoints.getToken( 0, ',', nIndex ) );
            awt::Point aPt( getMeasure( aX ), getMeasure( aY ) );
            vPoints.push_back( aPt );
        }
        while ( nIndex >= 0 );

        drawing::PointSequenceSequence aPointSeq( 1 );
        aPointSeq[ 0 ] = drawing::PointSequence( &vPoints.front(), vPoints.size() );
        static const ::rtl::OUString sPolyPolygon( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) );
        rShape.maPath.Name = sPolyPolygon;
        rShape.maPath.Value <<= aPointSeq;

/* not sure if the following is needed

        // calculating the bounding box
        sal_Int32 nGlobalLeft  = SAL_MAX_INT32;
        sal_Int32 nGlobalRight = SAL_MIN_INT32;
        sal_Int32 nGlobalTop   = SAL_MAX_INT32;
        sal_Int32 nGlobalBottom= SAL_MIN_INT32;
        std::vector< awt::Point >::const_iterator aIter( vPoints.begin() );
        while( aIter != vPoints.end() )
        {
            sal_Int32 x = (*aIter).X;
            sal_Int32 y = (*aIter).Y;
            if ( nGlobalLeft > x )
                nGlobalLeft = x;
            if ( nGlobalRight < x )
                nGlobalRight = x;
            if ( nGlobalTop > y )
                nGlobalTop = y;
            if ( nGlobalBottom < y )
                nGlobalBottom = y;
            aIter++;
        }
        rShape.maPosition.X = nGlobalLeft;
        rShape.maPosition.Y = nGlobalTop;
        rShape.maSize.Width = nGlobalRight - nGlobalLeft;
        rShape.maSize.Height = nGlobalBottom - nGlobalTop;
*/
    }
}

Reference< XFastContextHandler > PolyLineContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
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
    ShapeContext( ContextHandler& rParent,
        sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
};

ShapeContext::ShapeContext( ContextHandler& rParent,
    sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
{
    ApplyShapeAttributes( xAttribs, rShape );
    ApplyCoreAttributes( xAttribs, rShape );
//  rShape.msPath = xAttribs->getOptionalValue( XML_path );
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

class GroupShapeContext : public BasicShapeContext
{
public:
    GroupShapeContext( ContextHandler& rParent, sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs,
        Shape& rShape, std::vector< ShapePtr >& rShapeTypes );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
        const Reference< XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, RuntimeException);
private:
    Shape& mrShape;
    std::vector< ShapePtr >& mrShapeTypes;
};

GroupShapeContext::GroupShapeContext( ContextHandler& rParent,
    sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs,
        Shape& rShape, std::vector< ShapePtr >& rShapeTypes )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
, mrShape( rShape )
, mrShapeTypes( rShapeTypes )
{
    AttributeList aAttributeList( xAttribs );
    if ( xAttribs->hasAttribute( XML_filled ) )
        rShape.moFilled = ::boost::optional< sal_Bool >( aAttributeList.getBool( XML_filled, sal_False ) );
    if ( xAttribs->hasAttribute( XML_fillcolor ) )
        rShape.moFillColor = ::boost::optional< rtl::OUString >( xAttribs->getOptionalValue( XML_fillcolor ) );
    ApplyCoreAttributes( xAttribs, rShape );
}
Reference< XFastContextHandler > GroupShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    return DrawingFragmentHandler::StaticCreateContext( *this, aElementToken, xAttribs, mrShape.getChilds(), mrShapeTypes );
}

//--------------------------------------------------------------------------------------------------------------

DrawingFragmentHandler::DrawingFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath,
                                                std::vector< ShapePtr >& rShapes, std::vector< ShapePtr >& rShapeTypes )
    throw()
: FragmentHandler( rFilter, rFragmentPath )
, mrShapes( rShapes )
, mrShapeTypes( rShapeTypes )
, maFragmentPath( rFragmentPath )
{
}
DrawingFragmentHandler::~DrawingFragmentHandler()
    throw()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > DrawingFragmentHandler::StaticCreateContext( oox::core::ContextHandler& rParent,
    sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
        std::vector< ShapePtr >& rShapes, std::vector< ShapePtr >& rShapeTypes )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
    static const ::rtl::OUString sCustomShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.CustomShape" ) );

    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_VML|XML_group :
            {
                static const ::rtl::OUString sGroupShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GroupShape" ) );
                ShapePtr pShapePtr( new Shape( sGroupShape ) );
                xRet = new GroupShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get(), rShapeTypes );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_shapetype :
            {
                ShapePtr pShapePtr( new Shape( OUString() ) );
                xRet = new ShapeTypeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapeTypes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_shape:
            {
                ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_oval:
            {
                static const ::rtl::OUString sEllipseShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.EllipseShape" ) );
                ShapePtr pShapePtr( new Shape( sEllipseShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_polyline:
            {
                static const ::rtl::OUString sPolyLineShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.PolyLineShape" ) );
                ShapePtr pShapePtr( new Shape( sPolyLineShape ) );
                xRet = new PolyLineContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_rect:
            {
                static const ::rtl::OUString sRectangleShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.RectangleShape" ) );
                ShapePtr pShapePtr( new Shape( sRectangleShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_roundrect:
            {
                static const ::rtl::OUString sRectangleShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.RectangleShape" ) );
                ShapePtr pShapePtr( new Shape( sRectangleShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;

        // TODO:
        case NMSP_VML|XML_arc:
            {
                ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_curve:
            {
                ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_line:
            {
                ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_OFFICE|XML_diagram:
            {
                ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
        case NMSP_VML|XML_image:
            {
                ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
                rShapes.push_back( pShapePtr );
            }
        break;
    }
    return xRet;
}


// CT_GROUP
Reference< XFastContextHandler > DrawingFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    return aElementToken == XML_xml
        ? getFastContextHandler()
        : StaticCreateContext( *this, aElementToken, xAttribs, mrShapes, mrShapeTypes );
}

void SAL_CALL DrawingFragmentHandler::endDocument()
    throw (SAXException, RuntimeException)
{
}

//--------------------------------------------------------------------------------------------------------------



} }

