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

#include <cuisrchdlg.hxx>

#include "optjsearch.hxx"


// class SvxJSearchOptionsDialog -----------------------------------------

SvxJSearchOptionsDialog::SvxJSearchOptionsDialog(weld::Window *pParent,
    const SfxItemSet& rOptionsSet, TransliterationFlags nInitialFlags)
    : weld::GenericDialogController(pParent,
        u"cui/ui/jsearchoptionsdialog.ui"_ustr, u"JSearchOptionsDialog"_ustr)
    , m_xContainer(m_xBuilder->weld_container(u"container"_ustr))
{
    std::unique_ptr<SfxTabPage> xPage = SvxJSearchOptionsPage::Create(m_xContainer.get(), this, &rOptionsSet);
    m_xPage.reset(static_cast<SvxJSearchOptionsPage*>(xPage.release()));
    m_xPage->EnableSaveOptions(false);
    m_xPage->SetTransliterationFlags(nInitialFlags);
}

SvxJSearchOptionsDialog::~SvxJSearchOptionsDialog()
{
}

TransliterationFlags SvxJSearchOptionsDialog::GetTransliterationFlags() const
{
    // Update transliteration flags from the current checkbox state
    m_xPage->FillItemSet(nullptr);
    return m_xPage->GetTransliterationFlags();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
