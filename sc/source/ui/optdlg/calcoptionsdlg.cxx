/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "calcoptionsdlg.hxx"
#include "sc.hrc"
#include "scresid.hxx"

#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"

#if HAVE_FEATURE_OPENCL
#include "platforminfo.hxx"
#endif

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

ScCalcOptionsDialog::ScCalcOptionsDialog(Window* pParent, const ScCalcConfig& rConfig)
    : ModalDialog(pParent, "FormulaCalculationOptions",
        "modules/scalc/ui/formulacalculationoptions.ui")
    , maCalcA1(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1).toString())
    , maExcelA1(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1).toString())
    , maExcelR1C1(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1).toString())
    , maConfig(rConfig)
{
    get(mpLbSettings, "settings");
    get(mpLbOptionEdit, "edit");
    get(mpFtAnnotation, "annotation");
    get(mpBtnTrue, "true");
    get(mpBtnFalse, "false");
    get(mpOpenclInfoList, "opencl_list");
    get(mpBtnAutomaticSelectionTrue, "automatic_select_true");
    get(mpBtnAutomaticSelectionFalse, "automatic_select_false");

    mpOpenclInfoList->set_height_request(4* mpOpenclInfoList->GetTextHeight());
    mpOpenclInfoList->SetStyle(mpOpenclInfoList->GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    mpOpenclInfoList->SetHighlightRange();
    mpOpenclInfoList->GetParent()->Hide();

    mpBtnAutomaticSelectionTrue->SetToggleHdl(LINK(this, ScCalcOptionsDialog, BtnAutomaticSelectHdl));

    maCaptionStringRefSyntax = get<Window>("ref_syntax_caption")->GetText();
    maDescStringRefSyntax = get<Window>("ref_syntax_desc")->GetText();
    maUseFormulaSyntax = get<Window>("use_formula_syntax")->GetText();
    maCaptionEmptyStringAsZero = get<Window>("empty_str_as_zero_caption")->GetText();
    maDescEmptyStringAsZero = get<Window>("empty_str_as_zero_desc")->GetText();
    maCaptionOpenCLEnabled = get<Window>("opencl_enabled")->GetText();
    maDescOpenCLEnabled = get<Window>("opencl_enabled_desc")->GetText();

    mpLbSettings->set_height_request(8 * mpLbSettings->GetTextHeight());
    mpLbSettings->SetStyle(mpLbSettings->GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    mpLbSettings->SetHighlightRange();

    Link aLink = LINK(this, ScCalcOptionsDialog, SettingsSelHdl);
    mpLbSettings->SetSelectHdl(aLink);
    mpLbOptionEdit->SetSelectHdl(aLink);

    aLink = LINK(this, ScCalcOptionsDialog, BtnToggleHdl);
    mpBtnTrue->SetToggleHdl(aLink); // Set handler only to the 'True' button.

    maTrue = mpBtnTrue->GetText();
    maFalse = mpBtnFalse->GetText();

    FillOptionsList();
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
    SvTreeList *pModel = mpLbSettings->GetModel();
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

#if HAVE_FEATURE_OPENCL

void ScCalcOptionsDialog::fillOpenclList()
{
    mpOpenclInfoList->SetUpdateMode(false);
    mpOpenclInfoList->Clear();
    std::vector<sc::OpenclPlatformInfo> aPlatformInfo = sc::listAllOpenclPlatforms();
    for(std::vector<sc::OpenclPlatformInfo>::const_iterator it = aPlatformInfo.begin(),
            itEnd = aPlatformInfo.end(); it != itEnd; ++it)
    {
        for(std::vector<sc::OpenclDeviceInfo>::const_iterator
                itr = it->maDevices.begin(), itrEnd = it->maDevices.end(); itr != itrEnd; ++itr)
        {
            mpOpenclInfoList->InsertEntry(it->maVendor + " " + itr->maName);
        }
    }

    mpOpenclInfoList->SetUpdateMode(true);
}

#endif

void ScCalcOptionsDialog::FillOptionsList()
{
    mpLbSettings->SetUpdateMode(false);
    mpLbSettings->Clear();

    SvTreeList* pModel = mpLbSettings->GetModel();

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
#if HAVE_FEATURE_OPENCL
    pModel->Insert(createBoolItem(maCaptionOpenCLEnabled,maConfig.mbOpenCLEnabled));
    fillOpenclList();
#endif

    mpLbSettings->SetUpdateMode(true);
}

void ScCalcOptionsDialog::SelectionChanged()
{
    sal_uInt16 nSelectedPos = mpLbSettings->GetSelectEntryPos();
    switch ((CalcOptionOrder)nSelectedPos)
    {
        case CALC_OPTION_REF_SYNTAX:
        {
            // Formula syntax for INDIRECT function.
            mpBtnTrue->Hide();
            mpBtnFalse->Hide();
            mpLbOptionEdit->Show();
            mpOpenclInfoList->GetParent()->Hide();

            mpLbOptionEdit->Clear();
            mpLbOptionEdit->InsertEntry(maUseFormulaSyntax);
            mpLbOptionEdit->InsertEntry(maCalcA1);
            mpLbOptionEdit->InsertEntry(maExcelA1);
            mpLbOptionEdit->InsertEntry(maExcelR1C1);
            switch (maConfig.meStringRefAddressSyntax)
            {
                case formula::FormulaGrammar::CONV_OOO:
                    mpLbOptionEdit->SelectEntryPos(1);
                break;
                case formula::FormulaGrammar::CONV_XL_A1:
                    mpLbOptionEdit->SelectEntryPos(2);
                break;
                case formula::FormulaGrammar::CONV_XL_R1C1:
                    mpLbOptionEdit->SelectEntryPos(3);
                break;
                case formula::FormulaGrammar::CONV_UNSPECIFIED:
                default:
                    mpLbOptionEdit->SelectEntryPos(0);
            }
            mpFtAnnotation->SetText(maDescStringRefSyntax);
        }
        break;

        // booleans
        case CALC_OPTION_EMPTY_AS_ZERO:
        case CALC_OPTION_ENABLE_OPENCL:
        {
            // Treat empty string as zero.
            mpLbOptionEdit->Hide();
            mpBtnTrue->Show();
            mpBtnFalse->Show();

            bool bValue = false;
            if ( nSelectedPos == CALC_OPTION_EMPTY_AS_ZERO )
            {
                bValue = maConfig.mbEmptyStringAsZero;
                mpFtAnnotation->SetText(maDescEmptyStringAsZero);
                mpOpenclInfoList->GetParent()->Hide();
            }
            else
            {
                bValue = maConfig.mbOpenCLEnabled;
                mpFtAnnotation->SetText(maDescOpenCLEnabled);
                mpOpenclInfoList->GetParent()->Show();
                if(bValue)
                    mpOpenclInfoList->GetParent()->Enable();
                else
                    mpOpenclInfoList->GetParent()->Disable();

                OpenclAutomaticSelectionChanged();
            }

            if ( bValue )
            {
                mpBtnTrue->Check(true);
                mpBtnFalse->Check(false);
            }
            else
            {
                mpBtnTrue->Check(false);
                mpBtnFalse->Check(true);
            }
        }
        break;
        default:
            ;
    }
}

void ScCalcOptionsDialog::ListOptionValueChanged()
{
    sal_uInt16 nSelected = mpLbSettings->GetSelectEntryPos();
    switch ((CalcOptionOrder) nSelected)
    {
        case CALC_OPTION_REF_SYNTAX:
        {
            // Formula syntax for INDIRECT function.
            sal_uInt16 nPos = mpLbOptionEdit->GetSelectEntryPos();
            maConfig.meStringRefAddressSyntax = toAddressConvention(nPos);

            setValueAt(nSelected, toString(maConfig.meStringRefAddressSyntax));
        }
        break;

        case CALC_OPTION_EMPTY_AS_ZERO:
        case CALC_OPTION_ENABLE_OPENCL:
            break;
    }
}

void ScCalcOptionsDialog::OpenclAutomaticSelectionChanged()
{
    bool bValue = mpBtnAutomaticSelectionTrue->IsChecked();
    if(bValue)
        mpOpenclInfoList->Disable();
    else
        mpOpenclInfoList->Enable();
}

void ScCalcOptionsDialog::RadioValueChanged()
{
    sal_uInt16 nSelected = mpLbSettings->GetSelectEntryPos();
    bool bValue = mpBtnTrue->IsChecked();
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
    if (pCtrl == mpLbSettings)
        SelectionChanged();
    else if (pCtrl == mpLbOptionEdit)
        ListOptionValueChanged();

    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, BtnToggleHdl)
{
    RadioValueChanged();
    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, BtnAutomaticSelectHdl)
{
    OpenclAutomaticSelectionChanged();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
