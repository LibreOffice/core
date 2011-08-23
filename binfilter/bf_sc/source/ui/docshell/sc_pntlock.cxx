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

#include "pntlock.hxx"
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ ScPaintLockData::ScPaintLockData(USHORT nNewMode) :
/*N*/ 	nMode( nNewMode ),
/*N*/ 	nParts( 0 ),
/*N*/ 	nLevel( 0 ),
/*N*/ 	nDocLevel( 0 ),
/*N*/ 	bModified( FALSE )
/*N*/ {
/*N*/ }

/*N*/ ScPaintLockData::~ScPaintLockData()
/*N*/ {
/*N*/ }

/*N*/ void ScPaintLockData::AddRange( const ScRange& rRange, USHORT nP )
/*N*/ {
/*N*/ 	if (!xRangeList.Is())
/*N*/ 		xRangeList = new ScRangeList;
/*N*/ 
/*N*/ 	xRangeList->Join( rRange );
/*N*/ 	nParts |= nP;
/*N*/ }




}
