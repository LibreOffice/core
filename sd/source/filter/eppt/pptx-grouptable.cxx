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

#include "grouptable.hxx"

using ::com::sun::star::container::XIndexAccess;

GroupTable::GroupTable()
    : mnIndex(0)
    , mnGroupsClosed(0)
    , mvGroupEntry(32)
{
}

GroupTable::~GroupTable()
{
}

bool GroupTable::EnterGroup( css::uno::Reference< css::container::XIndexAccess >& rXIndexAccessRef )
{
    bool bRet = false;
    if ( rXIndexAccessRef.is() )
    {
        std::unique_ptr<GroupEntry> pNewGroup( new GroupEntry( rXIndexAccessRef ) );
        if ( pNewGroup->mnCount )
        {
            mvGroupEntry.push_back( std::move(pNewGroup) );
            bRet = true;
        }
    }
    return bRet;
}

sal_uInt32 GroupTable::GetGroupsClosed()
{
    sal_uInt32 nRet = mnGroupsClosed;
    mnGroupsClosed = 0;
    return nRet;
}

void GroupTable::ClearGroupTable()
{
    mvGroupEntry.clear();
}

void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mvGroupEntry.push_back( std::unique_ptr<GroupEntry>(new GroupEntry( nCount )) );
}

bool GroupTable::GetNextGroupEntry()
{
    while ( !mvGroupEntry.empty() )
    {
        mnIndex = mvGroupEntry.back()->mnCurrentPos++;

        if ( mvGroupEntry.back()->mnCount > mnIndex )
            return true;

        mvGroupEntry.pop_back();

        if ( !mvGroupEntry.empty() )
            mnGroupsClosed++;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
