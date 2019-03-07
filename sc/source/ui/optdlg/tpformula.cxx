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

#undef SC_DLLIMPLEMENTATION

#include <global.hxx>
#include <tpformula.hxx>
#include <formulaopt.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <scresid.hxx>
#include <formula/grammar.hxx>
#include "calcoptionsdlg.hxx"
#include <vcl/edit.hxx>

#include <unotools/localedatawrapper.hxx>


ScTpFormulaOptions::ScTpFormulaOptions(vcl::Window* pParent, const SfxItemSet& rCoreAttrs) :
    SfxTabPage(pParent, "OptFormula", "modules/scalc/ui/optformula.ui", &rCoreAttrs),
    mnDecSep(0)
{
    get(mpLbFormulaSyntax, "formulasyntax");
    get(mpCbEnglishFuncName, "englishfuncname");
    get(mpBtnCustomCalcDefault, "calcdefault");
    get(mpBtnCustomCalcCustom, "calccustom");
    get(mpBtnCustomCalcDetails, "details");
    get(mpEdSepFuncArg, "function");
    get(mpEdSepArrayCol, "arraycolumn");
    get(mpEdSepArrayRow, "arrayrow");
    get(mpBtnSepReset, "reset");
    get(mpLbOOXMLRecalcOptions, "ooxmlrecalc");
    get(mpLbODFRecalcOptions, "odfrecalc");

    mpLbFormulaSyntax->InsertEntry(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1));
    mpLbFormulaSyntax->InsertEntry(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1));
    mpLbFormulaSyntax->InsertEntry(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1));

    Link<Button*,void> aLink2 = LINK( this, ScTpFormulaOptions, ButtonHdl );
    mpBtnSepReset->SetClickHdl(aLink2);
    mpBtnCustomCalcDefault->SetClickHdl(aLink2);
    mpBtnCustomCalcCustom->SetClickHdl(aLink2);
    mpBtnCustomCalcDetails->SetClickHdl(aLink2);

    Link<Edit&,void> aLink = LINK( this, ScTpFormulaOptions, SepModifyHdl );
    mpEdSepFuncArg->SetModifyHdl(aLink);
    mpEdSepArrayCol->SetModifyHdl(aLink);
    mpEdSepArrayRow->SetModifyHdl(aLink);

    Link<Control&,void> aLink3 = LINK( this, ScTpFormulaOptions, SepEditOnFocusHdl );
    mpEdSepFuncArg->SetGetFocusHdl(aLink3);
    mpEdSepArrayCol->SetGetFocusHdl(aLink3);
    mpEdSepArrayRow->SetGetFocusHdl(aLink3);

    // Get the decimal separator for current locale.
    OUString aSep = ScGlobal::GetpLocaleData()->getNumDecimalSep();
    mnDecSep = aSep.isEmpty() ? u'.' : aSep[0];

    maSavedDocOptions = static_cast<const ScTpCalcItem&>(rCoreAttrs.Get(
            GetWhich(SID_SCDOCOPTIONS))).GetDocOptions();
}

ScTpFormulaOptions::~ScTpFormulaOptions()
{
    disposeOnce();
}

void ScTpFormulaOptions::dispose()
{
    mpLbFormulaSyntax.clear();
    mpCbEnglishFuncName.clear();
    mpBtnCustomCalcDefault.clear();
    mpBtnCustomCalcCustom.clear();
    mpBtnCustomCalcDetails.clear();
    mpEdSepFuncArg.clear();
    mpEdSepArrayCol.clear();
    mpEdSepArrayRow.clear();
    mpBtnSepReset.clear();
    mpLbOOXMLRecalcOptions.clear();
    mpLbODFRecalcOptions.clear();
    SfxTabPage::dispose();
}

void ScTpFormulaOptions::ResetSeparators()
{
    OUString aFuncArg, aArrayCol, aArrayRow;
    ScFormulaOptions::GetDefaultFormulaSeparators(aFuncArg, aArrayCol, aArrayRow);
    mpEdSepFuncArg->SetText(aFuncArg);
    mpEdSepArrayCol->SetText(aArrayCol);
    mpEdSepArrayRow->SetText(aArrayRow);
}

void ScTpFormulaOptions::OnFocusSeparatorInput(Edit* pEdit)
{
    if (!pEdit)
        return;

    // Make sure the entire text is selected.
    sal_Int32 nLen = pEdit->GetText().getLength();
    Selection aSel(0, static_cast<sal_uInt16>(nLen));
    pEdit->SetSelection(aSel);
    maOldSepValue = pEdit->GetText();
}

void ScTpFormulaOptions::UpdateCustomCalcRadioButtons(bool bDefault)
{
    if (bDefault)
    {
        mpBtnCustomCalcDefault->Check();
        mpBtnCustomCalcCustom->Check(false);
        mpBtnCustomCalcDetails->Disable();
    }
    else
    {
        mpBtnCustomCalcDefault->Check(false);
        mpBtnCustomCalcCustom->Check();
        mpBtnCustomCalcDetails->Enable();
    }
}

void ScTpFormulaOptions::LaunchCustomCalcSettings()
{
    ScCalcOptionsDialog aDlg(GetFrameWeld(), maCurrentConfig, maCurrentDocOptions.IsWriteCalcConfig());
    if (aDlg.run() == RET_OK)
    {
        maCurrentConfig = aDlg.GetConfig();
        maCurrentDocOptions.SetWriteCalcConfig(aDlg.GetWriteCalcConfig());
    }
}

bool ScTpFormulaOptions::IsValidSeparator(const OUString& rSep) const
{
    if (rSep.getLength() != 1)
        // Must be one-character long.
        return false;

    if (rSep.compareToAscii("a") >= 0 && rSep.compareToAscii("z") <= 0)
        return false;

    if (rSep.compareToAscii("A") >= 0 && rSep.compareToAscii("Z") <= 0)
        return false;

    sal_Unicode c = rSep[0];
    switch (c)
    {
        case '+':
        case '-':
        case '/':
        case '*':
        case '<':
        case '>':
        case '[':
        case ']':
        case '(':
        case ')':
        case '"':
        case '\'':
            // Disallowed characters.  Anything else we want to disallow ?
            return false;
    }

    if (c == mnDecSep)
        // decimal separator is not allowed.
        return false;

    return true;
}

bool ScTpFormulaOptions::IsValidSeparatorSet() const
{
    // Make sure the column and row separators are different.
    OUString aColStr = mpEdSepArrayCol->GetText();
    OUString aRowStr = mpEdSepArrayRow->GetText();
    return aColStr != aRowStr;
}

IMPL_LINK( ScTpFormulaOptions, ButtonHdl, Button*, pBtn, void )
{
    if (pBtn == mpBtnSepReset)
        ResetSeparators();
    else if (pBtn == mpBtnCustomCalcDefault)
        UpdateCustomCalcRadioButtons(true);
    else if (pBtn == mpBtnCustomCalcCustom)
        UpdateCustomCalcRadioButtons(false);
    else if (pBtn == mpBtnCustomCalcDetails)
        LaunchCustomCalcSettings();
}

IMPL_LINK( ScTpFormulaOptions, SepModifyHdl, Edit&, rEdit, void )
{
    OUString aStr = rEdit.GetText();
    if (aStr.getLength() > 1)
    {
        // In case the string is more than one character long, only grab the
        // first character.
        aStr = aStr.copy(0, 1);
        rEdit.SetText(aStr);
    }

    if ((!IsValidSeparator(aStr) || !IsValidSeparatorSet()) && !maOldSepValue.isEmpty())
        // Invalid separator.  Restore the old value.
        rEdit.SetText(maOldSepValue);

    OnFocusSeparatorInput(&rEdit);
}

IMPL_LINK( ScTpFormulaOptions, SepEditOnFocusHdl, Control&, rControl, void )
{
    OnFocusSeparatorInput(static_cast<Edit*>(&rControl));
}

VclPtr<SfxTabPage> ScTpFormulaOptions::Create(TabPageParent pParent, const SfxItemSet* rCoreSet)
{
    return VclPtr<ScTpFormulaOptions>::Create(pParent.pParent, *rCoreSet);
}

bool ScTpFormulaOptions::FillItemSet(SfxItemSet* rCoreSet)
{
    bool bRet = false;
    ScFormulaOptions aOpt;
    bool bEnglishFuncName = mpCbEnglishFuncName->IsChecked();
    sal_Int16 aSyntaxPos      = mpLbFormulaSyntax->GetSelectedEntryPos();
    OUString aSep             = mpEdSepFuncArg->GetText();
    OUString aSepArrayCol     = mpEdSepArrayCol->GetText();
    OUString aSepArrayRow     = mpEdSepArrayRow->GetText();
    sal_Int16 nOOXMLRecalcMode = mpLbOOXMLRecalcOptions->GetSelectedEntryPos();
    sal_Int16 nODFRecalcMode = mpLbODFRecalcOptions->GetSelectedEntryPos();

    if (mpBtnCustomCalcDefault->IsChecked())
    {
        // When Default is selected, reset all the calc config settings to default.
        maCurrentConfig.reset();
    }

    if ( mpLbFormulaSyntax->GetSavedValue() != aSyntaxPos
         || mpCbEnglishFuncName->GetSavedValue() != (bEnglishFuncName ? 1 : 0)
         || mpEdSepFuncArg->GetSavedValue() != aSep
         || mpEdSepArrayCol->GetSavedValue() != aSepArrayCol
         || mpEdSepArrayRow->GetSavedValue() != aSepArrayRow
         || mpLbOOXMLRecalcOptions->GetSavedValue() != nOOXMLRecalcMode
         || mpLbODFRecalcOptions->GetSavedValue() != nODFRecalcMode
         || maSavedConfig != maCurrentConfig
         || maSavedDocOptions != maCurrentDocOptions )
    {
        ::formula::FormulaGrammar::Grammar eGram = ::formula::FormulaGrammar::GRAM_DEFAULT;

        switch (aSyntaxPos)
        {
        case 0:
            eGram = ::formula::FormulaGrammar::GRAM_NATIVE;
        break;
        case 1:
            eGram = ::formula::FormulaGrammar::GRAM_NATIVE_XL_A1;
        break;
        case 2:
            eGram = ::formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1;
        break;
        }

        ScRecalcOptions eOOXMLRecalc = static_cast<ScRecalcOptions>(nOOXMLRecalcMode);
        ScRecalcOptions eODFRecalc = static_cast<ScRecalcOptions>(nODFRecalcMode);

        aOpt.SetFormulaSyntax(eGram);
        aOpt.SetUseEnglishFuncName(bEnglishFuncName);
        aOpt.SetFormulaSepArg(aSep);
        aOpt.SetFormulaSepArrayCol(aSepArrayCol);
        aOpt.SetFormulaSepArrayRow(aSepArrayRow);
        aOpt.SetCalcConfig(maCurrentConfig);
        aOpt.SetOOXMLRecalcOptions(eOOXMLRecalc);
        aOpt.SetODFRecalcOptions(eODFRecalc);
        aOpt.SetWriteCalcConfig( maCurrentDocOptions.IsWriteCalcConfig());

        rCoreSet->Put( ScTpFormulaItem( aOpt ) );
        rCoreSet->Put( ScTpCalcItem( SID_SCDOCOPTIONS, maCurrentDocOptions ) );

        bRet = true;
    }
    return bRet;
}

void ScTpFormulaOptions::Reset(const SfxItemSet* rCoreSet)
{
    ScFormulaOptions aOpt;
    const SfxPoolItem* pItem = nullptr;

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SCFORMULAOPTIONS, false , &pItem))
        aOpt = static_cast<const ScTpFormulaItem*>(pItem)->GetFormulaOptions();

    // formula grammar.
    ::formula::FormulaGrammar::Grammar eGram = aOpt.GetFormulaSyntax();

    switch (eGram)
    {
    case ::formula::FormulaGrammar::GRAM_NATIVE:
        mpLbFormulaSyntax->SelectEntryPos(0);
        break;
    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_A1:
        mpLbFormulaSyntax->SelectEntryPos(1);
        break;
    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1:
        mpLbFormulaSyntax->SelectEntryPos(2);
        break;
    default:
        mpLbFormulaSyntax->SelectEntryPos(0);
    }

    mpLbFormulaSyntax->SaveValue();

    ScRecalcOptions eOOXMLRecalc = aOpt.GetOOXMLRecalcOptions();
    mpLbOOXMLRecalcOptions->SelectEntryPos(static_cast<sal_uInt16>(eOOXMLRecalc));
    mpLbOOXMLRecalcOptions->SaveValue();

    ScRecalcOptions eODFRecalc = aOpt.GetODFRecalcOptions();
    mpLbODFRecalcOptions->SelectEntryPos(static_cast<sal_uInt16>(eODFRecalc));
    mpLbODFRecalcOptions->SaveValue();

    // english function name.
    mpCbEnglishFuncName->Check( aOpt.GetUseEnglishFuncName() );
    mpCbEnglishFuncName->SaveValue();

    // Separators
    OUString aSep = aOpt.GetFormulaSepArg();
    OUString aSepArrayRow = aOpt.GetFormulaSepArrayRow();
    OUString aSepArrayCol = aOpt.GetFormulaSepArrayCol();

    if (aSep.getLength() == 1 && aSepArrayRow.getLength() == 1 && aSepArrayCol.getLength() == 1)
    {
        // Each separator must be one character long.
        mpEdSepFuncArg->SetText(aSep);
        mpEdSepArrayCol->SetText(aSepArrayCol);
        mpEdSepArrayRow->SetText(aSepArrayRow);

        mpEdSepFuncArg->SaveValue();
        mpEdSepArrayCol->SaveValue();
        mpEdSepArrayRow->SaveValue();
    }
    else
        ResetSeparators();

    // detailed calc settings.
    ScFormulaOptions aDefaults;

    maSavedConfig = aOpt.GetCalcConfig();
    bool bDefault = aDefaults.GetCalcConfig() == maSavedConfig;
    UpdateCustomCalcRadioButtons(bDefault);

    maCurrentConfig = maSavedConfig;

    maCurrentDocOptions = maSavedDocOptions;
}

DeactivateRC ScTpFormulaOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    // What's this method for ?
    return DeactivateRC::KeepPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
