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
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/chi_squared_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/negative_binomial_distribution.hpp>

#include "RandomNumberGeneratorDialog.hxx"

#define ABS_DREF3D SCA_VALID | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE | SCA_TAB_3D

namespace
{

static const sal_Int64 DIST_UNIFORM             = 0;
static const sal_Int64 DIST_NORMAL              = 1;
static const sal_Int64 DIST_CAUCHY              = 2;
static const sal_Int64 DIST_BERNOULLI           = 3;
static const sal_Int64 DIST_BINOMIAL            = 4;
static const sal_Int64 DIST_CHI_SQUARED         = 5;
static const sal_Int64 DIST_GEOMETRIC           = 6;
static const sal_Int64 DIST_NEGATIVE_BINOMIAL   = 7;
static const sal_Int64 DIST_UNIFORM_INTEGER     = 8;

static const sal_Int64 PERCISION   = 10000;
static const sal_Int64 DIGITS      = 4;
}

ScRandomNumberGeneratorDialog::ScRandomNumberGeneratorDialog(
        SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, ScViewData* pViewData ) :
    ScAnyRefDlg     ( pB, pCW, pParent, "RandomNumberGeneratorDialog", "modules/scalc/ui/randomnumbergenerator.ui" ),
    mViewData       ( pViewData ),
    mDocument       ( pViewData->GetDocument() ),
    mAddressDetails ( mDocument->GetAddressConvention(), 0, 0 ),
    mDialogLostFocus( false )
{
    get(mpFtVariableCell, "cell-range-label");
    get(mpEdVariableCell, "cell-range-edit");
    mpEdVariableCell->SetReferences(this, mpFtVariableCell);
    get(mpRBVariableCell, "cell-range-button");
    mpRBVariableCell->SetReferences(this, mpEdVariableCell);

    get(mpButtonOk,     "ok");
    get(mpButtonApply,  "apply");
    get(mpButtonCancel, "cancel");

    get(mpParameter1Value, "parameter1-spin");
    get(mpParameter1Text,  "parameter1-label");
    get(mpParameter2Value, "parameter2-spin");
    get(mpParameter2Text,  "parameter2-label");

    get(mpEnableSeed, "enable-seed-check");
    get(mpSeed, "seed-spin");

    get(mpDistributionCombo, "distribution-combo");

    Init();
    GetRangeFromSelection();
}

void ScRandomNumberGeneratorDialog::Init()
{
    mpButtonOk->SetClickHdl( LINK( this, ScRandomNumberGeneratorDialog, OkClicked ) );
    mpButtonCancel->SetClickHdl( LINK( this, ScRandomNumberGeneratorDialog, CancelClicked ) );
    mpButtonApply->SetClickHdl( LINK( this, ScRandomNumberGeneratorDialog, ApplyClicked ) );

    Link aLink = LINK( this, ScRandomNumberGeneratorDialog, GetFocusHandler );
    mpEdVariableCell->SetGetFocusHdl( aLink );
    mpRBVariableCell->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScRandomNumberGeneratorDialog, LoseFocusHandler );
    mpEdVariableCell->SetLoseFocusHdl ( aLink );
    mpRBVariableCell->SetLoseFocusHdl ( aLink );

    mpParameter1Value->SetModifyHdl( LINK( this, ScRandomNumberGeneratorDialog, Parameter1ValueModified ));
    mpParameter2Value->SetModifyHdl( LINK( this, ScRandomNumberGeneratorDialog, Parameter2ValueModified ));

    mpDistributionCombo->SetSelectHdl( LINK( this, ScRandomNumberGeneratorDialog, DistributionChanged ));

    mpEnableSeed->SetToggleHdl( LINK( this, ScRandomNumberGeneratorDialog, SeedCheckChanged ));

    mpParameter1Value->SetMin( SAL_MIN_INT64 );
    mpParameter1Value->SetMax( SAL_MAX_INT64 );
    mpParameter2Value->SetMin( SAL_MIN_INT64 );
    mpParameter2Value->SetMax( SAL_MAX_INT64 );

    DistributionChanged(NULL);
    SeedCheckChanged(NULL);
}

void ScRandomNumberGeneratorDialog::GetRangeFromSelection()
{
    String  aCurrentString;

    SCCOL   nStartCol   = 0;
    SCROW   nStartRow   = 0;
    SCTAB   nStartTab   = 0;
    SCCOL   nEndCol     = 0;
    SCROW   nEndRow     = 0;
    SCTAB   nEndTab     = 0;

    mViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                              nEndCol,   nEndRow,  nEndTab );

    mCurrentRange = ScRange( ScAddress( nStartCol, nStartRow, nStartTab ),
                            ScAddress( nEndCol,   nEndRow,   nEndTab ) );

    mCurrentRange.Format( aCurrentString, ABS_DREF3D, mDocument, mAddressDetails );

    mpEdVariableCell->SetText( aCurrentString );
}


ScRandomNumberGeneratorDialog::~ScRandomNumberGeneratorDialog()
{
}

void ScRandomNumberGeneratorDialog::SetActive()
{
    if ( mDialogLostFocus )
    {
        mDialogLostFocus = false;
        if( mpEdVariableCell )
            mpEdVariableCell->GrabFocus();
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

sal_Bool ScRandomNumberGeneratorDialog::Close()
{
    return DoClose( ScRandomNumberGeneratorDialogWrapper::GetChildWindowId() );
}

void ScRandomNumberGeneratorDialog::SetReference( const ScRange& rReferenceRange, ScDocument* pDocument )
{
    if ( mpEdVariableCell->IsEnabled() )
    {
        if ( rReferenceRange.aStart != rReferenceRange.aEnd )
            RefInputStart( mpEdVariableCell );

        mCurrentRange = rReferenceRange;

        String aReferenceString;
        mCurrentRange.Format( aReferenceString, ABS_DREF3D, pDocument, mAddressDetails );
        mpEdVariableCell->SetRefString( aReferenceString );
    }
}

void ScRandomNumberGeneratorDialog::SelectGeneratorAndGenerateNumbers()
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = (sal_Int64) mpDistributionCombo->GetEntryData(aSelectedIndex);

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

    boost::mt19937 seed(seedValue);

    sal_Int64 parameterInteger1 = mpParameter1Value->GetValue();
    sal_Int64 parameterInteger2 = mpParameter2Value->GetValue();

    double parameter1 = parameterInteger1 / static_cast<double>(PERCISION);
    double parameter2 = parameterInteger2 / static_cast<double>(PERCISION);

    switch(aSelectedId)
    {
        case DIST_UNIFORM:
        {
            boost::random::uniform_real_distribution<> distribution(parameter1, parameter2);
            boost::variate_generator<boost::mt19937&, boost::random::uniform_real_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_UNIFORM_REAL));
            break;
        }
        case DIST_UNIFORM_INTEGER:
        {
            boost::random::uniform_int_distribution<> distribution(parameterInteger1, parameterInteger2);
            boost::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_UNIFORM_INTEGER));
            break;
        }
        case DIST_NORMAL:
        {
            boost::random::normal_distribution<> distribution(parameter1, parameter2);
            boost::variate_generator<boost::mt19937&, boost::random::normal_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_NORMAL));
            break;
        }
        case DIST_CAUCHY:
        {
            boost::random::cauchy_distribution<> distribution(parameter1);
            boost::variate_generator<boost::mt19937&, boost::random::cauchy_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_CAUCHY));
            break;
        }
        case DIST_BERNOULLI:
        {
            boost::random::bernoulli_distribution<> distribution(parameter1);
            boost::variate_generator<boost::mt19937&, boost::random::bernoulli_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_BERNOULLI));
            break;
        }
        case DIST_BINOMIAL:
        {
            boost::random::binomial_distribution<> distribution(parameterInteger2, parameter1);
            boost::variate_generator<boost::mt19937&, boost::random::binomial_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_BINOMIAL));
            break;
        }
        case DIST_NEGATIVE_BINOMIAL:
        {
            boost::random::negative_binomial_distribution<> distribution(parameterInteger2, parameter1);
            boost::variate_generator<boost::mt19937&, boost::random::negative_binomial_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_NEGATIVE_BINOMIAL));
            break;
        }
        case DIST_CHI_SQUARED:
        {
            boost::random::chi_squared_distribution<> distribution(parameter1);
            boost::variate_generator<boost::mt19937&, boost::random::chi_squared_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_CHI_SQUARED));
            break;
        }
        case DIST_GEOMETRIC:
        {
            boost::random::geometric_distribution<> distribution(parameter1);
            boost::variate_generator<boost::mt19937&, boost::random::geometric_distribution<> > rng(seed, distribution);
            GenerateNumbers(rng, SC_RESSTR(STR_DISTRIBUTION_GEOMETRIC));
            break;
        }
    }
}

template<class RNG>
void ScRandomNumberGeneratorDialog::GenerateNumbers(RNG randomGenerator, OUString aDistributionName)
{
    OUString aUndo = SC_RESSTR(STR_UNDO_DISTRIBUTION_TEMPLATE);
    aUndo = aUndo.replaceAll("$(DISTRIBUTION)",  aDistributionName);

    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    SCROW nRowStart = mCurrentRange.aStart.Row();
    SCROW nRowEnd   = mCurrentRange.aEnd.Row();
    SCCOL nColStart = mCurrentRange.aStart.Col();
    SCCOL nColEnd   = mCurrentRange.aEnd.Col();
    SCTAB nTabStart = mCurrentRange.aStart.Tab();
    SCTAB nTabEnd   = mCurrentRange.aEnd.Tab();

    for (SCROW nTab = nTabStart; nTab <= nTabEnd; nTab++)
    {
        for (SCROW nRow = nRowStart; nRow <= nRowEnd; nRow++)
        {
            for (SCCOL nCol = nColStart; nCol <= nColEnd; nCol++)
            {
                pDocShell->GetDocFunc().SetValueCell(ScAddress(nCol, nRow, nTab), randomGenerator(), true);
            }
        }
    }

    pUndoManager->LeaveListAction();

    pDocShell->PostPaint( mCurrentRange, PAINT_GRID );
}

IMPL_LINK( ScRandomNumberGeneratorDialog, OkClicked, PushButton*, /*pButton*/ )
{
    SelectGeneratorAndGenerateNumbers();
    Close();
    return 0;
}


IMPL_LINK( ScRandomNumberGeneratorDialog, ApplyClicked, PushButton*, /*pButton*/ )
{
    SelectGeneratorAndGenerateNumbers();
    return 0;
}

IMPL_LINK( ScRandomNumberGeneratorDialog, CancelClicked, PushButton*, /*pButton*/ )
{
    Close();
    return 0;
}

IMPL_LINK( ScRandomNumberGeneratorDialog, GetFocusHandler, Control*, pCtrl )
{
    Edit* pEdit = NULL;

    if( (pCtrl == (Control*)mpEdVariableCell) || (pCtrl == (Control*)mpRBVariableCell) )
        pEdit = mpEdVariableCell;

    if( pEdit )
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, LoseFocusHandler)
{
    mDialogLostFocus = !IsActive();
    return 0;
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, Parameter1ValueModified)
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = (sal_Int64) mpDistributionCombo->GetEntryData(aSelectedIndex);
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
    return 0;
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, Parameter2ValueModified)
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = (sal_Int64) mpDistributionCombo->GetEntryData(aSelectedIndex);
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
    return 0;
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, SeedCheckChanged)
{
    mpSeed->Enable(mpEnableSeed->IsChecked());
    return 0;
}

IMPL_LINK_NOARG(ScRandomNumberGeneratorDialog, DistributionChanged)
{
    sal_Int16 aSelectedIndex = mpDistributionCombo-> GetSelectEntryPos();
    sal_Int64 aSelectedId = (sal_Int64) mpDistributionCombo->GetEntryData(aSelectedIndex);

    mpParameter1Value->SetMin( SAL_MIN_INT64 );
    mpParameter1Value->SetMax( SAL_MAX_INT64 );
    mpParameter2Value->SetMin( SAL_MIN_INT64 );
    mpParameter2Value->SetMax( SAL_MAX_INT64 );

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
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
