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

#include <vcl/wrkwin.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/msgbox.hxx>
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
#include <com/sun/star/i18n/TransliterationModules.hpp>

#include "cuisrchdlg.hxx"

#include <cuires.hrc>

#include <svl/srchitem.hxx>
#include <svx/pageitem.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <optjsearch.hxx>
#include <editeng/brushitem.hxx>
#include "backgrnd.hxx"


// class SvxJSearchOptionsDialog -----------------------------------------

SvxJSearchOptionsDialog::SvxJSearchOptionsDialog(Window *pParent,
    const SfxItemSet& rOptionsSet, sal_Int32 nInitialFlags)
    : SfxSingleTabDialog(pParent, rOptionsSet)
    , nInitialTlFlags( nInitialFlags )
{
    pPage = (SvxJSearchOptionsPage *)
        SvxJSearchOptionsPage::Create(get_content_area(), rOptionsSet );
    setTabPage( pPage );    //! implicitly calls pPage->Reset(...)!
    pPage->EnableSaveOptions(false);
}


SvxJSearchOptionsDialog::~SvxJSearchOptionsDialog()
{
    // pPage will be implicitly destroyed by the
    // SfxNoLayoutSingleTabDialog destructor
}


void SvxJSearchOptionsDialog::Activate()
{
    pPage->SetTransliterationFlags( nInitialTlFlags );
}


sal_Int32 SvxJSearchOptionsDialog::GetTransliterationFlags() const
{
    return pPage->GetTransliterationFlags();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
