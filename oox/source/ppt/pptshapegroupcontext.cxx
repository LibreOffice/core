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
#include "oox/ppt/pptshape.hxx"
#include "oox/ppt/pptgraphicshapecontext.hxx"
#include "oox/ppt/pptshapecontext.hxx"
#include "oox/ppt/pptshapegroupcontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/customshapegeometry.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "extdrawingfragmenthandler.hxx"

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

PPTShapeGroupContext::PPTShapeGroupContext(
        ContextHandler& rParent,
        const oox::ppt::SlidePersistPtr pSlidePersistPtr,
        const ShapeLocation eShapeLocation,
        oox::drawingml::ShapePtr pMasterShapePtr,
        oox::drawingml::ShapePtr pGroupShapePtr )
: ShapeGroupContext( rParent, pMasterShapePtr, pGroupShapePtr )
, mpSlidePersistPtr( pSlidePersistPtr )
, meShapeLocation( eShapeLocation )
, pGraphicShape( (PPTShape *)NULL )
{
}

Reference< XFastContextHandler > PPTShapeGroupContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    if( getNamespace( aElementToken ) == NMSP_dsp )
        aElementToken = NMSP_ppt | getBaseToken( aElementToken );

    switch( aElementToken )
    {
    case PPT_TOKEN( cNvPr ):
    {
        AttributeList aAttribs( xAttribs );
        mpGroupShapePtr->setHidden( aAttribs.getBool( XML_hidden, false ) );
        mpGroupShapePtr->setId( xAttribs->getOptionalValue( XML_id ) );
        mpGroupShapePtr->setName( xAttribs->getOptionalValue( XML_name ) );
        break;
    }
    case PPT_TOKEN( ph ):
        mpGroupShapePtr->setSubType( xAttribs->getOptionalValueToken( XML_type, FastToken::DONTKNOW ) );
        if( xAttribs->hasAttribute( XML_idx ) )
            mpGroupShapePtr->setSubTypeIndex( xAttribs->getOptionalValue( XML_idx ).toInt32() );
        break;
    // nvSpPr CT_ShapeNonVisual end

    case PPT_TOKEN( grpSpPr ):
        xRet = new oox::drawingml::ShapePropertiesContext( *this, *mpGroupShapePtr );
        break;
    case PPT_TOKEN( spPr ):
        xRet = new oox::drawingml::ShapePropertiesContext( *this, *mpGroupShapePtr );
        break;
/*
    case PPT_TOKEN( style ):
        xRet = new ShapeStyleContext( getParser() );
        break;
*/
    case PPT_TOKEN( cxnSp ):        // connector shape
        xRet.set( new oox::drawingml::ConnectorShapeContext( *this, mpGroupShapePtr, oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.ConnectorShape" ) ) ) );
        break;
    case PPT_TOKEN( grpSp ):        // group shape
        xRet.set( new PPTShapeGroupContext( *this, mpSlidePersistPtr, meShapeLocation, mpGroupShapePtr, oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) ) ) );
        break;
    case PPT_TOKEN( sp ):           // Shape
        {
            AttributeList aAttribs( xAttribs );
            boost::shared_ptr<PPTShape> pShape( new PPTShape( meShapeLocation, "com.sun.star.drawing.CustomShape" ) );
            if( aAttribs.getBool( XML_useBgFill, false ) )
            {
                ::oox::drawingml::FillProperties &aFill = pShape->getFillProperties();
                aFill.moFillType = XML_solidFill;
                // This is supposed to fill with slide (background) color, but
                // TODO: We are using white here, because thats the closest we can assume (?)
                aFill.maFillColor.setSrgbClr( API_RGB_WHITE );
            }
            pShape->setModelId(xAttribs->getOptionalValue( XML_modelId ));
            xRet.set( new PPTShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr, pShape ) );
        }
        break;
    case PPT_TOKEN( pic ):          // CT_Picture
        xRet.set( new PPTGraphicShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr,  oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GraphicObjectShape" ) ) ) );
        break;
    case PPT_TOKEN( graphicFrame ): // CT_GraphicalObjectFrame
        {
            if( pGraphicShape )
                importExtDrawings();
            pGraphicShape = oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.OLE2Shape" ) );
            xRet.set( new oox::drawingml::GraphicalObjectFrameContext( *this, mpGroupShapePtr, pGraphicShape, true ) );
        }
        break;

    }
    if( !xRet.is() )
        xRet.set( this );


    return xRet;
}

void PPTShapeGroupContext::importExtDrawings( )
{
    if( pGraphicShape )
    {
        for( ::std::vector<OUString>::const_iterator aIt = pGraphicShape->getExtDrawings().begin(), aEnd = pGraphicShape->getExtDrawings().end();
                    aIt != aEnd; ++aIt )
            {
                getFilter().importFragment( new ExtDrawingFragmentHandler( getFilter(), getFragmentPathFromRelId( *aIt ),
                                                                           mpSlidePersistPtr,
                                                                           meShapeLocation,
                                                                           mpMasterShapePtr,
                                                                           mpGroupShapePtr,
                                                                           pGraphicShape ) );
            }
            pGraphicShape = oox::drawingml::ShapePtr( (PPTShape *)NULL );
    }
}

void PPTShapeGroupContext::endFastElement( sal_Int32 /*nElement*/ ) throw (SAXException, RuntimeException)
{
    importExtDrawings();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
