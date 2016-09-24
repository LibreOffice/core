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

#include "ids.hrc"
#include "trylater.hxx"

TryLaterQueryBox::TryLaterQueryBox( vcl::Window* pParent, ResMgr* pResMgr, const OUString& aMessage ) :
    MessBox(pParent, 0,
            ResId(STR_TRYLATER_TITLE, *pResMgr).toString(),
            aMessage )
{
    SetImage( QueryBox::GetStandardImage() );

    AddButton(ResId(STR_TRYLATER_RETRYSAVING_BTN, *pResMgr).toString(), RET_YES,
            ButtonDialogFlags::Default | ButtonDialogFlags::OK | ButtonDialogFlags::Focus);
    AddButton(ResId(STR_TRYLATER_SAVEAS_BTN, *pResMgr).toString(), RET_NO);
    AddButton( StandardButtonType::Cancel, RET_CANCEL, ButtonDialogFlags::Cancel );

    SetButtonHelpText( RET_YES, OUString() );
    SetButtonHelpText( RET_NO, OUString() );
}

TryLaterQueryBox::~TryLaterQueryBox()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
