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
    , mxFrameData(m_xBuilder->weld_frame("frmData"))
    , mxInputRangeLabel(m_xBuilder->weld_label("lbInputRange"))
    , mxInputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("edInputRange")))
    , mxInputRangeButton(new formula::RefButton(m_xBuilder->weld_button("btnInputRange")))
    , mxOutputRangeLabel(m_xBuilder->weld_label("lbOutputRange"))
    , mxOutputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry("edOutputRange")))
    , mxOutputRangeButton(new formula::RefButton(m_xBuilder->weld_button("btnOutputRange")))
    , mxColorSeries(new ColorListBox(m_xBuilder->weld_menu_button("colSeries"),
                                     [pWindow] { return pWindow; }))
    , mxColorNegative(new ColorListBox(m_xBuilder->weld_menu_button("colNegative"),
                                       [pWindow] { return pWindow; }))
    , mxColorMarker(new ColorListBox(m_xBuilder->weld_menu_button("colMarker"),
                                     [pWindow] { return pWindow; }))
    , mxColorHigh(
          new ColorListBox(m_xBuilder->weld_menu_button("colHigh"), [pWindow] { return pWindow; }))
    , mxColorLow(
          new ColorListBox(m_xBuilder->weld_menu_button("colLow"), [pWindow] { return pWindow; }))
    , mxColorFirst(
          new ColorListBox(m_xBuilder->weld_menu_button("colFirst"), [pWindow] { return pWindow; }))
    , mxColorLast(
          new ColorListBox(m_xBuilder->weld_menu_button("colLast"), [pWindow] { return pWindow; }))
    , mxCheckButtonNegative(m_xBuilder->weld_check_button("cbNegative"))
    , mxCheckButtonMarker(m_xBuilder->weld_check_button("cbMarker"))
    , mxCheckButtonHigh(m_xBuilder->weld_check_button("cbHigh"))
    , mxCheckButtonLow(m_xBuilder->weld_check_button("cbLow"))
    , mxCheckButtonFirst(m_xBuilder->weld_check_button("cbFirst"))
    , mxCheckButtonLast(m_xBuilder->weld_check_button("cbLast"))
    , mxSpinLineWidth(m_xBuilder->weld_spin_button("seLineWidth"))
    , mxType(m_xBuilder->weld_combo_box("cbType"))
    , mxCheckDisplayXAxis(m_xBuilder->weld_check_button("cbDisplayXAxis"))
    , mxCheckDisplayHidden(m_xBuilder->weld_check_button("cbHidden"))
    , mxCheckRightToLeft(m_xBuilder->weld_check_button("cbRTL"))
    , mxDisplayEmptyGap(m_xBuilder->weld_combo_box("cbEmptyCells"))
    , mxComboMinAxisType(m_xBuilder->weld_combo_box("cbMinAxisType"))
    , mxComboMaxAxisType(m_xBuilder->weld_combo_box("cbMaxAxisType"))
    , mxSpinCustomMin(m_xBuilder->weld_formatted_spin_button("seMinAxis"))
    , mxSpinCustomMax(m_xBuilder->weld_formatted_spin_button("seMaxAxis"))
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

    mxType->connect_changed(LINK(this, SparklineDialog, SelectSparklineType));
    mxDisplayEmptyGap->connect_changed(LINK(this, SparklineDialog, SelectSparklineType));

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
            mpSparklineGroup = pSparkline->getSparklineGroup();
            maAttributes = mpSparklineGroup->getAttributes();
            mxFrameData->set_visible(false);
            mbEditMode = true;
        }
    }
    else
    {
        maInputRange = aSelectionRange;
    }

    setInputSelection();

    switch (maAttributes.getType())
    {
        case sc::SparklineType::Line:
            mxType->set_active(0);
            break;
        case sc::SparklineType::Column:
            mxType->set_active(1);
            break;
        case sc::SparklineType::Stacked:
            mxType->set_active(2);
            break;
    }

    switch (maAttributes.getDisplayEmptyCellsAs())
    {
        case sc::DisplayEmptyCellsAs::Gap:
            mxDisplayEmptyGap->set_active(0);
            break;
        case sc::DisplayEmptyCellsAs::Zero:
            mxDisplayEmptyGap->set_active(1);
            break;
        case sc::DisplayEmptyCellsAs::Span:
            mxDisplayEmptyGap->set_active(2);
            break;
    }

    mxColorSeries->SelectEntry(maAttributes.getColorSeries());
    mxColorNegative->SelectEntry(maAttributes.getColorNegative());
    mxColorMarker->SelectEntry(maAttributes.getColorMarkers());
    mxColorHigh->SelectEntry(maAttributes.getColorHigh());
    mxColorLow->SelectEntry(maAttributes.getColorLow());
    mxColorFirst->SelectEntry(maAttributes.getColorFirst());
    mxColorLast->SelectEntry(maAttributes.getColorLast());

    mxCheckButtonNegative->set_active(maAttributes.isNegative());
    mxCheckButtonMarker->set_active(maAttributes.isMarkers());
    mxCheckButtonHigh->set_active(maAttributes.isHigh());
    mxCheckButtonLow->set_active(maAttributes.isLow());
    mxCheckButtonFirst->set_active(maAttributes.isFirst());
    mxCheckButtonLast->set_active(maAttributes.isLast());

    mxSpinLineWidth->set_value(sal_Int64(maAttributes.getLineWeight() * 100.0));

    mxCheckDisplayXAxis->set_active(maAttributes.shouldDisplayXAxis());
    mxCheckDisplayHidden->set_active(maAttributes.shouldDisplayHidden());
    mxCheckRightToLeft->set_active(maAttributes.isRightToLeft());

    switch (maAttributes.getMinAxisType())
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
            if (maAttributes.getManualMin())
                mxSpinCustomMin->GetFormatter().SetValue(*maAttributes.getManualMin());
            break;
    }
    ComboValueChanged(*mxComboMinAxisType);

    switch (maAttributes.getMaxAxisType())
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
            if (maAttributes.getManualMax())
                mxSpinCustomMax->GetFormatter().SetValue(*maAttributes.getManualMax());
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
    if (mxCheckButtonNegative.get() == &rToggle)
        maAttributes.setNegative(mxCheckButtonNegative->get_active());
    if (mxCheckButtonMarker.get() == &rToggle)
        maAttributes.setMarkers(mxCheckButtonMarker->get_active());
    if (mxCheckButtonHigh.get() == &rToggle)
        maAttributes.setHigh(mxCheckButtonHigh->get_active());
    if (mxCheckButtonLow.get() == &rToggle)
        maAttributes.setLow(mxCheckButtonLow->get_active());
    if (mxCheckButtonFirst.get() == &rToggle)
        maAttributes.setFirst(mxCheckButtonFirst->get_active());
    if (mxCheckButtonLast.get() == &rToggle)
        maAttributes.setLast(mxCheckButtonLast->get_active());
    if (mxCheckDisplayXAxis.get() == &rToggle)
        maAttributes.setDisplayXAxis(mxCheckDisplayXAxis->get_active());
    if (mxCheckDisplayHidden.get() == &rToggle)
        maAttributes.setDisplayHidden(mxCheckDisplayHidden->get_active());
    if (mxCheckRightToLeft.get() == &rToggle)
        maAttributes.setRightToLeft(mxCheckRightToLeft->get_active());
}

IMPL_LINK_NOARG(SparklineDialog, SelectSparklineType, weld::ComboBox&, void)
{
    switch (mxType->get_active())
    {
        case 0:
            maAttributes.setType(sc::SparklineType::Line);
            break;
        case 1:
            maAttributes.setType(sc::SparklineType::Column);
            break;
        case 2:
            maAttributes.setType(sc::SparklineType::Stacked);
            break;
    }
    switch (mxDisplayEmptyGap->get_active())
    {
        case 1:
            maAttributes.setDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs::Gap);
            break;
        case 2:
            maAttributes.setDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs::Zero);
            break;
        case 3:
            maAttributes.setDisplayEmptyCellsAs(sc::DisplayEmptyCellsAs::Span);
            break;
    }
}

IMPL_LINK_NOARG(SparklineDialog, SpinLineWidthChanged, weld::SpinButton&, void)
{
    double value = mxSpinLineWidth->get_value() / 100.0;
    maAttributes.setLineWeight(value);
}

IMPL_LINK(SparklineDialog, SpinCustomChanged, weld::FormattedSpinButton&, rFormatted, void)
{
    if (mxSpinCustomMin.get() == &rFormatted)
    {
        maAttributes.setManualMin(rFormatted.GetFormatter().GetValue());
    }
    else if (mxSpinCustomMax.get() == &rFormatted)
    {
        maAttributes.setManualMax(rFormatted.GetFormatter().GetValue());
    }
}

IMPL_LINK(SparklineDialog, ComboValueChanged, weld::ComboBox&, rComboBox, void)
{
    int nActive = rComboBox.get_active();

    if (mxComboMinAxisType.get() == &rComboBox)
    {
        switch (nActive)
        {
            case 0:
                maAttributes.setMinAxisType(sc::AxisType::Individual);
                mxSpinCustomMin->set_sensitive(false);
                break;
            case 1:
                maAttributes.setMinAxisType(sc::AxisType::Group);
                mxSpinCustomMin->set_sensitive(false);
                break;
            case 2:
                maAttributes.setMinAxisType(sc::AxisType::Custom);
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
                maAttributes.setMaxAxisType(sc::AxisType::Individual);
                mxSpinCustomMax->set_sensitive(false);
                break;
            case 1:
                maAttributes.setMaxAxisType(sc::AxisType::Group);
                mxSpinCustomMax->set_sensitive(false);
                break;
            case 2:
                maAttributes.setMaxAxisType(sc::AxisType::Custom);
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
    maAttributes.setColorSeries(mxColorSeries->GetSelectEntryColor());
    maAttributes.setColorNegative(mxColorNegative->GetSelectEntryColor());
    maAttributes.setColorMarkers(mxColorMarker->GetSelectEntryColor());
    maAttributes.setColorHigh(mxColorHigh->GetSelectEntryColor());
    maAttributes.setColorLow(mxColorLow->GetSelectEntryColor());
    maAttributes.setColorFirst(mxColorFirst->GetSelectEntryColor());
    maAttributes.setColorLast(mxColorLast->GetSelectEntryColor());

    auto& rDocFunc = mrViewData.GetDocShell()->GetDocFunc();

    if (mpSparklineGroup)
    {
        rDocFunc.ChangeSparklineGroupAttributes(mpSparklineGroup, maAttributes);
    }
    else
    {
        auto pNewSparklineGroup = std::make_shared<sc::SparklineGroup>(maAttributes);
        rDocFunc.InsertSparklines(maInputRange, maOutputRange, pNewSparklineGroup);
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
