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

#include "fuoutl.hxx"

#include <editeng/outliner.hxx>
#include "OutlineView.hxx"
#include "OutlineViewShell.hxx"
#include "Window.hxx"

namespace sd {


FuOutline::FuOutline (
    ViewShell* pViewShell,
    ::sd::Window* pWindow,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewShell, pWindow, pView, pDoc, rReq),
      pOutlineViewShell (static_cast<OutlineViewShell*>(pViewShell)),
      pOutlineView (static_cast<OutlineView*>(pView))
{
}

/**
 * forward to OutlinerView
 */
bool FuOutline::Command(const CommandEvent& rCEvt)
{
    bool bResult = false;

    OutlinerView* pOlView =
        static_cast<OutlineView*>(mpView)->GetViewByWindow(mpWindow);
    DBG_ASSERT (pOlView, "no OutlineView found");

    if (pOlView)
    {
        pOlView->Command(rCEvt);        // unfortunately, we do not get a return value
        bResult = true;
    }
    return bResult;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
