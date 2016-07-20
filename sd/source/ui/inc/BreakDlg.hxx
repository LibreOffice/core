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

#ifndef INCLUDED_SD_SOURCE_UI_INC_BREAKDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_BREAKDLG_HXX

#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/edit.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/idle.hxx>

class SvdProgressInfo;
class SfxProgress;

namespace sd {

class DrawDocShell;
class DrawView;

/**
 * dialog to break meta files
 */
class BreakDlg
    : public SfxModalDialog
{
public:
    BreakDlg (
        vcl::Window* pWindow,
        DrawView* pDrView,
        DrawDocShell* pShell,
        sal_uLong nSumActionCount,
        sal_uLong nObjCount);
    virtual ~BreakDlg() override;
    virtual void dispose() override;

    short Execute() override;

private:
    VclPtr<FixedText>      m_pFiObjInfo;
    VclPtr<FixedText>      m_pFiActInfo;
    VclPtr<FixedText>      m_pFiInsInfo;
    VclPtr<CancelButton>   m_pBtnCancel;

    DrawView*   pDrView;

    bool            bCancel;

    Idle            m_aUpdateIdle;
    SvdProgressInfo *pProgrInfo;
    SfxProgress     *mpProgress;

    DECL_LINK_TYPED( CancelButtonHdl, Button*, void );
    DECL_LINK_TYPED( UpDate, void*, bool );
    DECL_LINK_TYPED( InitialUpdate, Idle*, void );
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
