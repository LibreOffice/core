/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: showview.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:46:47 $
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

#include "showview.hxx"
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif


#include "drawdoc.hxx"
#include "sdpage.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif

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
      pViewSh(pViewShell),
      pWindowForPlugIns(pWin),
      nAllowInvalidateSmph(0),
      bAllowMasterPageCaching(TRUE)
{
    // #114898#
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);

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

void ShowView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, ::sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{
// #110094#-7
//  BOOL bMPCache = FALSE;

//  if (bAllowMasterPageCaching && pViewSh &&
//      pViewSh == (ViewShell*) SfxViewShell::Current() &&
//      pViewSh->GetFrameView()->IsMasterPagePaintCaching() &&
//      pOutDev->GetOutDevType() != OUTDEV_PRINTER)
//  {
//      // Aktive ViewShell: Caching einschalten
//      bMPCache = TRUE;
//  }

//  if (bMPCache)
//  {
//      if (!IsMasterPagePaintCaching())
//      {
//          SetMasterPagePaintCaching(TRUE);
//      }
//  }
//  else
//  {
//      if (IsMasterPagePaintCaching())
//      {
//          ReleaseMasterPagePaintCache();
//          SetMasterPagePaintCaching(FALSE);
//      }
//  }

    FmFormView::CompleteRedraw(pOutDev, rReg, SDRPAINTMODE_ANILIKEPRN, pRedirector);
}

/*************************************************************************
|*
|* DoConnect
|*
\************************************************************************/

void ShowView::DoConnect(SdrOle2Obj* pOleObj)
{
    // connected wird jetzt in FuSlideShow::ShowPlugIns()
}

} // end of namespace sd
