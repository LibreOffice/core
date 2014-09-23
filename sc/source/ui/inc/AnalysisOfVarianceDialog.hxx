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

    NumericField* mpAlphaField;
    RadioButton*  mpSingleFactorRadio;
    RadioButton*  mpTwoFactorRadio;
    NumericField* mpRowsPerSampleField;

    DECL_LINK(FactorChanged,   void*);

    AnovaFactor meFactor;

    void RowColumn(ScRangeList& rRangeList, AddressWalkerWriter& aOutput,
                   FormulaTemplate& aTemplate, OUString& sFormula,
                   GroupedBy aGroupedBy, ScRange* pResultRange);

    void AnovaSingleFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate);
    void AnovaTwoFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate);

public:
    ScAnalysisOfVarianceDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScAnalysisOfVarianceDialog();

    virtual bool Close() SAL_OVERRIDE;

protected:
    virtual sal_Int16 GetUndoNameId() SAL_OVERRIDE;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
