/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "calcoptionsdlg.hxx"
#include "calcoptionsdlg.hrc"
#include "scresid.hxx"

#include "svtools/svlbitm.hxx"

namespace {

class OptionString : public SvLBoxString
{
    rtl::OUString maDesc;
    rtl::OUString maValue;
public:
    OptionString(const rtl::OUString& rDesc, const rtl::OUString& rValue) :
        maDesc(rDesc), maValue(rValue) {}

    virtual void Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
};

void OptionString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 /*nFlags*/, SvLBoxEntry* /*pEntry*/)
{
    Point aPos = rPos;
    rtl::OUString aDesc = maDesc + rtl::OUString(": ");
    rDev.DrawText(aPos, aDesc);

    aPos.X() += rDev.GetTextWidth(aDesc);
    Font aOldFont = rDev.GetFont();
    Font aFont = aOldFont;
    aFont.SetWeight(WEIGHT_BOLD);

    rDev.SetFont(aFont);
    rDev.DrawText(aPos, maValue);

    rDev.SetFont(aOldFont);
}

formula::FormulaGrammar::AddressConvention toAddressConvention(sal_uInt16 nPos)
{
    switch (nPos)
    {
        case 1:
            return formula::FormulaGrammar::CONV_OOO;
        case 2:
            return formula::FormulaGrammar::CONV_XL_A1;
        case 3:
            return formula::FormulaGrammar::CONV_XL_R1C1;
        case 0:
        default:
            ;
    }

    return formula::FormulaGrammar::CONV_UNSPECIFIED;
}

}

ScCalcOptionsDialog::ScCalcOptionsDialog(Window* pParent, const ScCalcConfig& rConfig) :
    ModalDialog(pParent, ScResId(RID_SCDLG_FORMULA_CALCOPTIONS)),
    maLbSettings(this, ScResId(LB_SETTINGS)),
    maFtOptionEditCaption(this, ScResId(FT_OPTION_EDIT_CAPTION)),
    maLbOptionEdit(this, ScResId(LB_OPTION_EDIT)),
    maFlAnnotation(this, ScResId(FL_ANNOTATION)),
    maFtAnnotation(this, ScResId(FT_ANNOTATION)),
    maBtnOK(this, ScResId(BTN_OK)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maCalcA1(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1).toString()),
    maExcelA1(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1).toString()),
    maExcelR1C1(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1).toString()),
    maCaptionStringRefSyntax(ScResId(STR_STRING_REF_SYNTAX_CAPTION).toString()),
    maDescStringRefSyntax(ScResId(STR_STRING_REF_SYNTAX_DESC).toString()),
    maUseFormulaSyntax(ScResId(STR_USE_FORMULA_SYNTAX).toString()),
    maConfig(rConfig)
{
    maLbSettings.SetStyle(maLbSettings.GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    maLbSettings.SetHighlightRange();

    Link aLink = LINK(this, ScCalcOptionsDialog, SettingsSelHdl);
    maLbSettings.SetSelectHdl(aLink);
    maLbOptionEdit.SetSelectHdl(aLink);

    FillOptionsList();
    FreeResource();
    SelectionChanged();
}

ScCalcOptionsDialog::~ScCalcOptionsDialog() {}

const ScCalcConfig& ScCalcOptionsDialog::GetConfig() const
{
    return maConfig;
}

void ScCalcOptionsDialog::FillOptionsList()
{
    maLbSettings.SetUpdateMode(false);
    maLbSettings.Clear();

    SvLBoxTreeList* pModel = maLbSettings.GetModel();

    {
        // Syntax for INDIRECT function.
        SvLBoxEntry* pEntry = new SvLBoxEntry;
        pEntry->AddItem(new SvLBoxString(pEntry, 0, rtl::OUString()));
        pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), 0));
        OptionString* pItem = new OptionString(
            maCaptionStringRefSyntax, toString(maConfig.meStringRefAddressSyntax));
        pEntry->AddItem(pItem);
        pModel->Insert(pEntry);
    }

    maLbSettings.SetUpdateMode(true);
}

void ScCalcOptionsDialog::SelectionChanged()
{
    if (true)
    {
        // Formula syntax for INDIRECT function.
        maLbOptionEdit.Clear();
        maLbOptionEdit.InsertEntry(maUseFormulaSyntax);
        maLbOptionEdit.InsertEntry(maCalcA1);
        maLbOptionEdit.InsertEntry(maExcelA1);
        maLbOptionEdit.InsertEntry(maExcelR1C1);
        switch (maConfig.meStringRefAddressSyntax)
        {
            case formula::FormulaGrammar::CONV_OOO:
                maLbOptionEdit.SelectEntryPos(1);
            break;
            case formula::FormulaGrammar::CONV_XL_A1:
                maLbOptionEdit.SelectEntryPos(2);
            break;
            case formula::FormulaGrammar::CONV_XL_R1C1:
                maLbOptionEdit.SelectEntryPos(3);
            break;
            case formula::FormulaGrammar::CONV_UNSPECIFIED:
            default:
                maLbOptionEdit.SelectEntryPos(0);
        }
        maFtAnnotation.SetText(maDescStringRefSyntax);
    }
}

void ScCalcOptionsDialog::ListOptionValueChanged()
{
    if (true)
    {
        // Formula syntax for INDIRECT function.
        sal_uInt16 nPos = maLbOptionEdit.GetSelectEntryPos();
        maConfig.meStringRefAddressSyntax = toAddressConvention(nPos);

        maLbSettings.SetUpdateMode(false);

        SvLBoxTreeList* pModel = maLbSettings.GetModel();
        SvLBoxEntry* pEntry = pModel->GetEntry(NULL, 0);
        if (!pEntry)
            return;

        OptionString* pItem = new OptionString(
            maCaptionStringRefSyntax, toString(maConfig.meStringRefAddressSyntax));
        pEntry->ReplaceItem(pItem, 2);

        maLbSettings.SetUpdateMode(true);
    }
}

rtl::OUString ScCalcOptionsDialog::toString(formula::FormulaGrammar::AddressConvention eConv) const
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            return maCalcA1;
        case formula::FormulaGrammar::CONV_XL_A1:
            return maExcelA1;
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return maExcelR1C1;
        case formula::FormulaGrammar::CONV_UNSPECIFIED:
        default:
            ;
    }
    return maUseFormulaSyntax;
}

IMPL_LINK(ScCalcOptionsDialog, SettingsSelHdl, Control*, pCtrl)
{
    if (pCtrl == &maLbSettings)
        SelectionChanged();
    else if (pCtrl == &maLbOptionEdit)
        ListOptionValueChanged();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
