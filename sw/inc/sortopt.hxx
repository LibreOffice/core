/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SORTOPT_HXX
#define _SORTOPT_HXX


#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include "swdllapi.h"

enum SwSortOrder        { SRT_ASCENDING, SRT_DESCENDING };
enum SwSortDirection    { SRT_COLUMNS, SRT_ROWS         };

/*--------------------------------------------------------------------
    Beschreibung: SortierSchluessel
 --------------------------------------------------------------------*/
struct SW_DLLPUBLIC SwSortKey
{
    SwSortKey();
    SwSortKey( sal_uInt16 nId, const String& rSrtType, SwSortOrder eOrder );
    SwSortKey( const SwSortKey& rOld );

    String          sSortType;
    SwSortOrder     eSortOrder;
    sal_uInt16          nColumnId;
    sal_Bool            bIsNumeric;
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
    sal_uInt16          nLanguage;
    sal_Bool            bTable;
    sal_Bool            bIgnoreCase;
};

#endif  // _SORTOPT_HXX
