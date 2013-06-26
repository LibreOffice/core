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

typedef enum {
    CALC_OPTION_REF_SYNTAX    = 0,
    CALC_OPTION_EMPTY_AS_ZERO = 1,
    CALC_OPTION_ENABLE_OPENCL = 2
} CalcOptionOrder;

class OptionString : public SvLBoxString
{
    OUString maDesc;
    OUString maValue;
public:
    OptionString(const OUString& rDesc, const OUString& rValue) :
        maDesc(rDesc), maValue(rValue) {}

    void SetValue(const OUString &rValue) { maValue = rValue; }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);

    virtual void InitViewData(SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData);
};

void OptionString::InitViewData(
    SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    OUString aDesc = maDesc + OUString(": ");
    Size aDescSize(pView->GetTextWidth(aDesc), pView->GetTextHeight());

    Font aOldFont = pView->GetFont();
    Font aFont = aOldFont;
    aFont.SetWeight(WEIGHT_BOLD);
    //To not make the SvTreeListBox try and recalculate all rows, call the
    //underlying SetFont, we just want to know what size this text will be
    //and are going to reset the font to the original again afterwards
    pView->Control::SetFont(aFont);
    Size aValueSize(pView->GetTextWidth(maValue), pView->GetTextHeight());
    pView->Control::SetFont(aOldFont);

    pViewData->maSize = Size(aDescSize.Width() + aValueSize.Width(), std::max(aDescSize.Height(), aValueSize.Height()));
}

void OptionString::Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/, const SvTreeListEntry* /*pEntry*/)
{
    Point aPos = rPos;
    OUString aDesc = maDesc + OUString(": ");
    rDev.DrawText(aPos, aDesc);

    aPos.X() += rDev.GetTextWidth(aDesc);
    Font aOldFont = rDev.GetFont();
    Font aFont = aOldFont;
    aFont.SetWeight(WEIGHT_BOLD);

    //To not make the SvTreeListBox try and recalculate all rows, call the
    //underlying SetFont, we are going to draw this string and then going to
    //reset the font to the original again afterwards
    rDev.Control::SetFont(aFont);
    rDev.DrawText(aPos, maValue);
    rDev.Control::SetFont(aOldFont);
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
    maCaptionOpenCLEnabled(ScResId(STR_OPENCL_ENABLED).toString()),
    maDescOpenCLEnabled(ScResId(STR_OPENCL_ENABLED_DESC).toString()),
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

SvTreeListEntry *ScCalcOptionsDialog::createBoolItem(const OUString &rCaption, bool bValue) const
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->AddItem(new SvLBoxString(pEntry, 0, OUString()));
    pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), 0));
    OptionString* pItem = new OptionString(rCaption, toString(bValue));
    pEntry->AddItem(pItem);
    return pEntry;
}

void ScCalcOptionsDialog::setValueAt(size_t nPos, const OUString &rValue)
{
    SvTreeList *pModel = maLbSettings.GetModel();
    SvTreeListEntry* pEntry = pModel->GetEntry(NULL, nPos);
    if (!pEntry)
    {
        SAL_WARN("sc", "missing entry at " << nPos << " in value view");
        return;
    }
    OptionString* pOpt = dynamic_cast<OptionString *>(pEntry->GetItem(2));
    if (!pOpt)
    {
        SAL_WARN("sc", "missing option string item so can't set " << rValue);
        return;
    }

    pOpt->SetValue(rValue);
    pModel->InvalidateEntry(pEntry);
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

    pModel->Insert(createBoolItem(maCaptionEmptyStringAsZero,maConfig.mbEmptyStringAsZero));
    pModel->Insert(createBoolItem(maCaptionOpenCLEnabled,maConfig.mbOpenCLEnabled));

    maLbSettings.SetUpdateMode(true);
}

void ScCalcOptionsDialog::SelectionChanged()
{
    sal_uInt16 nSelectedPos = maLbSettings.GetSelectEntryPos();
    switch ((CalcOptionOrder)nSelectedPos)
    {
        case CALC_OPTION_REF_SYNTAX:
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

        // booleans
        case CALC_OPTION_EMPTY_AS_ZERO:
        case CALC_OPTION_ENABLE_OPENCL:
        {
            // Treat empty string as zero.
            maLbOptionEdit.Hide();
            maBtnTrue.Show();
            maBtnFalse.Show();

            bool bValue = false;
            if ( nSelectedPos == CALC_OPTION_EMPTY_AS_ZERO )
                bValue = maConfig.mbEmptyStringAsZero;
            else
                bValue = maConfig.mbOpenCLEnabled;

            if ( bValue )
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
    switch ((CalcOptionOrder) nSelected)
    {
        case CALC_OPTION_REF_SYNTAX:
        {
            // Formula syntax for INDIRECT function.
            sal_uInt16 nPos = maLbOptionEdit.GetSelectEntryPos();
            maConfig.meStringRefAddressSyntax = toAddressConvention(nPos);

            setValueAt(nSelected, toString(maConfig.meStringRefAddressSyntax));
        }
        break;

        case CALC_OPTION_EMPTY_AS_ZERO:
        case CALC_OPTION_ENABLE_OPENCL:
            break;
    }
}

void ScCalcOptionsDialog::RadioValueChanged()
{
    sal_uInt16 nSelected = maLbSettings.GetSelectEntryPos();
    bool bValue = maBtnTrue.IsChecked();
    switch (nSelected)
    {
        case CALC_OPTION_REF_SYNTAX:
            return;
        case CALC_OPTION_EMPTY_AS_ZERO:
            maConfig.mbEmptyStringAsZero = bValue;
            break;
        case CALC_OPTION_ENABLE_OPENCL:
            maConfig.mbOpenCLEnabled = bValue;
            break;
    }

    setValueAt(nSelected, toString(bValue));
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
