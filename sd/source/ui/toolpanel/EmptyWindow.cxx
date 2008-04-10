/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EmptyWindow.cxx,v $
 * $Revision: 1.6 $
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

#include "EmptyWindow.hxx"

namespace sd { namespace toolpanel {


EmptyWindow::EmptyWindow (
        Window* pParentWindow,
        Color aBackgroundColor,
        const String& rText)
    : Window (pParentWindow, WinBits()),
      msText (rText)
{
    SetBackground (aBackgroundColor);
}




EmptyWindow::~EmptyWindow (void)
{
}




void EmptyWindow::Paint (const Rectangle& )
{
    Size aWindowSize (GetOutputSizePixel());
    Point aPosition(aWindowSize.Width()/2,aWindowSize.Height()/2);

    Rectangle aBBox;
    aPosition = Point(10,aWindowSize.Height()/2);
    if (GetTextBoundRect (aBBox, msText))
        aPosition = Point (
            (aWindowSize.Width()-aBBox.GetWidth())/2,
            (aWindowSize.Height()-aBBox.GetHeight())/2);
    DrawText (aPosition, msText);

    String sSizeString = String::CreateFromInt32(aWindowSize.Width());
    sSizeString.Append (String::CreateFromAscii(" x "));
    sSizeString.Append (String::CreateFromInt32(aWindowSize.Height()));
    if (GetTextBoundRect (aBBox, sSizeString))
        aPosition = Point (
            (aWindowSize.Width()-aBBox.GetWidth())/2,
            aPosition.Y() + GetTextHeight() + aBBox.GetHeight());
    DrawText (aPosition, sSizeString);
}




void EmptyWindow::Resize (void)
{
    Window::Resize();
}




void EmptyWindow::GetFocus (void)
{
    GetParent()->GrabFocus();
}


} } // end of namespace ::sd::tpv
