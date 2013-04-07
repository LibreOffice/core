/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_SORTPARAM_HXX
#define SC_SORTPARAM_HXX

#define DEFSORT 3

#include <vector>

#include "address.hxx"
#include <tools/solar.h>
#include <com/sun/star/lang/Locale.hpp>
#include "scdllapi.h"

struct ScSubTotalParam;
struct ScQueryParam;

struct ScSortKeyState
{
    bool     bDoSort;
    SCCOLROW nField;
    bool     bAscending;
};

typedef ::std::vector<ScSortKeyState> ScSortKeyStateVec;

struct SC_DLLPUBLIC ScSortParam
{
    SCCOL       nCol1;
    SCROW       nRow1;
    SCCOL       nCol2;
    SCROW       nRow2;
    sal_uInt16  nUserIndex;
    bool        bHasHeader;
    bool        bByRow;
    bool        bCaseSens;
    bool        bNaturalSort;
    bool        bUserDef;
    bool        bIncludePattern;
    bool        bInplace;
    SCTAB       nDestTab;
    SCCOL       nDestCol;
    SCROW       nDestRow;
    ScSortKeyStateVec maKeyState;
    ::com::sun::star::lang::Locale aCollatorLocale;
    OUString aCollatorAlgorithm;
    sal_uInt16  nCompatHeader;


    ScSortParam();
    ScSortParam( const ScSortParam& r );
    /// SubTotals sort
    ScSortParam( const ScSubTotalParam& rSub, const ScSortParam& rOld );
    /// TopTen sort
    ScSortParam( const ScQueryParam&, SCCOL nCol );

    ScSortParam&    operator=  ( const ScSortParam& r );
    bool            operator== ( const ScSortParam& rOther ) const;
    void            Clear       ();
    void            MoveToDest();

    inline sal_uInt16 GetSortKeyCount() const { return maKeyState.size(); }
};


#endif // SC_SORTPARAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
