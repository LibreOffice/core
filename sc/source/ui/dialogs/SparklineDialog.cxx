/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SparklineDialog.hxx>
#include <SparklineData.hxx>
#include <SparklineGroup.hxx>
#include <Sparkline.hxx>
#include <reffact.hxx>

#include <docfunc.hxx>

#include <svx/colorbox.hxx>

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
    , mxColorSeries(new ColorListBox(m_xBuilder->weld_menu_button("color-button-series"),
                                     [pWindow] { return pWindow; }))
    , mxColorNegative(new ColorListBox(m_xBuilder->weld_menu_button("color-button-negative"),
                                       [pWindow] { return pWindow; }))
    , mxColorMarker(new ColorListBox(m_xBuilder->weld_menu_button("color-button-marker"),
                                     [pWindow] { return pWindow; }))
    , mxColorHigh(new ColorListBox(m_xBuilder->weld_menu_button("color-button-high"),
                                   [pWindow] { return pWindow; }))
    , mxColorLow(new ColorListBox(m_xBuilder->weld_menu_button("color-button-low"),
                                  [pWindow] { return pWindow; }))
    , mxColorFirst(new ColorListBox(m_xBuilder->weld_menu_button("color-button-first"),
                                    [pWindow] { return pWindow; }))
    , mxColorLast(new ColorListBox(m_xBuilder->weld_menu_button("color-button-last"),
                                   [pWindow] { return pWindow; }))
    , mxCheckButtonNegative(m_xBuilder->weld_check_button("check-negative"))
    , mxCheckButtonMarker(m_xBuilder->weld_check_button("check-marker"))
    , mxCheckButtonHigh(m_xBuilder->weld_check_button("check-high"))
    , mxCheckButtonLow(m_xBuilder->weld_check_button("check-low"))
    , mxCheckButtonFirst(m_xBuilder->weld_check_button("check-first"))
    , mxCheckButtonLast(m_xBuilder->weld_check_button("check-last"))
    , mxRadioLine(m_xBuilder->weld_radio_button("line-radiobutton"))
    , mxRadioColumn(m_xBuilder->weld_radio_button("column-radiobutton"))
    , mxRadioStacked(m_xBuilder->weld_radio_button("stacked-radiobutton"))
    , mpLocalSparklineGroup(new sc::SparklineGroup())
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

    Link<weld::Toggleable&, void> aRadioButtonLink
        = LINK(this, SparklineDialog, SelectSparklineType);
    mxRadioLine->connect_toggled(aRadioButtonLink);
    mxRadioColumn->connect_toggled(aRadioButtonLink);
    mxRadioStacked->connect_toggled(aRadioButtonLink);

    Link<weld::Toggleable&, void> aLink = LINK(this, SparklineDialog, ToggleHandler);
    mxCheckButtonNegative->connect_toggled(aLink);
    mxCheckButtonMarker->connect_toggled(aLink);
    mxCheckButtonHigh->connect_toggled(aLink);
    mxCheckButtonLow->connect_toggled(aLink);
    mxCheckButtonFirst->connect_toggled(aLink);
    mxCheckButtonLast->connect_toggled(aLink);

    setupValues(mpLocalSparklineGroup);

    GetRangeFromSelection();

    mxOutputRangeEdit->GrabFocus();
}

SparklineDialog::~SparklineDialog() {}

void SparklineDialog::setupValues(std::shared_ptr<sc::SparklineGroup> const& pSparklineGroup)
{
    switch (pSparklineGroup->m_eType)
    {
        case sc::SparklineType::Line:
            mxRadioLine->set_active(true);
            break;
        case sc::SparklineType::Column:
            mxRadioColumn->set_active(true);
            break;
        case sc::SparklineType::Stacked:
            mxRadioStacked->set_active(true);
            break;
    }

    mxColorSeries->SelectEntry(pSparklineGroup->m_aColorSeries);
    mxColorNegative->SelectEntry(pSparklineGroup->m_aColorNegative);
    mxColorMarker->SelectEntry(pSparklineGroup->m_aColorMarkers);
    mxColorHigh->SelectEntry(pSparklineGroup->m_aColorHigh);
    mxColorLow->SelectEntry(pSparklineGroup->m_aColorLow);
    mxColorFirst->SelectEntry(pSparklineGroup->m_aColorFirst);
    mxColorLast->SelectEntry(pSparklineGroup->m_aColorLast);

    mxCheckButtonNegative->set_active(pSparklineGroup->m_bNegative);
    mxCheckButtonMarker->set_active(pSparklineGroup->m_bMarkers);
    mxCheckButtonHigh->set_active(pSparklineGroup->m_bHigh);
    mxCheckButtonLow->set_active(pSparklineGroup->m_bLow);
    mxCheckButtonFirst->set_active(pSparklineGroup->m_bFirst);
    mxCheckButtonLast->set_active(pSparklineGroup->m_bLast);
}

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

IMPL_LINK(SparklineDialog, ToggleHandler, weld::Toggleable&, rToggle, void)
{
    if (mxCheckButtonNegative.get() == &rToggle)
        mpLocalSparklineGroup->m_bNegative = mxCheckButtonNegative->get_active();
    if (mxCheckButtonMarker.get() == &rToggle)
        mpLocalSparklineGroup->m_bMarkers = mxCheckButtonMarker->get_active();
    if (mxCheckButtonHigh.get() == &rToggle)
        mpLocalSparklineGroup->m_bHigh = mxCheckButtonHigh->get_active();
    if (mxCheckButtonLow.get() == &rToggle)
        mpLocalSparklineGroup->m_bLow = mxCheckButtonLow->get_active();
    if (mxCheckButtonFirst.get() == &rToggle)
        mpLocalSparklineGroup->m_bFirst = mxCheckButtonFirst->get_active();
    if (mxCheckButtonLast.get() == &rToggle)
        mpLocalSparklineGroup->m_bLast = mxCheckButtonLast->get_active();
}

IMPL_LINK_NOARG(SparklineDialog, SelectSparklineType, weld::Toggleable&, void)
{
    if (mxRadioLine->get_active())
        mpLocalSparklineGroup->m_eType = sc::SparklineType::Line;
    else if (mxRadioColumn->get_active())
        mpLocalSparklineGroup->m_eType = sc::SparklineType::Column;
    else if (mxRadioStacked->get_active())
        mpLocalSparklineGroup->m_eType = sc::SparklineType::Stacked;
}

bool SparklineDialog::checkValidInputOutput()
{
    if (!maInputRange.IsValid() || !maOutputRange.IsValid())
        return false;

    sc::RangeOrientation eInputOrientation = sc::RangeOrientation::Unknown;
    if (maOutputRange.aStart.Col() == maOutputRange.aEnd.Col())
    {
        sal_Int32 nOutputRowSize = maOutputRange.aEnd.Row() - maOutputRange.aStart.Row();
        eInputOrientation = sc::calculateOrientation(nOutputRowSize, maInputRange);
    }
    else if (maOutputRange.aStart.Row() == maOutputRange.aEnd.Row())
    {
        sal_Int32 nOutputColSize = maOutputRange.aEnd.Col() - maOutputRange.aStart.Col();
        eInputOrientation = sc::calculateOrientation(nOutputColSize, maInputRange);
    }

    return eInputOrientation != sc::RangeOrientation::Unknown;
}

void SparklineDialog::perform()
{
    mpLocalSparklineGroup->m_aColorSeries = mxColorSeries->GetSelectEntryColor();
    mpLocalSparklineGroup->m_aColorNegative = mxColorNegative->GetSelectEntryColor();
    mpLocalSparklineGroup->m_aColorMarkers = mxColorMarker->GetSelectEntryColor();
    mpLocalSparklineGroup->m_aColorHigh = mxColorHigh->GetSelectEntryColor();
    mpLocalSparklineGroup->m_aColorLow = mxColorLow->GetSelectEntryColor();
    mpLocalSparklineGroup->m_aColorFirst = mxColorFirst->GetSelectEntryColor();
    mpLocalSparklineGroup->m_aColorLast = mxColorLast->GetSelectEntryColor();

    auto& rDocFunc = mrViewData.GetDocShell()->GetDocFunc();

    rDocFunc.InsertSparklines(maInputRange, maOutputRange, mpLocalSparklineGroup);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
