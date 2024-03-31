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


#include <drawingml/table/tablestylecontext.hxx>
#include <drawingml/table/tablebackgroundstylecontext.hxx>
#include <drawingml/table/tablepartstylecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;

namespace oox::drawingml::table {

TableStyleContext::TableStyleContext( ContextHandler2Helper const & rParent,
    const AttributeList& rAttribs, TableStyle& rTableStyle )
: ContextHandler2( rParent )
, mrTableStyle( rTableStyle )
{
    mrTableStyle.getStyleId() = rAttribs.getStringDefaulted( XML_styleId );
    mrTableStyle.getStyleName() = rAttribs.getStringDefaulted( XML_styleName );
}

TableStyleContext::~TableStyleContext()
{
}

ContextHandlerRef
TableStyleContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& /* rAttribs */ )
{
    switch( aElementToken )
    {
        case A_TOKEN( tblBg ):      // CT_TableBackgroundStyle
            return new TableBackgroundStyleContext( *this, mrTableStyle );
        case A_TOKEN( wholeTbl ):   // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getWholeTbl() );
        case A_TOKEN( band1H ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getBand1H() );
        case A_TOKEN( band2H ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getBand2H() );
        case A_TOKEN( band1V ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getBand1V() );
        case A_TOKEN( band2V ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getBand2V() );
        case A_TOKEN( lastCol ):    // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getLastCol() );
        case A_TOKEN( firstCol ):   // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getFirstCol() );
        case A_TOKEN( lastRow ):    // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getLastRow() );
        case A_TOKEN( seCell ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getSeCell() );
        case A_TOKEN( swCell ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getSwCell() );
        case A_TOKEN( firstRow ):   // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getFirstRow() );
        case A_TOKEN( neCell ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getNeCell() );
        case A_TOKEN( nwCell ):     // CT_TablePartStyle
            return new TablePartStyleContext( *this, mrTableStyle.getNwCell() );
        case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
            break;
    }
    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
