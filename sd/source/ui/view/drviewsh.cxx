/*************************************************************************
 *
 *  $RCSfile: drviewsh.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2000-10-23 10:45:55 $
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

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
//#ifndef _IDETEMP_HXX
//#include <basctl/idetemp.hxx>
//#endif

#pragma hdrstop

#ifndef _SVX_FMSHELL_HXX            // XXX nur temp (dg)
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"
#include "sdpage.hxx"
#include "frmview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "sdwindow.hxx"
#include "drviewsh.hxx"
#include "grviewsh.hxx"
#include "drawview.hxx"

#define TABCONTROL_INITIAL_SIZE     500

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

BOOL SdDrawViewShell::GotoBookmark(const String& rBookmark)
{
    return (pDocSh->GotoBookmark(rBookmark));
}

/*************************************************************************
|*
|* Bereich sichtbar machen (Bildausschnitt scrollen)
|*
\************************************************************************/

void SdDrawViewShell::MakeVisible(const Rectangle& rRect, Window& rWin)
{
    Size aLogicSize(rRect.GetSize());

    // Sichtbarer Bereich
    Size aVisSizePixel(rWin.GetOutputSizePixel());
    Rectangle aVisArea(rWin.PixelToLogic(Rectangle(Point(0,0), aVisSizePixel)));
    Size aVisAreaSize(aVisArea.GetSize());

    if(!aVisArea.IsInside(rRect) && !pFuSlideShow)
    {
        // Objekt liegt nicht komplett im sichtbaren Bereich
        INT32 nFreeSpaceX(aVisAreaSize.Width() - aLogicSize.Width());
        INT32 nFreeSpaceY(aVisAreaSize.Height() - aLogicSize.Height());

        if(nFreeSpaceX < 0 || nFreeSpaceY < 0)
        {
            // Objekt passt nicht in sichtbaren Bereich -> auf Objektgroesse zoomen
            SetZoomRect(rRect);
        }
        else
        {
            // aVisArea needs to be moved to make rRect lie inside it
            INT32 nMaxMoveSize(((rRect.GetWidth() * 2L) + (rRect.GetHeight() * 2L)) / 2L);
            Point aPos(aVisArea.TopLeft());

            if(!nMaxMoveSize)
                nMaxMoveSize = 1;

            if(nFreeSpaceX > nMaxMoveSize)
                nFreeSpaceX = nMaxMoveSize;

            if(nFreeSpaceY > nMaxMoveSize)
                nFreeSpaceY = nMaxMoveSize;

            while(rRect.Right() > aPos.X() + aVisArea.GetWidth())
                aPos.X() += nFreeSpaceX;
            while(rRect.Left() < aPos.X())
                aPos.X() -= nFreeSpaceX;

            while(rRect.Top() < aPos.Y())
                aPos.Y() -= nFreeSpaceY;
            while(rRect.Bottom() > aPos.Y() + aVisArea.GetHeight())
                aPos.Y() += nFreeSpaceY;

            aVisArea.SetPos(aPos);
            SetZoomRect(aVisArea);
        }
    }
}


