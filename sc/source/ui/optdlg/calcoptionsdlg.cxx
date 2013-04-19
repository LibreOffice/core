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
 */

#include "calcoptionsdlg.hxx"
#include "calcoptionsdlg.hrc"
#include "scresid.hxx"

#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"

namespace {

class OptionString : public SvLBoxString
{
    OUString maDesc;
    OUString maValue;
public:
    OptionString(const OUString& rDesc, const OUString& rValue) :
        maDesc(rDesc), maValue(rValue) {}

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
};

void OptionString::Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/, const SvTreeListEntry* /*pEntry*/)
{
    Point aPos = rPos;
    OUString aDesc = maDesc + OUString(": ");
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
    maBtnTrue(this, ScResId(BTN_OPTION_TRUE)),
    maBtnFalse(this, ScResId(BTN_OPTION_FALSE)),
    maFlAnnotation(this, ScResId(FL_ANNOTATION)),
    maFtAnnotation(this, ScResId(FT_ANNOTATION)),
    maBtnOK(this, ScResId(BTN_OK)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maTrue(ScResId(STR_TRUE).toString()),
    maFalse(ScResId(STR_FALSE).toString()),
    maCalcA1(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1).toString()),
    maExcelA1(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1).toString()),
    maExcelR1C1(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1).toString()),
    maCaptionStringRefSyntax(ScResId(STR_STRING_REF_SYNTAX_CAPTION).toString()),
    maDescStringRefSyntax(ScResId(STR_STRING_REF_SYNTAX_DESC).toString()),
    maUseFormulaSyntax(ScResId(STR_USE_FORMULA_SYNTAX).toString()),
    maCaptionEmptyStringAsZero(ScResId(STR_EMPTY_STRING_AS_ZERO_CAPTION).toString()),
    maDescEmptyStringAsZero(ScResId(STR_EMPTY_STRING_AS_ZERO_DESC).toString()),
    maConfig(rConfig)
{
    maLbSettings.SetStyle(maLbSettings.GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    maLbSettings.SetHighlightRange();

    Link aLink = LINK(this, ScCalcOptionsDialog, SettingsSelHdl);
    maLbSettings.SetSelectHdl(aLink);
    maLbOptionEdit.SetSelectHdl(aLink);

    aLink = LINK(this, ScCalcOptionsDialog, BtnToggleHdl);
    maBtnTrue.SetToggleHdl(aLink); // Set handler only to the 'True' button.

    maBtnTrue.SetText(maTrue);
    maBtnFalse.SetText(maFalse);

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

    SvTreeList* pModel = maLbSettings.GetModel();

    {
        // Syntax for INDIRECT function.
        SvTreeListEntry* pEntry = new SvTreeListEntry;
        pEntry->AddItem(new SvLBoxString(pEntry, 0, OUString()));
        pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), 0));
        OptionString* pItem = new OptionString(
            maCaptionStringRefSyntax, toString(maConfig.meStringRefAddressSyntax));
        pEntry->AddItem(pItem);
        pModel->Insert(pEntry);
    }

    {
        // Treat empty string as zero.
        SvTreeListEntry* pEntry = new SvTreeListEntry;
        pEntry->AddItem(new SvLBoxString(pEntry, 0, OUString()));
        pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), 0));
        OptionString* pItem = new OptionString(
            maCaptionEmptyStringAsZero, toString(maConfig.mbEmptyStringAsZero));
        pEntry->AddItem(pItem);
        pModel->Insert(pEntry);
    }

    maLbSettings.SetUpdateMode(true);
}

void ScCalcOptionsDialog::SelectionChanged()
{
    sal_uInt16 nSelectedPos = maLbSettings.GetSelectEntryPos();
    switch (nSelectedPos)
    {
        case 0:
        {
            // Formula syntax for INDIRECT function.
            maBtnTrue.Hide();
            maBtnFalse.Hide();
            maLbOptionEdit.Show();

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
        break;
        case 1:
        {
            // Treat empty string as zero.
            maLbOptionEdit.Hide();
            maBtnTrue.Show();
            maBtnFalse.Show();

            if (maConfig.mbEmptyStringAsZero)
            {
                maBtnTrue.Check(true);
                maBtnFalse.Check(false);
            }
            else
            {
                maBtnTrue.Check(false);
                maBtnFalse.Check(true);
            }
            maFtAnnotation.SetText(maDescEmptyStringAsZero);
        }
        break;
        default:
            ;
    }
}

void ScCalcOptionsDialog::ListOptionValueChanged()
{
    sal_uInt16 nSelected = maLbSettings.GetSelectEntryPos();
    switch (nSelected)
    {
        case 0:
        {
            // Formula syntax for INDIRECT function.
            sal_uInt16 nPos = maLbOptionEdit.GetSelectEntryPos();
            maConfig.meStringRefAddressSyntax = toAddressConvention(nPos);

            maLbSettings.SetUpdateMode(false);

            SvTreeList* pModel = maLbSettings.GetModel();
            SvTreeListEntry* pEntry = pModel->GetEntry(NULL, 0);
            if (!pEntry)
                return;

            OptionString* pItem = new OptionString(
                maCaptionStringRefSyntax, toString(maConfig.meStringRefAddressSyntax));
            pEntry->ReplaceItem(pItem, 2);

            maLbSettings.SetUpdateMode(true);
        }
        break;
        default:
            ;
    }
}

void ScCalcOptionsDialog::RadioValueChanged()
{
    sal_uInt16 nSelected = maLbSettings.GetSelectEntryPos();
    switch (nSelected)
    {
        case 1:
        {
            // Treat empty string as zero.
            maConfig.mbEmptyStringAsZero = maBtnTrue.IsChecked();
            maLbSettings.SetUpdateMode(false);

            SvTreeList* pModel = maLbSettings.GetModel();
            SvTreeListEntry* pEntry = pModel->GetEntry(NULL, 1);
            if (!pEntry)
                return;

            OptionString* pItem = new OptionString(
                maCaptionEmptyStringAsZero, toString(maConfig.mbEmptyStringAsZero));
            pEntry->ReplaceItem(pItem, 2);

            maLbSettings.SetUpdateMode(true);
        }
        break;
        default:
            ;
    }
}

OUString ScCalcOptionsDialog::toString(formula::FormulaGrammar::AddressConvention eConv) const
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

OUString ScCalcOptionsDialog::toString(bool bVal) const
{
    return bVal ? maTrue : maFalse;
}

IMPL_LINK(ScCalcOptionsDialog, SettingsSelHdl, Control*, pCtrl)
{
    if (pCtrl == &maLbSettings)
        SelectionChanged();
    else if (pCtrl == &maLbOptionEdit)
        ListOptionValueChanged();

    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, BtnToggleHdl)
{
    RadioValueChanged();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
