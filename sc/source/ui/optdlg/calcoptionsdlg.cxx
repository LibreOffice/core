/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cmath>

#include "calcconfig.hxx"
#include "calcoptionsdlg.hxx"
#include "docfunc.hxx"
#include "docsh.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "scopetools.hxx"
#include "viewdata.hxx"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <comphelper/random.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>

#if HAVE_FEATURE_OPENCL
#include "formulagroup.hxx"
#include "globalnames.hxx"
#endif

namespace {

typedef enum {
    CALC_OPTION_STRING_CONVERSION,
    CALC_OPTION_EMPTY_AS_ZERO,
    CALC_OPTION_REF_SYNTAX,
    CALC_OPTION_ENABLE_OPENCL_SUBSET,
    CALC_OPTION_OPENCL_MIN_SIZE,
    CALC_OPTION_OPENCL_SUBSET_OPS,
} CalcOptionOrder;

class OptionString : public SvLBoxString
{
    OUString maDesc;
    OUString maValue;
public:
    OptionString(const OUString& rDesc, const OUString& rValue) :
        maDesc(rDesc), maValue(rValue) {}

    void SetValue(const OUString &rValue) { maValue = rValue; }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;

    virtual void InitViewData(SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData) SAL_OVERRIDE;
};

void OptionString::InitViewData(
    SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    OUString aDesc = maDesc + ": ";
    Size aDescSize(pView->GetTextWidth(aDesc), pView->GetTextHeight());

    vcl::Font aOldFont = pView->GetFont();
    vcl::Font aFont = aOldFont;
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
    vcl::Font aOldFont = rDev.GetFont();
    vcl::Font aFont = aOldFont;
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

ScCalcOptionsDialog::ScCalcOptionsDialog(vcl::Window* pParent, const ScCalcConfig& rConfig)
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
    get(mpSpinButton, "spinbutton");
    get(mpEditField, "entry");
    get(mpOpenclInfoList, "opencl_list");
    get(mpBtnAutomaticSelectionTrue, "automatic_select_true");
    get(mpBtnAutomaticSelectionFalse, "automatic_select_false");
    get(mpFtFrequency, "frequency");
    get(mpFtComputeUnits, "compute_units");
    get(mpFtMemory, "memory");

    mpSpinButton->SetModifyHdl(LINK(this, ScCalcOptionsDialog, NumModifiedHdl));
    mpEditField->SetModifyHdl(LINK(this, ScCalcOptionsDialog, EditModifiedHdl));

    mpOpenclInfoList->set_height_request(4* mpOpenclInfoList->GetTextHeight());
    mpOpenclInfoList->SetStyle(mpOpenclInfoList->GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    mpOpenclInfoList->SetHighlightRange();
    mpOpenclInfoList->GetParent()->Hide();
    mpOpenclInfoList->SetSelectHdl(LINK(this, ScCalcOptionsDialog, DeviceSelHdl));

    mpBtnAutomaticSelectionTrue->SetToggleHdl(LINK(this, ScCalcOptionsDialog, BtnAutomaticSelectHdl));

    maCaptionStringRefSyntax = get<vcl::Window>("ref_syntax_caption")->GetText();
    maDescStringRefSyntax = get<vcl::Window>("ref_syntax_desc")->GetText();
    maUseFormulaSyntax = get<vcl::Window>("use_formula_syntax")->GetText();

    maCaptionStringConversion = get<vcl::Window>("string_conversion_caption")->GetText();
    maDescStringConversion = get<vcl::Window>("string_conversion_desc")->GetText();
    maStringConversionAsError = get<vcl::Window>("string_conversion_as_error")->GetText();
    maStringConversionAsZero = get<vcl::Window>("string_conversion_as_zero")->GetText();
    maStringConversionUnambiguous = get<vcl::Window>("string_conversion_unambiguous")->GetText();
    maStringConversionLocaleDependent = get<vcl::Window>("string_conversion_locale_dependent")->GetText();

    maCaptionEmptyStringAsZero = get<vcl::Window>("empty_str_as_zero_caption")->GetText();
    maDescEmptyStringAsZero = get<vcl::Window>("empty_str_as_zero_desc")->GetText();

    maCaptionOpenCLSubsetEnabled = get<vcl::Window>("opencl_subset_enabled")->GetText();
    maDescOpenCLSubsetEnabled = get<vcl::Window>("opencl_subset_enabled_desc")->GetText();

    maCaptionOpenCLMinimumFormulaSize = get<vcl::Window>("opencl_minimum_size")->GetText();
    maDescOpenCLMinimumFormulaSize = get<vcl::Window>("opencl_minimum_size_desc")->GetText();

    maCaptionOpenCLSubsetOpCodes = get<vcl::Window>("opencl_subset_opcodes")->GetText();
    maDescOpenCLSubsetOpCodes = get<vcl::Window>("opencl_subset_opcodes_desc")->GetText();

    maSoftware = get<vcl::Window>("software")->GetText();

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

SvTreeListEntry *ScCalcOptionsDialog::createItem(const OUString &rCaption, const OUString& sValue) const
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->AddItem(new SvLBoxString(pEntry, 0, OUString()));
    pEntry->AddItem(new SvLBoxContextBmp(pEntry, 0, Image(), Image(), false));
    OptionString* pItem = new OptionString(rCaption, sValue);
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

void ScCalcOptionsDialog::fillOpenCLList()
{
    mpOpenclInfoList->SetUpdateMode(false);
    mpOpenclInfoList->Clear();
    SvTreeListEntry* pSoftwareEntry = mpOpenclInfoList->InsertEntry(maSoftware);

    OUString aStoredDevice = maConfig.maOpenCLDevice;

    SvTreeListEntry* pSelectedEntry = NULL;

    sc::FormulaGroupInterpreter::fillOpenCLInfo(maPlatformInfo);
    for(std::vector<OpenCLPlatformInfo>::iterator it = maPlatformInfo.begin(),
            itEnd = maPlatformInfo.end(); it != itEnd; ++it)
    {
        for(std::vector<OpenCLDeviceInfo>::iterator
                itr = it->maDevices.begin(), itrEnd = it->maDevices.end(); itr != itrEnd; ++itr)
        {
            OUString aDeviceId = it->maVendor + " " + itr->maName + " " + itr->maDriver;
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

    pModel->Insert(createItem(maCaptionEmptyStringAsZero,toString(maConfig.mbEmptyStringAsZero)));

    {
        // Syntax for INDIRECT function.
        OptionString* pItem = new OptionString(
            maCaptionStringRefSyntax, toString(maConfig.meStringRefAddressSyntax));
        addOption( pModel, pItem);
    }

#if HAVE_FEATURE_OPENCL
    pModel->Insert(createItem(maCaptionOpenCLSubsetEnabled,toString(maConfig.mbOpenCLSubsetOnly)));
    pModel->Insert(createItem(maCaptionOpenCLMinimumFormulaSize,toString(maConfig.mnOpenCLMinimumFormulaGroupSize)));
    pModel->Insert(createItem(maCaptionOpenCLSubsetOpCodes,ScOpCodeSetToSymbolicString(maConfig.maOpenCLSubsetOpCodes)));

    fillOpenCLList();

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
            mpSpinButton->Hide();
            mpEditField->Hide();
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
            mpSpinButton->Hide();
            mpEditField->Hide();
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
        case CALC_OPTION_ENABLE_OPENCL_SUBSET:
        {
            mpLbOptionEdit->Hide();
            mpBtnTrue->Show();
            mpBtnFalse->Show();
            mpSpinButton->Hide();
            mpEditField->Hide();

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
            else if ( nSelectedPos == CALC_OPTION_ENABLE_OPENCL_SUBSET )
            {
                bValue = maConfig.mbOpenCLSubsetOnly;
                mpFtAnnotation->SetText(maDescOpenCLSubsetEnabled);
                mpOpenclInfoList->GetParent()->Hide();
            }
            else
            {
                assert(false);
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

        // numeric fields
        case CALC_OPTION_OPENCL_MIN_SIZE:
        {
            // just one numeric field so far
            sal_Int32 nValue = maConfig.mnOpenCLMinimumFormulaGroupSize;
            mpLbOptionEdit->Hide();
            mpBtnTrue->Hide();
            mpBtnFalse->Hide();
            mpSpinButton->Show();
            mpEditField->Hide();
            mpOpenclInfoList->GetParent()->Hide();
            mpFtAnnotation->SetText(maDescOpenCLMinimumFormulaSize);
            mpSpinButton->SetValue(nValue);
        }
        break;

        // strings
        case CALC_OPTION_OPENCL_SUBSET_OPS:
        {
            // just one string field so far
            OUString sValue = ScOpCodeSetToSymbolicString(maConfig.maOpenCLSubsetOpCodes);
            mpLbOptionEdit->Hide();
            mpBtnTrue->Hide();
            mpBtnFalse->Hide();
            mpSpinButton->Hide();
            mpEditField->Show();
            mpOpenclInfoList->GetParent()->Hide();
            mpFtAnnotation->SetText(maDescOpenCLSubsetOpCodes);
            mpEditField->SetText(sValue);
        }
        break;

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
        case CALC_OPTION_ENABLE_OPENCL_SUBSET:
        case CALC_OPTION_OPENCL_MIN_SIZE:
        case CALC_OPTION_OPENCL_SUBSET_OPS:
        break;
    }
}

void ScCalcOptionsDialog::OpenCLAutomaticSelectionChanged()
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

    OpenCLDeviceInfo* pInfo = reinterpret_cast<OpenCLDeviceInfo*>(pEntry->GetUserData());
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
        case CALC_OPTION_ENABLE_OPENCL_SUBSET:
            maConfig.mbOpenCLSubsetOnly = bValue;
            break;
    }

    setValueAt(nSelected, toString(bValue));
}

void ScCalcOptionsDialog::SpinButtonValueChanged()
{
    // We know that the mpSpinButton is used for only one thing at the moment,
    // the OpenCL minimum formula size
    sal_Int64 nVal = mpSpinButton->GetValue();
    maConfig.mnOpenCLMinimumFormulaGroupSize = nVal;
}

void ScCalcOptionsDialog::EditFieldValueChanged(Control *pCtrl)
{
    Edit& rEdit(dynamic_cast<Edit&>(*pCtrl));

    OUString sVal = rEdit.GetText();

    if (&rEdit == mpEditField)
    {
        // We know that the mpEditField is used for only one thing at the moment,
        // the OpenCL subset list of opcodes
        maConfig.maOpenCLSubsetOpCodes = ScStringToOpCodeSet(sVal);
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

OUString ScCalcOptionsDialog::toString(sal_Int32 nVal) const
{
    return OUString::number(nVal);
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
    OpenCLAutomaticSelectionChanged();
    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, DeviceSelHdl)
{
    SelectedDeviceChanged();
    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, NumModifiedHdl)
{
    SpinButtonValueChanged();
    return 0;
}

IMPL_LINK(ScCalcOptionsDialog, EditModifiedHdl, Control*, pCtrl)
{
    EditFieldValueChanged(pCtrl);
    return 0;
}

#if 0

namespace {

struct Area
{
    OUString msTitle;
    int mnRows;

    Area(const OUString& rTitle, int nRows) :
        msTitle(rTitle),
        mnRows(nRows)
    {
    }

    virtual ~Area()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const = 0;

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const = 0;

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const = 0;
};

struct OpenCLTester
{
    int mnTestAreas;
    ScDocShell* mpDocShell;
    ScDocument *mpDoc;

    OpenCLTester() :
        mnTestAreas(0)
    {
        css::uno::Reference< css::uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
        css::uno::Reference< css::frame::XDesktop2 > xComponentLoader = css::frame::Desktop::create(xContext);
        css::uno::Reference< css::lang::XComponent >
            xComponent( xComponentLoader->loadComponentFromURL( "private:factory/scalc",
                                                                "_blank", 0,
                                                                css::uno::Sequence < css::beans::PropertyValue >() ) );
        mpDocShell = dynamic_cast<ScDocShell*>(SfxObjectShell::GetShellFromComponent(xComponent));

        assert(mpDocShell);

        mpDoc = &mpDocShell->GetDocument();

        mpDoc->SetString(ScAddress(0,0,0), "Result:");
    }

    void addTest(const Area &rArea)
    {
        sc::AutoCalcSwitch aACSwitch(*mpDoc, true);

        mnTestAreas++;
        (void) mpDocShell->GetDocFunc().InsertTable(mnTestAreas, rArea.msTitle, false, true);

        rArea.addHeader(mpDoc, mnTestAreas);

        for (int i = 0; i < rArea.mnRows; ++i)
            rArea.addRow(mpDoc, i, mnTestAreas);

        mpDoc->SetString(ScAddress(0,1+mnTestAreas-1,0), rArea.msTitle + ":");
        mpDoc->SetString(ScAddress(1,1+mnTestAreas-1,0), rArea.getSummaryFormula(mpDoc, mnTestAreas));

        mpDoc->SetString(ScAddress(1,0,0),
                        OUString("=IF(SUM(") +
                        ScRange(ScAddress(1,1,0),
                                ScAddress(1,1+mnTestAreas-1,0)).Format(SCA_VALID|SCA_VALID_COL|SCA_VALID_ROW) +
                        ")=0,\"PASS\",\"FAIL\")");
    }
};

struct Op : Area
{
    OUString msOp;
    double mnRangeLo;
    double mnRangeHi;
    double mnEpsilon;

    Op(const OUString& rTitle,
       const OUString& rOp,
       double nRangeLo, double nRangeHi,
       double nEpsilon) :
        Area(rTitle, 200),
        msOp(rOp),
        mnRangeLo(nRangeLo),
        mnRangeHi(nRangeHi),
        mnEpsilon(nEpsilon)
    {
    }

    virtual ~Op()
    {
    }
};

struct UnOp : Op
{
    double (*mpFun)(double nArg);
    bool (*mpFilterOut)(double nArg);

    UnOp(const OUString& rTitle,
         const OUString& rOp,
         double nRangeLo, double nRangeHi,
         double nEpsilon,
         double (*pFun)(double nArg),
         bool (*pFilterOut)(double nArg) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~UnOp()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        pDoc->SetString(ScAddress(0,0,nTab), "arg");
        pDoc->SetString(ScAddress(1,0,nTab), msOp + "(arg)");
        pDoc->SetString(ScAddress(2,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const SAL_OVERRIDE
    {
        double nArg;

        do {
            nArg = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nArg));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nArg);

        pDoc->SetString(ScAddress(1,1+nRow,nTab),
                        OUString("=") + msOp + "(" + ScAddress(0,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) + ")");

        pDoc->SetValue(ScAddress(2,1+nRow,nTab), mpFun(nArg));

        if (mnEpsilon < 0)
        {
            // relative epsilon
            pDoc->SetString(ScAddress(3,1+nRow,nTab),
                            OUString("=IF(ABS((") + ScAddress(1,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            "-" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            ")/" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            ")<=" + OUString::number(-mnEpsilon) +
                            ",0,1)");
        }
        else
        {
            // absolute epsilon
            pDoc->SetString(ScAddress(3,1+nRow,nTab),
                            OUString("=IF(ABS(") + ScAddress(1,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            "-" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            ")<=" + OUString::number(mnEpsilon) +
                            ",0,1)");
        }
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        return OUString("=SUM(") +
            ScRange(ScAddress(3,1,nTab),
                    ScAddress(3,1+mnRows-1,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB, pDoc) +
            ")";
    }
};

struct BinOp : Op
{
    double (*mpFun)(double nLhs, double nRhs);
    bool (*mpFilterOut)(double nLhs, double nRhs);

    BinOp(const OUString& rTitle,
          const OUString& rOp,
          double nRangeLo, double nRangeHi,
          double nEpsilon,
          double (*pFun)(double nLhs, double nRhs),
          bool (*pFilterOut)(double nLhs, double nRhs) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~BinOp()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        pDoc->SetString(ScAddress(0,0,nTab), "lhs");
        pDoc->SetString(ScAddress(1,0,nTab), "rhs");
        pDoc->SetString(ScAddress(2,0,nTab), OUString("lhs") + msOp + "rhs");
        pDoc->SetString(ScAddress(3,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const SAL_OVERRIDE
    {
        double nLhs, nRhs;

        do {
            nLhs = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
            nRhs = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nLhs, nRhs));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nLhs);
        pDoc->SetValue(ScAddress(1,1+nRow,nTab), nRhs);

        pDoc->SetString(ScAddress(2,1+nRow,nTab),
                        OUString("=") + ScAddress(0,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                        msOp + ScAddress(1,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW));

        pDoc->SetValue(ScAddress(3,1+nRow,nTab), mpFun(nLhs, nRhs));

        pDoc->SetString(ScAddress(4,1+nRow,nTab),
                        OUString("=IF(ABS(") + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                        "-" + ScAddress(3,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                        ")<=" + OUString::number(mnEpsilon) +
                        ",0,1)");
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        return OUString("=SUM(") +
            ScRange(ScAddress(4,1,nTab),
                    ScAddress(4,1+mnRows-1,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB, pDoc) +
            ")";
    }
};

struct Reduction : Op
{
    int mnNum;
    double mnAccumInitial;
    double (*mpFun)(double nAccum, double nArg);
    bool (*mpFilterOut)(double nArg);

    Reduction(const OUString& rTitle,
              const OUString& rOp,
              int nNum,
              double nAccumInitial,
              double nRangeLo, double nRangeHi,
              double nEpsilon,
              double (*pFun)(double nAccum, double nArg),
              bool (*pFilterOut)(double nArg) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mnNum(nNum),
        mnAccumInitial(nAccumInitial),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~Reduction()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        pDoc->SetString(ScAddress(0,0,nTab), "x");
        pDoc->SetString(ScAddress(1,0,nTab), msOp);
        pDoc->SetString(ScAddress(2,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const SAL_OVERRIDE
    {
        double nArg;

        do {
            nArg = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nArg));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nArg);

        if (nRow >= mnNum-1)
        {
            pDoc->SetString(ScAddress(1,1+nRow-mnNum+1,nTab),
                            OUString("=") + msOp + "(" +
                            ScRange(ScAddress(0,1+nRow-mnNum+1,nTab),
                                    ScAddress(0,1+nRow,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW) +
                            ")");

            double nAccum(mnAccumInitial);
            for (int i = 0; i < mnNum; i++)
                nAccum = mpFun(nAccum, pDoc->GetValue(ScAddress(0,1+nRow-mnNum+i+1,nTab)));

            pDoc->SetValue(ScAddress(2,1+nRow-mnNum+1,nTab), nAccum);

            if (mnEpsilon != 0)
                pDoc->SetString(ScAddress(3,1+nRow-mnNum+1,nTab),
                                OUString("=IF(ABS(") + ScAddress(1,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                "-" + ScAddress(2,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                ")<=" + OUString::number(mnEpsilon) +
                                ",0,1)");
            else
                pDoc->SetString(ScAddress(3,1+nRow-mnNum+1,nTab),
                                OUString("=IF(") + ScAddress(1,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                "=" + ScAddress(2,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                ",0,1)");
        }
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        return OUString("=SUM(") +
            ScRange(ScAddress(3,1+0,nTab),
                    ScAddress(3,1+mnRows-mnNum-1,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB, pDoc) +
            ")";
    }
};

}

IMPL_LINK( ScCalcOptionsDialog, TestClickHdl, PushButton*, )
{
    // Automatically test the current implementation of OpenCL. If it
    // seems good, whitelist it. If it seems bad, blacklist it.

    std::unique_ptr<OpenCLTester> xTestDocument(new OpenCLTester());

    xTestDocument->addTest(BinOp("Plus", "+", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs + nRhs;
                                 }));

    xTestDocument->addTest(BinOp("Minus", "-", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs - nRhs;
                                 }));

    xTestDocument->addTest(BinOp("Times", "*", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs * nRhs;
                                 }));

    xTestDocument->addTest(BinOp("Divided", "/", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs / nRhs;
                                 },
                                 [] (double, double nRhs)
                                 {
                                     return (nRhs == 0);
                                 }));

    xTestDocument->addTest(UnOp("Sin", "SIN", -10, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return sin(nArg);
                                }));

    xTestDocument->addTest(UnOp("Cos", "COS", -10, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return cos(nArg);
                                }));

    xTestDocument->addTest(UnOp("Tan", "TAN", 0, 10, -3e-10,
                                [] (double nArg)
                                {
                                    return tan(nArg);
                                },
                                [] (double nArg)
                                {
                                    return (std::fmod(nArg, M_PI) == M_PI/2);
                                }));

    xTestDocument->addTest(UnOp("Atan", "ATAN", -10, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return atan(nArg);
                                }));

    xTestDocument->addTest(UnOp("Sqrt", "SQRT", 0, 1000, 3e-10,
                                [] (double nArg)
                                {
                                    return sqrt(nArg);
                                }));

    xTestDocument->addTest(UnOp("Exp", "EXP", 0, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return exp(nArg);
                                }));

    xTestDocument->addTest(UnOp("Ln", "LN", 0, 1000, 3e-10,
                                [] (double nArg)
                                {
                                    return log(nArg);
                                },
                                [] (double nArg)
                                {
                                    return (nArg == 0);
                                }));

    xTestDocument->addTest(Reduction("Sum", "SUM", 100, 0, -1000, 1000, 3e-10,
                                     [] (double nAccum, double nArg)
                                     {
                                         return (nAccum + nArg);
                                     }));

    xTestDocument->addTest(Reduction("Average", "AVERAGE", 100, 0, -1000, 1000, 3e-10,
                                     [] (double nAccum, double nArg)
                                     {
                                         return (nAccum + nArg/100.);
                                     }));

    xTestDocument->addTest(Reduction("Product", "PRODUCT", 100, 1, 0.1, 2.5, 3e-10,
                                     [] (double nAccum, double nArg)
                                     {
                                         return (nAccum * nArg);
                                     }));

    xTestDocument->addTest(Reduction("Min", "MIN", 100, DBL_MAX, -1000, 1000, 0,
                                     [] (double nAccum, double nArg)
                                     {
                                         return std::min(nAccum, nArg);
                                     }));

    xTestDocument->addTest(Reduction("Max", "MAX", 100, -DBL_MAX, -1000, 1000, 0,
                                     [] (double nAccum, double nArg)
                                     {
                                         return std::max(nAccum, nArg);
                                     }));

    return 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
