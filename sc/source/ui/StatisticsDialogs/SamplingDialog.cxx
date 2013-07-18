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

#include "SamplingDialog.hxx"

ScSamplingDialog::ScSamplingDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent,
                      "SamplingDialog", "modules/scalc/ui/samplingdialog.ui" ),
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

    get(mpSampleSize, "sample-size-spin");
    get(mpPeriod,     "period-spin");

    get(mpRandomMethodRadio,   "random-method-radio");
    get(mpPeriodicMethodRadio, "periodic-method-radio");

    get(mpButtonOk,     "ok");
    get(mpButtonApply,  "apply");
    get(mpButtonClose,  "close");

    Init();
    GetRangeFromSelection();
}

void ScSamplingDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScSamplingDialog, OkClicked ) );
    mpButtonClose->SetClickHdl( LINK( this, ScSamplingDialog, CloseClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScSamplingDialog, ApplyClicked ) );
    mpButtonOk->Enable(false);
    mpButtonApply->Enable(false);

    Link aLink = LINK( this, ScSamplingDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScSamplingDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl( aLink );
    mpInputRangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );

    mpSampleSize->SetModifyHdl( LINK( this, ScSamplingDialog, SamplingSizeValueModified ));

    mpPeriodicMethodRadio->SetToggleHdl( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );
    mpRandomMethodRadio->SetToggleHdl( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );

    mpSampleSize->SetMin( 0 );
    mpSampleSize->SetMax( SAL_MAX_INT64 );

    mpOutputRangeEdit->GrabFocus();
    mpPeriodicMethodRadio->Check(true);

    ToggleSamplingMethod(NULL);
}

void ScSamplingDialog::GetRangeFromSelection()
{
    OUString aCurrentString;
    mViewData->GetSimpleArea(mInputRange);
    mInputRange.Format(aCurrentString, SCR_ABS_3D, mDocument, mAddressDetails);
    mpInputRangeEdit->SetText(aCurrentString);
}

ScSamplingDialog::~ScSamplingDialog()
{}

void ScSamplingDialog::SetActive()
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

sal_Bool ScSamplingDialog::Close()
{
    return DoClose( ScSamplingDialogWrapper::GetChildWindowId() );
}

void ScSamplingDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
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

            // Change sampling size according to output range selection
            sal_Int64 aSelectedSampleSize = rReferenceRange.aEnd.Row() - rReferenceRange.aStart.Row() + 1;
            if (aSelectedSampleSize > 1)
                mpSampleSize->SetValue(aSelectedSampleSize);
            SamplingSizeValueModified(NULL);

            // Enable OK, Cancel if output range is set
            mpButtonOk->Enable(!mpOutputRangeEdit->GetText().isEmpty());
            mpButtonApply->Enable(!mpOutputRangeEdit->GetText().isEmpty());
        }
    }
}

ScRange ScSamplingDialog::PerformPeriodicSampling(ScDocShell* pDocShell)
{
    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    sal_Int64 aPeriod = mpPeriod->GetValue();

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        outCol = mOutputAddress.Col();
        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            sal_Int64 i = 0;
            outRow = mOutputAddress.Row();
            for (SCROW inRow = aStart.Row(); inRow <= aEnd.Row(); inRow++)
            {
                if (i % aPeriod == aPeriod - 1 ) // Sample the last of period
                {
                    double aValue = mDocument->GetValue(ScAddress(inCol, inRow, inTab));
                    pDocShell->GetDocFunc().SetValueCell(ScAddress(outCol, outRow, outTab), aValue, true);
                    outRow++;
                }
                i++;
            }
            outCol++;
        }
        outTab++;
    }

    return ScRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
}

ScRange ScSamplingDialog::PerformRandomSampling(ScDocShell* pDocShell)
{
    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    TimeValue now;
    osl_getSystemTime(&now);
    boost::mt19937 seed(now.Nanosec);
    boost::uniform_01<boost::mt19937> rng(seed);

    SCROW inRow;

    sal_Int64 aSampleSize = mpSampleSize->GetValue();

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        outCol = mOutputAddress.Col();
        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            SCROW aPopulationSize = (aEnd.Row() - aStart.Row()) + 1;

            outRow = mOutputAddress.Row();
            inRow  = aStart.Row();

            double aRandomValue;

            while ((outRow - mOutputAddress.Row()) < aSampleSize)
            {
                aRandomValue = rng();

                if ( (aPopulationSize - (inRow - aStart.Row())) * aRandomValue >= aSampleSize - (outRow - mOutputAddress.Row()) )
                {
                    inRow++;
                }
                else
                {
                    double aValue = mDocument->GetValue( ScAddress(inCol, inRow, inTab) );
                    pDocShell->GetDocFunc().SetValueCell(ScAddress(outCol, outRow, outTab), aValue, true);
                    inRow++;
                    outRow++;
                }
            }
            outCol++;
        }
        outTab++;
    }

    return ScRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
}

void ScSamplingDialog::PerformSampling()
{
    OUString aUndo(ScResId(STR_SAMPLING_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();

    ScRange aModifiedRange;

    pUndoManager->EnterListAction( aUndo, aUndo );

    if (mpRandomMethodRadio->IsChecked())
    {
        aModifiedRange = PerformRandomSampling(pDocShell);
    }
    else if (mpPeriodicMethodRadio->IsChecked())
    {
        aModifiedRange = PerformPeriodicSampling(pDocShell);
    }

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint(aModifiedRange, PAINT_GRID);
}

IMPL_LINK( ScSamplingDialog, OkClicked, PushButton*, /*pButton*/ )
{
    ApplyClicked(NULL);
    CloseClicked(NULL);
    return 0;
}


IMPL_LINK( ScSamplingDialog, ApplyClicked, PushButton*, /*pButton*/ )
{
    PerformSampling();
    return 0;
}

IMPL_LINK( ScSamplingDialog, CloseClicked, PushButton*, /*pButton*/ )
{
    Close();
    return 0;
}

IMPL_LINK( ScSamplingDialog, GetFocusHandler, Control*, pCtrl )
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

IMPL_LINK_NOARG(ScSamplingDialog, LoseFocusHandler)
{
    mDialogLostFocus = !IsActive();
    return 0;
}

IMPL_LINK_NOARG(ScSamplingDialog, SamplingSizeValueModified)
{
    sal_Int64 aPopulationSize = mInputRange.aEnd.Row() - mInputRange.aStart.Row() + 1;
    if (mpSampleSize->GetValue() > aPopulationSize)
        mpSampleSize->SetValue(aPopulationSize);
    return 0;
}

IMPL_LINK_NOARG(ScSamplingDialog, ToggleSamplingMethod)
{
    if (mpRandomMethodRadio->IsChecked())
    {
        mpPeriod->Enable(false);
        mpSampleSize->Enable(true);
    }
    else if (mpPeriodicMethodRadio->IsChecked())
    {
        mpPeriod->Enable(true);
        mpSampleSize->Enable(false);
    }
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
