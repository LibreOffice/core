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

#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <svtools/langtab.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/string.hxx>

#include <stack>
#include <algorithm>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

using namespace ::com::sun::star;

IMPL_LINK_NOARG( SvxThesaurusDialog, ModifyTimer_Hdl, Timer *, void )
{
    LookUp(m_xWordCB->get_active_text());
    m_aModifyIdle.Stop();
}

IMPL_LINK_NOARG(SvxThesaurusDialog, ReplaceEditHdl_Impl, weld::Entry&, void)
{
    m_xReplaceBtn->set_sensitive(!m_xReplaceEdit->get_text().isEmpty());
}

IMPL_LINK(SvxThesaurusDialog, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKey = rKEvt.GetKeyCode();

    if (rKey.GetCode() == KEY_RETURN)
    {
        m_xDialog->response(RET_OK);
        return true;
    }

    return false;
}

uno::Sequence< uno::Reference< linguistic2::XMeaning > > SvxThesaurusDialog::queryMeanings_Impl(
        OUString& rTerm,
        const lang::Locale& rLocale,
        const beans::PropertyValues& rProperties )
{
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings(
            xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if ( !aMeanings.hasElements() && rTerm.endsWith(".") )
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        OUString aTxt(comphelper::string::stripEnd(rTerm, '.'));
        aMeanings = xThesaurus->queryMeanings( aTxt, rLocale, rProperties );
        if (aMeanings.hasElements())
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

    m_xAlternativesCT->freeze();

    m_xAlternativesCT->clear();
    int nRow = 0;
    for (sal_Int32 i = 0;  i < nMeanings;  ++i)
    {
        OUString rMeaningTxt = aMeanings[i]->getMeaning();
        uno::Sequence<OUString> aSynonyms(aMeanings[i]->querySynonyms());
        DBG_ASSERT( !rMeaningTxt.isEmpty(), "meaning with empty text" );
        DBG_ASSERT(aSynonyms.hasElements(), "meaning without synonym");

        OUString sHeading = OUString::number(i + 1) + ". " + rMeaningTxt;
        m_xAlternativesCT->append_text(sHeading);
        m_xAlternativesCT->set_text_emphasis(nRow, true, 0);
        ++nRow;

        for (auto& synonym : aSynonyms)
        {
            // GetThesaurusReplaceText will strip the leading spaces
            m_xAlternativesCT->append_text("   " + synonym);
            m_xAlternativesCT->set_text_emphasis(nRow, false, 0);
            ++nRow;
        }
    }

    m_xAlternativesCT->thaw();

    return nMeanings > 0;
}

void SvxThesaurusDialog::LookUp( const OUString &rText )
{
    if (rText != m_xWordCB->get_active_text()) // avoid moving of the cursor if the text is the same
        m_xWordCB->set_entry_text(rText);
    LookUp_Impl();
}

IMPL_LINK_NOARG(SvxThesaurusDialog, LeftBtnHdl_Impl, weld::Button&, void)
{
    if (aLookUpHistory.size() >= 2)
    {
        aLookUpHistory.pop();                       // remove current look up word from stack
        m_xWordCB->set_entry_text(aLookUpHistory.top()); // retrieve previous look up word
        aLookUpHistory.pop();
        LookUp_Impl();
    }
}

IMPL_LINK( SvxThesaurusDialog, LanguageHdl_Impl, weld::ComboBox&, rLB, void )
{
    OUString aLangText(rLB.get_active_text());
    LanguageType nLang = SvtLanguageTable::GetLanguageType( aLangText );
    DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
    if (xThesaurus->hasLocale( LanguageTag::convertToLocale( nLang ) ))
        nLookUpLanguage = nLang;
    SetWindowTitle( nLang );
    LookUp_Impl();
}

void SvxThesaurusDialog::LookUp_Impl()
{
    OUString aText(m_xWordCB->get_active_text());

    aLookUpText = aText;
    if (!aLookUpText.isEmpty() &&
            (aLookUpHistory.empty() || aLookUpText != aLookUpHistory.top()))
        aLookUpHistory.push( aLookUpText );

    m_bWordFound = UpdateAlternativesBox_Impl();
    m_xAlternativesCT->set_visible(m_bWordFound);
    m_xNotFound->set_visible(!m_bWordFound);

    if (m_bWordFound && !m_nSelectFirstEvent)
        m_nSelectFirstEvent = Application::PostUserEvent(LINK(this, SvxThesaurusDialog, SelectFirstHdl_Impl));

    if (m_xWordCB->find_text(aText) == -1)
        m_xWordCB->append_text(aText);

    m_xReplaceEdit->set_text( OUString() );
    ReplaceEditHdl_Impl(*m_xReplaceEdit);
    m_xLeftBtn->set_sensitive( aLookUpHistory.size() > 1 );
}

IMPL_LINK_NOARG(SvxThesaurusDialog, WordSelectHdl_Impl, weld::ComboBox&, void)
{
    m_aModifyIdle.Start();
}

IMPL_LINK( SvxThesaurusDialog, AlternativesSelectHdl_Impl, weld::TreeView&, rBox, void )
{
    int nEntry = rBox.get_selected_index();
    if (nEntry != -1)
    {
        bool bIsHeader = rBox.get_text_emphasis(nEntry, 0);
        if (bIsHeader)
        {
            ++nEntry;
            rBox.select(nEntry);
        }
        OUString aStr = linguistic::GetThesaurusReplaceText(rBox.get_text(nEntry));
        m_xReplaceEdit->set_text(aStr);
        ReplaceEditHdl_Impl(*m_xReplaceEdit);
    }
}

IMPL_LINK( SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl, weld::TreeView&, rBox, bool )
{
    int nEntry = rBox.get_selected_index();
    if (nEntry != -1)
    {
        bool bIsHeader = rBox.get_text_emphasis(nEntry, 0);
        if (bIsHeader)
        {
            ++nEntry;
            rBox.select(nEntry);
        }
        OUString aStr = linguistic::GetThesaurusReplaceText(rBox.get_text(nEntry));
        m_xWordCB->set_entry_text(aStr);
        if (!aStr.isEmpty())
            LookUp_Impl();
    }

    //! workaround to set the selection since calling SelectEntryPos within
    //! the double click handler does not work
    if (!m_nSelectFirstEvent)
        m_nSelectFirstEvent = Application::PostUserEvent(LINK(this, SvxThesaurusDialog, SelectFirstHdl_Impl));

    return true;
}

IMPL_LINK_NOARG(SvxThesaurusDialog, SelectFirstHdl_Impl, void *, void)
{
    m_nSelectFirstEvent = nullptr;
    if (m_xAlternativesCT->n_children() >= 2)
    {
        m_xAlternativesCT->select(1);  // pos 0 is a 'header' that is not selectable
        AlternativesSelectHdl_Impl(*m_xAlternativesCT);
    }
}

// class SvxThesaurusDialog ----------------------------------------------

SvxThesaurusDialog::SvxThesaurusDialog(
    weld::Widget* pParent,
    uno::Reference< linguistic2::XThesaurus > const & xThes,
    const OUString &rWord,
    LanguageType nLanguage)
    : SfxDialogController(pParent, u"cui/ui/thesaurus.ui"_ustr, u"ThesaurusDialog"_ustr)
    , m_aModifyIdle("cui SvxThesaurusDialog LookUp Modify")
    , nLookUpLanguage(LANGUAGE_NONE)
    , m_bWordFound(false)
    , m_xLeftBtn(m_xBuilder->weld_button(u"left"_ustr))
    , m_xWordCB(m_xBuilder->weld_combo_box(u"wordcb"_ustr))
    , m_xAlternativesCT(m_xBuilder->weld_tree_view(u"alternatives"_ustr))
    , m_xNotFound(m_xBuilder->weld_label(u"notfound"_ustr))
    , m_xReplaceEdit(m_xBuilder->weld_entry(u"replaceed"_ustr))
    , m_xLangLB(m_xBuilder->weld_combo_box(u"langcb"_ustr))
    , m_xReplaceBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_nSelectFirstEvent(nullptr)
{
    m_aModifyIdle.SetInvokeHandler( LINK( this, SvxThesaurusDialog, ModifyTimer_Hdl ) );
    m_aModifyIdle.SetPriority( TaskPriority::LOWEST );

    m_xReplaceEdit->connect_changed( LINK( this, SvxThesaurusDialog, ReplaceEditHdl_Impl ) );
    m_xReplaceBtn->connect_clicked( LINK( this, SvxThesaurusDialog, ReplaceBtnHdl_Impl ) );
    m_xLeftBtn->connect_clicked( LINK( this, SvxThesaurusDialog, LeftBtnHdl_Impl ) );
    m_xWordCB->set_entry_completion(false);
    m_xWordCB->connect_changed( LINK( this, SvxThesaurusDialog, WordSelectHdl_Impl ) );
    m_xLangLB->connect_changed( LINK( this, SvxThesaurusDialog, LanguageHdl_Impl ) );
    m_xAlternativesCT->connect_changed( LINK( this, SvxThesaurusDialog, AlternativesSelectHdl_Impl ));
    m_xAlternativesCT->connect_row_activated( LINK( this, SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl ));
    m_xAlternativesCT->connect_key_press(LINK(this, SvxThesaurusDialog, KeyInputHdl));

    xThesaurus = xThes;
    aLookUpText = rWord;
    nLookUpLanguage = nLanguage;
    if (!rWord.isEmpty())
        aLookUpHistory.push( rWord );

    OUString aTmp( rWord );
    (void)linguistic::RemoveHyphens( aTmp );
    (void)linguistic::ReplaceControlChars( aTmp );
    m_xReplaceEdit->set_text( aTmp );
    ReplaceEditHdl_Impl(*m_xReplaceEdit);
    m_xWordCB->append_text( aTmp );

    LookUp( aTmp );
    m_xAlternativesCT->grab_focus();
    m_xLeftBtn->set_sensitive(false);

    // fill language menu button list
    uno::Sequence< lang::Locale > aLocales;
    if (xThesaurus.is())
        aLocales = xThesaurus->getLocales();
    m_xLangLB->clear();
    std::vector< OUString > aLangVec;
    for (auto& locale : aLocales)
    {
        const LanguageType nLang = LanguageTag::convertToLanguageType(locale);
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        aLangVec.push_back( SvtLanguageTable::GetLanguageString( nLang ) );
    }
    std::sort( aLangVec.begin(), aLangVec.end() );
    m_xLangLB->freeze();
    for (const OUString & i : aLangVec)
        m_xLangLB->append_text(i);
    m_xLangLB->thaw();

    std::vector< OUString >::iterator aI = std::find(aLangVec.begin(), aLangVec.end(),
            SvtLanguageTable::GetLanguageString(nLanguage));
    if (aI != aLangVec.end())
    {
        m_xLangLB->set_active_text(*aI);
    }

    SetWindowTitle(nLanguage);

    // disable controls if service is missing
    if (!xThesaurus.is())
        m_xDialog->set_sensitive(false);
    else
        m_xWordCB->grab_focus();
}

SvxThesaurusDialog::~SvxThesaurusDialog()
{
    if (m_nSelectFirstEvent)
    {
        Application::RemoveUserEvent(m_nSelectFirstEvent);
        m_nSelectFirstEvent = nullptr;
    }
}

IMPL_LINK_NOARG(SvxThesaurusDialog, ReplaceBtnHdl_Impl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

void SvxThesaurusDialog::SetWindowTitle( LanguageType nLanguage )
{
    // adjust language
    OUString aStr(m_xDialog->get_title());
    sal_Int32 nIndex = aStr.indexOf( '(' );
    if( nIndex != -1 )
        aStr = aStr.copy( 0, nIndex - 1 );
    aStr += " (" + SvtLanguageTable::GetLanguageString( nLanguage ) + ")";
    m_xDialog->set_title(aStr);    // set window title
}

OUString SvxThesaurusDialog::GetWord() const
{
    return m_xReplaceEdit->get_text();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
