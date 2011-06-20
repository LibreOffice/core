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

#include "NotesDockingWindow.hxx"
#include "NotesChildWindow.hrc"

#include "EditWindow.hxx"
#include "TextLogger.hxx"
#include <svx/dlgctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include "DrawViewShell.hxx"
#include "drawdoc.hxx"
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
        mpEditWindow = new EditWindow (this, &pDocument->GetPool());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
