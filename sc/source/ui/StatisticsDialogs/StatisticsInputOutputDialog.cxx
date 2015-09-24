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

#include "StatisticsInputOutputDialog.hxx"

ScRangeList ScStatisticsInputOutputDialog::MakeColumnRangeList(SCTAB aTab, ScAddress aStart, ScAddress aEnd)
{
    ScRangeList aRangeList;
    for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
    {
        ScRange aColumnRange (
            ScAddress(inCol, aStart.Row(), aTab),
            ScAddress(inCol, aEnd.Row(),   aTab) );

        aRangeList.Append(aColumnRange);
    }
    return aRangeList;
}

ScRangeList ScStatisticsInputOutputDialog::MakeRowRangeList(SCTAB aTab, ScAddress aStart, ScAddress aEnd)
{
    ScRangeList aRangeList;
    for (SCROW inRow = aStart.Row(); inRow <= aEnd.Row(); inRow++)
    {
        ScRange aRowRange (
            ScAddress(aStart.Col(), inRow, aTab),
            ScAddress(aEnd.Col(),   inRow, aTab) );

        aRangeList.Append(aRowRange);
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
    mpActiveEdit    ( NULL ),
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

    Link<> aLink2 = LINK( this, ScStatisticsInputOutputDialog, RefInputModifyHandler);
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
    OUString aCurrentString(mInputRange.Format(SCR_ABS_3D, mDocument, mAddressDetails));
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
            aReferenceString = mInputRange.Format(SCR_ABS_3D, pDocument, mAddressDetails);
            mpInputRangeEdit->SetRefString( aReferenceString );
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            mOutputAddress = rReferenceRange.aStart;

            sal_uInt16 nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ? SCA_ABS : SCA_ABS_3D;
            aReferenceString = mOutputAddress.Format(nFormat, pDocument, pDocument->GetAddressConvention());
            mpOutputRangeEdit->SetRefString( aReferenceString );
        }
    }

    // Enable OK if both, input range and output address are set.
    if (mInputRange.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();
}

IMPL_LINK_NOARG_TYPED( ScStatisticsInputOutputDialog, OkClicked, Button*, void )
{
    CalculateInputAndWriteToOutput();
    Close();
}

IMPL_LINK_TYPED( ScStatisticsInputOutputDialog, GetFocusHandler, Control&, rCtrl, void )
{
    mpActiveEdit = NULL;

    if(      (&rCtrl == static_cast<Control*>(mpInputRangeEdit))  || (&rCtrl == static_cast<Control*>(mpInputRangeButton)) )
        mpActiveEdit = mpInputRangeEdit;
    else if( (&rCtrl == static_cast<Control*>(mpOutputRangeEdit)) || (&rCtrl == static_cast<Control*>(mpOutputRangeButton)) )
        mpActiveEdit = mpOutputRangeEdit;

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG_TYPED( ScStatisticsInputOutputDialog, LoseFocusHandler, Control&, void )
{
    mDialogLostFocus = !IsActive();
}

IMPL_LINK_NOARG_TYPED( ScStatisticsInputOutputDialog, GroupByChanged, RadioButton&, void )
{
    if (mpGroupByColumnsRadio->IsChecked())
        mGroupedBy = BY_COLUMN;
    else if (mpGroupByRowsRadio->IsChecked())
        mGroupedBy = BY_ROW;
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, RefInputModifyHandler )
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
                    sal_uInt16 nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ? SCA_ABS : SCA_ABS_3D;
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

    // Enable OK if both, input range and output address are set.
    if (mInputRange.IsValid() && mOutputAddress.IsValid())
        mpButtonOk->Enable();
    else
        mpButtonOk->Disable();

    return 0;
}

void ScStatisticsInputOutputDialog::CalculateInputAndWriteToOutput()
{
    OUString aUndo(SC_STRLOAD(RID_STATISTICS_DLGS, GetUndoNameId()));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScRange aOutputRange = ApplyOutput(pDocShell);

    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
