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

#include <querydialog.hxx>
#include <bitmaps.hlst>
#include <vcl/weld/Builder.hxx>

QueryDialog::QueryDialog(weld::Window* pParent, const OUString& sTitle, const OUString& sText,
                         const OUString& sQuestion, bool bHideShowAgain)
    : GenericDialogController(pParent, u"cui/ui/querydialog.ui"_ustr, u"QueryDialog"_ustr)
    , m_pTitle(m_xBuilder->weld_label(u"lbTitle"_ustr))
    , m_pText(m_xBuilder->weld_label(u"lbText"_ustr))
    , m_pQuestion(m_xBuilder->weld_label(u"lbQuestion"_ustr))
    , m_pImage(m_xBuilder->weld_image(u"imQuery"_ustr))
    , m_pCheckBox(m_xBuilder->weld_check_button(u"cbDontShowAgain"_ustr))
    , m_pYes(m_xBuilder->weld_button(u"btnYes"_ustr))
    , m_pNo(m_xBuilder->weld_button(u"btnNo"_ustr))
    , m_pYesLarge(m_xBuilder->weld_button(u"btnYesLarge"_ustr))
    , m_pNoLarge(m_xBuilder->weld_button(u"btnNoLarge"_ustr))
{
    m_pTitle->set_label(sTitle);
    m_pText->set_label(sText);
    m_pQuestion->set_label(sQuestion);
    m_pImage->set_from_icon_name(RID_CUIBMP_QUERY);

    if (bHideShowAgain)
        m_pCheckBox->hide();
}

void QueryDialog::SetYesLabel(const OUString& rLabel)
{
    m_pYesLarge->set_label(rLabel);
    m_pYesLarge->set_visible(true);
    m_pYes->set_visible(false);
}

void QueryDialog::SetNoLabel(const OUString& rLabel)
{
    m_pNoLarge->set_label(rLabel);
    m_pNoLarge->set_visible(!rLabel.isEmpty());
    m_pNo->set_visible(false);
}

void QueryDialog::SetTypeWarn() { m_pImage->set_from_icon_name(RID_CUIBMP_WARN); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
