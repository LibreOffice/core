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

#include <memory>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/core/fragmenthandler2.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/ppt/pptshape.hxx>
#include <oox/ppt/pptgraphicshapecontext.hxx>
#include <oox/ppt/pptshapecontext.hxx>
#include <oox/ppt/pptshapegroupcontext.hxx>
#include <oox/drawingml/graphicshapecontext.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/shapepropertiescontext.hxx>
#include <oox/drawingml/connectorshapecontext.hxx>
#include <drawingml/fillproperties.hxx>
#include "extdrawingfragmenthandler.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox::ppt {

PPTShapeGroupContext::PPTShapeGroupContext(
        FragmentHandler2 const & rParent,
        const oox::ppt::SlidePersistPtr& rSlidePersistPtr,
        const ShapeLocation eShapeLocation,
        const oox::drawingml::ShapePtr& pMasterShapePtr,
        const oox::drawingml::ShapePtr& pGroupShapePtr )
: ShapeGroupContext( rParent, pMasterShapePtr, pGroupShapePtr )
, mpSlidePersistPtr( rSlidePersistPtr )
, meShapeLocation( eShapeLocation )
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
        // don't override SmartArt properties for embedded drawing's spTree
        mpGroupShapePtr->setHidden( rAttribs.getBool( XML_hidden, false ) );
        if (mpGroupShapePtr->getId().isEmpty())
            mpGroupShapePtr->setId(rAttribs.getString(XML_id).get());
        if (mpGroupShapePtr->getName().isEmpty())
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
        return new oox::drawingml::ConnectorShapeContext( *this, mpGroupShapePtr, std::make_shared<PPTShape>( meShapeLocation, "com.sun.star.drawing.ConnectorShape" ) );
    case PPT_TOKEN( grpSp ):        // group shape
        return new PPTShapeGroupContext( *this, mpSlidePersistPtr, meShapeLocation, mpGroupShapePtr, std::make_shared<PPTShape>( meShapeLocation, "com.sun.star.drawing.GroupShape" ) );
    case PPT_TOKEN( sp ):           // Shape
        {
            auto pShape = std::make_shared<PPTShape>( meShapeLocation, "com.sun.star.drawing.CustomShape" );
            bool bUseBgFill = rAttribs.getBool(XML_useBgFill, false);
            pShape->setUseBgFill(bUseBgFill);
            if (bUseBgFill)
            {
                oox::drawingml::FillPropertiesPtr pBackgroundPropertiesPtr = mpSlidePersistPtr->getBackgroundProperties();
                if (!pBackgroundPropertiesPtr)
                {
                    // The shape wants a background, but the slide doesn't have one.
                    SlidePersistPtr pMaster = mpSlidePersistPtr->getMasterPersist();
                    if (pMaster)
                    {
                        oox::drawingml::FillPropertiesPtr pMasterBackground
                            = pMaster->getBackgroundProperties();
                        if (pMasterBackground)
                        {
                            if (pMasterBackground->moFillType.has()
                                && pMasterBackground->moFillType.get() == XML_solidFill)
                            {
                                // Master has a solid background, use that.
                                pBackgroundPropertiesPtr = pMasterBackground;
                            }
                        }
                    }
                }
                if (pBackgroundPropertiesPtr)
                {
                    pShape->getFillProperties().assignUsed(*pBackgroundPropertiesPtr);
                }
            }
            pShape->setModelId(rAttribs.getString( XML_modelId ).get());
            return new PPTShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr, pShape );
        }
    case PPT_TOKEN( pic ):          // CT_Picture
        return new PPTGraphicShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr,  std::make_shared<PPTShape>( meShapeLocation, "com.sun.star.drawing.GraphicObjectShape" ) );
    case PPT_TOKEN( graphicFrame ): // CT_GraphicalObjectFrame
        {
            pGraphicShape = std::make_shared<PPTShape>( meShapeLocation, "com.sun.star.drawing.OLE2Shape" );
            return new oox::drawingml::GraphicalObjectFrameContext( *this, mpGroupShapePtr, pGraphicShape, true );
        }
    }

    return this;
}

void PPTShapeGroupContext::importExtDrawings( )
{
    if( !pGraphicShape )
        return;

    for (auto const& extDrawing : pGraphicShape->getExtDrawings())
    {
        OUString aFragmentPath = getFragmentPathFromRelId(extDrawing);
        getFilter().importFragment( new ExtDrawingFragmentHandler( getFilter(), aFragmentPath,
                                                                   mpSlidePersistPtr,
                                                                   meShapeLocation,
                                                                   mpGroupShapePtr,
                                                                   pGraphicShape ) );
        pGraphicShape->keepDiagramDrawing(getFilter(), aFragmentPath);

        // Apply font color imported from color fragment
        if( pGraphicShape->getFontRefColorForNodes().isUsed() )
            applyFontRefColor(mpGroupShapePtr, pGraphicShape->getFontRefColorForNodes());
    }
    pGraphicShape = oox::drawingml::ShapePtr( nullptr );
}

void PPTShapeGroupContext::applyFontRefColor(const oox::drawingml::ShapePtr& pShape, const oox::drawingml::Color& rFontRefColor)
{
    pShape->getShapeStyleRefs()[XML_fontRef].maPhClr = rFontRefColor;
    std::vector< oox::drawingml::ShapePtr >& vChildren = pShape->getChildren();
    for (auto const& child : vChildren)
    {
        applyFontRefColor(child, rFontRefColor);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
