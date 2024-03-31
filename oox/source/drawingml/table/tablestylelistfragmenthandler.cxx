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

#include <drawingml/table/tablestylelistfragmenthandler.hxx>
#include <drawingml/table/tablestylecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;

namespace oox::drawingml::table {

TableStyleListFragmentHandler::TableStyleListFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, TableStyleList& rTableStyleList ):
FragmentHandler2( rFilter, rFragmentPath ),
mrTableStyleList( rTableStyleList )
{
}

TableStyleListFragmentHandler::~TableStyleListFragmentHandler()
{
}

// CT_TableStyleList
ContextHandlerRef TableStyleListFragmentHandler::onCreateContext(
    sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( tblStyleLst ):    // CT_TableStyleList
            mrTableStyleList.getDefaultStyleId() = rAttribs.getStringDefaulted( XML_def );
            break;
        case A_TOKEN( tblStyle ):       // CT_TableStyle
            std::vector< TableStyle >& rTableStyles = mrTableStyleList.getTableStyles();
            rTableStyles.emplace_back();
            return new TableStyleContext( *this, rAttribs, rTableStyles.back() );
    }
    return this;
}

} // namespace oox::drawingml::table

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
