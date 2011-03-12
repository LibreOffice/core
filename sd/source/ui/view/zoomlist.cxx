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
#include "precompiled_sd.hxx"

#include "zoomlist.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>


#include "ViewShell.hxx"

namespace sd {

#define MAX_ENTRYS  10

/*************************************************************************
|*
|* Constructor
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
|* Destructor
|*
\************************************************************************/

ZoomList::~ZoomList()
{
#if ( defined GCC && defined C272 )
    for (sal_uLong nObject=0; nObject<List::Count(); nObject++)
#else
    for (sal_uLong nObject=0; nObject<Count(); nObject++)
#endif
    {
        // delete ZoomRects if necessary
        delete ((Rectangle*) GetObject(nObject));
    }
}


/*************************************************************************
|*
|* Insert new ZoomRect
|*
\************************************************************************/

void ZoomList::InsertZoomRect(const Rectangle& rRect)
{
    sal_uLong nRectCount = Count();

    if (nRectCount >= MAX_ENTRYS)
    {
        delete ((Rectangle*) GetObject(0));
        Remove((sal_uLong) 0);
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
|* Return next ZoomRect
|*
\************************************************************************/

Rectangle ZoomList::GetNextZoomRect()
{
    mnCurPos++;
    sal_uLong nRectCount = Count();

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
|* Return last ZoomRect
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
|* Is there a next ZoomRect?
|*
\************************************************************************/

sal_Bool ZoomList::IsNextPossible() const
{
    sal_Bool bPossible = sal_False;
    sal_uLong nRectCount = Count();

    if (nRectCount > 0 && mnCurPos < nRectCount - 1)
    {
        bPossible = sal_True;
    }

    return (bPossible);
}

/*************************************************************************
|*
|* Is there a previous ZoomRect?
|*
\************************************************************************/

sal_Bool ZoomList::IsPreviousPossible() const
{
    sal_Bool bPossible = sal_False;

    if (mnCurPos > 0)
    {
        bPossible = sal_True;
    }

    return (bPossible);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
