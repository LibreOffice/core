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

#include <StatisticsTwoVariableDialog.hxx>

ScStatisticsTwoVariableDialog::ScStatisticsTwoVariableDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData, const OUString& rID, const OUString& rUIXMLDescription ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent, rID, rUIXMLDescription ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mVariable1Range ( ScAddress::INITIALIZE_INVALID ),
    mVariable2Range ( ScAddress::INITIALIZE_INVALID ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mOutputAddress  ( ScAddress::INITIALIZE_INVALID ),
    mGroupedBy      ( BY_COLUMN ),
    mpActiveEdit    ( nullptr ),
    mCurrentAddress ( pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() ),
    mDialogLostFocus( false )
{
    get(mpVariable1RangeLabel,  "variable1-range-label");
    get(mpVariable1RangeEdit,   "variable1-range-edit");
    get(mpVariable1RangeButton, "variable1-range-button");
    mpVariable1RangeEdit->SetReferences(this, mpVariable1RangeLabel);
    mpVariable1RangeButton->SetReferences(this, mpVariable1RangeEdit);

    get(mpVariable2RangeLabel,  "variable2-range-label");
    get(mpVariable2RangeEdit,   "variable2-range-edit");
    get(mpVariable2RangeButton, "variable2-range-button");
    mpVariable2RangeEdit->SetReferences(this, mpVariable2RangeLabel);
    mpVariable2RangeButton->SetReferences(this, mpVariable2RangeEdit);

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

ScStatisticsTwoVariableDialog::~ScStatisticsTwoVariableDialog()
{
    disposeOnce();
}

void ScStatisticsTwoVariableDialog::dispose()
{
    mpVariable1RangeLabel.clear();
    mpVariable1RangeEdit.clear();
    mpVariable1RangeButton.clear();
    mpVariable2RangeLabel.clear();
    mpVariable2RangeEdit.clear();
    mpVariable2RangeButton.clear();
    mpOutputRangeLabel.clear();
    mpOutputRangeEdit.clear();
    mpOutputRangeButton.clear();
    mpButtonOk.clear();
    mpGroupByColumnsRadio.clear();
    mpGroupByRowsRadio.clear();
    mpActiveEdit.clear();
    ScAnyRefDlg::dispose();
}

void ScStatisticsTwoVariableDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScStatisticsTwoVariableDialog, OkClicked ) );
    mpButtonOk->Enable(false);

    Link<Control&,void> aLink = LINK( this, ScStatisticsTwoVariableDialog, GetFocusHandler );
    mpVariable1RangeEdit->SetGetFocusHdl( aLink );
    mpVariable1RangeButton->SetGetFocusHdl( aLink );
    mpVariable2RangeEdit->SetGetFocusHdl( aLink );
    mpVariable2RangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScStatisticsTwoVariableDialog, LoseFocusHandler );
    mpVariable1RangeEdit->SetLoseFocusHdl( aLink );
    mpVariable1RangeButton->SetLoseFocusHdl( aLink );
    mpVariable2RangeEdit->SetLoseFocusHdl( aLink );
    mpVariable2RangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );

    Link<Edit&,void> aLink2 = LINK( this, ScStatisticsTwoVariableDialog, RefInputModifyHandler);
    mpVariable1RangeEdit->SetModifyHdl( aLink2);
    mpVariable2RangeEdit->SetModifyHdl( aLink2);
    mpOutputRangeEdit->SetModifyHdl( aLink2);

    mpOutputRangeEdit->GrabFocus();

    mpGroupByColumnsRadio->SetToggleHdl( LINK( this, ScStatisticsTwoVariableDialog, GroupByChanged ) );
    mpGroupByRowsRadio->SetToggleHdl( LINK( this, ScStatisticsTwoVariableDialog, GroupByChanged ) );

    mpGroupByColumnsRadio->Check();
    mpGroupByRowsRadio->Check(false);
}

void ScStatisticsTwoVariableDialog::GetRangeFromSelection()
{
    OUString aCurrentString;

    ScRange aCurrentRange;
    mViewData->GetSimpleArea(aCurrentRange);

    if (aCurrentRange.aEnd.Col() - aCurrentRange.aStart.Col() == 1)
    {
        mVariable1Range = aCurrentRange;
        mVariable1Range.aEnd.SetCol(mVariable1Range.aStart.Col());
        aCurrentString = mVariable1Range.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails);
        mpVariable1RangeEdit->SetText(aCurrentString);

        mVariable2Range = aCurrentRange;
        mVariable2Range.aStart.SetCol(mVariable2Range.aEnd.Col());
        aCurrentString = mVariable2Range.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails);
        mpVariable2RangeEdit->SetText(aCurrentString);
    }
    else
    {
        mVariable1Range = aCurrentRange;
        aCurrentString = mVariable1Range.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails);
        mpVariable1RangeEdit->SetText(aCurrentString);
    }
}

void ScStatisticsTwoVariableDialog::SetActive()
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

void ScStatisticsTwoVariableDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpActiveEdit != nullptr )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if ( mpActiveEdit == mpVariable1RangeEdit )
        {
            mVariable1Range = rReferenceRange;
            aReferenceString = mVariable1Range.Format(ScRefFlags::RANGE_ABS_3D, pDocument, mAddressDetails);
            mpVariable1RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mpVariable2RangeEdit )
        {
            mVariable2Range = rReferenceRange;
            aReferenceString = mVariable2Range.Format(ScRefFlags::RANGE_ABS_3D, pDocument, mAddressDetails);
            mpVariable2RangeEdit->SetRefString(aReferenceString);
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

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, OkClicked, Button*, void )
{
    CalculateInputAndWriteToOutput();
    Close();
}

IMPL_LINK( ScStatisticsTwoVariableDialog, GetFocusHandler, Control&, rCtrl, void )
{
    mpActiveEdit = nullptr;
    if(      &rCtrl == mpVariable1RangeEdit
          || &rCtrl == mpVariable1RangeButton )
    {
        mpActiveEdit = mpVariable1RangeEdit;
    }
    else if( &rCtrl == mpVariable2RangeEdit
          || &rCtrl == mpVariable2RangeButton )
    {
        mpActiveEdit = mpVariable2RangeEdit;
    }
    else if( &rCtrl == mpOutputRangeEdit
          || &rCtrl == mpOutputRangeButton )
    {
        mpActiveEdit = mpOutputRangeEdit;
    }

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, LoseFocusHandler, Control&, void )
{
    mDialogLostFocus = !IsActive();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, GroupByChanged, RadioButton&, void )
{
    if (mpGroupByColumnsRadio->IsChecked())
        mGroupedBy = BY_COLUMN;
    else if (mpGroupByRowsRadio->IsChecked())
        mGroupedBy = BY_ROW;

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, RefInputModifyHandler, Edit&, void )
{
    if ( mpActiveEdit )
    {
        if ( mpActiveEdit == mpVariable1RangeEdit )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mpVariable1RangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mVariable1Range = *pRange;
                // Highlight the resulting range.
                mpVariable1RangeEdit->StartUpdateData();
            }
            else
            {
                mVariable1Range = ScRange( ScAddress::INITIALIZE_INVALID);
            }
        }
        else if ( mpActiveEdit == mpVariable2RangeEdit )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mpVariable2RangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mVariable2Range = *pRange;
                // Highlight the resulting range.
                mpVariable2RangeEdit->StartUpdateData();
            }
            else
            {
                mVariable2Range = ScRange( ScAddress::INITIALIZE_INVALID);
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

void ScStatisticsTwoVariableDialog::CalculateInputAndWriteToOutput()
{
    OUString aUndo(ScResId(GetUndoNameId()));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    ScRange aOutputRange = ApplyOutput(pDocShell);

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PaintPartFlags::Grid );
}

bool ScStatisticsTwoVariableDialog::InputRangesValid()
{
    return mVariable1Range.IsValid() && mVariable2Range.IsValid() && mOutputAddress.IsValid();
}

void ScStatisticsTwoVariableDialog::ValidateDialogInput()
{
    // Enable OK button if all inputs are ok.
    if (InputRangesValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();
}

ScStatisticsTwoVariableDialogController::ScStatisticsTwoVariableDialogController(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData* pViewData, const OUString& rUIXMLDescription, const OString& rID)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent, rUIXMLDescription, rID)
    , mxVariable1RangeLabel(m_xBuilder->weld_label("variable1-range-label"))
    , mxVariable1RangeEdit(new formula::WeldRefEdit(m_xBuilder->weld_entry("variable1-range-edit")))
    , mxVariable1RangeButton(new formula::WeldRefButton(m_xBuilder->weld_button("variable1-range-button")))
    , mxVariable2RangeLabel(m_xBuilder->weld_label("variable2-range-label"))
    , mxVariable2RangeEdit(new formula::WeldRefEdit(m_xBuilder->weld_entry("variable2-range-edit")))
    , mxVariable2RangeButton(new formula::WeldRefButton(m_xBuilder->weld_button("variable2-range-button")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("output-range-label"))
    , mxOutputRangeEdit(new formula::WeldRefEdit(m_xBuilder->weld_entry("output-range-edit")))
    , mxOutputRangeButton(new formula::WeldRefButton(m_xBuilder->weld_button("output-range-button")))
    , mViewData(pViewData)
    , mDocument(pViewData->GetDocument())
    , mVariable1Range(ScAddress::INITIALIZE_INVALID)
    , mVariable2Range(ScAddress::INITIALIZE_INVALID)
    , mAddressDetails(mDocument->GetAddressConvention(), 0, 0 )
    , mOutputAddress(ScAddress::INITIALIZE_INVALID)
    , mGroupedBy(BY_COLUMN)
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxGroupByColumnsRadio(m_xBuilder->weld_radio_button("groupedby-columns-radio"))
    , mxGroupByRowsRadio(m_xBuilder->weld_radio_button("groupedby-rows-radio"))
    , mpActiveEdit(nullptr)
    , mCurrentAddress(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() )
    , mDialogLostFocus(false)
{
    mxVariable1RangeEdit->SetReferences(this, mxVariable1RangeLabel.get());
    mxVariable1RangeButton->SetReferences(this, mxVariable1RangeEdit.get());

    mxVariable2RangeEdit->SetReferences(this, mxVariable2RangeLabel.get());
    mxVariable2RangeButton->SetReferences(this, mxVariable2RangeEdit.get());

    mxOutputRangeEdit->SetReferences(this, mxOutputRangeLabel.get());
    mxOutputRangeButton->SetReferences(this, mxOutputRangeEdit.get());

    Init();
    GetRangeFromSelection();
}

ScStatisticsTwoVariableDialogController::~ScStatisticsTwoVariableDialogController()
{
}

void ScStatisticsTwoVariableDialogController::Init()
{
    mxButtonOk->connect_clicked( LINK( this, ScStatisticsTwoVariableDialogController, OkClicked ) );
    mxButtonOk->set_sensitive(false);

    Link<formula::WeldRefEdit&,void> aEditLink = LINK( this, ScStatisticsTwoVariableDialogController, GetEditFocusHandler );
    mxVariable1RangeEdit->SetGetFocusHdl( aEditLink );
    mxVariable2RangeEdit->SetGetFocusHdl( aEditLink );
    mxOutputRangeEdit->SetGetFocusHdl( aEditLink );

    Link<formula::WeldRefButton&,void> aButtonLink = LINK( this, ScStatisticsTwoVariableDialogController, GetButtonFocusHandler );
    mxVariable1RangeButton->SetGetFocusHdl( aButtonLink );
    mxVariable2RangeButton->SetGetFocusHdl( aButtonLink );
    mxOutputRangeButton->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScStatisticsTwoVariableDialogController, LoseEditFocusHandler );
    mxVariable1RangeEdit->SetLoseFocusHdl( aEditLink );
    mxVariable2RangeEdit->SetLoseFocusHdl( aEditLink );
    mxOutputRangeEdit->SetLoseFocusHdl( aEditLink );

    aButtonLink = LINK( this, ScStatisticsTwoVariableDialogController, LoseButtonFocusHandler );
    mxVariable1RangeButton->SetLoseFocusHdl( aButtonLink );
    mxVariable2RangeButton->SetLoseFocusHdl( aButtonLink );
    mxOutputRangeButton->SetLoseFocusHdl( aButtonLink );

    Link<formula::WeldRefEdit&,void> aLink2 = LINK( this, ScStatisticsTwoVariableDialogController, RefInputModifyHandler);
    mxVariable1RangeEdit->SetModifyHdl( aLink2);
    mxVariable2RangeEdit->SetModifyHdl( aLink2);
    mxOutputRangeEdit->SetModifyHdl( aLink2);

    mxOutputRangeEdit->GrabFocus();

    mxGroupByColumnsRadio->connect_toggled( LINK( this, ScStatisticsTwoVariableDialogController, GroupByChanged ) );
    mxGroupByRowsRadio->connect_toggled( LINK( this, ScStatisticsTwoVariableDialogController, GroupByChanged ) );

    mxGroupByColumnsRadio->set_active(true);
    mxGroupByRowsRadio->set_active(false);
}

void ScStatisticsTwoVariableDialogController::GetRangeFromSelection()
{
    OUString aCurrentString;

    ScRange aCurrentRange;
    mViewData->GetSimpleArea(aCurrentRange);

    if (aCurrentRange.aEnd.Col() - aCurrentRange.aStart.Col() == 1)
    {
        mVariable1Range = aCurrentRange;
        mVariable1Range.aEnd.SetCol(mVariable1Range.aStart.Col());
        aCurrentString = mVariable1Range.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails);
        mxVariable1RangeEdit->SetText(aCurrentString);

        mVariable2Range = aCurrentRange;
        mVariable2Range.aStart.SetCol(mVariable2Range.aEnd.Col());
        aCurrentString = mVariable2Range.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails);
        mxVariable2RangeEdit->SetText(aCurrentString);
    }
    else
    {
        mVariable1Range = aCurrentRange;
        aCurrentString = mVariable1Range.Format(ScRefFlags::RANGE_ABS_3D, mDocument, mAddressDetails);
        mxVariable1RangeEdit->SetText(aCurrentString);
    }
}

void ScStatisticsTwoVariableDialogController::SetActive()
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

void ScStatisticsTwoVariableDialogController::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpActiveEdit != nullptr )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if ( mpActiveEdit == mxVariable1RangeEdit.get() )
        {
            mVariable1Range = rReferenceRange;
            aReferenceString = mVariable1Range.Format(ScRefFlags::RANGE_ABS_3D, pDocument, mAddressDetails);
            mxVariable1RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mxVariable2RangeEdit.get() )
        {
            mVariable2Range = rReferenceRange;
            aReferenceString = mVariable2Range.Format(ScRefFlags::RANGE_ABS_3D, pDocument, mAddressDetails);
            mxVariable2RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mxOutputRangeEdit.get() )
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

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialogController, OkClicked, weld::Button&, void )
{
    CalculateInputAndWriteToOutput();
    response(RET_OK);
}

IMPL_LINK(ScStatisticsTwoVariableDialogController, GetEditFocusHandler, formula::WeldRefEdit&, rCtrl, void)
{
    mpActiveEdit = nullptr;
    if (&rCtrl == mxVariable1RangeEdit.get())
    {
        mpActiveEdit = mxVariable1RangeEdit.get();
    }
    else if (&rCtrl == mxVariable2RangeEdit.get())
    {
        mpActiveEdit = mxVariable2RangeEdit.get();
    }
    else if (&rCtrl == mxOutputRangeEdit.get())
    {
        mpActiveEdit = mxOutputRangeEdit.get();
    }

    if( mpActiveEdit )
        mpActiveEdit->SelectAll();
}

IMPL_LINK( ScStatisticsTwoVariableDialogController, GetButtonFocusHandler, formula::WeldRefButton&, rCtrl, void )
{
    mpActiveEdit = nullptr;
    if (&rCtrl == mxVariable1RangeButton.get())
    {
        mpActiveEdit = mxVariable1RangeEdit.get();
    }
    else if (&rCtrl == mxVariable2RangeButton.get())
    {
        mpActiveEdit = mxVariable2RangeEdit.get();
    }
    else if (&rCtrl == mxOutputRangeButton.get())
    {
        mpActiveEdit = mxOutputRangeEdit.get();
    }

    if( mpActiveEdit )
        mpActiveEdit->SelectAll();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialogController, LoseEditFocusHandler, formula::WeldRefEdit&, void )
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialogController, LoseButtonFocusHandler, formula::WeldRefButton&, void )
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScStatisticsTwoVariableDialogController, GroupByChanged, weld::ToggleButton&, void)
{
    if (mxGroupByColumnsRadio->get_active())
        mGroupedBy = BY_COLUMN;
    else if (mxGroupByRowsRadio->get_active())
        mGroupedBy = BY_ROW;

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialogController, RefInputModifyHandler, formula::WeldRefEdit&, void )
{
    if ( mpActiveEdit )
    {
        if (mpActiveEdit == mxVariable1RangeEdit.get())
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mxVariable1RangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mVariable1Range = *pRange;
                // Highlight the resulting range.
                mxVariable1RangeEdit->StartUpdateData();
            }
            else
            {
                mVariable1Range = ScRange( ScAddress::INITIALIZE_INVALID);
            }
        }
        else if ( mpActiveEdit == mxVariable2RangeEdit.get() )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mxVariable2RangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                mVariable2Range = *pRange;
                // Highlight the resulting range.
                mxVariable2RangeEdit->StartUpdateData();
            }
            else
            {
                mVariable2Range = ScRange( ScAddress::INITIALIZE_INVALID);
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

void ScStatisticsTwoVariableDialogController::CalculateInputAndWriteToOutput()
{
    OUString aUndo(ScResId(GetUndoNameId()));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    ScRange aOutputRange = ApplyOutput(pDocShell);

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PaintPartFlags::Grid );
}

bool ScStatisticsTwoVariableDialogController::InputRangesValid()
{
    return mVariable1Range.IsValid() && mVariable2Range.IsValid() && mOutputAddress.IsValid();
}

void ScStatisticsTwoVariableDialogController::ValidateDialogInput()
{
    // Enable OK button if all inputs are ok.
    mxButtonOk->set_sensitive(InputRangesValid());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
