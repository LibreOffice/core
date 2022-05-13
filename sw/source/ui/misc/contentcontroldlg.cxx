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

#include <contentcontroldlg.hxx>

#include <vcl/weld.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <IDocumentState.hxx>

using namespace com::sun::star;

SwContentControlDlg::SwContentControlDlg(weld::Window* pParent, SwWrtShell& rWrtShell)
    : SfxDialogController(pParent, "modules/swriter/ui/contentcontroldlg.ui",
                          "ContentControlDialog")
    , m_rWrtShell(rWrtShell)
    , m_xShowingPlaceHolderCB(m_xBuilder->weld_check_button("showing_place_holder"))
    , m_xOk(m_xBuilder->weld_button("ok"))
{
    m_xOk->connect_clicked(LINK(this, SwContentControlDlg, OkHdl));

    const SwPosition* pStart = rWrtShell.GetCursor()->Start();
    SwTextNode* pTextNode = pStart->nNode.GetNode().GetTextNode();
    if (!pTextNode)
    {
        return;
    }

    SwTextAttr* pAttr = pTextNode->GetTextAttrAt(pStart->nContent.GetIndex(),
                                                 RES_TXTATR_CONTENTCONTROL, SwTextNode::PARENT);
    if (!pAttr)
    {
        return;
    }

    SwTextContentControl* pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    const SwFormatContentControl& rFormatContentControl = pTextContentControl->GetContentControl();
    m_pContentControl = rFormatContentControl.GetContentControl();

    bool bShowingPlaceHolder = m_pContentControl->GetShowingPlaceHolder();
    TriState eShowingPlaceHolder = bShowingPlaceHolder ? TRISTATE_TRUE : TRISTATE_FALSE;
    m_xShowingPlaceHolderCB->set_state(eShowingPlaceHolder);
    m_xShowingPlaceHolderCB->save_state();
}

SwContentControlDlg::~SwContentControlDlg() {}

IMPL_LINK_NOARG(SwContentControlDlg, OkHdl, weld::Button&, void)
{
    bool bChanged = false;
    if (m_xShowingPlaceHolderCB->get_state_changed_from_saved())
    {
        bool bShowingPlaceHolder = m_xShowingPlaceHolderCB->get_state() == TRISTATE_TRUE;
        m_pContentControl->SetShowingPlaceHolder(bShowingPlaceHolder);
        bChanged = true;
    }

    if (bChanged)
    {
        m_rWrtShell.GetDoc()->getIDocumentState().SetModified();
    }

    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
