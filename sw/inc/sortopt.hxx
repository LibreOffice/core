/*************************************************************************
 *
 *  $RCSfile: sortopt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-04-04 08:17:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SORTOPT_HXX
#define _SORTOPT_HXX


#ifndef _ONLY_SORT_KEY_TYPE
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#endif

// also used in the resources !

    // for all
#define SRT_NUMERIC     1
    // for western
#define SRT_APLHANUM    2
    // additional for chinese
#define SRT_PINYIN      3
#define SRT_STROKE      4
#define SRT_RADICALS    5
#define SRT_DATE        6
#define SRT_CHUYIN      7
    // additional for japanese
#define SRT_JIS         8
#define SRT_SYLLABEL    9
    // additional for korean
#define SRT_KS_CODE     10
#define SRT_DICTIONARY  11


#ifndef _ONLY_SORT_KEY_TYPE

typedef USHORT SwSortKeyType;

enum SwSortOrder        { SRT_ASCENDING, SRT_DESCENDING };
enum SwSortDirection    { SRT_COLUMNS, SRT_ROWS         };

/*--------------------------------------------------------------------
    Beschreibung: SortierSchluessel
 --------------------------------------------------------------------*/
struct SwSortKey
{
    SwSortKey();
    SwSortKey(USHORT nId, SwSortKeyType eTyp, SwSortOrder eOrder);
    SwSortKey(const SwSortKey& rOld);

    USHORT          nColumnId;
    SwSortKeyType   eSortKeyType;
    SwSortOrder     eSortOrder;
};

SV_DECL_PTRARR(SwSortKeys, SwSortKey*, 3, 1)

struct SwSortOptions
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

#endif // _ONLY_SORT_KEY_TYPE

#endif  // _SORTOPT_HXX
