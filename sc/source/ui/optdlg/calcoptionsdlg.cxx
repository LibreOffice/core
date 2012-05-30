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

rtl::OUString toString(formula::FormulaGrammar::AddressConvention eConv)
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            return rtl::OUString("Calc A1");
        case formula::FormulaGrammar::CONV_XL_A1:
            return rtl::OUString ("Excel A1");
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return rtl::OUString("Excel R1C1");
        case formula::FormulaGrammar::CONV_UNSPECIFIED:
        default:
            ;
    }
    return rtl::OUString("Use formula syntax");
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
    maCaptionIndirectSyntax(ScResId(STR_INDIRECT_SYNTAX_CAPTION).toString()),
    maDescIndirectSyntax(ScResId(STR_INDIRECT_SYNTAX_DESC).toString()),
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
            maCaptionIndirectSyntax, toString(maConfig.meIndirectRefSyntax));
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
        maLbOptionEdit.InsertEntry(rtl::OUString("Use formula syntax"));
        maLbOptionEdit.InsertEntry(rtl::OUString("Calc A1"));
        maLbOptionEdit.InsertEntry(rtl::OUString("Excel A1"));
        maLbOptionEdit.InsertEntry(rtl::OUString("Excel R1C1"));
        switch (maConfig.meIndirectRefSyntax)
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
        maFtAnnotation.SetText(maDescIndirectSyntax);
    }
}

void ScCalcOptionsDialog::ListOptionValueChanged()
{
    if (true)
    {
        // Formula syntax for INDIRECT function.
        sal_uInt16 nPos = maLbOptionEdit.GetSelectEntryPos();
        maConfig.meIndirectRefSyntax = toAddressConvention(nPos);
        FillOptionsList();
    }
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
