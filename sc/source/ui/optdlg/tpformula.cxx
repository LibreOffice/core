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

//------------------------------------------------------------------

#include <scmod.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include "tpformula.hxx"
#include "formulaopt.hxx"
#include "optdlg.hrc"
#include "scresid.hxx"
#include "formula/grammar.hxx"
#include "calcoptionsdlg.hxx"
#include "vcl/msgbox.hxx"

#include <unotools/localedatawrapper.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>

using ::com::sun::star::lang::Locale;
using ::com::sun::star::i18n::LocaleDataItem;

ScTpFormulaOptions::ScTpFormulaOptions(Window* pParent, const SfxItemSet& rCoreAttrs) :
    SfxTabPage(pParent, ScResId(RID_SCPAGE_FORMULA), rCoreAttrs),

    maFlFormulaOpt(this, ScResId(FL_FORMULA_OPTIONS)),
    maFtFormulaSyntax(this, ScResId(FT_FORMULA_SYNTAX)),
    maLbFormulaSyntax(this, ScResId(LB_FORMULA_SYNTAX)),
    maCbEnglishFuncName(this, ScResId(CB_ENGLISH_FUNC_NAME)),

    maFlCustomCalcOpt(this, ScResId(FL_CUSTOM_CALC_OPTIONS)),
    maBtnCustomCalcDefault(this, ScResId(BTN_CUSTOM_CALC_DEFAULT)),
    maBtnCustomCalcCustom(this, ScResId(BTN_CUSTOM_CALC_CUSTOM)),
    maBtnCustomCalcDetails(this, ScResId(BTN_CUSTOM_CALC_DETAILS)),

    maFlFormulaSeps(this, ScResId(FL_FORMULA_SEPS)),
    maFtSepFuncArg(this, ScResId(FT_FORMULA_SEP_ARG)),
    maEdSepFuncArg(this, ScResId(ED_FORMULA_SEP_ARG)),
    maFtSepArrayCol(this, ScResId(FT_FORMULA_SEP_ARRAY_C)),
    maEdSepArrayCol(this, ScResId(ED_FORMULA_SEP_ARRAY_C)),
    maFtSepArrayRow(this, ScResId(FT_FORMULA_SEP_ARRAY_R)),
    maEdSepArrayRow(this, ScResId(ED_FORMULA_SEP_ARRAY_R)),
    maBtnSepReset(this, ScResId(BTN_FORMULA_SEP_RESET)),
    maFlRecalcOptions(this, ScResId(FL_RECALC_OPTIONS)),
    maFtOOXMLRecalc(this, ScResId(FT_OOXML_RECALC)),
    maLbOOXMLRecalcOptions(this, ScResId(LB_OOXML_RECALC)),
    maFtODFRecalc(this, ScResId(FT_ODF_RECALC)),
    maLbODFRecalcOptions(this, ScResId(LB_ODF_RECALC)),
    mnDecSep(0)
{
    maLbFormulaSyntax.InsertEntry(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1).toString());
    maLbFormulaSyntax.InsertEntry(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1).toString());
    maLbFormulaSyntax.InsertEntry(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1).toString());

    FreeResource();

    Link aLink = LINK( this, ScTpFormulaOptions, ButtonHdl );
    maBtnSepReset.SetClickHdl(aLink);
    maBtnCustomCalcDefault.SetClickHdl(aLink);
    maBtnCustomCalcCustom.SetClickHdl(aLink);
    maBtnCustomCalcDetails.SetClickHdl(aLink);

    aLink = LINK( this, ScTpFormulaOptions, SepModifyHdl );
    maEdSepFuncArg.SetModifyHdl(aLink);
    maEdSepArrayCol.SetModifyHdl(aLink);
    maEdSepArrayRow.SetModifyHdl(aLink);

    aLink = LINK( this, ScTpFormulaOptions, SepEditOnFocusHdl );
    maEdSepFuncArg.SetGetFocusHdl(aLink);
    maEdSepArrayCol.SetGetFocusHdl(aLink);
    maEdSepArrayRow.SetGetFocusHdl(aLink);

    // Get the decimal separator for current locale.
    OUString aSep = ScGlobal::GetpLocaleData()->getNumDecimalSep();
    mnDecSep = aSep.isEmpty() ? sal_Unicode('.') : aSep[0];
}

ScTpFormulaOptions::~ScTpFormulaOptions()
{
}

void ScTpFormulaOptions::ResetSeparators()
{
    OUString aFuncArg, aArrayCol, aArrayRow;
    ScFormulaOptions::GetDefaultFormulaSeparators(aFuncArg, aArrayCol, aArrayRow);
    maEdSepFuncArg.SetText(aFuncArg);
    maEdSepArrayCol.SetText(aArrayCol);
    maEdSepArrayRow.SetText(aArrayRow);
}

void ScTpFormulaOptions::OnFocusSeparatorInput(Edit* pEdit)
{
    if (!pEdit)
        return;

    // Make sure the entire text is selected.
    sal_Int32 nLen = pEdit->GetText().getLength();
    Selection aSel(0, (sal_uInt16)nLen);
    pEdit->SetSelection(aSel);
    maOldSepValue = pEdit->GetText();
}

void ScTpFormulaOptions::UpdateCustomCalcRadioButtons(bool bDefault)
{
    if (bDefault)
    {
        maBtnCustomCalcDefault.Check(true);
        maBtnCustomCalcCustom.Check(false);
        maBtnCustomCalcDetails.Disable();
    }
    else
    {
        maBtnCustomCalcDefault.Check(false);
        maBtnCustomCalcCustom.Check(true);
        maBtnCustomCalcDetails.Enable();
    }
}

void ScTpFormulaOptions::LaunchCustomCalcSettings()
{
    ScCalcOptionsDialog aDlg(this, maCurrentConfig);
    if (aDlg.Execute() == RET_OK)
    {
        maCurrentConfig = aDlg.GetConfig();
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

    sal_Unicode c = rSep.getStr()[0];
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
    String aColStr = maEdSepArrayCol.GetText();
    String aRowStr = maEdSepArrayRow.GetText();
    if (aColStr == aRowStr)
        return false;

    return true;
}

IMPL_LINK( ScTpFormulaOptions, ButtonHdl, Button*, pBtn )
{
    if (pBtn == &maBtnSepReset)
        ResetSeparators();
    else if (pBtn == &maBtnCustomCalcDefault)
        UpdateCustomCalcRadioButtons(true);
    else if (pBtn == &maBtnCustomCalcCustom)
        UpdateCustomCalcRadioButtons(false);
    else if (pBtn == &maBtnCustomCalcDetails)
        LaunchCustomCalcSettings();

    return 0;
}

IMPL_LINK( ScTpFormulaOptions, SepModifyHdl, Edit*, pEdit )
{
    if (!pEdit)
        return 0;

    String aStr = pEdit->GetText();
    if (aStr.Len() > 1)
    {
        // In case the string is more than one character long, only grab the
        // first character.
        aStr = aStr.Copy(0, 1);
        pEdit->SetText(aStr);
    }

    if ((!IsValidSeparator(aStr) || !IsValidSeparatorSet()) && !maOldSepValue.isEmpty())
        // Invalid separator.  Restore the old value.
        pEdit->SetText(maOldSepValue);

    OnFocusSeparatorInput(pEdit);
    return 0;
}

IMPL_LINK( ScTpFormulaOptions, SepEditOnFocusHdl, Edit*, pEdit )
{
    OnFocusSeparatorInput(pEdit);
    return 0;
}

SfxTabPage* ScTpFormulaOptions::Create(Window* pParent, const SfxItemSet& rCoreSet)
{
    return new ScTpFormulaOptions(pParent, rCoreSet);
}

sal_Bool ScTpFormulaOptions::FillItemSet(SfxItemSet& rCoreSet)
{
    bool bRet = false;
    ScFormulaOptions aOpt;
    sal_Bool bEnglishFuncName = maCbEnglishFuncName.IsChecked();
    sal_Int16 aSyntaxPos      = maLbFormulaSyntax.GetSelectEntryPos();
    OUString aSep             = maEdSepFuncArg.GetText();
    OUString aSepArrayCol     = maEdSepArrayCol.GetText();
    OUString aSepArrayRow     = maEdSepArrayRow.GetText();
    sal_Int16 nOOXMLRecalcMode = maLbOOXMLRecalcOptions.GetSelectEntryPos();
    sal_Int16 nODFRecalcMode = maLbODFRecalcOptions.GetSelectEntryPos();

    if (maBtnCustomCalcDefault.IsChecked())
    {
        // When Default is selected, reset all the calc config settings to default.
        maCurrentConfig.reset();
    }

    if ( maLbFormulaSyntax.GetSavedValue() != aSyntaxPos
         || maCbEnglishFuncName.GetSavedValue() != bEnglishFuncName
         || static_cast<OUString>(maEdSepFuncArg.GetSavedValue()) != aSep
         || static_cast<OUString>(maEdSepArrayCol.GetSavedValue()) != aSepArrayCol
         || static_cast<OUString>(maEdSepArrayRow.GetSavedValue()) != aSepArrayRow
         || maLbOOXMLRecalcOptions.GetSavedValue() != nOOXMLRecalcMode
         || maLbODFRecalcOptions.GetSavedValue() != nODFRecalcMode
         || maSavedConfig != maCurrentConfig )
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

        rCoreSet.Put( ScTpFormulaItem( SID_SCFORMULAOPTIONS, aOpt ) );
        bRet = true;
    }
    return bRet;
}

void ScTpFormulaOptions::Reset(const SfxItemSet& rCoreSet)
{
    ScFormulaOptions aOpt;
    const SfxPoolItem* pItem = NULL;

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCFORMULAOPTIONS, false , &pItem))
        aOpt = ((const ScTpFormulaItem*)pItem)->GetFormulaOptions();

    // formula grammar.
    ::formula::FormulaGrammar::Grammar eGram = aOpt.GetFormulaSyntax();

    switch (eGram)
    {
    case ::formula::FormulaGrammar::GRAM_NATIVE:
        maLbFormulaSyntax.SelectEntryPos(0);
        break;
    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_A1:
        maLbFormulaSyntax.SelectEntryPos(1);
        break;
    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1:
        maLbFormulaSyntax.SelectEntryPos(2);
        break;
    default:
        maLbFormulaSyntax.SelectEntryPos(0);
    }

    maLbFormulaSyntax.SaveValue();

    ScRecalcOptions eOOXMLRecalc = aOpt.GetOOXMLRecalcOptions();
    maLbOOXMLRecalcOptions.SelectEntryPos(static_cast<sal_uInt16>(eOOXMLRecalc));
    maLbOOXMLRecalcOptions.SaveValue();

    ScRecalcOptions eODFRecalc = aOpt.GetODFRecalcOptions();
    maLbODFRecalcOptions.SelectEntryPos(static_cast<sal_uInt16>(eODFRecalc));
    maLbODFRecalcOptions.SaveValue();

    // english function name.
    maCbEnglishFuncName.Check( aOpt.GetUseEnglishFuncName() );
    maCbEnglishFuncName.SaveValue();

    // Separators
    OUString aSep = aOpt.GetFormulaSepArg();
    OUString aSepArrayRow = aOpt.GetFormulaSepArrayRow();
    OUString aSepArrayCol = aOpt.GetFormulaSepArrayCol();

    if (aSep.getLength() == 1 && aSepArrayRow.getLength() == 1 && aSepArrayCol.getLength() == 1)
    {
        // Each separator must be one character long.
        maEdSepFuncArg.SetText(aSep);
        maEdSepArrayCol.SetText(aSepArrayCol);
        maEdSepArrayRow.SetText(aSepArrayRow);

        maEdSepFuncArg.SaveValue();
        maEdSepArrayCol.SaveValue();
        maEdSepArrayRow.SaveValue();
    }
    else
        ResetSeparators();

    // detailed calc settings.
    ScFormulaOptions aDefaults;

    maSavedConfig = aOpt.GetCalcConfig();
    bool bDefault = aDefaults.GetCalcConfig() == maSavedConfig;
    UpdateCustomCalcRadioButtons(bDefault);

    maCurrentConfig = maSavedConfig;
}

int ScTpFormulaOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    // What's this method for ?
    return KEEP_PAGE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
