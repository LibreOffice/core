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

#include <mmoutputtypepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <vcl/msgbox.hxx>
#include <dbui.hrc>
#include <swtypes.hxx>

SwMailMergeOutputTypePage::SwMailMergeOutputTypePage(SwMailMergeWizard* pParent)
    : svt::OWizardPage(pParent, "MMOutputTypePage",
        "modules/swriter/ui/mmoutputtypepage.ui")
    , m_pWizard(pParent)
{
    get(m_pLetterRB, "letter");
    get(m_pMailRB, "email");
    get(m_pLetterHint, "letterft");
    get(m_pMailHint, "emailft");

    Link aLink = LINK(this, SwMailMergeOutputTypePage, TypeHdl_Impl);
    m_pLetterRB->SetClickHdl(aLink);
    m_pMailRB->SetClickHdl(aLink);

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(rConfigItem.IsOutputToLetter())
        m_pLetterRB->Check();
    else
        m_pMailRB->Check();
    TypeHdl_Impl(m_pLetterRB);

}

IMPL_LINK_NOARG(SwMailMergeOutputTypePage, TypeHdl_Impl)
{
    bool bLetter = m_pLetterRB->IsChecked();
    m_pLetterHint->Show(bLetter);
    m_pMailHint->Show(!bLetter);
    m_pWizard->GetConfigItem().SetOutputToLetter(bLetter);
    m_pWizard->updateRoadmapItemLabel( MM_ADDRESSBLOCKPAGE );
    m_pWizard->UpdateRoadmap();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
