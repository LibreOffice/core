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


#include "fuoutl.hxx"

#include <editeng/outliner.hxx>
#include "OutlineView.hxx"
#include "OutlineViewShell.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Command, weiterleiten an OutlinerView
|*
\************************************************************************/

bool FuOutline::Command(const CommandEvent& rCEvt)
{
    bool bResult = false;

    OutlinerView* pOlView =
        static_cast<OutlineView*>(mpView)->GetViewByWindow(mpWindow);
    DBG_ASSERT (pOlView, "keine OutlinerView gefunden");

    if (pOlView)
    {
        pOlView->Command(rCEvt);        // liefert leider keinen Returnwert
        bResult = true;
    }
    return bResult;
}

void FuOutline::ScrollStart()
{
}

void FuOutline::ScrollEnd()
{
}


} // end of namespace sd
