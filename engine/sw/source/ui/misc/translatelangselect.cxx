/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <algorithm>
#include <map>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>
#include <svtools/langtab.hxx>
#include <unotools/charclass.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <uitool.hxx>
#include <translatelangselect.hxx>
#include <translatehelper.hxx>

namespace
{
/// Target languages offered when the translation service cannot list its own.
const std::vector<OString>& getDefaultLanguages()
{
    static const std::vector<OString> aLanguages{
        "ar"_ostr,    "bg"_ostr,    "cs"_ostr, "da"_ostr, "de"_ostr,      "el"_ostr,
        "en-GB"_ostr, "en-US"_ostr, "es"_ostr, "et"_ostr, "fi"_ostr,      "fr"_ostr,
        "he"_ostr,    "hu"_ostr,    "id"_ostr, "it"_ostr, "ja"_ostr,      "ko"_ostr,
        "lt"_ostr,    "lv"_ostr,    "nb"_ostr, "nl"_ostr, "pl"_ostr,      "pt-BR"_ostr,
        "pt-PT"_ostr, "ro"_ostr,    "ru"_ostr, "sk"_ostr, "sl"_ostr,      "sv"_ostr,
        "th"_ostr,    "tr"_ostr,    "uk"_ostr, "vi"_ostr, "zh-Hans"_ostr, "zh-Hant"_ostr
    };
    return aLanguages;
}

/// The name of the language rTag in the UI language.
OUString getLanguageName(const LanguageTag& rTag)
{
    // The language table has a translated name for most tags, for a bare language often
    // only under its default locale, e.g. "de-DE" for "de". A language without a known
    // locale falls back to "en-US", whose name is not the language's.
    LanguageType eType = rTag.getLanguageType();
    if (SvtLanguageTable::HasLanguageType(eType))
        return SvtLanguageTable::GetLanguageString(eType);
    LanguageTag aFallbackTag(rTag);
    aFallbackTag.makeFallback();
    if (aFallbackTag.getLanguage() == rTag.getLanguage())
    {
        eType = aFallbackTag.getLanguageType();
        if (SvtLanguageTable::HasLanguageType(eType))
            return SvtLanguageTable::GetLanguageString(eType);
        // zh-Hant has no default locale to fall back to, unlike zh-Hans with zh-CN.
        if (eType == LANGUAGE_CHINESE_TRADITIONAL_LSO)
            return SvtLanguageTable::GetLanguageString(LANGUAGE_CHINESE_TRADITIONAL);
    }

    // An ICU name is written as inside a sentence, which is lowercase in many languages.
    const LanguageTag& rUILanguage = Application::GetSettings().GetUILanguageTag();
    const OUString aName = LanguageTagIcu::getDisplayName(rTag, rUILanguage);
    if (aName.isEmpty())
        return rTag.getBcp47();
    return CharClass(rUILanguage).titlecase(aName, 0, 1) + aName.subView(1);
}
}

std::optional<SwLanguageListItem> SwTranslateLangSelectDlg::s_oSelectedLanguage;

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

    std::vector<OString> aCodes = SwTranslateHelper::GetTargetLanguages();
    if (aCodes.empty())
        aCodes = getDefaultLanguages();

    // A bare language and its default locale, e.g. "de" and "de-DE", get the same name:
    // list only the more specific one.
    std::map<OUString, OString> aCodeByName;
    for (const OString& rCode : aCodes)
    {
        const OUString aName
            = getLanguageName(LanguageTag(OStringToOUString(rCode, RTL_TEXTENCODING_ASCII_US)));
        OString& rListedCode = aCodeByName[aName];
        if (rCode.getLength() > rListedCode.getLength())
            rListedCode = rCode;
    }

    for (const auto & [ rName, rCode ] : aCodeByName)
        m_aLanguages.emplace_back(rCode, rName);

    // List the languages sorted in the UI language.
    const comphelper::string::NaturalStringSorter aSorter(
        comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale());
    std::stable_sort(m_aLanguages.begin(), m_aLanguages.end(),
                     [&aSorter](const SwLanguageListItem& rLHS, const SwLanguageListItem& rRHS) {
                         return aSorter.compare(rLHS.getName(), rRHS.getName()) < 0;
                     });

    for (const SwLanguageListItem& rLanguage : m_aLanguages)
        m_xLanguageListBox->append(
            OStringToOUString(rLanguage.getLanguage(), RTL_TEXTENCODING_ASCII_US),
            rLanguage.getName());

    if (s_oSelectedLanguage)
    {
        const OUString aSelectedId
            = OStringToOUString(s_oSelectedLanguage->getLanguage(), RTL_TEXTENCODING_ASCII_US);
        if (m_xLanguageListBox->find_id(aSelectedId) != -1)
        {
            m_xLanguageListBox->set_active_id(aSelectedId);
            m_oLanguage = s_oSelectedLanguage;
        }
        else
            s_oSelectedLanguage.reset();
    }
    // The box can be typed into: translating needs a language picked from the list.
    m_xBtnTranslate->set_sensitive(m_oLanguage.has_value());
}

IMPL_LINK(SwTranslateLangSelectDlg, LangSelectHdl, weld::ComboBox&, rBox, void)
{
    const sal_Int32 nActive = rBox.get_active();
    if (nActive == -1)
        m_oLanguage.reset();
    else
        m_oLanguage.emplace(m_aLanguages[nActive]);
    if (!m_bTranslationStarted)
        m_xBtnTranslate->set_sensitive(m_oLanguage.has_value());
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
    // The button is only sensitive with a language picked and no translation running.
    if (m_bTranslationStarted || !m_oLanguage)
        return;

    // Remember the language only when translating: typing into the box to search must
    // not lose it when the dialog is cancelled.
    s_oSelectedLanguage = m_oLanguage;
    const OString aTargetLang = m_oLanguage->getLanguage();

    m_bTranslationStarted = true;
    m_xBtnTranslate->set_sensitive(false);
    bool bRet = SwTranslateHelper::TranslateDocumentCancellable(m_rWrtSh, aTargetLang,
                                                                m_bCancelTranslation);
    m_xDialog->response(bRet ? RET_OK : RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
