/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusnapln.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:31:23 $
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

#include "fusnapln.hxx"

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif


#include "strings.hrc"

#include "sdattr.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
//CHINA001 #include "dlgsnap.hxx"
#include "sdenumdef.hxx" //CHINA001
#include "sdresid.hxx"
#include "sdabstdlg.hxx" //CHINA001
#include "dlgsnap.hrc" //CHINA001
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

namespace sd {

TYPEINIT1( FuSnapLine, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSnapLine::FuSnapLine(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                       SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuSnapLine::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSnapLine( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSnapLine::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    SdrPageView* pPV;
    USHORT  nHelpLine;
    BOOL    bCreateNew = TRUE;

    if ( !pArgs )
    {
        SfxItemSet aNewAttr(pViewShell->GetPool(), ATTR_SNAPLINE_START,
                                                ATTR_SNAPLINE_END);
        Point aLinePos = static_cast<DrawViewShell*>(pViewShell)->GetMousePos();
        static_cast<DrawViewShell*>(pViewShell)->SetMousePosFreezed( FALSE );
        BOOL bLineExist = FALSE;

        pPV = pView->GetSdrPageView();

        if ( aLinePos.X() >= 0 )
        {
            aLinePos = pWindow->PixelToLogic(aLinePos);
            USHORT nHitLog = (USHORT) pWindow->PixelToLogic(Size(HITPIX,0)).Width();
            bLineExist = pView->PickHelpLine(aLinePos, nHitLog, *pWindow,
                                             nHelpLine, pPV);
            if ( bLineExist )
                aLinePos = (pPV->GetHelpLines())[nHelpLine].GetPos();
            else
                pPV = pView->GetSdrPageView();

            pPV->LogicToPagePos(aLinePos);
        }
        else
            aLinePos = Point(0,0);

        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_X, aLinePos.X()));
        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, aLinePos.Y()));

        //CHINA001 SdSnapLineDlg* pDlg = new SdSnapLineDlg( NULL, aNewAttr, pView );
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
        DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
        AbstractSdSnapLineDlg* pDlg = pFact->CreateSdSnapLineDlg(ResId( DLG_SNAPLINE ), NULL, aNewAttr, pView );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
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

        pPV = pView->GetSdrPageView();

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

void FuSnapLine::Activate()
{
}

void FuSnapLine::Deactivate()
{
}

} // end of namespace sd
