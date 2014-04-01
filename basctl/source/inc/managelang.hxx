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

#ifndef BASCTL_MANAGELANG_HXX
#define BASCTL_MANAGELANG_HXX

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
    ::com::sun::star::lang::Locale  m_aLocale;
    bool                            m_bIsDefault;

    LanguageEntry( const OUString& _rLanguage,
                   const ::com::sun::star::lang::Locale& _rLocale,
                   bool _bIsDefault ) :
        m_sLanguage( _rLanguage ),
        m_aLocale( _rLocale ),
        m_bIsDefault( _bIsDefault ) {}
};

extern bool localesAreEqual( const ::com::sun::star::lang::Locale& rLocaleLeft,
                             const ::com::sun::star::lang::Locale& rLocaleRight );

class ManageLanguageDialog : public ModalDialog
{
private:
    ListBox*            m_pLanguageLB;
    PushButton*         m_pAddPB;
    PushButton*         m_pDeletePB;
    PushButton*         m_pMakeDefPB;

    boost::shared_ptr<LocalizationMgr> m_xLocalizationMgr;

    OUString            m_sDefLangStr;
    OUString            m_sDeleteStr;
    OUString            m_sCreateLangStr;

    void                Init();
    void                FillLanguageBox();
    void                ClearLanguageBox();

    DECL_LINK(AddHdl, void *);
    DECL_LINK(DeleteHdl, void *);
    DECL_LINK(MakeDefHdl, void *);
    DECL_LINK(SelectHdl, void *);

public:
    ManageLanguageDialog( Window* pParent, boost::shared_ptr<LocalizationMgr> _pLMgr );
    virtual ~ManageLanguageDialog();
};

class SetDefaultLanguageDialog : public ModalDialog
{
private:
    FixedText*          m_pLanguageFT;
    SvxLanguageBox*     m_pLanguageLB;
    FixedText*          m_pCheckLangFT;
    SvxCheckListBox*    m_pCheckLangLB;
    FixedText*          m_pDefinedFT;
    FixedText*          m_pAddedFT;

    boost::shared_ptr<LocalizationMgr> m_xLocalizationMgr;

    void                FillLanguageBox();

public:
    SetDefaultLanguageDialog(Window* pParent, boost::shared_ptr<LocalizationMgr> xLMgr);

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >   GetLocales() const;
};

} // namespace basctl

#endif // BASCTL_MANAGELANG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
