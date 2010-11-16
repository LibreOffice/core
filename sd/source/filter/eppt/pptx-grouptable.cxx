/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// ---------------------------------------------------------------------------------------------

GroupTable::~GroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; delete mpGroupEntry[ i++ ] ) ;
    delete[] mpGroupEntry;
}

// ---------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------

sal_uInt32 GroupTable::GetGroupsClosed()
{
    sal_uInt32 nRet = mnGroupsClosed;
    mnGroupsClosed = 0;
    return nRet;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ClearGroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++, delete mpGroupEntry[ i ] ) ;
    mnCurrentGroupEntry = 0;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( nCount );
}

// ---------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------

void GroupTable::SkipCurrentGroup()
{
    if ( mnCurrentGroupEntry )
        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );
}

// ---------------------------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
