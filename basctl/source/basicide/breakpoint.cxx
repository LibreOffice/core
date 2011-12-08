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

#include "breakpoint.hxx"

#include <basic/sbmod.hxx>
#include <tools/debug.hxx>

#include <stddef.h>

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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
