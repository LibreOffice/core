/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "zoomlist.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>

#include "ViewShell.hxx"

namespace sd {

#define MAX_ENTRYS  10

ZoomList::ZoomList(ViewShell* pViewShell)
: mpViewShell (pViewShell)
, mnCurPos(0)
{
}

void ZoomList::InsertZoomRect(const Rectangle& rRect)
{
    size_t nRectCount = maRectangles.size();

    if (nRectCount >= MAX_ENTRYS)
        maRectangles.erase(maRectangles.begin());
    else if (nRectCount == 0)
        mnCurPos = 0;
    else
        mnCurPos++;

    maRectangles.insert(maRectangles.begin()+mnCurPos,rRect);

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );
}

Rectangle ZoomList::GetNextZoomRect()
{
    mnCurPos++;
    size_t nRectCount = maRectangles.size();

    if (nRectCount > 0 && mnCurPos > nRectCount - 1)
        mnCurPos = nRectCount - 1;

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    return maRectangles[mnCurPos];
}

Rectangle ZoomList::GetPreviousZoomRect()
{
    if (mnCurPos > 0)
        mnCurPos--;

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_ZOOM_NEXT );
    rBindings.Invalidate( SID_ZOOM_PREV );

    return maRectangles[mnCurPos];
}

sal_Bool ZoomList::IsNextPossible() const
{
    size_t nRectCount = maRectangles.size();

    return nRectCount > 0 && mnCurPos < nRectCount - 1;
}

sal_Bool ZoomList::IsPreviousPossible() const
{
    return mnCurPos > 0;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
