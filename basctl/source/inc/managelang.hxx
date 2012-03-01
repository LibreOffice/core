/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BASCTL_MANAGELANG_HXX
#define _BASCTL_MANAGELANG_HXX

#include <svx/checklbx.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

class LocalizationMgr;
class SvxLanguageBox;

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

    bool                m_bIsDefaultMode;
    boost::shared_ptr<LocalizationMgr> m_pLocalizationMgr;

    void                FillLanguageBox();
    void                CalcInfoSize();

public:
    SetDefaultLanguageDialog( Window* pParent, boost::shared_ptr<LocalizationMgr> _pLMgr );
    ~SetDefaultLanguageDialog();

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >   GetLocales() const;
};

#endif //_BASCTL_MANAGELANG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
