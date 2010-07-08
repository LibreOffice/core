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
#include "precompiled_sd.hxx"

#include "zoomlist.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#ifndef _SFXVIEWSHELL_HXX
#include <sfx2/viewsh.hxx>
#endif


#include "ViewShell.hxx"

namespace sd {

#define MAX_ENTRYS  10

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

ZoomList::ZoomList(ViewShell* pViewShell)
: List()
, mpViewShell (pViewShell)
, mnCurPos(0)
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
    for (ULONG nObject=0; nObject<List::Count(); nObject++)
#else
    for (ULONG nObject=0; nObject<Count(); nObject++)
#endif
    {
        // Ggf. ZoomRects loeschen
        delete ((Rectangle*) GetObject(nObject));
    }
}


/*************************************************************************
|*
|* Neues ZoomRect aufnehmen
|*
\************************************************************************/

void ZoomList::InsertZoomRect(const Rectangle& rRect)
{
    ULONG nRectCount = Count();

    if (nRectCount >= MAX_ENTRYS)
    {
        delete ((Rectangle*) GetObject(0));
        Remove((ULONG) 0);
    }
    else if (nRectCount == 0)
    {
        mnCurPos = 0;
    }
    else
    {
        mnCurPos++;
    }

    Rectangle* pRect = new Rectangle(rRect);
    Insert(pRect, mnCurPos);

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );
}

/*************************************************************************
|*
|* Naechstes ZoomRect herausgeben
|*
\************************************************************************/

Rectangle ZoomList::GetNextZoomRect()
{
    mnCurPos++;
    ULONG nRectCount = Count();

    if (nRectCount > 0 && mnCurPos > nRectCount - 1)
    {
        mnCurPos = nRectCount - 1;
    }

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    Rectangle aRect(*(Rectangle*) GetObject(mnCurPos));
    return (aRect);
}

/*************************************************************************
|*
|* Letztes ZoomRect herausgeben
|*
\************************************************************************/

Rectangle ZoomList::GetPreviousZoomRect()
{
    if (mnCurPos > 0)
    {
        mnCurPos--;
    }

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    Rectangle aRect(*(Rectangle*) GetObject(mnCurPos));
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
    ULONG nRectCount = Count();

    if (nRectCount > 0 && mnCurPos < nRectCount - 1)
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

    if (mnCurPos > 0)
    {
        bPossible = TRUE;
    }

    return (bPossible);
}

} // end of namespace sd
