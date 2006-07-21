/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: invmerge.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 15:02:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <vcl/window.hxx>
#include <tools/debug.hxx>

#include "invmerge.hxx"

//------------------------------------------------------------------

ScInvertMerger::ScInvertMerger( Window* pWindow ) :
    pWin( pWindow )
{
    //  both rectangles empty
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
}

void ScInvertMerger::FlushTotal()
{
    if( aTotalRect.IsEmpty() )
        return;                         // nothing to do

    pWin->Invert( aTotalRect, INVERT_HIGHLIGHT );
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
    if ( aLineRect.IsEmpty() )
    {
        aLineRect = rRect;          // start new line rect
    }
    else
    {
        Rectangle aJustified = rRect;
        if ( rRect.Left() > rRect.Right() )     // switch for RTL layout
        {
            aJustified.Left() = rRect.Right();
            aJustified.Right() = rRect.Left();
        }

        BOOL bDone = FALSE;
        if ( aJustified.Top()    == aLineRect.Top()    &&
             aJustified.Bottom() == aLineRect.Bottom() )
        {
            // try to extend line rect
            if ( aJustified.Left() == aLineRect.Right() + 1 )
            {
                aLineRect.Right() = aJustified.Right();
                bDone = TRUE;
            }
            else if ( aJustified.Right() + 1 == aLineRect.Left() )  // for RTL layout
            {
                aLineRect.Left() = aJustified.Left();
                bDone = TRUE;
            }
        }
        if (!bDone)
        {
            FlushLine();                // use old line rect for total rect
            aLineRect = aJustified;     // and start new one
        }
    }
}




