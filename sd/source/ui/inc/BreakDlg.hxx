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

#pragma once

#include <sfx2/basedlgs.hxx>
#include <sfx2/progress.hxx>
#include <svx/svdetc.hxx>
#include <vcl/idle.hxx>

namespace sd
{
class DrawDocShell;
class DrawView;

/**
 * dialog to break meta files
 */
class BreakDlg : public SfxDialogController
{
public:
    BreakDlg(weld::Window* pWindow, DrawView* pDrView, DrawDocShell* pShell,
             sal_uLong nSumActionCount, sal_uLong nObjCount);

    virtual short run() override;

private:
    std::unique_ptr<weld::Label> m_xFiObjInfo;
    std::unique_ptr<weld::Label> m_xFiActInfo;
    std::unique_ptr<weld::Label> m_xFiInsInfo;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    DrawView* m_pDrView;

    bool m_bCancel;

    Idle m_aUpdateIdle;
    std::unique_ptr<SvdProgressInfo> m_xProgrInfo;
    std::unique_ptr<SfxProgress> m_xProgress;

    DECL_LINK(CancelButtonHdl, weld::Button&, void);
    DECL_LINK(UpDate, void*, bool);
    DECL_LINK(InitialUpdate, Timer*, void);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
