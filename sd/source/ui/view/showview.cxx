/*************************************************************************
 *
 *  $RCSfile: showview.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:23:30 $
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

#include "showview.hxx"
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif

#pragma hdrstop

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
    SetBufferedOutputAllowed(sal_True);

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
