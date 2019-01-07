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


#include "diagramdefinitioncontext.hxx"
#include "diagramfragmenthandler.hxx"
#include "datamodelcontext.hxx"
#include <drawingml/colorchoicecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace oox { namespace drawingml {

DiagramDataFragmentHandler::DiagramDataFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramDataPtr& rDataPtr )
    : FragmentHandler2( rFilter, rFragmentPath )
    , mpDataPtr( rDataPtr )
{
}

DiagramDataFragmentHandler::~DiagramDataFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramDataFragmentHandler::endDocument()
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

DiagramLayoutFragmentHandler::DiagramLayoutFragmentHandler( XmlFilterBase& rFilter,
                                                        const OUString& rFragmentPath,
                                                        const DiagramLayoutPtr& rDataPtr )
    : FragmentHandler2( rFilter, rFragmentPath )
    , mpDataPtr( rDataPtr )
{
}

DiagramLayoutFragmentHandler::~DiagramLayoutFragmentHandler( ) throw ()
{

}

void SAL_CALL DiagramLayoutFragmentHandler::endDocument()
{

}

ContextHandlerRef
DiagramLayoutFragmentHandler::onCreateContext( ::sal_Int32 aElement,
                                               const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case DGM_TOKEN( layoutDef ):
        return new DiagramDefinitionContext( *this, rAttribs, mpDataPtr );
    default:
        break;
    }

    return this;
}

DiagramQStylesFragmentHandler::DiagramQStylesFragmentHandler( XmlFilterBase& rFilter,
                                                              const OUString& rFragmentPath,
                                                              DiagramQStyleMap& rStylesMap ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    maStyleName(),
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
    return new ColorContext( *this, o_rStyle.maPhClr );
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
        maStyleName = rAttribs.getString( XML_name, OUString() );
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
    maColorName(),
    maColorEntry(),
    mrColorsMap(rColorsMap)
{}

::oox::core::ContextHandlerRef ColorFragmentHandler::onCreateContext( sal_Int32 nElement,
                                                                      const AttributeList& /*rAttribs*/ )
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

                // TODO(F1): well, actually, there might be *several* color
                // definitions in it, after all its called list. but
                // apparently ColorContext doesn't handle that anyway...
                case DGM_TOKEN(fillClrLst):
                    return new ColorContext( *this, maColorEntry.maFillColor );
                case DGM_TOKEN(linClrLst):
                    return new ColorContext( *this, maColorEntry.maLineColor );
                case DGM_TOKEN(effectClrLst):
                    return new ColorContext( *this, maColorEntry.maEffectColor );
                case DGM_TOKEN(txFillClrLst):
                    return new ColorContext( *this, maColorEntry.maTextFillColor );
                case DGM_TOKEN(txLinClrLst):
                    return new ColorContext( *this, maColorEntry.maTextLineColor );
                case DGM_TOKEN(txEffectClrLst):
                    return new ColorContext( *this, maColorEntry.maTextEffectColor );
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
        maColorName = rAttribs.getString( XML_name, OUString() );
        maColorEntry = mrColorsMap[maColorName];
    }
}

void ColorFragmentHandler::onEndElement( )
{
    if( getCurrentElement() == DGM_TOKEN(styleLbl) )
        mrColorsMap[maColorName] = maColorEntry;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
