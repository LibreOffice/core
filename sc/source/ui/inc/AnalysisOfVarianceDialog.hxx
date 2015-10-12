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

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

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

    VclPtr<NumericField> mpAlphaField;
    VclPtr<RadioButton>  mpSingleFactorRadio;
    VclPtr<RadioButton>  mpTwoFactorRadio;
    VclPtr<NumericField> mpRowsPerSampleField;

    DECL_LINK_TYPED(FactorChanged, RadioButton&, void);
    void FactorChanged();

    AnovaFactor meFactor;

    static void RowColumn(ScRangeList& rRangeList, AddressWalkerWriter& aOutput,
                   FormulaTemplate& aTemplate, OUString& sFormula,
                   GroupedBy aGroupedBy, ScRange* pResultRange);

    void AnovaSingleFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate);
    void AnovaTwoFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate);

public:
    ScAnalysisOfVarianceDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScAnalysisOfVarianceDialog();
    virtual void dispose() override;

    virtual bool Close() override;

protected:
    virtual sal_Int16 GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
