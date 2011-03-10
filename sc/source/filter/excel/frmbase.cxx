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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "formel.hxx"

_ScRangeListTabs::_ScRangeListTabs()
{
}

_ScRangeListTabs::~_ScRangeListTabs()
{
}


void _ScRangeListTabs::Append( ScSingleRefData a, SCsTAB nTab, const BOOL b )
{
    if( b )
    {
        if( a.nTab > MAXTAB )
            a.nTab = MAXTAB;

        if( a.nCol > MAXCOL )
            a.nCol = MAXCOL;

        if( a.nRow > MAXROW )
            a.nRow = MAXROW;
    }
    else
    {
        DBG_ASSERT( ValidTab(a.nTab), "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
    }

    if( nTab == SCTAB_MAX)
        return;
    if( nTab < 0)
        nTab = a.nTab;

    if (nTab < 0 || MAXTAB < nTab)
        return;

    TabRangeType::iterator itr = maTabRanges.find(nTab);
    if (itr == maTabRanges.end())
    {
        // No entry for this table yet.  Insert a new one.
        std::pair<TabRangeType::iterator, bool> r =
            maTabRanges.insert(nTab, new RangeListType);

        if (!r.second)
            // Insertion failed.
            return;

        itr = r.first;
    }
    itr->second->push_back(ScRange(a.nCol,a.nRow,a.nTab));
}

void _ScRangeListTabs::Append( ScComplexRefData a, SCsTAB nTab, const BOOL b )
{
    if( b )
    {
        // ignore 3D ranges
        if( a.Ref1.nTab != a.Ref2.nTab )
            return;

        SCsTAB& rTab = a.Ref1.nTab;
        if( rTab > MAXTAB )
            rTab = MAXTAB;
        else if( rTab < 0 )
            rTab = 0;

        SCsCOL& rCol1 = a.Ref1.nCol;
        if( rCol1 > MAXCOL )
            rCol1 = MAXCOL;
        else if( rCol1 < 0 )
            rCol1 = 0;

        SCsROW& rRow1 = a.Ref1.nRow;
        if( rRow1 > MAXROW )
            rRow1 = MAXROW;
        else if( rRow1 < 0 )
            rRow1 = 0;

        SCsCOL& rCol2 = a.Ref2.nCol;
        if( rCol2 > MAXCOL )
            rCol2 = MAXCOL;
        else if( rCol2 < 0 )
            rCol2 = 0;

        SCsROW& rRow2 = a.Ref2.nRow;
        if( rRow2 > MAXROW )
            rRow2 = MAXROW;
        else if( rRow2 < 0 )
            rRow2 = 0;
    }
    else
    {
        DBG_ASSERT( ValidTab(a.Ref1.nTab),
            "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
        DBG_ASSERT( a.Ref1.nTab == a.Ref2.nTab,
            "+_ScRangeListTabs::Append(): 3D-Ranges werden in SC nicht unterstuetzt!" );
    }

    if( nTab == SCTAB_MAX)
        return;

    if( nTab < -1)
        nTab = a.Ref1.nTab;

    if (nTab < 0 || MAXTAB < nTab)
        return;

    TabRangeType::iterator itr = maTabRanges.find(nTab);
    if (itr == maTabRanges.end())
    {
        // No entry for this table yet.  Insert a new one.
        std::pair<TabRangeType::iterator, bool> r =
            maTabRanges.insert(nTab, new RangeListType);

        if (!r.second)
            // Insertion failed.
            return;

        itr = r.first;
    }
    itr->second->push_back(
        ScRange(a.Ref1.nCol,a.Ref1.nRow,a.Ref1.nTab,
                a.Ref2.nCol,a.Ref2.nRow,a.Ref2.nTab));
}

const ScRange* _ScRangeListTabs::First( const UINT16 n )
{
    DBG_ASSERT( ValidTab(n), "-_ScRangeListTabs::First(): Und tschuessssssss!" );

    TabRangeType::iterator itr = maTabRanges.find(static_cast<SCsTAB>(n));
    if (itr == maTabRanges.end())
        // No range list exists for this table.
        return NULL;

    const RangeListType& rList = *itr->second;
    maItrCur = rList.begin();
    maItrCurEnd = rList.end();
    return rList.empty() ? NULL : &(*maItrCur);
}

const ScRange* _ScRangeListTabs::Next ()
{
    ++maItrCur;
    if (maItrCur == maItrCurEnd)
        return NULL;

    return &(*maItrCur);
}

ConverterBase::ConverterBase( UINT16 nNewBuffer ) :
    aEingPos( 0, 0, 0 ),
    eStatus( ConvOK ),
    nBufferSize( nNewBuffer )
{
    DBG_ASSERT( nNewBuffer > 0, "ConverterBase::ConverterBase - nNewBuffer == 0!" );
    pBuffer = new sal_Char[ nNewBuffer ];
}

ConverterBase::~ConverterBase()
{
    delete[] pBuffer;
}

void ConverterBase::Reset()
{
    eStatus = ConvOK;
    aPool.Reset();
    aStack.Reset();
}




ExcelConverterBase::ExcelConverterBase( UINT16 nNewBuffer ) :
    ConverterBase( nNewBuffer )
{
}

ExcelConverterBase::~ExcelConverterBase()
{
}

void ExcelConverterBase::Reset( const ScAddress& rEingPos )
{
    ConverterBase::Reset();
    aEingPos = rEingPos;
}

void ExcelConverterBase::Reset()
{
    ConverterBase::Reset();
    aEingPos.Set( 0, 0, 0 );
}




LotusConverterBase::LotusConverterBase( SvStream &rStr, UINT16 nNewBuffer ) :
    ConverterBase( nNewBuffer ),
    aIn( rStr ),
    nBytesLeft( 0 )
{
}

LotusConverterBase::~LotusConverterBase()
{
}

void LotusConverterBase::Reset( const ScAddress& rEingPos )
{
    ConverterBase::Reset();
    nBytesLeft = 0;
    aEingPos = rEingPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
