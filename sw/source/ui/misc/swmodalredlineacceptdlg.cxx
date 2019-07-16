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

#include <redline.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <unotools/viewoptions.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <uitool.hxx>

#include <cmdid.h>

#include <vector>
#include <redlndlg.hxx>
#include <swmodalredlineacceptdlg.hxx>

#include <unomid.h>

SwModalRedlineAcceptDlg::SwModalRedlineAcceptDlg(weld::Window *pParent)
    : SfxDialogController(pParent, "svx/ui/acceptrejectchangesdialog.ui",
                          "AcceptRejectChangesDialog")
    , m_xContentArea(m_xDialog->weld_content_area())
{
    m_xDialog->set_modal(true);

    m_xImplDlg.reset(new SwRedlineAcceptDlg(m_xDialog, m_xBuilder.get(), m_xContentArea.get(), true));

    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    if (aDlgOpt.Exists())
    {
        css::uno::Any aUserItem = aDlgOpt.GetUserItem("UserItem");
        OUString sExtraData;
        aUserItem >>= sExtraData;
        m_xImplDlg->Initialize(sExtraData);
    }
    m_xImplDlg->Activate();   // for data's initialisation
}

SwModalRedlineAcceptDlg::~SwModalRedlineAcceptDlg()
{
    AcceptAll(false);   // refuse everything remaining

    OUString sExtraData;
    m_xImplDlg->FillInfo(sExtraData);
    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetUserItem("UserItem", css::uno::makeAny(sExtraData));

    m_xDialog->set_modal(false);
}

void SwModalRedlineAcceptDlg::Activate()
{
}

void SwModalRedlineAcceptDlg::AcceptAll( bool bAccept )
{
    SvxTPFilter* pFilterTP = m_xImplDlg->GetChgCtrl().GetFilterPage();

    if (pFilterTP->IsDate() || pFilterTP->IsAuthor() ||
        pFilterTP->IsRange() || pFilterTP->IsAction())
    {
        pFilterTP->CheckDate(false);    // turn off all filters
        pFilterTP->CheckAuthor(false);
        pFilterTP->CheckRange(false);
        pFilterTP->CheckAction(false);
        m_xImplDlg->FilterChangedHdl(nullptr);
    }

    m_xImplDlg->CallAcceptReject( false, bAccept );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
