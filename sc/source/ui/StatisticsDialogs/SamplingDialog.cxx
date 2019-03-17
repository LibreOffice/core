/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svl/undo.hxx>
#include <comphelper/random.hxx>
#include <rangelst.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <reffact.hxx>
#include <docfunc.hxx>
#include <SamplingDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

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
    mnLastSampleSizeValue(1),
    mnLastPeriodValue(1),
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
    get(mpWithReplacement,     "with-replacement");
    get(mpKeepOrder,           "keep-order");
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
    mpWithReplacement.clear();
    mpKeepOrder.clear();
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
    mpPeriod->SetModifyHdl( LINK( this, ScSamplingDialog, PeriodValueModified ));

    mpPeriodicMethodRadio->SetToggleHdl( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );
    mpRandomMethodRadio->SetToggleHdl( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );

    mpWithReplacement->SetClickHdl( LINK( this, ScSamplingDialog, CheckHdl));
    mpKeepOrder->SetClickHdl( LINK( this, ScSamplingDialog, CheckHdl));

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

            LimitSampleSizeAndPeriod();
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
    // Disable if at least one is invalid.
    if (mInputRange.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Enable(false);
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
                assert(aPeriod && "div-by-zero");
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

    const sal_Int64 nSampleSize = mpSampleSize->GetValue();

    // This implementation groups by columns. Other options could be grouping
    // by rows or area.
    const sal_Int64 nPopulationSize = aEnd.Row() - aStart.Row() + 1;

    const bool bWithReplacement = mpWithReplacement->IsEnabled() && mpWithReplacement->IsChecked();

    // WOR (WithOutReplacement) can't draw more than population. Catch that in
    // the caller.
    assert( bWithReplacement || nSampleSize <= nPopulationSize);
    if (!bWithReplacement && nSampleSize > nPopulationSize)
        // Would enter an endless loop below, bail out.
        return ScRange( mOutputAddress);

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        SCCOL outCol = mOutputAddress.Col();
        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            outRow = mOutputAddress.Row();
            std::vector<bool> vUsed( nPopulationSize, false);

            while ((outRow - mOutputAddress.Row()) < nSampleSize)
            {
                // [a,b] *both* inclusive
                SCROW nRandom = comphelper::rng::uniform_int_distribution( aStart.Row(), aEnd.Row());

                if (!bWithReplacement)
                {
                    nRandom -= aStart.Row();
                    if (vUsed[nRandom])
                    {
                        // Find a nearest one, preferring forwards.
                        // Again: it's essential that the loop is entered only
                        // if nSampleSize<=nPopulationSize, which is checked
                        // above.
                        SCROW nBack = nRandom;
                        SCROW nForw = nRandom;
                        do
                        {
                            if (nForw < nPopulationSize - 1 && !vUsed[++nForw])
                            {
                                nRandom = nForw;
                                break;
                            }
                            if (nBack > 0 && !vUsed[--nBack])
                            {
                                nRandom = nBack;
                                break;
                            }
                        }
                        while (true);
                    }
                    vUsed[nRandom] = true;
                    nRandom += aStart.Row();
                }

                const double fValue = mDocument->GetValue( ScAddress(inCol, nRandom, inTab) );
                pDocShell->GetDocFunc().SetValueCell(ScAddress(outCol, outRow, outTab), fValue, true);
                outRow++;
            }
            outCol++;
        }
        outTab++;
    }

    return ScRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
}

ScRange ScSamplingDialog::PerformRandomSamplingKeepOrder(ScDocShell* pDocShell)
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
    OUString aUndo(ScResId(STR_SAMPLING_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();

    ScRange aModifiedRange;

    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    if (mpRandomMethodRadio->IsChecked())
    {
        if (mpKeepOrder->IsEnabled() && mpKeepOrder->IsChecked())
            aModifiedRange = PerformRandomSamplingKeepOrder(pDocShell);
        else
            aModifiedRange = PerformRandomSampling(pDocShell);
    }
    else if (mpPeriodicMethodRadio->IsChecked())
    {
        aModifiedRange = PerformPeriodicSampling(pDocShell);
    }

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint(aModifiedRange, PaintPartFlags::Grid);
}

sal_Int64 ScSamplingDialog::GetPopulationSize() const
{
    return mInputRange.IsValid() ? mInputRange.aEnd.Row() - mInputRange.aStart.Row() + 1 : 0;
}

void ScSamplingDialog::LimitSampleSizeAndPeriod()
{
    // Limit sample size (for WOR methods) and period if population is smaller
    // than last known value. When enlargening the input population range the
    // values will be adjusted up to the last known value again.
    const sal_Int64 nPopulationSize = GetPopulationSize();
    if (nPopulationSize <= mnLastSampleSizeValue && !mpWithReplacement->IsChecked())
        mpSampleSize->SetValue( nPopulationSize);
    if (nPopulationSize <= mnLastPeriodValue)
        mpPeriod->SetValue( nPopulationSize);
}

IMPL_LINK_NOARG(ScSamplingDialog, SamplingSizeValueModified, Edit&, void)
{
    if (!mpWithReplacement->IsChecked())
    {
        // For all WOR methods limit sample size to population size.
        const sal_Int64 nPopulationSize = GetPopulationSize();
        if (mpSampleSize->GetValue() > nPopulationSize)
            mpSampleSize->SetValue(nPopulationSize);
    }
    mnLastSampleSizeValue = mpSampleSize->GetValue();
}

IMPL_LINK_NOARG(ScSamplingDialog, PeriodValueModified, Edit&, void)
{
    // Limit period to population size.
    const sal_Int64 nPopulationSize = GetPopulationSize();
    if (mpPeriod->GetValue() > nPopulationSize)
        mpPeriod->SetValue(nPopulationSize);
    mnLastPeriodValue = mpPeriod->GetValue();
}

IMPL_LINK( ScSamplingDialog, GetFocusHandler, Control&, rCtrl, void )
{
    if (     (&rCtrl == static_cast<Control*>(mpInputRangeEdit))  || (&rCtrl == static_cast<Control*>(mpInputRangeButton)))
        mpActiveEdit = mpInputRangeEdit;
    else if ((&rCtrl == static_cast<Control*>(mpOutputRangeEdit)) || (&rCtrl == static_cast<Control*>(mpOutputRangeButton)))
        mpActiveEdit = mpOutputRangeEdit;
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG( ScSamplingDialog, OkClicked, Button*, void )
{
    PerformSampling();
    Close();
}

IMPL_LINK_NOARG(ScSamplingDialog, LoseFocusHandler, Control&, void)
{
    mDialogLostFocus = !IsActive();
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
        mpWithReplacement->Enable();
        mpKeepOrder->Enable();
    }
    else if (mpPeriodicMethodRadio->IsChecked())
    {
        // WOR keeping order.
        mpPeriod->Enable();
        mpSampleSize->Enable(false);
        mpWithReplacement->Check(false);
        mpWithReplacement->Enable(false);
        mpKeepOrder->Check();
        mpKeepOrder->Enable(false);
    }
}

IMPL_LINK( ScSamplingDialog, CheckHdl, Button*, pBtn, void )
{
    // Keep both checkboxes enabled so user can easily switch between the three
    // possible combinations (one or the other or none), just uncheck the other
    // one if one is checked. Otherwise the other checkbox would had to be
    // disabled until user unchecks the enabled one again, which would force
    // user to two clicks to switch.
    if (pBtn == mpWithReplacement)
    {
        if (static_cast<const CheckBox*>(pBtn)->IsChecked())
        {
            // For WR can't keep order.
            mpKeepOrder->Check(false);
        }
        else
        {
            // For WOR limit sample size to population size.
            SamplingSizeValueModified(*mpSampleSize);
        }
    }
    else if (pBtn == mpKeepOrder)
    {
        if (static_cast<const CheckBox*>(pBtn)->IsChecked())
        {
            // Keep order is always WOR.
            mpWithReplacement->Check(false);
            SamplingSizeValueModified(*mpSampleSize);
        }
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
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mInputRange = *pRange;
                // Highlight the resulting range.
                mpInputRangeEdit->StartUpdateData();

                LimitSampleSizeAndPeriod();
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
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
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
