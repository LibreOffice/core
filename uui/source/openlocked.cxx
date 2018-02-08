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
#include <unotools/resmgr.hxx>

OpenLockedQueryBox::OpenLockedQueryBox( vcl::Window* pParent, const std::locale& rResLocale, const OUString& aMessage, bool bEnableOverride ) :
    MessBox(pParent, MessBoxStyle::NONE, 0,
            Translate::get(STR_OPENLOCKED_TITLE, rResLocale),
            aMessage )
{
    SetImage( QueryBox::GetStandardImage() );

    AddButton(Translate::get(STR_OPENLOCKED_OPENREADONLY_BTN, rResLocale), RET_YES,
            ButtonDialogFlags::Default | ButtonDialogFlags::OK | ButtonDialogFlags::Focus);
    SetButtonHelpText(RET_YES, OUString());

    AddButton(Translate::get(STR_OPENLOCKED_OPENCOPY_BTN, rResLocale), RET_NO);
    SetButtonHelpText(RET_NO, OUString());

    if (bEnableOverride)
    {
        // Present option to ignore the (stale?) lock file and open the document
        AddButton(Translate::get(STR_ALREADYOPEN_OPEN_BTN, rResLocale), RET_IGNORE);
        SetButtonHelpText(RET_IGNORE, OUString());
    }

    AddButton( StandardButtonType::Cancel, RET_CANCEL, ButtonDialogFlags::Cancel );
}

OpenLockedQueryBox::~OpenLockedQueryBox()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
