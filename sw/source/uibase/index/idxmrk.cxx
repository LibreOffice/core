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

#include <idxmrk.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>

SFX_IMPL_CHILDWINDOW_WITHID(SwInsertIdxMarkWrapper, FN_INSERT_IDX_ENTRY_DLG)

SwInsertIdxMarkWrapper::SwInsertIdxMarkWrapper( vcl::Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    xAbstDlg = pFact->CreateIndexMarkFloatDlg(pBindings, this, pParentWindow->GetFrameWeld(), pInfo);
    SetController(xAbstDlg->GetController());
}

SfxChildWinInfo SwInsertIdxMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();

    return aInfo;
}

void SwInsertIdxMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    xAbstDlg->ReInitDlg(rWrtShell);
}

SFX_IMPL_CHILDWINDOW_WITHID(SwInsertAuthMarkWrapper, FN_INSERT_AUTH_ENTRY_DLG)

SwInsertAuthMarkWrapper::SwInsertAuthMarkWrapper(   vcl::Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    m_xAbstDlg = pFact->CreateAuthMarkFloatDlg(pBindings, this, pParentWindow->GetFrameWeld(), pInfo);
    SetController(m_xAbstDlg->GetController());
}

SfxChildWinInfo SwInsertAuthMarkWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

void SwInsertAuthMarkWrapper::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_xAbstDlg->ReInitDlg(rWrtShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
