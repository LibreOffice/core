/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_ANALYSISOFVARIANCEDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ANALYSISOFVARIANCEDIALOG_HXX

#include <address.hxx>
#include "viewdata.hxx"

#include "StatisticsInputOutputDialog.hxx"

class FormulaTemplate;
class AddressWalkerWriter;

class ScAnalysisOfVarianceDialog : public ScStatisticsInputOutputDialog
{
private:
    enum AnovaFactor
    {
        SINGLE_FACTOR,
        TWO_FACTOR
    };

    DECL_LINK(FactorChanged, weld::ToggleButton&, void);
    void FactorChanged();

    AnovaFactor meFactor;

    std::unique_ptr<weld::SpinButton> mxAlphaField;
    std::unique_ptr<weld::RadioButton> mxSingleFactorRadio;
    std::unique_ptr<weld::RadioButton> mxTwoFactorRadio;
    std::unique_ptr<weld::SpinButton> mxRowsPerSampleField;

    static void RowColumn(ScRangeList& rRangeList, AddressWalkerWriter& aOutput,
                   FormulaTemplate& aTemplate, const OUString& sFormula,
                   GroupedBy aGroupedBy, ScRange* pResultRange);

    void AnovaSingleFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate);
    void AnovaTwoFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate);

public:
    ScAnalysisOfVarianceDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData& rViewData );

    virtual ~ScAnalysisOfVarianceDialog() override;

    virtual void Close() override;

protected:
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
