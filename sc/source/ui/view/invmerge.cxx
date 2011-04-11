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



#include <vcl/window.hxx>
#include <tools/debug.hxx>

#include "invmerge.hxx"

//------------------------------------------------------------------

ScInvertMerger::ScInvertMerger( Window* pWindow ) :
    pWin( pWindow ),
    pRects( NULL )
{
    //  both rectangles empty
}

ScInvertMerger::ScInvertMerger( ::std::vector< Rectangle >* pRectangles ) :
    pWin( NULL ),
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

    DBG_ASSERT( aLineRect.IsEmpty() && aTotalRect.IsEmpty(), "Flush: not empty" );

    if ( pRects )
    {
        //
        // also join vertically if there are non-adjacent columns involved
        //

        size_t nComparePos = 0;
        while ( nComparePos < pRects->size() )
        {
            Rectangle aCompRect = (*pRects)[nComparePos];
            sal_Int32 nBottom = aCompRect.Bottom();
            size_t nOtherPos = nComparePos + 1;

            while ( nOtherPos < pRects->size() )
            {
                Rectangle aOtherRect = (*pRects)[nOtherPos];
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
                    aCompRect.Bottom() = nBottom;
                    (*pRects)[nComparePos].Bottom() = nBottom;

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
}

void ScInvertMerger::FlushTotal()
{
    if( aTotalRect.IsEmpty() )
        return;                         // nothing to do

    if ( pWin )
        pWin->Invert( aTotalRect, INVERT_HIGHLIGHT );
    else if ( pRects )
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
            aTotalRect.Bottom() = aLineRect.Bottom();
        }
        else
        {
            FlushTotal();                   // draw old total rect
            aTotalRect = aLineRect;         // and start new one
        }
    }

    aLineRect.SetEmpty();
}

void ScInvertMerger::AddRect( const Rectangle& rRect )
{
    Rectangle aJustified = rRect;
    if ( rRect.Left() > rRect.Right() )     // switch for RTL layout
    {
        aJustified.Left() = rRect.Right();
        aJustified.Right() = rRect.Left();
    }

    if ( aLineRect.IsEmpty() )
    {
        aLineRect = aJustified;             // start new line rect
    }
    else
    {
        sal_Bool bDone = false;
        if ( aJustified.Top()    == aLineRect.Top()    &&
             aJustified.Bottom() == aLineRect.Bottom() )
        {
            // try to extend line rect
            if ( aJustified.Left() == aLineRect.Right() + 1 )
            {
                aLineRect.Right() = aJustified.Right();
                bDone = sal_True;
            }
            else if ( aJustified.Right() + 1 == aLineRect.Left() )  // for RTL layout
            {
                aLineRect.Left() = aJustified.Left();
                bDone = sal_True;
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
