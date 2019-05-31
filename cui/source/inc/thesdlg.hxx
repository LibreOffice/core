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
#ifndef INCLUDED_CUI_SOURCE_INC_THESDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_THESDLG_HXX

#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <svx/checklbx.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/idle.hxx>
#include <sfx2/basedlgs.hxx>

#include <memory>
#include <stack>

class SvxThesaurusDialog;

class LookUpComboBox : public ComboBox
{
    Idle                        m_aModifyIdle;
    VclPtr<SvxThesaurusDialog>         m_pDialog;

    LookUpComboBox( const LookUpComboBox & ) = delete;
    LookUpComboBox& operator = ( const LookUpComboBox & ) = delete;

public:
    LookUpComboBox(vcl::Window *pParent);
    virtual ~LookUpComboBox() override;
    virtual void dispose() override;

    DECL_LINK( ModifyTimer_Hdl, Timer *, void );

    void init(SvxThesaurusDialog *pDialog);

    // ComboBox
    virtual void        Modify() override;
};

class AlternativesExtraData
{
    OUString  sText;
    bool      bHeader;

public:
    AlternativesExtraData() : bHeader( false ) {}
    AlternativesExtraData( const OUString &rText, bool bIsHeader ) :
        sText(rText),
        bHeader(bIsHeader)
        {
        }

    bool  IsHeader() const          { return bHeader; }
    const OUString& GetText() const   { return sText; }
};

class ThesaurusAlternativesCtrl
    : public SvxCheckListBox
{
    VclPtr<SvxThesaurusDialog>     m_pDialog;

    typedef std::map< const SvTreeListEntry *, AlternativesExtraData >  UserDataMap_t;
    UserDataMap_t           m_aUserData;

    ThesaurusAlternativesCtrl( const ThesaurusAlternativesCtrl & ) = delete;
    ThesaurusAlternativesCtrl & operator = ( const ThesaurusAlternativesCtrl & ) = delete;

public:
    ThesaurusAlternativesCtrl(vcl::Window* pParent);

    void init(SvxThesaurusDialog *pDialog);
    virtual ~ThesaurusAlternativesCtrl() override;
    virtual void dispose() override;

    SvTreeListEntry *   AddEntry( sal_Int32 nVal, const OUString &rText, bool bIsHeader );

    void            ClearExtraData();
    void            SetExtraData( const SvTreeListEntry *pEntry, const AlternativesExtraData &rData );
    AlternativesExtraData * GetExtraData( const SvTreeListEntry *pEntry );

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect ) override;
};

class ReplaceEdit : public Edit
{
    VclPtr<Button>       m_pBtn;

    ReplaceEdit( const ReplaceEdit & ) = delete;
    ReplaceEdit & operator = ( const ReplaceEdit & ) = delete;

public:
    ReplaceEdit(vcl::Window *pParent);
    virtual ~ReplaceEdit() override;
    virtual void dispose() override;

    void init(Button *pBtn)  { m_pBtn = pBtn; }

    // Edit
    virtual void        Modify() override;
    virtual void        SetText( const OUString& rStr ) override;
    virtual void        SetText( const OUString& rStr, const Selection& rNewSelection ) override;
};

class SvxThesaurusDialog : public SfxDialogController
{
    OUString                m_aErrStr;

    css::uno::Reference< css::linguistic2::XThesaurus >   xThesaurus;
    OUString                aLookUpText;
    LanguageType            nLookUpLanguage;
    std::stack< OUString >  aLookUpHistory;
    bool                    m_bWordFound;

    std::unique_ptr<weld::Button> m_xLeftBtn;
    std::unique_ptr<weld::ComboBox> m_xWordCB;
    std::unique_ptr<weld::TreeView> m_xAlternativesCT;
    std::unique_ptr<weld::Entry> m_xReplaceEdit;
    std::unique_ptr<weld::ComboBox> m_xLangLB;
    std::unique_ptr<weld::Button> m_xReplaceBtn;

public:
    virtual ~SvxThesaurusDialog() override;

    bool                    WordFound() const { return m_bWordFound; }
    const OUString&         getErrStr() const { return m_aErrStr; }

    // Handler
    DECL_LINK( ReplaceBtnHdl_Impl, weld::Button&, void );
    DECL_LINK( LeftBtnHdl_Impl, weld::Button&, void );
    DECL_LINK( LanguageHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( WordSelectHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( AlternativesSelectHdl_Impl, weld::TreeView&, void );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, weld::TreeView&, void );
    DECL_LINK( SelectFirstHdl_Impl, void*, void );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XMeaning > >
            queryMeanings_Impl( OUString& rTerm, const css::lang::Locale& rLocale, const css::beans::PropertyValues& rProperties );

    bool    UpdateAlternativesBox_Impl();
    void    LookUp( const OUString &rText );
    void    LookUp_Impl();

public:
    SvxThesaurusDialog(weld::Window* pParent,
                       css::uno::Reference< css::linguistic2::XThesaurus > const & xThesaurus,
                       const OUString &rWord, LanguageType nLanguage);
    void            SetWindowTitle( LanguageType nLanguage );
    OUString        GetWord();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
