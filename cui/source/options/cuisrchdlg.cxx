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

#include <sfx2/basedlgs.hxx>

#include <cuisrchdlg.hxx>

#include "optjsearch.hxx"


// class SvxJSearchOptionsDialog -----------------------------------------

SvxJSearchOptionsDialog::SvxJSearchOptionsDialog(weld::Window *pParent,
    const SfxItemSet& rOptionsSet, TransliterationFlags nInitialFlags)
    : SfxSingleTabDialogController(pParent, &rOptionsSet)
{
    // m_xPage will be implicitly destroyed by the
    // SfxSingleTabDialog destructor
    SetTabPage(SvxJSearchOptionsPage::Create(get_content_area(), this, &rOptionsSet)); //! implicitly calls m_xPage->Reset(...)!
    m_pPage = static_cast<SvxJSearchOptionsPage*>(GetTabPage());
    m_pPage->EnableSaveOptions(false);
    m_pPage->SetTransliterationFlags(nInitialFlags);
}

SvxJSearchOptionsDialog::~SvxJSearchOptionsDialog()
{
}

TransliterationFlags SvxJSearchOptionsDialog::GetTransliterationFlags() const
{
    return m_pPage->GetTransliterationFlags();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
