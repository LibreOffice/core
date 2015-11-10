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

#include "drawingml/shapestylecontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "drawingml/colorchoicecontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// CT_ShapeStyle

ShapeStyleContext::ShapeStyleContext( ContextHandler2Helper& rParent, Shape& rShape )
: ContextHandler2( rParent )
, mrShape( rShape )
{
}

ShapeStyleContext::~ShapeStyleContext()
{
}

ContextHandlerRef ShapeStyleContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( lnRef ) :     // CT_StyleMatrixReference
        case A_TOKEN( fillRef ) :   // CT_StyleMatrixReference
        case A_TOKEN( effectRef ) : // CT_StyleMatrixReference
        case A_TOKEN( fontRef ) :   // CT_FontReference
        {
            sal_Int32 nToken = getBaseToken( aElementToken );
            ShapeStyleRef& rStyleRef = mrShape.getShapeStyleRefs()[ nToken ];
            rStyleRef.mnThemedIdx = (nToken == XML_fontRef) ? rAttribs.getToken( XML_idx, XML_none ) : rAttribs.getInteger( XML_idx, 0 );
            // Set default Text Color. Some xml files don't seem
            // to have color definitions inside fontRef - Use
            // tx1 in such cases
            if( nToken == XML_fontRef && !rStyleRef.maPhClr.isUsed() )
                rStyleRef.maPhClr.setSchemeClr(XML_tx1);
            return new ColorContext( *this, rStyleRef.maPhClr );
        }
    }
    return nullptr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
