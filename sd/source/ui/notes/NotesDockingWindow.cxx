/*************************************************************************
 *
 *  $RCSfile: NotesDockingWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:08:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    SfxBindings *pBindings,
    SfxChildWindow *pChildWindow,
    Window* pParent)
    : SfxDockingWindow (pBindings, pChildWindow, pParent,
        SdResId(FLT_WIN_NOTES))
{
    ViewShellBase* pBase = ViewShellBase::GetViewShellBase (
        pBindings->GetDispatcher()->GetFrame());
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

