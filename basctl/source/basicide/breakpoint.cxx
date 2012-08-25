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

#include <stddef.h>

namespace basctl
{

BreakPointList::BreakPointList()
{}

BreakPointList::BreakPointList(BreakPointList const & rList)
{
    for (size_t i = 0; i < rList.size(); ++i)
        maBreakPoints.push_back( new BreakPoint(*rList.at( i ) ) );
}

BreakPointList::~BreakPointList()
{
    reset();
}

void BreakPointList::reset()
{
    for ( size_t i = 0, n = maBreakPoints.size(); i < n; ++i )
        delete maBreakPoints[ i ];
    maBreakPoints.clear();
}

void BreakPointList::transfer(BreakPointList & rList)
{
    reset();
    for (size_t i = 0; i < rList.size(); ++i)
        maBreakPoints.push_back( rList.at( i ) );
    rList.clear();
}

void BreakPointList::InsertSorted(BreakPoint* pNewBrk)
{
    for ( ::std::vector< BreakPoint* >::iterator i = maBreakPoints.begin(); i < maBreakPoints.end(); ++i )
    {
        if ( pNewBrk->nLine <= (*i)->nLine )
        {
            DBG_ASSERT( ( (*i)->nLine != pNewBrk->nLine ) || pNewBrk->bTemp, "BreakPoint existiert schon!" );
            maBreakPoints.insert( i, pNewBrk );
            return;
        }
    }
    // no insert position found => LIST_APPEND
    maBreakPoints.push_back( pNewBrk );
}

void BreakPointList::SetBreakPointsInBasic(SbModule* pModule)
{
    pModule->ClearAllBP();

    for ( size_t i = 0, n = maBreakPoints.size(); i < n; ++i )
    {
        BreakPoint* pBrk = maBreakPoints[ i ];
        if ( pBrk->bEnabled )
            pModule->SetBP( (sal_uInt16)pBrk->nLine );
    }
}

BreakPoint* BreakPointList::FindBreakPoint(size_t nLine)
{
    for ( size_t i = 0, n = maBreakPoints.size(); i < n; ++i )
    {
        BreakPoint* pBrk = maBreakPoints[ i ];
        if ( pBrk->nLine == nLine )
            return pBrk;
    }
    return NULL;
}

void BreakPointList::AdjustBreakPoints(size_t nLine, bool bInserted)
{
    for ( size_t i = 0; i < maBreakPoints.size(); )
    {
        BreakPoint* pBrk = maBreakPoints[ i ];
        bool bDelBrk = false;
        if ( pBrk->nLine == nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                bDelBrk = true;
        }
        else if ( pBrk->nLine > nLine )
        {
            if ( bInserted )
                pBrk->nLine++;
            else
                pBrk->nLine--;
        }

        if ( bDelBrk )
        {
            delete remove( pBrk );
        }
        else
        {
            ++i;
        }
    }
}

void BreakPointList::ResetHitCount()
{
    for ( size_t i = 0, n = maBreakPoints.size(); i < n; ++i )
    {
        BreakPoint* pBrk = maBreakPoints[ i ];
        pBrk->nHitCount = 0;
    }
}

BreakPoint* BreakPointList::remove(BreakPoint* ptr)
{
    for ( ::std::vector< BreakPoint* >::iterator i = maBreakPoints.begin(); i < maBreakPoints.end(); ++i )
    {
        if ( ptr == *i )
        {
            maBreakPoints.erase( i );
            return ptr;
        }
    }
    return NULL;
}

size_t BreakPointList::size() const
{
    return maBreakPoints.size();
}

BreakPoint* BreakPointList::at(size_t i)
{
    return i < maBreakPoints.size() ? maBreakPoints[ i ] : NULL;
}

const BreakPoint* BreakPointList::at(size_t i) const
{
    return i < maBreakPoints.size() ? maBreakPoints[ i ] : NULL;
}

void BreakPointList::clear()
{
    maBreakPoints.clear();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
