/*************************************************************************
 *
 *  $RCSfile: zoomlist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dl $ $Date: 2000-10-18 08:54:49 $
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

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXVIEWSHELL_HXX
#include <sfx2/viewsh.hxx>
#endif

#pragma hdrstop

#include "zoomlist.hxx"

#define MAX_ENTRYS  10

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

ZoomList::ZoomList( SfxViewShell* pViewShell )
    : List(),
    pViewSh( pViewShell ),
    nCurPos(0)
{
}


/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

ZoomList::~ZoomList()
{
#if ( defined GCC && defined C272 )
    for (ULONG nCount=0; nCount<List::Count(); nCount++)
#else
    for (ULONG nCount=0; nCount<Count(); nCount++)
#endif
    {
        // Ggf. ZoomRects loeschen
        delete ((Rectangle*) GetObject(nCount));
    }
}


/*************************************************************************
|*
|* Neues ZoomRect aufnehmen
|*
\************************************************************************/

void ZoomList::InsertZoomRect(const Rectangle& rRect)
{
    ULONG nCount = Count();

    if (nCount >= MAX_ENTRYS)
    {
        delete ((Rectangle*) GetObject(0));
        Remove((ULONG) 0);
    }
    else if (nCount == 0)
    {
        nCurPos = 0;
    }
    else
    {
        nCurPos++;
    }

    Rectangle* pRect = new Rectangle(rRect);
    Insert(pRect, nCurPos);

    SfxBindings& rBindings = pViewSh->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );
}


/*************************************************************************
|*
|* Aktuelles ZoomRect herausgeben
|*
\************************************************************************/

Rectangle ZoomList::GetCurrentZoomRect() const
{
    Rectangle aRect(*(Rectangle*) GetObject(nCurPos));
    return (aRect);
}

/*************************************************************************
|*
|* Naechstes ZoomRect herausgeben
|*
\************************************************************************/

Rectangle ZoomList::GetNextZoomRect()
{
    nCurPos++;
    ULONG nCount = Count();

    if (nCount > 0 && nCurPos > nCount - 1)
    {
        nCurPos = nCount - 1;
    }

    SfxBindings& rBindings = pViewSh->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    Rectangle aRect(*(Rectangle*) GetObject(nCurPos));
    return (aRect);
}

/*************************************************************************
|*
|* Letztes ZoomRect herausgeben
|*
\************************************************************************/

Rectangle ZoomList::GetPreviousZoomRect()
{
    if (nCurPos > 0)
    {
        nCurPos--;
    }

    SfxBindings& rBindings = pViewSh->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    Rectangle aRect(*(Rectangle*) GetObject(nCurPos));
    return (aRect);
}

/*************************************************************************
|*
|* Gibt es ein naechstes ZoomRect?
|*
\************************************************************************/

BOOL ZoomList::IsNextPossible() const
{
    BOOL bPossible = FALSE;
    ULONG nCount = Count();

    if (nCount > 0 && nCurPos < nCount - 1)
    {
        bPossible = TRUE;
    }

    return (bPossible);
}

/*************************************************************************
|*
|* Gibt es ein vorheriges ZoomRect?
|*
\************************************************************************/

BOOL ZoomList::IsPreviousPossible() const
{
    BOOL bPossible = FALSE;

    if (nCurPos > 0)
    {
        bPossible = TRUE;
    }

    return (bPossible);
}


