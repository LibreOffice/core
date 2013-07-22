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

#include "StatisticsInputOutputDialog.hxx"

ScStatisticsInputOutputDialog::ScStatisticsInputOutputDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData, const OString& rID, const OUString& rUIXMLDescription ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent, rID, rUIXMLDescription ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
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
    get(mpButtonApply,  "apply");
    get(mpButtonClose,  "close");

    Init();
    GetRangeFromSelection();
}

ScStatisticsInputOutputDialog::~ScStatisticsInputOutputDialog()
{}

void ScStatisticsInputOutputDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScStatisticsInputOutputDialog, OkClicked ) );
    mpButtonClose->SetClickHdl( LINK( this, ScStatisticsInputOutputDialog, CloseClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScStatisticsInputOutputDialog, ApplyClicked ) );
    mpButtonOk->Enable(false);
    mpButtonApply->Enable(false);

    Link aLink = LINK( this, ScStatisticsInputOutputDialog, GetFocusHandler );
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
}

void ScStatisticsInputOutputDialog::GetRangeFromSelection()
{
    OUString aCurrentString;
    mViewData->GetSimpleArea(mInputRange);
    mInputRange.Format(aCurrentString, SCR_ABS_3D, mDocument, mAddressDetails);
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

        String aReferenceString;

        if ( mpActiveEdit == mpInputRangeEdit )
        {
            mInputRange = rReferenceRange;
            mInputRange.Format( aReferenceString, SCR_ABS_3D, pDocument, mAddressDetails );
            mpInputRangeEdit->SetRefString( aReferenceString );
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            mOutputAddress = rReferenceRange.aStart;

            sal_uInt16 nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ? SCA_ABS : SCA_ABS_3D;
            mOutputAddress.Format( aReferenceString, nFormat, pDocument, pDocument->GetAddressConvention() );
            mpOutputRangeEdit->SetRefString( aReferenceString );

            // Enable OK, Cancel if output range is set
            mpButtonOk->Enable(!mpOutputRangeEdit->GetText().isEmpty());
            mpButtonApply->Enable(!mpOutputRangeEdit->GetText().isEmpty());
        }
    }
}

IMPL_LINK( ScStatisticsInputOutputDialog, OkClicked, PushButton*, /*pButton*/ )
{
    ApplyClicked(NULL);
    CloseClicked(NULL);
    return 0;
}


IMPL_LINK( ScStatisticsInputOutputDialog, ApplyClicked, PushButton*, /*pButton*/ )
{
    CalculateInputAndWriteToOutput();
    return 0;
}

IMPL_LINK( ScStatisticsInputOutputDialog, CloseClicked, PushButton*, /*pButton*/ )
{
    Close();
    return 0;
}

IMPL_LINK( ScStatisticsInputOutputDialog, GetFocusHandler, Control*, pCtrl )
{
    mpActiveEdit = NULL;

    if(      (pCtrl == (Control*) mpInputRangeEdit)  || (pCtrl == (Control*) mpInputRangeButton) )
        mpActiveEdit = mpInputRangeEdit;
    else if( (pCtrl == (Control*) mpOutputRangeEdit) || (pCtrl == (Control*) mpOutputRangeButton) )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
