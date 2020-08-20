/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_SUBTOTALPARAM_HXX
#define INCLUDED_SC_INC_SUBTOTALPARAM_HXX

#include "global.hxx"

struct SC_DLLPUBLIC ScSubTotalParam
{
    SCCOL           nCol1;                      ///< selected area
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
    sal_uInt16      nUserIndex;                 ///< index into list
    bool            bRemoveOnly:1;
    bool            bReplace:1;                 ///< replace existing results
    bool            bPagebreak:1;               ///< page break at change of group
    bool            bCaseSens:1;
    bool            bDoSort:1;                  ///< presort
    bool            bAscending:1;               ///< sort ascending
    bool            bUserDef:1;                 ///< sort user defined
    bool            bIncludePattern:1;          ///< sort formats
    bool            bGroupActive[MAXSUBTOTAL];  ///< active groups
    SCCOL           nField[MAXSUBTOTAL];        ///< associated field
    SCCOL           nSubTotals[MAXSUBTOTAL];    ///< number of SubTotals
    SCCOL*          pSubTotals[MAXSUBTOTAL];    ///< array of columns to be calculated
    ScSubTotalFunc* pFunctions[MAXSUBTOTAL];    ///< array of associated functions

    ScSubTotalParam();
    ScSubTotalParam( const ScSubTotalParam& r );

    ScSubTotalParam& operator= ( const ScSubTotalParam& r );
    bool operator== ( const ScSubTotalParam& r ) const;
    void Clear();
    void SetSubTotals( sal_uInt16 nGroup,
                       const SCCOL* ptrSubTotals,
                       const ScSubTotalFunc* ptrFunctions,
                       sal_uInt16 nCount );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
