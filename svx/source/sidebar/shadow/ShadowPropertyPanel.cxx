/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ShadowPropertyPanel.hxx"
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svx/colorbox.hxx>
#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdshtitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/sdshcitm.hxx>
#include <comphelper/lok.hxx>

using namespace css;
using namespace css::uno;

namespace svx::sidebar {

ShadowPropertyPanel::ShadowPropertyPanel(
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "ShadowPropertyPanel", "svx/ui/sidebarshadow.ui", rxFrame),
    maShadowController(SID_ATTR_FILL_SHADOW, *pBindings, *this),
    maShadowTransController(SID_ATTR_SHADOW_TRANSPARENCE, *pBindings, *this),
    maShadowBlurController(SID_ATTR_SHADOW_BLUR, *pBindings, *this),
    maShadowColorController(SID_ATTR_SHADOW_COLOR, *pBindings, *this),
    maShadowXDistanceController(SID_ATTR_SHADOW_XDISTANCE, *pBindings, *this),
    maShadowYDistanceController(SID_ATTR_SHADOW_YDISTANCE, *pBindings, *this),
    mpBindings(pBindings),
    nX(0),
    nY(0),
    nXY(0),
    mxShowShadow(m_xBuilder->weld_check_button("SHOW_SHADOW")),
    mxShadowDistance(m_xBuilder->weld_metric_spin_button("LB_DISTANCE", FieldUnit::POINT)),
    mxLBShadowColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_SHADOW_COLOR"), [this]{ return GetFrameWeld(); })),
    mxShadowAngle(m_xBuilder->weld_combo_box("LB_ANGLE")),
    mxFTAngle(m_xBuilder->weld_label("angle")),
    mxFTDistance(m_xBuilder->weld_label("distance")),
    mxFTTransparency(m_xBuilder->weld_label("transparency_label")),
    mxFTBlur(m_xBuilder->weld_label("blur_label")),
    mxFTColor(m_xBuilder->weld_label("color")),
    mxShadowTransSlider(m_xBuilder->weld_scale("transparency_slider")),
    mxShadowTransMetric(m_xBuilder->weld_metric_spin_button("FIELD_TRANSPARENCY", FieldUnit::PERCENT)),
    mxShadowBlurMetric(m_xBuilder->weld_metric_spin_button("LB_SHADOW_BLUR", FieldUnit::POINT))
{
    Initialize();
}

ShadowPropertyPanel::~ShadowPropertyPanel()
{
    disposeOnce();
}

void ShadowPropertyPanel::dispose()
{
    mxShowShadow.reset();
    mxFTAngle.reset();
    mxShadowAngle.reset();
    mxFTDistance.reset();
    mxShadowDistance.reset();
    mxFTTransparency.reset();
    mxShadowTransSlider.reset();
    mxShadowTransMetric.reset();
    mxShadowBlurMetric.reset();
    mxFTBlur.reset();
    mxFTColor.reset();
    mxLBShadowColor.reset();

    maShadowController.dispose();
    maShadowTransController.dispose();
    maShadowBlurController.dispose();
    maShadowColorController.dispose();
    maShadowXDistanceController.dispose();
    maShadowYDistanceController.dispose();
    PanelLayout::dispose();
}

void ShadowPropertyPanel::Initialize()
{
    mxShowShadow->set_state( TRISTATE_FALSE );
    mxShowShadow->connect_toggled( LINK(this, ShadowPropertyPanel, ClickShadowHdl ) );
    mxShadowTransMetric->connect_value_changed( LINK(this, ShadowPropertyPanel, ModifyShadowTransMetricHdl) );
    mxLBShadowColor->SetSelectHdl( LINK( this, ShadowPropertyPanel, ModifyShadowColorHdl ) );
    mxShadowAngle->connect_changed( LINK(this, ShadowPropertyPanel, ModifyShadowAngleHdl) );
    mxShadowDistance->connect_value_changed( LINK(this, ShadowPropertyPanel, ModifyShadowDistanceHdl) );
    mxShadowTransSlider->set_range(0, 100);
    mxShadowTransSlider->connect_value_changed( LINK(this, ShadowPropertyPanel, ModifyShadowTransSliderHdl) );
    mxShadowBlurMetric->set_range(0, 150, FieldUnit::POINT);
    mxShadowBlurMetric->connect_value_changed(LINK(this, ShadowPropertyPanel, ModifyShadowBlurMetricHdl));
    InsertAngleValues();
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ClickShadowHdl, weld::ToggleButton&, void)
{
    if( mxShowShadow->get_state() == TRISTATE_FALSE )
    {
        SdrOnOffItem aItem(makeSdrShadowItem(false));
        GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_SHADOW,
                SfxCallMode::RECORD, { &aItem });

        if (comphelper::LibreOfficeKit::isActive())
        {
            mxShowShadow->set_state( TRISTATE_FALSE );
            UpdateControls();
        }
    }
    else
    {
        SdrOnOffItem aItem(makeSdrShadowItem(true));
        GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_FILL_SHADOW,
                SfxCallMode::RECORD, { &aItem });

        if (mxShadowDistance->get_value(FieldUnit::POINT) == 0)
            mxShadowDistance->set_value( 8, FieldUnit::POINT );

        if (comphelper::LibreOfficeKit::isActive())
        {
            mxShowShadow->set_state( TRISTATE_TRUE );
            UpdateControls();
        }
    }
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ModifyShadowColorHdl, ColorListBox&, void)
{
    XColorItem aItem(makeSdrShadowColorItem(mxLBShadowColor->GetSelectEntryColor()));
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_COLOR,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ModifyShadowTransMetricHdl, weld::MetricSpinButton&, void)
{
    sal_uInt16 nVal = mxShadowTransMetric->get_value(FieldUnit::PERCENT);
    SetTransparencyValue(nVal);
    SdrPercentItem aItem( makeSdrShadowTransparenceItem(nVal) );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_TRANSPARENCE,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ModifyShadowBlurMetricHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_SHADOWBLUR, mxShadowBlurMetric->get_value(FieldUnit::MM_100TH));

    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_BLUR, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ModifyShadowTransSliderHdl, weld::Scale&, void)
{
    sal_uInt16 nVal = mxShadowTransSlider->get_value();
    SetTransparencyValue(nVal);
    SdrPercentItem aItem( makeSdrShadowTransparenceItem(nVal) );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_TRANSPARENCE,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ModifyShadowAngleHdl, weld::ComboBox&, void)
{
    ModifyShadowDistance();
}

IMPL_LINK_NOARG(ShadowPropertyPanel, ModifyShadowDistanceHdl, weld::MetricSpinButton&, void)
{
    ModifyShadowDistance();
}

void ShadowPropertyPanel::ModifyShadowDistance()
{
    auto nAngle = mxShadowAngle->get_active_id().toInt32();
    nXY = mxShadowDistance->get_value(FieldUnit::MM_100TH);
    switch (nAngle)
    {
        case 0: nX = nXY; nY = 0;             break;
        case 45: nX = nXY; nY = -nXY;         break;
        case 90: nX = 0; nY = - nXY;          break;
        case 135: nX = nY = -nXY;             break;
        case 180: nX = -nXY; nY = 0;          break;
        case 225: nX = -nXY; nY = nXY;        break;
        case 270: nX = 0; nY = nXY;           break;
        case 315: nX = nY = nXY;              break;
    }
    SdrMetricItem aXItem(makeSdrShadowXDistItem(nX));
    SdrMetricItem aYItem(makeSdrShadowYDistItem(nY));
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_XDISTANCE,
            SfxCallMode::RECORD, { &aXItem });
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_SHADOW_YDISTANCE,
            SfxCallMode::RECORD, { &aYItem });
}

void ShadowPropertyPanel::UpdateControls()
{
    if (mxShowShadow->get_state() == TRISTATE_FALSE)
    {
        mxShadowDistance->set_sensitive(false);
        mxLBShadowColor->set_sensitive(false);
        mxShadowAngle->set_sensitive(false);
        mxFTAngle->set_sensitive(false);
        mxFTDistance->set_sensitive(false);
        mxFTTransparency->set_sensitive(false);
        mxFTBlur->set_sensitive(false);
        mxFTColor->set_sensitive(false);
        mxShadowTransSlider->set_sensitive(false);
        mxShadowTransMetric->set_sensitive(false);
        mxShadowBlurMetric->set_sensitive(false);

        return;
    }
    else
    {
        mxShadowDistance->set_sensitive(true);
        mxLBShadowColor->set_sensitive(true);
        mxShadowAngle->set_sensitive(true);
        mxFTAngle->set_sensitive(true);
        mxFTDistance->set_sensitive(true);
        mxFTTransparency->set_sensitive(true);
        mxFTBlur->set_sensitive(true);
        mxFTColor->set_sensitive(true);
        mxShadowTransSlider->set_sensitive(true);
        mxShadowTransMetric->set_sensitive(true);
        mxShadowBlurMetric->set_sensitive(true);
    }

    if(nX > 0 && nY == 0) { mxShadowAngle->set_active(0); nXY = nX; }
    else if( nX > 0 && nY < 0 ) { mxShadowAngle->set_active(1); nXY = nX; }
    else if( nX == 0 && nY < 0 ) { mxShadowAngle->set_active(2); nXY = -nY; }
    else if( nX < 0 && nY < 0 ) { mxShadowAngle->set_active(3); nXY = -nY; }
    else if( nX < 0 && nY == 0 ) { mxShadowAngle->set_active(4); nXY = -nX; }
    else if( nX < 0 && nY > 0 ) { mxShadowAngle->set_active(5); nXY = nY; }
    else if( nX == 0 && nY > 0 ) { mxShadowAngle->set_active(6); nXY = nY; }
    else if( nX > 0 && nY > 0 ) { mxShadowAngle->set_active(7); nXY = nX; }
    else { nXY = 0; }
    mxShadowDistance->set_value(nXY, FieldUnit::MM_100TH);
}

void ShadowPropertyPanel::SetTransparencyValue(tools::Long nVal)
{
    mxShadowTransSlider->set_value(nVal);
    mxShadowTransMetric->set_value(nVal, FieldUnit::PERCENT);
}

void ShadowPropertyPanel::DataChanged(const DataChangedEvent& /*rEvent*/)
{
}

void ShadowPropertyPanel::InsertAngleValues()
{
    OUString sSuffix = weld::MetricSpinButton::MetricToString(FieldUnit::DEGREE);

    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();

    mxShadowAngle->append(OUString::number(0), rLocaleData.getNum(0, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(45), rLocaleData.getNum(45, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(90), rLocaleData.getNum(90, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(135), rLocaleData.getNum(135, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(180), rLocaleData.getNum(180, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(225), rLocaleData.getNum(225, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(270), rLocaleData.getNum(270, 0, true, true) + sSuffix);
    mxShadowAngle->append(OUString::number(315), rLocaleData.getNum(315, 0, true, true) + sSuffix);
}

void ShadowPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    switch(nSID)
    {
        case SID_ATTR_FILL_SHADOW:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrOnOffItem* pItem = dynamic_cast< const SdrOnOffItem* >(pState);
                if(pItem)
                {
                    if (pItem->GetValue())
                        mxShowShadow->set_state(TRISTATE_TRUE);
                    else
                        mxShowShadow->set_state(TRISTATE_FALSE);
                }
                else
                    mxShowShadow.reset();
            }
        }
        break;

        case SID_ATTR_SHADOW_TRANSPARENCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrPercentItem* pTransparencyItem = dynamic_cast< const SdrPercentItem* >(pState);
                if(pTransparencyItem)
                {
                    const sal_uInt16 nVal = pTransparencyItem->GetValue();
                    SetTransparencyValue(nVal);
                }
                else
                    SetTransparencyValue(0);
            }
        }
        break;
        case SID_ATTR_SHADOW_BLUR:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pRadiusItem = dynamic_cast<const SdrMetricItem*>(pState);
                if (pRadiusItem)
                {
                    mxShadowBlurMetric->set_value(pRadiusItem->GetValue(), FieldUnit::MM_100TH);
                }
            }
        }
        break;
        case SID_ATTR_SHADOW_COLOR:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const XColorItem* pColorItem = dynamic_cast< const XColorItem* >(pState);
                if(pColorItem)
                {
                   mxLBShadowColor->SelectEntry(pColorItem->GetColorValue());
                }
            }
        }
        break;
        case SID_ATTR_SHADOW_XDISTANCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pXDistItem = dynamic_cast< const SdrMetricItem* >(pState);
                if(pXDistItem)
                {
                    nX = pXDistItem->GetValue();
                }
            }
        }
        break;
        case SID_ATTR_SHADOW_YDISTANCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pYDistItem = dynamic_cast< const SdrMetricItem* >(pState);
                if(pYDistItem)
                {
                    nY = pYDistItem->GetValue();
                }
            }
        }
        break;
    }
    UpdateControls();
}

VclPtr<PanelLayout> ShadowPropertyPanel::Create (
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if(pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ShadowPropertyPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw lang::IllegalArgumentException("no XFrame given to ShadowPropertyPanel::Create", nullptr, 1);
    if(pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to ShadowPropertyPanel::Create", nullptr, 2);

    return VclPtr<ShadowPropertyPanel>::Create(pParent, rxFrame, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
