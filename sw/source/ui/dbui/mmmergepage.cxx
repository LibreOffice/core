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

#include <mmmergepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <swtypes.hxx>
#include <view.hxx>
#include <dbui.hrc>
#include <svl/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <swabstdlg.hxx>

SwMailMergeMergePage::SwMailMergeMergePage(SwMailMergeWizard* _pParent)
    : svt::OWizardPage(_pParent, "MMMergePage",
        "modules/swriter/ui/mmmergepage.ui")
    , m_pWizard(_pParent)
{
    get(m_pEditFI, "helplabel");
    get(m_pEditPB, "edit");
    get(m_pFindPB, "find");
    get(m_pWholeWordsCB, "wholewords");
    get(m_pBackwardsCB, "backwards");
    get(m_pMatchCaseCB, "matchcase");
    get(m_pFindED, "entry");
    OUString sTemp(m_pEditFI->GetText());
    sTemp = sTemp.replaceFirst("%1", m_pEditPB->GetText());
    m_pEditFI->SetText(sTemp);
    m_pEditPB->SetClickHdl( LINK( this, SwMailMergeMergePage, EditDocumentHdl_Impl));
    m_pFindPB->SetClickHdl( LINK( this, SwMailMergeMergePage, FindHdl_Impl ));

    m_pFindED->SetReturnActionLink( LINK(this, SwMailMergeMergePage, EnteredFindStringHdl_Impl ));

}

IMPL_LINK_NOARG(SwMailMergeMergePage, EditDocumentHdl_Impl)
{
    m_pWizard->SetRestartPage(MM_MERGEPAGE);
    m_pWizard->EndDialog(RET_EDIT_RESULT_DOC);
    return 0;
}

IMPL_LINK_NOARG(SwMailMergeMergePage, FindHdl_Impl)
{
    SvxSearchItem aSearchItem( SID_SEARCH_ITEM );

    SfxBoolItem aQuiet( SID_SEARCH_QUIET, false );
    aSearchItem.SetSearchString(m_pFindED->GetText());

    aSearchItem.SetWordOnly(m_pWholeWordsCB->IsChecked());
    aSearchItem.SetExact(m_pMatchCaseCB->IsChecked());
    aSearchItem.SetBackward(m_pBackwardsCB->IsChecked());

    SwView* pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(pTargetView)
    {
        pTargetView->GetViewFrame()->GetDispatcher()->Execute(
            FID_SEARCH_NOW, SFX_CALLMODE_SYNCHRON, &aSearchItem, &aQuiet, 0L );
    }

    return 0;
}

IMPL_LINK_NOARG(SwMailMergeMergePage, EnteredFindStringHdl_Impl)
{
    m_pFindPB->GetClickHdl().Call(m_pFindPB);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
