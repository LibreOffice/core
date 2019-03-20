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

#include <svl/slstitm.hxx>
#include <svl/itemiter.hxx>
#include <svl/style.hxx>
#include <unotools/searchopt.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/cjkoptions.hxx>

#include <cuisrchdlg.hxx>

#include <svl/srchitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/dlgutil.hxx>
#include "optjsearch.hxx"
#include <editeng/brushitem.hxx>
#include <backgrnd.hxx>


// class SvxJSearchOptionsDialog -----------------------------------------

SvxJSearchOptionsDialog::SvxJSearchOptionsDialog(weld::Window *pParent,
    const SfxItemSet& rOptionsSet, TransliterationFlags nInitialFlags)
    : SfxSingleTabDialogController(pParent, &rOptionsSet)
{
    // m_xPage will be implicitly destroyed by the
    // SfxSingleTabDialog destructor
    TabPageParent pPageParent(get_content_area(), this);
    m_xPage.reset(static_cast<SvxJSearchOptionsPage*>(
                        SvxJSearchOptionsPage::Create(
                                pPageParent, &rOptionsSet).get()));
    SetTabPage(m_xPage); //! implicitly calls m_xPage->Reset(...)!
    m_xPage->EnableSaveOptions(false);
    m_xPage->SetTransliterationFlags(nInitialFlags);
}

SvxJSearchOptionsDialog::~SvxJSearchOptionsDialog()
{
    m_xPage.clear();
}

TransliterationFlags SvxJSearchOptionsDialog::GetTransliterationFlags() const
{
    return m_xPage->GetTransliterationFlags();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
