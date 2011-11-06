/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

