/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/ruler.hxx>
#include <idxmrk.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <globdoc.hxx>
#include <navipi.hxx>
#include <fldwrap.hxx>
#include <redlndlg.hxx>
#include <dpage.hxx>
#include <edtwin.hxx>
#include "formatclipboard.hxx"
#include <cmdid.h>
// header for class SfxRequest
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>

extern int bDocSzUpdated;

void SwView::Activate(sal_Bool bMDIActivate)
{
    // fdo#40438 Update the layout to make sure everything is correct before showing the content
    m_pWrtShell->StartAction();
    m_pWrtShell->EndAction( sal_True );

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
        DocSzChgd(m_aDocSz);

    // make selection visible
    if(m_bMakeSelectionVisible)
    {
        m_pWrtShell->MakeSelVisible();
        m_bMakeSelectionVisible = sal_False;
    }
    m_pHRuler->SetActive( sal_True );
    m_pVRuler->SetActive( sal_True );

    if ( bMDIActivate )
    {
        m_pWrtShell->ShGetFcs(sal_False);     // Selektionen sichtbar

        if( m_sSwViewData.Len() )
        {
            ReadUserData( m_sSwViewData, sal_False );
            m_sSwViewData.Erase();
        }

        AttrChangedNotify(m_pWrtShell);

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
            pIdxMrk->ReInitDlg(*m_pWrtShell);

        // reinit AuthMarkDlg
        nId = SwInsertAuthMarkWrapper::GetChildWindowId();
        SwInsertAuthMarkWrapper *pAuthMrk = (SwInsertAuthMarkWrapper*)pVFrame->
                                                                GetChildWindow(nId);
        if (pAuthMrk)
            pAuthMrk->ReInitDlg(*m_pWrtShell);
    }
    else
        //Wenigstens das Notify rufen (vorsichtshalber wegen der SlotFilter
        AttrChangedNotify(m_pWrtShell);

    SfxViewShell::Activate(bMDIActivate);
}

void SwView::Deactivate(sal_Bool bMDIActivate)
{
    extern bool bFlushCharBuffer ;
        // Befinden sich noch Zeichen im Input Buffer?
    if( bFlushCharBuffer )
        GetEditWin().FlushInBuffer();

    if( bMDIActivate )
    {
        m_pWrtShell->ShLooseFcs();    // Selektionen unsichtbar

        m_pHRuler->SetActive( sal_False );
        m_pVRuler->SetActive( sal_False );
    }
    SfxViewShell::Deactivate(bMDIActivate);
}

void SwView::MarginChanged()
{
    GetWrtShell().SetBrowseBorder( GetMargin() );
}

void SwView::ExecFormatPaintbrush(SfxRequest& rReq)
{
    if(!m_pFormatClipboard)
        return;

    if( m_pFormatClipboard->HasContent() )
    {
        m_pFormatClipboard->Erase();

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

        m_pFormatClipboard->Copy( GetWrtShell(), GetPool(), bPersistentCopy );

        SwApplyTemplate aTemplate;
        aTemplate.m_pFormatClipboard = m_pFormatClipboard;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void SwView::StateFormatPaintbrush(SfxItemSet &rSet)
{
    if(!m_pFormatClipboard)
        return;

    bool bHasContent = m_pFormatClipboard && m_pFormatClipboard->HasContent();
    rSet.Put(SfxBoolItem(SID_FORMATPAINTBRUSH, bHasContent));
    if(!bHasContent)
    {
        if( !m_pFormatClipboard->CanCopyThisType( GetWrtShell().GetSelectionType() ) )
            rSet.DisableItem( SID_FORMATPAINTBRUSH );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
