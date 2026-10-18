/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "EffectPropertyPanel.hxx"

#include <o3tl/untaint.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sfx2/dispatch.hxx>
#include <svx/colorbox.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>

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
    , maGlowController(SID_ATTR_GLOW, *pBindings, *this)
    , maSoftEdgeController(SID_ATTR_SOFTEDGE, *pBindings, *this)
    , maReflectionController(SID_ATTR_REFLECTION, *pBindings, *this)
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
    maGlowController.dispose();
    maSoftEdgeController.dispose();
    maReflectionController.dispose();
}

void EffectPropertyPanel::NotifyItemUpdate(const sal_uInt16 nSID, const SfxItemState eState,
                                           const SfxPoolItem* pState)
{
    const bool bDefaultOrSet(SfxItemState::DEFAULT <= eState);
    switch (nSID)
    {
        case SID_ATTR_REFLECTION:
        {
            mbReflectionAvailable = eState != SfxItemState::DISABLED;
            mxReflection->set_sensitive(mbReflectionAvailable);
            const auto* pItem
                = bDefaultOrSet ? dynamic_cast<const SdrReflectionEffectItem*>(pState) : nullptr;
            if (!pItem)
            {
                mpReflection.reset();
                mxReflection->set_state(TRISTATE_INDET);
                break;
            }
            mpReflection.reset(pItem->Clone());
            mxReflection->set_active(pItem->isEnabled());
            mxReflectionTransparency->set_value(pItem->getStartTransparency(), FieldUnit::PERCENT);
            mxReflectionTransparencySlider->set_value(pItem->getStartTransparency());
            mxReflectionSize->set_value(pItem->getEndPosition(), FieldUnit::PERCENT);
            mxReflectionSizeSlider->set_value(pItem->getEndPosition());
            mxReflectionDistance->set_value(pItem->getEffect().maDistance.as_hmm<sal_Int64>(),
                                            FieldUnit::MM_100TH);
            mxReflectionBlur->set_value(pItem->getEffect().maBlurRadius.as_hmm<sal_Int64>(),
                                        FieldUnit::MM_100TH);
            break;
        }
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

SdrReflectionEffectItem EffectPropertyPanel::getReflection() const
{
    if (mpReflection)
        return *mpReflection;
    return SdrReflectionEffectItem(SDRATTR_REFLECTION);
}

void EffectPropertyPanel::setReflection(const SdrReflectionEffectItem& rReflection)
{
    // The next change starts from this one, also before the state of the selection comes back.
    mpReflection.reset(rReflection.Clone());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_REFLECTION, SfxCallMode::SLOT,
                                             { &rReflection });
}

IMPL_LINK_NOARG(EffectPropertyPanel, ToggleReflectionHdl, weld::Toggleable&, void)
{
    SdrReflectionEffectItem aReflection(getReflection());
    aReflection.setEnabled(mxReflection->get_active());
    setReflection(aReflection);
    UpdateReflectionControls();
}

// The spin field and the slider show the same value, so a change in one is shown in the other.
void EffectPropertyPanel::SetReflectionTransparency(sal_uInt16 nTransparency)
{
    mxReflectionTransparency->set_value(nTransparency, FieldUnit::PERCENT);
    mxReflectionTransparencySlider->set_value(nTransparency);
    SdrReflectionEffectItem aReflection(getReflection());
    aReflection.setStartTransparency(nTransparency);
    setReflection(aReflection);
}

void EffectPropertyPanel::SetReflectionSize(sal_uInt16 nSize)
{
    mxReflectionSize->set_value(nSize, FieldUnit::PERCENT);
    mxReflectionSizeSlider->set_value(nSize);
    SdrReflectionEffectItem aReflection(getReflection());
    aReflection.setEndPosition(nSize);
    setReflection(aReflection);
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
    SdrReflectionEffectItem aReflection(getReflection());
    aReflection.setDistance(gfx::Length::hmm(mxReflectionDistance->get_value(FieldUnit::MM_100TH)));
    setReflection(aReflection);
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyReflectionBlurHdl, weld::MetricSpinButton&, void)
{
    SdrReflectionEffectItem aReflection(getReflection());
    aReflection.setBlurRadius(gfx::Length::hmm(mxReflectionBlur->get_value(FieldUnit::MM_100TH)));
    setReflection(aReflection);
}

void EffectPropertyPanel::setGlow(const SdrGlowEffectItem& rGlow)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW, SfxCallMode::SLOT, { &rGlow });
}

void EffectPropertyPanel::setSoftEdge(const SdrSoftEdgeEffectItem& rSoftEdge)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_SOFTEDGE, SfxCallMode::SLOT, { &rSoftEdge });
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
