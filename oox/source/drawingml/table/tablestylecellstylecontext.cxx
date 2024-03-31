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


#include <drawingml/table/tablestylecellstylecontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/linepropertiescontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace oox::drawingml::table {

TableStyleCellStyleContext::TableStyleCellStyleContext( ContextHandler2Helper const & rParent, TableStylePart& rTableStylePart )
: ContextHandler2( rParent )
, mrTableStylePart( rTableStylePart )
, mnLineType( XML_none )
{
}

TableStyleCellStyleContext::~TableStyleCellStyleContext()
{
}

// CT_TableStyleCellStyle
ContextHandlerRef
TableStyleCellStyleContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( tcBdr ):      // CT_TableCellBorderStyle
            break;
        case A_TOKEN( left ):       // CT_ThemeableLineStyle
        case A_TOKEN( right ):
        case A_TOKEN( top ):
        case A_TOKEN( bottom ):
        case A_TOKEN( insideH ):
        case A_TOKEN( insideV ):
        case A_TOKEN( tl2br ):
        case A_TOKEN( tr2bl ):
            mnLineType = getBaseToken( aElementToken );
            break;

        case A_TOKEN( ln ):
            {
                if ( mnLineType != XML_none )
                {
                    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >& rLineBorders = mrTableStylePart.getLineBorders();
                    ::oox::drawingml::LinePropertiesPtr pLineProperties = std::make_shared<oox::drawingml::LineProperties>();
                    rLineBorders[ mnLineType ] = pLineProperties;
                    return new LinePropertiesContext( *this, rAttribs, *pLineProperties );
                }
            }
            break;
        case A_TOKEN( lnRef ):
            {
                if ( mnLineType != XML_none )
                {
                    ShapeStyleRef& rLineStyleRef = mrTableStylePart.getStyleRefs()[ mnLineType ];
                    rLineStyleRef.mnThemedIdx = rAttribs.getInteger( XML_idx, 0 );
                    return new ColorContext( *this, rLineStyleRef.maPhClr );
                }
            }
            break;

        // EG_ThemeableFillStyle (choice)
        case A_TOKEN( fill ):       // CT_FillProperties
            {
                FillPropertiesPtr& rxFillProperties = mrTableStylePart.getFillProperties();
                rxFillProperties = std::make_shared<FillProperties>();
                return new FillPropertiesContext( *this, *rxFillProperties );
            }
        case A_TOKEN( fillRef ):    // CT_StyleMatrixReference
            {
                ShapeStyleRef& rStyleRef = mrTableStylePart.getStyleRefs()[ XML_fillRef ];
                rStyleRef.mnThemedIdx = rAttribs.getInteger( XML_idx, 0 );
                return new ColorContext( *this, rStyleRef.maPhClr );
            }
        case A_TOKEN( cell3D ):     // CT_Cell3D
            break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
