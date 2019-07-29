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

ScTpFormulaOptions::ScTpFormulaOptions(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "modules/scalc/ui/optformula.ui", "OptFormula", &rCoreAttrs)
    , mnDecSep(0)
    , mxLbFormulaSyntax(m_xBuilder->weld_combo_box("formulasyntax"))
    , mxCbEnglishFuncName(m_xBuilder->weld_check_button("englishfuncname"))
    , mxBtnCustomCalcDefault(m_xBuilder->weld_radio_button("calcdefault"))
    , mxBtnCustomCalcCustom(m_xBuilder->weld_radio_button("calccustom"))
    , mxBtnCustomCalcDetails(m_xBuilder->weld_button("details"))
    , mxEdSepFuncArg(m_xBuilder->weld_entry("function"))
    , mxEdSepArrayCol(m_xBuilder->weld_entry("arraycolumn"))
    , mxEdSepArrayRow(m_xBuilder->weld_entry("arrayrow"))
    , mxBtnSepReset(m_xBuilder->weld_button("reset"))
    , mxLbOOXMLRecalcOptions(m_xBuilder->weld_combo_box("ooxmlrecalc"))
    , mxLbODFRecalcOptions(m_xBuilder->weld_combo_box("odfrecalc"))
{
    mxLbFormulaSyntax->append_text(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1));
    mxLbFormulaSyntax->append_text(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1));
    mxLbFormulaSyntax->append_text(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1));

    Link<weld::Button&,void> aLink2 = LINK( this, ScTpFormulaOptions, ButtonHdl );
    mxBtnSepReset->connect_clicked(aLink2);
    mxBtnCustomCalcDefault->connect_clicked(aLink2);
    mxBtnCustomCalcCustom->connect_clicked(aLink2);
    mxBtnCustomCalcDetails->connect_clicked(aLink2);

    Link<weld::Entry&,void> aLink = LINK( this, ScTpFormulaOptions, SepModifyHdl );
    mxEdSepFuncArg->connect_changed(aLink);
    mxEdSepArrayCol->connect_changed(aLink);
    mxEdSepArrayRow->connect_changed(aLink);

    Link<weld::Widget&,void> aLink3 = LINK( this, ScTpFormulaOptions, SepEditOnFocusHdl );
    mxEdSepFuncArg->connect_focus_in(aLink3);
    mxEdSepArrayCol->connect_focus_in(aLink3);
    mxEdSepArrayRow->connect_focus_in(aLink3);

    // Get the decimal separator for current locale.
    OUString aSep = ScGlobal::GetpLocaleData()->getNumDecimalSep();
    mnDecSep = aSep.isEmpty() ? u'.' : aSep[0];

    maSavedDocOptions = static_cast<const ScTpCalcItem&>(rCoreAttrs.Get(
            GetWhich(SID_SCDOCOPTIONS))).GetDocOptions();
}

ScTpFormulaOptions::~ScTpFormulaOptions()
{
}

void ScTpFormulaOptions::ResetSeparators()
{
    OUString aFuncArg, aArrayCol, aArrayRow;
    ScFormulaOptions::GetDefaultFormulaSeparators(aFuncArg, aArrayCol, aArrayRow);
    mxEdSepFuncArg->set_text(aFuncArg);
    mxEdSepArrayCol->set_text(aArrayCol);
    mxEdSepArrayRow->set_text(aArrayRow);
}

void ScTpFormulaOptions::OnFocusSeparatorInput(weld::Entry* pEdit)
{
    if (!pEdit)
        return;

    // Make sure the entire text is selected.
    pEdit->select_region(0, -1);
    maOldSepValue = pEdit->get_text();
}

void ScTpFormulaOptions::UpdateCustomCalcRadioButtons(bool bDefault)
{
    if (bDefault)
    {
        mxBtnCustomCalcDefault->set_active(true);
        mxBtnCustomCalcCustom->set_active(false);
        mxBtnCustomCalcDetails->set_sensitive(false);
    }
    else
    {
        mxBtnCustomCalcDefault->set_active(false);
        mxBtnCustomCalcCustom->set_active(true);
        mxBtnCustomCalcDetails->set_sensitive(true);
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
    OUString aColStr = mxEdSepArrayCol->get_text();
    OUString aRowStr = mxEdSepArrayRow->get_text();
    return aColStr != aRowStr;
}

IMPL_LINK( ScTpFormulaOptions, ButtonHdl, weld::Button&, rBtn, void )
{
    if (&rBtn == mxBtnSepReset.get())
        ResetSeparators();
    else if (&rBtn == mxBtnCustomCalcDefault.get())
        UpdateCustomCalcRadioButtons(true);
    else if (&rBtn == mxBtnCustomCalcCustom.get())
        UpdateCustomCalcRadioButtons(false);
    else if (&rBtn == mxBtnCustomCalcDetails.get())
        LaunchCustomCalcSettings();
}

IMPL_LINK( ScTpFormulaOptions, SepModifyHdl, weld::Entry&, rEdit, void )
{
    OUString aStr = rEdit.get_text();
    if (aStr.getLength() > 1)
    {
        // In case the string is more than one character long, only grab the
        // first character.
        aStr = aStr.copy(0, 1);
        rEdit.set_text(aStr);
    }

    if ((!IsValidSeparator(aStr) || !IsValidSeparatorSet()) && !maOldSepValue.isEmpty())
        // Invalid separator.  Restore the old value.
        rEdit.set_text(maOldSepValue);

    OnFocusSeparatorInput(&rEdit);
}

IMPL_LINK( ScTpFormulaOptions, SepEditOnFocusHdl, weld::Widget&, rControl, void )
{
    OnFocusSeparatorInput(dynamic_cast<weld::Entry*>(&rControl));
}

VclPtr<SfxTabPage> ScTpFormulaOptions::Create(TabPageParent pParent, const SfxItemSet* rCoreSet)
{
    return VclPtr<ScTpFormulaOptions>::Create(pParent, *rCoreSet);
}

bool ScTpFormulaOptions::FillItemSet(SfxItemSet* rCoreSet)
{
    bool bRet = false;
    ScFormulaOptions aOpt;
    bool bEnglishFuncName = mxCbEnglishFuncName->get_active();
    sal_Int16 aSyntaxPos      = mxLbFormulaSyntax->get_active();
    OUString aSep             = mxEdSepFuncArg->get_text();
    OUString aSepArrayCol     = mxEdSepArrayCol->get_text();
    OUString aSepArrayRow     = mxEdSepArrayRow->get_text();
    sal_Int16 nOOXMLRecalcMode = mxLbOOXMLRecalcOptions->get_active();
    sal_Int16 nODFRecalcMode = mxLbODFRecalcOptions->get_active();

    if (mxBtnCustomCalcDefault->get_active())
    {
        // When Default is selected, reset all the calc config settings to default.
        maCurrentConfig.reset();
    }

    if ( mxLbFormulaSyntax->get_saved_value() != mxLbFormulaSyntax->get_text(aSyntaxPos)
         || mxCbEnglishFuncName->get_saved_state() != (bEnglishFuncName ? 1 : 0)
         || mxEdSepFuncArg->get_saved_value() != aSep
         || mxEdSepArrayCol->get_saved_value() != aSepArrayCol
         || mxEdSepArrayRow->get_saved_value() != aSepArrayRow
         || mxLbOOXMLRecalcOptions->get_saved_value() != mxLbOOXMLRecalcOptions->get_text(nOOXMLRecalcMode)
         || mxLbODFRecalcOptions->get_saved_value() != mxLbODFRecalcOptions->get_text(nODFRecalcMode)
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
        mxLbFormulaSyntax->set_active(0);
        break;
    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_A1:
        mxLbFormulaSyntax->set_active(1);
        break;
    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1:
        mxLbFormulaSyntax->set_active(2);
        break;
    default:
        mxLbFormulaSyntax->set_active(0);
    }

    mxLbFormulaSyntax->save_value();

    ScRecalcOptions eOOXMLRecalc = aOpt.GetOOXMLRecalcOptions();
    mxLbOOXMLRecalcOptions->set_active(static_cast<sal_uInt16>(eOOXMLRecalc));
    mxLbOOXMLRecalcOptions->save_value();

    ScRecalcOptions eODFRecalc = aOpt.GetODFRecalcOptions();
    mxLbODFRecalcOptions->set_active(static_cast<sal_uInt16>(eODFRecalc));
    mxLbODFRecalcOptions->save_value();

    // english function name.
    mxCbEnglishFuncName->set_active( aOpt.GetUseEnglishFuncName() );
    mxCbEnglishFuncName->save_state();

    // Separators
    OUString aSep = aOpt.GetFormulaSepArg();
    OUString aSepArrayRow = aOpt.GetFormulaSepArrayRow();
    OUString aSepArrayCol = aOpt.GetFormulaSepArrayCol();

    if (aSep.getLength() == 1 && aSepArrayRow.getLength() == 1 && aSepArrayCol.getLength() == 1)
    {
        // Each separator must be one character long.
        mxEdSepFuncArg->set_text(aSep);
        mxEdSepArrayCol->set_text(aSepArrayCol);
        mxEdSepArrayRow->set_text(aSepArrayRow);

        mxEdSepFuncArg->save_value();
        mxEdSepArrayCol->save_value();
        mxEdSepArrayRow->save_value();
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
