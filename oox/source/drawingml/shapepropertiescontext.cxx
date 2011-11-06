/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/drawingml/shapepropertiescontext.hxx"

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/transform2dcontext.hxx"
#include "oox/drawingml/customshapegeometry.hxx"

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
    case A_TOKEN( xfrm ):
        xRet.set( new Transform2DContext( *this, xAttribs, mrShape ) );
        break;

    // GeometryGroup
    case A_TOKEN( custGeom ):   // custom geometry "CT_CustomGeometry2D"
        xRet.set( new CustomShapeGeometryContext( *this, xAttribs, *(mrShape.getCustomShapeProperties()) ) );
        break;


    case A_TOKEN( prstGeom ):   // preset geometry "CT_PresetGeometry2D"
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

    case A_TOKEN( prstTxWarp ):
        xRet.set( new PresetTextShapeContext( *this, xAttribs, *(mrShape.getCustomShapeProperties()) ) );
        break;

    // CT_LineProperties
    case A_TOKEN( ln ):
        xRet.set( new LinePropertiesContext( *this, xAttribs, mrShape.getLineProperties() ) );
        break;

    // EffectPropertiesGroup
    // todo not supported by core
    case A_TOKEN( effectLst ):  // CT_EffectList
    case A_TOKEN( effectDag ):  // CT_EffectContainer
        break;

    // todo
    case A_TOKEN( scene3d ):    // CT_Scene3D
    case A_TOKEN( sp3d ):       // CT_Shape3D
        break;
    }

    // FillPropertiesGroupContext
    if( !xRet.is() )
        xRet.set( FillPropertiesContext::createFillContext( *this, aElementToken, xAttribs, mrShape.getFillProperties() ) );

    return xRet;
}

} }
