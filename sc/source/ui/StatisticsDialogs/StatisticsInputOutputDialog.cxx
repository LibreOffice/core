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

#include <rangelst.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <tabvwsh.hxx>

#include <StatisticsInputOutputDialog.hxx>

ScRangeList ScStatisticsInputOutputDialog::MakeColumnRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd)
{
    ScRangeList aRangeList;
    for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
    {
        ScRange aColumnRange (
            ScAddress(inCol, aStart.Row(), aTab),
            ScAddress(inCol, aEnd.Row(),   aTab) );

        aRangeList.push_back(aColumnRange);
    }
    return aRangeList;
}

ScRangeList ScStatisticsInputOutputDialog::MakeRowRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd)
{
    ScRangeList aRangeList;
    for (SCROW inRow = aStart.Row(); inRow <= aEnd.Row(); inRow++)
    {
        ScRange aRowRange (
            ScAddress(aStart.Col(), inRow, aTab),
            ScAddress(aEnd.Col(),   inRow, aTab) );

        aRangeList.push_back(aRowRange);
    }
    return aRangeList;
}

ScStatisticsInputOutputDialog::ScStatisticsInputOutputDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData* pViewData, const OUString& rUIXMLDescription, const OString& rID)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent, rUIXMLDescription, rID)
    , mxInputRangeLabel(m_xBuilder->weld_label("input-range-label"))
    , mxInputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("input-range-edit")))
    , mxInputRangeButton(new formula::RefButton(m_xBuilder->weld_button("input-range-button")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("output-range-label"))
    , mxOutputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("output-range-edit")))
    , mxOutputRangeButton(new formula::RefButton(m_xBuilder->weld_button("output-range-button")))
    , mxGroupByColumnsRadio(m_xBuilder->weld_radio_button("groupedby-columns-radio"))
    , mxGroupByRowsRadio(m_xBuilder->weld_radio_button("groupedby-rows-radio"))
    , mViewData(pViewData)
    , mDocument(pViewData->GetDocument())
    , mInputRange(ScAddress::INITIALIZE_INVALID)
    , mAddressDetails(mDocument.GetAddressConvention(), 0, 0)
    , mOutputAddress(ScAddress::INITIALIZE_INVALID)
    , mGroupedBy(BY_COLUMN)
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mpActiveEdit(nullptr)
    , mCurrentAddress(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo())
    , mDialogLostFocus(false)
{
    mxInputRangeEdit->SetReferences(this, mxInputRangeLabel.get());
    mxInputRangeButton->SetReferences(this, mxInputRangeEdit.get());

    mxOutputRangeEdit->SetReferences(this, mxOutputRangeLabel.get());
    mxOutputRangeButton->SetReferences(this, mxOutputRangeEdit.get());

    Init();
    GetRangeFromSelection();
}

ScStatisticsInputOutputDialog::~ScStatisticsInputOutputDialog()
{
}

void ScStatisticsInputOutputDialog::Init()
{
    mxButtonOk->connect_clicked( LINK( this, ScStatisticsInputOutputDialog, OkClicked ) );
    mxButtonOk->set_sensitive(false);

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScStatisticsInputOutputDialog, GetEditFocusHandler );
    mxInputRangeEdit->SetGetFocusHdl( aEditLink );
    mxOutputRangeEdit->SetGetFocusHdl( aEditLink );
    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScStatisticsInputOutputDialog, GetButtonFocusHandler );
    mxInputRangeButton->SetGetFocusHdl( aButtonLink );
    mxOutputRangeButton->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScStatisticsInputOutputDialog, LoseEditFocusHandler );
    mxInputRangeEdit->SetLoseFocusHdl( aEditLink );
    mxOutputRangeEdit->SetLoseFocusHdl( aEditLink );
    aButtonLink = LINK( this, ScStatisticsInputOutputDialog, LoseButtonFocusHandler );
    mxInputRangeButton->SetLoseFocusHdl( aButtonLink );
    mxOutputRangeButton->SetLoseFocusHdl( aButtonLink );

    Link<formula::RefEdit&,void> aLink2 = LINK( this, ScStatisticsInputOutputDialog, RefInputModifyHandler);
    mxInputRangeEdit->SetModifyHdl( aLink2);
    mxOutputRangeEdit->SetModifyHdl( aLink2);

    mxOutputRangeEdit->GrabFocus();

    mxGroupByColumnsRadio->connect_toggled( LINK( this, ScStatisticsInputOutputDialog, GroupByChanged ) );
    mxGroupByRowsRadio->connect_toggled( LINK( this, ScStatisticsInputOutputDialog, GroupByChanged ) );

    mxGroupByColumnsRadio->set_active(true);
    mxGroupByRowsRadio->set_active(false);
}

void ScStatisticsInputOutputDialog::GetRangeFromSelection()
{
    mViewData->GetSimpleArea(mInputRange);
    OUString aCurrentString(mInputRange.Format(mDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails));
    mxInputRangeEdit->SetText(aCurrentString);
}

void ScStatisticsInputOutputDialog::SetActive()
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

void ScStatisticsInputOutputDialog::SetReference( const ScRange& rReferenceRange, ScDocument& rDocument )
{
    if ( mpActiveEdit )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if (mpActiveEdit == mxInputRangeEdit.get())
        {
            mInputRange = rReferenceRange;
            aReferenceString = mInputRange.Format(rDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
            mxInputRangeEdit->SetRefString( aReferenceString );
        }
        else if (mpActiveEdit == mxOutputRangeEdit.get())
        {
            mOutputAddress = rReferenceRange.aStart;

            ScRefFlags nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ?
                                                             ScRefFlags::ADDR_ABS :
                                                             ScRefFlags::ADDR_ABS_3D;
            aReferenceString = mOutputAddress.Format(nFormat, &rDocument, rDocument.GetAddressConvention());
            mxOutputRangeEdit->SetRefString( aReferenceString );
        }
    }

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, OkClicked, weld::Button&, void )
{
    CalculateInputAndWriteToOutput();
    response(RET_OK);
}

IMPL_LINK(ScStatisticsInputOutputDialog, GetEditFocusHandler, formula::RefEdit&, rCtrl, void)
{
    mpActiveEdit = nullptr;

    if (&rCtrl == mxInputRangeEdit.get())
        mpActiveEdit = mxInputRangeEdit.get();
    if (&rCtrl == mxOutputRangeEdit.get())
        mpActiveEdit = mxOutputRangeEdit.get();

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK(ScStatisticsInputOutputDialog, GetButtonFocusHandler, formula::RefButton&, rCtrl, void)
{
    mpActiveEdit = nullptr;

    if (&rCtrl == mxInputRangeButton.get())
        mpActiveEdit = mxInputRangeEdit.get();
    else if (&rCtrl == mxOutputRangeButton.get())
        mpActiveEdit = mxOutputRangeEdit.get();

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK_NOARG(ScStatisticsInputOutputDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScStatisticsInputOutputDialog, LoseButtonFocusHandler, formula::RefButton&, void)
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, GroupByChanged, weld::ToggleButton&, void )
{
    if (mxGroupByColumnsRadio->get_active())
        mGroupedBy = BY_COLUMN;
    else if (mxGroupByRowsRadio->get_active())
        mGroupedBy = BY_ROW;

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, RefInputModifyHandler, formula::RefEdit&, void )
{
    if ( mpActiveEdit )
    {
        if (mpActiveEdit == mxInputRangeEdit.get())
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mxInputRangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mInputRange = *pRange;
                // Highlight the resulting range.
                mxInputRangeEdit->StartUpdateData();
            }
            else
            {
                mInputRange = ScRange( ScAddress::INITIALIZE_INVALID);
            }
        }
        else if (mpActiveEdit == mxOutputRangeEdit.get())
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

                // Highlight the resulting range.
                mxOutputRangeEdit->StartUpdateData();
            }
            else
            {
                mOutputAddress = ScAddress( ScAddress::INITIALIZE_INVALID);
            }
        }
    }

    ValidateDialogInput();
}

void ScStatisticsInputOutputDialog::CalculateInputAndWriteToOutput()
{
    OUString aUndo(ScResId(GetUndoNameId()));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    ScRange aOutputRange = ApplyOutput(pDocShell);

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PaintPartFlags::Grid );
}

bool ScStatisticsInputOutputDialog::InputRangesValid()
{
    return mInputRange.IsValid() && mOutputAddress.IsValid();
}

void ScStatisticsInputOutputDialog::ValidateDialogInput()
{
    // Enable OK button if all inputs are ok.
    mxButtonOk->set_sensitive(InputRangesValid());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
