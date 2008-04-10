/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: indexmap.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"





#include <memory.h>

#include "indexmap.hxx"


ScIndexMap::ScIndexMap( USHORT nEntries )
{
    nCount = nEntries;
    ULONG nC = nEntries ? ((ULONG) nEntries * 2) : 2;
    pMap = new USHORT [ nC ];
    memset( pMap, 0, nC * sizeof(USHORT) );
}


ScIndexMap::~ScIndexMap()
{
    delete [] pMap;
}


void ScIndexMap::SetPair( USHORT nEntry, USHORT nIndex1, USHORT nIndex2 )
{
    if ( nEntry < nCount )
    {
        ULONG nOff = (ULONG) nEntry * 2;
        pMap[nOff] = nIndex1;
        pMap[nOff+1] = nIndex2;
    }
}


USHORT ScIndexMap::Find( USHORT nIndex1 ) const
{
    USHORT* pStop = pMap + (ULONG) nCount * 2;
    for ( USHORT* pOff = pMap; pOff < pStop; pOff += 2 )
    {
        if ( *pOff == nIndex1 )
            return *(pOff+1);
    }
    return nIndex1;
}


