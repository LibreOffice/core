/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_FOURIERANALYSISDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FOURIERANALYSISDIALOG_HXX

#include "StatisticsInputOutputDialog.hxx"

class ScFourierAnalysisDialog : public ScStatisticsInputOutputDialog
{
    VclPtr<CheckBox> mpWithLabelsCheckBox;
    VclPtr<CheckBox> mpInverseCheckBox;
    VclPtr<CheckBox> mpPolarCheckBox;
    VclPtr<NumericField> mpMinMagnitudeField;
    VclPtr<FixedText> mpErrorMessage;

    ScAddress maLabelAddr;
    ScRange maActualInputRange;
    SCSIZE mnLen;

    double mfMinMag;

    bool mbUse3DAddresses : 1;
    bool mbGroupedByColumn : 1;
    bool mbWithLabels : 1;
    bool mbInverse : 1;
    bool mbPolar : 1;

public:
    ScFourierAnalysisDialog(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                            ScViewData* pViewData);

    virtual ~ScFourierAnalysisDialog() override;

    virtual bool Close() override;

protected:
    void dispose() override;
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
    virtual bool InputRangesValid() override;

private:
    void getOptions();
    void getDataLabel(OUString& rLabel);
    void genFormula(OUString& rFormula);

    DECL_LINK(CheckBoxHdl, CheckBox&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
