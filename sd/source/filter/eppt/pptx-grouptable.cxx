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

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XIndexAccess;

GroupTable::GroupTable() :
    mnCurrentGroupEntry ( 0 ),
    mnMaxGroupEntry     ( 0 ),
    mnGroupsClosed      ( 0 ),
    mpGroupEntry        ( NULL )
{
    ImplResizeGroupTable( 32 );
}



GroupTable::~GroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; delete mpGroupEntry[ i++ ] ) ;
    delete[] mpGroupEntry;
}



void GroupTable::ImplResizeGroupTable( sal_uInt32 nEntrys )
{
    if ( nEntrys > mnMaxGroupEntry )
    {
        mnMaxGroupEntry         = nEntrys;
        GroupEntry** pTemp = new GroupEntry*[ nEntrys ];
        for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++ )
            pTemp[ i ] = mpGroupEntry[ i ];
        if ( mpGroupEntry )
            delete[] mpGroupEntry;
        mpGroupEntry = pTemp;
    }
}



sal_Bool GroupTable::EnterGroup( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rXIndexAccessRef )
{
    sal_Bool bRet = sal_False;
    if ( rXIndexAccessRef.is() )
    {
        GroupEntry* pNewGroup = new GroupEntry( rXIndexAccessRef );
        if ( pNewGroup->mnCount )
        {
            if ( mnMaxGroupEntry == mnCurrentGroupEntry )
                ImplResizeGroupTable( mnMaxGroupEntry + 8 );
            mpGroupEntry[ mnCurrentGroupEntry++ ] = pNewGroup;
            bRet = sal_True;
        }
        else
            delete pNewGroup;
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
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++, delete mpGroupEntry[ i ] ) ;
    mnCurrentGroupEntry = 0;
}



void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( nCount );
}



sal_Bool GroupTable::GetNextGroupEntry()
{
    while ( mnCurrentGroupEntry )
    {
        mnIndex = mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCurrentPos++;

        if ( mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCount > mnIndex )
            return sal_True;

        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );

        if ( mnCurrentGroupEntry )
            mnGroupsClosed++;
    }
    return sal_False;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
