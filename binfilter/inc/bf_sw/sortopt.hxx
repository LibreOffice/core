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
#ifndef _SORTOPT_HXX
#define _SORTOPT_HXX


#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
namespace binfilter {

enum SwSortOrder  		{ SRT_ASCENDING, SRT_DESCENDING	};
enum SwSortDirection	{ SRT_COLUMNS, SRT_ROWS			};

/*--------------------------------------------------------------------
    Beschreibung: SortierSchluessel
 --------------------------------------------------------------------*/
struct SwSortKey
{
    SwSortKey();
    SwSortKey( const SwSortKey& rOld );

    String			sSortType;
    SwSortOrder		eSortOrder;
    USHORT 			nColumnId;
    BOOL			bIsNumeric;
};

SV_DECL_PTRARR(SwSortKeys, SwSortKey*, 3, 1)

 struct SwSortOptions
 {
     SwSortOptions();
     ~SwSortOptions();
     SwSortOptions(const SwSortOptions& rOpt);
 
      SwSortKeys		aKeys;
     SwSortDirection	eDirection;
     sal_Unicode		cDeli;
     USHORT			nLanguage;
     BOOL			bTable;
     BOOL 			bIgnoreCase;
 };

} //namespace binfilter
#endif	// _SORTOPT_HXX
