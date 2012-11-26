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

#include "NotesDockingWindow.hxx"
#include "NotesChildWindow.hrc"

#include "EditWindow.hxx"
#include "TextLogger.hxx"
#include <svx/dlgctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include "DrawViewShell.hxx"
#ifndef SD_DRAW_DOC_HXX
#include "drawdoc.hxx"
#endif
#include "sdresid.hxx"
#include "ViewShellBase.hxx"

namespace sd { namespace notes {

NotesDockingWindow::NotesDockingWindow (
    SfxBindings *_pBindings,
    SfxChildWindow *pChildWindow,
    Window* _pParent)
    : SfxDockingWindow (_pBindings, pChildWindow, _pParent,
        SdResId(FLT_WIN_NOTES))
{
    ViewShellBase* pBase = ViewShellBase::GetViewShellBase (
        _pBindings->GetDispatcher()->GetFrame());
    if (pBase != NULL)
    {
        SdDrawDocument* pDocument = pBase->GetDocument();
        mpEditWindow = new EditWindow (this, &pDocument->GetItemPool());
        mpEditWindow->Show();
        TextLogger::Instance().ConnectToEditWindow (mpEditWindow);
    }
}




NotesDockingWindow::~NotesDockingWindow (void)
{
    delete mpEditWindow;
}




void NotesDockingWindow::Paint (const Rectangle& rBoundingBox)
{
    SfxDockingWindow::Paint (rBoundingBox);

    Size aWindowSize (GetOutputSizePixel());
    Rectangle aBBox;
    Point aPosition(10,aWindowSize.Height()/2);
    String aString (UniString::CreateFromAscii("Bottom Panel Docking Window"));
    if (GetTextBoundRect (aBBox, aString))
        aPosition = Point (
            (aWindowSize.Width()-aBBox.GetWidth())/2,
            (aWindowSize.Height()-aBBox.GetHeight())/2);

    DrawText (aPosition, aString);
}




void NotesDockingWindow::Resize (void)
{
    SfxDockingWindow::Resize();

    Size aWinSize (GetOutputSizePixel());

    if (mpEditWindow != NULL)
    {
        mpEditWindow->SetPosSizePixel (
            Point(0,0),
            aWinSize);
    }
}



} } // end of namespace ::sd::notes

