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

#include <tablecolumnsbuffer.hxx>

#include <sal/log.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/tokens.hxx>
#include <dbdata.hxx>

namespace oox {
namespace xls {

TableColumn::TableColumn( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnId( -1 ),
    mnDataDxfId( -1 )
{
}

void TableColumn::importTableColumn( const AttributeList& rAttribs )
{
    mnId = rAttribs.getInteger( XML_id, -1 );
    maName = rAttribs.getString( XML_name, OUString() );
    mnDataDxfId = rAttribs.getInteger( XML_dataDxfId, -1 );
}

void TableColumn::importTableColumn( SequenceInputStream& /*rStrm*/ )
{
    /* XXX not implemented */
    (void) mnId;
}

const OUString& TableColumn::getName() const
{
    return maName;
}

TableColumns::TableColumns( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnCount(0)
{
}

void TableColumns::importTableColumns( const AttributeList& rAttribs )
{
    mnCount = rAttribs.getInteger( XML_count, 0 );
}

void TableColumns::importTableColumns( SequenceInputStream& /*rStrm*/ )
{
    /* XXX not implemented */
    (void) mnCount;
}

TableColumn& TableColumns::createTableColumn()
{
    TableColumnVector::value_type xTableColumn( new TableColumn( *this ) );
    maTableColumnVector.push_back( xTableColumn );
    return *xTableColumn;
}

bool TableColumns::finalizeImport( ScDBData* pDBData )
{
    SAL_WARN_IF( static_cast<size_t>(mnCount) != maTableColumnVector.size(), "sc.filter",
            "TableColumns::finalizeImport - count attribute doesn't match number of tableColumn elements");
    if ( pDBData )
    {
        /* TODO: use svl::SharedString for names */
        ::std::vector< OUString > aNames( maTableColumnVector.size());
        size_t i = 0;
        for (const auto& rxTableColumn : maTableColumnVector)
        {
            aNames[i] = rxTableColumn->getName();
            ++i;
        }
        pDBData->SetTableColumnNames( aNames);
        return true;
    }
    return false;
}

TableColumnsBuffer::TableColumnsBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

TableColumns& TableColumnsBuffer::createTableColumns()
{
    TableColumnsVector::value_type xTableColumns( new TableColumns( *this ) );
    maTableColumnsVector.push_back( xTableColumns );
    return *xTableColumns;
}

void TableColumnsBuffer::finalizeImport( ScDBData* pDBData )
{
    TableColumns* pTableColumns = getActiveTableColumns();
    if ( pTableColumns )
        pTableColumns->finalizeImport( pDBData );
}

TableColumns* TableColumnsBuffer::getActiveTableColumns()
{
    // not more than one table columns descriptor per table
    SAL_WARN_IF( maTableColumnsVector.size() > 1, "sc.filter",
            "TableColumnsBuffer::getActiveTableColumns - too many table columns" );
    // stick to the last imported table columns
    return maTableColumnsVector.empty() ? nullptr : maTableColumnsVector.back().get();
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
