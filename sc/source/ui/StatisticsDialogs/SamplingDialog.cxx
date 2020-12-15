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

ScSamplingDialog::ScSamplingDialog(SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                                   weld::Window* pParent, ScViewData& rViewData)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent,
                          "modules/scalc/ui/samplingdialog.ui", "SamplingDialog")
    , mpActiveEdit(nullptr)
    , mViewData(rViewData)
    , mDocument(rViewData.GetDocument())
    , mInputRange(ScAddress::INITIALIZE_INVALID)
    , mAddressDetails(mDocument.GetAddressConvention(), 0, 0)
    , mOutputAddress(ScAddress::INITIALIZE_INVALID)
    , mCurrentAddress(rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo())
    , mnLastSampleSizeValue(1)
    , mnLastPeriodValue(1)
    , mDialogLostFocus(false)
    , mxInputRangeLabel(m_xBuilder->weld_label("input-range-label"))
    , mxInputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("input-range-edit")))
    , mxInputRangeButton(new formula::RefButton(m_xBuilder->weld_button("input-range-button")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("output-range-label"))
    , mxOutputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("output-range-edit")))
    , mxOutputRangeButton(new formula::RefButton(m_xBuilder->weld_button("output-range-button")))
    , mxSampleSize(m_xBuilder->weld_spin_button("sample-size-spin"))
    , mxPeriod(m_xBuilder->weld_spin_button("period-spin"))
    , mxRandomMethodRadio(m_xBuilder->weld_radio_button("random-method-radio"))
    , mxWithReplacement(m_xBuilder->weld_check_button("with-replacement"))
    , mxKeepOrder(m_xBuilder->weld_check_button("keep-order"))
    , mxPeriodicMethodRadio(m_xBuilder->weld_radio_button("periodic-method-radio"))
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxButtonCancel(m_xBuilder->weld_button("cancel"))
{
    mxInputRangeEdit->SetReferences(this, mxInputRangeLabel.get());
    mxInputRangeButton->SetReferences(this, mxInputRangeEdit.get());

    mxOutputRangeEdit->SetReferences(this, mxOutputRangeLabel.get());
    mxOutputRangeButton->SetReferences(this, mxOutputRangeEdit.get());

    Init();
    GetRangeFromSelection();
}

ScSamplingDialog::~ScSamplingDialog()
{
}

void ScSamplingDialog::Init()
{
    mxButtonCancel->connect_clicked( LINK( this, ScSamplingDialog, ButtonClicked ) );
    mxButtonOk->connect_clicked( LINK( this, ScSamplingDialog, ButtonClicked ) );
    mxButtonOk->set_sensitive(false);

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScSamplingDialog, GetEditFocusHandler );
    mxInputRangeEdit->SetGetFocusHdl( aEditLink );
    mxOutputRangeEdit->SetGetFocusHdl( aEditLink );
    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScSamplingDialog, GetButtonFocusHandler );
    mxInputRangeButton->SetGetFocusHdl( aButtonLink );
    mxOutputRangeButton->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScSamplingDialog, LoseEditFocusHandler );
    mxInputRangeEdit->SetLoseFocusHdl( aEditLink );
    mxOutputRangeEdit->SetLoseFocusHdl( aEditLink );
    aButtonLink = LINK( this, ScSamplingDialog, LoseButtonFocusHandler );
    mxInputRangeButton->SetLoseFocusHdl( aButtonLink );
    mxOutputRangeButton->SetLoseFocusHdl( aButtonLink );

    Link<formula::RefEdit&,void> aLink2 = LINK( this, ScSamplingDialog, RefInputModifyHandler);
    mxInputRangeEdit->SetModifyHdl( aLink2);
    mxOutputRangeEdit->SetModifyHdl( aLink2);

    mxSampleSize->connect_value_changed( LINK( this, ScSamplingDialog, SamplingSizeValueModified ));
    mxPeriod->connect_value_changed( LINK( this, ScSamplingDialog, PeriodValueModified ));

    mxPeriodicMethodRadio->connect_toggled( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );
    mxRandomMethodRadio->connect_toggled( LINK( this, ScSamplingDialog, ToggleSamplingMethod ) );

    mxWithReplacement->connect_clicked( LINK( this, ScSamplingDialog, CheckHdl));
    mxKeepOrder->connect_clicked( LINK( this, ScSamplingDialog, CheckHdl));

    mxOutputRangeEdit->GrabFocus();
    mxPeriodicMethodRadio->set_active(true);

    ToggleSamplingMethod();
}

void ScSamplingDialog::GetRangeFromSelection()
{
    mViewData.GetSimpleArea(mInputRange);
    OUString aCurrentString(mInputRange.Format(mDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails));
    mxInputRangeEdit->SetText(aCurrentString);
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
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void ScSamplingDialog::Close()
{
    DoClose( ScSamplingDialogWrapper::GetChildWindowId() );
}

void ScSamplingDialog::SetReference( const ScRange& rReferenceRange, ScDocument& rDocument )
{
    if ( mpActiveEdit )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if ( mpActiveEdit == mxInputRangeEdit.get() )
        {
            mInputRange = rReferenceRange;
            aReferenceString = mInputRange.Format(rDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
            mxInputRangeEdit->SetRefString( aReferenceString );

            LimitSampleSizeAndPeriod();
        }
        else if ( mpActiveEdit == mxOutputRangeEdit.get() )
        {
            mOutputAddress = rReferenceRange.aStart;

            ScRefFlags nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ?
                                                             ScRefFlags::ADDR_ABS :
                                                             ScRefFlags::ADDR_ABS_3D;
            aReferenceString = mOutputAddress.Format(nFormat, &rDocument, rDocument.GetAddressConvention());
            mxOutputRangeEdit->SetRefString( aReferenceString );

            // Change sampling size according to output range selection
            sal_Int64 aSelectedSampleSize = rReferenceRange.aEnd.Row() - rReferenceRange.aStart.Row() + 1;
            if (aSelectedSampleSize > 1)
                mxSampleSize->set_value(aSelectedSampleSize);
            SamplingSizeValueModified(*mxSampleSize);
        }
    }

    // Enable OK if both, input range and output address are set.
    // Disable if at least one is invalid.
    mxButtonOk->set_sensitive(mInputRange.IsValid() && mOutputAddress.IsValid());
}

ScRange ScSamplingDialog::PerformPeriodicSampling(ScDocShell* pDocShell)
{
    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCROW outRow = mOutputAddress.Row();

    sal_Int64 aPeriod = mxPeriod->get_value();

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
                    double aValue = mDocument.GetValue(ScAddress(inCol, inRow, inTab));
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

    const sal_Int64 nSampleSize = mxSampleSize->get_value();

    // This implementation groups by columns. Other options could be grouping
    // by rows or area.
    const sal_Int64 nPopulationSize = aEnd.Row() - aStart.Row() + 1;

    const bool bWithReplacement = mxWithReplacement->get_sensitive() && mxWithReplacement->get_active();

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

                const double fValue = mDocument.GetValue( ScAddress(inCol, nRandom, inTab) );
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

    sal_Int64 aSampleSize = mxSampleSize->get_value();

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
                    double aValue = mDocument.GetValue( ScAddress(inCol, inRow, inTab) );
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
    ScDocShell* pDocShell = mViewData.GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();

    ScRange aModifiedRange;

    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData.GetViewShell()->GetViewShellId() );

    if (mxRandomMethodRadio->get_active())
    {
        if (mxKeepOrder->get_sensitive() && mxKeepOrder->get_active())
            aModifiedRange = PerformRandomSamplingKeepOrder(pDocShell);
        else
            aModifiedRange = PerformRandomSampling(pDocShell);
    }
    else if (mxPeriodicMethodRadio->get_active())
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
    if (nPopulationSize <= mnLastSampleSizeValue && !mxWithReplacement->get_active())
        mxSampleSize->set_value( nPopulationSize);
    if (nPopulationSize <= mnLastPeriodValue)
        mxPeriod->set_value( nPopulationSize);
}

IMPL_LINK_NOARG(ScSamplingDialog, SamplingSizeValueModified, weld::SpinButton&, void)
{
    if (!mxWithReplacement->get_active())
    {
        // For all WOR methods limit sample size to population size.
        const sal_Int64 nPopulationSize = GetPopulationSize();
        if (mxSampleSize->get_value() > nPopulationSize)
            mxSampleSize->set_value(nPopulationSize);
    }
    mnLastSampleSizeValue = mxSampleSize->get_value();
}

IMPL_LINK_NOARG(ScSamplingDialog, PeriodValueModified, weld::SpinButton&, void)
{
    // Limit period to population size.
    const sal_Int64 nPopulationSize = GetPopulationSize();
    if (mxPeriod->get_value() > nPopulationSize)
        mxPeriod->set_value(nPopulationSize);
    mnLastPeriodValue = mxPeriod->get_value();
}

IMPL_LINK( ScSamplingDialog, GetEditFocusHandler, formula::RefEdit&, rCtrl, void )
{
    if (&rCtrl == mxInputRangeEdit.get())
        mpActiveEdit = mxInputRangeEdit.get();
    else if (&rCtrl == mxOutputRangeEdit.get())
        mpActiveEdit = mxOutputRangeEdit.get();
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK(ScSamplingDialog, GetButtonFocusHandler, formula::RefButton&, rCtrl, void)
{
    if (&rCtrl == mxInputRangeButton.get())
        mpActiveEdit = mxInputRangeEdit.get();
    else if (&rCtrl == mxOutputRangeButton.get())
        mpActiveEdit = mxOutputRangeEdit.get();
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}


IMPL_LINK(ScSamplingDialog, ButtonClicked, weld::Button&, rButton, void)
{
    if (&rButton == mxButtonOk.get())
    {
        PerformSampling();
        response(RET_OK);
    }
    else
        response(RET_CANCEL);
}

IMPL_LINK_NOARG(ScSamplingDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScSamplingDialog, LoseButtonFocusHandler, formula::RefButton&, void)
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScSamplingDialog, ToggleSamplingMethod, weld::ToggleButton&, void)
{
    ToggleSamplingMethod();
}

void ScSamplingDialog::ToggleSamplingMethod()
{
    if (mxRandomMethodRadio->get_active())
    {
        mxPeriod->set_sensitive(false);
        mxSampleSize->set_sensitive(true);
        mxWithReplacement->set_sensitive(true);
        mxKeepOrder->set_sensitive(true);
    }
    else if (mxPeriodicMethodRadio->get_active())
    {
        // WOR keeping order.
        mxPeriod->set_sensitive(true);
        mxSampleSize->set_sensitive(false);
        mxWithReplacement->set_active(false);
        mxWithReplacement->set_sensitive(false);
        mxKeepOrder->set_active(true);
        mxKeepOrder->set_sensitive(false);
    }
}

IMPL_LINK(ScSamplingDialog, CheckHdl, weld::Button&, rBtn, void)
{
    // Keep both checkboxes enabled so user can easily switch between the three
    // possible combinations (one or the other or none), just uncheck the other
    // one if one is checked. Otherwise the other checkbox would had to be
    // disabled until user unchecks the enabled one again, which would force
    // user to two clicks to switch.
    if (&rBtn == mxWithReplacement.get())
    {
        if (mxWithReplacement->get_active())
        {
            // For WR can't keep order.
            mxKeepOrder->set_active(false);
        }
        else
        {
            // For WOR limit sample size to population size.
            SamplingSizeValueModified(*mxSampleSize);
        }
    }
    else if (&rBtn == mxKeepOrder.get())
    {
        if (mxKeepOrder->get_active())
        {
            // Keep order is always WOR.
            mxWithReplacement->set_active(false);
            SamplingSizeValueModified(*mxSampleSize);
        }
    }
}

IMPL_LINK_NOARG(ScSamplingDialog, RefInputModifyHandler, formula::RefEdit&, void)
{
    if ( mpActiveEdit )
    {
        if ( mpActiveEdit == mxInputRangeEdit.get() )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mxInputRangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mInputRange = *pRange;
                // Highlight the resulting range.
                mxInputRangeEdit->StartUpdateData();

                LimitSampleSizeAndPeriod();
            }
            else
            {
                mInputRange = ScRange( ScAddress::INITIALIZE_INVALID);
            }
        }
        else if ( mpActiveEdit == mxOutputRangeEdit.get() )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mxOutputRangeEdit->GetText(), mDocument);
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
                    OUString aReferenceString = mOutputAddress.Format(nFormat, &mDocument, mDocument.GetAddressConvention());
                    mxOutputRangeEdit->SetRefString( aReferenceString );
                }

                // Change sampling size according to output range selection
                sal_Int64 aSelectedSampleSize = pRange->aEnd.Row() - pRange->aStart.Row() + 1;
                if (aSelectedSampleSize > 1)
                    mxSampleSize->set_value(aSelectedSampleSize);
                SamplingSizeValueModified(*mxSampleSize);

                // Highlight the resulting range.
                mxOutputRangeEdit->StartUpdateData();
            }
            else
            {
                mOutputAddress = ScAddress( ScAddress::INITIALIZE_INVALID);
            }
        }
    }

    // Enable OK if both, input range and output address are set.
    mxButtonOk->set_sensitive(mInputRange.IsValid() && mOutputAddress.IsValid());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
