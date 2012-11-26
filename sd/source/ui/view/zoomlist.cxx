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
    for (sal_uLong nObject=0; nObject<List::Count(); nObject++)
#else
    for (sal_uLong nObject=0; nObject<Count(); nObject++)
#endif
    {
        // Ggf. ZoomRects loeschen
        delete ((basegfx::B2DRange*) GetObject(nObject));
    }
}


/*************************************************************************
|*
|* Neues ZoomRect aufnehmen
|*
\************************************************************************/

void ZoomList::InsertZoomRange(const basegfx::B2DRange& rRange)
{
    const sal_uInt32 nRangeCount(Count());

    if (nRangeCount >= MAX_ENTRYS)
    {
        delete ((basegfx::B2DRange*) GetObject(0));
        Remove((sal_uIntPtr) 0);
    }
    else if (nRangeCount == 0)
    {
        mnCurPos = 0;
    }
    else
    {
        mnCurPos++;
    }

    basegfx::B2DRange* pRange = new basegfx::B2DRange(rRange);
    Insert(pRange, mnCurPos);

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );
}

/*************************************************************************
|*
|* Naechstes ZoomRect herausgeben
|*
\************************************************************************/

basegfx::B2DRange ZoomList::GetNextZoomRange()
{
    mnCurPos++;
    const sal_uInt32 nRangeCount(Count());

    if (nRangeCount > 0 && mnCurPos > nRangeCount - 1)
    {
        mnCurPos = nRangeCount - 1;
    }

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    basegfx::B2DRange aRange(*(basegfx::B2DRange*)GetObject(mnCurPos));
    return aRange;
}

/*************************************************************************
|*
|* Letztes ZoomRect herausgeben
|*
\************************************************************************/

basegfx::B2DRange ZoomList::GetPreviousZoomRange()
{
    if (mnCurPos > 0)
    {
        mnCurPos--;
    }

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    basegfx::B2DRange aRange(*(basegfx::B2DRange*) GetObject(mnCurPos));
    return aRange;
}

/*************************************************************************
|*
|* Gibt es ein naechstes ZoomRect?
|*
\************************************************************************/

bool ZoomList::IsNextPossible() const
{
    bool bPossible = false;
    const sal_uInt32 nRangeCount(Count());

    if (nRangeCount > 0 && mnCurPos < nRangeCount - 1)
    {
        bPossible = true;
    }

    return bPossible;
}

/*************************************************************************
|*
|* Gibt es ein vorheriges ZoomRect?
|*
\************************************************************************/

bool ZoomList::IsPreviousPossible() const
{
    bool bPossible = false;

    if (mnCurPos > 0)
    {
        bPossible = true;
    }

    return (bPossible);
}

} // end of namespace sd
