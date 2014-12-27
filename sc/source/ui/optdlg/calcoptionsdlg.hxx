/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_OPTDLG_CALCOPTIONSDLG_HXX
#define INCLUDED_SC_SOURCE_UI_OPTDLG_CALCOPTIONSDLG_HXX

#include <config_features.h>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <svx/checklbx.hxx>
#include <svtools/treelistbox.hxx>

#include "calcconfig.hxx"

#if HAVE_FEATURE_OPENCL
#include <opencl/openclconfig.hxx>
#include <opencl/platforminfo.hxx>
#endif

class ScCalcOptionsDialog : public ModalDialog
{
public:
    ScCalcOptionsDialog(vcl::Window* pParent, const ScCalcConfig& rConfig);
    virtual ~ScCalcOptionsDialog();

    DECL_LINK( SettingsSelHdl, Control* );
    DECL_LINK( BtnToggleHdl, void* );
    DECL_LINK( BtnAutomaticSelectHdl, void* );
    DECL_LINK( DeviceSelHdl, void* );
    DECL_LINK( NumModifiedHdl, void * );
    DECL_LINK( EditModifiedHdl, Control * );
    DECL_LINK( TestClickHdl, PushButton* );

    const ScCalcConfig& GetConfig() const { return maConfig;}

private:
    void FillOptionsList();
    void SelectionChanged();
    void ListOptionValueChanged();
    void RadioValueChanged();
    void OpenCLAutomaticSelectionChanged();
    void SelectedDeviceChanged();
    void SpinButtonValueChanged();
    void EditFieldValueChanged(Control *pCtrl);
#if HAVE_FEATURE_OPENCL
    void fillOpenCLList();
#endif

    OUString toString(formula::FormulaGrammar::AddressConvention eConv) const;
    OUString toString(ScCalcConfig::StringConversion eConv) const;
    OUString toString(bool bVal) const;
    OUString toString(sal_Int32 nVal) const;
    SvTreeListEntry *createItem(const OUString &rCaption, const OUString& sValue) const;
    void     setValueAt(size_t nPos, const OUString &rString);
#if HAVE_FEATURE_OPENCL
    OpenCLConfig::ImplMatcherSet& CurrentWhiteOrBlackList();
    const OpenCLConfig::ImplMatcher& CurrentWhiteOrBlackListEntry();
#endif

private:
    SvxCheckListBox* mpLbSettings;

    ListBox* mpLbOptionEdit;
    RadioButton* mpBtnTrue;
    RadioButton* mpBtnFalse;
    NumericField* mpSpinButton;
    Edit* mpEditField;
    PushButton* mpTestButton;

    FixedText* mpFtAnnotation;
    FixedText* mpFtFrequency;
    FixedText* mpFtComputeUnits;
    FixedText* mpFtMemory;

    SvTreeListBox* mpOpenclInfoList;
    RadioButton* mpBtnAutomaticSelectionTrue;
    RadioButton* mpBtnAutomaticSelectionFalse;

    OUString maTrue;
    OUString maFalse;

    OUString maCalcA1;
    OUString maExcelA1;
    OUString maExcelR1C1;

    OUString maCaptionStringRefSyntax;
    OUString maDescStringRefSyntax;
    OUString maUseFormulaSyntax;

    OUString maStringConversionAsError;
    OUString maStringConversionAsZero;
    OUString maStringConversionUnambiguous;
    OUString maStringConversionLocaleDependent;

    OUString maCaptionStringConversion;
    OUString maDescStringConversion;

    OUString maCaptionEmptyStringAsZero;
    OUString maDescEmptyStringAsZero;

    OUString maCaptionOpenCLSubsetEnabled;
    OUString maDescOpenCLSubsetEnabled;

    OUString maCaptionOpenCLMinimumFormulaSize;
    OUString maDescOpenCLMinimumFormulaSize;

    OUString maCaptionOpenCLSubsetOpCodes;
    OUString maDescOpenCLSubsetOpCodes;

    OUString maSoftware;

    ScCalcConfig maConfig;
#if HAVE_FEATURE_OPENCL
    std::vector<OpenCLPlatformInfo> maPlatformInfo;
#endif

    bool mbSelectedEmptyStringAsZero;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
