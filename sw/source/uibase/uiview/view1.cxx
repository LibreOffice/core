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
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
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
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <wordcountdialog.hxx>

extern bool bDocSzUpdated;

void SwView::Activate(bool bMDIActivate)
{
    // fdo#40438 Update the layout to make sure everything is correct before showing the content
    m_pWrtShell->StartAction();
    m_pWrtShell->EndAction( true );

    // Register the current View at the DocShell.
    // The view remains active at the DocShell until it will
    // be destroyed or by Activate a new one will be set.
    SwDocShell* pDocSh = GetDocShell();
    if(pDocSh)
        pDocSh->SetView(this);
    SwModule* pSwMod = SW_MOD();
    pSwMod->SetView(this);

    // Document size has changed.
    if(!bDocSzUpdated)
        DocSzChgd(m_aDocSz);

    // make selection visible
    if(m_bMakeSelectionVisible)
    {
        m_pWrtShell->MakeSelVisible();
        m_bMakeSelectionVisible = false;
    }
    m_pHRuler->SetActive();
    m_pVRuler->SetActive();

    if ( bMDIActivate )
    {
        m_pWrtShell->ShellGetFocus(false);     // Selections visible

        if( !m_sSwViewData.isEmpty() )
        {
            ReadUserData(m_sSwViewData);
            m_sSwViewData.clear();
        }

        AttrChangedNotify(m_pWrtShell);

        // Initialize Fielddlg newly if necessary (e.g. for TYP_SETVAR)
        sal_uInt16 nId = SwFieldDlgWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = GetViewFrame();
        SwFieldDlgWrapper *pWrp = static_cast<SwFieldDlgWrapper*>(pVFrame->GetChildWindow(nId));
        if (pWrp)
            pWrp->ReInitDlg(GetDocShell());

        // Initialize RedlineDlg newly if necessary
        nId = SwRedlineAcceptChild::GetChildWindowId();
        SwRedlineAcceptChild *pRed = static_cast<SwRedlineAcceptChild*>(pVFrame->GetChildWindow(nId));
        if (pRed)
            pRed->ReInitDlg(GetDocShell());

        // reinit IdxMarkDlg
        nId = SwInsertIdxMarkWrapper::GetChildWindowId();
        SwInsertIdxMarkWrapper *pIdxMrk = static_cast<SwInsertIdxMarkWrapper*>(pVFrame->GetChildWindow(nId));
        if (pIdxMrk)
            pIdxMrk->ReInitDlg(*m_pWrtShell);

        // reinit AuthMarkDlg
        nId = SwInsertAuthMarkWrapper::GetChildWindowId();
        SwInsertAuthMarkWrapper *pAuthMrk = static_cast<SwInsertAuthMarkWrapper*>(pVFrame->
                                                                GetChildWindow(nId));
        if (pAuthMrk)
            pAuthMrk->ReInitDlg(*m_pWrtShell);
    }
    else
        // At least call the Notify (as a precaution because of the SlotFilter).
        AttrChangedNotify(m_pWrtShell);

    SfxViewShell::Activate(bMDIActivate);
}

void SwView::Deactivate(bool bMDIActivate)
{
    extern bool g_bFlushCharBuffer ;
        // Are Characters still in the input buffer?
    if( g_bFlushCharBuffer )
        GetEditWin().FlushInBuffer();

    if( bMDIActivate )
    {
        m_pWrtShell->ShellLoseFocus();    // Selections invisible

        m_pHRuler->SetActive( false );
        m_pVRuler->SetActive( false );
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
            bPersistentCopy = static_cast<bool>(static_cast<const SfxBoolItem &>(pArgs->Get(
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
        if( !SwFormatClipboard::CanCopyThisType( GetWrtShell().GetSelectionType() ) )
            rSet.DisableItem( SID_FORMATPAINTBRUSH );
    }
}

void SwView::UpdateWordCount(SfxShell* pShell, sal_uInt16 nSlot)
{
    SfxViewFrame* pVFrame = GetViewFrame();
    if (pVFrame != nullptr)
    {
        pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
        pShell->Invalidate(nSlot);

        SwWordCountWrapper *pWrdCnt = static_cast<SwWordCountWrapper*>(pVFrame->GetChildWindow(SwWordCountWrapper::GetChildWindowId()));
        if (pWrdCnt)
            pWrdCnt->UpdateCounts();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
