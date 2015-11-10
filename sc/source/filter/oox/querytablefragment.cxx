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

#include "querytablefragment.hxx"

#include "biffinputstream.hxx"
#include "querytablebuffer.hxx"

namespace oox {
namespace xls {

using namespace ::oox::core;

QueryTableFragment::QueryTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mrQueryTable( getQueryTables().createQueryTable() )
{
}

ContextHandlerRef QueryTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( queryTable ) )
                mrQueryTable.importQueryTable( rAttribs );
        break;
    }
    return nullptr;
}

ContextHandlerRef QueryTableFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_QUERYTABLE )
                mrQueryTable.importQueryTable( rStrm );
        break;
    }
    return nullptr;
}

const RecordInfo* QueryTableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_QUERYTABLE,         BIFF12_ID_QUERYTABLE + 1        },
        { BIFF12_ID_QUERYTABLEREFRESH,  BIFF12_ID_QUERYTABLEREFRESH + 1 },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
