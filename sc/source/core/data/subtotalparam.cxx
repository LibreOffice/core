/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "subtotalparam.hxx"

ScSubTotalParam::ScSubTotalParam()
{
    for ( sal_uInt16 i=0; i<MAXSUBTOTAL; i++ )
    {
        nSubTotals[i] = 0;
        pSubTotals[i] = NULL;
        pFunctions[i] = NULL;
    }

    Clear();
}

//------------------------------------------------------------------------

ScSubTotalParam::ScSubTotalParam( const ScSubTotalParam& r ) :
        nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),nUserIndex(r.nUserIndex),
        bRemoveOnly(r.bRemoveOnly),bReplace(r.bReplace),bPagebreak(r.bPagebreak),bCaseSens(r.bCaseSens),
        bDoSort(r.bDoSort),bAscending(r.bAscending),bUserDef(r.bUserDef),
        bIncludePattern(r.bIncludePattern)
{
    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = r.bGroupActive[i];
        nField[i]       = r.nField[i];

        if ( (r.nSubTotals[i] > 0) && r.pSubTotals[i] && r.pFunctions[i] )
        {
            nSubTotals[i] = r.nSubTotals[i];
            pSubTotals[i] = new SCCOL   [r.nSubTotals[i]];
            pFunctions[i] = new ScSubTotalFunc  [r.nSubTotals[i]];

            for (SCCOL j=0; j<r.nSubTotals[i]; j++)
            {
                pSubTotals[i][j] = r.pSubTotals[i][j];
                pFunctions[i][j] = r.pFunctions[i][j];
            }
        }
        else
        {
            nSubTotals[i] = 0;
            pSubTotals[i] = NULL;
            pFunctions[i] = NULL;
        }
    }
}

//------------------------------------------------------------------------

void ScSubTotalParam::Clear()
{
    nCol1=nCol2= 0;
    nRow1=nRow2 = 0;
    nUserIndex = 0;
    bPagebreak=bCaseSens=bUserDef=bIncludePattern=bRemoveOnly = false;
    bAscending=bReplace=bDoSort = true;

    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = false;
        nField[i]       = 0;

        if ( (nSubTotals[i] > 0) && pSubTotals[i] && pFunctions[i] )
        {
            for ( SCCOL j=0; j<nSubTotals[i]; j++ ) {
                pSubTotals[i][j] = 0;
                pFunctions[i][j] = SUBTOTAL_FUNC_NONE;
            }
        }
    }
}

//------------------------------------------------------------------------

ScSubTotalParam& ScSubTotalParam::operator=( const ScSubTotalParam& r )
{
    nCol1           = r.nCol1;
    nRow1           = r.nRow1;
    nCol2           = r.nCol2;
    nRow2           = r.nRow2;
    bRemoveOnly     = r.bRemoveOnly;
    bReplace        = r.bReplace;
    bPagebreak      = r.bPagebreak;
    bCaseSens       = r.bCaseSens;
    bDoSort         = r.bDoSort;
    bAscending      = r.bAscending;
    bUserDef        = r.bUserDef;
    nUserIndex      = r.nUserIndex;
    bIncludePattern = r.bIncludePattern;

    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = r.bGroupActive[i];
        nField[i]       = r.nField[i];
        nSubTotals[i]   = r.nSubTotals[i];

        if ( pSubTotals[i] ) delete [] pSubTotals[i];
        if ( pFunctions[i] ) delete [] pFunctions[i];

        if ( r.nSubTotals[i] > 0 )
        {
            pSubTotals[i] = new SCCOL   [r.nSubTotals[i]];
            pFunctions[i] = new ScSubTotalFunc  [r.nSubTotals[i]];

            for (SCCOL j=0; j<r.nSubTotals[i]; j++)
            {
                pSubTotals[i][j] = r.pSubTotals[i][j];
                pFunctions[i][j] = r.pFunctions[i][j];
            }
        }
        else
        {
            nSubTotals[i] = 0;
            pSubTotals[i] = NULL;
            pFunctions[i] = NULL;
        }
    }

    return *this;
}

//------------------------------------------------------------------------

bool ScSubTotalParam::operator==( const ScSubTotalParam& rOther ) const
{
    bool bEqual =   (nCol1          == rOther.nCol1)
                 && (nRow1          == rOther.nRow1)
                 && (nCol2          == rOther.nCol2)
                 && (nRow2          == rOther.nRow2)
                 && (nUserIndex     == rOther.nUserIndex)
                 && (bRemoveOnly    == rOther.bRemoveOnly)
                 && (bReplace       == rOther.bReplace)
                 && (bPagebreak     == rOther.bPagebreak)
                 && (bDoSort        == rOther.bDoSort)
                 && (bCaseSens      == rOther.bCaseSens)
                 && (bAscending     == rOther.bAscending)
                 && (bUserDef       == rOther.bUserDef)
                 && (bIncludePattern== rOther.bIncludePattern);

    if ( bEqual )
    {
        bEqual = true;
        for ( sal_uInt16 i=0; i<MAXSUBTOTAL && bEqual; i++ )
        {
            bEqual =   (bGroupActive[i] == rOther.bGroupActive[i])
                    && (nField[i]       == rOther.nField[i])
                    && (nSubTotals[i]   == rOther.nSubTotals[i]);

            if ( bEqual && (nSubTotals[i] > 0) )
            {
                for (SCCOL j=0; (j<nSubTotals[i]) && bEqual; j++)
                {
                    bEqual =   bEqual
                            && (pSubTotals[i][j] == rOther.pSubTotals[i][j])
                            && (pFunctions[i][j] == rOther.pFunctions[i][j]);
                }
            }
        }
    }

    return bEqual;
}

//------------------------------------------------------------------------

void ScSubTotalParam::SetSubTotals( sal_uInt16 nGroup,
                                    const SCCOL* ptrSubTotals,
                                    const ScSubTotalFunc* ptrFunctions,
                                    sal_uInt16 nCount )
{
    OSL_ENSURE( (nGroup <= MAXSUBTOTAL),
                "ScSubTotalParam::SetSubTotals(): nGroup > MAXSUBTOTAL!" );
    OSL_ENSURE( ptrSubTotals,
                "ScSubTotalParam::SetSubTotals(): ptrSubTotals == NULL!" );
    OSL_ENSURE( ptrFunctions,
                "ScSubTotalParam::SetSubTotals(): ptrFunctions == NULL!" );
    OSL_ENSURE( (nCount > 0),
                "ScSubTotalParam::SetSubTotals(): nCount <= 0!" );

    if ( ptrSubTotals && ptrFunctions && (nCount > 0) && (nGroup <= MAXSUBTOTAL) )
    {
        // 0 wird als 1 aufgefasst, sonst zum Array-Index dekrementieren
        if (nGroup != 0)
            nGroup--;

        delete [] pSubTotals[nGroup];
        delete [] pFunctions[nGroup];

        pSubTotals[nGroup] = new SCCOL      [nCount];
        pFunctions[nGroup] = new ScSubTotalFunc [nCount];
        nSubTotals[nGroup] = static_cast<SCCOL>(nCount);

        for ( sal_uInt16 i=0; i<nCount; i++ )
        {
            pSubTotals[nGroup][i] = ptrSubTotals[i];
            pFunctions[nGroup][i] = ptrFunctions[i];
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
