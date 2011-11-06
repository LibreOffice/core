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
#include "precompiled_sw.hxx"



#include "swtypes.hxx"
#include "swrect.hxx"
#include "scroll.hxx"

#define SCROLL_LINE_SIZE 250


SwScrollbar::SwScrollbar( Window *pWin, sal_Bool bHoriz ) :
    ScrollBar( pWin,
    WinBits( WB_3DLOOK | WB_HIDE | ( bHoriz ? WB_HSCROLL : WB_VSCROLL)  ) ),
    bHori( bHoriz ),
    bAuto( sal_False ),
    bThumbEnabled( sal_True ),
    bVisible(sal_False),
    bSizeSet(sal_False)
{
    // SSA: --- RTL --- no mirroring for horizontal scrollbars
    if( bHoriz )
        EnableRTL( sal_False );
}


 SwScrollbar::~SwScrollbar() {}

/*------------------------------------------------------------------------
 Beschreibung:  wird nach einer Aenderung der Dokumentgroesse gerufen, um den
                Range des Scrollbars neu einzustellen.
------------------------------------------------------------------------*/

void SwScrollbar::DocSzChgd( const Size &rSize )
{
    aDocSz = rSize;
    SetRange( Range( 0, bHori ? rSize.Width() : rSize.Height()) );
    const sal_uLong nVisSize = GetVisibleSize();
    SetLineSize( SCROLL_LINE_SIZE );
//    SetLineSize( nVisSize * 10 / 100 );
    SetPageSize( nVisSize * 77 / 100 );
}

/*------------------------------------------------------------------------
 Beschreibung:  wird nach einer Veraenderung des sichtbaren Ausschnittes
                gerufen.
------------------------------------------------------------------------*/


void SwScrollbar::ViewPortChgd( const Rectangle &rRect )
{
    long nThumb, nVisible;
    if( bHori )
    {
        nThumb = rRect.Left();
        nVisible = rRect.GetWidth();
    }
    else
    {
        nThumb = rRect.Top();
        nVisible = rRect.GetHeight();
    }

    SetVisibleSize( nVisible );
    DocSzChgd(aDocSz);
    if ( bThumbEnabled )
        SetThumbPos( nThumb );
    if(bAuto)
        AutoShow();
}

/*-----------------10/21/97 02:48pm-----------------

--------------------------------------------------*/
void SwScrollbar::ExtendedShow( sal_Bool bSet )
{
    bVisible = bSet;
    if( (!bSet ||  !bAuto) && IsUpdateMode() && bSizeSet)
        ScrollBar::Show(bSet);
}

/*-----------------10/21/97 03:23pm-----------------

--------------------------------------------------*/
void SwScrollbar::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    ScrollBar::SetPosSizePixel(rNewPos, rNewSize);
    bSizeSet = sal_True;
    if(bVisible)
        ExtendedShow();

}


/*-----------------14.04.98 11:38-------------------

--------------------------------------------------*/
void SwScrollbar::SetAuto(sal_Bool bSet)
{
    if(bAuto != bSet)
    {
        bAuto = bSet;

        // automatisch versteckt - dann anzeigen
        if(!bAuto && bVisible && !ScrollBar::IsVisible())
            ExtendedShow(sal_True);
        else if(bAuto)
            AutoShow(); // oder automatisch verstecken
    }
}
/*-----------------14.04.98 11:43-------------------

--------------------------------------------------*/
void SwScrollbar::AutoShow()
{
    long nVis = GetVisibleSize();
    long nLen = GetRange().Len();
    {
        if( nVis >= nLen - 1)
        {
            if(ScrollBar::IsVisible())
                ScrollBar::Show(sal_False);
        }
        else if ( !ScrollBar::IsVisible() &&
                  (!bHori || nVis) )        //Optimierung fuer Browser.
                                            //Horizontaler Scrollbar per
                                            //default aus.
        {
            ScrollBar::Show(sal_True);
        }
    }
}
