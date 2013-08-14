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
#include <svtools/miscopt.hxx>
#include <iostream>
#include <officecfg/Office/BasicIDE.hxx>
#include <boost/shared_ptr.hpp>
#include <cuires.hrc>
#include <svtools/miscopt.hxx>

SvxBasicIDEOptionsPage::SvxBasicIDEOptionsPage( Window* pParent, const SfxItemSet& rSet )
: SfxTabPage(pParent, "OptBasicIDEPage", "cui/ui/optbasicidepage.ui", rSet)
{
    SvtMiscOptions aMiscOpt;
    if( ! aMiscOpt.IsExperimentalMode() )
    {
        Disable();
    }

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
}

void SvxBasicIDEOptionsPage::LoadConfig()
{
    bool bProcClose = officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::get();
    bool bExtended = officecfg::Office::BasicIDE::Autocomplete::UseExtended::get();
    bool bCodeCompleteOn = officecfg::Office::BasicIDE::Autocomplete::CodeComplete::get();
    bool bParenClose = officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::get();
    bool bQuoteClose = officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::get();
    bool bCorrect = officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::get();

    pCodeCompleteChk->Check( bCodeCompleteOn );
    pAutocloseProcChk->Check( bProcClose );
    pAutocloseQuotesChk->Check( bQuoteClose );
    pAutocloseParenChk->Check( bParenClose );
    pAutoCorrectChk->Check( bCorrect );
    pUseExtendedTypesChk->Check( bExtended );
}

void SvxBasicIDEOptionsPage::SaveConfig()
{
    boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::set( pAutocloseProcChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::CodeComplete::set( pCodeCompleteChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::UseExtended::set( pUseExtendedTypesChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::set( pAutocloseParenChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::set( pAutocloseQuotesChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::set( pAutoCorrectChk->IsChecked(), batch );
    batch->commit();
}

sal_Bool SvxBasicIDEOptionsPage::FillItemSet( SfxItemSet& /*rCoreSet*/ )
{
    sal_Bool bModified = sal_False;

    if( pAutocloseProcChk->IsChecked() != pAutocloseProcChk->GetSavedValue() )
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::set( pAutocloseProcChk->IsChecked(), batch );
        batch->commit();
        bModified = sal_True;
    }

    if( pCodeCompleteChk->IsChecked() != pCodeCompleteChk->GetSavedValue() )
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::CodeComplete::set( pCodeCompleteChk->IsChecked(), batch );
        batch->commit();
        bModified = sal_True;
    }

    if( pUseExtendedTypesChk->IsChecked() != pUseExtendedTypesChk->GetSavedValue() )
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::UseExtended::set( pUseExtendedTypesChk->IsChecked(), batch );
        batch->commit();
        bModified = sal_True;
    }

    if( pAutocloseParenChk->IsChecked() != pAutocloseParenChk->GetSavedValue() )
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::set( pAutocloseParenChk->IsChecked(), batch );
        batch->commit();
        bModified = sal_True;
    }

    if( pAutocloseQuotesChk->IsChecked() != pAutocloseQuotesChk->GetSavedValue() )
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::set( pAutocloseQuotesChk->IsChecked(), batch );
        batch->commit();
        bModified = sal_True;
    }

    if( pAutoCorrectChk->IsChecked() != pAutoCorrectChk->GetSavedValue() )
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::set( pAutoCorrectChk->IsChecked(), batch );
        batch->commit();
        bModified = sal_True;
    }

    return bModified;
}

void SvxBasicIDEOptionsPage::Reset( const SfxItemSet& /*rSet*/ )
{
    LoadConfig();
    pCodeCompleteChk->SaveValue();

    pAutocloseProcChk->SaveValue();

    pAutocloseQuotesChk->SaveValue();

    pAutocloseParenChk->SaveValue();

    pAutoCorrectChk->SaveValue();

    pUseExtendedTypesChk->SaveValue();
}

SfxTabPage* SvxBasicIDEOptionsPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxBasicIDEOptionsPage( pParent, rAttrSet ) );
}

void SvxBasicIDEOptionsPage::FillUserData()
{
    OUString aUserData;
    SetUserData( aUserData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
