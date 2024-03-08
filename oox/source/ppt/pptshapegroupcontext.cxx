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
#include <utility>

using namespace oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;

namespace oox::ppt {

PPTShapeGroupContext::PPTShapeGroupContext(
        FragmentHandler2 const & rParent,
        oox::ppt::SlidePersistPtr pSlidePersistPtr,
        const ShapeLocation eShapeLocation,
        const oox::drawingml::ShapePtr& pMasterShapePtr,
        const oox::drawingml::ShapePtr& pGroupShapePtr )
: ShapeGroupContext( rParent, pMasterShapePtr, pGroupShapePtr )
, mpSlidePersistPtr(std::move( pSlidePersistPtr ))
, meShapeLocation( eShapeLocation )
{
}

ContextHandlerRef PPTShapeGroupContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( getNamespace( aElementToken ) == NMSP_dsp )
        aElementToken = NMSP_ppt | getBaseToken( aElementToken );

    switch( aElementToken )
    {
    case OOX_TOKEN(dml, extLst):
    case OOX_TOKEN(dml, ext):
        break;
    case OOX_TOKEN(adec, decorative):
        {
            mpGroupShapePtr->setDecorative(rAttribs.getBool(XML_val, false));
        }
        break;
    case PPT_TOKEN( cNvPr ):
    {
        // don't override SmartArt properties for embedded drawing's spTree
        mpGroupShapePtr->setHidden( rAttribs.getBool( XML_hidden, false ) );
        if (mpGroupShapePtr->getId().isEmpty())
            mpGroupShapePtr->setId(rAttribs.getStringDefaulted(XML_id));
        if (mpGroupShapePtr->getName().isEmpty())
            mpGroupShapePtr->setName( rAttribs.getStringDefaulted( XML_name ) );
        break;
    }
    case PPT_TOKEN( ph ):
        mpGroupShapePtr->setSubType( rAttribs.getToken( XML_type, FastToken::DONTKNOW ) );
        if( rAttribs.hasAttribute( XML_idx ) )
            mpGroupShapePtr->setSubTypeIndex( rAttribs.getInteger( XML_idx, 0 ) );
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
    {
        auto pShape = std::make_shared<PPTShape>(meShapeLocation, u"com.sun.star.drawing.ConnectorShape"_ustr);
        return new oox::drawingml::ConnectorShapeContext(*this, mpGroupShapePtr, pShape,
                                                         pShape->getConnectorShapeProperties());
    }
    case PPT_TOKEN( grpSp ):        // group shape
        return new PPTShapeGroupContext( *this, mpSlidePersistPtr, meShapeLocation, mpGroupShapePtr, std::make_shared<PPTShape>( meShapeLocation, u"com.sun.star.drawing.GroupShape"_ustr ) );
    case PPT_TOKEN( sp ):           // Shape
        {
            auto pShape = std::make_shared<PPTShape>( meShapeLocation, u"com.sun.star.drawing.CustomShape"_ustr );
            bool bUseBgFill = rAttribs.getBool(XML_useBgFill, false);
            if (bUseBgFill)
            {
                pShape->getFillProperties().moFillType = XML_noFill;
                pShape->getFillProperties().moUseBgFill = true;
            }
            pShape->setModelId(rAttribs.getStringDefaulted( XML_modelId ));
            return new PPTShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr, pShape );
        }
    case PPT_TOKEN( pic ):          // CT_Picture
        return new PPTGraphicShapeContext( *this, mpSlidePersistPtr, mpGroupShapePtr,  std::make_shared<PPTShape>( meShapeLocation, u"com.sun.star.drawing.GraphicObjectShape"_ustr ) );
    case PPT_TOKEN( graphicFrame ): // CT_GraphicalObjectFrame
        {
            pGraphicShape = std::make_shared<PPTShape>( meShapeLocation, u"com.sun.star.drawing.OLE2Shape"_ustr );
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
