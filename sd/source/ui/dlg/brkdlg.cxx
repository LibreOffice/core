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

#include "BreakDlg.hxx"
#include <sfx2/progress.hxx>

#include <svx/svdedtv.hxx>
#include <svx/svdetc.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "View.hxx"
#include "drawview.hxx"
#include "strings.hrc"
#include "DrawDocShell.hxx"

namespace sd {

/**
 * dialog to split metafiles
 */

BreakDlg::BreakDlg(
    vcl::Window* pWindow,
    DrawView* _pDrView,
    DrawDocShell* pShell,
    sal_uLong nSumActionCount,
    sal_uLong nObjCount )
    : SfxModalDialog(pWindow, "BreakDialog", "modules/sdraw/ui/breakdialog.ui")
    , mpProgress( nullptr )
{
    m_aUpdateIdle.SetPriority( SchedulerPriority::REPAINT );
    m_aUpdateIdle.SetIdleHdl( LINK( this, BreakDlg, InitialUpdate ) );
    m_aUpdateIdle.SetDebugName( "sd::BreakDlg m_aUpdateIdle" );

    get(m_pFiObjInfo, "metafiles");
    get(m_pFiActInfo, "metaobjects");
    get(m_pFiInsInfo, "drawingobjects");
    get(m_pBtnCancel, "cancel");

    m_pBtnCancel->SetClickHdl( LINK( this, BreakDlg, CancelButtonHdl));

    mpProgress = new SfxProgress( pShell, SD_RESSTR(STR_BREAK_METAFILE), nSumActionCount*3 );

    pProgrInfo = new SvdProgressInfo( LINK(this, BreakDlg, UpDate) );
    // every action is edited 3 times in DoImport()
    pProgrInfo->Init( nSumActionCount*3, nObjCount );

    pDrView = _pDrView;
    bCancel = false;
}

BreakDlg::~BreakDlg()
{
    disposeOnce();
}

void BreakDlg::dispose()
{
    delete mpProgress;
    mpProgress = nullptr;
    delete pProgrInfo;
    pProgrInfo = nullptr;
    m_pFiObjInfo.clear();
    m_pFiActInfo.clear();
    m_pFiInsInfo.clear();
    m_pBtnCancel.clear();
    SfxModalDialog::dispose();
}

// Control-Handler for cancel button
IMPL_LINK_NOARG_TYPED(BreakDlg, CancelButtonHdl, Button*, void)
{
    bCancel = true;
    m_pBtnCancel->Disable();
}

/**
 * The working function has to call the UpDate method periodically.
 * With the first call, the overall number of actions is provided.
 * Every following call should contain the finished actions since the
 * last call of UpDate.
 */
IMPL_LINK_TYPED( BreakDlg, UpDate, void*, nInit, bool )
{
    if(pProgrInfo == nullptr)
      return true;

    // update status bar or show a error message?
    if(nInit == reinterpret_cast<void*>(1L))
    {
        ScopedVclPtrInstance< MessageDialog > aErrBox(this, SD_RESSTR(STR_BREAK_FAIL));
        aErrBox->Execute();
    }
    else
    {
        if(mpProgress)
            mpProgress->SetState( pProgrInfo->GetSumCurAction() );
    }

    // which object is shown at the moment?
    OUString info = OUString::number( pProgrInfo->GetCurObj() )
            + "/"
            + OUString::number( pProgrInfo->GetObjCount() );
    m_pFiObjInfo->SetText(info);

    // how many actions are started?
    if(pProgrInfo->GetActionCount() == 0)
    {
        m_pFiActInfo->SetText( OUString() );
    }
    else
    {
        info = OUString::number( pProgrInfo->GetCurAction() )
            + "/"
            + OUString::number( pProgrInfo->GetActionCount() );
        m_pFiActInfo->SetText(info);
    }

    // and inserted????
    if(pProgrInfo->GetInsertCount() == 0)
    {
        m_pFiInsInfo->SetText( OUString() );
    }
    else
    {
        info = OUString::number( pProgrInfo->GetCurInsert() )
            + "/"
            + OUString::number( pProgrInfo->GetInsertCount() );
        m_pFiInsInfo->SetText(info);
    }

    Application::Reschedule();
    return bCancel;
}

/**
 * open a modal dialog and start a timer which calls the working function after
 * the opening of the dialog
 */
short BreakDlg::Execute()
{
  m_aUpdateIdle.Start();

  return SfxModalDialog::Execute();
}

/**
 * link-method which starts the working function
 */
IMPL_LINK_NOARG_TYPED(BreakDlg, InitialUpdate, Idle *, void)
{
    pDrView->DoImportMarkedMtf(pProgrInfo);
    EndDialog(RET_OK);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
