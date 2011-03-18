/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "tpformula.hxx"
#include "optdlg.hrc"
#include "scresid.hxx"
#include "formula/grammar.hxx"
#include "docoptio.hxx"
#include "global.hxx"

#include <unotools/localedatawrapper.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>

using ::rtl::OUString;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::i18n::LocaleDataItem;

ScTpFormulaOptions::ScTpFormulaOptions(Window* pParent, const SfxItemSet& rCoreAttrs) :
    SfxTabPage(pParent, ScResId(RID_SCPAGE_FORMULA), rCoreAttrs),

    maFlFormulaOpt(this, ScResId(FL_FORMULA_OPTIONS)),
    maFtFormulaSyntax(this, ScResId(FT_FORMULA_SYNTAX)),
    maLbFormulaSyntax(this, ScResId(LB_FORMULA_SYNTAX)),
    maCbEnglishFuncName(this, ScResId(CB_ENGLISH_FUNC_NAME)),
    maFlFormulaSeps(this, ScResId(FL_FORMULA_SEPS)),
    maFtSepFuncArg(this, ScResId(FT_FORMULA_SEP_ARG)),
    maEdSepFuncArg(this, ScResId(ED_FORMULA_SEP_ARG)),
    maFtSepArrayCol(this, ScResId(FT_FORMULA_SEP_ARRAY_C)),
    maEdSepArrayCol(this, ScResId(ED_FORMULA_SEP_ARRAY_C)),
    maFtSepArrayRow(this, ScResId(FT_FORMULA_SEP_ARRAY_R)),
    maEdSepArrayRow(this, ScResId(ED_FORMULA_SEP_ARRAY_R)),
    maBtnSepReset(this, ScResId(BTN_FORMULA_SEP_RESET)),

    mpOldOptions(NULL),
    mpNewOptions(NULL),
    mnDecSep(0)
{
    FreeResource();

    const ScTpCalcItem& rItem = static_cast<const ScTpCalcItem&>(
        rCoreAttrs.Get(GetWhich(SID_SCDOCOPTIONS)));
    mpOldOptions.reset(new ScDocOptions(rItem.GetDocOptions()));
    mpNewOptions.reset(new ScDocOptions(rItem.GetDocOptions()));

    Init();
}

ScTpFormulaOptions::~ScTpFormulaOptions()
{
}

void ScTpFormulaOptions::Init()
{
    Link aLink = LINK( this, ScTpFormulaOptions, ButtonHdl );
    maBtnSepReset.SetClickHdl(aLink);

    aLink = LINK( this, ScTpFormulaOptions, SepModifyHdl );
    maEdSepFuncArg.SetModifyHdl(aLink);
    maEdSepArrayCol.SetModifyHdl(aLink);
    maEdSepArrayRow.SetModifyHdl(aLink);

    aLink = LINK( this, ScTpFormulaOptions, SepEditOnFocusHdl );
    maEdSepFuncArg.SetGetFocusHdl(aLink);
    maEdSepArrayCol.SetGetFocusHdl(aLink);
    maEdSepArrayRow.SetGetFocusHdl(aLink);

    // Get the decimal separator for current locale.
    String aSep = mpOldOptions->GetLocaleDataWrapper().getNumDecimalSep();
    mnDecSep = aSep.Len() ? aSep.GetChar(0) : sal_Unicode('.');
}

void ScTpFormulaOptions::ResetSeparators()
{
    ScDocOptions aOpt;
    maEdSepFuncArg.SetText(aOpt.GetFormulaSepArg());
    maEdSepArrayCol.SetText(aOpt.GetFormulaSepArrayCol());
    maEdSepArrayRow.SetText(aOpt.GetFormulaSepArrayRow());
}

void ScTpFormulaOptions::OnFocusSeparatorInput(Edit* pEdit)
{
    if (!pEdit)
        return;

    // Make sure the entire text is selected.
    xub_StrLen nLen = pEdit->GetText().Len();
    Selection aSel(0, nLen);
    pEdit->SetSelection(aSel);
    maOldSepValue = pEdit->GetText();
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

IMPL_LINK( ScTpFormulaOptions, ButtonHdl, PushButton*, pBtn )
{
    if (pBtn == &maBtnSepReset)
        ResetSeparators();

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

    if ((!IsValidSeparator(aStr) || !IsValidSeparatorSet()) && maOldSepValue.getLength())
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
    ::formula::FormulaGrammar::Grammar eGram = ::formula::FormulaGrammar::GRAM_DEFAULT;
    switch (maLbFormulaSyntax.GetSelectEntryPos())
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

    mpNewOptions->SetFormulaSyntax(eGram);
    mpNewOptions->SetUseEnglishFuncName(maCbEnglishFuncName.IsChecked());
    mpNewOptions->SetFormulaSepArg(maEdSepFuncArg.GetText());
    mpNewOptions->SetFormulaSepArrayCol(maEdSepArrayCol.GetText());
    mpNewOptions->SetFormulaSepArrayRow(maEdSepArrayRow.GetText());

    if (*mpNewOptions != *mpOldOptions)
    {
        rCoreSet.Put(ScTpCalcItem(GetWhich(SID_SCDOCOPTIONS), *mpNewOptions));
        return true;
    }
    else
        return false;
}

void ScTpFormulaOptions::Reset(const SfxItemSet& /*rCoreSet*/)
{
    ::formula::FormulaGrammar::Grammar eGram = mpNewOptions->GetFormulaSyntax();
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

    maCbEnglishFuncName.Check(mpNewOptions->GetUseEnglishFuncName());

    OUString aSep = mpNewOptions->GetFormulaSepArg();
    OUString aSepArrayRow = mpNewOptions->GetFormulaSepArrayRow();
    OUString aSepArrayCol = mpNewOptions->GetFormulaSepArrayCol();

    if (aSep.getLength() == 1 && aSepArrayRow.getLength() == 1 && aSepArrayCol.getLength() == 1)
    {
        // Each separator must be one character long.
        maEdSepFuncArg.SetText(aSep);
        maEdSepArrayCol.SetText(aSepArrayCol);
        maEdSepArrayRow.SetText(aSepArrayRow);
    }
    else
        ResetSeparators();
}

int ScTpFormulaOptions::DeactivatePage(SfxItemSet* /*pSet*/)
{
    // What's this method for ?
    return KEEP_PAGE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
