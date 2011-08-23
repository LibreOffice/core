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


#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include <memory.h>

#include "indexmap.hxx"
namespace binfilter {


/*N*/ ScIndexMap::ScIndexMap( USHORT nEntries )
/*N*/ {
/*N*/ 	nCount = nEntries;
/*N*/ 	ULONG nC = nEntries ? ((ULONG) nEntries * 2) : 2;
/*N*/ 	pMap = new USHORT [ nC ];
/*N*/ 	memset( pMap, 0, nC * sizeof(USHORT) );
/*N*/ }


/*N*/ ScIndexMap::~ScIndexMap()
/*N*/ {
/*N*/ 	delete [] pMap;
/*N*/ }


/*N*/ void ScIndexMap::SetPair( USHORT nEntry, USHORT nIndex1, USHORT nIndex2 )
/*N*/ {
/*N*/ 	if ( nEntry < nCount )
/*N*/ 	{
/*N*/ 		ULONG nOff = (ULONG) nEntry * 2;
/*N*/ 		pMap[nOff] = nIndex1;
/*N*/ 		pMap[nOff+1] = nIndex2;
/*N*/ 	}
/*N*/ }

}
