/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawingml/shapepropertiescontext.hxx>

#include <drawingml/scene3dcontext.hxx>
#include <drawingml/linepropertiescontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/transform2dcontext.hxx>
#include <drawingml/customshapegeometry.hxx>
#include <drawingml/effectpropertiescontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <drawingml/customshapeproperties.hxx>

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

namespace oox::drawingml {

// CT_ShapeProperties
ShapePropertiesContext::ShapePropertiesContext( ContextHandler2Helper const & rParent, Shape& rShape )
: ContextHandler2( rParent )
, mrShape( rShape )
{
}

ContextHandlerRef ShapePropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    // CT_Transform2D
    case A_TOKEN( xfrm ):
        return new Transform2DContext( *this, rAttribs, mrShape );

    // GeometryGroup
    case A_TOKEN( custGeom ):   // custom geometry "CT_CustomGeometry2D"
        return new CustomShapeGeometryContext( *this, *mrShape.getCustomShapeProperties() );

    case A_TOKEN( prstGeom ):   // preset geometry "CT_PresetGeometry2D"
        {
            sal_Int32 nToken = rAttribs.getToken( XML_prst, 0 );
            // TODO: Move the following checks to a separate place or as a separate function
            if (nToken == XML_line && !mrShape.isConnectorShape())
            {
                mrShape.setServiceName(u"com.sun.star.drawing.LineShape"_ustr);
            }

            // We got a preset geometry, forget the geometry inherited from the placeholder shape.
            mrShape.getCustomShapeProperties() = std::make_shared<CustomShapeProperties>();

            return new PresetShapeGeometryContext( *this, rAttribs, *(mrShape.getCustomShapeProperties()) );
        }

    case A_TOKEN( prstTxWarp ):
        return new PresetTextShapeContext( *this, rAttribs, *(mrShape.getCustomShapeProperties()) );

    // CT_LineProperties
    case A_TOKEN( ln ):
        return new LinePropertiesContext( *this, rAttribs, mrShape.getLineProperties() );

    // EffectPropertiesGroup
    // todo not supported by core
    case A_TOKEN( effectLst ):  // CT_EffectList
    case A_TOKEN( effectDag ):  // CT_EffectContainer
        return new EffectPropertiesContext( *this, mrShape.getEffectProperties() );

    // todo not supported by core, only for preservation via grab bags
    case A_TOKEN( scene3d ):    // CT_Scene3D
        return new Scene3DPropertiesContext( *this, mrShape.get3DProperties() );

    // todo not supported by core, only for preservation via grab bags
    case A_TOKEN( sp3d ):       // CT_Shape3D
        return new Shape3DPropertiesContext( *this, rAttribs, mrShape.get3DProperties() );
    }

    return FillPropertiesContext::createFillContext(*this, aElementToken, rAttribs, mrShape.getFillProperties(), nullptr);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
