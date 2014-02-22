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

#include "oox/drawingml/table/tablestylecontext.hxx"
#include "oox/drawingml/table/tablebackgroundstylecontext.hxx"
#include "oox/drawingml/table/tablepartstylecontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml { namespace table {

TableStyleContext::TableStyleContext( ContextHandler2Helper& rParent,
    const AttributeList& rAttribs, TableStyle& rTableStyle )
: ContextHandler2( rParent )
, mrTableStyle( rTableStyle )
{
    mrTableStyle.getStyleId() = rAttribs.getString( XML_styleId ).get();
    mrTableStyle.getStyleName() = rAttribs.getString( XML_styleName ).get();
}

TableStyleContext::~TableStyleContext()
{
}

ContextHandlerRef
TableStyleContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& /* rAttribs */ )
{
    switch( aElementToken )
    {
        case A_TOKEN( tblBg ):      
            return new TableBackgroundStyleContext( *this, mrTableStyle );
        case A_TOKEN( wholeTbl ):   
            return new TablePartStyleContext( *this, mrTableStyle.getWholeTbl() );
        case A_TOKEN( band1H ):     
            return new TablePartStyleContext( *this, mrTableStyle.getBand1H() );
        case A_TOKEN( band2H ):     
            return new TablePartStyleContext( *this, mrTableStyle.getBand2H() );
        case A_TOKEN( band1V ):     
            return new TablePartStyleContext( *this, mrTableStyle.getBand1V() );
        case A_TOKEN( band2V ):     
            return new TablePartStyleContext( *this, mrTableStyle.getBand2V() );
        case A_TOKEN( lastCol ):    
            return new TablePartStyleContext( *this, mrTableStyle.getLastCol() );
        case A_TOKEN( firstCol ):   
            return new TablePartStyleContext( *this, mrTableStyle.getFirstCol() );
        case A_TOKEN( lastRow ):    
            return new TablePartStyleContext( *this, mrTableStyle.getLastRow() );
        case A_TOKEN( seCell ):     
            return new TablePartStyleContext( *this, mrTableStyle.getSeCell() );
        case A_TOKEN( swCell ):     
            return new TablePartStyleContext( *this, mrTableStyle.getSwCell() );
        case A_TOKEN( firstRow ):   
            return new TablePartStyleContext( *this, mrTableStyle.getFirstRow() );
        case A_TOKEN( neCell ):     
            return new TablePartStyleContext( *this, mrTableStyle.getNeCell() );
        case A_TOKEN( nwCell ):     
            return new TablePartStyleContext( *this, mrTableStyle.getNwCell() );
        case A_TOKEN( extLst ):     
            break;
    }
    return this;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
