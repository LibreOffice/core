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
#include <vcl/idle.hxx>
#include <mdiexp.hxx>
#include <strings.hrc>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <linguistic/translate.hxx>
#include <officecfg/Office/Linguistic.hxx>

static const std::vector<SwLanguageListItem>& getLanguageVec()
{
    static const std::vector<SwLanguageListItem> gLanguageVec{
        SwLanguageListItem("BG"_ostr, "Bulgarian"_ostr),
        SwLanguageListItem("CS"_ostr, "Czech"_ostr),
        SwLanguageListItem("DA"_ostr, "Danish"_ostr),
        SwLanguageListItem("DE"_ostr, "German"_ostr),
        SwLanguageListItem("EL"_ostr, "Greek"_ostr),
        SwLanguageListItem("EN-GB"_ostr, "English (British)"_ostr),
        SwLanguageListItem("EN-US"_ostr, "English (American)"_ostr),
        SwLanguageListItem("ES"_ostr, "Spanish"_ostr),
        SwLanguageListItem("ET"_ostr, "Estonian"_ostr),
        SwLanguageListItem("FI"_ostr, "Finnish"_ostr),
        SwLanguageListItem("FR"_ostr, "French"_ostr),
        SwLanguageListItem("HU"_ostr, "Hungarian"_ostr),
        SwLanguageListItem("ID"_ostr, "Indonesian"_ostr),
        SwLanguageListItem("IT"_ostr, "Italian"_ostr),
        SwLanguageListItem("JA"_ostr, "Japanese"_ostr),
        SwLanguageListItem("LT"_ostr, "Lithuanian"_ostr),
        SwLanguageListItem("LV"_ostr, "Latvian"_ostr),
        SwLanguageListItem("NL"_ostr, "Dutch"_ostr),
        SwLanguageListItem("PL"_ostr, "Polish"_ostr),
        SwLanguageListItem("PT-BR"_ostr, "Portuguese (Brazilian)"_ostr),
        SwLanguageListItem("PT-PT"_ostr, "Portuguese (European)"_ostr),
        SwLanguageListItem("RO"_ostr, "Romanian"_ostr),
        SwLanguageListItem("RU"_ostr, "Russian"_ostr),
        SwLanguageListItem("SK"_ostr, "Slovak"_ostr),
        SwLanguageListItem("SL"_ostr, "Slovenian"_ostr),
        SwLanguageListItem("SV"_ostr, "Swedish"_ostr),
        SwLanguageListItem("TR"_ostr, "Turkish"_ostr),
        SwLanguageListItem("ZH"_ostr, "Chinese (simplified)"_ostr)
    };
    return gLanguageVec;
}

int SwTranslateLangSelectDlg::selectedLangIdx = -1;
SwTranslateLangSelectDlg::SwTranslateLangSelectDlg(weld::Window* pParent, SwWrtShell& rSh)
    : GenericDialogController(pParent, u"modules/swriter/ui/translationdialog.ui"_ustr,
                              u"LanguageSelectDialog"_ustr)
    , m_rWrtSh(rSh)
    , m_xLanguageListBox(m_xBuilder->weld_combo_box(u"combobox1"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xBtnTranslate(m_xBuilder->weld_button(u"translate"_ustr))
    , m_bTranslationStarted(false)
    , m_bCancelTranslation(false)
{
    m_xLanguageListBox->connect_changed(LINK(this, SwTranslateLangSelectDlg, LangSelectHdl));
    m_xBtnCancel->connect_clicked(LINK(this, SwTranslateLangSelectDlg, LangSelectCancelHdl));
    m_xBtnTranslate->connect_clicked(LINK(this, SwTranslateLangSelectDlg, LangSelectTranslateHdl));

    for (const auto& item : getLanguageVec())
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
        return getLanguageVec().at(SwTranslateLangSelectDlg::selectedLangIdx);
    }

    return {};
}

IMPL_STATIC_LINK(SwTranslateLangSelectDlg, LangSelectHdl, weld::ComboBox&, rBox, void)
{
    const auto selected = rBox.get_active();
    SwTranslateLangSelectDlg::selectedLangIdx = selected;
}

IMPL_LINK_NOARG(SwTranslateLangSelectDlg, LangSelectCancelHdl, weld::Button&, void)
{
    // stop translation first
    if (m_bTranslationStarted)
        m_bCancelTranslation = true;
    else
        m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(SwTranslateLangSelectDlg, LangSelectTranslateHdl, weld::Button&, void)
{
    if (m_bTranslationStarted)
        return;
    if (SwTranslateLangSelectDlg::selectedLangIdx == -1)
    {
        m_xDialog->response(RET_CANCEL);
        return;
    }

    std::optional<OUString> oDeeplAPIUrl
        = officecfg::Office::Linguistic::Translation::Deepl::ApiURL::get();
    std::optional<OUString> oDeeplKey
        = officecfg::Office::Linguistic::Translation::Deepl::AuthKey::get();
    if (!oDeeplAPIUrl || oDeeplAPIUrl->isEmpty() || !oDeeplKey || oDeeplKey->isEmpty())
    {
        SAL_WARN("sw.ui", "SwTranslateLangSelectDlg: API options are not set");
        m_xDialog->response(RET_CANCEL);
        return;
    }

    const OString aAPIUrl
        = OUStringToOString(rtl::Concat2View(*oDeeplAPIUrl + "?tag_handling=html"),
                            RTL_TEXTENCODING_UTF8)
              .trim();
    const OString aAuthKey = OUStringToOString(*oDeeplKey, RTL_TEXTENCODING_UTF8).trim();
    const auto aTargetLang
        = getLanguageVec().at(SwTranslateLangSelectDlg::selectedLangIdx).getLanguage();

    m_bTranslationStarted = true;

    SwTranslateHelper::TranslateAPIConfig aConfig({ aAPIUrl, aAuthKey, aTargetLang });
    SwTranslateHelper::TranslateDocumentCancellable(m_rWrtSh, aConfig, m_bCancelTranslation);
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
