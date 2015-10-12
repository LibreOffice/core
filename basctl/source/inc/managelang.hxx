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

class SvxLanguageBox;

namespace basctl
{

class LocalizationMgr;

struct LanguageEntry
{
    OUString                        m_sLanguage;
    css::lang::Locale               m_aLocale;
    bool                            m_bIsDefault;

    LanguageEntry( const OUString& _rLanguage,
                   const css::lang::Locale& _rLocale,
                   bool _bIsDefault ) :
        m_sLanguage( _rLanguage ),
        m_aLocale( _rLocale ),
        m_bIsDefault( _bIsDefault ) {}
};

extern bool localesAreEqual( const css::lang::Locale& rLocaleLeft,
                             const css::lang::Locale& rLocaleRight );

class ManageLanguageDialog : public ModalDialog
{
private:
    VclPtr<ListBox>            m_pLanguageLB;
    VclPtr<PushButton>         m_pAddPB;
    VclPtr<PushButton>         m_pDeletePB;
    VclPtr<PushButton>         m_pMakeDefPB;

    std::shared_ptr<LocalizationMgr> m_xLocalizationMgr;

    OUString            m_sDefLangStr;
    OUString            m_sCreateLangStr;

    void                Init();
    void                FillLanguageBox();
    void                ClearLanguageBox();

    DECL_LINK_TYPED(AddHdl, Button*, void);
    DECL_LINK_TYPED(DeleteHdl, Button*, void);
    DECL_LINK_TYPED(MakeDefHdl, Button*, void);
    DECL_LINK_TYPED(SelectHdl, ListBox&, void);

public:
    ManageLanguageDialog( vcl::Window* pParent, std::shared_ptr<LocalizationMgr> _pLMgr );
    virtual ~ManageLanguageDialog();
    virtual void dispose() override;
};

class SetDefaultLanguageDialog : public ModalDialog
{
private:
    VclPtr<FixedText>          m_pLanguageFT;
    VclPtr<SvxLanguageBox>     m_pLanguageLB;
    VclPtr<FixedText>          m_pCheckLangFT;
    VclPtr<SvxCheckListBox>    m_pCheckLangLB;
    VclPtr<FixedText>          m_pDefinedFT;
    VclPtr<FixedText>          m_pAddedFT;

    std::shared_ptr<LocalizationMgr> m_xLocalizationMgr;

    void                FillLanguageBox();

public:
    SetDefaultLanguageDialog(vcl::Window* pParent, std::shared_ptr<LocalizationMgr> xLMgr);
    virtual ~SetDefaultLanguageDialog();
    virtual void dispose() override;

    css::uno::Sequence< css::lang::Locale >   GetLocales() const;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_MANAGELANG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
