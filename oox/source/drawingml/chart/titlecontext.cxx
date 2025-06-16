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

#include <drawingml/chart/titlecontext.hxx>

#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/chart/datasourcecontext.hxx>
#include <drawingml/chart/titlemodel.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>

#include <osl/diagnose.h>


namespace oox::drawingml::chart {

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

TextContext::TextContext( ContextHandler2Helper& rParent, TextModel& rModel ) :
    ContextBase< TextModel >( rParent, rModel )
{
}

TextContext::~TextContext()
{
}

ContextHandlerRef TextContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    // this context handler is used for <c:tx> and embedded <c:v> elements
    if( isCurrentElement( C_TOKEN( tx ) ) || isCurrentElement(CX_TOKEN(tx)) ) switch( nElement )
    {
        case C_TOKEN( rich ):
        case CX_TOKEN( rich ):
            return new TextBodyContext( *this, mrModel.mxTextBody.create() );

        case C_TOKEN( strRef ):
            OSL_ENSURE( !mrModel.mxDataSeq, "TextContext::onCreateContext - multiple data sequences" );
            return new StringSequenceContext( *this, mrModel.mxDataSeq.create() );

        case C_TOKEN( v ):
        case CX_TOKEN( v ):
            OSL_ENSURE( !mrModel.mxDataSeq, "TextContext::onCreateContext - multiple data sequences" );
            return this;    // collect value in onCharacters()
        case CX_TOKEN( txData ):
            // CT_TextData can have a <cx:v> element or a sequence
            // <cx:f> <cx:v>. The former case will be handled through the
            // CX_TOKEN(v) above, but the latter is not handled. TODO
            return this;
    }
    return nullptr;
}

void TextContext::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( C_TOKEN( v ) ) )
    {
        // Static text is stored as a single string formula token for Excel document.
        mrModel.mxDataSeq.create().maFormula =  "\"" + rChars + "\"";

        // Also store it as a single element type for non-Excel document.
        mrModel.mxDataSeq->maData[0] <<= rChars;
        mrModel.mxDataSeq->mnPointCount = 1;
    }
}

TitleContext::TitleContext( ContextHandler2Helper& rParent, TitleModel& rModel ) :
    ContextBase< TitleModel >( rParent, rModel )
{
}

TitleContext::~TitleContext()
{
}

ContextHandlerRef TitleContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );

        case C_TOKEN( overlay ):
            mrModel.mbOverlay = rAttribs.getBool( XML_val, true );
            return nullptr;

        case C_TOKEN( spPr ):
        case CX_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );

        case C_TOKEN( tx ):
        case CX_TOKEN( tx ):
            return new TextContext( *this, mrModel.mxText.create() );

        case C_TOKEN( txPr ):
        case CX_TOKEN( txPr ):
            return new TextBodyContext( *this, mrModel.mxTextProp.create() );
    }
    return nullptr;
}

LegendEntryContext::LegendEntryContext( ContextHandler2Helper& rParent, LegendEntryModel& rModel ) :
    ContextBase< LegendEntryModel >( rParent, rModel )
{
}

LegendEntryContext::~LegendEntryContext()
{
}

ContextHandlerRef LegendEntryContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // this context handler is used for <c:legendEntry> only
    switch( nElement )
    {
        case C_TOKEN( idx ):
            mrModel.mnLegendEntryIdx = rAttribs.getInteger( XML_val, -1 );
            return nullptr;

        case C_TOKEN( delete ):
            mrModel.mbLabelDeleted = rAttribs.getBool( XML_val, true );
            return nullptr;
    }
    return nullptr;
}

LegendContext::LegendContext( ContextHandler2Helper& rParent,
        LegendModel& rModel,
        bool bOverlay /* = false */,
        sal_Int32 nPos /* = XML_r */) :
    ContextBase< LegendModel >( rParent, rModel )
{
    // These can't be in the initializer list because they're members of
    // ContextBase<LegendModel>
    mrModel.mbOverlay = bOverlay;
    mrModel.mnPosition = nPos;
}

LegendContext::~LegendContext()
{
}

ContextHandlerRef LegendContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );

        case C_TOKEN( legendPos ):
            mrModel.mnPosition = rAttribs.getToken( XML_val, XML_r );
            return nullptr;

        case C_TOKEN( legendEntry ):
            return new LegendEntryContext( *this, mrModel.maLegendEntries.create() );

        case C_TOKEN( overlay ):
            // For cx, overlay is an attribute of <cx:legend>
            mrModel.mbOverlay = rAttribs.getBool( XML_val, true );
            return nullptr;

        case C_TOKEN( spPr ):
        case CX_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );

        case C_TOKEN( txPr ):
        case CX_TOKEN( txPr ):
            return new TextBodyContext( *this, mrModel.mxTextProp.create() );
    }
    return nullptr;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
