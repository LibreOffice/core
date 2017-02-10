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
#include <comphelper/random.hxx>
#include "rangelst.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "strload.hxx"
#include "docfunc.hxx"

#include "SamplingDialog.hxx"

ScSamplingDialog::ScSamplingDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent,
                      "SamplingDialog", "modules/scalc/ui/samplingdialog.ui" ),
    mpActiveEdit    ( nullptr  ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mInputRange     ( ScAddress::INITIALIZE_INVALID ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mOutputAddress  ( ScAddress::INITIALIZE_INVALID ),
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

    Init();
    GetRangeFromSelection();
}

ScSamplingDialog::~ScSamplingDialog()
{
    disposeOnce();
}

void ScSamplingDialog::dispose()
{
    mpInputRangeLabel.clear();
    mpInputRangeEdit.clear();
    mpInputRangeButton.clear();
    mpOutputRangeLabel.clear();
    mpOutputRangeEdit.clear();
    mpOutputRangeButton.clear();
    mpSampleSize.clear();
    mpPeriod.clear();
    mpRandomMethodRadio.clear();
    mpPeriodicMethodRadio.clear();
    mpButtonOk.clear();
    mpActiveEdit.clear();
    ScAnyRefDlg::dispose();
}

void ScSamplingDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScSamplingDialog, OkClicked ) );
    mpButtonOk->Enable(false);

    Link<Control&,void> aLink = LINK( this, ScSamplingDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScSamplingDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl( aLink );
    mpInputRangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );

    Link<Edit&,void> aLink2 = LINK( this, ScSamplingDialog, RefInputModifyHandler);
    mpInputRangeEdit->SetModifyHdl( aLink2);
    mpOutputRangeEdit->SetModifyHdl( aLink2);

    mpSampleSize->SetModifyHdl( LINK( this, ScSamplingDialog, SamplingSizeValueModified ));

    mpPeriodicMethodRadio->SetToggleHdl( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );
    mpRandomMethodRadio->SetToggleHdl( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );

    mpSampleSize->SetMin( 0 );
    mpSampleSize->SetMax( SAL_MAX_INT64 );

    mpOutputRangeEdit->GrabFocus();
    mpPeriodicMethodRadio->Check();

    ToggleSamplingMethod();
}

void ScSamplingDialog::GetRangeFromSelection()
{
    mViewData->GetSimpleArea(mInputRange);
    OUString aCurrentString(mInputRange.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails));
    mpInputRangeEdit->SetText(aCurrentString);
}

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

bool ScSamplingDialog::Close()
{
    return DoClose( ScSamplingDialogWrapper::GetChildWindowId() );
}

void ScSamplingDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpActiveEdit )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if ( mpActiveEdit == mpInputRangeEdit )
        {
            mInputRange = rReferenceRange;
            aReferenceString = mInputRange.Format(ScRefFlags::RANGE_ABS_3D, pDocument, mAddressDetails);
            mpInputRangeEdit->SetRefString( aReferenceString );
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            mOutputAddress = rReferenceRange.aStart;

            ScRefFlags nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ?
                                                             ScRefFlags::ADDR_ABS :
                                                             ScRefFlags::ADDR_ABS_3D;
            aReferenceString = mOutputAddress.Format(nFormat, pDocument, pDocument->GetAddressConvention());
            mpOutputRangeEdit->SetRefString( aReferenceString );

            // Change sampling size according to output range selection
            sal_Int64 aSelectedSampleSize = rReferenceRange.aEnd.Row() - rReferenceRange.aStart.Row() + 1;
            if (aSelectedSampleSize > 1)
                mpSampleSize->SetValue(aSelectedSampleSize);
            SamplingSizeValueModified(*mpSampleSize);
        }
    }

    // Enable OK if both, input range and output address are set.
    if (mInputRange.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
}

ScRange ScSamplingDialog::PerformPeriodicSampling(ScDocShell* pDocShell)
{
    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCROW outRow = mOutputAddress.Row();

    sal_Int64 aPeriod = mpPeriod->GetValue();

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        SCCOL outCol = mOutputAddress.Col();
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
    SCROW outRow = mOutputAddress.Row();

    SCROW inRow;

    sal_Int64 aSampleSize = mpSampleSize->GetValue();

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        SCCOL outCol = mOutputAddress.Col();
        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            SCROW aPopulationSize = (aEnd.Row() - aStart.Row()) + 1;

            outRow = mOutputAddress.Row();
            inRow  = aStart.Row();

            while ((outRow - mOutputAddress.Row()) < aSampleSize)
            {
                double aRandomValue = comphelper::rng::uniform_real_distribution();

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
    OUString aUndo(SC_RESSTR(STR_SAMPLING_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();

    ScRange aModifiedRange;

    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    if (mpRandomMethodRadio->IsChecked())
    {
        aModifiedRange = PerformRandomSampling(pDocShell);
    }
    else if (mpPeriodicMethodRadio->IsChecked())
    {
        aModifiedRange = PerformPeriodicSampling(pDocShell);
    }

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint(aModifiedRange, PaintPartFlags::Grid);
}

IMPL_LINK_NOARG( ScSamplingDialog, OkClicked, Button*, void )
{
    PerformSampling();
    Close();
}

IMPL_LINK( ScSamplingDialog, GetFocusHandler, Control&, rCtrl, void )
{
    mpActiveEdit = nullptr;

    if(      (&rCtrl == static_cast<Control*>(mpInputRangeEdit))  || (&rCtrl == static_cast<Control*>(mpInputRangeButton)) )
        mpActiveEdit = mpInputRangeEdit;
    else if( (&rCtrl == static_cast<Control*>(mpOutputRangeEdit)) || (&rCtrl == static_cast<Control*>(mpOutputRangeButton)) )
        mpActiveEdit = mpOutputRangeEdit;

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG(ScSamplingDialog, LoseFocusHandler, Control&, void)
{
    mDialogLostFocus = !IsActive();
}

IMPL_LINK_NOARG(ScSamplingDialog, SamplingSizeValueModified, Edit&, void)
{
    sal_Int64 aPopulationSize = mInputRange.aEnd.Row() - mInputRange.aStart.Row() + 1;
    if (mpSampleSize->GetValue() > aPopulationSize)
        mpSampleSize->SetValue(aPopulationSize);
}

IMPL_LINK_NOARG(ScSamplingDialog, ToggleSamplingMethod, RadioButton&, void)
{
    ToggleSamplingMethod();
}

void ScSamplingDialog::ToggleSamplingMethod()
{
    if (mpRandomMethodRadio->IsChecked())
    {
        mpPeriod->Enable(false);
        mpSampleSize->Enable();
    }
    else if (mpPeriodicMethodRadio->IsChecked())
    {
        mpPeriod->Enable();
        mpSampleSize->Enable(false);
    }
}

IMPL_LINK_NOARG(ScSamplingDialog, RefInputModifyHandler, Edit&, void)
{
    if ( mpActiveEdit )
    {
        if ( mpActiveEdit == mpInputRangeEdit )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mpInputRangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? aRangeList[0] : nullptr;
            if (pRange)
            {
                mInputRange = *pRange;
                // Highlight the resulting range.
                mpInputRangeEdit->StartUpdateData();
            }
            else
            {
                mInputRange = ScRange( ScAddress::INITIALIZE_INVALID);
            }
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mpOutputRangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? aRangeList[0] : nullptr;
            if (pRange)
            {
                mOutputAddress = pRange->aStart;

                // Crop output range to top left address for Edit field.
                if (pRange->aStart != pRange->aEnd)
                {
                    ScRefFlags nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ?
                                                                     ScRefFlags::ADDR_ABS :
                                                                     ScRefFlags::ADDR_ABS_3D;
                    OUString aReferenceString = mOutputAddress.Format(nFormat, mDocument, mDocument->GetAddressConvention());
                    mpOutputRangeEdit->SetRefString( aReferenceString );
                }

                // Change sampling size according to output range selection
                sal_Int64 aSelectedSampleSize = pRange->aEnd.Row() - pRange->aStart.Row() + 1;
                if (aSelectedSampleSize > 1)
                    mpSampleSize->SetValue(aSelectedSampleSize);
                SamplingSizeValueModified(*mpSampleSize);

                // Highlight the resulting range.
                mpOutputRangeEdit->StartUpdateData();
            }
            else
            {
                mOutputAddress = ScAddress( ScAddress::INITIALIZE_INVALID);
            }
        }
    }

    // Enable OK if both, input range and output address are set.
    if (mInputRange.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
