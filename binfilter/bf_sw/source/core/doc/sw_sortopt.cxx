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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _SORTOPT_HXX
#include <sortopt.hxx>
#endif
namespace binfilter {


/*N*/ SV_IMPL_PTRARR(SwSortKeys, SwSortKey*)

/*--------------------------------------------------------------------
    Beschreibung:	Sortier-Schluessel
 --------------------------------------------------------------------*/

/*N*/ SwSortKey::SwSortKey() :
/*N*/ 	eSortOrder( SRT_ASCENDING ),
/*N*/ 	nColumnId( 0 ),
/*N*/ 	bIsNumeric( TRUE )
/*N*/ {
/*N*/ }

/*N*/ SwSortKey::SwSortKey(const SwSortKey& rOld) :
/*N*/ 	eSortOrder( rOld.eSortOrder ),
/*N*/ 	sSortType( rOld.sSortType ),
/*N*/ 	nColumnId( rOld.nColumnId ),
/*N*/ 	bIsNumeric( rOld.bIsNumeric )
/*N*/ {
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Sortieroptionen fuers Sortieren
 --------------------------------------------------------------------*/


/*N*/ SwSortOptions::SwSortOptions()
/*?*/ 	: //STRIP001 eDirection( SRT_ROWS ),
/*N*/ 	nLanguage( LANGUAGE_SYSTEM ),
/*N*/ 	cDeli( 9 ),
/*N*/ 	bTable( FALSE ),
/*N*/ 	bIgnoreCase( FALSE )
/*N*/ {
/*N*/ }


/*N*/ SwSortOptions::SwSortOptions(const SwSortOptions& rOpt) :
/*N*/ 	eDirection( rOpt.eDirection ),
/*N*/ 	cDeli( rOpt.cDeli ),
/*N*/ 	nLanguage( rOpt.nLanguage ),
/*N*/ 	bTable( rOpt.bTable ),
/*N*/ 	bIgnoreCase( rOpt.bIgnoreCase )
/*N*/ {
/*N*/ 	for( USHORT i=0; i < rOpt.aKeys.Count(); ++i )
/*N*/ 	{
/*N*/ 		SwSortKey* pNew = new SwSortKey(*rOpt.aKeys[i]);
/*N*/ 		aKeys.C40_INSERT( SwSortKey, pNew, aKeys.Count());
/*N*/ 	}
/*N*/ }


/*N*/ SwSortOptions::~SwSortOptions()
/*N*/ {
/*N*/ 	aKeys.DeleteAndDestroy(0, aKeys.Count());
/*N*/ }



}
