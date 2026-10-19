
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
#pragma once
#include <vcl/weld.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <optional>
#include <vector>

class SwWrtShell;

// SwLanguageListItem Helper class for displaying available languages with their names and tags on the listbox.
class SwLanguageListItem final
{
public:
    SwLanguageListItem(const OString& sLanguage, const OUString& sName)
        : m_sLanguage(sLanguage)
        , m_sName(sName)
    {
    }
    /// The BCP 47 tag of the language, e.g. "en-GB", as the translation provider lists it.
    const OString& getLanguage() const { return m_sLanguage; }
    /// The name of the language in the UI language.
    const OUString& getName() const { return m_sName; }

private:
    OString m_sLanguage;
    OUString m_sName;
};

// SwTranslateLangSelectDlg Language selection dialog for translation API.
class SwTranslateLangSelectDlg final : public weld::GenericDialogController
{
public:
    SwTranslateLangSelectDlg(weld::Window* pParent, SwWrtShell& rSh);

private:
    /// The language last picked in the dialog, offered again the next time.
    static std::optional<SwLanguageListItem> s_oSelectedLanguage;

    SwWrtShell& m_rWrtSh;
    std::unique_ptr<weld::ComboBox> m_xLanguageListBox;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnTranslate;
    /// The languages in the order they are listed.
    std::vector<SwLanguageListItem> m_aLanguages;
    /// The language picked in the box, if the box holds one.
    std::optional<SwLanguageListItem> m_oLanguage;

    bool m_bTranslationStarted;
    bool m_bCancelTranslation;

    DECL_LINK(LangSelectHdl, weld::ComboBox&, void);
    DECL_LINK(LangSelectCancelHdl, weld::Button&, void);
    DECL_LINK(LangSelectTranslateHdl, weld::Button&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
