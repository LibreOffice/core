/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapestylecontext.cxx,v $
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

#include "oox/drawingml/shapestylecontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// -------------------------
// CT_StyleMatrixReference
// -------------------------
class StyleMatrixReferenceContext : public ::oox::core::Context
{
public:
    StyleMatrixReferenceContext( const ::oox::core::ContextRef& rxParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs,
            const oox::drawingml::ShapeStyle eShapeStyle, oox::drawingml::Shape& rShape );
    ~StyleMatrixReferenceContext();

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    ::oox::drawingml::Shape&        mrShape;
    ::oox::drawingml::ShapeStyle    meShapeStyle;
    ::oox::drawingml::ColorPtr      maColor;
};

StyleMatrixReferenceContext::StyleMatrixReferenceContext( const ::oox::core::ContextRef& rxParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
    const oox::drawingml::ShapeStyle eShapeStyle, oox::drawingml::Shape& rShape )
: Context( rxParent->getHandler() )
, mrShape( rShape )
, meShapeStyle( eShapeStyle )
, maColor( new Color() )
{
    mrShape.getShapeStylesIndex()[ meShapeStyle ] = rxAttribs->getOptionalValue( XML_idx );
}

StyleMatrixReferenceContext::~StyleMatrixReferenceContext()
{

}

// --------------------------------------------------------------------

void StyleMatrixReferenceContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
    mrShape.getShapeStylesColor()[ meShapeStyle ] = maColor;
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > StyleMatrixReferenceContext::createFastChildContext( sal_Int32 /* aElementToken */, const Reference< XFastAttributeList >& /* rxAttributes */ ) throw (SAXException, RuntimeException)
{
    return new colorChoiceContext( getHandler(), *maColor.get() );
}

// ---------------
// CT_ShapeStyle
// ---------------
ShapeStyleContext::ShapeStyleContext( const ::oox::core::ContextRef& rxParent, oox::drawingml::Shape& rShape )
: Context( rxParent->getHandler() )
, mrShape( rShape )
{
}

ShapeStyleContext::~ShapeStyleContext()
{
}

// --------------------------------------------------------------------

void ShapeStyleContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > ShapeStyleContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rxAttributes )
    throw ( SAXException, RuntimeException )
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_lnRef :     // CT_StyleMatrixReference
            xRet.set( new StyleMatrixReferenceContext( this, rxAttributes, oox::drawingml::SHAPESTYLE_ln, mrShape ) );
            break;
        case NMSP_DRAWINGML|XML_fillRef :   // CT_StyleMatrixReference
            xRet.set( new StyleMatrixReferenceContext( this, rxAttributes, oox::drawingml::SHAPESTYLE_fill, mrShape ) );
            break;
        case NMSP_DRAWINGML|XML_effectRef : // CT_StyleMatrixReference
            xRet.set( new StyleMatrixReferenceContext( this, rxAttributes, oox::drawingml::SHAPESTYLE_effect, mrShape ) );
            break;
        case NMSP_DRAWINGML|XML_fontRef :   // CT_FontReference
            xRet.set( new StyleMatrixReferenceContext( this, rxAttributes, oox::drawingml::SHAPESTYLE_font, mrShape ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

