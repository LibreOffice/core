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

#include "optbasic.hxx"
#include <basic/codecompletecache.hxx>
#include <officecfg/Office/BasicIDE.hxx>

SvxBasicIDEOptionsPage::SvxBasicIDEOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optbasicidepage.ui"_ustr, u"OptBasicIDEPage"_ustr, &rSet)
    , m_xCodeCompleteChk(m_xBuilder->weld_check_button(u"codecomplete_enable"_ustr))
    , m_xAutocloseProcChk(m_xBuilder->weld_check_button(u"autoclose_proc"_ustr))
    , m_xAutocloseParenChk(m_xBuilder->weld_check_button(u"autoclose_paren"_ustr))
    , m_xAutocloseQuotesChk(m_xBuilder->weld_check_button(u"autoclose_quotes"_ustr))
    , m_xAutoCorrectChk(m_xBuilder->weld_check_button(u"autocorrect"_ustr))
    , m_xUseExtendedTypesChk(m_xBuilder->weld_check_button(u"extendedtypes_enable"_ustr))
{
    LoadConfig();
}

SvxBasicIDEOptionsPage::~SvxBasicIDEOptionsPage()
{
}

void SvxBasicIDEOptionsPage::LoadConfig()
{
    m_xCodeCompleteChk->set_active( officecfg::Office::BasicIDE::Autocomplete::CodeComplete::get() );
    m_xCodeCompleteChk->set_sensitive( !officecfg::Office::BasicIDE::Autocomplete::CodeComplete::isReadOnly() );
    m_xAutocloseProcChk->set_active( officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::get() );
    m_xAutocloseProcChk->set_sensitive( !officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::isReadOnly() );
    m_xAutocloseQuotesChk->set_active( officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::get() );
    m_xAutocloseQuotesChk->set_sensitive( !officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::isReadOnly() );
    m_xAutocloseParenChk->set_active( officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::get() );
    m_xAutocloseParenChk->set_sensitive( !officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::isReadOnly() );
    m_xAutoCorrectChk->set_active( officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::get() );
    m_xAutoCorrectChk->set_sensitive( !officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::isReadOnly() );
    m_xUseExtendedTypesChk->set_active( officecfg::Office::BasicIDE::Autocomplete::UseExtended::get() );
    m_xUseExtendedTypesChk->set_sensitive( !officecfg::Office::BasicIDE::Autocomplete::UseExtended::isReadOnly() );
}

OUString SvxBasicIDEOptionsPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label3"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"codecomplete_enable"_ustr, u"autocorrect"_ustr,    u"autoclose_quotes"_ustr,
                               u"autoclose_paren"_ustr,     u"autoclose_proc"_ustr, u"extendedtypes_enable"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxBasicIDEOptionsPage::FillItemSet( SfxItemSet* /*rCoreSet*/ )
{
    bool bModified = false;
    std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );

    if( m_xAutocloseProcChk->get_state_changed_from_saved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::set( m_xAutocloseProcChk->get_active(), batch );
        CodeCompleteOptions::SetProcedureAutoCompleteOn( m_xAutocloseProcChk->get_active() );
        bModified = true;
    }

    if( m_xCodeCompleteChk->get_state_changed_from_saved() )
    {
        //std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::CodeComplete::set( m_xCodeCompleteChk->get_active(), batch );
        CodeCompleteOptions::SetCodeCompleteOn( m_xCodeCompleteChk->get_active() );
        bModified = true;
    }

    if( m_xUseExtendedTypesChk->get_state_changed_from_saved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::UseExtended::set( m_xUseExtendedTypesChk->get_active(), batch );
        CodeCompleteOptions::SetExtendedTypeDeclaration( m_xUseExtendedTypesChk->get_active() );
        bModified = true;
    }

    if( m_xAutocloseParenChk->get_state_changed_from_saved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::set( m_xAutocloseParenChk->get_active(), batch );
        CodeCompleteOptions::SetAutoCloseParenthesisOn( m_xAutocloseParenChk->get_active() );
        bModified = true;
    }

    if( m_xAutocloseQuotesChk->get_state_changed_from_saved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::set( m_xAutocloseQuotesChk->get_active(), batch );
        CodeCompleteOptions::SetAutoCloseQuotesOn( m_xAutocloseQuotesChk->get_active() );
        bModified = true;
    }

    if( m_xAutoCorrectChk->get_state_changed_from_saved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::set( m_xAutoCorrectChk->get_active(), batch );
        CodeCompleteOptions::SetAutoCorrectOn( m_xAutoCorrectChk->get_active() );
        bModified = true;
    }

    if( bModified )
        batch->commit();

    return bModified;
}

void SvxBasicIDEOptionsPage::Reset( const SfxItemSet* /*rSet*/ )
{
    LoadConfig();
    m_xCodeCompleteChk->save_state();
    m_xAutocloseProcChk->save_state();
    m_xAutocloseQuotesChk->save_state();
    m_xAutocloseParenChk->save_state();
    m_xAutoCorrectChk->save_state();
    m_xUseExtendedTypesChk->save_state();
}

std::unique_ptr<SfxTabPage> SvxBasicIDEOptionsPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxBasicIDEOptionsPage>(pPage, pController, *rAttrSet);
}

void SvxBasicIDEOptionsPage::FillUserData()
{
    SetUserData( OUString() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
