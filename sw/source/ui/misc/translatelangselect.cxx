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

#include <vcl/svapp.hxx>
#include <osl/diagnose.h>
#include <uitool.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <translatelangselect.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <sal/log.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <svtools/deeplcfg.hxx>
#include <vcl/idle.hxx>
#include <mdiexp.hxx>
#include <strings.hrc>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <linguistic/translate.hxx>

int SwTranslateLangSelectDlg::selectedLangIdx = -1;
SwTranslateLangSelectDlg::SwTranslateLangSelectDlg(weld::Window* pParent, SwWrtShell& rSh)
    : GenericDialogController(pParent, "modules/swriter/ui/translationdialog.ui",
                              "LanguageSelectDialog")
    , rWrtSh(rSh)
    , m_xLanguageListBox(m_xBuilder->weld_combo_box("combobox1"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnTranslate(m_xBuilder->weld_button("translate"))
    , m_xLanguageVec({
          SwLanguageListItem("BG", "Bulgarian"),
          SwLanguageListItem("CS", "Czech"),
          SwLanguageListItem("DA", "Danish"),
          SwLanguageListItem("DE", "German"),
          SwLanguageListItem("EL", "Greek"),
          SwLanguageListItem("EN-GB", "English (British)"),
          SwLanguageListItem("EN-US", "English (American)"),
          SwLanguageListItem("ET", "Estonian"),
          SwLanguageListItem("FI", "Finnish"),
          SwLanguageListItem("FR", "French"),
          SwLanguageListItem("HU", "Hungarian"),
          SwLanguageListItem("ID", "Indonesian"),
          SwLanguageListItem("IT", "Italian"),
          SwLanguageListItem("JA", "Japanese"),
          SwLanguageListItem("LT", "Lithuanian"),
          SwLanguageListItem("LV", "Dutch"),
          SwLanguageListItem("PL", "Polish"),
          SwLanguageListItem("PT-BR", "Portuguese (Brazilian)"),
          SwLanguageListItem("PT-PT", "Portuguese (European)"),
          SwLanguageListItem("RO", "Romanian"),
          SwLanguageListItem("RU", "Russian"),
          SwLanguageListItem("SK", "Slovak"),
          SwLanguageListItem("SL", "Slovenian"),
          SwLanguageListItem("SV", "Swedish"),
          SwLanguageListItem("TR", "Turkish"),
          SwLanguageListItem("ZH", "Chinese (simplified)"),
      })
    , m_bTranslationStarted(false)
    , m_bCancelTranslation(false)
{
    m_xLanguageListBox->connect_changed(LINK(this, SwTranslateLangSelectDlg, LangSelectHdl));
    m_xBtnCancel->connect_clicked(LINK(this, SwTranslateLangSelectDlg, LangSelectCancelHdl));
    m_xBtnTranslate->connect_clicked(LINK(this, SwTranslateLangSelectDlg, LangSelectTranslateHdl));

    for (const auto& item : m_xLanguageVec)
    {
        m_xLanguageListBox->append_text(OStringToOUString(item.getName(), RTL_TEXTENCODING_UTF8));
    }

    if (SwTranslateLangSelectDlg::selectedLangIdx != -1)
    {
        m_xLanguageListBox->set_active(SwTranslateLangSelectDlg::selectedLangIdx);
    }
}

std::optional<SwLanguageListItem> SwTranslateLangSelectDlg::GetSelectedLanguage()
{
    if (SwTranslateLangSelectDlg::selectedLangIdx != -1)
    {
        return m_xLanguageVec.at(SwTranslateLangSelectDlg::selectedLangIdx);
    }

    return {};
}

IMPL_LINK(SwTranslateLangSelectDlg, LangSelectHdl, weld::ComboBox&, rBox, void)
{
    const auto selected = rBox.get_active();
    SwTranslateLangSelectDlg::selectedLangIdx = selected;
}

IMPL_LINK(SwTranslateLangSelectDlg, LangSelectCancelHdl, weld::Button&, rButton, void)
{
    (void)rButton;

    // stop translation first
    if (m_bTranslationStarted)
        m_bCancelTranslation = true;
    else
        m_xDialog->response(RET_CANCEL);
}

IMPL_LINK(SwTranslateLangSelectDlg, LangSelectTranslateHdl, weld::Button&, rButton, void)
{
    (void)rButton;

    if (SwTranslateLangSelectDlg::selectedLangIdx == -1)
    {
        m_xDialog->response(RET_CANCEL);
        return;
    }

    SvxDeeplOptions& rDeeplOptions = SvxDeeplOptions::Get();
    if (rDeeplOptions.getAPIUrl().isEmpty() || rDeeplOptions.getAuthKey().isEmpty())
    {
        SAL_WARN("langselectdlg", "API options are not set");
        m_xDialog->response(RET_CANCEL);
        return;
    }

    const OString aAPIUrl
        = OUStringToOString(OUString(rDeeplOptions.getAPIUrl() + "?tag_handling=html"),
                            RTL_TEXTENCODING_UTF8)
              .trim();
    const OString aAuthKey
        = OUStringToOString(rDeeplOptions.getAuthKey(), RTL_TEXTENCODING_UTF8).trim();
    const auto aTargetLang
        = m_xLanguageVec.at(SwTranslateLangSelectDlg::selectedLangIdx).getLanguage();

    m_bTranslationStarted = true;

    SwTranslateHelper::TranslateAPIConfig aConfig({ aAPIUrl, aAuthKey, aTargetLang });
    SwTranslateHelper::TranslateDocumentCancellable(rWrtSh, aConfig, m_bCancelTranslation);
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
