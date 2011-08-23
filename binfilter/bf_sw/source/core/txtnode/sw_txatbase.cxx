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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SFXITEMPOOL_HXX
#include <bf_svtools/itempool.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
namespace binfilter {

/*N*/ SwTxtAttr::SwTxtAttr( const SfxPoolItem& rAttr, xub_StrLen nStt )
/*N*/ 	: pAttr( &rAttr ), nStart( nStt )
/*N*/ {
/*N*/ 	bDontExpand = bLockExpandFlag =	bDontMergeAttr = bDontMoveAttr =
/*N*/         bCharFmtAttr = bOverlapAllowedAttr = bPriorityAttr =
/*N*/ 		bDontExpandStart = FALSE;
/*N*/ }

/*N*/ SwTxtAttr::~SwTxtAttr( )
/*N*/ {
/*N*/ }

/*N*/ xub_StrLen* SwTxtAttr::GetEnd()
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

    // RemoveFromPool muss immer vorm DTOR Aufruf erfolgen!!
    // Meldet sein Attribut beim Pool ab
/*N*/ void SwTxtAttr::RemoveFromPool( SfxItemPool& rPool )
/*N*/ {
/*N*/ 	rPool.Remove( GetAttr() );
/*N*/ 	pAttr = 0;
/*N*/ }


/*N*/ SwTxtAttrEnd::SwTxtAttrEnd( const SfxPoolItem& rAttr, xub_StrLen nS,
/*N*/ 							xub_StrLen nE )
/*N*/ 	: SwTxtAttr( rAttr, nS ), nEnd( nE )
/*N*/ {
/*N*/ }

/*N*/ xub_StrLen* SwTxtAttrEnd::GetEnd()
/*N*/ {
/*N*/ 	return &nEnd;
/*N*/ }
}
