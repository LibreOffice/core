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

#include <sfx2/app.hxx>
#include <svx/sidebar/AreaTransparencyGradientPopup.hxx>
#include <svx/sidebar/AreaPropertyPanelBase.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xgrad.hxx>

namespace svx { namespace sidebar {

AreaTransparencyGradientPopup::AreaTransparencyGradientPopup(AreaPropertyPanelBase& rPanel)
    : FloatingWindow(SfxGetpApp()->GetTopWindow(), "FloatingAreaStyle", "svx/ui/floatingareastyle.ui")
    , mrAreaPropertyPanel(rPanel)
{
    get(maCenterGrid, "centergrid");
    get(maAngleGrid, "anglegrid");
    get(maMtrTrgrCenterX, "centerx");
    get(maMtrTrgrCenterY, "centery");
    get(maMtrTrgrAngle, "angle");
    get(maBtnLeft45, "lefttoolbox");
    get(maBtnRight45, "righttoolbox");
    get(maMtrTrgrStartValue, "start");
    get(maMtrTrgrEndValue, "end");
    get(maMtrTrgrBorder, "border");

    Link<Edit&,void> aLink = LINK(this, AreaTransparencyGradientPopup, ModifiedTrgrHdl_Impl);
    maMtrTrgrCenterX->SetModifyHdl(aLink);
    maMtrTrgrCenterY->SetModifyHdl(aLink);
    maMtrTrgrAngle->SetModifyHdl(aLink);
    maMtrTrgrBorder->SetModifyHdl(aLink);
    maMtrTrgrStartValue->SetModifyHdl(aLink);
    maMtrTrgrEndValue->SetModifyHdl(aLink);
    maBtnLeft45->SetSelectHdl(LINK(this, AreaTransparencyGradientPopup, Left_Click45_Impl));
    maBtnRight45->SetSelectHdl(LINK(this, AreaTransparencyGradientPopup, Right_Click45_Impl));
}

AreaTransparencyGradientPopup::~AreaTransparencyGradientPopup()
{
    disposeOnce();
}

void AreaTransparencyGradientPopup::InitStatus(XFillFloatTransparenceItem const * pGradientItem)
{
    const XGradient& rGradient = pGradientItem->GetGradientValue();

    XGradient aGradient;

    if (rGradient.GetXOffset() == AreaPropertyPanelBase::DEFAULT_CENTERX
        && rGradient.GetYOffset() == AreaPropertyPanelBase::DEFAULT_CENTERY
        && (rGradient.GetAngle() / 10) == AreaPropertyPanelBase::DEFAULT_ANGLE
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
    maMtrTrgrCenterX->SetValue(aGradient.GetXOffset());
    maMtrTrgrCenterY->SetValue(aGradient.GetYOffset());
    maMtrTrgrAngle->SetValue(aGradient.GetAngle() / 10);
    maMtrTrgrStartValue->SetValue(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aGradient.GetStartColor().GetRed()) + 1) * 100) / 255));
    maMtrTrgrEndValue->SetValue(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(aGradient.GetEndColor().GetRed()) + 1) * 100) / 255));
    maMtrTrgrBorder->SetValue(aGradient.GetBorder());
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
            maCenterGrid->Hide();
            maAngleGrid->Show();
            break;
        case css::awt::GradientStyle_RADIAL:
            maCenterGrid->Show();
            maAngleGrid->Hide();
            break;
        case css::awt::GradientStyle_ELLIPTICAL:
        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            maCenterGrid->Show();
            maAngleGrid->Show();
            break;
        default:
            break;
    }
}

void AreaTransparencyGradientPopup::ExecuteValueModify(sal_uInt8 nStartCol, sal_uInt8 nEndCol)
{
    //Added
    sal_Int16 aMtrValue = static_cast<sal_Int16>(maMtrTrgrAngle->GetValue());
    while(aMtrValue<0)
        aMtrValue += 360;
    sal_uInt16 nVal = aMtrValue/360;
    nVal = aMtrValue - nVal*360;
    maMtrTrgrAngle->SetValue(nVal);
    //End of new code
    XGradient aTmpGradient(
        Color(nStartCol, nStartCol, nStartCol),
        Color(nEndCol, nEndCol, nEndCol),
        static_cast<css::awt::GradientStyle>(mrAreaPropertyPanel.GetSelectedTransparencyTypeIndex()-2),
        static_cast<sal_uInt16>(maMtrTrgrAngle->GetValue()) * 10,
        static_cast<sal_uInt16>(maMtrTrgrCenterX->GetValue()),
        static_cast<sal_uInt16>(maMtrTrgrCenterY->GetValue()),
        static_cast<sal_uInt16>(maMtrTrgrBorder->GetValue()),
        100, 100);

    mrAreaPropertyPanel.SetGradient(aTmpGradient);

    XFillFloatTransparenceItem aGradientItem(aTmpGradient, true );

    mrAreaPropertyPanel.setFillFloatTransparence(aGradientItem);
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, ModifiedTrgrHdl_Impl, Edit&, void)
{
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(maMtrTrgrStartValue->GetValue()) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(maMtrTrgrEndValue->GetValue()) * 255) / 100);
    ExecuteValueModify( nStartCol, nEndCol );
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, Left_Click45_Impl, ToolBox *, void)
{
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(maMtrTrgrStartValue->GetValue()) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(maMtrTrgrEndValue->GetValue()) * 255) / 100);
    sal_uInt16 nTemp = static_cast<sal_uInt16>(maMtrTrgrAngle->GetValue());
    if (nTemp>=315)
        nTemp -= 360;
    nTemp += 45;
    maMtrTrgrAngle->SetValue(nTemp);
    ExecuteValueModify(nStartCol, nEndCol);
}

IMPL_LINK_NOARG(AreaTransparencyGradientPopup, Right_Click45_Impl, ToolBox *, void)
{
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(maMtrTrgrStartValue->GetValue()) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(maMtrTrgrEndValue->GetValue()) * 255) / 100);
    sal_uInt16 nTemp = static_cast<sal_uInt16>(maMtrTrgrAngle->GetValue());
    if (nTemp<45)
        nTemp += 360;
    nTemp -= 45;
    maMtrTrgrAngle->SetValue(nTemp);
    ExecuteValueModify(nStartCol, nEndCol);
}

void AreaTransparencyGradientPopup::dispose()
{
    maCenterGrid.clear();
    maAngleGrid.clear();
    maMtrTrgrCenterX.clear();
    maMtrTrgrCenterY.clear();
    maMtrTrgrAngle.clear();
    maBtnLeft45.clear();
    maBtnRight45.clear();
    maMtrTrgrStartValue.clear();
    maMtrTrgrEndValue.clear();
    maMtrTrgrBorder.clear();
    FloatingWindow::dispose();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
