/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unitsconversiondlg.hxx"

#include "document.hxx"
#include "reffact.hxx"
#include "units.hxx"
#include "viewdata.hxx"
#include "docsh.hxx"

using namespace sc::units;

ScUnitsConversionDialog::ScUnitsConversionDialog( SfxBindings* pB,
                                                  SfxChildWindow* pCW,
                                                  vcl::Window* pParent,
                                                  ScViewData* pViewData)
    : ScAnyRefDlg( pB, pCW, pParent, "UnitsConversionDialog",
                   "modules/scalc/ui/unitsconversiondialog.ui" )
    , mpViewData( pViewData )
    , mpDoc( pViewData->GetDocument() )
    , mInputRange( new ScRangeList )
    , mbDialogLostFocus( false )
{
    mpUnits = Units::GetUnits();

    get( mpButtonOk, "ok" );

    get( mpInputRangeLabel, "cell-range-label" );
    get( mpInputRangeEdit, "cell-range-edit" );
    get( mpInputRangeButton, "cell-range-button" );
    mpInputRangeEdit->SetReferences( this, mpInputRangeLabel );
    mpInputRangeButton->SetReferences( this, mpInputRangeEdit );

    get( mpLabelInputUnits, "inputunits" );
    get( mpIncompatibleInputsBox, "incompatible-inputs-box" );

    get( mpOutputUnitsEdit, "output-units-edit" );
    get( mpIncompatibleOutputBox, "incompatible-output-box" );

    Init();

    msNone = mpLabelInputUnits->GetText();
    GetRangeFromSelection();
    UpdateInputUnits();
}

ScUnitsConversionDialog::~ScUnitsConversionDialog()
{
    disposeOnce();
}

void ScUnitsConversionDialog::dispose()
{
    mpButtonOk.clear();

    mpInputRangeLabel.clear();
    mpInputRangeEdit.clear();
    mpInputRangeButton.clear();

    mpLabelInputUnits.clear();
    mpIncompatibleInputsBox.clear();

    mpOutputUnitsEdit.clear();
    mpIncompatibleOutputBox.clear();

    ScAnyRefDlg::dispose();
}

void ScUnitsConversionDialog::Init()
{
    Link<> aLink = LINK( this, ScUnitsConversionDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScUnitsConversionDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl ( aLink );
    mpInputRangeButton->SetLoseFocusHdl ( aLink );

    mpOutputUnitsEdit->SetModifyHdl( LINK( this, ScUnitsConversionDialog, OutputUnitsModified ) );

    mpOutputUnitsEdit->SetGetFocusHdl( LINK( this, ScUnitsConversionDialog, OutputUnitsGetFocusHandler ) );
    mpOutputUnitsEdit->SetLoseFocusHdl( LINK( this, ScUnitsConversionDialog, OutputUnitsLoseFocusHandler ) );

    mpButtonOk->SetClickHdl( LINK( this, ScUnitsConversionDialog, OkClicked ) );
}

void ScUnitsConversionDialog::GetRangeFromSelection()
{
    mpViewData->GetMultiArea( mInputRange );
    OUString sCurrentString;
    mInputRange->Format( sCurrentString, SCA_VALID, mpDoc, mpDoc->GetAddressConvention() );
    mpInputRangeEdit->SetText( sCurrentString );
}

void ScUnitsConversionDialog::UpdateInputUnits()
{
    mUnits = mpUnits->getUnitsForRange( *mInputRange, mpDoc );

    if ( mUnits.units.size() > 0 )
    {
        OUStringBuffer mUnitsBuf;

        for ( OUString sUnit : mUnits.units )
        {
            // Test that the buffer has already got a unit, so that
            // we only add the separator after units (but not at the
            // beginnning).
            if ( !mUnitsBuf.isEmpty() )
            {
                mUnitsBuf.append( ", " );
            }
            mUnitsBuf.append( sUnit );
        }
        mpLabelInputUnits->SetText( mUnitsBuf.makeStringAndClear() );
    }
    else
    {
        mpLabelInputUnits->SetText( msNone );
    }

    mpIncompatibleInputsBox->Show( !mUnits.compatible );
}

void ScUnitsConversionDialog::SetReference( const ScRange& rRange, ScDocument* pDoc )
{
    if ( mpInputRangeEdit->IsEnabled() )
    {
        if ( rRange.aStart != rRange.aEnd )
        {
            RefInputStart( mpInputRangeEdit );
        }

        mInputRange->RemoveAll();
        mInputRange->Append(rRange);

        OUString aRangeString = rRange.Format( SCA_VALID, pDoc, pDoc->GetAddressConvention() );
        mpInputRangeEdit->SetRefString( aRangeString );

        UpdateInputUnits();
    }
}

void ScUnitsConversionDialog::SetActive()
{
    if ( mbDialogLostFocus )
    {
        mbDialogLostFocus = false;
        if( mpInputRangeEdit )
        {
            mpInputRangeEdit->GrabFocus();
        }
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

bool ScUnitsConversionDialog::Close()
{
    return DoClose( ScUnitsConversionDialogWrapper::GetChildWindowId() );
}

bool ScUnitsConversionDialog::CheckUnitsAreConvertible()
{
    OUString sOutputUnit = mpOutputUnitsEdit->GetText();

    bool bCompatibleInputFound = false;
    for ( auto aIt = mUnits.units.cbegin(); aIt < mUnits.units.cend(); aIt++ )
    {
        if ( mpUnits->areUnitsCompatible( *aIt, sOutputUnit ) ) {
            bCompatibleInputFound = true;
            break;
        }

        // Optimisation: break after first unit if all input units are compatible, since
        // the output will either be compatible with all or none of the inputs.
        if ( mUnits.compatible )
        {
            break;
        }
    }

    return bCompatibleInputFound;
}

IMPL_LINK_TYPED( ScUnitsConversionDialog, OkClicked, Button*, /*pButton*/, void )
{
    if (!CheckUnitsAreConvertible())
    {
        // As we have now clicked on this button, the output units entry
        // box has lost focus, so the "no conversion possible" warning
        // will already be shown by the OutputUnitsComplete handler.
        return;
    }

    PerformConversion();
    Close();
}

IMPL_LINK( ScUnitsConversionDialog, GetFocusHandler, Control*, pCtrl )
{
    Edit* pEdit = NULL;

    if( (pCtrl == mpInputRangeEdit.get()) || (pCtrl == mpInputRangeButton.get()) ) {
        pEdit = mpInputRangeEdit;
    }

    if( pEdit ) {
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
    }

    return 0;
}

IMPL_LINK_NOARG( ScUnitsConversionDialog, LoseFocusHandler )
{
    mbDialogLostFocus = !IsActive();
    return 0;
}

IMPL_LINK_NOARG( ScUnitsConversionDialog, OutputUnitsGetFocusHandler )
{
    // The warning box may have been enabled because of an incompatible unit,
    // however we should disable it during editing (it will then be reenabled
    // if needed once the user completes editing their desired output unit).
    mpIncompatibleOutputBox->Show( false );

    return 0;
}

IMPL_LINK_NOARG( ScUnitsConversionDialog, OutputUnitsModified )
{
    OUString sOutputUnit = mpOutputUnitsEdit->GetText();

    if (!mpUnits->isValidUnit(sOutputUnit))
    {
        mpOutputUnitsEdit->SetControlForeground( Color( COL_LIGHTRED ) );
        mpOutputUnitsEdit->set_font_attribute( "underline", "true" );
    }
    else
    {
        mpOutputUnitsEdit->SetControlForeground( Color( COL_BLACK ) );
        mpOutputUnitsEdit->set_font_attribute( "underline", "false" );
    }

    return 0;
}

IMPL_LINK_NOARG( ScUnitsConversionDialog, OutputUnitsLoseFocusHandler )
{
    mpIncompatibleOutputBox->Show( !CheckUnitsAreConvertible() );

    return 0;
}

void ScUnitsConversionDialog::PerformConversion()
{
    OUString sOutputUnit = mpOutputUnitsEdit->GetText();

    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDoc->GetDocumentShell());
    ScDocShellModificator aModificator(*pDocShell);
    mpUnits->convertCellUnits( *mInputRange, mpDoc, sOutputUnit );
    pDocShell->PostPaint(*mInputRange, PAINT_GRID);

    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast(SfxSimpleHint(SC_HINT_AREAS_CHANGED));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
