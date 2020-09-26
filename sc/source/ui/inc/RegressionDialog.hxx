/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_REGRESSIONDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_REGRESSIONDIALOG_HXX

#include "StatisticsTwoVariableDialog.hxx"

class ScRegressionDialog : public ScStatisticsTwoVariableDialog
{
    bool mbUnivariate;
    size_t mnNumIndependentVars;
    size_t mnNumObservations;
    bool mbUse3DAddresses;
    bool mbCalcIntercept;

    std::unique_ptr<weld::CheckButton> mxWithLabelsCheckBox;
    std::unique_ptr<weld::RadioButton> mxLinearRadioButton;
    std::unique_ptr<weld::RadioButton> mxLogarithmicRadioButton;
    std::unique_ptr<weld::RadioButton> mxPowerRadioButton;
    std::unique_ptr<weld::Label> mxErrorMessage;
    std::unique_ptr<weld::SpinButton> mxConfidenceLevelField;
    std::unique_ptr<weld::CheckButton> mxCalcResidualsCheckBox;
    std::unique_ptr<weld::CheckButton> mxNoInterceptCheckBox;

public:
    ScRegressionDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData& rViewData );

    virtual ~ScRegressionDialog() override;

    virtual void Close() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
    virtual bool InputRangesValid() override;

private:

    using CellValueGetter = const OUString&(size_t, size_t);
    using CellWriter = void(const OUString&, size_t, size_t);

    size_t GetRegressionTypeIndex() const;
    ScRange GetDataRange(const ScRange& rRange);
    OUString GetVariableNameFormula(bool bXVar, size_t nIndex, bool bWithLog);
    OUString GetXVariableNameFormula(size_t nIndex, bool bWithLog);
    OUString GetYVariableNameFormula(bool bWithLog);

    // Helper methods for writing different parts of regression results.
    void WriteRawRegressionResults(AddressWalkerWriter& rOutput,
                                   FormulaTemplate& rTemplate,
                                   size_t nRegressionIndex);
    void WriteRegressionStatistics(AddressWalkerWriter& rOutput,
                                   FormulaTemplate& rTemplate);
    void WriteRegressionANOVAResults(AddressWalkerWriter& rOutput,
                                     FormulaTemplate& rTemplate);
    void WriteRegressionEstimatesWithCI(AddressWalkerWriter& rOutput,
                                        FormulaTemplate& rTemplate,
                                        bool bTakeLogX);
    void WritePredictionsWithResiduals(AddressWalkerWriter& rOutput,
                                       FormulaTemplate& rTemplate,
                                       size_t nRegressionIndex);
    // Generic table writer
    static void WriteTable(const std::function<CellValueGetter>& rCellGetter, size_t nRowsInTable,
                    size_t nColsInTable, AddressWalkerWriter& rOutput,
                    const std::function<CellWriter>& rFunc);

    DECL_LINK( CheckBoxHdl, weld::ToggleButton&, void );
    DECL_LINK( NumericFieldHdl, weld::SpinButton&, void );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
