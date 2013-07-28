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

#include "dlg_InsertTitle.hxx"
#include "dlg_InsertTitle.hrc"
#include "res_Titles.hxx"
#include "ResId.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "ObjectNameProvider.hxx"

namespace chart
{

SchTitleDlg::SchTitleDlg(Window* pWindow, const TitleDialogData& rInput )
    : ModalDialog(pWindow, SchResId(DLG_TITLE))
    , m_apTitleResources( new TitleResources(this,true) )
    , aBtnOK(this, SchResId(BTN_OK))
    , aBtnCancel(this, SchResId(BTN_CANCEL))
    , aBtnHelp(this, SchResId(BTN_HELP))
{
    FreeResource();

    this->SetText( ObjectNameProvider::getName(OBJECTTYPE_TITLE,true) );
    m_apTitleResources->writeToResources( rInput );
}

SchTitleDlg::~SchTitleDlg()
{
}

void SchTitleDlg::getResult( TitleDialogData& rOutput )
{
    m_apTitleResources->readFromResources( rOutput );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
