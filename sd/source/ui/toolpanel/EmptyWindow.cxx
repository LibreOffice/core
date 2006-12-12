/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EmptyWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:39:47 $
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
