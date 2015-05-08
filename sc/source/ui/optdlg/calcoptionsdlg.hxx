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
    virtual void dispose() SAL_OVERRIDE;

    DECL_LINK( BtnAutomaticSelectHdl, void* );
    DECL_LINK( DeviceSelHdl, void* );
    DECL_LINK( NumModifiedHdl, void * );
    DECL_LINK( EditModifiedHdl, Edit * );
    DECL_STATIC_LINK( ScCalcOptionsDialog, TestClickHdl, PushButton* );
    DECL_LINK( AsZeroModifiedHdl, CheckBox*);
    DECL_LINK( ConversionModifiedHdl, ListBox*);
    DECL_LINK( SyntaxModifiedHdl, ListBox*);
    DECL_LINK( CBUseOpenCLHdl, CheckBox*);
    DECL_LINK( SpinOpenCLMinSizeHdl, NumericField*);

    const ScCalcConfig& GetConfig() const { return maConfig;}

private:
    void OpenCLAutomaticSelectionChanged();
    void SelectedDeviceChanged();

    SvTreeListEntry *createItem(const OUString &rCaption, const OUString& sValue) const;
    void     setValueAt(size_t nPos, const OUString &rString);
#if HAVE_FEATURE_OPENCL
    OpenCLConfig::ImplMatcherSet& CurrentWhiteOrBlackList();
    const OpenCLConfig::ImplMatcher& CurrentWhiteOrBlackListEntry();
#endif

private:
    VclPtr<CheckBox> mpEmptyAsZero;
    VclPtr<ListBox> mpConversion;
    VclPtr<ListBox> mpSyntax;

    VclPtr<CheckBox> mpUseOpenCL;
    VclPtr<NumericField> mpSpinButton;
    VclPtr<VclMultiLineEdit> mpEditField;
    VclPtr<PushButton> mpTestButton;

    VclPtr<FixedText> mpFtFrequency;
    VclPtr<FixedText> mpFtComputeUnits;
    VclPtr<FixedText> mpFtMemory;

    VclPtr<SvTreeListBox> mpOpenclInfoList;
    VclPtr<RadioButton> mpBtnAutomaticSelectionTrue;
    VclPtr<RadioButton> mpBtnAutomaticSelectionFalse;

    OUString maSoftware;

    ScCalcConfig maConfig;
#if HAVE_FEATURE_OPENCL
    std::vector<OpenCLPlatformInfo> maPlatformInfo;
#endif

    bool mbSelectedEmptyStringAsZero;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
