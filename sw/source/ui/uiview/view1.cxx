/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sw.hxx"


#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/ruler.hxx>
#include <idxmrk.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#include <navipi.hxx>
#include <fldwrap.hxx>
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#include <dpage.hxx>
#include <edtwin.hxx>
#include "formatclipboard.hxx"
#ifndef _CMDID_H
#include <cmdid.h>
#endif
// header for class SfxRequest
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>

extern int bDocSzUpdated;


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwView::Activate(sal_Bool bMDIActivate)
{
    // aktuelle View anmelden an der DocShell
    // die View bleibt solange an der DocShell
    // aktiv bis Sie zerstoert wird oder durch Activate eine
    // neue gesetzt wird
    SwDocShell* pDocSh = GetDocShell();
    if(pDocSh)
        pDocSh->SetView(this);
    SwModule* pSwMod = SW_MOD();
    pSwMod->SetView(this);

    // Dokumentgroesse hat sich geaendert
    if(!bDocSzUpdated)
        DocSzChgd(aDocSz);

    // #b6330459# make selection visible
    if(bMakeSelectionVisible)
    {
        pWrtShell->MakeSelVisible();
        bMakeSelectionVisible = sal_False;
    }
    pHRuler->SetActive( sal_True );
    pVRuler->SetActive( sal_True );

    if ( bMDIActivate )
    {
        pWrtShell->ShGetFcs(sal_False);     // Selektionen sichtbar

        if( sSwViewData.Len() )
        {
            ReadUserData( sSwViewData, sal_False );
            sSwViewData.Erase();
        }

        AttrChangedNotify(pWrtShell);

        // Flddlg ggf neu initialisieren (z.B. fuer TYP_SETVAR)
        sal_uInt16 nId = SwFldDlgWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = GetViewFrame();
        SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
        if (pWrp)
            pWrp->ReInitDlg(GetDocShell());

        // RedlineDlg ggf neu initialisieren
        nId = SwRedlineAcceptChild::GetChildWindowId();
        SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->GetChildWindow(nId);
        if (pRed)
            pRed->ReInitDlg(GetDocShell());

        // reinit IdxMarkDlg
        nId = SwInsertIdxMarkWrapper::GetChildWindowId();
        SwInsertIdxMarkWrapper *pIdxMrk = (SwInsertIdxMarkWrapper*)pVFrame->GetChildWindow(nId);
        if (pIdxMrk)
            pIdxMrk->ReInitDlg(*pWrtShell);

        // reinit AuthMarkDlg
        nId = SwInsertAuthMarkWrapper::GetChildWindowId();
        SwInsertAuthMarkWrapper *pAuthMrk = (SwInsertAuthMarkWrapper*)pVFrame->
                                                                GetChildWindow(nId);
        if (pAuthMrk)
            pAuthMrk->ReInitDlg(*pWrtShell);
    }
    else
        //Wenigstens das Notify rufen (vorsichtshalber wegen der SlotFilter
        AttrChangedNotify(pWrtShell);

    SfxViewShell::Activate(bMDIActivate);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwView::Deactivate(sal_Bool bMDIActivate)
{
    extern sal_Bool bFlushCharBuffer ;
        // Befinden sich noch Zeichen im Input Buffer?
    if( bFlushCharBuffer )
        GetEditWin().FlushInBuffer();

    if( bMDIActivate )
    {
        pWrtShell->ShLooseFcs();    // Selektionen unsichtbar

        pHRuler->SetActive( sal_False );
        pVRuler->SetActive( sal_False );
    }
    SfxViewShell::Deactivate(bMDIActivate);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwView::MarginChanged()
{
    GetWrtShell().SetBrowseBorder( GetMargin() );
}

/*--------------------------------------------------------------------
 --------------------------------------------------------------------*/

void SwView::ExecFormatPaintbrush(SfxRequest& rReq)
{
    if(!pFormatClipboard)
        return;

    if( pFormatClipboard->HasContent() )
    {
        pFormatClipboard->Erase();

        SwApplyTemplate aTemplate;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    else
    {
        bool bPersistentCopy = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
        {
            bPersistentCopy = static_cast<bool>(((SfxBoolItem &)pArgs->Get(
                                    SID_FORMATPAINTBRUSH)).GetValue());
        }

        pFormatClipboard->Copy( GetWrtShell(), GetPool(), bPersistentCopy );

        SwApplyTemplate aTemplate;
        aTemplate.pFormatClipboard = pFormatClipboard;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void SwView::StateFormatPaintbrush(SfxItemSet &rSet)
{
    if(!pFormatClipboard)
        return;

    bool bHasContent = pFormatClipboard && pFormatClipboard->HasContent();
    rSet.Put(SfxBoolItem(SID_FORMATPAINTBRUSH, bHasContent));
    if(!bHasContent)
    {
        if( !pFormatClipboard->CanCopyThisType( GetWrtShell().GetSelectionType() ) )
            rSet.DisableItem( SID_FORMATPAINTBRUSH );
    }
}
