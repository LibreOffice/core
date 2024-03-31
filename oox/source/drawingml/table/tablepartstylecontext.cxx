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


#include <drawingml/table/tablepartstylecontext.hxx>
#include <drawingml/table/tablestyletextstylecontext.hxx>
#include <drawingml/table/tablestylecellstylecontext.hxx>
#include <oox/token/namespaces.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;

namespace oox::drawingml::table {

TablePartStyleContext::TablePartStyleContext( ContextHandler2Helper const & rParent, TableStylePart& rTableStylePart )
: ContextHandler2( rParent )
, mrTableStylePart( rTableStylePart )
{
}

TablePartStyleContext::~TablePartStyleContext()
{
}

// CT_TablePartStyle
ContextHandlerRef
TablePartStyleContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( tcTxStyle ):  // CT_TableStyleTextStyle
            return new TableStyleTextStyleContext( *this, rAttribs, mrTableStylePart );
        case A_TOKEN( tcStyle ):    // CT_TableStyleCellStyle
            return new TableStyleCellStyleContext( *this,  mrTableStylePart );
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
