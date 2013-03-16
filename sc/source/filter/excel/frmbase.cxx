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


#include "formel.hxx"

_ScRangeListTabs::_ScRangeListTabs()
{
}

_ScRangeListTabs::~_ScRangeListTabs()
{
}


void _ScRangeListTabs::Append( ScSingleRefData a, SCTAB nTab, const bool b )
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
        OSL_ENSURE( ValidTab(a.nTab), "-_ScRangeListTabs::Append(): A lie has no crash!" );
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

void _ScRangeListTabs::Append( ScComplexRefData a, SCTAB nTab, bool b )
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
        OSL_ENSURE( ValidTab(a.Ref1.nTab),
            "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
        OSL_ENSURE( a.Ref1.nTab == a.Ref2.nTab,
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

const ScRange* _ScRangeListTabs::First( SCTAB n )
{
    OSL_ENSURE( ValidTab(n), "-_ScRangeListTabs::First(): Good bye!" );

    TabRangeType::iterator itr = maTabRanges.find(n);
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

ConverterBase::ConverterBase( sal_uInt16 nNewBuffer ) :
    aEingPos( 0, 0, 0 ),
    eStatus( ConvOK ),
    nBufferSize( nNewBuffer )
{
    OSL_ENSURE( nNewBuffer > 0, "ConverterBase::ConverterBase - nNewBuffer == 0!" );
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




ExcelConverterBase::ExcelConverterBase( sal_uInt16 nNewBuffer ) :
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




LotusConverterBase::LotusConverterBase( SvStream &rStr, sal_uInt16 nNewBuffer ) :
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
