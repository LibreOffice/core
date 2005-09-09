/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sortopt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:09:45 $
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
#ifndef _SORTOPT_HXX
#define _SORTOPT_HXX


#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

enum SwSortOrder        { SRT_ASCENDING, SRT_DESCENDING };
enum SwSortDirection    { SRT_COLUMNS, SRT_ROWS         };

/*--------------------------------------------------------------------
    Beschreibung: SortierSchluessel
 --------------------------------------------------------------------*/
struct SW_DLLPUBLIC SwSortKey
{
    SwSortKey();
    SwSortKey( USHORT nId, const String& rSrtType, SwSortOrder eOrder );
    SwSortKey( const SwSortKey& rOld );

    String          sSortType;
    SwSortOrder     eSortOrder;
    USHORT          nColumnId;
    BOOL            bIsNumeric;
};

SV_DECL_PTRARR(SwSortKeys, SwSortKey*, 3, 1)

struct SW_DLLPUBLIC SwSortOptions
{
    SwSortOptions();
    ~SwSortOptions();
    SwSortOptions(const SwSortOptions& rOpt);

    SwSortKeys      aKeys;
    SwSortDirection eDirection;
    sal_Unicode     cDeli;
    USHORT          nLanguage;
    BOOL            bTable;
    BOOL            bIgnoreCase;
};

#endif  // _SORTOPT_HXX
