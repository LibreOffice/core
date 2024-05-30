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
#include <officecfg/Office/Calc.hxx>
#include "calcoptionsdlg.hxx"

#include <unotools/localedatawrapper.hxx>

ScTpFormulaOptions::ScTpFormulaOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/optformula.ui"_ustr, u"OptFormula"_ustr, &rCoreAttrs)
    , mnDecSep(0)
    , mxLbFormulaSyntax(m_xBuilder->weld_combo_box(u"formulasyntax"_ustr))
    , mxCbEnglishFuncName(m_xBuilder->weld_check_button(u"englishfuncname"_ustr))
    , mxBtnCustomCalcDefault(m_xBuilder->weld_radio_button(u"calcdefault"_ustr))
    , mxBtnCustomCalcCustom(m_xBuilder->weld_radio_button(u"calccustom"_ustr))
    , mxBtnCustomCalcDetails(m_xBuilder->weld_button(u"details"_ustr))
    , mxEdSepFuncArg(m_xBuilder->weld_entry(u"function"_ustr))
    , mxEdSepArrayCol(m_xBuilder->weld_entry(u"arraycolumn"_ustr))
    , mxEdSepArrayRow(m_xBuilder->weld_entry(u"arrayrow"_ustr))
    , mxBtnSepReset(m_xBuilder->weld_button(u"reset"_ustr))
    , mxLbOOXMLRecalcOptions(m_xBuilder->weld_combo_box(u"ooxmlrecalc"_ustr))
    , mxLbODFRecalcOptions(m_xBuilder->weld_combo_box(u"odfrecalc"_ustr))
    , mxLbRowHeightReCalcOptions(m_xBuilder->weld_combo_box(u"rowheightrecalc"_ustr))
{
    mxLbFormulaSyntax->append_text(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1));
    mxLbFormulaSyntax->append_text(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1));
    mxLbFormulaSyntax->append_text(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1));

    Link<weld::Button&,void> aLink2 = LINK( this, ScTpFormulaOptions, ButtonHdl );
    mxBtnSepReset->connect_clicked(aLink2);
    mxBtnCustomCalcDetails->connect_clicked(aLink2);

    Link<weld::Toggleable&,void> aToggleLink = LINK( this, ScTpFormulaOptions, ToggleHdl );
    mxBtnCustomCalcDefault->connect_toggled(aToggleLink);
    mxBtnCustomCalcCustom->connect_toggled(aToggleLink);

    mxEdSepFuncArg->connect_insert_text(LINK( this, ScTpFormulaOptions, SepInsertTextHdl ));
    mxEdSepArrayCol->connect_insert_text(LINK( this, ScTpFormulaOptions, ColSepInsertTextHdl ));
    mxEdSepArrayRow->connect_insert_text(LINK( this, ScTpFormulaOptions, RowSepInsertTextHdl ));

    Link<weld::Entry&,void> aLink = LINK( this, ScTpFormulaOptions, SepModifyHdl );
    mxEdSepFuncArg->connect_changed(aLink);
    mxEdSepArrayCol->connect_changed(aLink);
    mxEdSepArrayRow->connect_changed(aLink);

    Link<weld::Widget&,void> aLink3 = LINK( this, ScTpFormulaOptions, SepEditOnFocusHdl );
    mxEdSepFuncArg->connect_focus_in(aLink3);
    mxEdSepArrayCol->connect_focus_in(aLink3);
    mxEdSepArrayRow->connect_focus_in(aLink3);

    // Get the decimal separator for current locale.
    OUString aSep = ScGlobal::getLocaleData().getNumDecimalSep();
    mnDecSep = aSep.isEmpty() ? u'.' : aSep[0];

    maSavedDocOptions = rCoreAttrs.Get(SID_SCDOCOPTIONS).GetDocOptions();
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
    OUString sSepValue = pEdit->get_text();
    if (!sSepValue.isEmpty())
        maOldSepValue = sSepValue;
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

bool ScTpFormulaOptions::IsValidSeparator(std::u16string_view aSep, bool bArray) const
{
    if (aSep.size() != 1)
        // Must be one-character long.
        return false;

    const sal_Unicode c = aSep[0];

    if (c == mnDecSep)
        // decimal separator is not allowed.
        return false;

    if (c <= 0x20 || c == 0x7f)
        // Disallow non-printables including space and DEL.
        return false;

    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
        // Disallow alphanumeric.
        return false;

    if (bArray)
    {
        switch (c)
        {
            case '+':
            case '-':
            case '{':
            case '}':
            case '"':
                // All following just to prevent confusion, they are not
                // evaluated in inline arrays and theoretically would be
                // possible.
            case '%':
            case '/':
            case '*':
            case '=':
            case '<':
            case '>':
            case '[':
            case ']':
            case '(':
            case ')':
            case '\'':
                // Disallowed characters.  Anything else we want to disallow ?
                return false;
        }
    }
    else if (c <= 0x7f)
    {
        switch (c)
        {
            default:
                // Anything bad except the knowns.
                return false;
            case ';':
            case ',':
                ; // nothing
        }
    }
    else
    {
        // Any Unicode character, would have to ask the compiler's localized
        // symbol map whether it's a known symbol but not a separator
        // (ocSep,ocArrayRowSep,ocArrayColSep), which we're about to set here.
        // But really..
        return false;
    }

    return true;
}

IMPL_LINK( ScTpFormulaOptions, ButtonHdl, weld::Button&, rBtn, void )
{
    if (&rBtn == mxBtnSepReset.get())
        ResetSeparators();
    else if (&rBtn == mxBtnCustomCalcDetails.get())
        LaunchCustomCalcSettings();
}

IMPL_LINK( ScTpFormulaOptions, ToggleHdl, weld::Toggleable&, rBtn, void )
{
    if (!rBtn.get_active())
        return;
    if (mxBtnCustomCalcDefault->get_active())
        UpdateCustomCalcRadioButtons(true);
    else if (mxBtnCustomCalcCustom->get_active())
        UpdateCustomCalcRadioButtons(false);
}

IMPL_LINK(ScTpFormulaOptions, SepInsertTextHdl, OUString&, rTest, bool)
{
    if (!IsValidSeparator(rTest, false) && !maOldSepValue.isEmpty())
        // Invalid separator.  Restore the old value.
        rTest = maOldSepValue;
    return true;
}

IMPL_LINK(ScTpFormulaOptions, RowSepInsertTextHdl, OUString&, rTest, bool)
{
    // Invalid separator or same as ColStr - Restore the old value.
    if ((!IsValidSeparator(rTest, true) || rTest == mxEdSepArrayCol->get_text()) && !maOldSepValue.isEmpty())
        rTest = maOldSepValue;
    return true;
}

IMPL_LINK(ScTpFormulaOptions, ColSepInsertTextHdl, OUString&, rTest, bool)
{
    // Invalid separator or same as RowStr - Restore the old value.
    if ((!IsValidSeparator(rTest, true) || rTest == mxEdSepArrayRow->get_text()) && !maOldSepValue.isEmpty())
        rTest = maOldSepValue;
    return true;
}

IMPL_LINK( ScTpFormulaOptions, SepModifyHdl, weld::Entry&, rEdit, void )
{
    OnFocusSeparatorInput(&rEdit);
}

IMPL_LINK( ScTpFormulaOptions, SepEditOnFocusHdl, weld::Widget&, rControl, void )
{
    OnFocusSeparatorInput(dynamic_cast<weld::Entry*>(&rControl));
}

std::unique_ptr<SfxTabPage> ScTpFormulaOptions::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet)
{
    return std::make_unique<ScTpFormulaOptions>(pPage, pController, *rCoreSet);
}

OUString ScTpFormulaOptions::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"formulasyntaxlabel"_ustr,
                          u"label3"_ustr, u"label6"_ustr,
                          u"label7"_ustr, u"label8"_ustr,
                          u"label2"_ustr, u"label4"_ustr,
                          u"label9"_ustr, u"label10"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"calcdefault"_ustr, u"calccustom"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    OUString buttons[] = { u"reset"_ustr, u"details"_ustr };

    for (const auto& btn : buttons)
    {
        if (const auto& pString = m_xBuilder->weld_button(btn))
            sAllStrings += pString->get_label() + " ";
    }

    // check button
    sAllStrings += mxCbEnglishFuncName->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
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
    sal_Int16 nReCalcOptRowHeights = mxLbRowHeightReCalcOptions->get_active();

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
         || mxLbRowHeightReCalcOptions->get_saved_value() != mxLbRowHeightReCalcOptions->get_text(nReCalcOptRowHeights)
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
        ScRecalcOptions eReCalcOptRowHeights = static_cast<ScRecalcOptions>(nReCalcOptRowHeights);

        aOpt.SetFormulaSyntax(eGram);
        aOpt.SetUseEnglishFuncName(bEnglishFuncName);
        aOpt.SetFormulaSepArg(aSep);
        aOpt.SetFormulaSepArrayCol(aSepArrayCol);
        aOpt.SetFormulaSepArrayRow(aSepArrayRow);
        aOpt.SetCalcConfig(maCurrentConfig);
        aOpt.SetOOXMLRecalcOptions(eOOXMLRecalc);
        aOpt.SetODFRecalcOptions(eODFRecalc);
        aOpt.SetReCalcOptiRowHeights(eReCalcOptRowHeights);
        aOpt.SetWriteCalcConfig( maCurrentDocOptions.IsWriteCalcConfig());

        rCoreSet->Put( ScTpFormulaItem( std::move(aOpt) ) );
        rCoreSet->Put( ScTpCalcItem( SID_SCDOCOPTIONS, maCurrentDocOptions ) );

        bRet = true;
    }
    return bRet;
}

void ScTpFormulaOptions::Reset(const SfxItemSet* rCoreSet)
{
    ScFormulaOptions aOpt;
    if(const ScTpFormulaItem* pItem = rCoreSet->GetItemIfSet(SID_SCFORMULAOPTIONS, false))
        aOpt = pItem->GetFormulaOptions();

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
    mxLbFormulaSyntax->set_sensitive( !officecfg::Office::Calc::Formula::Syntax::Grammar::isReadOnly() );

    ScRecalcOptions eOOXMLRecalc = aOpt.GetOOXMLRecalcOptions();
    mxLbOOXMLRecalcOptions->set_active(static_cast<sal_uInt16>(eOOXMLRecalc));
    mxLbOOXMLRecalcOptions->save_value();
    mxLbOOXMLRecalcOptions->set_sensitive( !officecfg::Office::Calc::Formula::Load::OOXMLRecalcMode::isReadOnly() );

    ScRecalcOptions eODFRecalc = aOpt.GetODFRecalcOptions();
    mxLbODFRecalcOptions->set_active(static_cast<sal_uInt16>(eODFRecalc));
    mxLbODFRecalcOptions->save_value();
    mxLbODFRecalcOptions->set_sensitive( !officecfg::Office::Calc::Formula::Load::ODFRecalcMode::isReadOnly() );

    // Recalc optimal row heights at load
    ScRecalcOptions eReCalcOptRowHeights = aOpt.GetReCalcOptiRowHeights();
    mxLbRowHeightReCalcOptions->set_active(static_cast<sal_uInt16>(eReCalcOptRowHeights));
    mxLbRowHeightReCalcOptions->save_value();
    mxLbRowHeightReCalcOptions->set_sensitive(
        !officecfg::Office::Calc::Formula::Load::RecalcOptimalRowHeightMode::isReadOnly() );

    // english function name.
    mxCbEnglishFuncName->set_active( aOpt.GetUseEnglishFuncName() );
    mxCbEnglishFuncName->save_state();
    mxCbEnglishFuncName->set_sensitive( !officecfg::Office::Calc::Formula::Syntax::EnglishFunctionName::isReadOnly() );

    // Separators
    OUString aSep = aOpt.GetFormulaSepArg();
    OUString aSepArrayRow = aOpt.GetFormulaSepArrayRow();
    OUString aSepArrayCol = aOpt.GetFormulaSepArrayCol();

    if (IsValidSeparator(aSep, false) && IsValidSeparator(aSepArrayRow, true) && IsValidSeparator(aSepArrayCol, true))
    {
        // Each and all separators must be valid.
        mxEdSepFuncArg->set_text(aSep);
        mxEdSepArrayCol->set_text(aSepArrayCol);
        mxEdSepArrayRow->set_text(aSepArrayRow);

        mxEdSepFuncArg->save_value();
        mxEdSepArrayCol->save_value();
        mxEdSepArrayRow->save_value();
    }
    else
        ResetSeparators();

    mxEdSepFuncArg->set_sensitive( !officecfg::Office::Calc::Formula::Syntax::SeparatorArg::isReadOnly() );
    mxEdSepArrayCol->set_sensitive( !officecfg::Office::Calc::Formula::Syntax::SeparatorArrayCol::isReadOnly() );
    mxEdSepArrayRow->set_sensitive( !officecfg::Office::Calc::Formula::Syntax::SeparatorArrayRow::isReadOnly() );
    mxBtnSepReset->set_sensitive ( !officecfg::Office::Calc::Formula::Syntax::SeparatorArg::isReadOnly()  &&
                            !officecfg::Office::Calc::Formula::Syntax::SeparatorArrayCol::isReadOnly() &&
                            !officecfg::Office::Calc::Formula::Syntax::SeparatorArrayRow::isReadOnly() );

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
