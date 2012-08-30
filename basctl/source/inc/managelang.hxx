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
    ::rtl::OUString                 m_sLanguage;
    ::com::sun::star::lang::Locale  m_aLocale;
    bool                            m_bIsDefault;

    LanguageEntry( const ::rtl::OUString& _rLanguage,
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
    FixedText           m_aLanguageFT;
    ListBox             m_aLanguageLB;
    PushButton          m_aAddPB;
    PushButton          m_aDeletePB;
    PushButton          m_aMakeDefPB;
    FixedText           m_aInfoFT;

    FixedLine           m_aBtnLine;
    HelpButton          m_aHelpBtn;
    OKButton            m_aCloseBtn;

    boost::shared_ptr<LocalizationMgr> m_pLocalizationMgr;

    ::rtl::OUString     m_sDefLangStr;
    ::rtl::OUString     m_sDeleteStr;
    ::rtl::OUString     m_sCreateLangStr;

    void                Init();
    void                CalcInfoSize();
    void                FillLanguageBox();
    void                ClearLanguageBox();

    DECL_LINK(AddHdl, void *);
    DECL_LINK(DeleteHdl, void *);
    DECL_LINK(MakeDefHdl, void *);
    DECL_LINK(SelectHdl, void *);

public:
    ManageLanguageDialog( Window* pParent, boost::shared_ptr<LocalizationMgr> _pLMgr );
    ~ManageLanguageDialog();
};

class SetDefaultLanguageDialog : public ModalDialog
{
private:
    FixedText           m_aLanguageFT;
    SvxLanguageBox*     m_pLanguageLB;
    SvxCheckListBox*    m_pCheckLangLB;
    FixedText           m_aInfoFT;

    FixedLine           m_aBtnLine;
    OKButton            m_aOKBtn;
    CancelButton        m_aCancelBtn;
    HelpButton          m_aHelpBtn;

    boost::shared_ptr<LocalizationMgr> m_pLocalizationMgr;

    void                FillLanguageBox();
    void                CalcInfoSize();

public:
    SetDefaultLanguageDialog( Window* pParent, boost::shared_ptr<LocalizationMgr> _pLMgr );
    ~SetDefaultLanguageDialog();

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >   GetLocales() const;
};

} // namespace basctl

#endif // BASCTL_MANAGELANG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
