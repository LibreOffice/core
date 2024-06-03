/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SparklineDataRangeDialog.hxx>
#include <Sparkline.hxx>
#include <reffact.hxx>
#include <docfunc.hxx>

namespace sc
{
SparklineDataRangeDialog::SparklineDataRangeDialog(SfxBindings* pBindings,
                                                   SfxChildWindow* pChildWindow,
                                                   weld::Window* pWindow, ScViewData& rViewData)
    : ScAnyRefDlgController(pBindings, pChildWindow, pWindow,
                            u"modules/scalc/ui/sparklinedatarangedialog.ui"_ustr,
                            u"SparklineDataRangeDialog"_ustr)
    , mrViewData(rViewData)
    , mrDocument(rViewData.GetDocument())
    , mpActiveEdit(nullptr)
    , mbDialogLostFocus(false)
    , mxButtonOk(m_xBuilder->weld_button(u"ok"_ustr))
    , mxButtonCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , mxDataRangeLabel(m_xBuilder->weld_label(u"cell-range-label"_ustr))
    , mxDataRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry(u"cell-range-edit"_ustr)))
    , mxDataRangeButton(new formula::RefButton(m_xBuilder->weld_button(u"cell-range-button"_ustr)))

{
    mxDataRangeEdit->SetReferences(this, mxDataRangeLabel.get());
    mxDataRangeButton->SetReferences(this, mxDataRangeEdit.get());

    mxButtonCancel->connect_clicked(LINK(this, SparklineDataRangeDialog, ButtonClicked));
    mxButtonOk->connect_clicked(LINK(this, SparklineDataRangeDialog, ButtonClicked));

    mxButtonOk->set_sensitive(false);

    Link<formula::RefEdit&, void> aEditLink
        = LINK(this, SparklineDataRangeDialog, EditFocusHandler);
    mxDataRangeEdit->SetGetFocusHdl(aEditLink);
    aEditLink = LINK(this, SparklineDataRangeDialog, LoseEditFocusHandler);
    mxDataRangeEdit->SetLoseFocusHdl(aEditLink);

    Link<formula::RefButton&, void> aButtonLink
        = LINK(this, SparklineDataRangeDialog, ButtonFocusHandler);
    mxDataRangeButton->SetGetFocusHdl(aButtonLink);
    aButtonLink = LINK(this, SparklineDataRangeDialog, LoseButtonFocusHandler);
    mxDataRangeButton->SetLoseFocusHdl(aButtonLink);

    Link<formula::RefEdit&, void> aModifyLink
        = LINK(this, SparklineDataRangeDialog, RefInputModifyHandler);
    mxDataRangeEdit->SetModifyHdl(aModifyLink);

    setupValues();

    mxDataRangeEdit->GrabFocus();
}

SparklineDataRangeDialog::~SparklineDataRangeDialog() = default;

void SparklineDataRangeDialog::setupValues()
{
    ScAddress aCurrentAddress = mrViewData.GetCurPos();
    mpSparkline = mrDocument.GetSparkline(aCurrentAddress);

    if (mpSparkline)
    {
        ScRangeList aRangeList(mpSparkline->getInputRange());
        if (!aRangeList.empty())
        {
            maDataRange = aRangeList[0];
            OUString aString
                = maDataRange.Format(mrDocument, ScRefFlags::VALID | ScRefFlags::TAB_3D,
                                     mrDocument.GetAddressConvention());
            mxDataRangeEdit->SetRefString(aString);
            mxButtonOk->set_sensitive(true);
        }
    }
}

void SparklineDataRangeDialog::Close()
{
    DoClose(sc::SparklineDataRangeDialogWrapper::GetChildWindowId());
}

void SparklineDataRangeDialog::SetActive()
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

void SparklineDataRangeDialog::SetReference(const ScRange& rReferenceRange, ScDocument& rDocument)
{
    if (mpActiveEdit)
    {
        if (rReferenceRange.aStart != rReferenceRange.aEnd)
            RefInputStart(mpActiveEdit);

        OUString aString;
        const ScRefFlags eFlags = ScRefFlags::VALID | ScRefFlags::TAB_3D;
        auto eAddressConvention = rDocument.GetAddressConvention();

        if (mpActiveEdit == mxDataRangeEdit.get())
        {
            maDataRange = rReferenceRange;
            aString = maDataRange.Format(rDocument, eFlags, eAddressConvention);
            mxDataRangeEdit->SetRefString(aString);
        }
    }
}

IMPL_LINK(SparklineDataRangeDialog, EditFocusHandler, formula::RefEdit&, rEdit, void)
{
    if (mxDataRangeEdit.get() == &rEdit)
        mpActiveEdit = mxDataRangeEdit.get();
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK(SparklineDataRangeDialog, ButtonFocusHandler, formula::RefButton&, rButton, void)
{
    if (mxDataRangeButton.get() == &rButton)
        mpActiveEdit = mxDataRangeEdit.get();
    else
        mpActiveEdit = nullptr;

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK_NOARG(SparklineDataRangeDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(SparklineDataRangeDialog, LoseButtonFocusHandler, formula::RefButton&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(SparklineDataRangeDialog, RefInputModifyHandler, formula::RefEdit&, void)
{
    if (mpActiveEdit)
    {
        if (mpActiveEdit == mxDataRangeEdit.get())
        {
            ScRangeList aRangeList;
            bool bValid = ParseWithNames(aRangeList, mxDataRangeEdit->GetText(), mrDocument);
            const ScRange* pRange = (bValid && aRangeList.size() == 1) ? &aRangeList[0] : nullptr;
            if (pRange)
            {
                maDataRange = *pRange;
                mxDataRangeEdit->StartUpdateData();
            }
            else
            {
                maDataRange = ScRange(ScAddress::INITIALIZE_INVALID);
            }
        }
    }
}

IMPL_LINK(SparklineDataRangeDialog, ButtonClicked, weld::Button&, rButton, void)
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

void SparklineDataRangeDialog::perform()
{
    ScRangeList aList{ maDataRange };

    auto& rDocFunc = mrViewData.GetDocShell()->GetDocFunc();
    rDocFunc.ChangeSparkline(mpSparkline, mrViewData.GetTabNo(), aList);
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
