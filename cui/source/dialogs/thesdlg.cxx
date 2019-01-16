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

#include <thesdlg.hxx>
#include "thesdlg_impl.hxx"
#include <strings.hrc>
#include <dialmgr.hxx>

#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svxerr.hxx>
#include <editeng/unolingu.hxx>
#include <svx/langbox.hxx>
#include <svtools/langtab.hxx>
#include <unotools/lingucfg.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <comphelper/string.hxx>
#include <osl/file.hxx>

#include <stack>
#include <algorithm>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

using namespace ::com::sun::star;

// class LookUpComboBox --------------------------------------------------

LookUpComboBox::LookUpComboBox(vcl::Window *pParent)
    : ComboBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK|WB_TABSTOP)
    , m_aModifyIdle("cui LookUpComboBox Modify")
    , m_pDialog(nullptr)
{
    EnableAutoSize(true);

    m_aModifyIdle.SetInvokeHandler( LINK( this, LookUpComboBox, ModifyTimer_Hdl ) );
    m_aModifyIdle.SetPriority( TaskPriority::LOWEST );

    EnableAutocomplete( false );
}

LookUpComboBox::~LookUpComboBox()
{
    disposeOnce();
}

void LookUpComboBox::dispose()
{
    m_pDialog.clear();
    ComboBox::dispose();
}

VCL_BUILDER_FACTORY(LookUpComboBox)

void LookUpComboBox::init(SvxThesaurusDialog *pDialog)
{
    m_pDialog = pDialog;
}

void LookUpComboBox::Modify()
{
    m_aModifyIdle.Start();
}

IMPL_LINK_NOARG( LookUpComboBox, ModifyTimer_Hdl, Timer *, void )
{
    m_pDialog->LookUp( GetText() );
    m_aModifyIdle.Stop();
}

// class ReplaceEdit --------------------------------------------------

ReplaceEdit::ReplaceEdit(vcl::Window *pParent)
    : Edit(pParent, WB_BORDER | WB_TABSTOP)
    , m_pBtn(nullptr)
{
}

ReplaceEdit::~ReplaceEdit()
{
    disposeOnce();
}

void ReplaceEdit::dispose()
{
    m_pBtn.clear();
    Edit::dispose();
}

VCL_BUILDER_FACTORY(ReplaceEdit)

void ReplaceEdit::Modify()
{
    if (m_pBtn)
        m_pBtn->Enable( !GetText().isEmpty() );
}

void ReplaceEdit::SetText( const OUString& rStr )
{
    Edit::SetText( rStr );
    Modify();
}

void ReplaceEdit::SetText( const OUString& rStr, const Selection& rNewSelection )
{
    Edit::SetText( rStr, rNewSelection );
    Modify();
}

// class ThesaurusAlternativesCtrl ----------------------------------

AlternativesString::AlternativesString( ThesaurusAlternativesCtrl &rControl, const OUString& rStr ) :
    SvLBoxString( rStr ),
    m_rControlImpl( rControl )
{
}

void AlternativesString::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                               const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    AlternativesExtraData* pData = m_rControlImpl.GetExtraData(&rEntry);
    Point aPos(rPos);
    rRenderContext.Push(PushFlags::FONT);
    if (pData && pData->IsHeader())
    {
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        rRenderContext.SetFont(aFont);
        aPos.setX( 0 );
    }
    else
        aPos.AdjustX(5 );
    rRenderContext.DrawText(aPos, GetText());
    rRenderContext.Pop();
}

ThesaurusAlternativesCtrl::ThesaurusAlternativesCtrl(vcl::Window* pParent)
    : SvxCheckListBox(pParent)
    , m_pDialog(nullptr)
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL );
    SetForceMakeVisible(true);
    SetHighlightRange();
}

VCL_BUILDER_FACTORY(ThesaurusAlternativesCtrl)

void ThesaurusAlternativesCtrl::init(SvxThesaurusDialog *pDialog)
{
    m_pDialog = pDialog;
}

ThesaurusAlternativesCtrl::~ThesaurusAlternativesCtrl()
{
    disposeOnce();
}

void ThesaurusAlternativesCtrl::dispose()
{
    ClearExtraData();
    m_pDialog.clear();
    SvxCheckListBox::dispose();
}

void ThesaurusAlternativesCtrl::ClearExtraData()
{
    UserDataMap_t   aEmpty;
    m_aUserData.swap( aEmpty );
}

void ThesaurusAlternativesCtrl::SetExtraData(
    const SvTreeListEntry *pEntry,
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
    const SvTreeListEntry *pEntry )
{
    AlternativesExtraData *pRes = nullptr;
    UserDataMap_t::iterator aIt( m_aUserData.find( pEntry ) );
    if (aIt != m_aUserData.end())
        pRes = &aIt->second;
    return pRes;
}

SvTreeListEntry * ThesaurusAlternativesCtrl::AddEntry( sal_Int32 nVal, const OUString &rText, bool bIsHeader )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    OUString aText;
    if (bIsHeader && nVal >= 0)
    {
        aText = OUString::number( nVal ) + ". ";
    }
    pEntry->AddItem(std::make_unique<SvLBoxString>(OUString())); // add empty column
    aText += rText;
    pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(Image(), Image(), false)); // otherwise crash
    pEntry->AddItem(std::make_unique<AlternativesString>(*this, aText));

    SetExtraData( pEntry, AlternativesExtraData( rText, bIsHeader ) );
    GetModel()->Insert( pEntry );

    if (bIsHeader)
        GetViewDataEntry( pEntry )->SetSelectable( false );

    return pEntry;
}

void ThesaurusAlternativesCtrl::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKey = rKEvt.GetKeyCode();

    if (rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_ESCAPE)
        GetParent()->KeyInput( rKEvt ); // parent will close dialog...
    else if (rKey.GetCode() == KEY_SPACE)
        m_pDialog->AlternativesDoubleClickHdl_Impl( this ); // look up current selected entry
    else if (GetEntryCount())
        SvxCheckListBox::KeyInput( rKEvt );
}

void ThesaurusAlternativesCtrl::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect)
{
    if (!m_pDialog->WordFound())
    {
        Size aTextSize(rRenderContext.GetTextWidth(m_pDialog->getErrStr()), rRenderContext.GetTextHeight());
        aTextSize  = rRenderContext.LogicToPixel(aTextSize);
        Point aPos;
        aPos.AdjustX(GetSizePixel().Width() / 2  - aTextSize.Width() / 2 );
        aPos.AdjustY(GetSizePixel().Height() / 2 );
        aPos = rRenderContext.PixelToLogic(aPos);
        rRenderContext.DrawText(aPos, m_pDialog->getErrStr());
    }
    else
        SvxCheckListBox::Paint(rRenderContext, rRect);
}

uno::Sequence< uno::Reference< linguistic2::XMeaning > > SvxThesaurusDialog::queryMeanings_Impl(
        OUString& rTerm,
        const lang::Locale& rLocale,
        const beans::PropertyValues& rProperties )
{
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings(
            xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if ( 0 == aMeanings.getLength() && rTerm.endsWith(".") )
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        OUString aTxt(comphelper::string::stripEnd(rTerm, '.'));
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
    lang::Locale aLocale( LanguageTag::convertToLocale( nLookUpLanguage ) );
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings = queryMeanings_Impl(
            aLookUpText, aLocale, uno::Sequence< beans::PropertyValue >() );
    const sal_Int32 nMeanings = aMeanings.getLength();
    const uno::Reference< linguistic2::XMeaning > *pMeanings = aMeanings.getConstArray();

    m_pAlternativesCT->SetUpdateMode( false );

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

    m_pAlternativesCT->SetUpdateMode( true );

    return nMeanings > 0;
}

void SvxThesaurusDialog::LookUp( const OUString &rText )
{
    if (rText != m_pWordCB->GetText()) // avoid moving of the cursor if the text is the same
        m_pWordCB->SetText( rText );
    LookUp_Impl();
}

IMPL_LINK( SvxThesaurusDialog, LeftBtnHdl_Impl, Button *, pBtn, void )
{
    if (pBtn && aLookUpHistory.size() >= 2)
    {
        aLookUpHistory.pop();                       // remove current look up word from stack
        m_pWordCB->SetText( aLookUpHistory.top() );    // retrieve previous look up word
        aLookUpHistory.pop();
        LookUp_Impl();
    }
}

IMPL_LINK( SvxThesaurusDialog, LanguageHdl_Impl, ListBox&, rLB, void )
{
    OUString aLangText( rLB.GetSelectedEntry() );
    LanguageType nLang = SvtLanguageTable::GetLanguageType( aLangText );
    DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
    if (xThesaurus->hasLocale( LanguageTag::convertToLocale( nLang ) ))
        nLookUpLanguage = nLang;
    SetWindowTitle( nLang );
    LookUp_Impl();
}

void SvxThesaurusDialog::LookUp_Impl()
{
    OUString aText( m_pWordCB->GetText() );

    aLookUpText = aText;
    if (!aLookUpText.isEmpty() &&
            (aLookUpHistory.empty() || aLookUpText != aLookUpHistory.top()))
        aLookUpHistory.push( aLookUpText );

    m_bWordFound = UpdateAlternativesBox_Impl();
    m_pAlternativesCT->Enable( m_bWordFound );

    if ( m_pWordCB->GetEntryPos( aText ) == LISTBOX_ENTRY_NOTFOUND )
        m_pWordCB->InsertEntry( aText );

    m_pReplaceEdit->SetText( OUString() );
    m_pLeftBtn->Enable( aLookUpHistory.size() > 1 );
}

IMPL_LINK( SvxThesaurusDialog, WordSelectHdl_Impl, ComboBox&, rBox, void )
{
    if (!m_pWordCB->IsTravelSelect())  // act only upon return key and not when traveling with cursor keys
    {
        const sal_Int32 nPos = rBox.GetSelectedEntryPos();
        OUString aStr( rBox.GetEntry( nPos ) );
        aStr = linguistic::GetThesaurusReplaceText( aStr );
        m_pWordCB->SetText( aStr );
        LookUp_Impl();
    }
}

IMPL_LINK( SvxThesaurusDialog, AlternativesSelectHdl_Impl, SvTreeListBox *, pBox, void )
{
    SvTreeListEntry *pEntry = pBox ? pBox->GetCurEntry() : nullptr;
    if (pEntry)
    {
        AlternativesExtraData * pData = m_pAlternativesCT->GetExtraData( pEntry );
        OUString aStr;
        if (pData && !pData->IsHeader())
        {
            aStr = pData->GetText();
            aStr = linguistic::GetThesaurusReplaceText( aStr );
        }
        m_pReplaceEdit->SetText( aStr );
    }
}

IMPL_LINK( SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl, SvTreeListBox*, pBox, bool )
{
    SvTreeListEntry *pEntry = pBox ? pBox->GetCurEntry() : nullptr;
    if (pEntry)
    {
        AlternativesExtraData * pData = m_pAlternativesCT->GetExtraData( pEntry );
        OUString aStr;
        if (pData && !pData->IsHeader())
        {
            aStr = pData->GetText();
            aStr = linguistic::GetThesaurusReplaceText( aStr );
        }

        m_pWordCB->SetText( aStr );
        if (!aStr.isEmpty())
            LookUp_Impl();
    }

    //! workaround to set the selection since calling SelectEntryPos within
    //! the double click handler does not work
    Application::PostUserEvent( LINK( this, SvxThesaurusDialog, SelectFirstHdl_Impl ), pBox, true );
    return false;
}

IMPL_STATIC_LINK( SvxThesaurusDialog, SelectFirstHdl_Impl, void *, p, void )
{
    SvxCheckListBox* pBox = static_cast<SvxCheckListBox*>(p);
    if (pBox && pBox->GetEntryCount() >= 2)
        pBox->SelectEntryPos( 1 );  // pos 0 is a 'header' that is not selectable
}

// class SvxThesaurusDialog ----------------------------------------------

SvxThesaurusDialog::SvxThesaurusDialog(
    vcl::Window* pParent,
    uno::Reference< linguistic2::XThesaurus > const & xThes,
    const OUString &rWord,
    LanguageType nLanguage)
    : SvxStandardDialog(pParent, "ThesaurusDialog", "cui/ui/thesaurus.ui")
    , m_aErrStr(CuiResId(RID_SVXSTR_ERR_TEXTNOTFOUND))
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

    get(m_pLangLB, "langcb");

    pReplaceBtn->SetClickHdl( LINK( this, SvxThesaurusDialog, ReplaceBtnHdl_Impl ) );
    m_pLeftBtn->SetClickHdl( LINK( this, SvxThesaurusDialog, LeftBtnHdl_Impl ) );
    m_pWordCB->SetSelectHdl( LINK( this, SvxThesaurusDialog, WordSelectHdl_Impl ) );
    m_pLangLB->SetSelectHdl( LINK( this, SvxThesaurusDialog, LanguageHdl_Impl ) );
    m_pAlternativesCT->SetSelectHdl( LINK( this, SvxThesaurusDialog, AlternativesSelectHdl_Impl ));
    m_pAlternativesCT->SetDoubleClickHdl( LINK( this, SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl ));

    xThesaurus = xThes;
    aLookUpText = rWord;
    nLookUpLanguage = nLanguage;
    if (!rWord.isEmpty())
        aLookUpHistory.push( rWord );

    OUString aTmp( rWord );
    (void)linguistic::RemoveHyphens( aTmp );
    (void)linguistic::ReplaceControlChars( aTmp );
    m_pReplaceEdit->SetText( aTmp );
    m_pWordCB->InsertEntry( aTmp );

    LookUp( aTmp );
    m_pAlternativesCT->GrabFocus();
    m_pLeftBtn->Enable( false );

    // fill language menu button list
    uno::Sequence< lang::Locale > aLocales;
    if (xThesaurus.is())
        aLocales = xThesaurus->getLocales();
    const sal_Int32 nLocales = aLocales.getLength();
    const lang::Locale *pLocales = aLocales.getConstArray();
    m_pLangLB->Clear();
    std::vector< OUString > aLangVec;
    for (sal_Int32 i = 0;  i < nLocales; ++i)
    {
        const LanguageType nLang = LanguageTag::convertToLanguageType( pLocales[i] );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        aLangVec.push_back( SvtLanguageTable::GetLanguageString( nLang ) );
    }
    std::sort( aLangVec.begin(), aLangVec.end() );
    for (OUString & i : aLangVec)
        m_pLangLB->InsertEntry( i );

    std::vector< OUString >::iterator aI = std::find(aLangVec.begin(), aLangVec.end(),
            SvtLanguageTable::GetLanguageString(nLanguage));
    if (aI != aLangVec.end())
    {
        m_pLangLB->SelectEntry(*aI);
    }

    SetWindowTitle(nLanguage);

    // disable controls if service is missing
    if (!xThesaurus.is())
        Enable( false );
}

SvxThesaurusDialog::~SvxThesaurusDialog()
{
    disposeOnce();
}

void SvxThesaurusDialog::dispose()
{
    m_pLeftBtn.clear();
    m_pWordCB.clear();
    m_pAlternativesCT.clear();
    m_pReplaceEdit.clear();
    m_pLangLB.clear();
    SvxStandardDialog::dispose();
}

IMPL_LINK_NOARG( SvxThesaurusDialog, ReplaceBtnHdl_Impl, Button *, void )
{
    EndDialog(RET_OK);
}

void SvxThesaurusDialog::SetWindowTitle( LanguageType nLanguage )
{
    // adjust language
    OUString aStr( GetText() );
    sal_Int32 nIndex = aStr.indexOf( '(' );
    if( nIndex != -1 )
        aStr = aStr.copy( 0, nIndex - 1 );
    aStr += " (" + SvtLanguageTable::GetLanguageString( nLanguage ) + ")";
    SetText( aStr );    // set window title
}

OUString SvxThesaurusDialog::GetWord()
{
    return m_pReplaceEdit->GetText();
}


void SvxThesaurusDialog::Apply()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
