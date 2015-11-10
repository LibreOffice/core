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

#include "tablefragment.hxx"

#include "autofilterbuffer.hxx"
#include "autofiltercontext.hxx"
#include "tablecolumnsbuffer.hxx"
#include "tablecolumnscontext.hxx"
#include "tablebuffer.hxx"

namespace oox {
namespace xls {

using namespace ::oox::core;

TableFragment::TableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mrTable( getTables().createTable() )
{
}

ContextHandlerRef TableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( table ) )
            {
                mrTable.importTable( rAttribs, getSheetIndex() );
                return this;
            }
        break;
        case XLS_TOKEN( table ):
            switch (nElement)
            {
                case XLS_TOKEN( autoFilter ):
                    return new AutoFilterContext( *this, mrTable.createAutoFilter() );
                case XLS_TOKEN( tableColumns ):
                    return new TableColumnsContext( *this, mrTable.createTableColumns() );
            }
        break;
    }
    return nullptr;
}

ContextHandlerRef TableFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_TABLE )
            {
                mrTable.importTable( rStrm, getSheetIndex() );
                return this;
            }
        break;
        case BIFF12_ID_TABLE:
            if( nRecId == BIFF12_ID_AUTOFILTER )
                return new AutoFilterContext( *this, mrTable.createAutoFilter() );
        break;
    }
    return nullptr;
}

const RecordInfo* TableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_AUTOFILTER,         BIFF12_ID_AUTOFILTER + 1        },
        { BIFF12_ID_CUSTOMFILTERS,      BIFF12_ID_CUSTOMFILTERS + 1     },
        { BIFF12_ID_DISCRETEFILTERS,    BIFF12_ID_DISCRETEFILTERS + 1   },
        { BIFF12_ID_FILTERCOLUMN,       BIFF12_ID_FILTERCOLUMN + 1      },
        { BIFF12_ID_TABLE,              BIFF12_ID_TABLE + 1             },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
