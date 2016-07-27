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
    ScCalcOptionsDialog(vcl::Window* pParent, const ScCalcConfig& rConfig, bool bWriteConfig);
    virtual ~ScCalcOptionsDialog();
    virtual void dispose() override;

    DECL_LINK_TYPED( AsZeroModifiedHdl, Button*, void);
    DECL_LINK_TYPED( ConversionModifiedHdl, ListBox&, void);
    DECL_LINK_TYPED( SyntaxModifiedHdl, ListBox&, void);
    DECL_LINK_TYPED( CurrentDocOnlyHdl, Button*, void);

    const ScCalcConfig& GetConfig() const { return maConfig;}
    bool GetWriteCalcConfig() const { return mbWriteConfig;}

private:
    void OpenCLAutomaticSelectionChanged();
    void SelectedDeviceChanged();
    void CoupleEmptyAsZeroToStringConversion();

private:
    VclPtr<CheckBox> mpEmptyAsZero;
    VclPtr<ListBox> mpConversion;
    VclPtr<ListBox> mpSyntax;
    VclPtr<CheckBox> mpCurrentDocOnly;

    ScCalcConfig maConfig;

    bool mbSelectedEmptyStringAsZero;
    bool mbWriteConfig;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
