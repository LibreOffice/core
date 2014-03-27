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
#include <svx/stddlg.hxx>

#include <memory>

class SvxThesaurusDialog;

class LookUpComboBox : public ComboBox
{
    Timer                       m_aModifyTimer;
    Selection                   m_aSelection;
    SvxThesaurusDialog*         m_pDialog;

    /// disable copy ctor and assignment operator
    LookUpComboBox( const LookUpComboBox & );
    LookUpComboBox& operator = ( const LookUpComboBox & );

public:
    LookUpComboBox(Window *pParent);
    virtual ~LookUpComboBox();

    DECL_LINK( ModifyTimer_Hdl, Timer * );

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
    SvxThesaurusDialog*     m_pDialog;

    typedef std::map< const SvTreeListEntry *, AlternativesExtraData >  UserDataMap_t;
    UserDataMap_t           m_aUserData;

    /// disable copy ctor and assignment operator
    ThesaurusAlternativesCtrl( const ThesaurusAlternativesCtrl & );
    ThesaurusAlternativesCtrl & operator = ( const ThesaurusAlternativesCtrl & );

public:
    ThesaurusAlternativesCtrl(Window* pParent);

    void init(SvxThesaurusDialog *pDialog);
    virtual ~ThesaurusAlternativesCtrl();


    SvTreeListEntry *   AddEntry( sal_Int32 nVal, const OUString &rText, bool bIsHeader );

    void            ClearExtraData();
    void            SetExtraData( const SvTreeListEntry *pEntry, const AlternativesExtraData &rData );
    AlternativesExtraData * GetExtraData( const SvTreeListEntry *pEntry );

    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
};

class ReplaceEdit : public Edit
{
    Button *                    m_pBtn;

    /// disable copy ctor and assignment operator
    ReplaceEdit( const ReplaceEdit & );
    ReplaceEdit & operator = ( const ReplaceEdit & );

public:
    ReplaceEdit(Window *pParent);
    virtual ~ReplaceEdit();

    void init(Button *pBtn)  { m_pBtn = pBtn; }

    // Edit
    virtual void        Modify() SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr, const Selection& rNewSelection ) SAL_OVERRIDE;
};

class SvxThesaurusDialog : public SvxStandardDialog
{
    PushButton*             m_pLeftBtn;
    LookUpComboBox*         m_pWordCB;
    ThesaurusAlternativesCtrl* m_pAlternativesCT;
    ReplaceEdit*            m_pReplaceEdit;
    ListBox*                m_pLangLB;

    OUString                m_aErrStr;

    css::uno::Reference< css::linguistic2::XThesaurus >   xThesaurus;
    OUString                aLookUpText;
    LanguageType            nLookUpLanguage;
    std::stack< OUString >  aLookUpHistory;
    bool                    m_bWordFound;

public:
    bool                    WordFound() const { return m_bWordFound; }
    OUString                getErrStr() const { return m_aErrStr; }

    // Handler
    DECL_LINK( ReplaceBtnHdl_Impl, Button * );
    DECL_LINK( LeftBtnHdl_Impl, Button * );
    DECL_LINK( LanguageHdl_Impl, ListBox * );
    DECL_LINK( LookUpHdl_Impl, Button * );
    DECL_LINK( WordSelectHdl_Impl, ComboBox * );
    DECL_LINK( AlternativesSelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, SvxCheckListBox * );

    DECL_STATIC_LINK( SvxThesaurusDialog, SelectFirstHdl_Impl, SvxCheckListBox * );

    css::uno::Sequence< css::uno::Reference< css::linguistic2::XMeaning > >
            queryMeanings_Impl( OUString& rTerm, const css::lang::Locale& rLocale, const css::beans::PropertyValues& rProperties ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException);

    bool    UpdateAlternativesBox_Impl();
    void    LookUp( const OUString &rText );
    void    LookUp_Impl();
    virtual void     Apply() SAL_OVERRIDE;

public:
    SvxThesaurusDialog( Window* pParent,
                        css::uno::Reference< css::linguistic2::XThesaurus >  xThesaurus,
                        const OUString &rWord, LanguageType nLanguage );
    ~SvxThesaurusDialog();

    void            SetWindowTitle( LanguageType nLanguage );
    OUString        GetWord();
    sal_uInt16      GetLanguage() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
