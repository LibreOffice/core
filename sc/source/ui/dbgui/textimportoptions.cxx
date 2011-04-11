/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

//------------------------------------------------------------------------

#include "textimportoptions.hxx"
#include "textimportoptions.hrc"

#include "scresid.hxx"
#include "vcl/window.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"

ScTextImportOptionsDlg::ScTextImportOptionsDlg(Window* pParent) :
    ModalDialog(pParent, ScResId(RID_SCDLG_TEXT_IMPORT_OPTIONS)),

    maBtnOk(this, ScResId(BTN_OK)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maBtnHelp(this, ScResId(BTN_HELP)),
    maFlChooseLang(this, ScResId(FL_CHOOSE_LANG)),
    maRbAutomatic(this, ScResId(RB_AUTOMATIC)),
    maRbCustom(this, ScResId(RB_CUSTOM)),
    maLbCustomLang(this, ScResId(LB_CUSTOM_LANG)),
    maFlOption(this, ScResId(FL_OPTION)),
    maBtnConvertDate(this, ScResId(BTN_CONVERT_DATE))
{
    FreeResource();
    init();
}

ScTextImportOptionsDlg::~ScTextImportOptionsDlg()
{
}

short ScTextImportOptionsDlg::Execute()
{
    return ModalDialog::Execute();
}

LanguageType ScTextImportOptionsDlg::getLanguageType() const
{
    if (maRbAutomatic.IsChecked())
        return LANGUAGE_SYSTEM;

    return maLbCustomLang.GetSelectLanguage();
}

bool ScTextImportOptionsDlg::isDateConversionSet() const
{
    return maBtnConvertDate.IsChecked();
}

void ScTextImportOptionsDlg::init()
{
    Link aLink = LINK( this, ScTextImportOptionsDlg, OKHdl );
    maBtnOk.SetClickHdl(aLink);
    aLink = LINK( this, ScTextImportOptionsDlg, RadioHdl );
    maRbAutomatic.SetClickHdl(aLink);
    maRbCustom.SetClickHdl(aLink);

    maRbAutomatic.Check(true);

    maLbCustomLang.SetLanguageList(
        LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, false, false);

    LanguageType eLang = Application::GetSettings().GetLanguage();
    maLbCustomLang.SelectLanguage(eLang);
    maLbCustomLang.Disable();
}

IMPL_LINK( ScTextImportOptionsDlg, OKHdl, OKButton*, EMPTYARG )
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK( ScTextImportOptionsDlg, RadioHdl, RadioButton*, pBtn )
{
    if (pBtn == &maRbAutomatic)
    {
        maLbCustomLang.Disable();
    }
    else if (pBtn == &maRbCustom)
    {
        maLbCustomLang.Enable();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
