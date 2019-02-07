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

#ifndef INCLUDED_BASCTL_SOURCE_INC_MANAGELANG_HXX
#define INCLUDED_BASCTL_SOURCE_INC_MANAGELANG_HXX

#include <svx/checklbx.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

class LanguageBox;

namespace basctl
{

class LocalizationMgr;

struct LanguageEntry
{
    css::lang::Locale               m_aLocale;
    bool                            m_bIsDefault;

    LanguageEntry( const css::lang::Locale& _rLocale,
                   bool _bIsDefault ) :
        m_aLocale( _rLocale ),
        m_bIsDefault( _bIsDefault ) {}
};

extern bool localesAreEqual( const css::lang::Locale& rLocaleLeft,
                             const css::lang::Locale& rLocaleRight );

class ManageLanguageDialog : public weld::GenericDialogController
{
private:
    std::shared_ptr<LocalizationMgr> m_xLocalizationMgr;

    OUString            m_sDefLangStr;
    OUString            m_sCreateLangStr;

    std::unique_ptr<weld::TreeView> m_xLanguageLB;
    std::unique_ptr<weld::Button> m_xAddPB;
    std::unique_ptr<weld::Button> m_xDeletePB;
    std::unique_ptr<weld::Button> m_xMakeDefPB;

    void                Init();
    void                FillLanguageBox();
    void                ClearLanguageBox();

    DECL_LINK(AddHdl, weld::Button&, void);
    DECL_LINK(DeleteHdl, weld::Button&, void);
    DECL_LINK(MakeDefHdl, weld::Button&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    ManageLanguageDialog(weld::Window* pParent, std::shared_ptr<LocalizationMgr> const & _pLMgr);
    virtual ~ManageLanguageDialog() override;
};

class SetDefaultLanguageDialog : public weld::GenericDialogController
{
private:
    std::shared_ptr<LocalizationMgr> m_xLocalizationMgr;

    void                FillLanguageBox();

    std::unique_ptr<weld::Label> m_xLanguageFT;
    std::unique_ptr<weld::TreeView> m_xLanguageLB;
    std::unique_ptr<weld::Label> m_xCheckLangFT;
    std::unique_ptr<weld::TreeView> m_xCheckLangLB;
    std::unique_ptr<weld::Label> m_xDefinedFT;
    std::unique_ptr<weld::Label> m_xAddedFT;
    std::unique_ptr<weld::Label> m_xAltTitle;
    std::unique_ptr<::LanguageBox> m_xLanguageCB;

public:
    SetDefaultLanguageDialog(weld::Window* pParent, std::shared_ptr<LocalizationMgr> const & xLMgr);
    virtual ~SetDefaultLanguageDialog() override;

    css::uno::Sequence< css::lang::Locale >   GetLocales() const;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_MANAGELANG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
