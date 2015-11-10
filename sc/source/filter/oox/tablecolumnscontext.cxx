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

#include "tablecolumnscontext.hxx"

#include "tablecolumnsbuffer.hxx"
#include "biffinputstream.hxx"

namespace oox {
namespace xls {

using ::oox::core::ContextHandlerRef;

TableColumnContext::TableColumnContext( WorksheetContextBase& rParent, TableColumn& rTableColumn ) :
    WorksheetContextBase( rParent ),
    mrTableColumn( rTableColumn )
{
}

ContextHandlerRef TableColumnContext::onCreateContext( sal_Int32 /*nElement*/, const AttributeList& /*rAttribs*/ )
{
    /* no known child elements */
    return nullptr;
}

void TableColumnContext::onStartElement( const AttributeList& rAttribs )
{
    mrTableColumn.importTableColumn( rAttribs );
}

ContextHandlerRef TableColumnContext::onCreateRecordContext( sal_Int32 /*nRecId*/, SequenceInputStream& /*rStrm*/ )
{
    /* no known child elements */
    return nullptr;
}

void TableColumnContext::onStartRecord( SequenceInputStream& rStrm )
{
    mrTableColumn.importTableColumn( rStrm );
}

TableColumnsContext::TableColumnsContext( WorksheetFragmentBase& rFragment, TableColumns& rTableColumns ) :
    WorksheetContextBase( rFragment ),
    mrTableColumns( rTableColumns )
{
}

ContextHandlerRef TableColumnsContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    if( (getCurrentElement() == XLS_TOKEN( tableColumns )) && (nElement == XLS_TOKEN( tableColumn )) )
        return new TableColumnContext( *this, mrTableColumns.createTableColumn() );
    return nullptr;
}

void TableColumnsContext::onStartElement( const AttributeList& rAttribs )
{
    mrTableColumns.importTableColumns( rAttribs );
}

ContextHandlerRef TableColumnsContext::onCreateRecordContext( sal_Int32 /*nRecId*/, SequenceInputStream& /*rStrm*/ )
{
    /* XXX not implemented */
#if 0
    if( (getCurrentElement() == BIFF12_ID_TABLECOLUMNS) && (nRecId == BIFF12_ID_TABLECOLUMN) )
        return new TableColumnContext( *this, mrTableColumns.createTableColumn() );
#endif
    return nullptr;
}

void TableColumnsContext::onStartRecord( SequenceInputStream& rStrm )
{
    mrTableColumns.importTableColumns( rStrm );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
