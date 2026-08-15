/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart2/AxisOrientation.hpp>

#include <comphelper/scopeguard.hxx>
#include <sal/log.hxx>
#include <vcl/formatter.hxx>
#include <vcl/svapp.hxx>

#include <Axis.hxx>
#include <AxisHelper.hxx>
#include "ChartAxisPanel.hxx"
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ChartType.hxx>

using namespace css;
using namespace css::uno;
using namespace cpo::uno;

namespace chart::sidebar {

namespace {

bool isLabelShown(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    rtl::Reference< ::chart::Axis > xAxis = ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return false;

    cpo::uno::Any aAny = xAxis->getPropertyValue(u"DisplayLabels"_ustr);
    if (!aAny.hasValue())
        return false;

    bool bVisible = false;
    aAny >>= bVisible;
    return bVisible;
}

void setLabelShown(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, bool bVisible)
{
    rtl::Reference< ::chart::Axis > xAxis = ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return;

    xAxis->setPropertyValue(u"DisplayLabels"_ustr, cpo::uno::Any(bVisible));
}

struct AxisLabelPosMap
{
    sal_Int32 nPos;
    css::chart::ChartAxisLabelPosition ePos;
};

AxisLabelPosMap const aLabelPosMap[] = {
    { 0, css::chart::ChartAxisLabelPosition_NEAR_AXIS },
    { 1, css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE },
    { 2, css::chart::ChartAxisLabelPosition_OUTSIDE_START },
    { 3, css::chart::ChartAxisLabelPosition_OUTSIDE_END }
};

sal_Int32 getLabelPosition(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    rtl::Reference< ::chart::Axis > xAxis = ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return 0;

    cpo::uno::Any aAny = xAxis->getPropertyValue(u"LabelPosition"_ustr);
    if (!aAny.hasValue())
        return 0;

    css::chart::ChartAxisLabelPosition ePos;
    aAny >>= ePos;
    for (AxisLabelPosMap const & i : aLabelPosMap)
    {
        if (i.ePos == ePos)
            return i.nPos;
    }

    return 0;
}

void setLabelPosition(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, sal_Int32 nPos)
{
    rtl::Reference< ::chart::Axis > xAxis = ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return;

    css::chart::ChartAxisLabelPosition ePos;
    for (AxisLabelPosMap const & i : aLabelPosMap)
    {
        if (i.nPos == nPos)
            ePos = i.ePos;
    }

    xAxis->setPropertyValue(u"LabelPosition"_ustr, cpo::uno::Any(ePos));
}

bool isReverse(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    rtl::Reference< Axis > xAxis =
        ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return false;

    css::chart2::ScaleData aData = xAxis->getScaleData();

    return aData.Orientation == css::chart2::AxisOrientation_REVERSE;
}

void setReverse(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, bool bReverse)
{
    rtl::Reference< Axis > xAxis =
        ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return;

    css::chart2::ScaleData aData = xAxis->getScaleData();
    if (bReverse)
        aData.Orientation = css::chart2::AxisOrientation_REVERSE;
    else
        aData.Orientation = css::chart2::AxisOrientation_MATHEMATICAL;

    xAxis->setScaleData(aData);
}

OUString getCID(const css::uno::Reference<css::frame::XModel>& xModel)
{
    css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController());
    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(xController, css::uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return OUString();

    cpo::uno::Any aAny = xSelectionSupplier->getSelection();
    OUString aCID;
    aAny >>= aCID;
#if defined DBG_UTIL && !defined NDEBUG
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if(eType != OBJECTTYPE_AXIS)
        SAL_WARN("chart2","Selected item is not an axis");
#endif

    return aCID;
}

void setAxisRotation(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID, double nVal)
{
    rtl::Reference< ::chart::Axis > xAxis =
        ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return;

    xAxis->setPropertyValue(u"TextRotation"_ustr, cpo::uno::Any(nVal));
}

double getAxisRotation(const rtl::Reference<::chart::ChartModel>& xModel,
        std::u16string_view rCID)
{
    rtl::Reference< ::chart::Axis > xAxis =
        ObjectIdentifier::getAxisForCID(rCID, xModel);

    if (!xAxis.is())
        return 0;

    cpo::uno::Any aAny = xAxis->getPropertyValue(u"TextRotation"_ustr);
    double nVal = 0;
    aAny >>= nVal;
    return nVal;
}

rtl::Reference<::chart::ChartType> getHistogramChartType(
    const rtl::Reference<::chart::ChartModel>& xModel, std::u16string_view rCID)
{
    if (!xModel.is())
    {
        return nullptr;
    }

    return AxisHelper::getHistogramChartTypeOfAxis(ObjectIdentifier::getAxisForCID(rCID, xModel),
                                                   xModel);
}

}

ChartAxisPanel::ChartAxisPanel(
    weld::Widget* pParent,
    ChartController* pController)
    : PanelLayout(pParent, u"ChartAxisPanel"_ustr, u"modules/schart/ui/sidebaraxis.ui"_ustr)
    , mxCBShowLabel(m_xBuilder->weld_check_button(u"checkbutton_show_label"_ustr))
    , mxCBReverse(m_xBuilder->weld_check_button(u"checkbutton_reverse"_ustr))
    , mxLBLabelPos(m_xBuilder->weld_combo_box(u"comboboxtext_label_position"_ustr))
    , mxGridLabel(m_xBuilder->weld_widget(u"label_props"_ustr))
    , mxNFRotation(m_xBuilder->weld_metric_spin_button(u"spinbutton1"_ustr, FieldUnit::DEGREE))
    , mxBxHistogramBinning(m_xBuilder->weld_widget(u"boxHISTOGRAM_BINNING"_ustr))
    , mxRBHistogramAutomatic(m_xBuilder->weld_radio_button(u"RB_HISTOGRAM_AUTOMATIC"_ustr))
    , mxRBHistogramBinWidth(m_xBuilder->weld_radio_button(u"RB_HISTOGRAM_BIN_WIDTH"_ustr))
    , mxHistogramBinWidth(m_xBuilder->weld_formatted_spin_button(u"EDT_HISTOGRAM_BIN_WIDTH"_ustr))
    , mxRBHistogramBinCount(m_xBuilder->weld_radio_button(u"RB_HISTOGRAM_BIN_COUNT"_ustr))
    , mxHistogramBinCount(m_xBuilder->weld_spin_button(u"MT_HISTOGRAM_BIN_COUNT"_ustr))
    , mxCBHistogramOverflow(m_xBuilder->weld_check_button(u"CBX_HISTOGRAM_OVERFLOW"_ustr))
    , mxHistogramOverflow(m_xBuilder->weld_formatted_spin_button(u"EDT_HISTOGRAM_OVERFLOW"_ustr))
    , mxCBHistogramUnderflow(m_xBuilder->weld_check_button(u"CBX_HISTOGRAM_UNDERFLOW"_ustr))
    , mxHistogramUnderflow(m_xBuilder->weld_formatted_spin_button(u"EDT_HISTOGRAM_UNDERFLOW"_ustr))
    , mxModel(pController->getChartModel())
    , mxModifyListener(new ChartSidebarModifyListener(this))
    , mxSelectionListener(new ChartSidebarSelectionListener(this, OBJECTTYPE_AXIS))
    , mbModelValid(true)
    , mbUpdating(false)
{
    Initialize();
}

ChartAxisPanel::~ChartAxisPanel()
{
    doUpdateModel(nullptr);

    mxCBShowLabel.reset();
    mxCBReverse.reset();

    mxLBLabelPos.reset();
    mxGridLabel.reset();

    mxNFRotation.reset();
}

void ChartAxisPanel::Initialize()
{
    mxModel->addModifyListener(mxModifyListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);

    for (weld::FormattedSpinButton* pField : { mxHistogramBinWidth.get(), mxHistogramOverflow.get(),
                                               mxHistogramUnderflow.get() })
    {
        pField->GetFormatter().ClearMinValue();
        pField->GetFormatter().ClearMaxValue();
    }

    updateData();

    Link<weld::Toggleable&,void> aLink = LINK(this, ChartAxisPanel, CheckBoxHdl);
    mxCBShowLabel->connect_toggled(aLink);
    mxCBReverse->connect_toggled(aLink);

    Link<weld::MetricSpinButton&, void> aSpinButtonLink = LINK(this, ChartAxisPanel, TextRotationHdl);
    mxNFRotation->connect_value_changed(aSpinButtonLink);

    mxLBLabelPos->connect_changed(LINK(this, ChartAxisPanel, ListBoxHdl));

    Link<weld::Toggleable&, void> aHistogramToggle = LINK(this, ChartAxisPanel, HistogramToggleHdl);
    mxRBHistogramAutomatic->connect_toggled(aHistogramToggle);
    mxRBHistogramBinWidth->connect_toggled(aHistogramToggle);
    mxRBHistogramBinCount->connect_toggled(aHistogramToggle);
    mxCBHistogramOverflow->connect_toggled(aHistogramToggle);
    mxCBHistogramUnderflow->connect_toggled(aHistogramToggle);

    Link<weld::FormattedSpinButton&, void> aHistogramValue = LINK(this, ChartAxisPanel, HistogramValueHdl);
    mxHistogramBinWidth->connect_value_changed(aHistogramValue);
    mxHistogramOverflow->connect_value_changed(aHistogramValue);
    mxHistogramUnderflow->connect_value_changed(aHistogramValue);
    mxHistogramBinCount->connect_value_changed(LINK(this, ChartAxisPanel, HistogramBinCountHdl));
}

void ChartAxisPanel::updateData()
{
    if (!mbModelValid)
        return;

    OUString aCID = getCID(mxModel);
    ObjectType eType = ObjectIdentifier::getObjectType(aCID);
    if (eType!=OBJECTTYPE_AXIS)
        return;

    SolarMutexGuard aGuard;

    mbUpdating = true;
    const comphelper::ScopeGuard aUpdateGuard([this]() { mbUpdating = false; });

    mxCBShowLabel->set_active(isLabelShown(mxModel, aCID));
    mxCBReverse->set_active(isReverse(mxModel, aCID));

    mxLBLabelPos->set_active(getLabelPosition(mxModel, aCID));
    mxNFRotation->set_value(getAxisRotation(mxModel, aCID), FieldUnit::DEGREE);

    rtl::Reference<::chart::ChartType> xHistogramType = getHistogramChartType(mxModel, aCID);
    mxBxHistogramBinning->set_visible(xHistogramType.is());

    if (!xHistogramType.is())
    {
        return;
    }

    sal_Int32 nFrequencyType = 0;
    double fBinWidth = 0.0;
    sal_Int32 nBinCount = 1;
    bool bUseOverflow = false;
    double fOverflow = 0.0;
    bool bUseUnderflow = false;
    double fUnderflow = 0.0;

    xHistogramType->getPropertyValue(u"FrequencyType"_ustr) >>= nFrequencyType;
    xHistogramType->getPropertyValue(u"BinWidth"_ustr) >>= fBinWidth;
    xHistogramType->getPropertyValue(u"BinCount"_ustr) >>= nBinCount;
    xHistogramType->getPropertyValue(u"UseOverflowBin"_ustr) >>= bUseOverflow;
    xHistogramType->getPropertyValue(u"OverflowBinValue"_ustr) >>= fOverflow;
    xHistogramType->getPropertyValue(u"UseUnderflowBin"_ustr) >>= bUseUnderflow;
    xHistogramType->getPropertyValue(u"UnderflowBinValue"_ustr) >>= fUnderflow;

    mxRBHistogramAutomatic->set_active(nFrequencyType != 1 && nFrequencyType != 2);
    mxRBHistogramBinWidth->set_active(nFrequencyType == 1);
    mxRBHistogramBinCount->set_active(nFrequencyType == 2);
    mxHistogramBinWidth->GetFormatter().SetValue(fBinWidth);
    mxHistogramBinCount->set_value(nBinCount);
    mxCBHistogramOverflow->set_active(bUseOverflow);
    mxHistogramOverflow->GetFormatter().SetValue(fOverflow);
    mxCBHistogramUnderflow->set_active(bUseUnderflow);
    mxHistogramUnderflow->GetFormatter().SetValue(fUnderflow);

    updateHistogramControlSensitivity();
}

std::unique_ptr<PanelLayout> ChartAxisPanel::Create (
    weld::Widget* pParent,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to ChartAxisPanel::Create"_ustr, nullptr, 0);
    return std::make_unique<ChartAxisPanel>(pParent, pController);
}

void ChartAxisPanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
    updateData();
}

void ChartAxisPanel::HandleContextChange(
    const vcl::EnumContext& )
{
    updateData();
}

void ChartAxisPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/ )
{
}

void ChartAxisPanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartAxisPanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxModifyListener);

        css::uno::Reference<css::view::XSelectionSupplier> oldSelectionSupplier(
            mxModel->getCurrentController(), css::uno::UNO_QUERY);
        if (oldSelectionSupplier.is()) {
            oldSelectionSupplier->removeSelectionChangeListener(mxSelectionListener);
        }
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxModel->addModifyListener(mxModifyListener);

    css::uno::Reference<css::view::XSelectionSupplier> xSelectionSupplier(mxModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier->addSelectionChangeListener(mxSelectionListener);
}

void ChartAxisPanel::updateHistogramControlSensitivity()
{
    mxHistogramBinWidth->set_sensitive(mxRBHistogramBinWidth->get_active());
    mxHistogramBinCount->set_sensitive(mxRBHistogramBinCount->get_active());
    mxHistogramOverflow->set_sensitive(mxCBHistogramOverflow->get_active());
    mxHistogramUnderflow->set_sensitive(mxCBHistogramUnderflow->get_active());
}

void ChartAxisPanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

void ChartAxisPanel::selectionChanged(bool bCorrectType)
{
    if (bCorrectType)
        updateData();
}

IMPL_LINK(ChartAxisPanel, CheckBoxHdl, weld::Toggleable&, rCheckbox, void)
{
    OUString aCID = getCID(mxModel);
    bool bChecked = rCheckbox.get_active();

    if (&rCheckbox == mxCBShowLabel.get())
    {
        mxGridLabel->set_sensitive(bChecked);
        setLabelShown(mxModel, aCID, bChecked);
    }
    else if (&rCheckbox == mxCBReverse.get())
        setReverse(mxModel, aCID, bChecked);
}

IMPL_LINK_NOARG(ChartAxisPanel, ListBoxHdl, weld::ComboBox&, void)
{
    OUString aCID = getCID(mxModel);
    sal_Int32 nPos = mxLBLabelPos->get_active();

    setLabelPosition(mxModel, aCID, nPos);
}

IMPL_LINK(ChartAxisPanel, TextRotationHdl, weld::MetricSpinButton&, rMetricField, void)
{
    OUString aCID = getCID(mxModel);
    double nVal = rMetricField.get_value(FieldUnit::DEGREE);
    setAxisRotation(mxModel, aCID, nVal);
}

IMPL_LINK(ChartAxisPanel, HistogramToggleHdl, weld::Toggleable&, rToggle, void)
{
    if (mbUpdating)
    {
        return;
    }

    rtl::Reference<::chart::ChartType> xHistogramType = getHistogramChartType(mxModel, getCID(mxModel));
    if (!xHistogramType.is())
    {
        return;
    }

    updateHistogramControlSensitivity();

    if (&rToggle == mxRBHistogramAutomatic.get() && rToggle.get_active())
    {
        xHistogramType->setPropertyValue(u"FrequencyType"_ustr, cpo::uno::Any(sal_Int32(0)));
    }
    else if (&rToggle == mxRBHistogramBinWidth.get() && rToggle.get_active())
    {
        xHistogramType->setPropertyValue(u"FrequencyType"_ustr, cpo::uno::Any(sal_Int32(1)));
    }
    else if (&rToggle == mxRBHistogramBinCount.get() && rToggle.get_active())
    {
        xHistogramType->setPropertyValue(u"FrequencyType"_ustr, cpo::uno::Any(sal_Int32(2)));
    }
    else if (&rToggle == mxCBHistogramOverflow.get())
    {
        xHistogramType->setPropertyValue(u"UseOverflowBin"_ustr, cpo::uno::Any(rToggle.get_active()));
    }
    else if (&rToggle == mxCBHistogramUnderflow.get())
    {
        xHistogramType->setPropertyValue(u"UseUnderflowBin"_ustr, cpo::uno::Any(rToggle.get_active()));
    }
}

IMPL_LINK(ChartAxisPanel, HistogramValueHdl, weld::FormattedSpinButton&, rField, void)
{
    if (mbUpdating)
    {
        return;
    }

    rtl::Reference<::chart::ChartType> xHistogramType = getHistogramChartType(mxModel, getCID(mxModel));
    if (!xHistogramType.is())
    {
        return;
    }

    const double fValue = rField.GetFormatter().GetValue();

    // While both special bins are on, the underflow boundary has to stay below the overflow
    // one. A pair that crosses is dropped when the bins are computed, so the edit is refused
    // here and the field goes back to the value the model holds.
    const bool bBothSpecialBins
        = mxCBHistogramUnderflow->get_active() && mxCBHistogramOverflow->get_active();

    if (&rField == mxHistogramBinWidth.get())
    {
        if (fValue <= 0.0)
        {
            updateData();
            return;
        }
        xHistogramType->setPropertyValue(u"BinWidth"_ustr, cpo::uno::Any(fValue));
    }
    else if (&rField == mxHistogramOverflow.get())
    {
        if (bBothSpecialBins && mxHistogramUnderflow->GetFormatter().GetValue() >= fValue)
        {
            updateData();
            return;
        }
        xHistogramType->setPropertyValue(u"OverflowBinValue"_ustr, cpo::uno::Any(fValue));
    }
    else if (&rField == mxHistogramUnderflow.get())
    {
        if (bBothSpecialBins && fValue >= mxHistogramOverflow->GetFormatter().GetValue())
        {
            updateData();
            return;
        }
        xHistogramType->setPropertyValue(u"UnderflowBinValue"_ustr, cpo::uno::Any(fValue));
    }
}

IMPL_LINK(ChartAxisPanel, HistogramBinCountHdl, weld::SpinButton&, rField, void)
{
    if (mbUpdating || rField.get_value() <= 0)
    {
        return;
    }

    rtl::Reference<::chart::ChartType> xHistogramType = getHistogramChartType(mxModel, getCID(mxModel));
    if (xHistogramType.is())
    {
        xHistogramType->setPropertyValue(u"BinCount"_ustr, cpo::uno::Any(sal_Int32(rField.get_value())));
    }
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
