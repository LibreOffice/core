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

#include <formel.hxx>

#include <osl/diagnose.h>

ScRangeListTabs::ScRangeListTabs()
{
}

ScRangeListTabs::~ScRangeListTabs()
{
}

void ScRangeListTabs::Append( const ScAddress& aSRD, SCTAB nTab )
{
    ScAddress a = aSRD;

    if (a.Tab() > MAXTAB)
        a.SetTab(MAXTAB);

    if (a.Col() > MAXCOL)
        a.SetCol(MAXCOL);

    if (a.Row() > MAXROW)
        a.SetRow(MAXROW);

    if( nTab == SCTAB_MAX)
        return;
    if( nTab < 0)
        nTab = a.Tab();

    if (nTab < 0 || MAXTAB < nTab)
        return;

    TabRangeType::iterator itr = m_TabRanges.find(nTab);
    if (itr == m_TabRanges.end())
    {
        // No entry for this table yet.  Insert a new one.
        std::pair<TabRangeType::iterator, bool> r =
            m_TabRanges.insert(std::make_pair(nTab, std::make_unique<RangeListType>()));

        if (!r.second)
            // Insertion failed.
            return;

        itr = r.first;
    }
    itr->second->push_back(ScRange(a.Col(),a.Row(),a.Tab()));
}

void ScRangeListTabs::Append( const ScRange& aCRD, SCTAB nTab )
{
    ScRange a = aCRD;

    // ignore 3D ranges
    if (a.aStart.Tab() != a.aEnd.Tab())
        return;

    if (a.aStart.Tab() > MAXTAB)
        a.aStart.SetTab(MAXTAB);
    else if (a.aStart.Tab() < 0)
        a.aStart.SetTab(0);

    if (a.aStart.Col() > MAXCOL)
        a.aStart.SetCol(MAXCOL);
    else if (a.aStart.Col() < 0)
        a.aStart.SetCol(0);

    if (a.aStart.Row() > MAXROW)
        a.aStart.SetRow(MAXROW);
    else if (a.aStart.Row() < 0)
        a.aStart.SetRow(0);

    if (a.aEnd.Col() > MAXCOL)
        a.aEnd.SetCol(MAXCOL);
    else if (a.aEnd.Col() < 0)
        a.aEnd.SetCol(0);

    if (a.aEnd.Row() > MAXROW)
        a.aEnd.SetRow(MAXROW);
    else if (a.aEnd.Row() < 0)
        a.aEnd.SetRow(0);

    if( nTab == SCTAB_MAX)
        return;

    if( nTab < -1)
        nTab = a.aStart.Tab();

    if (nTab < 0 || MAXTAB < nTab)
        return;

    TabRangeType::iterator itr = m_TabRanges.find(nTab);
    if (itr == m_TabRanges.end())
    {
        // No entry for this table yet.  Insert a new one.
        std::pair<TabRangeType::iterator, bool> r =
            m_TabRanges.insert(std::make_pair(nTab, std::make_unique<RangeListType>()));

        if (!r.second)
            // Insertion failed.
            return;

        itr = r.first;
    }
    itr->second->push_back(a);
}

const ScRange* ScRangeListTabs::First( SCTAB n )
{
    OSL_ENSURE( ValidTab(n), "-ScRangeListTabs::First(): Good bye!" );

    TabRangeType::iterator itr = m_TabRanges.find(n);
    if (itr == m_TabRanges.end())
        // No range list exists for this table.
        return nullptr;

    const RangeListType& rList = *itr->second;
    maItrCur = rList.begin();
    maItrCurEnd = rList.end();
    return rList.empty() ? nullptr : &(*maItrCur);
}

const ScRange* ScRangeListTabs::Next ()
{
    ++maItrCur;
    if (maItrCur == maItrCurEnd)
        return nullptr;

    return &(*maItrCur);
}

ConverterBase::ConverterBase( svl::SharedStringPool& rSPool, sal_uInt16 nNewBuffer ) :
    aPool(rSPool),
    aEingPos( 0, 0, 0 )
{
    OSL_ENSURE( nNewBuffer > 0, "ConverterBase::ConverterBase - nNewBuffer == 0!" );
    pBuffer.reset( new sal_Char[ nNewBuffer ] );
}

ConverterBase::~ConverterBase()
{
}

void ConverterBase::Reset()
{
    aPool.Reset();
    aStack.Reset();
}

ExcelConverterBase::ExcelConverterBase( svl::SharedStringPool& rSPool ) :
    ConverterBase(rSPool, 512)
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

LotusConverterBase::LotusConverterBase( SvStream &rStr, svl::SharedStringPool& rSPool  ) :
    ConverterBase(rSPool, 128),
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
