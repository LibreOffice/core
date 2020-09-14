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
                    weld::Window* pParent, ScViewData* pViewData, const OUString& rUIXMLDescription, const OString& rID)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent, rUIXMLDescription, rID)
    , mxVariable1RangeLabel(m_xBuilder->weld_label("variable1-range-label"))
    , mxVariable1RangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("variable1-range-edit")))
    , mxVariable1RangeButton(new formula::RefButton(m_xBuilder->weld_button("variable1-range-button")))
    , mxVariable2RangeLabel(m_xBuilder->weld_label("variable2-range-label"))
    , mxVariable2RangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("variable2-range-edit")))
    , mxVariable2RangeButton(new formula::RefButton(m_xBuilder->weld_button("variable2-range-button")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("output-range-label"))
    , mxOutputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("output-range-edit")))
    , mxOutputRangeButton(new formula::RefButton(m_xBuilder->weld_button("output-range-button")))
    , mViewData(pViewData)
    , mDocument(pViewData->GetDocument())
    , mVariable1Range(ScAddress::INITIALIZE_INVALID)
    , mVariable2Range(ScAddress::INITIALIZE_INVALID)
    , mAddressDetails(mDocument.GetAddressConvention(), 0, 0 )
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

ScStatisticsTwoVariableDialog::~ScStatisticsTwoVariableDialog()
{
}

void ScStatisticsTwoVariableDialog::Init()
{
    mxButtonOk->connect_clicked( LINK( this, ScStatisticsTwoVariableDialog, OkClicked ) );
    mxButtonOk->set_sensitive(false);

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScStatisticsTwoVariableDialog, GetEditFocusHandler );
    mxVariable1RangeEdit->SetGetFocusHdl( aEditLink );
    mxVariable2RangeEdit->SetGetFocusHdl( aEditLink );
    mxOutputRangeEdit->SetGetFocusHdl( aEditLink );

    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScStatisticsTwoVariableDialog, GetButtonFocusHandler );
    mxVariable1RangeButton->SetGetFocusHdl( aButtonLink );
    mxVariable2RangeButton->SetGetFocusHdl( aButtonLink );
    mxOutputRangeButton->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScStatisticsTwoVariableDialog, LoseEditFocusHandler );
    mxVariable1RangeEdit->SetLoseFocusHdl( aEditLink );
    mxVariable2RangeEdit->SetLoseFocusHdl( aEditLink );
    mxOutputRangeEdit->SetLoseFocusHdl( aEditLink );

    aButtonLink = LINK( this, ScStatisticsTwoVariableDialog, LoseButtonFocusHandler );
    mxVariable1RangeButton->SetLoseFocusHdl( aButtonLink );
    mxVariable2RangeButton->SetLoseFocusHdl( aButtonLink );
    mxOutputRangeButton->SetLoseFocusHdl( aButtonLink );

    Link<formula::RefEdit&,void> aLink2 = LINK( this, ScStatisticsTwoVariableDialog, RefInputModifyHandler);
    mxVariable1RangeEdit->SetModifyHdl( aLink2);
    mxVariable2RangeEdit->SetModifyHdl( aLink2);
    mxOutputRangeEdit->SetModifyHdl( aLink2);

    mxOutputRangeEdit->GrabFocus();

    mxGroupByColumnsRadio->connect_toggled( LINK( this, ScStatisticsTwoVariableDialog, GroupByChanged ) );
    mxGroupByRowsRadio->connect_toggled( LINK( this, ScStatisticsTwoVariableDialog, GroupByChanged ) );

    mxGroupByColumnsRadio->set_active(true);
    mxGroupByRowsRadio->set_active(false);
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
        aCurrentString = mVariable1Range.Format(mDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
        mxVariable1RangeEdit->SetText(aCurrentString);

        mVariable2Range = aCurrentRange;
        mVariable2Range.aStart.SetCol(mVariable2Range.aEnd.Col());
        aCurrentString = mVariable2Range.Format(mDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
        mxVariable2RangeEdit->SetText(aCurrentString);
    }
    else
    {
        mVariable1Range = aCurrentRange;
        aCurrentString = mVariable1Range.Format(mDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
        mxVariable1RangeEdit->SetText(aCurrentString);
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
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void ScStatisticsTwoVariableDialog::SetReference( const ScRange& rReferenceRange, ScDocument& rDocument )
{
    if ( mpActiveEdit != nullptr )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if ( mpActiveEdit == mxVariable1RangeEdit.get() )
        {
            mVariable1Range = rReferenceRange;
            aReferenceString = mVariable1Range.Format(rDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
            mxVariable1RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mxVariable2RangeEdit.get() )
        {
            mVariable2Range = rReferenceRange;
            aReferenceString = mVariable2Range.Format(rDocument, ScRefFlags::RANGE_ABS_3D, mAddressDetails);
            mxVariable2RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mxOutputRangeEdit.get() )
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

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, OkClicked, weld::Button&, void )
{
    CalculateInputAndWriteToOutput();
    response(RET_OK);
}

IMPL_LINK(ScStatisticsTwoVariableDialog, GetEditFocusHandler, formula::RefEdit&, rCtrl, void)
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

IMPL_LINK( ScStatisticsTwoVariableDialog, GetButtonFocusHandler, formula::RefButton&, rCtrl, void )
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

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, LoseEditFocusHandler, formula::RefEdit&, void )
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, LoseButtonFocusHandler, formula::RefButton&, void )
{
    mDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScStatisticsTwoVariableDialog, GroupByChanged, weld::ToggleButton&, void)
{
    if (mxGroupByColumnsRadio->get_active())
        mGroupedBy = BY_COLUMN;
    else if (mxGroupByRowsRadio->get_active())
        mGroupedBy = BY_ROW;

    ValidateDialogInput();
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, RefInputModifyHandler, formula::RefEdit&, void )
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
    mxButtonOk->set_sensitive(InputRangesValid());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
