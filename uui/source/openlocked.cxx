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
#include "openlocked.hxx"
#include <officecfg/Office/Common.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>

OpenLockedQueryBox::OpenLockedQueryBox(weld::Window* pParent, const std::locale& rResLocale, const OUString& rMessage, bool bEnableOverride)
    : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question, VclButtonsType::NONE, rMessage))
{
    m_xQueryBox->set_title(Translate::get(STR_OPENLOCKED_TITLE, rResLocale));
    m_xQueryBox->add_button(Translate::get(STR_OPENLOCKED_OPENREADONLY_BTN, rResLocale), RET_YES);
    m_xQueryBox->add_button(Translate::get(STR_OPENLOCKED_OPENCOPY_BTN, rResLocale), RET_NO);
    if (bEnableOverride && officecfg::Office::Common::Misc::AllowOverrideLocking::get())
    {
        // Present option to ignore the (stale?) lock file and open the document
        m_xQueryBox->add_button(Translate::get(STR_ALREADYOPEN_OPEN_BTN, rResLocale), RET_IGNORE);
    }
    m_xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
