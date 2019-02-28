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

#include <ids.hxx>
#include <strings.hrc>
#include "lockcorrupt.hxx"
#include <unotools/resmgr.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>

LockCorruptQueryBox::LockCorruptQueryBox(weld::Window* pParent, const std::locale& rResLocale)
    : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question,
                  VclButtonsType::NONE, Translate::get(STR_LOCKCORRUPT_MSG, rResLocale)))
{
    m_xQueryBox->set_title(Translate::get(STR_LOCKCORRUPT_TITLE, rResLocale));
    m_xQueryBox->add_button(Translate::get(STR_LOCKCORRUPT_OPENREADONLY_BTN, rResLocale), RET_OK);
    m_xQueryBox->add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
    m_xQueryBox->set_default_response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
