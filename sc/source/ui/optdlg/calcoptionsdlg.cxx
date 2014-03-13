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
#include "formulagroup.hxx"
#include "globalnames.hxx"
#endif

namespace {

typedef enum {
    CALC_OPTION_STRING_CONVERSION = 0,
    CALC_OPTION_EMPTY_AS_ZERO     = 1,
    CALC_OPTION_REF_SYNTAX        = 2,
    CALC_OPTION_ENABLE_OPENCL     = 3
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

    OUString aDesc = maDesc + ": ";
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
    OUString aDesc = maDesc + ": ";
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

formula::FormulaGrammar::AddressConvention toAddressConvention(sal_Int32 nPos)
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

ScCalcConfig::StringConversion toStringConversion(sal_Int32 nPos)
{
    switch (nPos)
    {
        case 0:
            return ScCalcConfig::STRING_CONVERSION_AS_ERROR;
        case 1:
            return ScCalcConfig::STRING_CONVERSION_AS_ZERO;
        case 2:
            return ScCalcConfig::STRING_CONVERSION_UNAMBIGUOUS;
        case 3:
            return ScCalcConfig::STRING_CONVERSION_LOCALE_DEPENDENT;
    }

    return ScCalcConfig::STRING_CONVERSION_AS_ERROR;
}

}

ScCalcOptionsDialog::ScCalcOptionsDialog(Window* pParent, const ScCalcConfig& rConfig)
    : ModalDialog(pParent, "FormulaCalculationOptions",
        "modules/scalc/ui/formulacalculationoptions.ui")
    , maCalcA1(ScResId(SCSTR_FORMULA_SYNTAX_CALC_A1).toString())
    , maExcelA1(ScResId(SCSTR_FORMULA_SYNTAX_XL_A1).toString())
    , maExcelR1C1(ScResId(SCSTR_FORMULA_SYNTAX_XL_R1C1).toString())
    , maConfig(rConfig)
    , mbSelectedEmptyStringAsZero(rConfig.mbEmptyStringAsZero)
{
    get(mpLbSettings, "settings");
    get(mpLbOptionEdit, "edit");
    get(mpFtAnnotation, "annotation");
    get(mpBtnTrue, "true");
    get(mpBtnFalse, "false");
    get(mpOpenclInfoList, "opencl_list");
    get(mpBtnAutomaticSelectionTrue, "automatic_select_true");
    get(mpBtnAutomaticSelectionFalse, "automatic_select_false");
    get(mpFtFrequency, "frequency");
    get(mpFtComputeUnits, "compute_units");
    get(mpFtMemory, "memory");

    mpOpenclInfoList->set_height_request(4* mpOpenclInfoList->GetTextHeight());
    mpOpenclInfoList->SetStyle(mpOpenclInfoList->GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    mpOpenclInfoList->SetHighlightRange();
    mpOpenclInfoList->GetParent()->Hide();
    mpOpenclInfoList->SetSelectHdl(LINK(this, ScCalcOptionsDialog, DeviceSelHdl));

    mpBtnAutomaticSelectionTrue->SetToggleHdl(LINK(this, ScCalcOptionsDialog, BtnAutomaticSelectHdl));

    maCaptionStringRefSyntax = get<Window>("ref_syntax_caption")->GetText();
    maDescStringRefSyntax = get<Window>("ref_syntax_desc")->GetText();
    maUseFormulaSyntax = get<Window>("use_formula_syntax")->GetText();

    maCaptionStringConversion = get<Window>("string_conversion_caption")->GetText();
    maDescStringConversion = get<Window>("string_conversion_desc")->GetText();
    maStringConversionAsError = get<Window>("string_conversion_as_error")->GetText();
    maStringConversionAsZero = get<Window>("string_conversion_as_zero")->GetText();
    maStringConversionUnambiguous = get<Window>("string_conversion_unambiguous")->GetText();
    maStringConversionLocaleDependent = get<Window>("string_conversion_locale_dependent")->GetText();

    maCaptionEmptyStringAsZero = get<Window>("empty_str_as_zero_caption")->GetText();
    maDescEmptyStringAsZero = get<Window>("empty_str_as_zero_desc")->GetText();

    maCaptionOpenCLEnabled = get<Window>("opencl_enabled")->GetText();
    maDescOpenCLEnabled = get<Window>("opencl_enabled_desc")->GetText();
    maSoftware = get<Window>("software")->GetText();

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
    pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), false));
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
    SvTreeListEntry* pSoftwareEntry = mpOpenclInfoList->InsertEntry(maSoftware);

    OUString aStoredDevice = maConfig.maOpenCLDevice;

    SvTreeListEntry* pSelectedEntry = NULL;

    sc::FormulaGroupInterpreter::fillOpenCLInfo(maPlatformInfo);
    for(std::vector<sc::OpenclPlatformInfo>::iterator it = maPlatformInfo.begin(),
            itEnd = maPlatformInfo.end(); it != itEnd; ++it)
    {
        for(std::vector<sc::OpenclDeviceInfo>::iterator
                itr = it->maDevices.begin(), itrEnd = it->maDevices.end(); itr != itrEnd; ++itr)
        {
            OUString aDeviceId = it->maVendor + " " + itr->maName;
            SvTreeListEntry* pEntry = mpOpenclInfoList->InsertEntry(aDeviceId);
            if(aDeviceId == aStoredDevice)
            {
                pSelectedEntry = pEntry;
            }
            pEntry->SetUserData(&(*itr));
        }
    }

    mpOpenclInfoList->SetUpdateMode(true);
    mpOpenclInfoList->GetModel()->GetView(0)->SelectAll(false, false);

    if (pSelectedEntry)
        mpOpenclInfoList->Select(pSelectedEntry);
    else if (aStoredDevice == OPENCL_SOFTWARE_DEVICE_CONFIG_NAME)
        mpOpenclInfoList->Select(pSoftwareEntry);

    SelectedDeviceChanged();
}

#endif


namespace {
void addOption( SvTreeList* pModel, OptionString* pItem )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->AddItem(new SvLBoxString(pEntry, 0, OUString()));
    pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), false));
    pEntry->AddItem(pItem);
    pModel->Insert(pEntry);
}
}

void ScCalcOptionsDialog::FillOptionsList()
{
    mpLbSettings->SetUpdateMode(false);
    mpLbSettings->Clear();

    SvTreeList* pModel = mpLbSettings->GetModel();

    {
        // String conversion for arithmetic operations.
        OptionString* pItem = new OptionString(
            maCaptionStringConversion, toString(maConfig.meStringConversion));
        addOption( pModel, pItem);
    }

    pModel->Insert(createBoolItem(maCaptionEmptyStringAsZero,maConfig.mbEmptyStringAsZero));

    {
        // Syntax for INDIRECT function.
        OptionString* pItem = new OptionString(
            maCaptionStringRefSyntax, toString(maConfig.meStringRefAddressSyntax));
        addOption( pModel, pItem);
    }

#if HAVE_FEATURE_OPENCL
    pModel->Insert(createBoolItem(maCaptionOpenCLEnabled,maConfig.mbOpenCLEnabled));
    fillOpenclList();

    mpBtnAutomaticSelectionFalse->Check(!maConfig.mbOpenCLAutoSelect);
    mpBtnAutomaticSelectionTrue->Check(maConfig.mbOpenCLAutoSelect);
#endif

    mpLbSettings->SetUpdateMode(true);
}

void ScCalcOptionsDialog::SelectionChanged()
{
    sal_uLong nSelectedPos = mpLbSettings->GetSelectEntryPos();
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

        case CALC_OPTION_STRING_CONVERSION:
        {
            // String conversion for arithmetic operations.
            mpBtnTrue->Hide();
            mpBtnFalse->Hide();
            mpLbOptionEdit->Show();
            mpOpenclInfoList->GetParent()->Hide();

            mpLbOptionEdit->Clear();
            mpLbOptionEdit->InsertEntry(maStringConversionAsError);
            mpLbOptionEdit->InsertEntry(maStringConversionAsZero);
            mpLbOptionEdit->InsertEntry(maStringConversionUnambiguous);
            mpLbOptionEdit->InsertEntry(maStringConversionLocaleDependent);
            switch (maConfig.meStringConversion)
            {
                case ScCalcConfig::STRING_CONVERSION_AS_ERROR:
                    mpLbOptionEdit->SelectEntryPos(0);
                break;
                case ScCalcConfig::STRING_CONVERSION_AS_ZERO:
                    mpLbOptionEdit->SelectEntryPos(1);
                break;
                case ScCalcConfig::STRING_CONVERSION_UNAMBIGUOUS:
                    mpLbOptionEdit->SelectEntryPos(2);
                break;
                case ScCalcConfig::STRING_CONVERSION_LOCALE_DEPENDENT:
                    mpLbOptionEdit->SelectEntryPos(3);
                break;
            }
            mpFtAnnotation->SetText(maDescStringConversion);
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
            bool bEnable = true;
            if ( nSelectedPos == CALC_OPTION_EMPTY_AS_ZERO )
            {
                bValue = maConfig.mbEmptyStringAsZero;
                mpFtAnnotation->SetText(maDescEmptyStringAsZero);
                mpOpenclInfoList->GetParent()->Hide();
                switch (maConfig.meStringConversion)
                {
                    case ScCalcConfig::STRING_CONVERSION_AS_ERROR:
                    case ScCalcConfig::STRING_CONVERSION_AS_ZERO:
                        bEnable = false;
                        break;
                    case ScCalcConfig::STRING_CONVERSION_UNAMBIGUOUS:
                    case ScCalcConfig::STRING_CONVERSION_LOCALE_DEPENDENT:
                        break;  // nothing
                }
            }
            else
            {
                bValue = maConfig.mbOpenCLEnabled;
                mpFtAnnotation->SetText(maDescOpenCLEnabled);
                mpOpenclInfoList->GetParent()->Show();
                setOptimalLayoutSize();
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
            if (bEnable)
            {
                mpBtnTrue->Enable();
                mpBtnFalse->Enable();
            }
            else
            {
                mpBtnTrue->Disable();
                mpBtnFalse->Disable();
            }
        }
        break;
        default:
            ;
    }
}

void ScCalcOptionsDialog::ListOptionValueChanged()
{
    sal_uLong nSelected = mpLbSettings->GetSelectEntryPos();
    switch ((CalcOptionOrder) nSelected)
    {
        case CALC_OPTION_REF_SYNTAX:
        {
            // Formula syntax for INDIRECT function.
            sal_Int32 nPos = mpLbOptionEdit->GetSelectEntryPos();
            maConfig.meStringRefAddressSyntax = toAddressConvention(nPos);

            setValueAt(nSelected, toString(maConfig.meStringRefAddressSyntax));
        }
        break;

        case CALC_OPTION_STRING_CONVERSION:
        {
            // String conversion for arithmetic operations.
            sal_Int32 nPos = mpLbOptionEdit->GetSelectEntryPos();
            maConfig.meStringConversion = toStringConversion(nPos);

            setValueAt(nSelected, toString(maConfig.meStringConversion));

            switch (maConfig.meStringConversion)
            {
                case ScCalcConfig::STRING_CONVERSION_AS_ERROR:
                    maConfig.mbEmptyStringAsZero = false;
                    setValueAt(CALC_OPTION_EMPTY_AS_ZERO, toString(maConfig.mbEmptyStringAsZero));
                    mpLbOptionEdit->SelectEntryPos(0);
                break;
                case ScCalcConfig::STRING_CONVERSION_AS_ZERO:
                    maConfig.mbEmptyStringAsZero = true;
                    setValueAt(CALC_OPTION_EMPTY_AS_ZERO, toString(maConfig.mbEmptyStringAsZero));
                    mpLbOptionEdit->SelectEntryPos(1);
                break;
                case ScCalcConfig::STRING_CONVERSION_UNAMBIGUOUS:
                case ScCalcConfig::STRING_CONVERSION_LOCALE_DEPENDENT:
                    // Reset to the value the user selected before.
                    maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero;
                    setValueAt(CALC_OPTION_EMPTY_AS_ZERO, toString(maConfig.mbEmptyStringAsZero));
                break;
            }
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

    maConfig.mbOpenCLAutoSelect = bValue;
}

void ScCalcOptionsDialog::SelectedDeviceChanged()
{
#if HAVE_FEATURE_OPENCL
    SvTreeListEntry* pEntry = mpOpenclInfoList->GetModel()->GetView(0)->FirstSelected();
    if(!pEntry)
        return;

    sc::OpenclDeviceInfo* pInfo = reinterpret_cast<sc::OpenclDeviceInfo*>(pEntry->GetUserData());
    if(pInfo)
    {
        mpFtFrequency->SetText(OUString::number(pInfo->mnFrequency));
        mpFtComputeUnits->SetText(OUString::number(pInfo->mnComputeUnits));
        mpFtMemory->SetText(OUString::number(pInfo->mnMemory/1024/1024));
    }
    else
    {
        mpFtFrequency->SetText(OUString());
        mpFtComputeUnits->SetText(OUString());
        mpFtMemory->SetText(OUString());
    }

    SvLBoxString* pBoxEntry = dynamic_cast<SvLBoxString*>(pEntry->GetItem(1));
    if (!pBoxEntry)
        return;

    OUString aDevice = pBoxEntry->GetText();
    // use english string for configuration
    if(aDevice == maSoftware)
        aDevice = OPENCL_SOFTWARE_DEVICE_CONFIG_NAME;

    maConfig.maOpenCLDevice = aDevice;
#endif
}

void ScCalcOptionsDialog::RadioValueChanged()
{
    sal_uLong nSelected = mpLbSettings->GetSelectEntryPos();
    bool bValue = mpBtnTrue->IsChecked();
    switch (nSelected)
    {
        case CALC_OPTION_REF_SYNTAX:
        case CALC_OPTION_STRING_CONVERSION:
            return;
        case CALC_OPTION_EMPTY_AS_ZERO:
            maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero = bValue;
            break;
        case CALC_OPTION_ENABLE_OPENCL:
            maConfig.mbOpenCLEnabled = bValue;
            if(bValue)
                mpOpenclInfoList->GetParent()->Enable();
            else
                mpOpenclInfoList->GetParent()->Disable();
            OpenclAutomaticSelectionChanged();
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

OUString ScCalcOptionsDialog::toString(ScCalcConfig::StringConversion eConv) const
{
    switch (eConv)
    {
        case ScCalcConfig::STRING_CONVERSION_AS_ERROR:
            return maStringConversionAsError;
        case ScCalcConfig::STRING_CONVERSION_AS_ZERO:
            return maStringConversionAsZero;
        case ScCalcConfig::STRING_CONVERSION_UNAMBIGUOUS:
            return maStringConversionUnambiguous;
        case ScCalcConfig::STRING_CONVERSION_LOCALE_DEPENDENT:
            return maStringConversionLocaleDependent;
    }
    return maStringConversionAsError;
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

IMPL_LINK_NOARG(ScCalcOptionsDialog, DeviceSelHdl)
{
    SelectedDeviceChanged();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
