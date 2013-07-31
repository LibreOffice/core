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

#include "codecompleteoptionsdlg.hxx"
#include <basic/codecompletecache.hxx>
#include <svtools/miscopt.hxx>
#include <basidesh.hrc>
#include <iostream>
#include <officecfg/Office/BasicIDE.hxx>
#include <boost/shared_ptr.hpp>

namespace basctl
{

CodeCompleteOptionsDlg::CodeCompleteOptionsDlg( Window* pWindow )
: ModalDialog(pWindow, "CodeCompleteOptionsDialog", "modules/BasicIDE/ui/codecompleteoptionsdlg.ui")
{
    get(pCancelBtn, "cancel");
    get(pOkBtn, "ok");

    get(pCodeCompleteChk, "codecomplete_enable");
    get(pAutocloseProcChk, "autoclose_proc");
    get(pAutocloseParenChk, "autoclose_paren");
    get(pAutocloseQuotesChk, "autoclose_quotes");
    get(pAutoCorrectSpellingChk, "autocorrect_spelling");
    get(pUseExtendedTypesChk, "extendedtypes_enable");

    pOkBtn->SetClickHdl( LINK( this, CodeCompleteOptionsDlg, OkHdl ) );
    pCancelBtn->SetClickHdl( LINK( this, CodeCompleteOptionsDlg, CancelHdl ) );

    pCodeCompleteChk->SetToggleHdl( LINK(this, CodeCompleteOptionsDlg, CodeCompleteHdl) );
    pUseExtendedTypesChk->SetToggleHdl( LINK(this, CodeCompleteOptionsDlg, ExtendedTypesHdl) );

    LoadConfig();

}

CodeCompleteOptionsDlg::~CodeCompleteOptionsDlg()
{
}

IMPL_LINK_NOARG(CodeCompleteOptionsDlg, OkHdl)
{
    CodeCompleteOptions::SetCodeCompleteOn( pCodeCompleteChk->IsChecked() );
    CodeCompleteOptions::SetProcedureAutoCompleteOn( pAutocloseProcChk->IsChecked() );
    CodeCompleteOptions::SetAutoCloseQuotesOn( pAutocloseQuotesChk->IsChecked() );
    CodeCompleteOptions::SetAutoCloseParenthesisOn( pAutocloseParenChk->IsChecked() );
    CodeCompleteOptions::SetAutoCorrectSpellingOn( pAutoCorrectSpellingChk->IsChecked() );
    CodeCompleteOptions::SetExtendedTypeDeclaration( pAutoCorrectSpellingChk->IsChecked() );

    SaveConfig();
    Close();
    return 0;
}

IMPL_LINK_NOARG(CodeCompleteOptionsDlg, CancelHdl)
{
    Close();
    return 0;
}

IMPL_LINK_NOARG(CodeCompleteOptionsDlg, ExtendedTypesHdl)
{
    pCodeCompleteChk->Check( pUseExtendedTypesChk->IsChecked() );
    return 0;
}

IMPL_LINK_NOARG(CodeCompleteOptionsDlg, CodeCompleteHdl)
{
    pUseExtendedTypesChk->Check( pCodeCompleteChk->IsChecked() );
    return 0;
}

short CodeCompleteOptionsDlg::Execute()
{
    return ModalDialog::Execute();
}

void CodeCompleteOptionsDlg::LoadConfig()
{
    bool bProcClose = officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::get();
    bool bExtended = officecfg::Office::BasicIDE::Autocomplete::UseExtended::get();
    bool bCodeCompleteOn = officecfg::Office::BasicIDE::Autocomplete::CodeComplete::get();
    bool bParenClose = officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::get();
    bool bQuoteClose = officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::get();
    bool bCorrect = officecfg::Office::BasicIDE::Autocomplete::AutoCorrectSpelling::get();

    pCodeCompleteChk->Check( bCodeCompleteOn );
    pAutocloseProcChk->Check( bProcClose );
    pAutocloseQuotesChk->Check( bQuoteClose );
    pAutocloseParenChk->Check( bParenClose );
    pAutoCorrectSpellingChk->Check( bCorrect );
    pUseExtendedTypesChk->Check( bExtended );
}

void CodeCompleteOptionsDlg::SaveConfig()
{
    boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::set( pAutocloseProcChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::CodeComplete::set( pCodeCompleteChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::UseExtended::set( pUseExtendedTypesChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::set( pAutocloseParenChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::set( pAutocloseQuotesChk->IsChecked(), batch );
    officecfg::Office::BasicIDE::Autocomplete::AutoCorrectSpelling::set( pAutoCorrectSpellingChk->IsChecked(), batch );
    batch->commit();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
