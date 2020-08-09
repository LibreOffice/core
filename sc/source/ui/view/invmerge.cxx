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

#include <osl/diagnose.h>

#include <invmerge.hxx>

ScInvertMerger::ScInvertMerger( ::std::vector< tools::Rectangle >* pRectangles ) :
    pRects( pRectangles )
{
    //  collect rectangles instead of inverting
}

ScInvertMerger::~ScInvertMerger()
{
    Flush();
}

void ScInvertMerger::Flush()
{
    FlushLine();
    FlushTotal();

    OSL_ENSURE( aLineRect.IsEmpty() && aTotalRect.IsEmpty(), "Flush: not empty" );

    if ( !pRects )
        return;

    // also join vertically if there are non-adjacent columns involved

    size_t nComparePos = 0;
    while ( nComparePos < pRects->size() )
    {
        tools::Rectangle aCompRect = (*pRects)[nComparePos];
        sal_Int32 nBottom = aCompRect.Bottom();
        size_t nOtherPos = nComparePos + 1;

        while ( nOtherPos < pRects->size() )
        {
            tools::Rectangle aOtherRect = (*pRects)[nOtherPos];
            if ( aOtherRect.Top() > nBottom + 1 )
            {
                // rectangles are sorted, so we can stop searching
                break;
            }
            if ( aOtherRect.Top() == nBottom + 1 &&
                 aOtherRect.Left() == aCompRect.Left() &&
                 aOtherRect.Right() == aCompRect.Right() )
            {
                // extend first rectangle
                nBottom = aOtherRect.Bottom();
                aCompRect.SetBottom( nBottom );
                (*pRects)[nComparePos].SetBottom( nBottom );

                // remove second rectangle
                pRects->erase( pRects->begin() + nOtherPos );

                // continue at unmodified nOtherPos
            }
            else
                ++nOtherPos;
        }

        ++nComparePos;
    }
}

void ScInvertMerger::FlushTotal()
{
    if( aTotalRect.IsEmpty() )
        return;                         // nothing to do

    if ( pRects )
        pRects->push_back( aTotalRect );

    aTotalRect.SetEmpty();
}

void ScInvertMerger::FlushLine()
{
    if( aLineRect.IsEmpty() )
        return;                         // nothing to do

    if ( aTotalRect.IsEmpty() )
    {
        aTotalRect = aLineRect;         // start new total rect
    }
    else
    {
        if ( aLineRect.Left()  == aTotalRect.Left()  &&
             aLineRect.Right() == aTotalRect.Right() &&
             aLineRect.Top()   == aTotalRect.Bottom() + 1 )
        {
            // extend total rect
            aTotalRect.SetBottom( aLineRect.Bottom() );
        }
        else
        {
            FlushTotal();                   // draw old total rect
            aTotalRect = aLineRect;         // and start new one
        }
    }

    aLineRect.SetEmpty();
}

void ScInvertMerger::AddRect( const tools::Rectangle& rRect )
{
    tools::Rectangle aJustified = rRect;
    if ( rRect.Left() > rRect.Right() )     // switch for RTL layout
    {
        aJustified.SetLeft( rRect.Right() );
        aJustified.SetRight( rRect.Left() );
    }

    if ( aLineRect.IsEmpty() )
    {
        aLineRect = aJustified;             // start new line rect
    }
    else
    {
        bool bDone = false;
        if ( aJustified.Top()    == aLineRect.Top()    &&
             aJustified.Bottom() == aLineRect.Bottom() )
        {
            // try to extend line rect
            if ( aJustified.Left() == aLineRect.Right() + 1 )
            {
                aLineRect.SetRight( aJustified.Right() );
                bDone = true;
            }
            else if ( aJustified.Right() + 1 == aLineRect.Left() )  // for RTL layout
            {
                aLineRect.SetLeft( aJustified.Left() );
                bDone = true;
            }
        }
        if (!bDone)
        {
            FlushLine();                // use old line rect for total rect
            aLineRect = aJustified;     // and start new one
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
