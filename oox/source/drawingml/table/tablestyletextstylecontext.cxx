/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>

#include "oox/drawingml/table/tablestyletextstylecontext.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml { namespace table {

TableStyleTextStyleContext::TableStyleTextStyleContext( ContextHandler2Helper& rParent,
    const AttributeList& rAttribs, TableStylePart& rTableStylePart )
: ContextHandler2( rParent )
, mrTableStylePart( rTableStylePart )
{
    if( rAttribs.hasAttribute( XML_b ) ) {
        sal_Int32 nB = rAttribs.getToken( XML_b, XML_def );
        if ( nB == XML_on )
            mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_True );
        else if ( nB == XML_off )
            mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_False );
    }

    if( rAttribs.hasAttribute( XML_i ) ) {
        sal_Int32 nI = rAttribs.getToken( XML_i, XML_def );
        if ( nI == XML_on )
            mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_True );
        else if ( nI == XML_off )
            mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_False );
    }
}

TableStyleTextStyleContext::~TableStyleTextStyleContext()
{
}


ContextHandlerRef
TableStyleTextStyleContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        
        case A_TOKEN( font ):       
            return this;
        case A_TOKEN( ea ):             
            mrTableStylePart.getAsianFont().setAttributes( rAttribs );
            return 0;
        case A_TOKEN( cs ):             
            mrTableStylePart.getComplexFont().setAttributes( rAttribs );
            return 0;
        case A_TOKEN( sym ):            
            mrTableStylePart.getSymbolFont().setAttributes( rAttribs );
            return 0;
        case A_TOKEN( latin ):          
            mrTableStylePart.getLatinFont().setAttributes( rAttribs );
            return 0;

        case A_TOKEN( fontRef ):    
            {
                ShapeStyleRef& rFontStyle = mrTableStylePart.getStyleRefs()[ XML_fontRef ];
                rFontStyle.mnThemedIdx = rAttribs.getToken( XML_idx, XML_none );
                return new ColorContext( *this, rFontStyle.maPhClr );
            }

        case A_TOKEN( extLst ):     
            break;
    }

    return new ColorValueContext( *this, mrTableStylePart.getTextColor() );
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
