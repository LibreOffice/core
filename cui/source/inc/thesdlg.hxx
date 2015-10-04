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
#include <svx/stddlg.hxx>

#include <memory>

class SvxThesaurusDialog;

class LookUpComboBox : public ComboBox
{
    Idle                        m_aModifyIdle;
    VclPtr<SvxThesaurusDialog>         m_pDialog;

    LookUpComboBox( const LookUpComboBox & ) SAL_DELETED_FUNCTION;
    LookUpComboBox& operator = ( const LookUpComboBox & ) SAL_DELETED_FUNCTION;

public:
    LookUpComboBox(vcl::Window *pParent);
    virtual ~LookUpComboBox();
    virtual void dispose() SAL_OVERRIDE;

    DECL_LINK_TYPED( ModifyTimer_Hdl, Idle *, void );

    void init(SvxThesaurusDialog *pDialog);

    // ComboBox
    virtual void        Modify() SAL_OVERRIDE;
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

    ThesaurusAlternativesCtrl( const ThesaurusAlternativesCtrl & ) SAL_DELETED_FUNCTION;
    ThesaurusAlternativesCtrl & operator = ( const ThesaurusAlternativesCtrl & ) SAL_DELETED_FUNCTION;

public:
    ThesaurusAlternativesCtrl(vcl::Window* pParent);

    void init(SvxThesaurusDialog *pDialog);
    virtual ~ThesaurusAlternativesCtrl();
    virtual void dispose() SAL_OVERRIDE;

    SvTreeListEntry *   AddEntry( sal_Int32 nVal, const OUString &rText, bool bIsHeader );

    void            ClearExtraData();
    void            SetExtraData( const SvTreeListEntry *pEntry, const AlternativesExtraData &rData );
    AlternativesExtraData * GetExtraData( const SvTreeListEntry *pEntry );

    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;
};

class ReplaceEdit : public Edit
{
    VclPtr<Button>       m_pBtn;

    ReplaceEdit( const ReplaceEdit & ) SAL_DELETED_FUNCTION;
    ReplaceEdit & operator = ( const ReplaceEdit & ) SAL_DELETED_FUNCTION;

public:
    ReplaceEdit(vcl::Window *pParent);
    virtual ~ReplaceEdit();
    virtual void dispose() SAL_OVERRIDE;

    void init(Button *pBtn)  { m_pBtn = pBtn; }

    // Edit
    virtual void        Modify() SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr, const Selection& rNewSelection ) SAL_OVERRIDE;
};

class SvxThesaurusDialog : public SvxStandardDialog
{
    VclPtr<PushButton>             m_pLeftBtn;
    VclPtr<LookUpComboBox>         m_pWordCB;
    VclPtr<ThesaurusAlternativesCtrl> m_pAlternativesCT;
    VclPtr<ReplaceEdit>            m_pReplaceEdit;
    VclPtr<ListBox>                m_pLangLB;

    OUString                m_aErrStr;

    css::uno::Reference< css::linguistic2::XThesaurus >   xThesaurus;
    OUString                aLookUpText;
    LanguageType            nLookUpLanguage;
    std::stack< OUString >  aLookUpHistory;
    bool                    m_bWordFound;

public:
    virtual ~SvxThesaurusDialog();
    virtual void dispose() SAL_OVERRIDE;

    bool                    WordFound() const { return m_bWordFound; }
    OUString                getErrStr() const { return m_aErrStr; }

    // Handler
    DECL_LINK_TYPED( ReplaceBtnHdl_Impl, Button *, void );
    DECL_LINK_TYPED( LeftBtnHdl_Impl, Button *, void );
    DECL_LINK_TYPED( LanguageHdl_Impl, ListBox&, void );
    DECL_LINK( WordSelectHdl_Impl, ComboBox * );
    DECL_LINK_TYPED( AlternativesSelectHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( AlternativesDoubleClickHdl_Impl, SvTreeListBox*, bool );

    DECL_STATIC_LINK_TYPED( SvxThesaurusDialog, SelectFirstHdl_Impl, void*, void );

    css::uno::Sequence< css::uno::Reference< css::linguistic2::XMeaning > >
            queryMeanings_Impl( OUString& rTerm, const css::lang::Locale& rLocale, const css::beans::PropertyValues& rProperties ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException);

    bool    UpdateAlternativesBox_Impl();
    void    LookUp( const OUString &rText );
    void    LookUp_Impl();
    virtual void     Apply() SAL_OVERRIDE;

public:
    SvxThesaurusDialog( vcl::Window* pParent,
                        css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                        const OUString &rWord, LanguageType nLanguage );

    void            SetWindowTitle( LanguageType nLanguage );
    OUString        GetWord();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
