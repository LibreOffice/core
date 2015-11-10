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

#include "drawingml/chart/titlecontext.hxx"

#include "drawingml/shapepropertiescontext.hxx"
#include "drawingml/textbodycontext.hxx"
#include "drawingml/chart/datasourcecontext.hxx"
#include "drawingml/chart/titlemodel.hxx"

#include "rtl/ustrbuf.hxx"
#include <osl/diagnose.h>


namespace oox {
namespace drawingml {
namespace chart {

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
    if( isCurrentElement( C_TOKEN( tx ) ) ) switch( nElement )
    {
        case C_TOKEN( rich ):
            return new TextBodyContext( *this, mrModel.mxTextBody.create() );

        case C_TOKEN( strRef ):
            OSL_ENSURE( !mrModel.mxDataSeq, "TextContext::onCreateContext - multiple data sequences" );
            return new StringSequenceContext( *this, mrModel.mxDataSeq.create() );

        case C_TOKEN( v ):
            OSL_ENSURE( !mrModel.mxDataSeq, "TextContext::onCreateContext - multiple data sequences" );
            return this;    // collect value in onCharacters()
    }
    return nullptr;
}

void TextContext::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( C_TOKEN( v ) ) )
    {
        // Static text is stored as a single string formula token for Excel document.
        OUStringBuffer aBuf;
        aBuf.append('"').append(rChars).append('"');
        mrModel.mxDataSeq.create().maFormula = aBuf.makeStringAndClear();

        // Also store it as a single element type for non-Excel document.
        mrModel.mxDataSeq->maData[0] <<= rChars;
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
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    // this context handler is used for <c:title> only
    switch( nElement )
    {
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );

        case C_TOKEN( overlay ):
            mrModel.mbOverlay = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;

        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );

        case C_TOKEN( tx ):
            return new TextContext( *this, mrModel.mxText.create() );

        case C_TOKEN( txPr ):
            return new TextBodyContext( *this, mrModel.mxTextProp.create() );
    }
    return nullptr;
}

LegendContext::LegendContext( ContextHandler2Helper& rParent, LegendModel& rModel ) :
    ContextBase< LegendModel >( rParent, rModel )
{
}

LegendContext::~LegendContext()
{
}

ContextHandlerRef LegendContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    // this context handler is used for <c:legend> only
    switch( nElement )
    {
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );

        case C_TOKEN( legendPos ):
            mrModel.mnPosition = rAttribs.getToken( XML_val, XML_r );
            return nullptr;

        case C_TOKEN( overlay ):
            mrModel.mbOverlay = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;

        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );

        case C_TOKEN( txPr ):
            return new TextBodyContext( *this, mrModel.mxTextProp.create() );
    }
    return nullptr;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
