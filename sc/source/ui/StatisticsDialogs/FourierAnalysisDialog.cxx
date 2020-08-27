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

#include <formulacell.hxx>
#include <rangelst.hxx>
#include <scitems.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <uiitems.hxx>
#include <reffact.hxx>
#include <docfunc.hxx>
#include <TableFillingAndNavigationTools.hxx>
#include <FourierAnalysisDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScFourierAnalysisDialog::ScFourierAnalysisDialog(SfxBindings* pSfxBindings,
                                                 SfxChildWindow* pChildWindow,
                                                 weld::Window* pParent, ScViewData* pViewData)
    : ScStatisticsInputOutputDialog(pSfxBindings, pChildWindow, pParent, pViewData,
                                    "modules/scalc/ui/fourieranalysisdialog.ui",
                                    "FourierAnalysisDialog")
    , maLabelAddr(ScAddress::INITIALIZE_INVALID)
    , maActualInputRange(ScAddress::INITIALIZE_INVALID)
    , mnLen(0)
    , mfMinMag(0.0)
    , mbUse3DAddresses(false)
    , mbGroupedByColumn(true)
    , mbWithLabels(false)
    , mbInverse(false)
    , mbPolar(false)
    , mxWithLabelsCheckBox(m_xBuilder->weld_check_button("withlabels-check"))
    , mxInverseCheckBox(m_xBuilder->weld_check_button("inverse-check"))
    , mxPolarCheckBox(m_xBuilder->weld_check_button("polar-check"))
    , mxMinMagnitudeField(m_xBuilder->weld_spin_button("minmagnitude-spin"))
    , mxErrorMessage(m_xBuilder->weld_label("error-message"))
{
    m_xDialog->set_title(ScResId(STR_FOURIER_ANALYSIS));

    mxWithLabelsCheckBox->connect_toggled(LINK(this, ScFourierAnalysisDialog, CheckBoxHdl));
}

ScFourierAnalysisDialog::~ScFourierAnalysisDialog() {}

void ScFourierAnalysisDialog::Close()
{
    DoClose(ScFourierAnalysisDialogWrapper::GetChildWindowId());
}

const char* ScFourierAnalysisDialog::GetUndoNameId() { return STR_FOURIER_ANALYSIS; }

ScRange ScFourierAnalysisDialog::ApplyOutput(ScDocShell* pDocShell)
{
    getOptions();
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, &mDocument,
                                formula::FormulaGrammar::mergeToGrammar(
                                    formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(&mDocument);
    aTemplate.autoReplaceUses3D(mbUse3DAddresses);

    aOutput.writeBoldString(mbInverse ? ScResId(STR_INVERSE_FOURIER_TRANSFORM)
                                      : ScResId(STR_FOURIER_TRANSFORM));
    aOutput.newLine();
    OUString aLabel;
    getDataLabel(aLabel);
    if (aLabel.startsWith("="))
        aOutput.writeFormula(aLabel);
    else
        aOutput.writeString(aLabel);

    aOutput.newLine();
    // Components header
    if (!mbPolar)
    {
        aOutput.writeString(ScResId(STR_REAL_PART));
        aOutput.nextColumn();
        aOutput.writeString(ScResId(STR_IMAGINARY_PART));
    }
    else
    {
        aOutput.writeString(ScResId(STR_MAGNITUDE_PART));
        aOutput.nextColumn();
        aOutput.writeString(ScResId(STR_PHASE_PART));
    }

    aOutput.newLine();
    aTemplate.autoReplaceRange("%INPUTRANGE%", maActualInputRange);

    OUString aFormula;
    genFormula(aFormula);

    aTemplate.setTemplate(aFormula);
    aOutput.writeMatrixFormula(aTemplate.getTemplate(), 2, mnLen);

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

bool ScFourierAnalysisDialog::InputRangesValid()
{
    if (!mInputRange.IsValid())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_INVALID_INPUT_RANGE));
        return false;
    }

    if (!mOutputAddress.IsValid())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_INVALID_OUTPUT_ADDR));
        return false;
    }

    mInputRange.PutInOrder();

    mbGroupedByColumn = mGroupedBy == BY_COLUMN;
    mbWithLabels = mxWithLabelsCheckBox->get_active();

    mbUse3DAddresses = mInputRange.aStart.Tab() != mOutputAddress.Tab();

    SCSIZE nRows = mInputRange.aEnd.Row() - mInputRange.aStart.Row() + 1;
    SCSIZE nCols = mInputRange.aEnd.Col() - mInputRange.aStart.Col() + 1;

    SCSIZE nLen = mbGroupedByColumn ? nRows : nCols;
    SCSIZE nComponents = mbGroupedByColumn ? nCols : nRows;

    if (nComponents > 2)
    {
        OUString aMsg = mbGroupedByColumn ? ScResId(STR_MESSAGE_INVALID_NUMCOLS)
                                          : ScResId(STR_MESSAGE_INVALID_NUMROWS);
        mxErrorMessage->set_label(aMsg);
        return false;
    }

    if (mbWithLabels && nLen < 2)
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_NODATA_IN_RANGE));
        return false;
    }

    // Include space for writing the title, label and Real/Imaginary/Magnitude/Phase heading.
    SCSIZE nLastOutputRow = mOutputAddress.Row() + nLen + 2;
    if (mbWithLabels)
        --nLastOutputRow;

    if (nLastOutputRow > MAXROW)
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_OUTPUT_TOO_LONG));
        return false;
    }

    ScAddress aActualStart(mInputRange.aStart);

    if (mbWithLabels)
    {
        if (mbGroupedByColumn)
            aActualStart.IncRow();
        else
            aActualStart.IncCol();

        if (nComponents == 1)
            maLabelAddr = mInputRange.aStart;
        else
            mbWithLabels = false;

        mnLen = nLen - 1;
    }
    else
    {
        mnLen = nLen;
    }

    maActualInputRange = ScRange(aActualStart, mInputRange.aEnd);
    mxErrorMessage->set_label("");

    return true;
}

void ScFourierAnalysisDialog::getOptions()
{
    mbInverse = mxInverseCheckBox->get_active();
    mbPolar = mxPolarCheckBox->get_active();

    sal_Int32 nDeciBels = static_cast<sal_Int32>(mxMinMagnitudeField->get_value());
    if (nDeciBels <= -150)
        mfMinMag = 0.0;
    else
        mfMinMag = pow(10.0, static_cast<double>(nDeciBels) / 10.0);
}

void ScFourierAnalysisDialog::getDataLabel(OUString& rLabel)
{
    if (mbWithLabels)
    {
        rLabel = "="
                 + maLabelAddr.Format(mbUse3DAddresses ? ScRefFlags::ADDR_ABS_3D
                                                       : ScRefFlags::ADDR_ABS,
                                      &mDocument, mAddressDetails);

        return;
    }

    OUString aDataSrc(mInputRange.Format(
        mDocument, mbUse3DAddresses ? ScRefFlags::RANGE_ABS_3D : ScRefFlags::RANGE_ABS,
        mAddressDetails));

    rLabel = ScResId(STR_INPUT_DATA_RANGE) + " : " + aDataSrc;
    return;
}

void ScFourierAnalysisDialog::genFormula(OUString& rFormula)
{
    static const OUStringLiteral aSep(u";");

    if (!mbPolar)
    {
        rFormula = "FOURIER(%INPUTRANGE%;" + OUString::boolean(mbGroupedByColumn) + aSep
                   + OUString::boolean(mbInverse) + ")";
        return;
    }

    rFormula = "FOURIER(%INPUTRANGE%;" + OUString::boolean(mbGroupedByColumn) + aSep
               + OUString::boolean(mbInverse) + ";true;" + OUString::number(mfMinMag) + ")";
}

IMPL_LINK_NOARG(ScFourierAnalysisDialog, CheckBoxHdl, weld::ToggleButton&, void)
{
    ValidateDialogInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
