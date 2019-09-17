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

#include <unotools/resmgr.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <strings.hrc>
#include "trylater.hxx"

TryLaterQueryBox::TryLaterQueryBox(weld::Window* pParent, const std::locale& rResLocale, const OUString& rMessage, bool bEnableOverride)
    : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question, VclButtonsType::NONE, rMessage))
{
    m_xQueryBox->set_title(Translate::get(STR_TRYLATER_TITLE, rResLocale));

    // Currently we don't have the retry/save-as functionality implemented for cases when file is locked.
    // So threat them mutually exclusive with overwrite here. TODO/LATER: just add the overwrite option
    // as third option when retrying and saving with another name would be possible along with overwriting
    if (bEnableOverride)
    {
        m_xQueryBox->add_button(Translate::get(STR_FILECHANGED_SAVEANYWAY_BTN, rResLocale), RET_IGNORE);
        m_xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
        m_xQueryBox->set_default_response(RET_IGNORE);
    }
    else
    {
        m_xQueryBox->add_button(Translate::get(STR_TRYLATER_RETRYSAVING_BTN, rResLocale), RET_YES);
        m_xQueryBox->add_button(Translate::get(STR_TRYLATER_SAVEAS_BTN, rResLocale), RET_NO);
        m_xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
        m_xQueryBox->set_default_response(RET_YES);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
