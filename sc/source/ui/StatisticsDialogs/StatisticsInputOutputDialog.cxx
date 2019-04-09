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
                    vcl::Window* pParent, ScViewData* pViewData, const OUString& rID, const OUString& rUIXMLDescription ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent, rID, rUIXMLDescription ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mInputRange     ( ScAddress::INITIALIZE_INVALID ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mOutputAddress  ( ScAddress::INITIALIZE_INVALID ),
    mGroupedBy      ( BY_COLUMN ),
    mpActiveEdit    ( nullptr ),
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

    get(mpGroupByColumnsRadio,   "groupedby-columns-radio");
    get(mpGroupByRowsRadio,      "groupedby-rows-radio");

    Init();
    GetRangeFromSelection();
}

ScStatisticsInputOutputDialog::~ScStatisticsInputOutputDialog()
{
    disposeOnce();
}

void ScStatisticsInputOutputDialog::dispose()
{
    mpInputRangeLabel.clear();
    mpInputRangeEdit.clear();
    mpInputRangeButton.clear();
    mpOutputRangeLabel.clear();
    mpOutputRangeEdit.clear();
    mpOutputRangeButton.clear();
    mpGroupByColumnsRadio.clear();
    mpGroupByRowsRadio.clear();
    mpButtonOk.clear();
    mpActiveEdit.clear();
    ScAnyRefDlg::dispose();
}

void ScStatisticsInputOutputDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScStatisticsInputOutputDialog, OkClicked ) );
    mpButtonOk->Enable(false);

    Link<Control&,void> aLink = LINK( this, ScStatisticsInputOutputDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScStatisticsInputOutputDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl( aLink );
    mpInputRangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );

    Link<Edit&,void> aLink2 = LINK( this, ScStatisticsInputOutputDialog, RefInputModifyHandler);
    mpInputRangeEdit->SetModifyHdl( aLink2);
    mpOutputRangeEdit->SetModifyHdl( aLink2);

    mpOutputRangeEdit->GrabFocus();

    mpGroupByColumnsRadio->SetToggleHdl( LINK( this, ScStatisticsInputOutputDialog, GroupByChanged ) );
    mpGroupByRowsRadio->SetToggleHdl( LINK( this, ScStatisticsInputOutputDialog, GroupByChanged ) );

    mpGroupByColumnsRadio->Check();
    mpGroupByRowsRadio->Check(false);
}

void ScStatisticsInputOutputDialog::GetRangeFromSelection()
{
    mViewData->GetSimpleArea(mInputRange);
    OUString aCurrentString(mInputRange.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails));
    mpInputRangeEdit->SetText(aCurrentString);
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
        GrabFocus();
    }
    RefInputDone();
}

void ScStatisticsInputOutputDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
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
        }
    }

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, OkClicked, Button*, void )
{
    CalculateInputAndWriteToOutput();
    Close();
}

IMPL_LINK( ScStatisticsInputOutputDialog, GetFocusHandler, Control&, rCtrl, void )
{
    mpActiveEdit = nullptr;

    if(      (&rCtrl == static_cast<Control*>(mpInputRangeEdit))  || (&rCtrl == static_cast<Control*>(mpInputRangeButton)) )
        mpActiveEdit = mpInputRangeEdit;
    else if( (&rCtrl == static_cast<Control*>(mpOutputRangeEdit)) || (&rCtrl == static_cast<Control*>(mpOutputRangeButton)) )
        mpActiveEdit = mpOutputRangeEdit;

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, LoseFocusHandler, Control&, void )
{
    mDialogLostFocus = !IsActive();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, GroupByChanged, RadioButton&, void )
{
    if (mpGroupByColumnsRadio->IsChecked())
        mGroupedBy = BY_COLUMN;
    else if (mpGroupByRowsRadio->IsChecked())
        mGroupedBy = BY_ROW;

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, RefInputModifyHandler, Edit&, void )
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

                // Highlight the resulting range.
                mpOutputRangeEdit->StartUpdateData();
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
    if (InputRangesValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();
}

ScRangeList ScStatisticsInputOutputDialogController::MakeColumnRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd)
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

ScRangeList ScStatisticsInputOutputDialogController::MakeRowRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd)
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

ScStatisticsInputOutputDialogController::ScStatisticsInputOutputDialogController(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData* pViewData, const OUString& rUIXMLDescription, const OString& rID)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent, rUIXMLDescription, rID)
    , mxInputRangeLabel(m_xBuilder->weld_label("input-range-label"))
    , mxInputRangeEdit(new formula::WeldRefEdit(m_xBuilder->weld_entry("input-range-edit")))
    , mxInputRangeButton(new formula::WeldRefButton(m_xBuilder->weld_button("input-range-button")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("output-range-label"))
    , mxOutputRangeEdit(new formula::WeldRefEdit(m_xBuilder->weld_entry("output-range-edit")))
    , mxOutputRangeButton(new formula::WeldRefButton(m_xBuilder->weld_button("output-range-button")))
    , mxGroupByColumnsRadio(m_xBuilder->weld_radio_button("groupedby-columns-radio"))
    , mxGroupByRowsRadio(m_xBuilder->weld_radio_button("groupedby-rows-radio"))
    , mViewData(pViewData)
    , mDocument(pViewData->GetDocument())
    , mInputRange(ScAddress::INITIALIZE_INVALID)
    , mAddressDetails(mDocument->GetAddressConvention(), 0, 0)
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

ScStatisticsInputOutputDialogController::~ScStatisticsInputOutputDialogController()
{
}

void ScStatisticsInputOutputDialogController::Init()
{
    mxButtonOk->connect_clicked( LINK( this, ScStatisticsInputOutputDialogController, OkClicked ) );
    mxButtonOk->set_sensitive(false);

    Link<formula::WeldRefEdit&,void> aEditLink = LINK( this, ScStatisticsInputOutputDialogController, GetEditFocusHandler );
    mxInputRangeEdit->SetGetFocusHdl( aEditLink );
    mxOutputRangeEdit->SetGetFocusHdl( aEditLink );
    Link<formula::WeldRefButton&,void> aButtonLink = LINK( this, ScStatisticsInputOutputDialogController, GetButtonFocusHandler );
    mxInputRangeButton->SetGetFocusHdl( aButtonLink );
    mxOutputRangeButton->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScStatisticsInputOutputDialogController, LoseEditFocusHandler );
    mxInputRangeEdit->SetLoseFocusHdl( aEditLink );
    mxOutputRangeEdit->SetLoseFocusHdl( aEditLink );
    aButtonLink = LINK( this, ScStatisticsInputOutputDialogController, LoseButtonFocusHandler );
    mxInputRangeButton->SetLoseFocusHdl( aButtonLink );
    mxOutputRangeButton->SetLoseFocusHdl( aButtonLink );

    Link<formula::WeldRefEdit&,void> aLink2 = LINK( this, ScStatisticsInputOutputDialogController, RefInputModifyHandler);
    mxInputRangeEdit->SetModifyHdl( aLink2);
    mxOutputRangeEdit->SetModifyHdl( aLink2);

    mxOutputRangeEdit->GrabFocus();

    mxGroupByColumnsRadio->connect_toggled( LINK( this, ScStatisticsInputOutputDialogController, GroupByChanged ) );
    mxGroupByRowsRadio->connect_toggled( LINK( this, ScStatisticsInputOutputDialogController, GroupByChanged ) );

    mxGroupByColumnsRadio->set_active(true);
    mxGroupByRowsRadio->set_active(false);
}

void ScStatisticsInputOutputDialogController::GetRangeFromSelection()
{
    mViewData->GetSimpleArea(mInputRange);
    OUString aCurrentString(mInputRange.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails));
    mxInputRangeEdit->SetText(aCurrentString);
}

void ScStatisticsInputOutputDialogController::SetActive()
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

void ScStatisticsInputOutputDialogController::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpActiveEdit )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if (mpActiveEdit == mxInputRangeEdit.get())
        {
            mInputRange = rReferenceRange;
            aReferenceString = mInputRange.Format(ScRefFlags::RANGE_ABS_3D, pDocument, mAddressDetails);
            mxInputRangeEdit->SetRefString( aReferenceString );
        }
        else if (mpActiveEdit == mxOutputRangeEdit.get())
        {
            mOutputAddress = rReferenceRange.aStart;

            ScRefFlags nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ?
                                                             ScRefFlags::ADDR_ABS :
                                                             ScRefFlags::ADDR_ABS_3D;
            aReferenceString = mOutputAddress.Format(nFormat, pDocument, pDocument->GetAddressConvention());
            mxOutputRangeEdit->SetRefString( aReferenceString );
        }
    }

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialogController, OkClicked, weld::Button&, void )
{
    CalculateInputAndWriteToOutput();
    response(RET_OK);
}

IMPL_LINK(ScStatisticsInputOutputDialogController, GetEditFocusHandler, formula::WeldRefEdit&, rCtrl, void)
{
    mpActiveEdit = nullptr;

    if (&rCtrl == mxInputRangeEdit.get())
        mpActiveEdit = mxInputRangeEdit.get();
    if (&rCtrl == mxOutputRangeEdit.get())
        mpActiveEdit = mxOutputRangeEdit.get();

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK(ScStatisticsInputOutputDialogController, GetButtonFocusHandler, formula::WeldRefButton&, rCtrl, void)
{
    mpActiveEdit = nullptr;

    if (&rCtrl == mxInputRangeButton.get())
        mpActiveEdit = mxInputRangeEdit.get();
    else if (&rCtrl == mxOutputRangeButton.get())
        mpActiveEdit = mxOutputRangeEdit.get();

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK_NOARG(ScStatisticsInputOutputDialogController, LoseEditFocusHandler, formula::WeldRefEdit&, void)
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScStatisticsInputOutputDialogController, LoseButtonFocusHandler, formula::WeldRefButton&, void)
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialogController, GroupByChanged, weld::ToggleButton&, void )
{
    if (mxGroupByColumnsRadio->get_active())
        mGroupedBy = BY_COLUMN;
    else if (mxGroupByRowsRadio->get_active())
        mGroupedBy = BY_ROW;

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialogController, RefInputModifyHandler, formula::WeldRefEdit&, void )
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
                    OUString aReferenceString = mOutputAddress.Format(nFormat, mDocument, mDocument->GetAddressConvention());
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

void ScStatisticsInputOutputDialogController::CalculateInputAndWriteToOutput()
{
    OUString aUndo(ScResId(GetUndoNameId()));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    ScRange aOutputRange = ApplyOutput(pDocShell);

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PaintPartFlags::Grid );
}

bool ScStatisticsInputOutputDialogController::InputRangesValid()
{
    return mInputRange.IsValid() && mOutputAddress.IsValid();
}

void ScStatisticsInputOutputDialogController::ValidateDialogInput()
{
    // Enable OK button if all inputs are ok.
    mxButtonOk->set_sensitive(InputRangesValid());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
