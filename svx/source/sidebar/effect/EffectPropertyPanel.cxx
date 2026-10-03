/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "EffectPropertyPanel.hxx"

#include <o3tl/untaint.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>
#include <svl/itemset.hxx>
#include <tools/fldunit.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/CheckButton.hxx>
#include <vcl/weld/Frame.hxx>
#include <vcl/weld/Scale.hxx>

namespace svx::sidebar
{
EffectPropertyPanel::EffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : EffectPropertyPanelBase(pParent)
    , mpBindings(pBindings)
    , mxReflectionFrame(m_xBuilder->weld_frame(u"reflectionframe"_ustr))
    , mxReflection(m_xBuilder->weld_check_button(u"CB_REFLECTION"_ustr))
    , mxReflectionTransparencyLabel(m_xBuilder->weld_label(u"reflectiontransparency"_ustr))
    , mxReflectionTransparency(m_xBuilder->weld_metric_spin_button(
          u"SB_REFLECTION_TRANSPARENCY"_ustr, FieldUnit::PERCENT))
    , mxReflectionTransparencySlider(m_xBuilder->weld_scale(u"SL_REFLECTION_TRANSPARENCY"_ustr))
    , mxReflectionSizeLabel(m_xBuilder->weld_label(u"reflectionsize"_ustr))
    , mxReflectionSize(
          m_xBuilder->weld_metric_spin_button(u"SB_REFLECTION_SIZE"_ustr, FieldUnit::PERCENT))
    , mxReflectionSizeSlider(m_xBuilder->weld_scale(u"SL_REFLECTION_SIZE"_ustr))
    , mxReflectionDistanceLabel(m_xBuilder->weld_label(u"reflectiondistance"_ustr))
    , mxReflectionDistance(
          m_xBuilder->weld_metric_spin_button(u"SB_REFLECTION_DISTANCE"_ustr, FieldUnit::POINT))
    , mxReflectionBlurLabel(m_xBuilder->weld_label(u"reflectionblur"_ustr))
    , mxReflectionBlur(
          m_xBuilder->weld_metric_spin_button(u"SB_REFLECTION_BLUR"_ustr, FieldUnit::POINT))
    , maGlowColorController(SID_ATTR_GLOW_COLOR, *pBindings, *this)
    , maGlowRadiusController(SID_ATTR_GLOW_RADIUS, *pBindings, *this)
    , maGlowTransparencyController(SID_ATTR_GLOW_TRANSPARENCY, *pBindings, *this)
    , maSoftEdgeRadiusController(SID_ATTR_SOFTEDGE_RADIUS, *pBindings, *this)
    , maReflectionController(SID_ATTR_REFLECTION, *pBindings, *this)
    , maReflectionTransparencyController(SID_ATTR_REFLECTION_START_TRANSPARENCY, *pBindings, *this)
    , maReflectionSizeController(SID_ATTR_REFLECTION_END_POSITION, *pBindings, *this)
    , maReflectionDistanceController(SID_ATTR_REFLECTION_DISTANCE, *pBindings, *this)
    , maReflectionBlurController(SID_ATTR_REFLECTION_BLUR_RADIUS, *pBindings, *this)
{
    // The reflection section is in the shared layout but shows only for shapes.
    mxReflectionFrame->show();

    mxReflection->connect_toggled(LINK(this, EffectPropertyPanel, ToggleReflectionHdl));
    mxReflectionTransparency->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyReflectionTransparencyHdl));
    mxReflectionSize->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyReflectionSizeHdl));
    mxReflectionTransparencySlider->set_range(0, 100);
    mxReflectionTransparencySlider->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyReflectionTransparencySliderHdl));
    mxReflectionSizeSlider->set_range(0, 100);
    mxReflectionSizeSlider->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyReflectionSizeSliderHdl));
    mxReflectionDistance->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyReflectionDistanceHdl));
    mxReflectionBlur->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyReflectionBlurHdl));
    UpdateReflectionControls();
}

EffectPropertyPanel::~EffectPropertyPanel()
{
    maGlowColorController.dispose();
    maGlowRadiusController.dispose();
    maGlowTransparencyController.dispose();
    maSoftEdgeRadiusController.dispose();
    maReflectionController.dispose();
    maReflectionTransparencyController.dispose();
    maReflectionSizeController.dispose();
    maReflectionDistanceController.dispose();
    maReflectionBlurController.dispose();
}

void EffectPropertyPanel::NotifyItemUpdate(const sal_uInt16 nSID, const SfxItemState eState,
                                           const SfxPoolItem* pState)
{
    const bool bDefaultOrSet(SfxItemState::DEFAULT <= eState);
    switch (nSID)
    {
        case SID_ATTR_REFLECTION:
            mbReflectionAvailable = eState != SfxItemState::DISABLED;
            mxReflection->set_sensitive(mbReflectionAvailable);
            if (const auto* pItem = dynamic_cast<const SfxBoolItem*>(pState);
                bDefaultOrSet && pItem)
                mxReflection->set_active(pItem->GetValue());
            else
                mxReflection->set_state(TRISTATE_INDET);
            break;
        case SID_ATTR_REFLECTION_START_TRANSPARENCY:
            if (const auto* pItem = dynamic_cast<const SdrPercentItem*>(pState);
                bDefaultOrSet && pItem)
            {
                mxReflectionTransparency->set_value(pItem->GetValue(), FieldUnit::PERCENT);
                mxReflectionTransparencySlider->set_value(pItem->GetValue());
            }
            break;
        case SID_ATTR_REFLECTION_END_POSITION:
            if (const auto* pItem = dynamic_cast<const SdrPercentItem*>(pState);
                bDefaultOrSet && pItem)
            {
                mxReflectionSize->set_value(pItem->GetValue(), FieldUnit::PERCENT);
                mxReflectionSizeSlider->set_value(pItem->GetValue());
            }
            break;
        case SID_ATTR_REFLECTION_DISTANCE:
            if (const auto* pItem = dynamic_cast<const SdrMetricItem*>(pState);
                bDefaultOrSet && pItem)
                mxReflectionDistance->set_value(pItem->GetValue(), FieldUnit::MM_100TH);
            break;
        case SID_ATTR_REFLECTION_BLUR_RADIUS:
            if (const auto* pItem = dynamic_cast<const SdrMetricItem*>(pState);
                bDefaultOrSet && pItem)
                mxReflectionBlur->set_value(pItem->GetValue(), FieldUnit::MM_100TH);
            break;
        default:
            EffectPropertyPanelBase::NotifyItemUpdate(nSID, eState, pState);
            return;
    }
    UpdateReflectionControls();
}

void EffectPropertyPanel::UpdateReflectionControls()
{
    // The values apply only while the reflection is shown. Each widget is set on its own, so a
    // jsdialog client receives the state of every one of them.
    const bool bEnabled = mbReflectionAvailable && mxReflection->get_state() == TRISTATE_TRUE;
    mxReflectionTransparencyLabel->set_sensitive(bEnabled);
    mxReflectionTransparency->set_sensitive(bEnabled);
    mxReflectionTransparencySlider->set_sensitive(bEnabled);
    mxReflectionSizeLabel->set_sensitive(bEnabled);
    mxReflectionSize->set_sensitive(bEnabled);
    mxReflectionSizeSlider->set_sensitive(bEnabled);
    mxReflectionDistanceLabel->set_sensitive(bEnabled);
    mxReflectionDistance->set_sensitive(bEnabled);
    mxReflectionBlurLabel->set_sensitive(bEnabled);
    mxReflectionBlur->set_sensitive(bEnabled);
}

IMPL_LINK_NOARG(EffectPropertyPanel, ToggleReflectionHdl, weld::Toggleable&, void)
{
    const SdrOnOffItem aItem(SDRATTR_REFLECTION, mxReflection->get_active());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_REFLECTION, SfxCallMode::RECORD, { &aItem });
    UpdateReflectionControls();
}

// The spin field and the slider show the same value, so a change in one is shown in the other.
void EffectPropertyPanel::SetReflectionTransparency(sal_uInt16 nTransparency)
{
    mxReflectionTransparency->set_value(nTransparency, FieldUnit::PERCENT);
    mxReflectionTransparencySlider->set_value(nTransparency);
    const SdrPercentItem aItem(SDRATTR_REFLECTION_START_TRANSPARENCY, nTransparency);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_REFLECTION_START_TRANSPARENCY,
                                             SfxCallMode::RECORD, { &aItem });
}

void EffectPropertyPanel::SetReflectionSize(sal_uInt16 nSize)
{
    mxReflectionSize->set_value(nSize, FieldUnit::PERCENT);
    mxReflectionSizeSlider->set_value(nSize);
    const SdrPercentItem aItem(SDRATTR_REFLECTION_END_POSITION, nSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_REFLECTION_END_POSITION, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionTransparencyHdl, weld::MetricSpinButton&, void)
{
    SetReflectionTransparency(
        o3tl::sanitizing_cast<sal_uInt16>(mxReflectionTransparency->get_value(FieldUnit::PERCENT)));
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionTransparencySliderHdl, weld::Scale&, void)
{
    SetReflectionTransparency(
        o3tl::sanitizing_cast<sal_uInt16>(mxReflectionTransparencySlider->get_value()));
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionSizeHdl, weld::MetricSpinButton&, void)
{
    SetReflectionSize(
        o3tl::sanitizing_cast<sal_uInt16>(mxReflectionSize->get_value(FieldUnit::PERCENT)));
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionSizeSliderHdl, weld::Scale&, void)
{
    SetReflectionSize(o3tl::sanitizing_cast<sal_uInt16>(mxReflectionSizeSlider->get_value()));
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionDistanceHdl, weld::MetricSpinButton&, void)
{
    const SdrMetricItem aItem(SDRATTR_REFLECTION_DISTANCE,
                              mxReflectionDistance->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_REFLECTION_DISTANCE, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionBlurHdl, weld::MetricSpinButton&, void)
{
    const SdrMetricItem aItem(SDRATTR_REFLECTION_BLUR_RADIUS,
                              mxReflectionBlur->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_REFLECTION_BLUR_RADIUS, SfxCallMode::RECORD,
                                             { &aItem });
}

void EffectPropertyPanel::setGlowRadius(const SdrMetricItem& rGlowRadius)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_RADIUS, SfxCallMode::RECORD,
                                             { &rGlowRadius });
}

void EffectPropertyPanel::setGlowColor(const XColorItem& rGlowColor)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_COLOR, SfxCallMode::RECORD,
                                             { &rGlowColor });
}

void EffectPropertyPanel::setGlowTransparency(const SdrPercentItem& rGlowTransparency)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TRANSPARENCY, SfxCallMode::RECORD,
                                             { &rGlowTransparency });
}

void EffectPropertyPanel::setSoftEdgeRadius(const SdrMetricItem& rSoftEdgeRadius)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_SOFTEDGE_RADIUS, SfxCallMode::RECORD,
                                             { &rSoftEdgeRadius });
}

std::unique_ptr<PanelLayout> EffectPropertyPanel::Create(weld::Widget* pParent,
                                                         SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to EffectPropertyPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to EffectPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<EffectPropertyPanel>(pParent, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
