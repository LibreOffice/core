/*************************************************************************
 *
 *  $RCSfile: conrect.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-16 14:46:39 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef sdtacitm_hxx //autogen
#include <svx/sdtacitm.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SDTAGITM_HXX //autogen
#include <svx/sdtagitm.hxx>
#endif
#ifndef SDTAKITM_HXX //autogen
#include <svx/sdtakitm.hxx>
#endif
#ifndef _SDTADITM_HXX //autogen
#include <svx/sdtaditm.hxx>
#endif
#ifndef _SDTAAITM_HXX //autogen
#include <svx/sdtaaitm.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _CONRECT_HXX
#include <conrect.hxx>
#endif


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

ConstRectangle::ConstRectangle( SwWrtShell* pWrtShell, SwEditWin* pEditWin,
                                SwView* pSwView )
    : SwDrawBase( pWrtShell, pEditWin, pSwView ),
    bMarquee(FALSE)
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL ConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == TRUE
                                    && pWin->GetDrawMode() == OBJ_CAPTION)
    {
        pView->NoRotate();
        if (pView->IsDrawSelMode())
        {
            pView->FlipDrawSelMode();
            pSh->GetDrawView()->SetFrameDragSingles(pView->IsDrawSelMode());
        }
    }
    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL ConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));

    BOOL bRet = SwDrawBase::MouseButtonUp(rMEvt);

    if (bRet && pWin->GetDrawMode() == OBJ_TEXT)
    {
        SdrView *pSdrView = pSh->GetDrawView();

        if (bMarquee)
        {
            BOOL bNewMode = (::GetHtmlMode(pView->GetDocShell()) & HTMLMODE_ON) != 0;
                pSh->ChgAnchor(FLY_IN_CNTNT);

            const SdrMarkList& rMarkList = pSdrView->GetMarkList();

            if (rMarkList.GetMark(0))
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetObj();

                // die fuer das Scrollen benoetigten Attribute setzen
                SfxItemSet aItemSet( pSdrView->GetModel()->GetItemPool(),
                                        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                aItemSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
                aItemSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SCROLL ) );
                aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                aItemSet.Put( SdrTextAniCountItem( 0 ) );
                aItemSet.Put( SdrTextAniAmountItem((INT16)pWin->PixelToLogic(Size(2,1)).Width()) );

//-/                pObj->SetAttributes(aItemSet, FALSE);
                pObj->SetItemSetAndBroadcast(aItemSet);
            }
        }

        const SdrMarkList& rMarkList = pSdrView->GetMarkList();
        if (rMarkList.GetMark(0))
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
            SdrPageView* pPV = pSdrView->GetPageViewPvNum(0);
            pView->BeginTextEdit( pObj, pPV, pWin, TRUE );
        }

        pView->LeaveDrawCreate();   // In Selektionsmode wechseln
        pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
    }
    return bRet;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void ConstRectangle::Activate(const USHORT nSlotId)
{
    bMarquee = FALSE;

    switch (nSlotId)
    {
    case SID_DRAW_LINE:
        pWin->SetDrawMode(OBJ_LINE);
        break;

    case SID_DRAW_RECT:
        pWin->SetDrawMode(OBJ_RECT);
        break;

    case SID_DRAW_ELLIPSE:
        pWin->SetDrawMode(OBJ_CIRC);
        break;

    case SID_DRAW_TEXT_MARQUEE:
        bMarquee = TRUE;
        // no break
    case SID_DRAW_TEXT:
    case SID_DRAW_TEXT_VERTICAL:
        pWin->SetDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_CAPTION:
    case SID_DRAW_CAPTION_VERTICAL:
        pWin->SetDrawMode(OBJ_CAPTION);
        break;

    default:
        pWin->SetDrawMode(OBJ_NONE);
        break;
    }

    SwDrawBase::Activate(nSlotId);
}

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/30 12:08:27  aw
      change SdrObjects to use SfxItemSet instead of SfxSetItems.
      Removed TakeAttributes() and SetAttributes(), new ItemSet
      modification methods (GetItem[Set], SetItem[Set], ClearItem,...)

      Revision 1.1.1.1  2000/09/18 17:14:46  hr
      initial import

      Revision 1.45  2000/09/18 16:06:01  willem.vandorp
      OpenOffice header added.

      Revision 1.44  2000/09/07 15:59:26  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.43  1998/06/09 13:34:00  OM
      VC-Controls entfernt


      Rev 1.42   09 Jun 1998 15:34:00   OM
   VC-Controls entfernt

      Rev 1.41   28 Nov 1997 20:05:50   MA
   includes

      Rev 1.40   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.39   18 Nov 1997 10:35:12   OM
   Lauftext nur im HTML-Mode zeichengebunden einfuegen

      Rev 1.38   03 Nov 1997 13:24:22   MA
   precomp entfernt

      Rev 1.37   05 Sep 1997 11:55:04   MH
   chg: header

      Rev 1.36   07 Apr 1997 13:39:28   MH
   chg: header

      Rev 1.35   07 Mar 1997 17:13:38   OM
   Lauftext unendlich oft durchlaufen

      Rev 1.34   07 Mar 1997 17:05:12   OM
   Lauftext unendlich oft durchlaufen

      Rev 1.33   24 Feb 1997 16:55:48   OM
   Lauftext-Objekt sofort editieren

      Rev 1.32   14 Feb 1997 14:00:34   OM
   lauftextobjekt zeichengebunden

      Rev 1.31   11 Feb 1997 14:03:32   OM
   Lauftext im Webmode

      Rev 1.30   10 Feb 1997 17:29:42   OM
   Lauftext

      Rev 1.29   09 Sep 1996 21:13:44   MH
   Umstellungen wg. internal Compiler Errors

      Rev 1.28   28 Aug 1996 13:39:22   OS
   includes

      Rev 1.27   14 Aug 1996 11:31:00   JP
   svdraw.hxx entfernt

      Rev 1.26   07 Aug 1996 14:59:24   JP
   Umstellung fuer Upd. 330

      Rev 1.25   24 Nov 1995 16:58:04   OM
   PCH->PRECOMPILED

      Rev 1.24   15 Nov 1995 18:35:28   OM
   Virtuelle Fkt rausgeworfen

      Rev 1.23   14 Nov 1995 17:36:56   OM
   Cast entfernt

      Rev 1.22   08 Nov 1995 15:29:48   OM
   Invalidate-Reihenfolge geaendert

      Rev 1.21   08 Nov 1995 14:15:50   OS
   Change => Set

      Rev 1.20   08 Nov 1995 10:35:42   OM
   Invalidate optimiert

      Rev 1.19   26 Oct 1995 16:41:36   OM
   Leere Textobjekte beim Verlassen loeschen

      Rev 1.18   08 Oct 1995 18:06:46   OM
   Textobjekt nach einfuegen oeffnen

*************************************************************************/


