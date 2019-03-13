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

#include "breakpoint.hxx"

#include <basic/sbmod.hxx>
#include <tools/debug.hxx>


namespace basctl
{

BreakPointList::BreakPointList()
{}

BreakPointList::BreakPointList(BreakPointList const & rList)
{
    for (size_t i = 0; i < rList.size(); ++i)
        maBreakPoints.push_back( rList.at( i ) );
}

BreakPointList::~BreakPointList()
{
}

void BreakPointList::reset()
{
    maBreakPoints.clear();
}

void BreakPointList::transfer(BreakPointList & rList)
{
    maBreakPoints = std::move(rList.maBreakPoints);
}

void BreakPointList::InsertSorted(BreakPoint aNewBrk)
{
    auto it = std::find_if(maBreakPoints.begin(), maBreakPoints.end(),
        [&aNewBrk](const BreakPoint& rBreakPoint) { return aNewBrk.nLine <= rBreakPoint.nLine; });
    if (it != maBreakPoints.end())
    {
        DBG_ASSERT( it->nLine != aNewBrk.nLine, "BreakPoint exists already!" );
        maBreakPoints.insert( it, aNewBrk );
        return;
    }
    // no insert position found => LIST_APPEND
    maBreakPoints.push_back( aNewBrk );
}

void BreakPointList::SetBreakPointsInBasic(SbModule* pModule)
{
    pModule->ClearAllBP();

    for (BreakPoint& rBrk : maBreakPoints)
    {
        if ( rBrk.bEnabled )
            pModule->SetBP( rBrk.nLine );
    }
}

BreakPoint* BreakPointList::FindBreakPoint(sal_uInt16 nLine)
{
    for (BreakPoint& rBrk : maBreakPoints)
    {
        if ( rBrk.nLine == nLine )
            return &rBrk;
    }
    return nullptr;
}

void BreakPointList::AdjustBreakPoints(sal_uInt16 nLine, bool bInserted)
{
    for ( size_t i = 0; i < maBreakPoints.size(); )
    {
        BreakPoint& rBrk = maBreakPoints[ i ];
        bool bDelBrk = false;
        if ( rBrk.nLine == nLine )
        {
            if ( bInserted )
                rBrk.nLine++;
            else
                bDelBrk = true;
        }
        else if ( rBrk.nLine > nLine )
        {
            if ( bInserted )
                rBrk.nLine++;
            else
                rBrk.nLine--;
        }

        if ( bDelBrk )
        {
            maBreakPoints.erase(maBreakPoints.begin() + i);
        }
        else
        {
            ++i;
        }
    }
}

void BreakPointList::ResetHitCount()
{
    for (BreakPoint& rBrk : maBreakPoints)
    {
        rBrk.nHitCount = 0;
    }
}

void BreakPointList::remove(const BreakPoint* ptr)
{
    auto i = std::find_if(maBreakPoints.begin(), maBreakPoints.end(),
        [&ptr](const BreakPoint& rBreakPoint) { return ptr == &rBreakPoint; });
    if (i != maBreakPoints.end())
        maBreakPoints.erase( i );
    return;
}

void BreakPointList::remove(size_t idx)
{
    maBreakPoints.erase( maBreakPoints.begin() + idx );
}

size_t BreakPointList::size() const
{
    return maBreakPoints.size();
}

BreakPoint& BreakPointList::at(size_t i)
{
    return maBreakPoints[ i ];
}

const BreakPoint& BreakPointList::at(size_t i) const
{
    return maBreakPoints[ i ];
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
