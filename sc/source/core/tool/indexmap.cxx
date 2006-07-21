/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:29:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


