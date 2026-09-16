/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include "diagramdefinitioncontext.hxx"
#include "diagramfragmenthandler.hxx"
#include "datamodelcontext.hxx"
#include <drawingml/colorchoicecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <utility>

using namespace ::oox::core;

namespace oox::drawingml {

DiagramDataFragmentHandler::DiagramDataFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        OoxDiagramDataPtr xDataPtr )
    : FragmentHandler2( rFilter, rFragmentPath )
    , mpDataPtr(std::move( xDataPtr ))
{
}

DiagramDataFragmentHandler::~DiagramDataFragmentHandler( ) noexcept
{

}

void DiagramDataFragmentHandler::endDocument()
{

}

ContextHandlerRef
DiagramDataFragmentHandler::onCreateContext( ::sal_Int32 aElement,
                                             const AttributeList& )
{
    switch( aElement )
    {
    case DGM_TOKEN( dataModel ):
        return new DataModelContext( *this, mpDataPtr );
    default:
        break;
    }

    return this;
}

DiagramLayoutFragmentHandler::DiagramLayoutFragmentHandler( SmartArtDiagram& rDgm,
                                                        XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        DiagramLayoutPtr xDataPtr )
    : FragmentHandler2( rFilter, rFragmentPath )
    , mpDataPtr(std::move( xDataPtr ))
    , mrDgm(rDgm)
{
}

DiagramLayoutFragmentHandler::~DiagramLayoutFragmentHandler( ) noexcept
{

}

void DiagramLayoutFragmentHandler::endDocument()
{

}

ContextHandlerRef
DiagramLayoutFragmentHandler::onCreateContext( ::sal_Int32 aElement,
                                               const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case DGM_TOKEN( layoutDef ):
        return new DiagramDefinitionContext( mrDgm, *this, rAttribs, mpDataPtr );
    default:
        break;
    }

    return this;
}

DiagramQStylesFragmentHandler::DiagramQStylesFragmentHandler( XmlFilterBase& rFilter,
                                                              const OUString& rFragmentPath,
                                                              DiagramQStyleMap& rStylesMap ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    maStyleEntry(),
    mrStylesMap( rStylesMap )
{}

::oox::core::ContextHandlerRef DiagramQStylesFragmentHandler::createStyleMatrixContext(
    sal_Int32 nElement,
    const AttributeList& rAttribs,
    ShapeStyleRef& o_rStyle )
{
    o_rStyle.mnThemedIdx = (nElement == A_TOKEN(fontRef)) ?
        rAttribs.getToken( XML_idx, XML_none ) : rAttribs.getInteger( XML_idx, 0 );
    return new ColorContext(*this, &o_rStyle.maPhClr);
}

::oox::core::ContextHandlerRef DiagramQStylesFragmentHandler::onCreateContext( sal_Int32 nElement,
                                                                               const AttributeList& rAttribs )
{
    // state-table like way of navigating the color fragment. we
    // currently ignore everything except styleLbl in the styleDef
    // element
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return nElement == DGM_TOKEN(styleDef) ? this : nullptr;
        case DGM_TOKEN(styleDef):
            return nElement == DGM_TOKEN(styleLbl) ? this : nullptr;
        case DGM_TOKEN(styleLbl):
            return nElement == DGM_TOKEN(style) ? this : nullptr;
        case DGM_TOKEN(style):
        {
            switch( nElement )
            {
                case A_TOKEN(lnRef):     // CT_StyleMatrixReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maLineStyle);
                case A_TOKEN(fillRef):   // CT_StyleMatrixReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maFillStyle);
                case A_TOKEN(effectRef): // CT_StyleMatrixReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maEffectStyle);
                case A_TOKEN(fontRef):   // CT_FontReference
                    return createStyleMatrixContext(nElement,rAttribs,
                                                    maStyleEntry.maTextStyle);
            }
            return nullptr;
        }
    }

    return nullptr;
}

void DiagramQStylesFragmentHandler::onStartElement( const AttributeList& rAttribs )
{
    if( getCurrentElement() == DGM_TOKEN( styleLbl ) )
    {
        maStyleName = rAttribs.getStringDefaulted( XML_name);
        maStyleEntry = mrStylesMap[maStyleName];
    }
}

void DiagramQStylesFragmentHandler::onEndElement( )
{
    if( getCurrentElement() == DGM_TOKEN(styleLbl) )
        mrStylesMap[maStyleName] = maStyleEntry;
}

ColorFragmentHandler::ColorFragmentHandler( ::oox::core::XmlFilterBase& rFilter,
                                            const OUString& rFragmentPath,
                                            DiagramColorMap& rColorsMap ) :
    FragmentHandler2(rFilter,rFragmentPath),
    maColorEntry(),
    mrColorsMap(rColorsMap)
{}

namespace
{
/// Reads the meth attribute of a color list into rList.
void lclInitColorList(DiagramColorList& rList, const AttributeList& rAttribs)
{
    // The standard gives span as the default when the attribute is absent. The hueDir
    // attribute beside it is deliberately not read, see the comment where a spread is
    // worked out.
    rList.mnMethod = rAttribs.getToken(XML_meth, XML_span);
}
}

::oox::core::ContextHandlerRef ColorFragmentHandler::onCreateContext( sal_Int32 nElement,
                                                                      const AttributeList& rAttribs )
{
    // state-table like way of navigating the color fragment. we
    // currently ignore everything except styleLbl in the colorsDef
    // element
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return nElement == DGM_TOKEN(colorsDef) ? this : nullptr;
        case DGM_TOKEN(colorsDef):
            return nElement == DGM_TOKEN(styleLbl) ? this : nullptr;
        case DGM_TOKEN(styleLbl):
        {
            switch( nElement )
            {
                // the actual colors - defer to color fragment handlers.

                case DGM_TOKEN(fillClrLst):
                    lclInitColorList(maColorEntry.maFillColors, rAttribs);
                    return new ColorsContext( *this, maColorEntry.maFillColors.maColors );
                case DGM_TOKEN(linClrLst):
                    lclInitColorList(maColorEntry.maLineColors, rAttribs);
                    return new ColorsContext( *this, maColorEntry.maLineColors.maColors );
                case DGM_TOKEN(effectClrLst):
                    lclInitColorList(maColorEntry.maEffectColors, rAttribs);
                    return new ColorsContext( *this, maColorEntry.maEffectColors.maColors );
                case DGM_TOKEN(txFillClrLst):
                    lclInitColorList(maColorEntry.maTextFillColors, rAttribs);
                    return new ColorsContext( *this, maColorEntry.maTextFillColors.maColors );
                case DGM_TOKEN(txLinClrLst):
                    lclInitColorList(maColorEntry.maTextLineColors, rAttribs);
                    return new ColorsContext( *this, maColorEntry.maTextLineColors.maColors );
                case DGM_TOKEN(txEffectClrLst):
                    lclInitColorList(maColorEntry.maTextEffectColors, rAttribs);
                    return new ColorsContext( *this, maColorEntry.maTextEffectColors.maColors );
            }
            break;
        }
    }

    return nullptr;
}

void ColorFragmentHandler::onStartElement( const AttributeList& rAttribs )
{
    if( getCurrentElement() == DGM_TOKEN(styleLbl) )
    {
        maColorName = rAttribs.getStringDefaulted( XML_name);
        maColorEntry = mrColorsMap[maColorName];
    }
}

void ColorFragmentHandler::onEndElement( )
{
    if( getCurrentElement() != DGM_TOKEN(styleLbl) )
        return;

    // Resolve the colors of a list that spreads them, span or cycle, while the theme of the
    // slide is still the current one, because a spread is worked out from the real colors.
    const GraphicHelper& rGraphicHelper(getFilter().getGraphicHelper());
    for (DiagramColorList* pList :
         { &maColorEntry.maFillColors, &maColorEntry.maLineColors, &maColorEntry.maEffectColors,
           &maColorEntry.maTextFillColors, &maColorEntry.maTextLineColors,
           &maColorEntry.maTextEffectColors })
    {
        if ((XML_span != pList->mnMethod && XML_cycle != pList->mnMethod)
            || pList->maColors.size() < 2)
            continue;

        for (const Color& rColor : pList->maColors)
            pList->maResolvedColors.push_back(rColor.getColor(rGraphicHelper));
    }

    mrColorsMap[maColorName] = maColorEntry;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
