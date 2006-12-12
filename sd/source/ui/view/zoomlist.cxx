/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomlist.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 19:24:28 $
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
#include "precompiled_sd.hxx"

#include "zoomlist.hxx"

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


#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif

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
|* Aktuelles ZoomRect herausgeben
|*
\************************************************************************/

Rectangle ZoomList::GetCurrentZoomRect() const
{
    Rectangle aRect(*(Rectangle*) GetObject(mnCurPos));
    return (aRect);
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
