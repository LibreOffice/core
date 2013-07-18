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

#include "formulacell.hxx"
#include "rangelst.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "random.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"

#include <boost/random.hpp>

#include "DescriptiveStatisticsDialog.hxx"

namespace
{

struct StatisticCalculation {
    const sal_Int16 aCalculationNameId;
    const char*     aFormula;
};

static const StatisticCalculation lclCalcDefinitions[] =
{
    { SID_CALC_MEAN,           "=AVERAGE(%RANGE%)" },
    { SID_CALC_STD_ERROR,      "=SQRT(VAR(%RANGE%)/COUNT(%RANGE%))"},
    { SID_CALC_MODE,           "=MODE(%RANGE%)"},
    { SID_CALC_MEDIAN,         "=MEDIAN(%RANGE%)"},
    { SID_CALC_VARIANCE,       "=VAR(%RANGE%)"},
    { SID_CALC_STD_DEVIATION,  "=STDEV(%RANGE%)"},
    { SID_CALC_KURTOSIS,       "=KURT(%RANGE%)"},
    { SID_CALC_SKEWNESS,       "=SKEW(%RANGE%)"},
    { SID_CALC_RANGE,          "=MAX(%RANGE%)-MIN(%RANGE%)"},
    { SID_CALC_MIN,            "=MIN(%RANGE%)"},
    { SID_CALC_MAX,            "=MAX(%RANGE%)"},
    { SID_CALC_SUM,            "=SUM(%RANGE%)"},
    { SID_CALC_COUNT,          "=COUNT(%RANGE%)" },
    { 0,                       NULL }
};

}

ScDescriptiveStatisticsDialog::ScDescriptiveStatisticsDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent,
                      "DescriptiveStatisticsDialog", "modules/scalc/ui/descriptivestatisticsdialog.ui" ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mCurrentAddress ( pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() ),
    mDialogLostFocus( false )
{
    get(mpInputRangeLabel,  "input-range-label");
    get(mpInputRangeEdit,   "input-range-edit");
    get(mpInputRangeButton, "input-range-button");
    mpInputRangeEdit->SetReferences(this, mpInputRangeLabel);
    mpInputRangeButton->SetReferences(this, mpInputRangeEdit);

    get(mpOutputRangeLabel,  "output-range-label");
    get(mpOutputRangeEdit,   "output-range-edit");
    get(mpOutputRangeButton, "output-range-button");
    mpOutputRangeEdit->SetReferences(this, mpOutputRangeLabel);
    mpOutputRangeButton->SetReferences(this, mpOutputRangeEdit);

    get(mpButtonOk,     "ok");
    get(mpButtonApply,  "apply");
    get(mpButtonClose,  "close");

    Init();
    GetRangeFromSelection();
}

void ScDescriptiveStatisticsDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScDescriptiveStatisticsDialog, OkClicked ) );
    mpButtonClose->SetClickHdl( LINK( this, ScDescriptiveStatisticsDialog, CloseClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScDescriptiveStatisticsDialog, ApplyClicked ) );
    mpButtonOk->Enable(false);
    mpButtonApply->Enable(false);

    Link aLink = LINK( this, ScDescriptiveStatisticsDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScDescriptiveStatisticsDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl( aLink );
    mpInputRangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );

    mpOutputRangeEdit->GrabFocus();
}

void ScDescriptiveStatisticsDialog::GetRangeFromSelection()
{
    OUString aCurrentString;
    mViewData->GetSimpleArea(mInputRange);
    mInputRange.Format(aCurrentString, SCR_ABS_3D, mDocument, mAddressDetails);
    mpInputRangeEdit->SetText(aCurrentString);
}

ScDescriptiveStatisticsDialog::~ScDescriptiveStatisticsDialog()
{}

void ScDescriptiveStatisticsDialog::SetActive()
{
    if ( mDialogLostFocus )
    {
        mDialogLostFocus = false;
        if( mpActiveEdit )
            mpActiveEdit->GrabFocus();
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

sal_Bool ScDescriptiveStatisticsDialog::Close()
{
    return DoClose( ScDescriptiveStatisticsDialogWrapper::GetChildWindowId() );
}

void ScDescriptiveStatisticsDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpActiveEdit )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        String aReferenceString;

        if ( mpActiveEdit == mpInputRangeEdit )
        {
            mInputRange = rReferenceRange;
            mInputRange.Format( aReferenceString, SCR_ABS_3D, pDocument, mAddressDetails );
            mpInputRangeEdit->SetRefString( aReferenceString );
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            mOutputAddress = rReferenceRange.aStart;

            sal_uInt16 nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ? SCA_ABS : SCA_ABS_3D;
            mOutputAddress.Format( aReferenceString, nFormat, pDocument, pDocument->GetAddressConvention() );
            mpOutputRangeEdit->SetRefString( aReferenceString );

            // Enable OK, Cancel if output range is set
            mpButtonOk->Enable(!mpOutputRangeEdit->GetText().isEmpty());
            mpButtonApply->Enable(!mpOutputRangeEdit->GetText().isEmpty());
        }
    }
}

void ScDescriptiveStatisticsDialog::ApplyCalculationsFormulas( )
{
    OUString aUndo(ScResId(STR_DESCRIPTIVE_STATISTICS_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    OUString aReferenceString;
    ScAddress aAddress;

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        outCol = mOutputAddress.Col();

        for(sal_Int32 i = 0; lclCalcDefinitions[i].aFormula != NULL; i++)
        {
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aCalculationName(ScResId(lclCalcDefinitions[i].aCalculationNameId));
            pDocShell->GetDocFunc().SetStringCell(aAddress, aCalculationName, true);
            outRow++;
        }
        outCol++;

        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            outRow = mOutputAddress.Row();
            ScRange aColumnRange (
                ScAddress(inCol, mInputRange.aStart.Row(), inTab),
                ScAddress(inCol, mInputRange.aEnd.Row(), inTab)
            );

            aColumnRange.Format( aReferenceString, SCR_ABS, mDocument, mAddressDetails );
            OUString sRange("%RANGE%");
            OUString aFormulaString;
            OUString aFormulaTemplate;

            for(sal_Int32 i = 0; lclCalcDefinitions[i].aFormula != NULL; i++)
            {
                aAddress = ScAddress(outCol, outRow, outTab);
                aFormulaTemplate = OUString::createFromAscii(lclCalcDefinitions[i].aFormula);
                aFormulaString = aFormulaTemplate.replaceAll(sRange, aReferenceString);
                pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
                outRow++;
            }
            outCol++;
        }
        outTab++;
    }

    ScRange aOutputRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

IMPL_LINK( ScDescriptiveStatisticsDialog, OkClicked, PushButton*, /*pButton*/ )
{
    ApplyClicked(NULL);
    CloseClicked(NULL);
    return 0;
}


IMPL_LINK( ScDescriptiveStatisticsDialog, ApplyClicked, PushButton*, /*pButton*/ )
{
    ApplyCalculationsFormulas();
    return 0;
}

IMPL_LINK( ScDescriptiveStatisticsDialog, CloseClicked, PushButton*, /*pButton*/ )
{
    Close();
    return 0;
}

IMPL_LINK( ScDescriptiveStatisticsDialog, GetFocusHandler, Control*, pCtrl )
{
    mpActiveEdit = NULL;

    if(      (pCtrl == (Control*) mpInputRangeEdit)  || (pCtrl == (Control*) mpInputRangeButton) )
        mpActiveEdit = mpInputRangeEdit;
    else if( (pCtrl == (Control*) mpOutputRangeEdit) || (pCtrl == (Control*) mpOutputRangeButton) )
        mpActiveEdit = mpOutputRangeEdit;

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

IMPL_LINK_NOARG(ScDescriptiveStatisticsDialog, LoseFocusHandler)
{
    mDialogLostFocus = !IsActive();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
