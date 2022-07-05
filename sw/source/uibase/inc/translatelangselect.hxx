
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
#pragma once
#include <vcl/weld.hxx>
#include <rtl/string.h>
#include <vector>
#include <optional>
#include <translatehelper.hxx>

class SwWrtShell;

// SwLanguageListItem Helper class for displaying available languages with their names and tags on the listbox.
class SwLanguageListItem final
{
public:
    SwLanguageListItem(const OString& sLanguage, const OString& sName)
        : m_sLanguage(sLanguage)
        , m_sName(sName)
    {
    }
    const OString& getLanguage() const { return m_sLanguage; }
    const OString& getName() const { return m_sName; }

private:
    const OString m_sLanguage;
    const OString m_sName;
};

// SwTranslateLangSelectDlg Language selection dialog for translation API.
// Also responsible for iterating the nodes for translation
class SwTranslateLangSelectDlg final : public weld::GenericDialogController
{
public:
    static int selectedLangIdx;
    SwTranslateLangSelectDlg(weld::Window* pParent, SwWrtShell& rSh);
    std::optional<SwLanguageListItem> GetSelectedLanguage();

private:
    SwWrtShell& rWrtSh;
    std::unique_ptr<weld::ComboBox> m_xLanguageListBox;
    std::unique_ptr<weld::Button> m_xBtnCancel;
    std::unique_ptr<weld::Button> m_xBtnTranslate;
    std::vector<SwLanguageListItem> m_xLanguageVec;

    bool m_bTranslationStarted;
    bool m_bCancelTranslation;

    DECL_LINK(LangSelectHdl, weld::ComboBox&, void);
    DECL_LINK(LangSelectCancelHdl, weld::Button&, void);
    DECL_LINK(LangSelectTranslateHdl, weld::Button&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
