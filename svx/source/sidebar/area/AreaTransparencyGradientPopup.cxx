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
#include <svx/xgrad.hxx>
#include <vcl/svapp.hxx>

namespace svx::sidebar {

AreaTransparencyGradientPopup::AreaTransparencyGradientPopup(AreaPropertyPanelBase& rPanel, weld::Widget* pParent)
    : mrAreaPropertyPanel(rPanel)
    , mxBuilder(Application::CreateBuilder(pParent, "svx/ui/floatingareastyle.ui"))
    , mxTopLevel(mxBuilder->weld_container("FloatingAreaStyle"))
    , mxCenterGrid(mxBuilder->weld_widget("centergrid"))
    , mxAngleGrid(mxBuilder->weld_widget("anglegrid"))
    , mxMtrTrgrCenterX(mxBuilder->weld_metric_spin_button("centerx", FieldUnit::PERCENT))
    , mxMtrTrgrCenterY(mxBuilder->weld_metric_spin_button("centery", FieldUnit::PERCENT))
    , mxMtrTrgrAngle(mxBuilder->weld_metric_spin_button("angle", FieldUnit::DEGREE))
    , mxBtnLeft45(mxBuilder->weld_toolbar("lefttoolbox"))
    , mxBtnRight45(mxBuilder->weld_toolbar("righttoolbox"))
    , mxMtrTrgrStartValue(mxBuilder->weld_metric_spin_button("start", FieldUnit::PERCENT))
    , mxMtrTrgrEndValue(mxBuilder->weld_metric_spin_button("end", FieldUnit::PERCENT))
    , mxMtrTrgrBorder(mxBuilder->weld_metric_spin_button("border", FieldUnit::PERCENT))
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
    mxTopLevel->connect_focus_in(LINK(this, AreaTransparencyGradientPopup, FocusHdl));
}

AreaTransparencyGradientPopup::~AreaTransparencyGradientPopup()
{
}

void AreaTransparencyGradientPopup::InitStatus(XFillFloatTransparenceItem const * pGradientItem)
{
    const XGradient& rGradient = pGradientItem->GetGradientValue();

    XGradient aGradient;

    if (rGradient.GetXOffset() == AreaPropertyPanelBase::DEFAULT_CENTERX
        && rGradient.GetYOffset() == AreaPropertyPanelBase::DEFAULT_CENTERY
        && (rGradient.GetAngle().get() / 10) == AreaPropertyPanelBase::DEFAULT_ANGLE
        && static_cast<sal_uInt16>(((static_cast<sal_uInt16>(rGradient.GetStartColor().GetRed()) + 1) * 100) / 255)
            == AreaPropertyPanelBase::DEFAULT_STARTVALUE
        && static_cast<sal_uInt16>(((static_cast<sal_uInt16>(rGradient.GetEndColor().GetRed()) + 1) * 100) / 255)
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
    mxMtrTrgrAngle->set_value(aGradient.GetAngle().get() / 10, FieldUnit::DEGREE);
    mxMtrTrgrStartValue->set_value(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aGradient.GetStartColor().GetRed()) + 1) * 100) / 255), FieldUnit::PERCENT);
    mxMtrTrgrEndValue->set_value(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aGradient.GetEndColor().GetRed()) + 1) * 100) / 255), FieldUnit::PERCENT);
    mxMtrTrgrBorder->set_value(aGradient.GetBorder(), FieldUnit::PERCENT);
}

void AreaTransparencyGradientPopup::Rearrange(XFillFloatTransparenceItem const * pGradientItem)
{
    InitStatus(pGradientItem);
    const XGradient& rGradient = pGradientItem->GetGradientValue();
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

void AreaTransparencyGradientPopup::ExecuteValueModify(sal_uInt8 nStartCol, sal_uInt8 nEndCol)
{
    //Added
    sal_Int16 aMtrValue = static_cast<sal_Int16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE));
    while(aMtrValue<0)
        aMtrValue += 360;
    sal_uInt16 nVal = aMtrValue/360;
    nVal = aMtrValue - nVal*360;
    mxMtrTrgrAngle->set_value(nVal, FieldUnit::DEGREE);
    //End of new code
    XGradient aTmpGradient(
        Color(nStartCol, nStartCol, nStartCol),
        Color(nEndCol, nEndCol, nEndCol),
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
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxMtrTrgrStartValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxMtrTrgrEndValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    ExecuteValueModify( nStartCol, nEndCol );
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, Left_Click45_Impl, const OString&, void)
{
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxMtrTrgrStartValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxMtrTrgrEndValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    sal_uInt16 nTemp = static_cast<sal_uInt16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE));
    if (nTemp>=315)
        nTemp -= 360;
    nTemp += 45;
    mxMtrTrgrAngle->set_value(nTemp, FieldUnit::DEGREE);
    ExecuteValueModify(nStartCol, nEndCol);
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, Right_Click45_Impl, const OString&, void)
{
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxMtrTrgrStartValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(mxMtrTrgrEndValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    sal_uInt16 nTemp = static_cast<sal_uInt16>(mxMtrTrgrAngle->get_value(FieldUnit::DEGREE));
    if (nTemp<45)
        nTemp += 360;
    nTemp -= 45;
    mxMtrTrgrAngle->set_value(nTemp, FieldUnit::DEGREE);
    ExecuteValueModify(nStartCol, nEndCol);
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, FocusHdl, weld::Widget&, void)
{
    mxMtrTrgrCenterX->grab_focus();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
