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
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
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

    Link<> aLink = LINK( this, ScStatisticsInputOutputDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScStatisticsInputOutputDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl( aLink );
    mpInputRangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );

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

            // Enable OK, Cancel if output range is set
            mpButtonOk->Enable(!mpOutputRangeEdit->GetText().isEmpty());
        }
    }
}

IMPL_LINK( ScStatisticsInputOutputDialog, OkClicked, PushButton*, /*pButton*/ )
{
    CalculateInputAndWriteToOutput();
    Close();
    return 0;
}

IMPL_LINK( ScStatisticsInputOutputDialog, GetFocusHandler, Control*, pCtrl )
{
    mpActiveEdit = NULL;

    if(      (pCtrl == static_cast<Control*>(mpInputRangeEdit))  || (pCtrl == static_cast<Control*>(mpInputRangeButton)) )
        mpActiveEdit = mpInputRangeEdit;
    else if( (pCtrl == static_cast<Control*>(mpOutputRangeEdit)) || (pCtrl == static_cast<Control*>(mpOutputRangeButton)) )
        mpActiveEdit = mpOutputRangeEdit;

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, LoseFocusHandler )
{
    mDialogLostFocus = !IsActive();
    return 0;
}

IMPL_LINK_NOARG( ScStatisticsInputOutputDialog, GroupByChanged )
{
    if (mpGroupByColumnsRadio->IsChecked())
        mGroupedBy = BY_COLUMN;
    else if (mpGroupByRowsRadio->IsChecked())
        mGroupedBy = BY_ROW;

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
