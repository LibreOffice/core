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
#include "LineWidthControl.hxx"
#include "LinePropertyPanel.hrc"
#include <svx/sidebar/LinePropertyPanelBase.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/viewoptions.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "svx/sidebar/PopupContainer.hxx"

namespace svx { namespace sidebar {

LineWidthControl::LineWidthControl (
    vcl::Window* pParent,
    LinePropertyPanelBase& rPanel)
    : svx::sidebar::PopupControl(pParent,SVX_RES(RID_POPUPPANEL_LINEPAGE_WIDTH)),
      mrLinePropertyPanel(rPanel),
      maVSWidth(VclPtr<LineWidthValueSet>::Create(this, SVX_RES(VS_WIDTH))),
      maFTCus( VclPtr<FixedText>::Create(this, SVX_RES(FT_CUSTOME))),
      maFTWidth( VclPtr<FixedText>::Create(this, SVX_RES(FT_LINE_WIDTH))),
      maMFWidth( VclPtr<MetricField>::Create(this, SVX_RES(MF_WIDTH))),
      meMapUnit(SFX_MAPUNIT_TWIP),
      rStr(nullptr),
      mstrPT(SVX_RESSTR(STR_PT)),
      mnCustomWidth(0),
      mbCustom(false),
      mbCloseByEdit(false),
      mnTmpCustomWidth(0),
      mbVSFocus(true),
      maIMGCus(SVX_RES(IMG_WIDTH_CUSTOM)),
      maIMGCusGray(SVX_RES(IMG_WIDTH_CUSTOM_GRAY))
{
    Initialize();
    FreeResource();
}

LineWidthControl::~LineWidthControl()
{
    disposeOnce();
}

void LineWidthControl::dispose()
{
    delete[] rStr;
    maVSWidth.disposeAndClear();
    maFTCus.disposeAndClear();
    maFTWidth.disposeAndClear();
    maMFWidth.disposeAndClear();
    svx::sidebar::PopupControl::dispose();
}

void LineWidthControl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rect)
{
    svx::sidebar::PopupControl::Paint(rRenderContext, rect);

    rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);

    Point aPos(rRenderContext.LogicToPixel(Point(CUSTOM_X, CUSTOM_Y), MAP_APPFONT));
    Size aSize(rRenderContext.LogicToPixel(Size(CUSTOM_W, CUSTOM_H), MAP_APPFONT));
    Rectangle aRect(aPos, aSize);
    aRect.Left() -= 1;
    aRect.Top() -= 1;
    aRect.Right() += 1;
    aRect.Bottom() += 1;

    Color aLineColor(189, 201, 219);
    if (!GetSettings().GetStyleSettings().GetHighContrastMode())
        rRenderContext.SetLineColor(aLineColor);
    else
        rRenderContext.SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
    rRenderContext.SetFillColor(COL_TRANSPARENT);
    rRenderContext.DrawRect(aRect);

    rRenderContext.Pop();
}

void LineWidthControl::Initialize()
{
    maVSWidth->SetStyle( maVSWidth->GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT  );// WB_NAMEFIELD | WB_ITEMBORDER |WB_DOUBLEBORDER | WB_NONEFIELD |
    //for high contrast wj
    if(GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        maVSWidth->SetColor(GetSettings().GetStyleSettings().GetMenuColor());
    //  maBorder.SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
        maFTWidth->SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
    }
    else
    {
        maVSWidth->SetColor(COL_WHITE);
    //  maBorder.SetBackground(Wallpaper(COL_WHITE));
        maFTWidth->SetBackground(Wallpaper(COL_WHITE));
    }

    sal_Int64 nFirst= maMFWidth->Denormalize( maMFWidth->GetFirst( FUNIT_TWIP ) );
    sal_Int64 nLast = maMFWidth->Denormalize( maMFWidth->GetLast( FUNIT_TWIP ) );
    sal_Int64 nMin = maMFWidth->Denormalize( maMFWidth->GetMin( FUNIT_TWIP ) );
    sal_Int64 nMax = maMFWidth->Denormalize( maMFWidth->GetMax( FUNIT_TWIP ) );
    maMFWidth->SetSpinSize( 10 );
    maMFWidth->SetUnit( FUNIT_POINT );
    if( maMFWidth->GetDecimalDigits() > 1 )
         maMFWidth->SetDecimalDigits( 1 );
    maMFWidth->SetFirst( maMFWidth->Normalize( nFirst ), FUNIT_TWIP );
    maMFWidth->SetLast( maMFWidth->Normalize( nLast ), FUNIT_TWIP );
    maMFWidth->SetMin( maMFWidth->Normalize( nMin ), FUNIT_TWIP );
    maMFWidth->SetMax( maMFWidth->Normalize( nMax ), FUNIT_TWIP );

    rStr = new OUString[9];
    //modify,
    rStr[0] = "0.5";
    rStr[1] = "0.8";
    rStr[2] = "1.0";
    rStr[3] = "1.5";
    rStr[4] = "2.3";
    rStr[5] = "3.0";
    rStr[6] = "4.5";
    rStr[7] = "6.0";
    rStr[8] = SVX_RESSTR(STR_WIDTH_LAST_CUSTOM);

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    for(int i = 0; i <= 7 ; i++)
    {
        rStr[i] = rStr[i].replace('.', cSep);//Modify
        rStr[i] += " ";
        rStr[i] += mstrPT;
    }
    //end

    for(sal_uInt16 i = 1 ; i <= 9 ; i++)
    {
        maVSWidth->InsertItem(i);
        maVSWidth->SetItemText(i, rStr[i-1]);
    }
    maVSWidth->SetUnit(rStr);
    maVSWidth->SetItemData(1, reinterpret_cast<void*>(5));
    maVSWidth->SetItemData(2, reinterpret_cast<void*>(8));
    maVSWidth->SetItemData(3, reinterpret_cast<void*>(10));
    maVSWidth->SetItemData(4, reinterpret_cast<void*>(15));
    maVSWidth->SetItemData(5, reinterpret_cast<void*>(23));
    maVSWidth->SetItemData(6, reinterpret_cast<void*>(30));
    maVSWidth->SetItemData(7, reinterpret_cast<void*>(45));
    maVSWidth->SetItemData(8, reinterpret_cast<void*>(60));
    maVSWidth->SetImage(maIMGCusGray);

    maVSWidth->SetSelItem(0);

    maVSWidth->SetSelectHdl(LINK( this, LineWidthControl, VSSelectHdl ));
    maMFWidth->SetModifyHdl(LINK(this, LineWidthControl, MFModifyHdl));

    maVSWidth->StartSelection();
    maVSWidth->Show();
}

void LineWidthControl::GetFocus()
{
    if (!mbVSFocus && maMFWidth)
        maMFWidth->GrabFocus();
    else if (maVSWidth)
        maVSWidth->GrabFocus();
}

void LineWidthControl::SetWidthSelect( long lValue, bool bValuable, SfxMapUnit eMapUnit)
{
    mbVSFocus = true;
    maVSWidth->SetSelItem(0);
    mbCloseByEdit = false;
    meMapUnit = eMapUnit;
    SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_LINE_WIDTH_GLOBAL_VALUE );
    if (aWinOpt.Exists())
    {
        css::uno::Sequence <css::beans::NamedValue> aSeq = aWinOpt.GetUserData();
        OUString aTmp;
        if ( aSeq.getLength())
            aSeq[0].Value >>= aTmp;

        OUString aWinData( aTmp );
        mnCustomWidth = aWinData.toInt32();
        mbCustom = true;
        maVSWidth->SetImage(maIMGCus);
        maVSWidth->SetCusEnable(true);

        OUString aStrTip( OUString::number( (double)mnCustomWidth / 10));
        aStrTip += mstrPT;
        maVSWidth->SetItemText(9, aStrTip);
    }
    else
    {
        mbCustom = false;
        maVSWidth->SetImage(maIMGCusGray);
        maVSWidth->SetCusEnable(false);
        //modify
        //String aStrTip(String(SVX_RES(STR_WIDTH_LAST_CUSTOM)));
        //maVSWidth->SetItemText(9, aStrTip);
        maVSWidth->SetItemText(9, rStr[8]);
    }

    if (bValuable)
    {
        sal_Int64 nVal = OutputDevice::LogicToLogic(lValue, (MapUnit) eMapUnit, MAP_100TH_MM );
        nVal = maMFWidth->Normalize(nVal);
        maMFWidth->SetValue( nVal, FUNIT_100TH_MM );
    }
    else
    {
        maMFWidth->SetText( "" );
    }

    OUString strCurrValue = maMFWidth->GetText();
    sal_uInt16 i = 0;
    for(; i < 8; i++)
    {
        if(strCurrValue == rStr[i])
        {
            maVSWidth->SetSelItem(i+1);
            break;
        }
    }

    if (i>=8)
    {
        mbVSFocus = false;
        maVSWidth->SetSelItem(0);
    }
    maVSWidth->SetFormat();
    maVSWidth->Invalidate();
    maVSWidth->StartSelection();
}

IMPL_LINK_TYPED(LineWidthControl, VSSelectHdl, ValueSet*, pControl, void)
{
    if (pControl == maVSWidth.get())
    {
        sal_uInt16 iPos = maVSWidth->GetSelectItemId();
        if (iPos >= 1 && iPos <= 8)
        {
            sal_IntPtr nVal = LogicToLogic(reinterpret_cast<sal_IntPtr>(maVSWidth->GetItemData( iPos )), MAP_POINT, (MapUnit)meMapUnit);
            nVal = maMFWidth->Denormalize(nVal);
            XLineWidthItem aWidthItem( nVal );
            mrLinePropertyPanel.setLineWidth(aWidthItem);
            mrLinePropertyPanel.SetWidthIcon(iPos);
            mrLinePropertyPanel.SetWidth(nVal);
            mbCloseByEdit = false;
            mnTmpCustomWidth = 0;
        }
        else if (iPos == 9)
        {//last custom
            //modified
            if (mbCustom)
            {
                long nVal = LogicToLogic(mnCustomWidth , MAP_POINT, (MapUnit)meMapUnit);
                nVal = maMFWidth->Denormalize(nVal);
                XLineWidthItem aWidthItem( nVal );
                mrLinePropertyPanel.setLineWidth(aWidthItem);
                mrLinePropertyPanel.SetWidth(nVal);
                mbCloseByEdit = false;
                mnTmpCustomWidth = 0;
            }
            else
            {
                maVSWidth->SetNoSelection();     //add , set no selection and keep the last select item
                maVSWidth->SetFormat();
                maVSWidth->Invalidate();
                Invalidate();
                maVSWidth->StartSelection();
            }
            //modify end
        }
        if ((iPos >= 1 && iPos <= 8) || (iPos == 9 && mbCustom)) //add
            mrLinePropertyPanel.EndLineWidthPopupMode();
    }
}

IMPL_LINK_TYPED(LineWidthControl, MFModifyHdl, Edit&, rControl, void)
{
    if (&rControl == maMFWidth.get())
    {
        if(maVSWidth->GetSelItem())
        {
            maVSWidth->SetSelItem(0);
            maVSWidth->SetFormat();
            maVSWidth->Invalidate();
            Invalidate();
            maVSWidth->StartSelection();
        }
        long nTmp = static_cast<long>(maMFWidth->GetValue());
        long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)meMapUnit );
        sal_Int32 nNewWidth = (short)maMFWidth->Denormalize( nVal );
        XLineWidthItem aWidthItem(nNewWidth);
        mrLinePropertyPanel.setLineWidth(aWidthItem);

        mbCloseByEdit = true;
        mnTmpCustomWidth = nTmp;
    }
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
