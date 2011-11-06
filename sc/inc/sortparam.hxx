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



#ifndef SC_SORTPARAM_HXX
#define SC_SORTPARAM_HXX

#include "address.hxx"
#include <tools/string.hxx>
#include <tools/solar.h>
#include <com/sun/star/lang/Locale.hpp>
#include "scdllapi.h"

#define MAXSORT     3


struct ScSubTotalParam;
struct ScQueryParam;

struct SC_DLLPUBLIC ScSortParam
{
    SCCOL       nCol1;
    SCROW       nRow1;
    SCCOL       nCol2;
    SCROW       nRow2;
    sal_Bool        bHasHeader;
    sal_Bool        bByRow;
    sal_Bool        bCaseSens;
    sal_Bool        bUserDef;
    sal_uInt16      nUserIndex;
    sal_Bool        bIncludePattern;
    sal_Bool        bInplace;
    SCTAB       nDestTab;
    SCCOL       nDestCol;
    SCROW       nDestRow;
    sal_Bool        bDoSort[MAXSORT];
    SCCOLROW    nField[MAXSORT];
    sal_Bool        bAscending[MAXSORT];
    ::com::sun::star::lang::Locale      aCollatorLocale;
    String      aCollatorAlgorithm;
    sal_uInt16      nCompatHeader;

    ScSortParam();
    ScSortParam( const ScSortParam& r );
    /// SubTotals sort
    ScSortParam( const ScSubTotalParam& rSub, const ScSortParam& rOld );
    /// TopTen sort
    ScSortParam( const ScQueryParam&, SCCOL nCol );

    ScSortParam&    operator=   ( const ScSortParam& r );
    sal_Bool            operator==  ( const ScSortParam& rOther ) const;
    void            Clear       ();

    void            MoveToDest();
};


#endif // SC_SORTPARAM_HXX
