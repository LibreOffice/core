/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuoutl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-12-14 17:01:34 $
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

#pragma hdrstop

#include "fuoutl.hxx"

#include <svx/outliner.hxx>

#ifndef SD_OUTLINE_VIEW_HXX
#include "OutlineView.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif

namespace sd {

TYPEINIT1( FuOutline, FuPoor );

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

FunctionReference FuOutline::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuOutline( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

/*************************************************************************
|*
|* Command, weiterleiten an OutlinerView
|*
\************************************************************************/

BOOL FuOutline::Command(const CommandEvent& rCEvt)
{
    BOOL bResult = FALSE;

    OutlinerView* pOlView =
        static_cast<OutlineView*>(pView)->GetViewByWindow(pWindow);
    DBG_ASSERT (pOlView, "keine OutlinerView gefunden");

    if (pOlView)
    {
        pOlView->Command(rCEvt);        // liefert leider keinen Returnwert
        bResult = TRUE;
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
