/*************************************************************************
 *
 *  $RCSfile: fuoutl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 11:08:47 $
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

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuOutline::~FuOutline()
{
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



} // end of namespace sd
