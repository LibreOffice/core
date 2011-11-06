/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BASCTL_MANAGELANG_HXX
#define _BASCTL_MANAGELANG_HXX

#include <vcl/dialog.hxx>
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svx/langbox.hxx>
#include <svx/checklbx.hxx>

class LocalizationMgr;

struct LanguageEntry
{
    String                          m_sLanguage;
    ::com::sun::star::lang::Locale  m_aLocale;
    bool                            m_bIsDefault;

    LanguageEntry( const String& _rLanguage,
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

    LocalizationMgr*    m_pLocalizationMgr;

    String              m_sDefLangStr;
    String              m_sDeleteStr;
    String              m_sCreateLangStr;

    void                Init();
    void                CalcInfoSize();
    void                FillLanguageBox();
    void                ClearLanguageBox();

    DECL_LINK(          AddHdl, Button * );
    DECL_LINK(          DeleteHdl, Button * );
    DECL_LINK(          MakeDefHdl, Button * );
    DECL_LINK(          SelectHdl, ListBox * );

public:
    ManageLanguageDialog( Window* pParent, LocalizationMgr* _pLMgr );
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
    LocalizationMgr*    m_pLocalizationMgr;

    void                FillLanguageBox();
    void                CalcInfoSize();

public:
    SetDefaultLanguageDialog( Window* pParent, LocalizationMgr* _pLMgr );
    ~SetDefaultLanguageDialog();

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >   GetLocales() const;
};

#endif //_BASCTL_MANAGELANG_HXX

