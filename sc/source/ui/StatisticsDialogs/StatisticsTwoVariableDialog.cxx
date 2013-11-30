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
#include "random.hxx"
#include "docfunc.hxx"
#include "strload.hxx"

#include "StatisticsTwoVariableDialog.hxx"

ScStatisticsTwoVariableDialog::ScStatisticsTwoVariableDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData, const OString& rID, const OUString& rUIXMLDescription ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent, rID, rUIXMLDescription ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mGroupedBy      ( BY_COLUMN ),
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
    get(mpButtonApply,  "apply");
    get(mpButtonClose,  "close");

    get(mpGroupByColumnsRadio,   "groupedby-columns-radio");
    get(mpGroupByRowsRadio,      "groupedby-rows-radio");

    Init();
    GetRangeFromSelection();
}

ScStatisticsTwoVariableDialog::~ScStatisticsTwoVariableDialog()
{}

void ScStatisticsTwoVariableDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScStatisticsTwoVariableDialog, OkClicked ) );
    mpButtonClose->SetClickHdl( LINK( this, ScStatisticsTwoVariableDialog, CloseClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScStatisticsTwoVariableDialog, ApplyClicked ) );
    mpButtonOk->Enable(false);
    mpButtonApply->Enable(false);

    Link aLink = LINK( this, ScStatisticsTwoVariableDialog, GetFocusHandler );
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

    mpOutputRangeEdit->GrabFocus();

    mpGroupByColumnsRadio->SetToggleHdl( LINK( this, ScStatisticsTwoVariableDialog, GroupByChanged ) );
    mpGroupByRowsRadio->SetToggleHdl( LINK( this, ScStatisticsTwoVariableDialog, GroupByChanged ) );

    mpGroupByColumnsRadio->Check(true);
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
        aCurrentString = mVariable1Range.Format(SCR_ABS_3D, mDocument, mAddressDetails);
        mpVariable1RangeEdit->SetText(aCurrentString);

        mVariable2Range = aCurrentRange;
        mVariable2Range.aStart.SetCol(mVariable2Range.aEnd.Col());
        aCurrentString = mVariable2Range.Format(SCR_ABS_3D, mDocument, mAddressDetails);
        mpVariable2RangeEdit->SetText(aCurrentString);
    }
    else
    {
        mVariable1Range = aCurrentRange;
        aCurrentString = mVariable1Range.Format(SCR_ABS_3D, mDocument, mAddressDetails);
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
    if ( mpActiveEdit != NULL )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        OUString aReferenceString;

        if ( mpActiveEdit == mpVariable1RangeEdit )
        {
            mVariable1Range = rReferenceRange;
            aReferenceString = mVariable1Range.Format(SCR_ABS_3D, pDocument, mAddressDetails);
            mpVariable1RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mpVariable2RangeEdit )
        {
            mVariable2Range = rReferenceRange;
            aReferenceString = mVariable2Range.Format(SCR_ABS_3D, pDocument, mAddressDetails);
            mpVariable2RangeEdit->SetRefString(aReferenceString);
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            mOutputAddress = rReferenceRange.aStart;

            sal_uInt16 nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ? SCA_ABS : SCA_ABS_3D;
            aReferenceString = mOutputAddress.Format(nFormat, pDocument, pDocument->GetAddressConvention());
            mpOutputRangeEdit->SetRefString( aReferenceString );

            // Enable OK, Cancel if output range is set
            mpButtonOk->Enable(!mpOutputRangeEdit->GetText().isEmpty());
            mpButtonApply->Enable(!mpOutputRangeEdit->GetText().isEmpty());
        }
    }
}

IMPL_LINK( ScStatisticsTwoVariableDialog, OkClicked, PushButton*, /*pButton*/ )
{
    ApplyClicked(NULL);
    CloseClicked(NULL);
    return 0;
}


IMPL_LINK( ScStatisticsTwoVariableDialog, ApplyClicked, PushButton*, /*pButton*/ )
{
    CalculateInputAndWriteToOutput();
    return 0;
}

IMPL_LINK( ScStatisticsTwoVariableDialog, CloseClicked, PushButton*, /*pButton*/ )
{
    Close();
    return 0;
}

IMPL_LINK( ScStatisticsTwoVariableDialog, GetFocusHandler, Control*, pCtrl )
{
    mpActiveEdit = NULL;
    if(      pCtrl == mpVariable1RangeEdit
          || pCtrl == mpVariable1RangeButton )
    {
        mpActiveEdit = mpVariable1RangeEdit;
    }
    else if( pCtrl == mpVariable2RangeEdit
          || pCtrl == mpVariable2RangeButton )
    {
        mpActiveEdit = mpVariable2RangeEdit;
    }
    else if( pCtrl == mpOutputRangeEdit
          || pCtrl == mpOutputRangeButton )
    {
        mpActiveEdit = mpOutputRangeEdit;
    }

    if( mpActiveEdit )
        mpActiveEdit->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, LoseFocusHandler )
{
    mDialogLostFocus = !IsActive();
    return 0;
}

IMPL_LINK_NOARG( ScStatisticsTwoVariableDialog, GroupByChanged )
{
    if (mpGroupByColumnsRadio->IsChecked())
        mGroupedBy = BY_COLUMN;
    else if (mpGroupByRowsRadio->IsChecked())
        mGroupedBy = BY_ROW;

    return 0;
}

void ScStatisticsTwoVariableDialog::CalculateInputAndWriteToOutput()
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
