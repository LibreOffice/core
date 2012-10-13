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
#ifndef _SVX_THESDLG_HXX
#define _SVX_THESDLG_HXX


#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <svx/checklbx.hxx>
#include <svx/stddlg.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/menubtn.hxx>

#include <memory>

using namespace ::com::sun::star;

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
    virtual void        Modify();
};

class AlternativesExtraData
{
    String  sText;
    bool    bHeader;

public:
    AlternativesExtraData() : bHeader( false ) {}
    AlternativesExtraData( const String &rText, bool bIsHeader ) :
        sText(rText),
        bHeader(bIsHeader)
        {
        }

    bool  IsHeader() const          { return bHeader; }
    const String& GetText() const   { return sText; }
};

class ThesaurusAlternativesCtrl
    : public SvxCheckListBox
{
    SvxThesaurusDialog*     m_pDialog;

    typedef std::map< const SvLBoxEntry *, AlternativesExtraData >  UserDataMap_t;
    UserDataMap_t           m_aUserData;

    /// disable copy ctor and assignment operator
    ThesaurusAlternativesCtrl( const ThesaurusAlternativesCtrl & );
    ThesaurusAlternativesCtrl & operator = ( const ThesaurusAlternativesCtrl & );

public:
    ThesaurusAlternativesCtrl(Window* pParent);

    void init(SvxThesaurusDialog *pDialog);
    virtual ~ThesaurusAlternativesCtrl();


    SvLBoxEntry *   AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader );

    void            ClearExtraData();
    void            SetExtraData( const SvLBoxEntry *pEntry, const AlternativesExtraData &rData );
    AlternativesExtraData * GetExtraData( const SvLBoxEntry *pEntry );

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
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
    virtual void        Modify();
    virtual void        SetText( const XubString& rStr );
    virtual void        SetText( const XubString& rStr, const Selection& rNewSelection );
};

class SvxThesaurusDialog : public SvxStandardDialog
{
    PushButton*             m_pLeftBtn;
    LookUpComboBox*         m_pWordCB;
    ThesaurusAlternativesCtrl* m_pAlternativesCT;
    ReplaceEdit*            m_pReplaceEdit;
    MenuButton*             m_pLangMBtn;

    OUString                m_aErrStr;

    uno::Reference< linguistic2::XThesaurus >   xThesaurus;
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
    DECL_LINK( LanguageHdl_Impl, MenuButton * );
    DECL_LINK( LookUpHdl_Impl, Button * );
    DECL_LINK( WordSelectHdl_Impl, ComboBox * );
    DECL_LINK( AlternativesSelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, SvxCheckListBox * );

    DECL_STATIC_LINK( SvxThesaurusDialog, SelectFirstHdl_Impl, SvxCheckListBox * );

    uno::Sequence< uno::Reference< linguistic2::XMeaning > >
            queryMeanings_Impl( ::rtl::OUString& rTerm, const lang::Locale& rLocale, const beans::PropertyValues& rProperties ) throw(lang::IllegalArgumentException, uno::RuntimeException);

    bool    UpdateAlternativesBox_Impl();
    void    LookUp( const String &rText );
    void    LookUp_Impl();
    virtual void     Apply();

public:
    SvxThesaurusDialog( Window* pParent,
                        uno::Reference< linguistic2::XThesaurus >  xThesaurus,
                        const String &rWord, LanguageType nLanguage );
    ~SvxThesaurusDialog();

    void            SetWindowTitle( LanguageType nLanguage );
    String          GetWord();
    sal_uInt16      GetLanguage() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
