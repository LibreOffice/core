/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

extern int bDocSzUpdated;

void SwView::Activate(sal_Bool bMDIActivate)
{
    
    m_pWrtShell->StartAction();
    m_pWrtShell->EndAction( sal_True );

    
    
    
    SwDocShell* pDocSh = GetDocShell();
    if(pDocSh)
        pDocSh->SetView(this);
    SwModule* pSwMod = SW_MOD();
    pSwMod->SetView(this);

    
    if(!bDocSzUpdated)
        DocSzChgd(m_aDocSz);

    
    if(m_bMakeSelectionVisible)
    {
        m_pWrtShell->MakeSelVisible();
        m_bMakeSelectionVisible = sal_False;
    }
    m_pHRuler->SetActive( sal_True );
    m_pVRuler->SetActive( sal_True );

    if ( bMDIActivate )
    {
        m_pWrtShell->ShGetFcs(sal_False);     

        if( !m_sSwViewData.isEmpty() )
        {
            ReadUserData(m_sSwViewData, false);
            m_sSwViewData = "";
        }

        AttrChangedNotify(m_pWrtShell);

        
        sal_uInt16 nId = SwFldDlgWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = GetViewFrame();
        SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
        if (pWrp)
            pWrp->ReInitDlg(GetDocShell());

        
        nId = SwRedlineAcceptChild::GetChildWindowId();
        SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->GetChildWindow(nId);
        if (pRed)
            pRed->ReInitDlg(GetDocShell());

        
        nId = SwInsertIdxMarkWrapper::GetChildWindowId();
        SwInsertIdxMarkWrapper *pIdxMrk = (SwInsertIdxMarkWrapper*)pVFrame->GetChildWindow(nId);
        if (pIdxMrk)
            pIdxMrk->ReInitDlg(*m_pWrtShell);

        
        nId = SwInsertAuthMarkWrapper::GetChildWindowId();
        SwInsertAuthMarkWrapper *pAuthMrk = (SwInsertAuthMarkWrapper*)pVFrame->
                                                                GetChildWindow(nId);
        if (pAuthMrk)
            pAuthMrk->ReInitDlg(*m_pWrtShell);
    }
    else
        
        AttrChangedNotify(m_pWrtShell);

    SfxViewShell::Activate(bMDIActivate);
}

void SwView::Deactivate(sal_Bool bMDIActivate)
{
    extern bool bFlushCharBuffer ;
        
    if( bFlushCharBuffer )
        GetEditWin().FlushInBuffer();

    if( bMDIActivate )
    {
        m_pWrtShell->ShLooseFcs();    

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
