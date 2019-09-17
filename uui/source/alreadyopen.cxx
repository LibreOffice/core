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

#include <strings.hrc>
#include "alreadyopen.hxx"
#include <unotools/resmgr.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>

AlreadyOpenQueryBox::AlreadyOpenQueryBox(weld::Window* pParent, const std::locale& rLocale, const OUString& rMessage, bool bIsStoring)
    : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question, VclButtonsType::NONE, rMessage))
{
    m_xQueryBox->set_title(Translate::get(STR_ALREADYOPEN_TITLE, rLocale));
    if (bIsStoring)
    {
        m_xQueryBox->add_button(Translate::get(STR_ALREADYOPEN_RETRY_SAVE_BTN, rLocale), RET_YES);
        m_xQueryBox->add_button(Translate::get(STR_ALREADYOPEN_SAVE_BTN, rLocale), RET_NO);
    }
    else
    {
        m_xQueryBox->add_button(Translate::get(STR_ALREADYOPEN_READONLY_BTN, rLocale), RET_YES);
        m_xQueryBox->add_button(Translate::get(STR_ALREADYOPEN_OPEN_BTN, rLocale), RET_NO);
    }
    m_xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
    m_xQueryBox->set_default_response(RET_YES);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
