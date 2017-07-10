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
#include <iostream>
#include <officecfg/Office/BasicIDE.hxx>
#include <cuires.hrc>

SvxBasicIDEOptionsPage::SvxBasicIDEOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet )
: SfxTabPage(pParent, "OptBasicIDEPage", "cui/ui/optbasicidepage.ui", &rSet)
{
    get(pCodeCompleteChk, "codecomplete_enable");
    get(pAutocloseProcChk, "autoclose_proc");
    get(pAutocloseParenChk, "autoclose_paren");
    get(pAutocloseQuotesChk, "autoclose_quotes");
    get(pAutoCorrectChk, "autocorrect");
    get(pUseExtendedTypesChk, "extendedtypes_enable");

    LoadConfig();
}

SvxBasicIDEOptionsPage::~SvxBasicIDEOptionsPage()
{
    disposeOnce();
}

void SvxBasicIDEOptionsPage::dispose()
{
    pCodeCompleteChk.clear();
    pAutocloseProcChk.clear();
    pAutocloseParenChk.clear();
    pAutocloseQuotesChk.clear();
    pAutoCorrectChk.clear();
    pUseExtendedTypesChk.clear();
    SfxTabPage::dispose();
}

void SvxBasicIDEOptionsPage::LoadConfig()
{
    pCodeCompleteChk->Check( officecfg::Office::BasicIDE::Autocomplete::CodeComplete::get() );
    pCodeCompleteChk->Enable( !officecfg::Office::BasicIDE::Autocomplete::CodeComplete::isReadOnly() );
    pAutocloseProcChk->Check( officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::get() );
    pAutocloseProcChk->Enable( !officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::isReadOnly() );
    pAutocloseQuotesChk->Check( officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::get() );
    pAutocloseQuotesChk->Enable( !officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::isReadOnly() );
    pAutocloseParenChk->Check( officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::get() );
    pAutocloseParenChk->Enable( !officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::isReadOnly() );
    pAutoCorrectChk->Check( officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::get() );
    pAutoCorrectChk->Enable( !officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::isReadOnly() );
    pUseExtendedTypesChk->Check( officecfg::Office::BasicIDE::Autocomplete::UseExtended::get() );
    pUseExtendedTypesChk->Enable( !officecfg::Office::BasicIDE::Autocomplete::UseExtended::isReadOnly() );
}

bool SvxBasicIDEOptionsPage::FillItemSet( SfxItemSet* /*rCoreSet*/ )
{
    bool bModified = false;
    std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );

    if( pAutocloseProcChk->IsValueChangedFromSaved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::set( pAutocloseProcChk->IsChecked(), batch );
        CodeCompleteOptions::SetProcedureAutoCompleteOn( pAutocloseProcChk->IsChecked() );
        bModified = true;
    }

    if( pCodeCompleteChk->IsValueChangedFromSaved() )
    {
        //std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::CodeComplete::set( pCodeCompleteChk->IsChecked(), batch );
        CodeCompleteOptions::SetCodeCompleteOn( pCodeCompleteChk->IsChecked() );
        bModified = true;
    }

    if( pUseExtendedTypesChk->IsValueChangedFromSaved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::UseExtended::set( pUseExtendedTypesChk->IsChecked(), batch );
        CodeCompleteOptions::SetExtendedTypeDeclaration( pUseExtendedTypesChk->IsChecked() );
        bModified = true;
    }

    if( pAutocloseParenChk->IsValueChangedFromSaved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::set( pAutocloseParenChk->IsChecked(), batch );
        CodeCompleteOptions::SetAutoCloseParenthesisOn( pAutocloseParenChk->IsChecked() );
        bModified = true;
    }

    if( pAutocloseQuotesChk->IsValueChangedFromSaved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::set( pAutocloseQuotesChk->IsChecked(), batch );
        CodeCompleteOptions::SetAutoCloseQuotesOn( pAutocloseQuotesChk->IsChecked() );
        bModified = true;
    }

    if( pAutoCorrectChk->IsValueChangedFromSaved() )
    {
        officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::set( pAutoCorrectChk->IsChecked(), batch );
        CodeCompleteOptions::SetAutoCorrectOn( pAutoCorrectChk->IsChecked() );
        bModified = true;
    }

    if( bModified )
        batch->commit();

    return bModified;
}

void SvxBasicIDEOptionsPage::Reset( const SfxItemSet* /*rSet*/ )
{
    LoadConfig();
    pCodeCompleteChk->SaveValue();

    pAutocloseProcChk->SaveValue();

    pAutocloseQuotesChk->SaveValue();

    pAutocloseParenChk->SaveValue();

    pAutoCorrectChk->SaveValue();

    pUseExtendedTypesChk->SaveValue();
}

VclPtr<SfxTabPage> SvxBasicIDEOptionsPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxBasicIDEOptionsPage>::Create( pParent, *rAttrSet );
}

void SvxBasicIDEOptionsPage::FillUserData()
{
    SetUserData( OUString() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
