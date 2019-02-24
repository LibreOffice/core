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

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <oox/helper/attributelist.hxx>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/drawingml/connectorshapecontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/customshapegeometry.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sal/log.hxx>

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

ShapeGroupContext::ShapeGroupContext( ContextHandler2Helper const & rParent, ShapePtr const & pMasterShapePtr, ShapePtr const & pGroupShapePtr )
: ContextHandler2( rParent )
, mpGroupShapePtr( pGroupShapePtr )
{
    if( pMasterShapePtr )
        mpGroupShapePtr->setWps(pMasterShapePtr->getWps());
    if( pMasterShapePtr.get() && mpGroupShapePtr.get() )
        pMasterShapePtr->addChild( mpGroupShapePtr );
}

ShapeGroupContext::~ShapeGroupContext()
{
}

ContextHandlerRef ShapeGroupContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( getBaseToken( aElementToken ) )
    {
    case XML_cNvPr:
    {
        mpGroupShapePtr->setHidden( rAttribs.getBool( XML_hidden, false ) );
        mpGroupShapePtr->setId( rAttribs.getString( XML_id ).get() );
        mpGroupShapePtr->setName( rAttribs.getString( XML_name ).get() );
        break;
    }
    case XML_ph:
        mpGroupShapePtr->setSubType( rAttribs.getToken( XML_type, FastToken::DONTKNOW ) );
        if( rAttribs.hasAttribute( XML_idx ) )
            mpGroupShapePtr->setSubTypeIndex( rAttribs.getString( XML_idx ).get().toInt32() );
        break;
    // nvSpPr CT_ShapeNonVisual end

    case XML_grpSpPr:
        return new ShapePropertiesContext( *this, *mpGroupShapePtr );
    case XML_nvGrpSpPr:
        return this;
    case XML_spPr:
        return new ShapePropertiesContext( *this, *mpGroupShapePtr );
/*
    case XML_style:
        return new ShapeStyleContext( getParser() );
*/
    case XML_cxnSp:         // connector shape
        {
            ShapePtr pShape(new Shape("com.sun.star.drawing.ConnectorShape"));
            pShape->setLockedCanvas(mpGroupShapePtr->getLockedCanvas());
            return new ConnectorShapeContext( *this, mpGroupShapePtr, pShape );
        }
    case XML_grpSp:         // group shape
        return new ShapeGroupContext( *this, mpGroupShapePtr, std::make_shared<Shape>( "com.sun.star.drawing.GroupShape" ) );
    case XML_sp:            // shape
    case XML_wsp:
        // Don't set default character height for WPS shapes, Writer has its
        // own way to set the default, and if we don't set it here, editing
        // properly inherits it.
        return new ShapeContext( *this, mpGroupShapePtr, std::make_shared<Shape>( "com.sun.star.drawing.CustomShape", getBaseToken(aElementToken) == XML_sp ) );
    case XML_pic:           // CT_Picture
        return new GraphicShapeContext( *this, mpGroupShapePtr, std::make_shared<Shape>( "com.sun.star.drawing.GraphicObjectShape" ) );
    case XML_graphicFrame:  // CT_GraphicalObjectFrame
        return new GraphicalObjectFrameContext( *this, mpGroupShapePtr, std::make_shared<Shape>( "com.sun.star.drawing.GraphicObjectShape" ), true );
    case XML_cNvGrpSpPr:
        break;
    case XML_grpSpLocks:
        break;
    default:
        SAL_WARN("oox", "ShapeGroupContext::onCreateContext: unhandled element: " << getBaseToken(aElementToken));
        break;
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
