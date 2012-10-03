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

class LookUpComboBox_Impl : public ComboBox
{
    Timer                       m_aModifyTimer;
    Selection                   m_aSelection;
    Button *                    m_pBtn;
    SvxThesaurusDialog&         m_rDialogImpl;

    /// disable copy ctor and assignment operator
    LookUpComboBox_Impl( const LookUpComboBox_Impl & );
    LookUpComboBox_Impl & operator = ( const LookUpComboBox_Impl & );

public:
    LookUpComboBox_Impl( Window *pParent, const ResId &rResId, SvxThesaurusDialog &rImpl );
    virtual ~LookUpComboBox_Impl();

    DECL_LINK( ModifyTimer_Hdl, Timer * );

    void SetButton( Button *pBtn )  { m_pBtn = pBtn; }

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

class ThesaurusAlternativesCtrl_Impl :
    public SvxCheckListBox
{
    SvxThesaurusDialog&     m_rDialogImpl;

    typedef std::map< const SvLBoxEntry *, AlternativesExtraData >  UserDataMap_t;
    UserDataMap_t           m_aUserData;

    /// disable copy ctor and assignment operator
    ThesaurusAlternativesCtrl_Impl( const ThesaurusAlternativesCtrl_Impl & );
    ThesaurusAlternativesCtrl_Impl & operator = ( const ThesaurusAlternativesCtrl_Impl & );

public:
    ThesaurusAlternativesCtrl_Impl( Window* pParent, SvxThesaurusDialog &rImpl );
    virtual ~ThesaurusAlternativesCtrl_Impl();


    SvLBoxEntry *   AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader );

    void            ClearExtraData();
    void            SetExtraData( const SvLBoxEntry *pEntry, const AlternativesExtraData &rData );
    AlternativesExtraData * GetExtraData( const SvLBoxEntry *pEntry );

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
};

class ReplaceEdit_Impl : public Edit
{
    Button *                    m_pBtn;

    /// disable copy ctor and assignment operator
    ReplaceEdit_Impl( const ReplaceEdit_Impl & );
    ReplaceEdit_Impl & operator = ( const ReplaceEdit_Impl & );

public:
    ReplaceEdit_Impl( Window *pParent, const ResId &rResId );
    virtual ~ReplaceEdit_Impl();

    void SetButton( Button *pBtn )  { m_pBtn = pBtn; }

    // Edit
    virtual void        Modify();
    virtual void        SetText( const XubString& rStr );
    virtual void        SetText( const XubString& rStr, const Selection& rNewSelection );
};

class SvxThesaurusDialog : public SvxStandardDialog
{
    FixedImage              aVendorImageFI;
    ImageButton             aLeftBtn;
    FixedText               aWordText;
    LookUpComboBox_Impl     aWordCB;
    FixedText               m_aAlternativesText;
    boost::shared_ptr< ThesaurusAlternativesCtrl_Impl > m_pAlternativesCT;
    FixedText               aReplaceText;
    ReplaceEdit_Impl        aReplaceEdit;
    FixedLine               aFL;
    HelpButton              aHelpBtn;
    MenuButton              aLangMBtn;
    OKButton                aReplaceBtn;
    CancelButton            aCancelBtn;

    String          aErrStr;
    Image           aVendorDefaultImage;

    uno::Reference< linguistic2::XThesaurus >   xThesaurus;
    OUString                aLookUpText;
    LanguageType            nLookUpLanguage;
    std::stack< OUString >  aLookUpHistory;
    bool                    m_bWordFound;

public:
    bool                    WordFound() const { return m_bWordFound; }
    OUString                getErrStr() const { return aErrStr; }

    // Handler
    DECL_LINK( LeftBtnHdl_Impl, Button * );
    DECL_LINK( LanguageHdl_Impl, MenuButton * );
    DECL_LINK( LookUpHdl_Impl, Button * );
    DECL_LINK( WordSelectHdl_Impl, ComboBox * );
    DECL_LINK( AlternativesSelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, SvxCheckListBox * );

    DECL_STATIC_LINK( SvxThesaurusDialog, SelectFirstHdl_Impl, SvxCheckListBox * );
    DECL_STATIC_LINK( SvxThesaurusDialog, VendorImageInitHdl, SvxThesaurusDialog * );

    uno::Sequence< uno::Reference< linguistic2::XMeaning > >
            queryMeanings_Impl( ::rtl::OUString& rTerm, const lang::Locale& rLocale, const beans::PropertyValues& rProperties ) throw(lang::IllegalArgumentException, uno::RuntimeException);

    bool    UpdateAlternativesBox_Impl();
    void    UpdateVendorImage();
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
