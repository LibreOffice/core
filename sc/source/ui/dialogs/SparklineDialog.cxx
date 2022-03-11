/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SparklineDialog.hxx>
#include <SparklineGroup.hxx>
#include <Sparkline.hxx>
#include <reffact.hxx>

namespace sc
{
SparklineDialog::SparklineDialog(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                                 weld::Window* pWindow, ScViewData& rViewData)
    : ScAnyRefDlgController(pBindings, pChildWindow, pWindow,
                            u"modules/scalc/ui/sparklinedialog.ui", "SparklineDialog")
    , mrViewData(rViewData)
    , mrDocument(rViewData.GetDocument())
    , mpActiveEdit(nullptr)
    , mbDialogLostFocus(false)
    , mxButtonOk(m_xBuilder->weld_button("ok"))
    , mxButtonCancel(m_xBuilder->weld_button("cancel"))
    , mxInputRangeText(m_xBuilder->weld_label("cell-range-label"))
    , mxInputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("cell-range-edit")))
    , mxInputRangeButton(new formula::RefButton(m_xBuilder->weld_button("cell-range-button")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("output-range-label"))
    , mxOutputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("output-range-edit")))
    , mxOutputRangeButton(new formula::RefButton(m_xBuilder->weld_button("output-range-button")))
{
    mxInputRangeEdit->SetReferences(this, mxInputRangeText.get());
    mxInputRangeButton->SetReferences(this, mxInputRangeEdit.get());

    mxOutputRangeEdit->SetReferences(this, mxOutputRangeLabel.get());
    mxOutputRangeButton->SetReferences(this, mxOutputRangeEdit.get());

    mxButtonCancel->connect_clicked(LINK(this, SparklineDialog, ButtonClicked));
    mxButtonOk->connect_clicked(LINK(this, SparklineDialog, ButtonClicked));
    mxButtonOk->set_sensitive(false);

    Link<formula::RefEdit&, void> aEditLink = LINK(this, SparklineDialog, EditFocusHandler);
    mxInputRangeEdit->SetGetFocusHdl(aEditLink);
    mxOutputRangeEdit->SetGetFocusHdl(aEditLink);
    aEditLink = LINK(this, SparklineDialog, LoseEditFocusHandler);
    mxInputRangeEdit->SetLoseFocusHdl(aEditLink);
    mxOutputRangeEdit->SetLoseFocusHdl(aEditLink);

    Link<formula::RefButton&, void> aButtonLink = LINK(this, SparklineDialog, ButtonFocusHandler);
    mxInputRangeButton->SetGetFocusHdl(aButtonLink);
    mxOutputRangeButton->SetGetFocusHdl(aButtonLink);
    aButtonLink = LINK(this, SparklineDialog, LoseButtonFocusHandler);
    mxInputRangeButton->SetLoseFocusHdl(aButtonLink);
    mxOutputRangeButton->SetLoseFocusHdl(aButtonLink);

    Link<formula::RefEdit&, void> aModifyLink = LINK(this, SparklineDialog, RefInputModifyHandler);
    mxInputRangeEdit->SetModifyHdl(aModifyLink);
    mxOutputRangeEdit->SetModifyHdl(aModifyLink);

    mxOutputRangeEdit->GrabFocus();

    GetRangeFromSelection();
}

SparklineDialog::~SparklineDialog() {}

void SparklineDialog::Close() { DoClose(sc::SparklineDialogWrapper::GetChildWindowId()); }

void SparklineDialog::SetActive()
{
    if (mbDialogLostFocus)
    {
        mbDialogLostFocus = false;
        if (mpActiveEdit)
            mpActiveEdit->GrabFocus();
    }
    else
    {
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void SparklineDialog::GetRangeFromSelection()
{
    mrViewData.GetSimpleArea(maInputRange);
    OUString aString = maInputRange.Format(mrDocument, ScRefFlags::VALID | ScRefFlags::TAB_3D,
                                           mrDocument.GetAddressConvention());
    mxInputRangeEdit->SetRefString(aString);
}

void SparklineDialog::SetReference(const ScRange& rReferenceRange, ScDocument& rDocument)
{
    if (mpActiveEdit)
    {
        if (rReferenceRange.aStart != rReferenceRange.aEnd)
            RefInputStart(mpActiveEdit);

        OUString aString;
        const ScRefFlags eFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D;
        auto eAddressConvention = rDocument.GetAddressConvention();

        if (mpActiveEdit == mxInputRangeEdit.get())
        {
            maInputRange = rReferenceRange;
            aString = maInputRange.Format(rDocument, eFlags, eAddressConvention);
            mxInputRangeEdit->SetRefString(aString);
        }
        else if (mpActiveEdit == mxOutputRangeEdit.get())
        {
            maOutputRange = rReferenceRange;
            aString = maOutputRange.Format(rDocument, eFlags, eAddressConvention);
            mxOutputRangeEdit->SetRefString(aString);
        }
    }

    mxButtonOk->set_sensitive(checkValidInputOutput());
}

IMPL_LINK(SparklineDialog, EditFocusHandler, formula::RefEdit&, rEdit, void)
{
    auto* pEdit = &rEdit;

    if (mxInputRangeEdit.get() == pEdit)
        mpActiveEdit = mxInputRangeEdit.get();
    else if (mxOutputRangeEdit.get() == pEdit)
        mpActiveEdit = mxOutputRangeEdit.get();
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK(SparklineDialog, ButtonFocusHandler, formula::RefButton&, rButton, void)
{
    auto* pButton = &rButton;

    if (mxInputRangeButton.get() == pButton)
        mpActiveEdit = mxInputRangeEdit.get();
    else if (mxOutputRangeButton.get() == pButton)
        mpActiveEdit = mxOutputRangeEdit.get();
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK_NOARG(SparklineDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(SparklineDialog, LoseButtonFocusHandler, formula::RefButton&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(SparklineDialog, RefInputModifyHandler, formula::RefEdit&, void)
{
    if (mpActiveEdit)
    {
        if (mpActiveEdit == mxInputRangeEdit.get())
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames(aRangeList, mxInputRangeEdit->GetText(), mrDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                maInputRange = *pRange;
                mxInputRangeEdit->StartUpdateData();
            }
            else
            {
                maInputRange = ScRange(ScAddress::INITIALIZE_INVALID);
            }
        }
        else if (mpActiveEdit == mxOutputRangeEdit.get())
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames(aRangeList, mxOutputRangeEdit->GetText(), mrDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                maOutputRange = *pRange;
                mxOutputRangeEdit->StartUpdateData();
            }
            else
            {
                maOutputRange = ScRange(ScAddress::INITIALIZE_INVALID);
            }
        }
    }

    mxButtonOk->set_sensitive(checkValidInputOutput());
}

IMPL_LINK(SparklineDialog, ButtonClicked, weld::Button&, rButton, void)
{
    if (mxButtonOk.get() == &rButton)
    {
        perform();
        response(RET_OK);
    }
    else
    {
        response(RET_CANCEL);
    }
}

namespace
{
enum class RangeOrientation
{
    Unknown,
    Row,
    Col
};

RangeOrientation calculateOrientation(sal_Int32 nOutputSize, ScRange const& rInputRange)
{
    sal_Int32 nRowSize = rInputRange.aEnd.Row() - rInputRange.aStart.Row();
    sal_Int32 nColSize = rInputRange.aEnd.Col() - rInputRange.aStart.Col();

    auto eInputOrientation = RangeOrientation::Unknown;
    if (nOutputSize == nRowSize)
        eInputOrientation = RangeOrientation::Row;
    else if (nOutputSize == nColSize)
        eInputOrientation = RangeOrientation::Col;
    return eInputOrientation;
}

} // end anonymous namespace

bool SparklineDialog::checkValidInputOutput()
{
    if (!maInputRange.IsValid() || !maOutputRange.IsValid())
        return false;

    RangeOrientation eInputOrientation = RangeOrientation::Unknown;
    if (maOutputRange.aStart.Col() == maOutputRange.aEnd.Col())
    {
        sal_Int32 nOutputRowSize = maOutputRange.aEnd.Row() - maOutputRange.aStart.Row();
        eInputOrientation = calculateOrientation(nOutputRowSize, maInputRange);
    }
    else if (maOutputRange.aStart.Row() == maOutputRange.aEnd.Row())
    {
        sal_Int32 nOutputColSize = maOutputRange.aEnd.Col() - maOutputRange.aStart.Col();
        eInputOrientation = calculateOrientation(nOutputColSize, maInputRange);
    }

    return eInputOrientation != RangeOrientation::Unknown;
}

void SparklineDialog::perform()
{
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    if (maOutputRange.aStart.Col() == maOutputRange.aEnd.Col())
    {
        sal_Int32 nOutputRowSize = maOutputRange.aEnd.Row() - maOutputRange.aStart.Row();

        auto eInputOrientation = calculateOrientation(nOutputRowSize, maInputRange);

        if (eInputOrientation == RangeOrientation::Unknown)
            return;

        sal_Int32 nIndex = 0;
        for (ScAddress aAddress = maOutputRange.aStart; aAddress.Row() <= maOutputRange.aEnd.Row();
             aAddress.IncRow())
        {
            ScRange aInputRangeSlice = maInputRange;
            if (eInputOrientation == RangeOrientation::Row)
            {
                aInputRangeSlice.aStart.SetRow(maInputRange.aStart.Row() + nIndex);
                aInputRangeSlice.aEnd.SetRow(maInputRange.aStart.Row() + nIndex);
            }
            else
            {
                aInputRangeSlice.aStart.SetCol(maInputRange.aStart.Col() + nIndex);
                aInputRangeSlice.aEnd.SetCol(maInputRange.aStart.Col() + nIndex);
            }
            auto* pCreated = mrDocument.CreateSparkline(aAddress, pSparklineGroup);
            pCreated->setInputRange(aInputRangeSlice);
            nIndex++;
        }
    }
    else if (maOutputRange.aStart.Row() == maOutputRange.aEnd.Row())
    {
        sal_Int32 nOutputColSize = maOutputRange.aEnd.Col() - maOutputRange.aStart.Col();

        auto eInputOrientation = calculateOrientation(nOutputColSize, maInputRange);

        if (eInputOrientation == RangeOrientation::Unknown)
            return;

        sal_Int32 nIndex = 0;

        for (ScAddress aAddress = maOutputRange.aStart; aAddress.Col() <= maOutputRange.aEnd.Col();
             aAddress.IncCol())
        {
            ScRange aInputRangeSlice = maInputRange;
            if (eInputOrientation == RangeOrientation::Row)
            {
                aInputRangeSlice.aStart.SetRow(maInputRange.aStart.Row() + nIndex);
                aInputRangeSlice.aEnd.SetRow(maInputRange.aStart.Row() + nIndex);
            }
            else
            {
                aInputRangeSlice.aStart.SetCol(maInputRange.aStart.Col() + nIndex);
                aInputRangeSlice.aEnd.SetCol(maInputRange.aStart.Col() + nIndex);
            }
            auto* pCreated = mrDocument.CreateSparkline(aAddress, pSparklineGroup);
            pCreated->setInputRange(aInputRangeSlice);
            nIndex++;
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
