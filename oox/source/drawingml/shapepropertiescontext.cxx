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

#include "oox/drawingml/shapepropertiescontext.hxx"

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/core/namespaces.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/transform2dcontext.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// ====================================================================

// CT_ShapeProperties
ShapePropertiesContext::ShapePropertiesContext( ContextHandler& rParent, Shape& rShape )
: ContextHandler( rParent )
, mrShape( rShape )
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > ShapePropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    // CT_Transform2D
    case NMSP_DRAWINGML|XML_xfrm:
        xRet.set( new Transform2DContext( *this, xAttribs, mrShape ) );
        break;

    // GeometryGroup
    case NMSP_DRAWINGML|XML_custGeom:	// custom geometry "CT_CustomGeometry2D"
        xRet.set( new CustomShapeGeometryContext( *this, xAttribs, *(mrShape.getCustomShapeProperties()) ) );
        break;


    case NMSP_DRAWINGML|XML_prstGeom:	// preset geometry "CT_PresetGeometry2D"
        {
            sal_Int32 nToken = xAttribs->getOptionalValueToken( XML_prst, 0 );
            if ( nToken == XML_line )
            {
                static const OUString sLineShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.LineShape" ) );
                mrShape.getServiceName() = sLineShape;
            }
            xRet.set( new PresetShapeGeometryContext( *this, xAttribs, *(mrShape.getCustomShapeProperties()) ) );
        }
        break;

    case NMSP_DRAWINGML|XML_prstTxWarp:
        xRet.set( new PresetTextShapeContext( *this, xAttribs, *(mrShape.getCustomShapeProperties()) ) );
        break;

    // CT_LineProperties
    case NMSP_DRAWINGML|XML_ln:
        xRet.set( new LinePropertiesContext( *this, xAttribs, mrShape.getLineProperties() ) );
        break;

    // EffectPropertiesGroup
    // todo not supported by core
    case NMSP_DRAWINGML|XML_effectLst:	// CT_EffectList
    case NMSP_DRAWINGML|XML_effectDag:	// CT_EffectContainer
        break;

    // todo
    case NMSP_DRAWINGML|XML_scene3d:	// CT_Scene3D
    case NMSP_DRAWINGML|XML_sp3d:		// CT_Shape3D
        break;
    }

    // FillPropertiesGroupContext
    if( !xRet.is() )
        xRet.set( FillPropertiesContext::createFillContext( *this, aElementToken, xAttribs, mrShape.getFillProperties() ) );

    return xRet;
}

} }
