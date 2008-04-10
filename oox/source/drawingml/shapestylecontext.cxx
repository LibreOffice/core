/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shapestylecontext.cxx,v $
 * $Revision: 1.4 $
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
class StyleMatrixReferenceContext : public ContextHandler
{
public:
    StyleMatrixReferenceContext( ContextHandler& rParent, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs,
            const ShapeStyle eShapeStyle, Shape& rShape );
    ~StyleMatrixReferenceContext();

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    Shape&              mrShape;
    ShapeStyle          meShapeStyle;
    ColorPtr            maColor;
};

StyleMatrixReferenceContext::StyleMatrixReferenceContext( ContextHandler& rParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
    const ShapeStyle eShapeStyle, Shape& rShape )
: ContextHandler( rParent )
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
    return new colorChoiceContext( *this, *maColor );
}

// ---------------
// CT_ShapeStyle
// ---------------
ShapeStyleContext::ShapeStyleContext( ContextHandler& rParent, Shape& rShape )
: ContextHandler( rParent )
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
            xRet.set( new StyleMatrixReferenceContext( *this, rxAttributes, SHAPESTYLE_ln, mrShape ) );
            break;
        case NMSP_DRAWINGML|XML_fillRef :   // CT_StyleMatrixReference
            xRet.set( new StyleMatrixReferenceContext( *this, rxAttributes, SHAPESTYLE_fill, mrShape ) );
            break;
        case NMSP_DRAWINGML|XML_effectRef : // CT_StyleMatrixReference
            xRet.set( new StyleMatrixReferenceContext( *this, rxAttributes, SHAPESTYLE_effect, mrShape ) );
            break;
        case NMSP_DRAWINGML|XML_fontRef :   // CT_FontReference
            xRet.set( new StyleMatrixReferenceContext( *this, rxAttributes, SHAPESTYLE_font, mrShape ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

