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
#include <vcl/formatter.hxx>

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
    , mxFrameData(m_xBuilder->weld_frame("frame-data"))
    , mxInputRangeLabel(m_xBuilder->weld_label("cell-range-label"))
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
    , mxSpinLineWidth(m_xBuilder->weld_spin_button("spin-line-width"))
    , mxRadioLine(m_xBuilder->weld_radio_button("line-radiobutton"))
    , mxRadioColumn(m_xBuilder->weld_radio_button("column-radiobutton"))
    , mxRadioStacked(m_xBuilder->weld_radio_button("stacked-radiobutton"))
    , mxCheckDisplayXAxis(m_xBuilder->weld_check_button("check-display-x-axis"))
    , mxCheckDisplayHidden(m_xBuilder->weld_check_button("check-display-hidden"))
    , mxCheckRightToLeft(m_xBuilder->weld_check_button("check-right-to-left"))
    , mxRadioDisplayEmptyGap(m_xBuilder->weld_radio_button("display-empty-radiobutton-gap"))
    , mxRadioDisplayEmptyZero(m_xBuilder->weld_radio_button("display-empty-radiobutton-zero"))
    , mxRadioDisplayEmptySpan(m_xBuilder->weld_radio_button("display-empty-radiobutton-span"))
    , mxComboMinAxisType(m_xBuilder->weld_combo_box("combo-min-axis-type"))
    , mxComboMaxAxisType(m_xBuilder->weld_combo_box("combo-max-axis-type"))
    , mxSpinCustomMin(m_xBuilder->weld_formatted_spin_button("spin-custom-min"))
    , mxSpinCustomMax(m_xBuilder->weld_formatted_spin_button("spin-custom-max"))
    , mbEditMode(false)
{
    mxInputRangeEdit->SetReferences(this, mxInputRangeLabel.get());
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
    mxRadioDisplayEmptyGap->connect_toggled(aRadioButtonLink);
    mxRadioDisplayEmptyZero->connect_toggled(aRadioButtonLink);
    mxRadioDisplayEmptySpan->connect_toggled(aRadioButtonLink);

    Link<weld::Toggleable&, void> aLink = LINK(this, SparklineDialog, ToggleHandler);
    mxCheckButtonNegative->connect_toggled(aLink);
    mxCheckButtonMarker->connect_toggled(aLink);
    mxCheckButtonHigh->connect_toggled(aLink);
    mxCheckButtonLow->connect_toggled(aLink);
    mxCheckButtonFirst->connect_toggled(aLink);
    mxCheckButtonLast->connect_toggled(aLink);
    mxCheckDisplayXAxis->connect_toggled(aLink);
    mxCheckDisplayHidden->connect_toggled(aLink);
    mxCheckRightToLeft->connect_toggled(aLink);

    mxSpinLineWidth->connect_value_changed(LINK(this, SparklineDialog, SpinLineWidthChanged));

    mxComboMinAxisType->connect_changed(LINK(this, SparklineDialog, ComboValueChanged));
    mxComboMaxAxisType->connect_changed(LINK(this, SparklineDialog, ComboValueChanged));

    mxSpinCustomMin->connect_value_changed(LINK(this, SparklineDialog, SpinCustomChanged));
    Formatter& rSpinCustomMinFormatter = mxSpinCustomMin->GetFormatter();
    rSpinCustomMinFormatter.ClearMinValue();
    rSpinCustomMinFormatter.ClearMaxValue();
    rSpinCustomMinFormatter.UseInputStringForFormatting();

    mxSpinCustomMax->connect_value_changed(LINK(this, SparklineDialog, SpinCustomChanged));
    Formatter& rSpinCustomMaxFormatter = mxSpinCustomMax->GetFormatter();
    rSpinCustomMaxFormatter.ClearMinValue();
    rSpinCustomMaxFormatter.ClearMaxValue();
    rSpinCustomMaxFormatter.UseInputStringForFormatting();

    setupValues();

    mxOutputRangeEdit->GrabFocus();
    mxButtonOk->set_sensitive(checkValidInputOutput());
}

SparklineDialog::~SparklineDialog() = default;

void SparklineDialog::setInputSelection()
{
    mrViewData.GetSimpleArea(maInputRange);
    OUString aString = maInputRange.Format(mrDocument, ScRefFlags::VALID | ScRefFlags::TAB_3D,
                                           mrDocument.GetAddressConvention());
    mxInputRangeEdit->SetRefString(aString);
}

void SparklineDialog::setupValues()
{
    ScRange aSelectionRange;
    mrViewData.GetSimpleArea(aSelectionRange);

    if (mrDocument.HasOneSparklineGroup(aSelectionRange))
    {
        if (auto pSparkline = mrDocument.GetSparkline(aSelectionRange.aStart))
        {
            mpLocalSparklineGroup = pSparkline->getSparklineGroup();
            mxFrameData->set_visible(false);
            mbEditMode = true;
        }
    }
    else
    {
        maInputRange = aSelectionRange;
    }

    if (!mpLocalSparklineGroup)
    {
        mpLocalSparklineGroup = std::make_shared<sc::SparklineGroup>();
    }

    setInputSelection();

    auto const& rAttribute = mpLocalSparklineGroup->getAttributes();

    switch (rAttribute.getType())
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

    switch (rAttribute.getDisplayEmptyCellsAs())
    {
        case sc::DisplayEmptyCellsAs::Gap:
            mxRadioDisplayEmptyGap->set_active(true);
            break;
        case sc::DisplayEmptyCellsAs::Zero:
            mxRadioDisplayEmptyZero->set_active(true);
            break;
        case sc::DisplayEmptyCellsAs::Span:
            mxRadioDisplayEmptySpan->set_active(true);
            break;
    }

    mxColorSeries->SelectEntry(rAttribute.getColorSeries());
    mxColorNegative->SelectEntry(rAttribute.getColorNegative());
    mxColorMarker->SelectEntry(rAttribute.getColorMarkers());
    mxColorHigh->SelectEntry(rAttribute.getColorHigh());
    mxColorLow->SelectEntry(rAttribute.getColorLow());
    mxColorFirst->SelectEntry(rAttribute.getColorFirst());
    mxColorLast->SelectEntry(rAttribute.getColorLast());

    mxCheckButtonNegative->set_active(rAttribute.isNegative());
    mxCheckButtonMarker->set_active(rAttribute.isMarkers());
    mxCheckButtonHigh->set_active(rAttribute.isHigh());
    mxCheckButtonLow->set_active(rAttribute.isLow());
    mxCheckButtonFirst->set_active(rAttribute.isFirst());
    mxCheckButtonLast->set_active(rAttribute.isLast());

    mxSpinLineWidth->set_value(sal_Int64(rAttribute.getLineWeight() * 100.0));

    mxCheckDisplayXAxis->set_active(rAttribute.shouldDisplayXAxis());
    mxCheckDisplayHidden->set_active(rAttribute.shouldDisplayHidden());
    mxCheckRightToLeft->set_active(rAttribute.isRightToLeft());

    switch (rAttribute.getMinAxisType())
    {
        case sc::AxisType::Individual:
            mxComboMinAxisType->set_active(0);
            mxSpinCustomMin->GetFormatter().SetValue(0.0);
            break;
        case sc::AxisType::Group:
            mxComboMinAxisType->set_active(1);
            mxSpinCustomMin->GetFormatter().SetValue(0.0);
            break;
        case sc::AxisType::Custom:
            mxComboMinAxisType->set_active(2);
            if (rAttribute.getManualMin())
                mxSpinCustomMin->GetFormatter().SetValue(*rAttribute.getManualMin());
            break;
    }
    ComboValueChanged(*mxComboMinAxisType);

    switch (rAttribute.getMaxAxisType())
    {
        case sc::AxisType::Individual:
            mxComboMaxAxisType->set_active(0);
            mxSpinCustomMax->GetFormatter().SetValue(0.0);
            break;
        case sc::AxisType::Group:
            mxComboMaxAxisType->set_active(1);
            mxSpinCustomMax->GetFormatter().SetValue(0.0);
            break;
        case sc::AxisType::Custom:
            mxComboMaxAxisType->set_active(2);
            if (rAttribute.getManualMin())
                mxSpinCustomMax->GetFormatter().SetValue(*rAttribute.getManualMax());
            break;
    }
    ComboValueChanged(*mxComboMaxAxisType);
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
    auto& rAttribute = mpLocalSparklineGroup->getAttributes();

    if (mxCheckButtonNegative.get() == &rToggle)
        rAttribute.setNegative(mxCheckButtonNegative->get_active());
    if (mxCheckButtonMarker.get() == &rToggle)
        rAttribute.setMarkers(mxCheckButtonMarker->get_active());
    if (mxCheckButtonHigh.get() == &rToggle)
        rAttribute.setHigh(mxCheckButtonHigh->get_active());
    if (mxCheckButtonLow.get() == &rToggle)
        rAttribute.setLow(mxCheckButtonLow->get_active());
    if (mxCheckButtonFirst.get() == &rToggle)
        rAttribute.setFirst(mxCheckButtonFirst->get_active());
    if (mxCheckButtonLast.get() == &rToggle)
        rAttribute.setLast(mxCheckButtonLast->get_active());
    if (mxCheckDisplayXAxis.get() == &rToggle)
        rAttribute.setDisplayXAxis(mxCheckDisplayXAxis->get_active());
    if (mxCheckDisplayHidden.get() == &rToggle)
        rAttribute.setDisplayHidden(mxCheckDisplayHidden->get_active());
    if (mxCheckRightToLeft.get() == &rToggle)
        rAttribute.setRightToLeft(mxCheckRightToLeft->get_active());
}

IMPL_LINK_NOARG(SparklineDialog, SelectSparklineType, weld::Toggleable&, void)
{
    auto& rAttribute = mpLocalSparklineGroup->getAttributes();

    if (mxRadioLine->get_active())
        rAttribute.setType(sc::SparklineType::Line);
    else if (mxRadioColumn->get_active())
        rAttribute.setType(sc::SparklineType::Column);
    else if (mxRadioStacked->get_active())
        rAttribute.setType(sc::SparklineType::Stacked);

    if (mxRadioDisplayEmptyGap->get_active())
        rAttribute.setDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs::Gap);
    else if (mxRadioDisplayEmptyZero->get_active())
        rAttribute.setDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs::Zero);
    else if (mxRadioDisplayEmptySpan->get_active())
        rAttribute.setDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs::Span);
}

IMPL_LINK_NOARG(SparklineDialog, SpinLineWidthChanged, weld::SpinButton&, void)
{
    auto& rAttribute = mpLocalSparklineGroup->getAttributes();

    double value = mxSpinLineWidth->get_value() / 100.0;
    rAttribute.setLineWeight(value);
}

IMPL_LINK(SparklineDialog, SpinCustomChanged, weld::FormattedSpinButton&, rFormatted, void)
{
    auto& rAttribute = mpLocalSparklineGroup->getAttributes();

    if (mxSpinCustomMin.get() == &rFormatted)
    {
        rAttribute.setManualMin(rFormatted.GetFormatter().GetValue());
    }
    else if (mxSpinCustomMax.get() == &rFormatted)
    {
        rAttribute.setManualMax(rFormatted.GetFormatter().GetValue());
    }
}

IMPL_LINK(SparklineDialog, ComboValueChanged, weld::ComboBox&, rComboBox, void)
{
    auto& rAttribute = mpLocalSparklineGroup->getAttributes();
    int nActive = rComboBox.get_active();

    if (mxComboMinAxisType.get() == &rComboBox)
    {
        switch (nActive)
        {
            case 0:
                rAttribute.setMinAxisType(sc::AxisType::Individual);
                mxSpinCustomMin->set_sensitive(false);
                break;
            case 1:
                rAttribute.setMinAxisType(sc::AxisType::Group);
                mxSpinCustomMin->set_sensitive(false);
                break;
            case 2:
                rAttribute.setMinAxisType(sc::AxisType::Custom);
                mxSpinCustomMin->set_sensitive(true);
                break;
            default:
                break;
        }
    }
    else if (mxComboMaxAxisType.get() == &rComboBox)
    {
        switch (nActive)
        {
            case 0:
                rAttribute.setMaxAxisType(sc::AxisType::Individual);
                mxSpinCustomMax->set_sensitive(false);
                break;
            case 1:
                rAttribute.setMaxAxisType(sc::AxisType::Group);
                mxSpinCustomMax->set_sensitive(false);
                break;
            case 2:
                rAttribute.setMaxAxisType(sc::AxisType::Custom);
                mxSpinCustomMax->set_sensitive(true);
                break;
            default:
                break;
        }
    }
}

bool SparklineDialog::checkValidInputOutput()
{
    if (mbEditMode)
        return true;

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
    auto& rAttribute = mpLocalSparklineGroup->getAttributes();

    rAttribute.setColorSeries(mxColorSeries->GetSelectEntryColor());
    rAttribute.setColorNegative(mxColorNegative->GetSelectEntryColor());
    rAttribute.setColorMarkers(mxColorMarker->GetSelectEntryColor());
    rAttribute.setColorHigh(mxColorHigh->GetSelectEntryColor());
    rAttribute.setColorLow(mxColorLow->GetSelectEntryColor());
    rAttribute.setColorFirst(mxColorFirst->GetSelectEntryColor());
    rAttribute.setColorLast(mxColorLast->GetSelectEntryColor());

    auto& rDocFunc = mrViewData.GetDocShell()->GetDocFunc();

    rDocFunc.InsertSparklines(maInputRange, maOutputRange, mpLocalSparklineGroup);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
