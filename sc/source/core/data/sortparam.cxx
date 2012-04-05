/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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




#include "sortparam.hxx"
#include "global.hxx"
#include "address.hxx"
#include "queryparam.hxx"
#include "subtotalparam.hxx"


//------------------------------------------------------------------------

ScSortParam::ScSortParam()
{
    Clear();
}

//------------------------------------------------------------------------

ScSortParam::ScSortParam( const ScSortParam& r ) :
        nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),nUserIndex(r.nUserIndex),
        bHasHeader(r.bHasHeader),bByRow(r.bByRow),bCaseSens(r.bCaseSens),
        bNaturalSort(r.bNaturalSort),bUserDef(r.bUserDef),
        bIncludePattern(r.bIncludePattern),bInplace(r.bInplace),
        nDestTab(r.nDestTab),nDestCol(r.nDestCol),nDestRow(r.nDestRow),
        maKeyState( r.maKeyState ),
        aCollatorLocale( r.aCollatorLocale ), aCollatorAlgorithm( r.aCollatorAlgorithm ),
        nCompatHeader( r.nCompatHeader )
{
}

//------------------------------------------------------------------------

void ScSortParam::Clear()
{
    ScSortKeyState aKeyState;

    nCol1=nCol2=nDestCol = 0;
    nRow1=nRow2=nDestRow = 0;
    nCompatHeader = 2;
    nDestTab = 0;
    nUserIndex = 0;
    bHasHeader=bCaseSens=bUserDef=bNaturalSort = false;
    bByRow=bIncludePattern=bInplace = true;
    aCollatorLocale = ::com::sun::star::lang::Locale();
    aCollatorAlgorithm = ::rtl::OUString();

    aKeyState.bDoSort = false;
    aKeyState.nField = 0;
    aKeyState.bAscending = true;

    // Initialize to default size
    maKeyState.assign( DEFSORT, aKeyState );
}

//------------------------------------------------------------------------

ScSortParam& ScSortParam::operator=( const ScSortParam& r )
{
    nCol1           = r.nCol1;
    nRow1           = r.nRow1;
    nCol2           = r.nCol2;
    nRow2           = r.nRow2;
    nUserIndex      = r.nUserIndex;
    bHasHeader      = r.bHasHeader;
    bByRow          = r.bByRow;
    bCaseSens       = r.bCaseSens;
    bNaturalSort    = r.bNaturalSort;
    bUserDef        = r.bUserDef;
    bIncludePattern = r.bIncludePattern;
    bInplace        = r.bInplace;
    nDestTab        = r.nDestTab;
    nDestCol        = r.nDestCol;
    nDestRow        = r.nDestRow;
    maKeyState      = r.maKeyState;
    aCollatorLocale         = r.aCollatorLocale;
    aCollatorAlgorithm      = r.aCollatorAlgorithm;
    nCompatHeader   = r.nCompatHeader;

    return *this;
}

//------------------------------------------------------------------------

bool ScSortParam::operator==( const ScSortParam& rOther ) const
{
    bool bEqual = false;
    // Anzahl der Sorts gleich?
    sal_uInt16 nLast      = 0;
    sal_uInt16 nOtherLast = 0;
    sal_uInt16 nSortSize = GetSortKeyCount();

    if ( !maKeyState.empty() )
    {
        while ( maKeyState[nLast++].bDoSort && nLast < nSortSize ) ;
        nLast--;
    }

    if ( !rOther.maKeyState.empty() )
    {
        while ( rOther.maKeyState[nOtherLast++].bDoSort && nOtherLast < nSortSize ) ;
        nOtherLast--;
    }

    if (   (nLast           == nOtherLast)
        && (nCol1           == rOther.nCol1)
        && (nRow1           == rOther.nRow1)
        && (nCol2           == rOther.nCol2)
        && (nRow2           == rOther.nRow2)
        && (bHasHeader      == rOther.bHasHeader)
        && (bByRow          == rOther.bByRow)
        && (bCaseSens       == rOther.bCaseSens)
        && (bNaturalSort    == rOther.bNaturalSort)
        && (bUserDef        == rOther.bUserDef)
        && (nUserIndex      == rOther.nUserIndex)
        && (bIncludePattern == rOther.bIncludePattern)
        && (bInplace        == rOther.bInplace)
        && (nDestTab        == rOther.nDestTab)
        && (nDestCol        == rOther.nDestCol)
        && (nDestRow        == rOther.nDestRow)
        && (aCollatorLocale.Language    == rOther.aCollatorLocale.Language)
        && (aCollatorLocale.Country     == rOther.aCollatorLocale.Country)
        && (aCollatorLocale.Variant     == rOther.aCollatorLocale.Variant)
        && (aCollatorAlgorithm          == rOther.aCollatorAlgorithm)
        && ( !maKeyState.empty() || !rOther.maKeyState.empty() )
        )
    {
        bEqual = true;
        for ( sal_uInt16 i=0; i<=nLast && bEqual; i++ )
            bEqual = ( maKeyState[i].nField == rOther.maKeyState[i].nField ) &&
                ( maKeyState[i].bAscending  == rOther.maKeyState[i].bAscending );
    }
    if ( maKeyState.empty() && rOther.maKeyState.empty() )
        bEqual = true;

    return bEqual;
}

//------------------------------------------------------------------------

ScSortParam::ScSortParam( const ScSubTotalParam& rSub, const ScSortParam& rOld ) :
        nCol1(rSub.nCol1),nRow1(rSub.nRow1),nCol2(rSub.nCol2),nRow2(rSub.nRow2),nUserIndex(rSub.nUserIndex),
        bHasHeader(true),bByRow(true),bCaseSens(rSub.bCaseSens),bNaturalSort(rOld.bNaturalSort),
        bUserDef(rSub.bUserDef),bIncludePattern(rSub.bIncludePattern),
        bInplace(true),
        nDestTab(0),nDestCol(0),nDestRow(0),
        aCollatorLocale( rOld.aCollatorLocale ), aCollatorAlgorithm( rOld.aCollatorAlgorithm ),
        nCompatHeader( rOld.nCompatHeader )
{
    sal_uInt16 i;

    //  zuerst die Gruppen aus den Teilergebnissen
    if (rSub.bDoSort)
        for (i=0; i<MAXSUBTOTAL; i++)
            if (rSub.bGroupActive[i])
            {
                ScSortKeyState key;
                key.bDoSort = true;
                key.nField = rSub.nField[i];
                key.bAscending = rSub.bAscending;
                maKeyState.push_back(key);
            }

    //  dann dahinter die alten Einstellungen
    for (i=0; i < rOld.GetSortKeyCount(); i++)
        if (rOld.maKeyState[i].bDoSort)
        {
            SCCOLROW nThisField = rOld.maKeyState[i].nField;
            bool bDouble = false;
            for (sal_uInt16 j = 0; j < GetSortKeyCount(); j++)
                if ( maKeyState[j].nField == nThisField )
                    bDouble = true;
            if (!bDouble)               // ein Feld nicht zweimal eintragen
            {
                ScSortKeyState key;
                key.bDoSort = true;
                key.nField = nThisField;
                key.bAscending = rOld.maKeyState[i].bAscending;
                maKeyState.push_back(key);
            }
        }
}

//------------------------------------------------------------------------

ScSortParam::ScSortParam( const ScQueryParam& rParam, SCCOL nCol ) :
        nCol1(nCol),nRow1(rParam.nRow1),nCol2(nCol),nRow2(rParam.nRow2),nUserIndex(0),
        bHasHeader(rParam.bHasHeader),bByRow(true),bCaseSens(rParam.bCaseSens),
        bNaturalSort(false),
//! TODO: what about Locale and Algorithm?
        bUserDef(false),bIncludePattern(false),
        bInplace(true),
        nDestTab(0),nDestCol(0),nDestRow(0), nCompatHeader(2)
{
    ScSortKeyState aKeyState;
    aKeyState.bDoSort = true;
    aKeyState.nField = nCol;
    aKeyState.bAscending = true;

    maKeyState.push_back( aKeyState );

    // Set the rest
    aKeyState.bDoSort = false;
    aKeyState.nField = 0;

    for (sal_uInt16 i=1; i<GetSortKeyCount(); i++)
        maKeyState.push_back( aKeyState );
}

//------------------------------------------------------------------------

void ScSortParam::MoveToDest()
{
    if (!bInplace)
    {
        SCsCOL nDifX = ((SCsCOL) nDestCol) - ((SCsCOL) nCol1);
        SCsROW nDifY = ((SCsROW) nDestRow) - ((SCsROW) nRow1);

        nCol1 = sal::static_int_cast<SCCOL>( nCol1 + nDifX );
        nRow1 = sal::static_int_cast<SCROW>( nRow1 + nDifY );
        nCol2 = sal::static_int_cast<SCCOL>( nCol2 + nDifX );
        nRow2 = sal::static_int_cast<SCROW>( nRow2 + nDifY );
        for (sal_uInt16 i=0; i<GetSortKeyCount(); i++)
            if (bByRow)
                maKeyState[i].nField += nDifX;
            else
                maKeyState[i].nField += nDifY;

        bInplace = true;
    }
    else
    {
        OSL_FAIL("MoveToDest, bInplace == TRUE");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
