/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <svx/sidebar/AreaTransparencyGradientPopup.hxx>
#include <svx/sidebar/AreaPropertyPanelBase.hxx>
#include <svx/xflftrit.hxx>

namespace svx::sidebar {

AreaTransparencyGradientPopup::AreaTransparencyGradientPopup(const css::uno::Reference<css::frame::XFrame>& rFrame,
                                            AreaPropertyPanelBase& rPanel, weld::Widget* pParent)
    : WeldToolbarPopup(rFrame, pParent, u"svx/ui/floatingareastyle.ui"_ustr, u"FloatingAreaStyle"_ustr)
    , mrAreaPropertyPanel(rPanel)
    , mxCenterGrid(m_xBuilder->weld_widget(u"centergrid"_ustr))
    , mxAngleGrid(m_xBuilder->weld_widget(u"anglegrid"_ustr))
    , mxMtrTrgrCenterX(m_xBuilder->weld_metric_spin_button(u"centerx"_ustr, FieldUnit::PERCENT))
    , mxMtrTrgrCenterY(m_xBuilder->weld_metric_spin_button(u"centery"_ustr, FieldUnit::PERCENT))
    , mxMtrTrgrAngle(m_xBuilder->weld_metric_spin_button(u"angle"_ustr, FieldUnit::DEGREE))
    , mxBtnLeft45(m_xBuilder->weld_toolbar(u"lefttoolbox"_ustr))
    , mxBtnRight45(m_xBuilder->weld_toolbar(u"righttoolbox"_ustr))
    , mxMtrTrgrStartValue(m_xBuilder->weld_metric_spin_button(u"start"_ustr, FieldUnit::PERCENT))
    , mxMtrTrgrEndValue(m_xBuilder->weld_metric_spin_button(u"end"_ustr, FieldUnit::PERCENT))
    , mxMtrTrgrBorder(m_xBuilder->weld_metric_spin_button(u"border"_ustr, FieldUnit::PERCENT))
{
    Link<weld::MetricSpinButton&,void> aLink = LINK(this, AreaTransparencyGradientPopup, ModifiedTrgrHdl_Impl);
    mxMtrTrgrCenterX->connect_value_changed(aLink);
    mxMtrTrgrCenterY->connect_value_changed(aLink);
    mxMtrTrgrAngle->connect_value_changed(aLink);
    mxMtrTrgrBorder->connect_value_changed(aLink);
    mxMtrTrgrStartValue->connect_value_changed(aLink);
    mxMtrTrgrEndValue->connect_value_changed(aLink);
    mxBtnLeft45->connect_clicked(LINK(this, AreaTransparencyGradientPopup, Left_Click45_Impl));
    mxBtnRight45->connect_clicked(LINK(this, AreaTransparencyGradientPopup, Right_Click45_Impl));
}

AreaTransparencyGradientPopup::~AreaTransparencyGradientPopup()
{
}

void AreaTransparencyGradientPopup::InitStatus(XFillFloatTransparenceItem const * pGradientItem)
{
    const basegfx::BGradient& rGradient = pGradientItem->GetGradientValue();

    basegfx::BGradient aGradient;
    Color aStart(rGradient.GetColorStops().front().getStopColor());
    Color aEnd(rGradient.GetColorStops().back().getStopColor());

    if (rGradient.GetXOffset() == AreaPropertyPanelBase::DEFAULT_CENTERX
        && rGradient.GetYOffset() == AreaPropertyPanelBase::DEFAULT_CENTERY
        && static_cast<sal_Int32>(toDegrees(rGradient.GetAngle())) == AreaPropertyPanelBase::DEFAULT_ANGLE
        && static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aStart.GetRed()) + 1) * 100) / 255)
            == AreaPropertyPanelBase::DEFAULT_STARTVALUE
        && static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aEnd.GetRed()) + 1) * 100) / 255)
            == AreaPropertyPanelBase::DEFAULT_ENDVALUE
        && rGradient.GetBorder() == AreaPropertyPanelBase::DEFAULT_BORDER)
    {
        aGradient = mrAreaPropertyPanel.GetGradient(rGradient.GetGradientStyle());
    }
    else
    {
        aGradient = rGradient;
    }
    mxMtrTrgrCenterX->set_value(aGradient.GetXOffset(), FieldUnit::PERCENT);
    mxMtrTrgrCenterY->set_value(aGradient.GetYOffset(), FieldUnit::PERCENT);
    mxMtrTrgrAngle->set_value(toDegrees(aGradient.GetAngle()), FieldUnit::DEGREE);
    aStart = Color(aGradient.GetColorStops().front().getStopColor());
    aEnd = Color(aGradient.GetColorStops().back().getStopColor());
    mxMtrTrgrStartValue->set_value(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aStart.GetRed()) + 1) * 100) / 255), FieldUnit::PERCENT);
    mxMtrTrgrEndValue->set_value(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aEnd.GetRed()) + 1) * 100) / 255), FieldUnit::PERCENT);

    // MCGR: preserve ColorStops if given
    // tdf#155901 We need offset of first and last stop, so include them.
    if (aGradient.GetColorStops().size() >= 2)
        maColorStops = aGradient.GetColorStops();
    else
        maColorStops.clear();

    mxMtrTrgrBorder->set_value(aGradient.GetBorder(), FieldUnit::PERCENT);
}

void AreaTransparencyGradientPopup::Rearrange(XFillFloatTransparenceItem const * pGradientItem)
{
    InitStatus(pGradientItem);
    const basegfx::BGradient& rGradient = pGradientItem->GetGradientValue();
    css::awt::GradientStyle eXGS(rGradient.GetGradientStyle());

    switch(eXGS)
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            mxCenterGrid->hide();
            mxAngleGrid->show();
            break;
        case css::awt::GradientStyle_RADIAL:
            mxCenterGrid->show();
            mxAngleGrid->hide();
            break;
        case css::awt::GradientStyle_ELLIPTICAL:
        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            mxCenterGrid->show();
            mxAngleGrid->show();
            break;
        default:
            break;
    }
}

void AreaTransparencyGradientPopup::ExecuteValueModify()
{
    //Added
    sal_Int16 aMtrValue = static_cast<sal_Int16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE));
    while(aMtrValue<0)
        aMtrValue += 360;
    sal_uInt16 nVal = aMtrValue/360;
    nVal = aMtrValue - nVal*360;
    mxMtrTrgrAngle->set_value(nVal, FieldUnit::DEGREE);
    //End of new code

    basegfx::BColorStops aColorStops;
    basegfx::BColor aStartBColor(mxMtrTrgrStartValue->get_value(FieldUnit::PERCENT) / 100.0);
    aStartBColor.clamp();
    basegfx::BColor aEndBColor(mxMtrTrgrEndValue->get_value(FieldUnit::PERCENT) / 100.0);
    aEndBColor.clamp();

    if (maColorStops.size() >= 2)
    {
        aColorStops = maColorStops;
        aColorStops.front() = basegfx::BColorStop(maColorStops.front().getStopOffset(), aStartBColor);
        aColorStops.back() = basegfx::BColorStop(maColorStops.back().getStopOffset(), aEndBColor);
    }
    else
    {
        aColorStops.emplace_back(0.0, aStartBColor);
        aColorStops.emplace_back(1.0, aEndBColor);
    }

    basegfx::BGradient aTmpGradient(
        aColorStops,
        static_cast<css::awt::GradientStyle>(mrAreaPropertyPanel.GetSelectedTransparencyTypeIndex()-2),
        Degree10(static_cast<sal_Int16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE)) * 10),
        static_cast<sal_uInt16>(mxMtrTrgrCenterX->get_value(FieldUnit::PERCENT)),
        static_cast<sal_uInt16>(mxMtrTrgrCenterY->get_value(FieldUnit::PERCENT)),
        static_cast<sal_uInt16>(mxMtrTrgrBorder->get_value(FieldUnit::PERCENT)),
        100, 100);

    mrAreaPropertyPanel.SetGradient(aTmpGradient);

    XFillFloatTransparenceItem aGradientItem(aTmpGradient, true );

    mrAreaPropertyPanel.setFillFloatTransparence(aGradientItem);
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, ModifiedTrgrHdl_Impl, weld::MetricSpinButton&, void)
{
    ExecuteValueModify();
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, Left_Click45_Impl, const OUString&, void)
{
    sal_uInt16 nTemp = static_cast<sal_uInt16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE));
    if (nTemp>=315)
        nTemp -= 360;
    nTemp += 45;
    mxMtrTrgrAngle->set_value(nTemp, FieldUnit::DEGREE);
    ExecuteValueModify();
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, Right_Click45_Impl, const OUString&, void)
{
    sal_uInt16 nTemp = static_cast<sal_uInt16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE));
    if (nTemp<45)
        nTemp += 360;
    nTemp -= 45;
    mxMtrTrgrAngle->set_value(nTemp, FieldUnit::DEGREE);
    ExecuteValueModify();
}

void AreaTransparencyGradientPopup::GrabFocus()
{
    mxMtrTrgrCenterX->grab_focus();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
