/*************************************************************************
 *
 *  $RCSfile: scroll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "swtypes.hxx"
#include "swrect.hxx"
#include "scroll.hxx"


SwScrollbar::SwScrollbar( Window *pWin, int bHoriz ) :
    ScrollBar( pWin,
    WinBits( WB_3DLOOK | WB_HIDE | ( bHoriz ? WB_HSCROLL : WB_VSCROLL)  ) ),
    bHori( bHoriz ),
    bAuto( FALSE ),
    bThumbEnabled( TRUE ),
    bVisible(FALSE),
    bSizeSet(FALSE)
{
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
    const ULONG nVisSize = GetVisibleSize();
    SetLineSize( nVisSize * 10 / 100 );
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
    long nVis = GetVisibleSize();
    long nLen = GetRange().Len();
    if(bAuto)
        AutoShow();
}

void SwScrollbar::EnableThumbPos( BOOL bEnable, const SwRect &rVisArea )
{
    bThumbEnabled = bEnable;
    if ( bEnable )
    {
        long nThumb = bHori ? rVisArea.Left() : rVisArea.Top();
        SetThumbPos( nThumb );
    }
}

/*-----------------10/21/97 02:48pm-----------------

--------------------------------------------------*/
void SwScrollbar::Show( BOOL bSet )
{
    bVisible = bSet;
    if( (!bSet || bSizeSet) && IsUpdateMode() )
        ScrollBar::Show(bSet);
}

/*-----------------10/21/97 03:23pm-----------------

--------------------------------------------------*/
void SwScrollbar::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    ScrollBar::SetPosSizePixel(rNewPos, rNewSize);
    bSizeSet = TRUE;
    if(bVisible)
        Show();

}

/*-----------------10/21/97 04:47pm-----------------

--------------------------------------------------*/
void SwScrollbar::SetUpdateMode( BOOL bUpdate )
{
    ScrollBar::SetUpdateMode( bUpdate );
    if(bUpdate &&
        bVisible != ScrollBar::IsVisible())
        Show(bVisible);
}
/*-----------------14.04.98 11:38-------------------

--------------------------------------------------*/
void SwScrollbar::SetAuto(BOOL bSet)
{
    if(bAuto != bSet)
    {
        bAuto = bSet;

        // automatisch versteckt - dann anzeigen
        if(!bAuto && bVisible && !ScrollBar::IsVisible())
            Show(TRUE);
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
            if(bVisible)
                Hide();
        }
        else if ( !ScrollBar::IsVisible() &&
                  (!bHori || nVis) )        //Optimierung fuer Browser.
                                            //Horizontaler Scrollbar per
                                            //default aus.
        {
            Show();
        }
    }
}
/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.63  2000/09/18 16:06:11  willem.vandorp
      OpenOffice header added.

      Revision 1.62  2000/03/03 15:17:04  os
      StarView remainders removed

      Revision 1.61  1999/03/03 11:52:18  MA
      #62722# horizontaler Scrollbar bei Auto per default aus


      Rev 1.60   03 Mar 1999 12:52:18   MA
   #62722# horizontaler Scrollbar bei Auto per default aus

      Rev 1.59   14 Apr 1998 12:10:08   OS
   autom. Scrollbars: SetAuto() muss Sichtbarkeit ueberpruefen #49061#

      Rev 1.58   21 Nov 1997 15:00:12   MA
   includes

      Rev 1.57   12 Nov 1997 17:09:14   MBA
   SP3:

      Rev 1.56   03 Nov 1997 13:58:30   MA
   precomp entfernt

      Rev 1.55   22 Oct 1997 08:12:42   OS
   eigener UpdateMode zur Verhinderung des Flackerns #43684#

      Rev 1.54   10 Sep 1997 10:12:34   OS
   Scrollbars mit WB_HIDE erzeugen 43684

      Rev 1.53   17 Jul 1997 16:58:44   HJS
   includes

      Rev 1.52   16 Jul 1997 18:52:34   MA
   new: ThumbPos kann disabled werden

      Rev 1.51   29 Jul 1996 15:47:22   MA
   includes

      Rev 1.50   23 May 1996 17:18:20   OS
   automatische Scrollbars in Frame-Docs

      Rev 1.49   06 May 1996 17:01:16   MA
   chg: Scrollbars fuer browse und ole richtig

      Rev 1.48   14 Mar 1996 17:10:48   MA
   fix#26338#, richtige Reihenfolge der Einstellungen; Opts

      Rev 1.47   03 Mar 1996 18:16:04   MA
   opt: unuetzer Member vom Scrollbar

      Rev 1.46   29 Nov 1995 12:12:56   OM
   Scrollbars reaktiviert

      Rev 1.45   27 Nov 1995 21:21:04   JP
   ueberfluessige Methoden entfernt, optimiert

      Rev 1.44   24 Nov 1995 16:57:54   OM
   PCH->PRECOMPILED

      Rev 1.43   08 Nov 1995 13:07:40   OS
   Change => Set

      Rev 1.42   30 Oct 1995 12:06:04   MA
   chg: Get/Set Page/LineSize entfernt

      Rev 1.41   19 Feb 1995 15:35:58   JP
   CTOR: setze bHori richtig

*************************************************************************/


