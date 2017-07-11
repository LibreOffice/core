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


#include "drawingml/table/tablerowcontext.hxx"
#include "drawingml/table/tablecellcontext.hxx"
#include "drawingml/table/tablerow.hxx"
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;

namespace oox { namespace drawingml { namespace table {

TableRowContext::TableRowContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, TableRow& rTableRow )
: ContextHandler2( rParent )
, mrTableRow( rTableRow )
{
    rTableRow.setHeight( rAttribs.getString( XML_h ).get().toInt32() );
}

TableRowContext::~TableRowContext()
{
}

ContextHandlerRef
TableRowContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case A_TOKEN( tc ):         // CT_TableCell
        {
            std::vector< TableCell >& rvTableCells = mrTableRow.getTableCells();
            rvTableCells.emplace_back();
            return new TableCellContext( *this, rAttribs, rvTableCells.back() );
        }
    case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
    default:
        break;
    }

    return this;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
