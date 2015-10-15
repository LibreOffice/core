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
#include "AreaTransparencyGradientControl.hxx"
#include <svx/sidebar/AreaPropertyPanelBase.hxx>
#include "AreaPropertyPanel.hrc"

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xflftrit.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace svx { namespace sidebar {

// positioning helpers
#define APOS1_1 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL), MAP_APPFONT))
#define APOS2_1 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL), MAP_APPFONT))
#define APOS1_2 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL+CONTROL_WIDTH+CONTROL_SPACING_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL), MAP_APPFONT))
#define APOS2_2 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL+CONTROL_WIDTH+CONTROL_SPACING_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL), MAP_APPFONT))
#define APOS1_3 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL), MAP_APPFONT))
#define APOS1_4 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 2*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL)+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL), MAP_APPFONT))
#define APOS2_3 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL+CONTROL_WIDTH+CONTROL_SPACING_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL), MAP_APPFONT))
#define APOS2_4 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL+CONTROL_WIDTH+CONTROL_SPACING_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 2*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL)+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL), MAP_APPFONT))
#define APOS1_5 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 2*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL)), MAP_APPFONT))
#define APOS1_6 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 3*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL)+2*(MBOX_HEIGHT+CONTROL_SPACING_VERTICAL)), MAP_APPFONT))
#define APOS2_5 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL+CONTROL_WIDTH+CONTROL_SPACING_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 2*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL)), MAP_APPFONT))
#define APOS2_6 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL+CONTROL_WIDTH+CONTROL_SPACING_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 3*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL)+2*(MBOX_HEIGHT+CONTROL_SPACING_VERTICAL)), MAP_APPFONT))
#define APOS1_7 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 3*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL+MBOX_HEIGHT+CONTROL_SPACING_VERTICAL)), MAP_APPFONT))
#define APOS1_8 Point(LogicToPixel(Point(POPUPPANEL_MARGIN_HORIZONTAL,POPUPPANEL_MARGIN_VERTICAL + 4*(FIXED_TEXT_HEIGHT + TEXT_CONTROL_SPACING_VERTICAL)+3*(MBOX_HEIGHT+CONTROL_SPACING_VERTICAL)), MAP_APPFONT))
#define APOS_Left_Right_1 Point(LogicToPixel(Point(LEFT_RIGHT_X1,LEFT_RIGHT_Y1), MAP_APPFONT))
#define APOS_Left_Right_2 Point(LogicToPixel(Point(LEFT_RIGHT_X2,LEFT_RIGHT_Y1), MAP_APPFONT))
#define APOS_Left_Right_3 Point(LogicToPixel(Point(LEFT_RIGHT_X1,LEFT_RIGHT_Y2), MAP_APPFONT))
#define APOS_Left_Right_4 Point(LogicToPixel(Point(LEFT_RIGHT_X2,LEFT_RIGHT_Y2), MAP_APPFONT))

AreaTransparencyGradientControl::AreaTransparencyGradientControl (
    vcl::Window* pParent,
    AreaPropertyPanelBase& rPanel)
    : PopupControl( pParent,SVX_RES(RID_POPUPPANEL_AREAPAGE_TRGR)),
      maFtTrgrCenterX(VclPtr<FixedText>::Create(this, SVX_RES(FT_TRGR_CENTER_X))),
      maMtrTrgrCenterX(VclPtr<MetricField>::Create(this, SVX_RES(MTR_TRGR_CENTER_X))),
      maFtTrgrCenterY(VclPtr<FixedText>::Create(this, SVX_RES(FT_TRGR_CENTER_Y))),
      maMtrTrgrCenterY(VclPtr<MetricField>::Create(this, SVX_RES(MTR_TRGR_CENTER_Y))),
      maFtTrgrAngle(VclPtr<FixedText>::Create(this, SVX_RES(FT_TRGR_ANGLE))),
      maMtrTrgrAngle(VclPtr<MetricField>::Create(this, SVX_RES(MTR_TRGR_ANGLE))),
      maBtnLeft45(VclPtr<ToolBox>::Create(this, SVX_RES(BTN_LEFT_SECOND))),
      maBtnRight45(VclPtr<ToolBox>::Create(this, SVX_RES(BTN_RIGHT_FIRST))),
      maFtTrgrStartValue(VclPtr<FixedText>::Create(this, SVX_RES(FT_TRGR_START_VALUE))),
      maMtrTrgrStartValue(VclPtr<MetricField>::Create(this, SVX_RES(MTR_TRGR_START_VALUE))),
      maFtTrgrEndValue(VclPtr<FixedText>::Create(this, SVX_RES(FT_TRGR_END_VALUE))),
      maMtrTrgrEndValue(VclPtr<MetricField>::Create(this, SVX_RES(MTR_TRGR_END_VALUE))),
      maFtTrgrBorder(VclPtr<FixedText>::Create(this, SVX_RES(FT_TRGR_BORDER))),
      maMtrTrgrBorder(VclPtr<MetricField>::Create(this, SVX_RES(MTR_TRGR_BORDER))),
      maRotLeft( SVX_RES(IMG_ROT_LEFT)),
      maRotRight( SVX_RES(IMG_ROT_RIGHT)),
      mrAreaPropertyPanel(rPanel)
{
    Link<Edit&,void> aLink = LINK( this, AreaTransparencyGradientControl, ModifiedTrgrHdl_Impl);
    maMtrTrgrCenterX->SetModifyHdl( aLink );
    maMtrTrgrCenterY->SetModifyHdl( aLink );
    maMtrTrgrAngle->SetModifyHdl( aLink );
    maMtrTrgrBorder->SetModifyHdl( aLink );
    maMtrTrgrStartValue->SetModifyHdl( aLink );
    maMtrTrgrEndValue->SetModifyHdl( aLink );
    maBtnLeft45->SetSelectHdl( LINK( this, AreaTransparencyGradientControl, Left_Click45_Impl) );
    maBtnRight45->SetSelectHdl( LINK( this, AreaTransparencyGradientControl, Right_Click45_Impl) );
    maBtnLeft45->SetItemImage(1,maRotLeft);
    Size aTbxSize = maBtnLeft45->CalcWindowSizePixel();
    maBtnLeft45->SetOutputSizePixel( aTbxSize );
    maBtnLeft45->SetQuickHelpText(1, SVX_RESSTR(STR_HELP_LEFT));    //acc wj

    maBtnRight45->SetItemImage(1,maRotRight);
    aTbxSize = maBtnRight45->CalcWindowSizePixel();
    maBtnRight45->SetOutputSizePixel( aTbxSize );
    maBtnRight45->SetQuickHelpText(1, SVX_RESSTR(STR_HELP_RIGHT));  //acc wj

    maBtnLeft45->SetBackground(Wallpaper());
    maBtnLeft45->SetPaintTransparent(true);
    maBtnRight45->SetBackground(Wallpaper());
    maBtnRight45->SetPaintTransparent(true);

    FreeResource();
}

AreaTransparencyGradientControl::~AreaTransparencyGradientControl()
{
    disposeOnce();
}

void AreaTransparencyGradientControl::dispose()
{
    maFtTrgrCenterX.disposeAndClear();
    maMtrTrgrCenterX.disposeAndClear();
    maFtTrgrCenterY.disposeAndClear();
    maMtrTrgrCenterY.disposeAndClear();
    maFtTrgrAngle.disposeAndClear();
    maMtrTrgrAngle.disposeAndClear();
    maBtnLeft45.disposeAndClear();
    maBtnRight45.disposeAndClear();
    maFtTrgrStartValue.disposeAndClear();
    maMtrTrgrStartValue.disposeAndClear();
    maFtTrgrEndValue.disposeAndClear();
    maMtrTrgrEndValue.disposeAndClear();
    maFtTrgrBorder.disposeAndClear();
    maMtrTrgrBorder.disposeAndClear();
    PopupControl::dispose();
}

void AreaTransparencyGradientControl::Rearrange(XFillFloatTransparenceItem* pGradientItem)
{
    InitStatus(pGradientItem);
    const XGradient& rGradient = pGradientItem->GetGradientValue();
    css::awt::GradientStyle eXGS(rGradient.GetGradientStyle());
    Size aSize(POP_WIDTH,POP_HEIGHT);
    aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) );
    Size aSize2(POP_WIDTH,POP_HEIGHT2);
    aSize2 = LogicToPixel( aSize2, MapMode(MAP_APPFONT) );
    long aPosY = 0;
    Point aPointAngle;
    Size aSizeAngle = maMtrTrgrAngle->GetSizePixel();
    Size aTbxSize = maBtnLeft45->CalcWindowSizePixel();

    switch(eXGS)
    {
    case css::awt::GradientStyle_LINEAR:
    case css::awt::GradientStyle_AXIAL:
        maFtTrgrCenterX->Hide();
        maMtrTrgrCenterX->Hide();
        maFtTrgrCenterY->Hide();
        maMtrTrgrCenterY->Hide();
        maFtTrgrAngle->Show();
        maFtTrgrAngle->SetPosPixel(APOS1_1);
        maMtrTrgrAngle->Show();
        maMtrTrgrAngle->SetPosPixel(APOS2_1);
        maFtTrgrStartValue->SetPosPixel(APOS1_3);
        maMtrTrgrStartValue->SetPosPixel(APOS1_4);
        maFtTrgrEndValue->SetPosPixel(APOS2_3);
        maMtrTrgrEndValue->SetPosPixel(APOS2_4);
        maFtTrgrBorder->SetPosPixel(APOS1_5);
        maMtrTrgrBorder->SetPosPixel(APOS1_6);

        maBtnLeft45->Show();
        maBtnRight45->Show();

        aPointAngle = maMtrTrgrAngle->GetPosPixel();
        aPosY = aPointAngle.getY() + aSizeAngle.getHeight() - aTbxSize.getHeight();

        maBtnLeft45->SetPosPixel(Point(APOS_Left_Right_1.getX(), aPosY));
        maBtnRight45->SetPosPixel(Point(APOS_Left_Right_2.getX(), aPosY));

        SetSizePixel(aSize2);
        break;

    case css::awt::GradientStyle_RADIAL:
        maFtTrgrCenterX->Show();
        maFtTrgrCenterX->SetPosPixel(APOS1_1);
        maMtrTrgrCenterX->Show();
        maMtrTrgrCenterX->SetPosPixel(APOS2_1);
        maFtTrgrCenterY->Show();
        maFtTrgrCenterY->SetPosPixel(APOS1_2);
        maMtrTrgrCenterY->Show();
        maMtrTrgrCenterY->SetPosPixel(APOS2_2);
        maFtTrgrAngle->Hide();
        maMtrTrgrAngle->Hide();
        maFtTrgrStartValue->SetPosPixel(APOS1_3);
        maMtrTrgrStartValue->SetPosPixel(APOS1_4);
        maFtTrgrEndValue->SetPosPixel(APOS2_3);
        maMtrTrgrEndValue->SetPosPixel(APOS2_4);
        maFtTrgrBorder->SetPosPixel(APOS1_5);
        maMtrTrgrBorder->SetPosPixel(APOS1_6);

        maBtnLeft45->Hide();
        maBtnRight45->Hide();

        SetSizePixel(aSize2);

        break;

    case css::awt::GradientStyle_ELLIPTICAL:
    case css::awt::GradientStyle_SQUARE:
    case css::awt::GradientStyle_RECT:
        maFtTrgrCenterX->Show();
        maFtTrgrCenterX->SetPosPixel(APOS1_1);
        maMtrTrgrCenterX->Show();
        maMtrTrgrCenterX->SetPosPixel(APOS2_1);
        maFtTrgrCenterY->Show();
        maFtTrgrCenterY->SetPosPixel(APOS1_2);
        maMtrTrgrCenterY->Show();
        maMtrTrgrCenterY->SetPosPixel(APOS2_2);
        maFtTrgrAngle->Show();
        maFtTrgrAngle->SetPosPixel(APOS1_3);
        maMtrTrgrAngle->Show();
        maMtrTrgrAngle->SetPosPixel(APOS1_4);

        maFtTrgrStartValue->SetPosPixel(APOS1_5);
        maMtrTrgrStartValue->SetPosPixel(APOS1_6);
        maFtTrgrEndValue->SetPosPixel(APOS2_5);
        maMtrTrgrEndValue->SetPosPixel(APOS2_6);
        maFtTrgrBorder->SetPosPixel(APOS1_7);
        maMtrTrgrBorder->SetPosPixel(APOS1_8);

        maBtnLeft45->Show();
        maBtnRight45->Show();

        aPointAngle = maMtrTrgrAngle->GetPosPixel();
        aPosY = aPointAngle.getY() + aSizeAngle.getHeight() - aTbxSize.getHeight();

        maBtnLeft45->SetPosPixel(Point(APOS_Left_Right_3.getX(), aPosY));
        maBtnRight45->SetPosPixel(Point(APOS_Left_Right_4.getX(), aPosY));

        SetSizePixel(aSize);

        break;

    default:
        break;
    }
}

void AreaTransparencyGradientControl::InitStatus(XFillFloatTransparenceItem* pGradientItem)
{
    const XGradient& rGradient = pGradientItem->GetGradientValue();

    XGradient aGradient;

    if (rGradient.GetXOffset() == AreaPropertyPanelBase::DEFAULT_CENTERX
        && rGradient.GetYOffset() == AreaPropertyPanelBase::DEFAULT_CENTERY
        && (rGradient.GetAngle() / 10) == AreaPropertyPanelBase::DEFAULT_ANGLE
        && ((sal_uInt16)((((sal_uInt16)rGradient.GetStartColor().GetRed() + 1) * 100) / 255))
            == AreaPropertyPanelBase::DEFAULT_STARTVALUE
        && ((sal_uInt16)((((sal_uInt16)rGradient.GetEndColor().GetRed() + 1) * 100) / 255))
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
    maMtrTrgrStartValue->SetValue((sal_uInt16)((((sal_uInt16)aGradient.GetStartColor().GetRed() + 1) * 100) / 255));
    maMtrTrgrEndValue->SetValue((sal_uInt16)((((sal_uInt16)aGradient.GetEndColor().GetRed() + 1) * 100) / 255));
    maMtrTrgrBorder->SetValue(aGradient.GetBorder());
}

void AreaTransparencyGradientControl::ExecuteValueModify( sal_uInt8 nStartCol, sal_uInt8 nEndCol )
{
    //Added
    sal_Int16 aMtrValue = (sal_Int16)maMtrTrgrAngle->GetValue();
    while(aMtrValue<0)
        aMtrValue += 360;
    sal_uInt16 nVal = aMtrValue/360;
    nVal = aMtrValue - nVal*360;
    maMtrTrgrAngle->SetValue(nVal);
    //End of new code
    XGradient aTmpGradient(
        Color(nStartCol, nStartCol, nStartCol),
        Color(nEndCol, nEndCol, nEndCol),
        (css::awt::GradientStyle)(mrAreaPropertyPanel.GetSelectedTransparencyTypeIndex()-2),
        (sal_uInt16)maMtrTrgrAngle->GetValue() * 10,
        (sal_uInt16)maMtrTrgrCenterX->GetValue(),
        (sal_uInt16)maMtrTrgrCenterY->GetValue(),
        (sal_uInt16)maMtrTrgrBorder->GetValue(),
        100, 100);

    mrAreaPropertyPanel.SetGradient(aTmpGradient);

    SfxItemPool* pPool = NULL;
    bool bEnable = true;
    XFillFloatTransparenceItem aGradientItem(pPool,aTmpGradient, bEnable );

    mrAreaPropertyPanel.setFillFloatTransparence(aGradientItem);
}

IMPL_LINK_NOARG_TYPED(AreaTransparencyGradientControl, ModifiedTrgrHdl_Impl, Edit&, void)
{
    sal_uInt8 nStartCol = (sal_uInt8)(((sal_uInt16)maMtrTrgrStartValue->GetValue() * 255) / 100);
    sal_uInt8 nEndCol = (sal_uInt8)(((sal_uInt16)maMtrTrgrEndValue->GetValue() * 255) / 100);
    ExecuteValueModify( nStartCol, nEndCol );
}

IMPL_LINK_NOARG_TYPED(AreaTransparencyGradientControl, Left_Click45_Impl, ToolBox *, void)
{
    sal_uInt8 nStartCol = (sal_uInt8)(((sal_uInt16)maMtrTrgrStartValue->GetValue() * 255) / 100);
    sal_uInt8 nEndCol = (sal_uInt8)(((sal_uInt16)maMtrTrgrEndValue->GetValue() * 255) / 100);
    sal_uInt16 aTemp = (sal_uInt16)maMtrTrgrAngle->GetValue();
    if(aTemp>=315)
        aTemp -= 360;
    aTemp += 45;
    maMtrTrgrAngle->SetValue(aTemp);
    ExecuteValueModify( nStartCol, nEndCol );
}

IMPL_LINK_NOARG_TYPED(AreaTransparencyGradientControl, Right_Click45_Impl, ToolBox *, void)
{
    sal_uInt8 nStartCol = (sal_uInt8)(((sal_uInt16)maMtrTrgrStartValue->GetValue() * 255) / 100);
    sal_uInt8 nEndCol = (sal_uInt8)(((sal_uInt16)maMtrTrgrEndValue->GetValue() * 255) / 100);
    sal_uInt16 aTemp = (sal_uInt16)maMtrTrgrAngle->GetValue();
    if(aTemp<45)
        aTemp += 360;
    aTemp -= 45;
    maMtrTrgrAngle->SetValue(aTemp);
    ExecuteValueModify( nStartCol, nEndCol );
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
