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

#include <BreakDlg.hxx>
#include <sfx2/progress.hxx>

#include <svx/svdetc.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>

#include <sdresid.hxx>
#include <drawview.hxx>
#include <strings.hrc>
#include <DrawDocShell.hxx>

namespace sd {

/**
 * dialog to split metafiles
 */

BreakDlg::BreakDlg(weld::Window* pWindow, DrawView* pDrView, DrawDocShell* pShell,
    sal_uLong nSumActionCount, sal_uLong nObjCount)
    : SfxDialogController(pWindow, "modules/sdraw/ui/breakdialog.ui", "BreakDialog")
    , m_xFiObjInfo(m_xBuilder->weld_label("metafiles"))
    , m_xFiActInfo(m_xBuilder->weld_label("metaobjects"))
    , m_xFiInsInfo(m_xBuilder->weld_label("drawingobjects"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_pDrView(pDrView)
    , m_bCancel(false)
{
    m_aUpdateIdle.SetPriority( TaskPriority::REPAINT );
    m_aUpdateIdle.SetInvokeHandler( LINK( this, BreakDlg, InitialUpdate ) );
    m_aUpdateIdle.SetDebugName( "sd::BreakDlg m_aUpdateIdle" );

    m_xBtnCancel->connect_clicked(LINK(this, BreakDlg, CancelButtonHdl));

    m_xProgress.reset(new SfxProgress(pShell, SdResId(STR_BREAK_METAFILE), nSumActionCount*3));

    m_xProgrInfo.reset(new SvdProgressInfo(LINK(this, BreakDlg, UpDate)));
    // every action is edited 3 times in DoImport()
    m_xProgrInfo->Init( nObjCount );
}

// Control-Handler for cancel button
IMPL_LINK_NOARG(BreakDlg, CancelButtonHdl, weld::Button&, void)
{
    m_bCancel = true;
    m_xBtnCancel->set_sensitive(false);
}

/**
 * The working function has to call the UpDate method periodically.
 * With the first call, the overall number of actions is provided.
 * Every following call should contain the finished actions since the
 * last call of UpDate.
 */
IMPL_LINK( BreakDlg, UpDate, void*, nInit, bool )
{
    if (!m_xProgrInfo)
      return true;

    // update status bar or show a error message?
    if(nInit == reinterpret_cast<void*>(1))
    {
        std::unique_ptr<weld::MessageDialog> xErrBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                     VclMessageType::Warning, VclButtonsType::Ok,
                                                     SdResId(STR_BREAK_FAIL)));
        xErrBox->run();
    }
    else
    {
        if (m_xProgress)
            m_xProgress->SetState(m_xProgrInfo->GetSumCurAction());
    }

    // which object is shown at the moment?
    OUString info = OUString::number(m_xProgrInfo->GetCurObj())
            + "/"
            + OUString::number(m_xProgrInfo->GetObjCount());
    m_xFiObjInfo->set_label(info);

    // how many actions are started?
    if (m_xProgrInfo->GetActionCount() == 0)
    {
        m_xFiActInfo->set_label( OUString() );
    }
    else
    {
        info = OUString::number(m_xProgrInfo->GetCurAction())
            + "/"
            + OUString::number(m_xProgrInfo->GetActionCount());
        m_xFiActInfo->set_label(info);
    }

    // and inserted????
    if (m_xProgrInfo->GetInsertCount() == 0)
    {
        m_xFiInsInfo->set_label( OUString() );
    }
    else
    {
        info = OUString::number(m_xProgrInfo->GetCurInsert())
            + "/"
            + OUString::number(m_xProgrInfo->GetInsertCount());
        m_xFiInsInfo->set_label(info);
    }

    // make sure dialog gets painted, it is intended to
    // show the progress to the user. Also necessary to
    // provide a clickable cancel button
    Application::Reschedule(true);

    // return okay-value (-> !cancel)
    return !m_bCancel;
}

/**
 * open a modal dialog and start a timer which calls the working function after
 * the opening of the dialog
 */
short BreakDlg::run()
{
    m_aUpdateIdle.Start();
    return SfxDialogController::run();
}

/**
 * link-method which starts the working function
 */
IMPL_LINK_NOARG(BreakDlg, InitialUpdate, Timer *, void)
{
    m_pDrView->DoImportMarkedMtf(m_xProgrInfo.get());
    m_xDialog->response(RET_OK);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
