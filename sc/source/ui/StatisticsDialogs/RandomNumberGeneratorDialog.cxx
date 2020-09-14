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
#include <rtl/math.hxx>
#include <osl/time.h>

#include <rangelst.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <reffact.hxx>
#include <docfunc.hxx>
#include <scresid.hxx>
#include <strings.hrc>

#include <random>

#include <RandomNumberGeneratorDialog.hxx>

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

const sal_Int64 PRECISION   = 10000;
const sal_Int64 DIGITS      = 4;

}

ScRandomNumberGeneratorDialog::ScRandomNumberGeneratorDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData* pViewData)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent,
                          "modules/scalc/ui/randomnumbergenerator.ui",
                          "RandomNumberGeneratorDialog")
    , mpViewData(pViewData)
    , mrDoc(pViewData->GetDocument())
    , mbDialogLostFocus(false)
    , mxInputRangeText(m_xBuilder->weld_label("cell-range-label"))
    , mxInputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("cell-range-edit")))
    , mxInputRangeButton(new formula::RefButton(m_xBuilder->weld_button("cell-range-button")))
    , mxDistributionCombo(m_xBuilder->weld_combo_box("distribution-combo"))
    , mxParameter1Text(m_xBuilder->weld_label("parameter1-label"))
    , mxParameter1Value(m_xBuilder->weld_spin_button("parameter1-spin"))
    , mxParameter2Text(m_xBuilder->weld_label("parameter2-label"))
    , mxParameter2Value(m_xBuilder->weld_spin_button("parameter2-spin"))
    , mxSeed(m_xBuilder->weld_spin_button("seed-spin"))
    , mxEnableSeed(m_xBuilder->weld_check_button("enable-seed-check"))
    , mxDecimalPlaces(m_xBuilder->weld_spin_button("decimal-places-spin"))
    , mxEnableRounding(m_xBuilder->weld_check_button("enable-rounding-check"))
    , mxButtonApply(m_xBuilder->weld_button("apply"))
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxButtonClose(m_xBuilder->weld_button("close"))
{
    mxInputRangeEdit->SetReferences(this, mxInputRangeText.get());
    mxInputRangeButton->SetReferences(this, mxInputRangeEdit.get());

    Init();
    GetRangeFromSelection();
}

ScRandomNumberGeneratorDialog::~ScRandomNumberGeneratorDialog()
{
}

void ScRandomNumberGeneratorDialog::Init()
{
    mxButtonOk->connect_clicked( LINK( this, ScRandomNumberGeneratorDialog, OkClicked ) );
    mxButtonClose->connect_clicked( LINK( this, ScRandomNumberGeneratorDialog, CloseClicked ) );
    mxButtonApply->connect_clicked( LINK( this, ScRandomNumberGeneratorDialog, ApplyClicked ) );

    mxInputRangeEdit->SetGetFocusHdl(LINK( this, ScRandomNumberGeneratorDialog, GetEditFocusHandler ));
    mxInputRangeButton->SetGetFocusHdl(LINK( this, ScRandomNumberGeneratorDialog, GetButtonFocusHandler ));

    mxInputRangeEdit->SetLoseFocusHdl (LINK( this, ScRandomNumberGeneratorDialog, LoseEditFocusHandler ));
    mxInputRangeButton->SetLoseFocusHdl (LINK( this, ScRandomNumberGeneratorDialog, LoseButtonFocusHandler ));

    mxInputRangeEdit->SetModifyHdl( LINK( this, ScRandomNumberGeneratorDialog, InputRangeModified ));
    mxParameter1Value->connect_value_changed( LINK( this, ScRandomNumberGeneratorDialog, Parameter1ValueModified ));
    mxParameter2Value->connect_value_changed( LINK( this, ScRandomNumberGeneratorDialog, Parameter2ValueModified ));

    mxDistributionCombo->connect_changed( LINK( this, ScRandomNumberGeneratorDialog, DistributionChanged ));

    mxEnableSeed->connect_toggled( LINK( this, ScRandomNumberGeneratorDialog, CheckChanged ));
    mxEnableRounding->connect_toggled( LINK( this, ScRandomNumberGeneratorDialog, CheckChanged ));

    DistributionChanged(*mxDistributionCombo);
    CheckChanged(*mxEnableSeed);
}

void ScRandomNumberGeneratorDialog::GetRangeFromSelection()
{
    mpViewData->GetSimpleArea(maInputRange);
    OUString aCurrentString(maInputRange.Format(mrDoc, ScRefFlags::RANGE_ABS_3D, mrDoc.GetAddressConvention()));
    mxInputRangeEdit->SetText( aCurrentString );
}

void ScRandomNumberGeneratorDialog::SetActive()
{
    if ( mbDialogLostFocus )
    {
        mbDialogLostFocus = false;
        if( mxInputRangeEdit )
            mxInputRangeEdit->GrabFocus();
    }
    else
    {
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void ScRandomNumberGeneratorDialog::Close()
{
    DoClose( ScRandomNumberGeneratorDialogWrapper::GetChildWindowId() );
}

void ScRandomNumberGeneratorDialog::SetReference( const ScRange& rReferenceRange, ScDocument& rDoc )
{
    if (!mxInputRangeEdit->GetWidget()->get_sensitive())
        return;

    if ( rReferenceRange.aStart != rReferenceRange.aEnd )
        RefInputStart(mxInputRangeEdit.get());

    maInputRange = rReferenceRange;

    OUString aReferenceString(maInputRange.Format(rDoc, ScRefFlags::RANGE_ABS_3D, rDoc.GetAddressConvention()));
    mxInputRangeEdit->SetRefString( aReferenceString );

    mxButtonApply->set_sensitive(true);
    mxButtonOk->set_sensitive(true);
}

void ScRandomNumberGeneratorDialog::SelectGeneratorAndGenerateNumbers()
{
    if (!maInputRange.IsValid())
        return;

    sal_Int64 aSelectedId = mxDistributionCombo->get_active_id().toInt64();

    sal_uInt32 seedValue;

    if( mxEnableSeed->get_active() )
    {
        seedValue = mxSeed->get_value();
    }
    else
    {
        TimeValue now;
        osl_getSystemTime(&now);
        seedValue = now.Nanosec;
    }

    std::mt19937 seed(seedValue);

    sal_Int64 parameterInteger1 = mxParameter1Value->get_value();
    sal_Int64 parameterInteger2 = mxParameter2Value->get_value();

    double parameter1 = parameterInteger1 / static_cast<double>(PRECISION);
    double parameter2 = parameterInteger2 / static_cast<double>(PRECISION);

    std::optional<sal_Int8> aDecimalPlaces;
    if (mxEnableRounding->get_active())
    {
        aDecimalPlaces = static_cast<sal_Int8>(mxDecimalPlaces->get_value());
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
            std::uniform_int_distribution<sal_Int64> distribution(parameterInteger1, parameterInteger2);
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
void ScRandomNumberGeneratorDialog::GenerateNumbers(RNG& randomGenerator, const char* pDistributionStringId, std::optional<sal_Int8> aDecimalPlaces)
{
    OUString aUndo = ScResId(STR_UNDO_DISTRIBUTION_TEMPLATE);
    OUString aDistributionName = ScResId(pDistributionStringId);
    aUndo = aUndo.replaceAll("$(DISTRIBUTION)",  aDistributionName);

    ScDocShell* pDocShell = mpViewData->GetDocShell();
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
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

IMPL_LINK_NOARG( ScRandomNumberGeneratorDialog, OkClicked, weld::Button&, void )
{
    ApplyClicked(*mxButtonApply);
    CloseClicked(*mxButtonClose);
}

IMPL_LINK_NOARG( ScRandomNumberGeneratorDialog, ApplyClicked, weld::Button&, void )
{
    SelectGeneratorAndGenerateNumbers();
}

IMPL_LINK_NOARG( ScRandomNumberGeneratorDialog, CloseClicked, weld::Button&, void )
{
    response(RET_CLOSE);
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, GetEditFocusHandler, formula::RefEdit&, void)
{
    mxInputRangeEdit->SelectAll();
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, GetButtonFocusHandler, formula::RefButton&, void)
{
    mxInputRangeEdit->SelectAll();
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, LoseButtonFocusHandler, formula::RefButton&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, InputRangeModified, formula::RefEdit&, void)
{
    ScRangeList aRangeList;
    bool bValid = ParseWithNames( aRangeList, mxInputRangeEdit->GetText(), mrDoc);
    const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
    if (pRange)
    {
        maInputRange = *pRange;
        mxButtonApply->set_sensitive(true);
        mxButtonOk->set_sensitive(true);
        // Highlight the resulting range.
        mxInputRangeEdit->StartUpdateData();
    }
    else
    {
        maInputRange = ScRange( ScAddress::INITIALIZE_INVALID);
        mxButtonApply->set_sensitive(false);
        mxButtonOk->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, Parameter1ValueModified, weld::SpinButton&, void)
{
    sal_Int64 aSelectedId = mxDistributionCombo->get_active_id().toInt64();
    if (aSelectedId == DIST_UNIFORM ||
        aSelectedId == DIST_UNIFORM_INTEGER)
    {
        sal_Int64 min = mxParameter1Value->get_value();
        sal_Int64 max = mxParameter2Value->get_value();
        if(min > max)
        {
            mxParameter2Value->set_value(min);
        }
    }
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, Parameter2ValueModified, weld::SpinButton&, void)
{
    sal_Int64 aSelectedId = mxDistributionCombo->get_active_id().toInt64();
    if (aSelectedId == DIST_UNIFORM ||
        aSelectedId == DIST_UNIFORM_INTEGER)
    {
        sal_Int64 min = mxParameter1Value->get_value();
        sal_Int64 max = mxParameter2Value->get_value();
        if(min > max)
        {
            mxParameter1Value->set_value(max);
        }
    }
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, CheckChanged, weld::ToggleButton&, void)
{
    mxSeed->set_sensitive(mxEnableSeed->get_active());
    mxDecimalPlaces->set_sensitive(mxEnableRounding->get_active());
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, DistributionChanged, weld::ComboBox&, void)
{
    sal_Int64 aSelectedId = mxDistributionCombo->get_active_id().toInt64();

    mxParameter1Value->set_range(SAL_MIN_INT32, SAL_MAX_INT32);
    mxParameter2Value->set_range(SAL_MIN_INT32, SAL_MAX_INT32);

    mxParameter1Value->set_digits(DIGITS);
    mxParameter1Value->set_increments(PRECISION, PRECISION * 10);

    mxParameter2Value->set_digits(DIGITS);
    mxParameter2Value->set_increments(PRECISION, PRECISION * 10);

    switch(aSelectedId)
    {
        case DIST_UNIFORM:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_MINIMUM));
            mxParameter2Text->set_label(ScResId(STR_RNG_PARAMETER_MAXIMUM));
            mxParameter2Text->show();
            mxParameter2Value->show();
            break;
        }
        case DIST_UNIFORM_INTEGER:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_MINIMUM));
            mxParameter1Value->set_digits(0);
            mxParameter1Value->set_increments(1, 10);

            mxParameter2Text->set_label(ScResId(STR_RNG_PARAMETER_MAXIMUM));
            mxParameter2Value->set_digits(0);
            mxParameter2Value->set_increments(1, 10);

            mxParameter2Text->show();
            mxParameter2Value->show();
            break;
        }
        case DIST_NORMAL:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_MEAN));
            mxParameter2Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_DEVIATION));
            mxParameter2Text->show();
            mxParameter2Value->show();
            break;
        }
        case DIST_CAUCHY:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_MEDIAN));
            mxParameter2Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_SIGMA));
            mxParameter2Text->show();
            mxParameter2Value->show();
            break;
        }
        case DIST_BERNOULLI:
        case DIST_GEOMETRIC:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_PROBABILITY));
            mxParameter1Value->set_range(0, PRECISION);
            mxParameter1Value->set_increments(1000, 10000);

            mxParameter2Text->hide();
            mxParameter2Value->hide();
            break;
        }
        case DIST_BINOMIAL:
        case DIST_NEGATIVE_BINOMIAL:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_PROBABILITY));
            mxParameter1Value->set_range(0, PRECISION);
            mxParameter1Value->set_increments(1000, 10000);

            mxParameter2Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_NUMBER_OF_TRIALS));
            mxParameter2Value->set_digits(0);
            mxParameter2Value->set_increments(1, 10);
            mxParameter2Value->set_min(0);

            mxParameter2Text->show();
            mxParameter2Value->show();
            break;
        }
        case DIST_CHI_SQUARED:
        {
            mxParameter1Text->set_label(ScResId(STR_RNG_PARAMETER_STANDARD_NU_VALUE));

            mxParameter2Text->hide();
            mxParameter2Value->hide();
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
