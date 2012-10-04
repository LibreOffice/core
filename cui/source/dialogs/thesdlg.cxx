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

#include "thesdlg.hxx"
#include "thesdlg_impl.hxx"
#include "cuires.hrc"
#include "dialmgr.hxx"

#include <tools/shl.hxx>
#include <svl/lngmisc.hxx>
#include <svtools/filter.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <thesdlg.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svxerr.hxx>
#include <editeng/unolingu.hxx>
#include <svx/langbox.hxx>
#include <svtools/langtab.hxx>
#include <unotools/lingucfg.hxx>
#include <i18npool/mslangid.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <osl/file.hxx>

#include <stack>
#include <algorithm>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>

using namespace ::com::sun::star;
using ::rtl::OUString;

// class LookUpComboBox --------------------------------------------------

LookUpComboBox::LookUpComboBox(Window *pParent)
    : ComboBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK)
{
    SetBestDropDownLineCount();

    m_aModifyTimer.SetTimeoutHdl( LINK( this, LookUpComboBox, ModifyTimer_Hdl ) );
    m_aModifyTimer.SetTimeout( 500 );

    EnableAutocomplete( sal_False );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeLookUpComboBox(Window *pParent, VclBuilder::stringmap &)
{
    return new LookUpComboBox(pParent);
}

void LookUpComboBox::init(SvxThesaurusDialog *pDialog)
{
    m_pDialog = pDialog;
}

LookUpComboBox::~LookUpComboBox()
{
}

void LookUpComboBox::Modify()
{
    m_aModifyTimer.Start();
}

IMPL_LINK( LookUpComboBox, ModifyTimer_Hdl, Timer *, EMPTYARG /*pTimer*/ )
{
    m_pDialog->LookUp( GetText() );
    m_aModifyTimer.Stop();
    return 0;
}

// class ReplaceEdit --------------------------------------------------

ReplaceEdit::ReplaceEdit(Window *pParent)
    : Edit(pParent)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeReplaceEdit(Window *pParent, VclBuilder::stringmap &)
{
    return new ReplaceEdit(pParent);
}

ReplaceEdit::~ReplaceEdit()
{
}

void ReplaceEdit::Modify()
{
    if (m_pBtn)
        m_pBtn->Enable( GetText().Len() > 0 );
}

void ReplaceEdit::SetText( const XubString& rStr )
{
    Edit::SetText( rStr );
    Modify();
}

void ReplaceEdit::SetText( const XubString& rStr, const Selection& rNewSelection )
{
    Edit::SetText( rStr, rNewSelection );
    Modify();
}

// class ThesaurusAlternativesCtrl ----------------------------------

AlternativesString::AlternativesString(
    ThesaurusAlternativesCtrl &rControl,
    SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rStr ) :

    SvLBoxString( pEntry, nFlags, rStr ),
    m_rControlImpl( rControl )
{
}

void AlternativesString::Paint(
    const Point& rPos,
    SvLBox& rDev, sal_uInt16,
    SvLBoxEntry* pEntry )
{
    AlternativesExtraData* pData = m_rControlImpl.GetExtraData( pEntry );
    Point aPos( rPos );
    Font aOldFont( rDev.GetFont());
    if (pData && pData->IsHeader())
    {
        Font aFont( aOldFont );
        aFont.SetWeight( WEIGHT_BOLD );
        rDev.SetFont( aFont );
        aPos.X() = 0;
    }
    else
        aPos.X() += 5;
    rDev.DrawText( aPos, GetText() );
    rDev.SetFont( aOldFont );
}

ThesaurusAlternativesCtrl::ThesaurusAlternativesCtrl(Window* pParent)
    : SvxCheckListBox(pParent)
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
    SetHighlightRange();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeThesaurusAlternativesCtrl(Window *pParent, VclBuilder::stringmap &)
{
    return new ThesaurusAlternativesCtrl(pParent);
}

void ThesaurusAlternativesCtrl::init(SvxThesaurusDialog *pDialog)
{
    m_pDialog = pDialog;
}

ThesaurusAlternativesCtrl::~ThesaurusAlternativesCtrl()
{
    ClearExtraData();
}

void ThesaurusAlternativesCtrl::ClearExtraData()
{
    UserDataMap_t   aEmpty;
    m_aUserData.swap( aEmpty );
}

void ThesaurusAlternativesCtrl::SetExtraData(
    const SvLBoxEntry *pEntry,
    const AlternativesExtraData &rData )
{
    if (!pEntry)
        return;

    UserDataMap_t::iterator aIt( m_aUserData.find( pEntry ) );
    if (aIt != m_aUserData.end())
        aIt->second = rData;
    else
        m_aUserData[ pEntry ] = rData;
}

AlternativesExtraData * ThesaurusAlternativesCtrl::GetExtraData(
    const SvLBoxEntry *pEntry )
{
    AlternativesExtraData *pRes = NULL;
    UserDataMap_t::iterator aIt( m_aUserData.find( pEntry ) );
    if (aIt != m_aUserData.end())
        pRes = &aIt->second;
    return pRes;
}

SvLBoxEntry * ThesaurusAlternativesCtrl::AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    String aText;
    if (bIsHeader && nVal >= 0)
    {
        aText = String::CreateFromInt32( nVal );
        aText += rtl::OUString(". ");
    }
    pEntry->AddItem( new SvLBoxString( pEntry, 0, String() ) ); // add empty column
    aText += rText;
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0 ) );  // otherwise crash
    pEntry->AddItem( new AlternativesString( *this, pEntry, 0, aText ) );

    SetExtraData( pEntry, AlternativesExtraData( rText, bIsHeader ) );
    GetModel()->Insert( pEntry );

    if (bIsHeader)
        GetViewDataEntry( pEntry )->SetSelectable( false );

    return pEntry;
}

void ThesaurusAlternativesCtrl::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKey = rKEvt.GetKeyCode();

    if (rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_ESCAPE)
        GetParent()->KeyInput( rKEvt ); // parent will close dialog...
    else if (rKey.GetCode() == KEY_SPACE)
        m_pDialog->AlternativesDoubleClickHdl_Impl( this ); // look up current selected entry
    else if (GetEntryCount())
        SvxCheckListBox::KeyInput( rKEvt );
}

void ThesaurusAlternativesCtrl::Paint( const Rectangle& rRect )
{
    if (!m_pDialog->WordFound())
    {
        Size aTextSize( GetTextWidth( m_pDialog->getErrStr() ), GetTextHeight() );
        aTextSize  = LogicToPixel( aTextSize );
        Point aPos;
        aPos.X() += GetSizePixel().Width() / 2  - aTextSize.Width() / 2;
        aPos.Y() += GetSizePixel().Height() / 2;
        aPos = PixelToLogic( aPos );
        DrawText( aPos, m_pDialog->getErrStr() );

    }
    else
        SvxCheckListBox::Paint( rRect );
}

uno::Sequence< uno::Reference< linguistic2::XMeaning > > SvxThesaurusDialog::queryMeanings_Impl(
        OUString& rTerm,
        const lang::Locale& rLocale,
        const beans::PropertyValues& rProperties )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings(
            xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if ( 0 == aMeanings.getLength() && !rTerm.isEmpty() &&
        rTerm.getStr()[ rTerm.getLength() - 1 ] == '.')
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        String aTxt(comphelper::string::stripEnd(rTerm, '.'));
        aMeanings = xThesaurus->queryMeanings( aTxt, rLocale, rProperties );
        if (aMeanings.getLength())
        {
            rTerm = aTxt;
        }
    }

    return aMeanings;
}

bool SvxThesaurusDialog::UpdateAlternativesBox_Impl()
{
    lang::Locale aLocale( SvxCreateLocale( nLookUpLanguage ) );
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings = queryMeanings_Impl(
            aLookUpText, aLocale, uno::Sequence< beans::PropertyValue >() );
    const sal_Int32 nMeanings = aMeanings.getLength();
    const uno::Reference< linguistic2::XMeaning > *pMeanings = aMeanings.getConstArray();

    m_pAlternativesCT->SetUpdateMode( sal_False );

    // clear old user data of control before creating new ones via AddEntry below
    m_pAlternativesCT->ClearExtraData();

    m_pAlternativesCT->Clear();
    for (sal_Int32 i = 0;  i < nMeanings;  ++i)
    {
        OUString rMeaningTxt = pMeanings[i]->getMeaning();
        uno::Sequence< OUString > aSynonyms( pMeanings[i]->querySynonyms() );
        const sal_Int32 nSynonyms = aSynonyms.getLength();
        const OUString *pSynonyms = aSynonyms.getConstArray();
        DBG_ASSERT( !rMeaningTxt.isEmpty(), "meaning with empty text" );
        DBG_ASSERT( nSynonyms > 0, "meaning without synonym" );

        m_pAlternativesCT->AddEntry( i + 1, rMeaningTxt, true );
        for (sal_Int32 k = 0;  k < nSynonyms;  ++k)
            m_pAlternativesCT->AddEntry( -1, pSynonyms[k], false );
    }

    m_pAlternativesCT->SetUpdateMode( sal_True );

    return nMeanings > 0;
}

void SvxThesaurusDialog::LookUp( const String &rText )
{
    if (rText != m_pWordCB->GetText()) // avoid moving of the cursor if the text is the same
        m_pWordCB->SetText( rText );
    LookUp_Impl();
}

IMPL_LINK( SvxThesaurusDialog, LeftBtnHdl_Impl, Button *, pBtn )
{
    if (pBtn && aLookUpHistory.size() >= 2)
    {
        aLookUpHistory.pop();                       // remove current look up word from stack
        m_pWordCB->SetText( aLookUpHistory.top() );    // retrieve previous look up word
        aLookUpHistory.pop();
        LookUp_Impl();
    }
    return 0;
}

IMPL_LINK( SvxThesaurusDialog, LanguageHdl_Impl, MenuButton*, pBtn )
{
    PopupMenu *pMenu = m_pLangMBtn->GetPopupMenu();
    if (pMenu && pBtn)
    {
        sal_uInt16 nItem = pBtn->GetCurItemId();
        String aLangText( pMenu->GetItemText( nItem ) );
        LanguageType nLang = SvtLanguageTable().GetType( aLangText );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        if (xThesaurus->hasLocale( SvxCreateLocale( nLang ) ))
            nLookUpLanguage = nLang;
        SetWindowTitle( nLang );
        LookUp_Impl();
    }
    return 0;
}

void SvxThesaurusDialog::LookUp_Impl()
{
    String aText( m_pWordCB->GetText() );

    aLookUpText = OUString( aText );
    if (!aLookUpText.isEmpty() &&
            (aLookUpHistory.empty() || aLookUpText != aLookUpHistory.top()))
        aLookUpHistory.push( aLookUpText );

    m_bWordFound = UpdateAlternativesBox_Impl();
    m_pAlternativesCT->Enable( m_bWordFound );

    if ( m_pWordCB->GetEntryPos( aText ) == LISTBOX_ENTRY_NOTFOUND )
        m_pWordCB->InsertEntry( aText );

    m_pReplaceEdit->SetText( String() );
    m_pLeftBtn->Enable( aLookUpHistory.size() > 1 );
}

IMPL_LINK( SvxThesaurusDialog, WordSelectHdl_Impl, ComboBox *, pBox )
{
    if (pBox && !m_pWordCB->IsTravelSelect())  // act only upon return key and not when traveling with cursor keys
    {
        sal_uInt16 nPos = pBox->GetSelectEntryPos();
        String aStr( pBox->GetEntry( nPos ) );
        aStr = linguistic::GetThesaurusReplaceText( aStr );
        m_pWordCB->SetText( aStr );
        LookUp_Impl();
    }

    return 0;
}

IMPL_LINK( SvxThesaurusDialog, AlternativesSelectHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox ? pBox->GetCurEntry() : NULL;
    if (pEntry)
    {
        AlternativesExtraData * pData = m_pAlternativesCT->GetExtraData( pEntry );
        String aStr;
        if (pData && !pData->IsHeader())
        {
            aStr = pData->GetText();
            aStr = linguistic::GetThesaurusReplaceText( aStr );
        }
        m_pReplaceEdit->SetText( aStr );
    }
    return 0;
}

IMPL_LINK( SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox ? pBox->GetCurEntry() : NULL;
    if (pEntry)
    {
        AlternativesExtraData * pData = m_pAlternativesCT->GetExtraData( pEntry );
        String aStr;
        if (pData && !pData->IsHeader())
        {
            aStr = pData->GetText();
            aStr = linguistic::GetThesaurusReplaceText( aStr );
        }

        m_pWordCB->SetText( aStr );
        if (aStr.Len() > 0)
            LookUp_Impl();
    }

    //! workaround to set the selection since calling SelectEntryPos within
    //! the double click handler does not work
    Application::PostUserEvent( STATIC_LINK( this, SvxThesaurusDialog, SelectFirstHdl_Impl ), pBox );
    return 0;
}

IMPL_STATIC_LINK( SvxThesaurusDialog, SelectFirstHdl_Impl, SvxCheckListBox *, pBox )
{
    (void) pThis;
    if (pBox && pBox->GetEntryCount() >= 2)
        pBox->SelectEntryPos( 1 );  // pos 0 is a 'header' that is not selectable
    return 0;
}

// class SvxThesaurusDialog ----------------------------------------------

SvxThesaurusDialog::SvxThesaurusDialog(
    Window* pParent,
    uno::Reference< linguistic2::XThesaurus >  xThes,
    const String &rWord,
    LanguageType nLanguage)
    : SvxStandardDialog(pParent, "ThesaurusDialog", "cui/ui/thesaurus.ui")
    , m_aErrStr(CUI_RESSTR(RID_SVXSTR_ERR_TEXTNOTFOUND))
    , xThesaurus(NULL)
    , aLookUpText()
    , nLookUpLanguage(LANGUAGE_NONE)
    , m_bWordFound(false)
{
    get(m_pLeftBtn, "left");

    get(m_pWordCB, "wordcb");
    m_pWordCB->init(this);

    get(m_pAlternativesCT, "alternatives");
    m_pAlternativesCT->init(this);

    get(m_pReplaceEdit, "replaceed");
    PushButton *pReplaceBtn = get<PushButton>("replace");
    m_pReplaceEdit->init(pReplaceBtn);

    get(m_pLangMBtn, "langcb");

    pReplaceBtn->SetClickHdl( LINK( this, SvxThesaurusDialog, ReplaceBtnHdl_Impl ) );
    m_pLeftBtn->SetClickHdl( LINK( this, SvxThesaurusDialog, LeftBtnHdl_Impl ) );
    m_pWordCB->SetSelectHdl( LINK( this, SvxThesaurusDialog, WordSelectHdl_Impl ) );
    m_pLangMBtn->SetSelectHdl( LINK( this, SvxThesaurusDialog, LanguageHdl_Impl ) );
    m_pAlternativesCT->SetSelectHdl( LINK( this, SvxThesaurusDialog, AlternativesSelectHdl_Impl ));
    m_pAlternativesCT->SetDoubleClickHdl( LINK( this, SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl ));

    xThesaurus = xThes;
    aLookUpText = OUString( rWord );
    nLookUpLanguage = nLanguage;
    if (rWord.Len() > 0)
        aLookUpHistory.push( rWord );

    OUString aTmp( rWord );
    linguistic::RemoveHyphens( aTmp );
    linguistic::ReplaceControlChars( aTmp );
    String aTmp2( aTmp );
    m_pReplaceEdit->SetText( aTmp2 );
    m_pWordCB->InsertEntry( aTmp2 );

    LookUp( aTmp2 );
    m_pAlternativesCT->GrabFocus();
    m_pLeftBtn->Enable( sal_False );

    // fill language menu button list
    SvtLanguageTable aLangTab;
    uno::Sequence< lang::Locale > aLocales;
    if (xThesaurus.is())
        aLocales = xThesaurus->getLocales();
    const sal_Int32 nLocales = aLocales.getLength();
    const lang::Locale *pLocales = aLocales.getConstArray();
    PopupMenu* pMenu = m_pLangMBtn->GetPopupMenu();
    pMenu->Clear();
    pMenu->SetMenuFlags(MENU_FLAG_NOAUTOMNEMONICS);
    std::vector< OUString > aLangVec;
    for (sal_Int32 i = 0;  i < nLocales; ++i)
    {
        const LanguageType nLang = SvxLocaleToLanguage( pLocales[i] );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        aLangVec.push_back( aLangTab.GetString( nLang ) );
    }
    std::sort( aLangVec.begin(), aLangVec.end() );
    for (size_t i = 0;  i < aLangVec.size();  ++i)
        pMenu->InsertItem( (sal_uInt16)i+1, aLangVec[i] );  // menu items should be enumerated from 1 and not 0

    std::vector< OUString >::iterator aI = std::find(aLangVec.begin(), aLangVec.end(), aLangTab.GetString(nLanguage));
    if (aI != aLangVec.end())
    {
        pMenu->SetSelectedEntry(std::distance(aLangVec.begin(), aI) + 1);
        pMenu->Select();
    }

    SetWindowTitle(nLanguage);

    // disable controls if service is missing
    if (!xThesaurus.is())
        Enable( sal_False );
}

IMPL_LINK( SvxThesaurusDialog, ReplaceBtnHdl_Impl, Button *, EMPTYARG /*pBtn*/ )
{
    EndDialog(true);
    return 0;
}

SvxThesaurusDialog::~SvxThesaurusDialog()
{
}

void SvxThesaurusDialog::SetWindowTitle( LanguageType nLanguage )
{
    // adjust language
    String aStr( GetText() );
    aStr.Erase( aStr.Search( sal_Unicode( '(' ) ) - 1 );
    aStr.Append( rtl::OUString(" (") );
    aStr += SvtLanguageTable().GetLanguageString( nLanguage );
    aStr.Append( sal_Unicode( ')' ) );
    SetText( aStr );    // set window title
}

String SvxThesaurusDialog::GetWord()
{
    return m_pReplaceEdit->GetText();
}

sal_uInt16 SvxThesaurusDialog::GetLanguage() const
{
    return nLookUpLanguage;
}

void SvxThesaurusDialog::Apply()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
