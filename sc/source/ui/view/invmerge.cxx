/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        sal_Bool bDone = sal_False;
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




