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

#include <cmdid.h>
#include <swtypes.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <fldwrap.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>

#include <helpid.h>
#include <fldui.hrc>
#include <globals.hrc>
#include "swabstdlg.hxx"

SFX_IMPL_CHILDWINDOW_WITHID(SwFieldDlgWrapper, FN_INSERT_FIELD)

SwChildWinWrapper::SwChildWinWrapper(vcl::Window *pParentWindow, sal_uInt16 nId) :
        SfxChildWindow(pParentWindow, nId),
        m_pDocSh(nullptr)
{
    // avoid flickering of buttons:
    m_aUpdateTimer.SetTimeout(200);
    m_aUpdateTimer.SetTimeoutHdl(LINK(this, SwChildWinWrapper, UpdateHdl));
}

IMPL_LINK_NOARG_TYPED(SwChildWinWrapper, UpdateHdl, Timer *, void)
{
    GetWindow()->Activate();    // update dialog
}

// newly initialise dialog after Doc switch
bool SwChildWinWrapper::ReInitDlg(SwDocShell *)
{
    bool bRet = false;

    if (m_pDocSh != GetOldDocShell())
    {
        m_aUpdateTimer.Stop();
        bRet = true;            // immediate Update
    }
    else
        m_aUpdateTimer.Start();

    return bRet;
}

SfxChildWinInfo SwFieldDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    aInfo.aPos = GetWindow()->OutputToAbsoluteScreenPixel(aInfo.aPos);
    return aInfo;
}

SwFieldDlgWrapper::SwFieldDlgWrapper( vcl::Window* _pParent, sal_uInt16 nId,
                                    SfxBindings* pB,
                                    SfxChildWinInfo*  )
    : SwChildWinWrapper( _pParent, nId )
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    assert(pFact && "SwAbstractDialogFactory fail!");
    AbstractSwFieldDlg* pDlg = pFact->CreateSwFieldDlg(pB, this, _pParent);
    assert(pDlg && "Dialog creation failed!");
    pDlgInterface = pDlg;
    SetWindow( pDlg->GetWindow() );
    pDlg->Start();
}

// newly initialise dialog after Doc switch
bool SwFieldDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    bool bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)))  // update immediately, Doc switch
    {
        pDlgInterface->ReInitDlg();
    }

    return bRet;
}

void SwFieldDlgWrapper::ShowReferencePage()
{
    pDlgInterface->ShowReferencePage();
}

SFX_IMPL_CHILDWINDOW(SwFieldDataOnlyDlgWrapper, FN_INSERT_FIELD_DATA_ONLY)

SfxChildWinInfo SwFieldDataOnlyDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
// prevent instatiation of dialog other than by calling
// the mail merge dialog
    aInfo.bVisible = false;
    return aInfo;
}

SwFieldDataOnlyDlgWrapper::SwFieldDataOnlyDlgWrapper( vcl::Window* _pParent, sal_uInt16 nId,
                                    SfxBindings* pB,
                                    SfxChildWinInfo* pInfo )
    : SwChildWinWrapper( _pParent, nId )
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwFieldDlg* pDlg = pFact->CreateSwFieldDlg(pB, this, _pParent);
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    pDlgInterface = pDlg;

    SetWindow( pDlg->GetWindow() );
    pDlg->ActivateDatabasePage();
    pDlg->Start();
    pDlg->Initialize( pInfo );
}

// re-init after doc activation
bool SwFieldDataOnlyDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    bool bRet;
    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)))  // update immediately, Doc switch
    {
        pDlgInterface->ReInitDlg();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
