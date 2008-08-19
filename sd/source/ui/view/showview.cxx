/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: showview.cxx,v $
 * $Revision: 1.15 $
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

#include "showview.hxx"
#include <svx/svdmodel.hxx>
#include <svx/fmview.hxx>


#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "FrameView.hxx"

namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVINPLACECLIENT_DEFINED
#define SO2_DECL_SVINPLACECLIENT_DEFINED
SO2_DECL_REF(SvInPlaceClient)
#endif

/*************************************************************************
|*
|* der Konstruktor setzt den MapMode und arrangiert die einzelnen Seiten
|*
\************************************************************************/

ShowView::ShowView (
    SdDrawDocument* pDoc,
    OutputDevice* pOut,
    ViewShell* pViewShell,
    ::Window* pWin)
    : FmFormView(pDoc, pOut),
      pDrDoc(pDoc),
      mpViewSh(pViewShell),
      pWindowForPlugIns(pWin),
      nAllowInvalidateSmph(0),
      bAllowMasterPageCaching(TRUE)
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_DrawImpress());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_DrawImpress());

    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

ShowView::~ShowView()
{
}

/*************************************************************************
|*
|* Zaehler fuer Invalidierungserlaubnis hoch- oder runter zaehlen
|*
\************************************************************************/

void ShowView::SetAllowInvalidate(BOOL bFlag)
{
    if (!bFlag)
    {
        nAllowInvalidateSmph++;
    }
    else if (nAllowInvalidateSmph > 0)
    {
        nAllowInvalidateSmph--;
    }
}

/*************************************************************************
|*
|* ermittelt, ob invalidiert werden darf
|*
\************************************************************************/

BOOL ShowView::IsInvalidateAllowed() const
{
    return (nAllowInvalidateSmph == 0);
}

/*************************************************************************
|*
|* Invalidate abfangen
|*
\************************************************************************/

void ShowView::InvalidateOneWin (::Window& rWin)
{
    if (IsInvalidateAllowed())
    {
        FmFormView::InvalidateOneWin(rWin);
    }
}

/*************************************************************************
|*
|* Invalidate abfangen
|*
\************************************************************************/

void ShowView::InvalidateOneWin (::Window& rWin, const Rectangle& rRect)
{
    if (IsInvalidateAllowed())
    {
        FmFormView::InvalidateOneWin(rWin, rRect);
    }
}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird an die View weitergeleitet
|*
\************************************************************************/

void ShowView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{
    FmFormView::CompleteRedraw(pOutDev, rReg, pRedirector);
}

/*************************************************************************
|*
|* DoConnect
|*
\************************************************************************/

void ShowView::DoConnect(SdrOle2Obj* )
{
    // connected wird jetzt in FuSlideShow::ShowPlugIns()
}

} // end of namespace sd
