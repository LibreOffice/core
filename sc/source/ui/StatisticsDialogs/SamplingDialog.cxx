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

#include "rangelst.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "random.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"

#include <boost/random.hpp>

#include "SamplingDialog.hxx"

#define ABS_DREF3D SCA_VALID | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE | SCA_TAB_3D

namespace
{
}

ScSamplingDialog::ScSamplingDialog(
        SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, ScViewData* pViewData ) :
    ScAnyRefDlg     ( pB, pCW, pParent, "SamplingDialog", "modules/scalc/ui/samplingdialog.ui" ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mCurrentAddress ( pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo() ),
    mDialogLostFocus( false )
{
    get(mpInputRangeLabel, "input-range-label");
    get(mpInputRangeEdit, "input-range-edit");
    mpInputRangeEdit->SetReferences(this, mpInputRangeLabel);
    get(mpInputRangeButton, "input-range-button");
    mpInputRangeButton->SetReferences(this, mpInputRangeEdit);

    get(mpOutputRangeLabel, "output-range-label");
    get(mpOutputRangeEdit, "output-range-edit");
    mpOutputRangeEdit->SetReferences(this, mpOutputRangeLabel);
    get(mpOutputRangeButton, "output-range-button");
    mpOutputRangeButton->SetReferences(this, mpOutputRangeEdit);

    get(mpButtonOk,     "ok");
    get(mpButtonApply,  "apply");
    get(mpButtonCancel, "cancel");

    Init();
    GetRangeFromSelection();
}

void ScSamplingDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScSamplingDialog, OkClicked ) );
    mpButtonCancel->SetClickHdl( LINK( this, ScSamplingDialog, CancelClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScSamplingDialog, ApplyClicked ) );

    Link aLink = LINK( this, ScSamplingDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );
    mpOutputRangeEdit->SetGetFocusHdl( aLink );
    mpOutputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScSamplingDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl( aLink );
    mpInputRangeButton->SetLoseFocusHdl( aLink );
    mpOutputRangeEdit->SetLoseFocusHdl( aLink );
    mpOutputRangeButton->SetLoseFocusHdl( aLink );
}

void ScSamplingDialog::GetRangeFromSelection()
{
    OUString aCurrentString;
    mViewData->GetSimpleArea(mInputRange);
    mInputRange.Format( aCurrentString, ABS_DREF3D, mDocument, mAddressDetails );
    mpInputRangeEdit->SetText( aCurrentString );
}


ScSamplingDialog::~ScSamplingDialog()
{
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
        GrabFocus();
    }
    RefInputDone();
}

sal_Bool ScSamplingDialog::Close()
{
    return DoClose( ScSamplingDialogWrapper::GetChildWindowId() );
}

void ScSamplingDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpActiveEdit )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpActiveEdit );

        String aReferenceString;

        if ( mpActiveEdit == mpInputRangeEdit )
        {
            mInputRange = rReferenceRange;
            mInputRange.Format( aReferenceString, ABS_DREF3D, pDocument, mAddressDetails );
            mpInputRangeEdit->SetRefString( aReferenceString );
        }
        else if ( mpActiveEdit == mpOutputRangeEdit )
        {
            mOutputAddress = rReferenceRange.aStart;
            sal_uInt16  nFormat = ( mOutputAddress.Tab() == mCurrentAddress.Tab() ) ? SCA_ABS : SCA_ABS_3D;
            mOutputAddress.Format( aReferenceString, nFormat, pDocument, pDocument->GetAddressConvention() );
            mpActiveEdit->SetRefString( aReferenceString );
        }
    }
}

void ScSamplingDialog::PerformSampling()
{
    OUString aUndo("A");
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    SCCOL nColStart = mInputRange.aStart.Col();
    SCCOL nColEnd   = mInputRange.aEnd.Col();
    SCROW nRowStart = mInputRange.aStart.Row();
    SCROW nRowEnd   = mInputRange.aEnd.Row();
    SCTAB nTabStart = mInputRange.aStart.Tab();
    SCTAB nTabEnd   = mInputRange.aEnd.Tab();

    TimeValue now;
    osl_getSystemTime(&now);
    boost::mt19937 seed(now.Nanosec);
    boost::uniform_01<boost::mt19937> rng(seed);

    SCTAB nOutTab = mOutputAddress.Tab();
    SCCOL nOutCol = mOutputAddress.Col();
    SCROW nOutRow = mOutputAddress.Row();
    for (SCROW nTab = nTabStart; nTab <= nTabEnd; nTab++)
    {
        nOutCol = mOutputAddress.Col();
        for (SCCOL nCol = nColStart; nCol <= nColEnd; nCol++)
        {
            nOutRow = mOutputAddress.Row();
            SCROW nRow = nRowStart;

            SCROW t = 0;
            SCROW N = (nRowEnd - nRowStart) + 1;
            int m = 0;
            int n = 3;
            double u;

            while (m < n)
            {
                u = rng();
                if ( (N - t)*u >= n - m )
                {
                    nRow++;
                    t++;
                }
                else
                {
                    double aValue = mDocument->GetValue( ScAddress(nCol, nRow, nTab) );
                    pDocShell->GetDocFunc().SetValueCell(ScAddress(nOutCol, nOutRow, nOutTab), aValue, true);
                    nRow++;
                    nOutRow++;
                    m++;
                    t++;
                }
            }

            nOutCol++;
        }
        nOutTab++;
    }

    ScRange aOutputRange(mOutputAddress, ScAddress(nOutCol,   nOutRow,   nOutTab) );

    pUndoManager->LeaveListAction();

    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

IMPL_LINK( ScSamplingDialog, OkClicked, PushButton*, /*pButton*/ )
{
    PerformSampling();
    Close();
    return 0;
}


IMPL_LINK( ScSamplingDialog, ApplyClicked, PushButton*, /*pButton*/ )
{
    PerformSampling();
    return 0;
}

IMPL_LINK( ScSamplingDialog, CancelClicked, PushButton*, /*pButton*/ )
{
    Close();
    return 0;
}

IMPL_LINK( ScSamplingDialog, GetFocusHandler, Control*, pCtrl )
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

IMPL_LINK_NOARG(ScSamplingDialog, LoseFocusHandler)
{
    mDialogLostFocus = !IsActive();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
