/*************************************************************************
 *
 *  $RCSfile: fusnapln.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:36 $
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

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#pragma hdrstop

#include "strings.hrc"

#include "sdattr.hxx"
#include "sdview.hxx"
#include "viewshel.hxx"
#include "drviewsh.hxx"
#include "sdwindow.hxx"
#include "dlgsnap.hxx"
#include "sdresid.hxx"
#include "fusnapln.hxx"

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif


TYPEINIT1( FuSnapLine, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSnapLine::FuSnapLine(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                       SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    SdrPageView* pPV;
    USHORT  nHelpLine;
    BOOL    bCreateNew = TRUE;

    if ( !pArgs )
    {
        SfxItemSet aNewAttr(pViewSh->GetPool(), ATTR_SNAPLINE_START,
                                                ATTR_SNAPLINE_END);
        Point aLinePos = ((SdDrawViewShell*) pViewSh)->GetMousePos();
        ((SdDrawViewShell*) pViewSh)->SetMousePosFreezed( FALSE );
        BOOL bLineExist = FALSE;

        pPV = pView->GetPageViewPvNum(0);

        if ( aLinePos.X() >= 0 )
        {
            aLinePos = pWindow->PixelToLogic(aLinePos);
            USHORT nHitLog = (USHORT) pWindow->PixelToLogic(Size(HITPIX,0)).Width();
            bLineExist = pView->PickHelpLine(aLinePos, nHitLog, *pWindow,
                                             nHelpLine, pPV);
            if ( bLineExist )
                aLinePos = (pPV->GetHelpLines())[nHelpLine].GetPos();
            else
                pPV = pView->GetPageViewPvNum(0);

            pPV->LogicToPagePos(aLinePos);
        }
        else
            aLinePos = Point(0,0);

        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_X, aLinePos.X()));
        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, aLinePos.Y()));

        SdSnapLineDlg* pDlg = new SdSnapLineDlg( NULL, aNewAttr, pView );
        if ( bLineExist )
        {
            pDlg->HideRadioGroup();

            const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];

            if ( rHelpLine.GetKind() == SDRHELPLINE_POINT )
            {
                pDlg->SetText(String(SdResId(STR_SNAPDLG_SETPOINT)));
                pDlg->SetInputFields(TRUE, TRUE);
            }
            else
            {
                pDlg->SetText(String(SdResId(STR_SNAPDLG_SETLINE)));

                if ( rHelpLine.GetKind() == SDRHELPLINE_VERTICAL )
                    pDlg->SetInputFields(TRUE, FALSE);
                else
                    pDlg->SetInputFields(FALSE, TRUE);
            }
            bCreateNew = FALSE;
        }
        else
            pDlg->HideDeleteBtn();

        USHORT nResult = pDlg->Execute();

        pDlg->GetAttr(aNewAttr);
        delete pDlg;

        switch( nResult )
        {
            case RET_OK:
                rReq.Done(aNewAttr);
                pArgs = rReq.GetArgs();
                break;

            case RET_SNAP_DELETE:
                // Fangobjekt loeschen
                if ( !bCreateNew )
                    pPV->DeleteHelpLine(nHelpLine);
                // und weiter wie bei default
            default:
                return;
        }
    }
    Point aHlpPos;

    aHlpPos.X() = ((const SfxUInt32Item&) pArgs->Get(ATTR_SNAPLINE_X)).GetValue();
    aHlpPos.Y() = ((const SfxUInt32Item&) pArgs->Get(ATTR_SNAPLINE_Y)).GetValue();
    pPV->PagePosToLogic(aHlpPos);

    if ( bCreateNew )
    {
        SdrHelpLineKind eKind;

        pPV = pView->GetPageViewPvNum(0);

        switch ( (SnapKind) ((const SfxAllEnumItem&)
                 pArgs->Get(ATTR_SNAPLINE_KIND)).GetValue() )
        {
            case SK_HORIZONTAL  : eKind = SDRHELPLINE_HORIZONTAL;   break;
            case SK_VERTICAL    : eKind = SDRHELPLINE_VERTICAL;     break;
            default             : eKind = SDRHELPLINE_POINT;        break;
        }
        pPV->InsertHelpLine(SdrHelpLine(eKind, aHlpPos));
    }
    else
    {
        const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];
        pPV->SetHelpLine(nHelpLine, SdrHelpLine(rHelpLine.GetKind(), aHlpPos));
    }
}



