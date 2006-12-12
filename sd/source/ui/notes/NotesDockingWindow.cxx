/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NotesDockingWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:03:34 $
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

#include "NotesDockingWindow.hxx"
#include "NotesChildWindow.hrc"

#include "EditWindow.hxx"
#include "TextLogger.hxx"

#ifndef _SVX_DLG_CTRL_HXX
#include <svx/dlgctrl.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_DOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

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

