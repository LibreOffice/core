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
        ContextHandler2Helper& rParent,
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

ContextHandlerRef PPTShapeGroupContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( getNamespace( aElementToken ) == NMSP_dsp )
        aElementToken = NMSP_ppt | getBaseToken( aElementToken );

    switch( aElementToken )
    {
    case PPT_TOKEN( cNvPr ):
    {
        mpGroupShapePtr->setHidden( rAttribs.getBool( XML_hidden, false ) );
        mpGroupShapePtr->setId( rAttribs.getString( XML_id ).get() );
        mpGroupShapePtr->setName( rAttribs.getString( XML_name ).get() );
        break;
    }
    case PPT_TOKEN( ph ):
        mpGroupShapePtr->setSubType( rAttribs.getToken( XML_type, FastToken::DONTKNOW ) );
        if( rAttribs.hasAttribute( XML_idx ) )
            mpGroupShapePtr->setSubTypeIndex( rAttribs.getString( XML_idx ).get().toInt32() );
        break;
    // nvSpPr CT_ShapeNonVisual end

    case PPT_TOKEN( grpSpPr ):
        return new oox::drawingml::ShapePropertiesContext( *this, *mpGroupShapePtr );
    case PPT_TOKEN( spPr ):
        return new oox::drawingml::ShapePropertiesContext( *this, *mpGroupShapePtr );
/*
    case PPT_TOKEN( style ):
        return new ShapeStyleContext( getParser() );
*/
    case PPT_TOKEN( cxnSp ):        // connector shape
        return new oox::drawingml::ConnectorShapeContext( *this, mpGroupShapePtr, oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.ConnectorShape" ) ) );
    case PPT_TOKEN( grpSp ):        // group shape
        return new PPTShapeGroupContext( *this, mpSlidePersistPtr, meShapeLocation, mpGroupShapePtr, oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) ) );
    case PPT_TOKEN( sp ):           // Shape
        {
            boost::shared_ptr<PPTShape> pShape( new PPTShape( meShapeLocation, "com.sun.star.drawing.CustomShape" ) );
            if( rAttribs.getBool( XML_useBgFill, false ) )
            {
                ::oox::drawingml::FillProperties &aFill = pShape->getFillProperties();
                aFill.moFillType = XML_solidFill;
                // This is supposed to fill with slide (background) color, but
                // TODO: We are using white here, because thats the closest we can assume (?)
                aFill.maFillColor.setSrgbClr( API_RGB_WHITE );
            }
            pShape->setModelId(rAttribs.getString( XML_modelId ).get());
            return new PPTShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr, pShape );
        }
    case PPT_TOKEN( pic ):          // CT_Picture
        return new PPTGraphicShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr,  oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GraphicObjectShape" ) ) );
    case PPT_TOKEN( graphicFrame ): // CT_GraphicalObjectFrame
        {
            if( pGraphicShape )
                importExtDrawings();
            pGraphicShape = oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.OLE2Shape" ) );
            return new oox::drawingml::GraphicalObjectFrameContext( *this, mpGroupShapePtr, pGraphicShape, true );
        }
    }

    return this;
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

void PPTShapeGroupContext::onEndElement()
{
    importExtDrawings();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
