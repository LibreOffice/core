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
    ScAddress maLabelAddr;
    ScRange maActualInputRange;
    SCSIZE mnLen;

    double mfMinMag;

    bool mbUse3DAddresses : 1;
    bool mbGroupedByColumn : 1;
    bool mbWithLabels : 1;
    bool mbInverse : 1;
    bool mbPolar : 1;

    std::unique_ptr<weld::CheckButton> mxWithLabelsCheckBox;
    std::unique_ptr<weld::CheckButton> mxInverseCheckBox;
    std::unique_ptr<weld::CheckButton> mxPolarCheckBox;
    std::unique_ptr<weld::SpinButton> mxMinMagnitudeField;
    std::unique_ptr<weld::Label> mxErrorMessage;

public:
    ScFourierAnalysisDialog(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                            ScViewData& rViewData);

    virtual ~ScFourierAnalysisDialog() override;

    virtual void Close() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
    virtual bool InputRangesValid() override;

private:
    void getOptions();
    void getDataLabel(OUString& rLabel);
    void genFormula(OUString& rFormula);

    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
