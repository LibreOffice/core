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

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "oox/drawingml/textbodycontext.hxx"

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

ShapeGroupContext::ShapeGroupContext( ContextHandler2Helper& rParent, ShapePtr pMasterShapePtr, ShapePtr pGroupShapePtr )
: ContextHandler2( rParent )
, mpGroupShapePtr( pGroupShapePtr )
, mpMasterShapePtr( pMasterShapePtr )
{
}

ShapeGroupContext::~ShapeGroupContext()
{
    if ( mpMasterShapePtr.get() && mpGroupShapePtr.get() )
        mpMasterShapePtr->addChild( mpGroupShapePtr );
}

Reference< XFastContextHandler > ShapeGroupContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( getBaseToken( aElementToken ) )
    {
    case XML_cNvPr:
    {
        AttributeList aAttribs( xAttribs );
        mpGroupShapePtr->setHidden( aAttribs.getBool( XML_hidden, false ) );
        mpGroupShapePtr->setId( xAttribs->getOptionalValue( XML_id ) );
        mpGroupShapePtr->setName( xAttribs->getOptionalValue( XML_name ) );
        break;
    }
    case XML_ph:
        mpGroupShapePtr->setSubType( xAttribs->getOptionalValueToken( XML_type, FastToken::DONTKNOW ) );
        if( xAttribs->hasAttribute( XML_idx ) )
            mpGroupShapePtr->setSubTypeIndex( xAttribs->getOptionalValue( XML_idx ).toInt32() );
        break;
    // nvSpPr CT_ShapeNonVisual end

    case XML_grpSpPr:
        xRet = new ShapePropertiesContext( *this, *mpGroupShapePtr );
        break;
    case XML_spPr:
        xRet = new ShapePropertiesContext( *this, *mpGroupShapePtr );
        break;
/*
    case XML_style:
        xRet = new ShapeStyleContext( getParser() );
        break;
*/
    case XML_cxnSp:         // connector shape
        {
            ShapePtr pShape(new Shape("com.sun.star.drawing.ConnectorShape"));
            pShape->setLockedCanvas(mpGroupShapePtr->getLockedCanvas());
            xRet.set( new ConnectorShapeContext( *this, mpGroupShapePtr, pShape ) );
        }
        break;
    case XML_grpSp:         // group shape
        xRet.set( new ShapeGroupContext( *this, mpGroupShapePtr, ShapePtr( new Shape( "com.sun.star.drawing.GroupShape" ) ) ) );
        break;
    case XML_sp:            // shape
        xRet.set( new ShapeContext( *this, mpGroupShapePtr, ShapePtr( new Shape( "com.sun.star.drawing.CustomShape" ) ) ) );
        break;
    case XML_pic:           // CT_Picture
        xRet.set( new GraphicShapeContext( *this, mpGroupShapePtr, ShapePtr( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) ) ) );
        break;
    case XML_graphicFrame:  // CT_GraphicalObjectFrame
        xRet.set( new GraphicalObjectFrameContext( *this, mpGroupShapePtr, ShapePtr( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) ), true ) );
        break;
    }
    if( !xRet.is() )
        xRet.set( this );


    return xRet;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
