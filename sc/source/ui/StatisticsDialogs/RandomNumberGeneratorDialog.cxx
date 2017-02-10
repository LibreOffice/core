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
#include <rtl/math.hxx>

#include "rangelst.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "strload.hxx"
#include "docfunc.hxx"

#include <random>

#include "RandomNumberGeneratorDialog.hxx"

namespace
{

const sal_Int64 DIST_UNIFORM             = 0;
const sal_Int64 DIST_NORMAL              = 1;
const sal_Int64 DIST_CAUCHY              = 2;
const sal_Int64 DIST_BERNOULLI           = 3;
const sal_Int64 DIST_BINOMIAL            = 4;
const sal_Int64 DIST_CHI_SQUARED         = 5;
const sal_Int64 DIST_GEOMETRIC           = 6;
const sal_Int64 DIST_NEGATIVE_BINOMIAL   = 7;
const sal_Int64 DIST_UNIFORM_INTEGER     = 8;

const sal_Int64 PERCISION   = 10000;
const sal_Int64 DIGITS      = 4;

}

ScRandomNumberGeneratorDialog::ScRandomNumberGeneratorDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScAnyRefDlg     ( pSfxBindings, pChildWindow, pParent,
                      "RandomNumberGeneratorDialog", "modules/scalc/ui/randomnumbergenerator.ui" ),
    mpViewData       ( pViewData ),
    mpDoc( pViewData->GetDocument() ),
    mbDialogLostFocus( false )
{
    get(mpInputRangeText,   "cell-range-label");
    get(mpInputRangeEdit,   "cell-range-edit");
    get(mpInputRangeButton, "cell-range-button");
    mpInputRangeEdit->SetReferences(this, mpInputRangeText);
    mpInputRangeButton->SetReferences(this, mpInputRangeEdit);

    get(mpParameter1Value, "parameter1-spin");
    get(mpParameter1Text,  "parameter1-label");
    get(mpParameter2Value, "parameter2-spin");
    get(mpParameter2Text,  "parameter2-label");

    get(mpEnableSeed, "enable-seed-check");
    get(mpSeed,       "seed-spin");

    get(mpEnableRounding, "enable-rounding-check");
    get(mpDecimalPlaces,  "decimal-places-spin");

    get(mpDistributionCombo, "distribution-combo");

    get(mpButtonOk,     "ok");
    get(mpButtonApply,  "apply");
    get(mpButtonClose,  "close");

    Init();
    GetRangeFromSelection();
}

ScRandomNumberGeneratorDialog::~ScRandomNumberGeneratorDialog()
{
    disposeOnce();
}

void ScRandomNumberGeneratorDialog::dispose()
{
    mpInputRangeText.clear();
    mpInputRangeEdit.clear();
    mpInputRangeButton.clear();
    mpDistributionCombo.clear();
    mpParameter1Text.clear();
    mpParameter1Value.clear();
    mpParameter2Text.clear();
    mpParameter2Value.clear();
    mpSeed.clear();
    mpEnableSeed.clear();
    mpDecimalPlaces.clear();
    mpEnableRounding.clear();
    mpButtonApply.clear();
    mpButtonOk.clear();
    mpButtonClose.clear();
    ScAnyRefDlg::dispose();
}

void ScRandomNumberGeneratorDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScRandomNumberGeneratorDialog, OkClicked ) );
    mpButtonClose->SetClickHdl( LINK( this, ScRandomNumberGeneratorDialog, CloseClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScRandomNumberGeneratorDialog, ApplyClicked ) );

    Link<Control&,void> aLink = LINK( this, ScRandomNumberGeneratorDialog, GetFocusHandler );
    mpInputRangeEdit->SetGetFocusHdl( aLink );
    mpInputRangeButton->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScRandomNumberGeneratorDialog, LoseFocusHandler );
    mpInputRangeEdit->SetLoseFocusHdl ( aLink );
    mpInputRangeButton->SetLoseFocusHdl ( aLink );

    mpInputRangeEdit->SetModifyHdl( LINK( this, ScRandomNumberGeneratorDialog, InputRangeModified ));
    mpParameter1Value->SetModifyHdl( LINK( this, ScRandomNumberGeneratorDialog, Parameter1ValueModified ));
    mpParameter2Value->SetModifyHdl( LINK( this, ScRandomNumberGeneratorDialog, Parameter2ValueModified ));

    mpDistributionCombo->SetSelectHdl( LINK( this, ScRandomNumberGeneratorDialog, DistributionChanged ));

    mpEnableSeed->SetToggleHdl( LINK( this, ScRandomNumberGeneratorDialog, CheckChanged ));
    mpEnableRounding->SetToggleHdl( LINK( this, ScRandomNumberGeneratorDialog, CheckChanged ));

    DistributionChanged(*mpDistributionCombo.get());
    CheckChanged(*mpEnableSeed);
}

void ScRandomNumberGeneratorDialog::GetRangeFromSelection()
{
    mpViewData->GetSimpleArea(maInputRange);
    OUString aCurrentString(maInputRange.Format(ScRefFlags::RANGE_ABS_3D, mpDoc, mpDoc->GetAddressConvention()));
    mpInputRangeEdit->SetText( aCurrentString );
}

void ScRandomNumberGeneratorDialog::SetActive()
{
    if ( mbDialogLostFocus )
    {
        mbDialogLostFocus = false;
        if( mpInputRangeEdit )
            mpInputRangeEdit->GrabFocus();
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

bool ScRandomNumberGeneratorDialog::Close()
{
    return DoClose( ScRandomNumberGeneratorDialogWrapper::GetChildWindowId() );
}

void ScRandomNumberGeneratorDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDoc )
{
    if ( mpInputRangeEdit->IsEnabled() )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpInputRangeEdit );

        maInputRange = rReferenceRange;

        OUString aReferenceString(maInputRange.Format(ScRefFlags::RANGE_ABS_3D, pDoc, pDoc->GetAddressConvention()));
        mpInputRangeEdit->SetRefString( aReferenceString );

        mpButtonApply->Enable();
        mpButtonOk->Enable();
    }
}

void ScRandomNumberGeneratorDialog::SelectGeneratorAndGenerateNumbers()
{
    if (!maInputRange.IsValid())
        return;

    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = reinterpret_cast<sal_Int64>(mpDistributionCombo->GetEntryData(aSelectedIndex));

    sal_uInt32 seedValue;

    if( mpEnableSeed->IsChecked() )
    {
        seedValue = mpSeed->GetValue();
    }
    else
    {
        TimeValue now;
        osl_getSystemTime(&now);
        seedValue = now.Nanosec;
    }

    std::mt19937 seed(seedValue);

    sal_Int64 parameterInteger1 = mpParameter1Value->GetValue();
    sal_Int64 parameterInteger2 = mpParameter2Value->GetValue();

    double parameter1 = parameterInteger1 / static_cast<double>(PERCISION);
    double parameter2 = parameterInteger2 / static_cast<double>(PERCISION);

    boost::optional<sal_Int8> aDecimalPlaces;
    if (mpEnableRounding->IsChecked())
    {
        aDecimalPlaces = static_cast<sal_Int8>(mpDecimalPlaces->GetValue());
    }

    switch(aSelectedId)
    {
        case DIST_UNIFORM:
        {
            std::uniform_real_distribution<> distribution(parameter1, parameter2);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_UNIFORM_REAL, aDecimalPlaces);
            break;
        }
        case DIST_UNIFORM_INTEGER:
        {
            std::uniform_int_distribution<> distribution(parameterInteger1, parameterInteger2);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_UNIFORM_INTEGER, aDecimalPlaces);
            break;
        }
        case DIST_NORMAL:
        {
            std::normal_distribution<> distribution(parameter1, parameter2);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_NORMAL, aDecimalPlaces);
            break;
        }
        case DIST_CAUCHY:
        {
            std::cauchy_distribution<> distribution(parameter1);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_CAUCHY, aDecimalPlaces);
            break;
        }
        case DIST_BERNOULLI:
        {
            std::bernoulli_distribution distribution(parameter1);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_BERNOULLI, aDecimalPlaces);
            break;
        }
        case DIST_BINOMIAL:
        {
            std::binomial_distribution<> distribution(parameterInteger2, parameter1);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_BINOMIAL, aDecimalPlaces);
            break;
        }
        case DIST_NEGATIVE_BINOMIAL:
        {
            std::negative_binomial_distribution<> distribution(parameterInteger2, parameter1);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_NEGATIVE_BINOMIAL, aDecimalPlaces);
            break;
        }
        case DIST_CHI_SQUARED:
        {
            std::chi_squared_distribution<> distribution(parameter1);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_CHI_SQUARED, aDecimalPlaces);
            break;
        }
        case DIST_GEOMETRIC:
        {
            std::geometric_distribution<> distribution(parameter1);
            auto rng = std::bind(distribution, seed);
            GenerateNumbers(rng, STR_DISTRIBUTION_GEOMETRIC, aDecimalPlaces);
            break;
        }
    }
}

template<class RNG>
void ScRandomNumberGeneratorDialog::GenerateNumbers(RNG& randomGenerator, const sal_Int16 aDistributionStringId, boost::optional<sal_Int8> aDecimalPlaces)
{
    OUString aUndo = SC_RESSTR(STR_UNDO_DISTRIBUTION_TEMPLATE);
    OUString aDistributionName = SC_RESSTR(aDistributionStringId);
    aUndo = aUndo.replaceAll("$(DISTRIBUTION)",  aDistributionName);

    ScDocShell* pDocShell = mpViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo, 0, mpViewData->GetViewShell()->GetViewShellId() );

    SCROW nRowStart = maInputRange.aStart.Row();
    SCROW nRowEnd   = maInputRange.aEnd.Row();
    SCCOL nColStart = maInputRange.aStart.Col();
    SCCOL nColEnd   = maInputRange.aEnd.Col();
    SCTAB nTabStart = maInputRange.aStart.Tab();
    SCTAB nTabEnd   = maInputRange.aEnd.Tab();

    std::vector<double> aVals;
    aVals.reserve(nRowEnd - nRowStart + 1);

    for (SCROW nTab = nTabStart; nTab <= nTabEnd; ++nTab)
    {
        for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        {
            aVals.clear();

            ScAddress aPos(nCol, nRowStart, nTab);
            for (SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
            {

                if (aDecimalPlaces)
                    aVals.push_back(rtl::math::round(randomGenerator(), *aDecimalPlaces));
                else
                    aVals.push_back(randomGenerator());
            }

            pDocShell->GetDocFunc().SetValueCells(aPos, aVals, true);
        }
    }

    pUndoManager->LeaveListAction();

    pDocShell->PostPaint( maInputRange, PaintPartFlags::Grid );
}

IMPL_LINK_NOARG( ScRandomNumberGeneratorDialog, OkClicked, Button*, void )
{
    ApplyClicked(nullptr);
    CloseClicked(nullptr);
}

IMPL_LINK_NOARG( ScRandomNumberGeneratorDialog, ApplyClicked, Button*, void )
{
    SelectGeneratorAndGenerateNumbers();
}

IMPL_LINK_NOARG( ScRandomNumberGeneratorDialog, CloseClicked, Button*, void )
{
    Close();
}

IMPL_LINK( ScRandomNumberGeneratorDialog, GetFocusHandler, Control&, rCtrl, void )
{
    Edit* pEdit = nullptr;

    if( (&rCtrl == static_cast<Control*>(mpInputRangeEdit)) || (&rCtrl == static_cast<Control*>(mpInputRangeButton)) )
        pEdit = mpInputRangeEdit;

    if( pEdit )
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, LoseFocusHandler, Control&, void)
{
    mbDialogLostFocus = !IsActive();
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, InputRangeModified, Edit&, void)
{
    ScRangeList aRangeList;
    bool bValid = ParseWithNames( aRangeList, mpInputRangeEdit->GetText(), mpDoc);
    const ScRange* pRange = (bValid && aRangeList.size() == 1) ? aRangeList[0] : nullptr;
    if (pRange)
    {
        maInputRange = *pRange;
        mpButtonApply->Enable();
        mpButtonOk->Enable();
        // Highlight the resulting range.
        mpInputRangeEdit->StartUpdateData();
    }
    else
    {
        maInputRange = ScRange( ScAddress::INITIALIZE_INVALID);
        mpButtonApply->Disable();
        mpButtonOk->Disable();
    }
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, Parameter1ValueModified, Edit&, void)
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = reinterpret_cast<sal_Int64>( mpDistributionCombo->GetEntryData(aSelectedIndex) );
    if (aSelectedId == DIST_UNIFORM ||
        aSelectedId == DIST_UNIFORM_INTEGER)
    {
        sal_Int64 min = mpParameter1Value->GetValue();
        sal_Int64 max = mpParameter2Value->GetValue();
        if(min > max)
        {
            mpParameter2Value->SetValue(min);
        }
    }
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, Parameter2ValueModified, Edit&, void)
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = reinterpret_cast<sal_Int64>( mpDistributionCombo->GetEntryData(aSelectedIndex) );
    if (aSelectedId == DIST_UNIFORM ||
        aSelectedId == DIST_UNIFORM_INTEGER)
    {
        sal_Int64 min = mpParameter1Value->GetValue();
        sal_Int64 max = mpParameter2Value->GetValue();
        if(min > max)
        {
            mpParameter1Value->SetValue(max);
        }
    }
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, CheckChanged, CheckBox&, void)
{
    mpSeed->Enable(mpEnableSeed->IsChecked());
    mpDecimalPlaces->Enable(mpEnableRounding->IsChecked());
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, DistributionChanged, ListBox&, void)
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = reinterpret_cast<sal_Int64>( mpDistributionCombo->GetEntryData(aSelectedIndex) );

    mpParameter1Value->SetMin(SAL_MIN_INT64);
    mpParameter1Value->SetMax(SAL_MAX_INT64);
    mpParameter2Value->SetMin(SAL_MIN_INT64);
    mpParameter2Value->SetMax(SAL_MAX_INT64);

    mpParameter1Value->SetDecimalDigits(DIGITS);
    mpParameter1Value->SetSpinSize(PERCISION);

    mpParameter2Value->SetDecimalDigits(DIGITS);
    mpParameter2Value->SetSpinSize(PERCISION);

    switch(aSelectedId)
    {
        case DIST_UNIFORM:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_MINIMUM));
            mpParameter2Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_MAXIMUM));
            mpParameter2Text->Show();
            mpParameter2Value->Show();
            break;
        }
        case DIST_UNIFORM_INTEGER:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_MINIMUM));
            mpParameter1Value->SetDecimalDigits(0);
            mpParameter1Value->SetSpinSize(1);

            mpParameter2Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_MAXIMUM));
            mpParameter2Value->SetDecimalDigits(0);
            mpParameter2Value->SetSpinSize(1);

            mpParameter2Text->Show();
            mpParameter2Value->Show();
            break;
        }
        case DIST_NORMAL:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_MEAN));
            mpParameter2Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_DEVIATION));
            mpParameter2Text->Show();
            mpParameter2Value->Show();
            break;
        }
        case DIST_CAUCHY:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_MEDIAN));
            mpParameter2Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_SIGMA));
            mpParameter2Text->Show();
            mpParameter2Value->Show();
            break;
        }
        case DIST_BERNOULLI:
        case DIST_GEOMETRIC:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_PROBABILITY));
            mpParameter1Value->SetMin(         0 );
            mpParameter1Value->SetMax( PERCISION );
            mpParameter1Value->SetSpinSize(1000);

            mpParameter2Text->Hide();
            mpParameter2Value->Hide();
            break;
        }
        case DIST_BINOMIAL:
        case DIST_NEGATIVE_BINOMIAL:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_PROBABILITY));
            mpParameter1Value->SetMin(         0 );
            mpParameter1Value->SetMax( PERCISION );
            mpParameter1Value->SetSpinSize(1000);

            mpParameter2Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_NUMBER_OF_TRIALS));
            mpParameter2Value->SetDecimalDigits(0);
            mpParameter2Value->SetSpinSize(1);
            mpParameter2Value->SetMin(0);

            mpParameter2Text->Show();
            mpParameter2Value->Show();
            break;
        }
        case DIST_CHI_SQUARED:
        {
            mpParameter1Text->SetText(SC_RESSTR(STR_RNG_PARAMETER_STANDARD_NU_VALUE));

            mpParameter2Text->Hide();
            mpParameter2Value->Hide();
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
