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
#include "docfunc.hxx"
#include "strload.hxx"

#include "StatisticsTwoVariableDialog.hxx"

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

    // Enable OK if all ranges are set.
    if (mVariable1Range.IsValid() && mVariable2Range.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();
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
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, RefInputModifyHandler, Edit&, void )
{
    if ( mpActiveEdit )
    {
        if ( mpActiveEdit == mpVariable1RangeEdit )
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames( aRangeList, mpVariable1RangeEdit->GetText(), mDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? aRangeList[0] : nullptr;
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
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? aRangeList[0] : nullptr;
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

                // Highlight the resulting range.
                mpOutputRangeEdit->StartUpdateData();
            }
            else
            {
                mOutputAddress = ScAddress( ScAddress::INITIALIZE_INVALID);
            }
        }
    }

    // Enable OK if all ranges are set.
    if (mVariable1Range.IsValid() && mVariable2Range.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();
}

void ScStatisticsTwoVariableDialog::CalculateInputAndWriteToOutput()
{
    OUString aUndo(SC_RESSTR(GetUndoNameId()));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo, 0, mViewData->GetViewShell()->GetViewShellId() );

    ScRange aOutputRange = ApplyOutput(pDocShell);

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PaintPartFlags::Grid );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
