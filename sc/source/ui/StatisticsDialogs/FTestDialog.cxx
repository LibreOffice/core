/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/dispatch.hxx>
#include <svl/zforlist.hxx>
#include <svl/undo.hxx>
#include <boost/random.hpp>
#include <boost/scoped_ptr.hpp>

#include "formulacell.hxx"
#include "rangelst.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "strload.hxx"
#include "random.hxx"
#include "docfunc.hxx"
#include "StatisticsDialogs.hrc"
#include "TableFillingAndNavigationTools.hxx"

#include "FTestDialog.hxx"

namespace
{

static const OUString strWildcardVariable1Range("%VAR1_RANGE%");
static const OUString strWildcardVariable2Range("%VAR2_RANGE%");

}

ScFTestDialog::ScFTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "TTestDialog", "modules/scalc/ui/ttestdialog.ui" )
{}

ScFTestDialog::~ScFTestDialog()
{}

bool ScFTestDialog::Close()
{
    return DoClose( ScFTestDialogWrapper::GetChildWindowId() );
}

sal_Int16 ScFTestDialog::GetUndoNameId()
{
    return STR_FTEST_UNDO_NAME;
}

ScRange ScFTestDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    boost::scoped_ptr<DataRangeIterator> pVariable1Iterator;
    if (mGroupedBy == BY_COLUMN)
        pVariable1Iterator.reset(new DataRangeByColumnIterator(mVariable1Range));
    else
        pVariable1Iterator.reset(new DataRangeByRowIterator(mVariable1Range));

    boost::scoped_ptr<DataRangeIterator> pVariable2Iterator;
    if (mGroupedBy == BY_COLUMN)
        pVariable2Iterator.reset(new DataRangeByColumnIterator(mVariable2Range));
    else
        pVariable2Iterator.reset(new DataRangeByRowIterator(mVariable2Range));

    aTemplate.autoReplaceRange(strWildcardVariable1Range, pVariable1Iterator->get());
    aTemplate.autoReplaceRange(strWildcardVariable2Range, pVariable2Iterator->get());

    aOutput.writeBoldString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_UNDO_NAME));
    aOutput.newLine();

    aOutput.writeString("Alpha");
    aOutput.nextColumn();
    aOutput.writeValue(0.05);
    aTemplate.autoReplaceAddress("%ALPHA%", aOutput.current());
    aOutput.newLine();

    aOutput.nextColumn();
    aOutput.writeBoldString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_VARIABLE_1_LABEL));
    aOutput.nextColumn();
    aOutput.writeBoldString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_VARIABLE_2_LABEL));
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STRID_CALC_MEAN));
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VAR1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STRID_CALC_VARIANCE));
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VAR1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VAR1_VARIANCE%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VAR2_VARIANCE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_OBSERVATIONS_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VAR1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VAR1_OBSERVATIONS%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VAR2_OBSERVATIONS%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ANOVA_LABEL_DF));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VAR1_OBSERVATIONS% - 1");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VAR1_DEGREE_OF_FREEDOM%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VAR2_OBSERVATIONS% - 1");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VAR2_DEGREE_OF_FREEDOM%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ANOVA_LABEL_F));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VAR1_VARIANCE% / %VAR2_VARIANCE%");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%F_VALUE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_P_RIGHT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FDIST(%F_VALUE%; %VAR1_DEGREE_OF_FREEDOM%; %VAR2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%P_RIGHT_TAIL_VALUE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_F_CRITICAL_RIGHT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(%ALPHA%; %VAR1_DEGREE_OF_FREEDOM%; %VAR2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_P_LEFT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=1 - %P_RIGHT_TAIL_VALUE%");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%P_LEFT_TAIL_VALUE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_F_CRITICAL_LEFT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(1-%ALPHA%; %VAR1_DEGREE_OF_FREEDOM%; %VAR2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_P_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=2*MIN(%P_RIGHT_TAIL_VALUE%; %P_LEFT_TAIL_VALUE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_FTEST_F_CRITICAL_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(1-(%ALPHA%/2); %VAR1_DEGREE_OF_FREEDOM%; %VAR2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(%ALPHA%/2; %VAR1_DEGREE_OF_FREEDOM%; %VAR2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
