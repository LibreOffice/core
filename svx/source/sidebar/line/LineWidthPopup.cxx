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
#include <svx/sidebar/LineWidthPopup.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xlnwtit.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include "LineWidthValueSet.hxx"
#include <bitmaps.hlst>

namespace svx::sidebar
{
LineWidthPopup::LineWidthPopup(weld::Widget* pParent, LinePropertyPanelBase& rParent)
    : WeldToolbarPopup(nullptr, pParent, u"svx/ui/floatinglineproperty.ui"_ustr,
                       u"FloatingLineProperty"_ustr)
    , m_rParent(rParent)
    , m_sPt(SvxResId(RID_SVXSTR_PT))
    , m_eMapUnit(MapUnit::MapTwip)
    , m_bVSFocus(true)
    , m_bCustom(false)
    , m_nCustomWidth(0)
    , m_aIMGCus(StockImage::Yes, RID_SVXBMP_WIDTH_CUSTOM)
    , m_aIMGCusGray(StockImage::Yes, RID_SVXBMP_WIDTH_CUSTOM_GRAY)
    , m_xMFWidth(m_xBuilder->weld_metric_spin_button(u"spin"_ustr, FieldUnit::POINT))
    , m_xVSWidth(new LineWidthValueSet())
    , m_xVSWidthWin(new weld::CustomWeld(*m_xBuilder, u"lineset"_ustr, *m_xVSWidth))
{
    m_xVSWidth->SetStyle(m_xVSWidth->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT);

    maStrUnits[0] = "0.5";
    maStrUnits[1] = "0.8";
    maStrUnits[2] = "1.0";
    maStrUnits[3] = "1.5";
    maStrUnits[4] = "2.3";
    maStrUnits[5] = "3.0";
    maStrUnits[6] = "4.5";
    maStrUnits[7] = "6.0";
    maStrUnits[8] = SvxResId(RID_SVXSTR_WIDTH_LAST_CUSTOM);

    const LocaleDataWrapper& rLocaleWrapper(Application::GetSettings().GetLocaleDataWrapper());
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    for (int i = 0; i <= 7; i++)
    {
        maStrUnits[i] = maStrUnits[i].replace('.', cSep); //Modify
        maStrUnits[i] += " ";
        maStrUnits[i] += m_sPt;
    }

    for (sal_uInt16 i = 1; i <= 9; ++i)
    {
        m_xVSWidth->InsertItem(i);
        m_xVSWidth->SetItemText(i, maStrUnits[i - 1]);
    }

    m_xVSWidth->SetUnit(maStrUnits);
    m_xVSWidth->SetItemData(1, reinterpret_cast<void*>(5));
    m_xVSWidth->SetItemData(2, reinterpret_cast<void*>(8));
    m_xVSWidth->SetItemData(3, reinterpret_cast<void*>(10));
    m_xVSWidth->SetItemData(4, reinterpret_cast<void*>(15));
    m_xVSWidth->SetItemData(5, reinterpret_cast<void*>(23));
    m_xVSWidth->SetItemData(6, reinterpret_cast<void*>(30));
    m_xVSWidth->SetItemData(7, reinterpret_cast<void*>(45));
    m_xVSWidth->SetItemData(8, reinterpret_cast<void*>(60));
    m_xVSWidth->SetImage(m_aIMGCusGray);

    m_xVSWidth->SetSelItem(0);

    m_xVSWidth->SetSelectHdl(LINK(this, LineWidthPopup, VSSelectHdl));
    m_xMFWidth->connect_value_changed(LINK(this, LineWidthPopup, MFModifyHdl));
}

LineWidthPopup::~LineWidthPopup() {}

IMPL_LINK_NOARG(LineWidthPopup, VSSelectHdl, ValueSet*, void)
{
    sal_uInt16 iPos = m_xVSWidth->GetSelectedItemId();
    if (iPos >= 1 && iPos <= 8)
    {
        sal_IntPtr nVal = OutputDevice::LogicToLogic(
            reinterpret_cast<sal_IntPtr>(m_xVSWidth->GetItemData(iPos)), MapUnit::MapPoint,
            m_eMapUnit);
        nVal = m_xMFWidth->denormalize(nVal);
        XLineWidthItem aWidthItem(nVal);
        m_rParent.setLineWidth(aWidthItem);
        m_rParent.SetWidthIcon(iPos);
        m_rParent.SetWidth(nVal);
    }
    else if (iPos == 9)
    { //last custom
        //modified
        if (m_bCustom)
        {
            tools::Long nVal
                = OutputDevice::LogicToLogic(m_nCustomWidth, MapUnit::MapPoint, m_eMapUnit);
            nVal = m_xMFWidth->denormalize(nVal);
            XLineWidthItem aWidthItem(nVal);
            m_rParent.setLineWidth(aWidthItem);
            m_rParent.SetWidth(nVal);
        }
        else
        {
            m_xVSWidth->SetNoSelection(); //add , set no selection and keep the last select item
            m_xVSWidth->SetFormat();
            m_xVSWidth->Invalidate();
        }
        //modify end
    }

    if ((iPos >= 1 && iPos <= 8) || (iPos == 9 && m_bCustom)) //add
    {
        m_rParent.EndLineWidthPopup();
    }
}

IMPL_LINK_NOARG(LineWidthPopup, MFModifyHdl, weld::MetricSpinButton&, void)
{
    if (m_xVSWidth->GetSelItem())
    {
        m_xVSWidth->SetSelItem(0);
        m_xVSWidth->SetFormat();
        m_xVSWidth->Invalidate();
    }
    tools::Long nTmp = static_cast<tools::Long>(m_xMFWidth->get_value(FieldUnit::NONE));
    tools::Long nVal = OutputDevice::LogicToLogic(nTmp, MapUnit::MapPoint, m_eMapUnit);
    sal_Int32 nNewWidth = static_cast<short>(m_xMFWidth->denormalize(nVal));
    XLineWidthItem aWidthItem(nNewWidth);
    m_rParent.setLineWidth(aWidthItem);
}

void LineWidthPopup::SetWidthSelect(tools::Long lValue, bool bValuable, MapUnit eMapUnit)
{
    m_bVSFocus = true;
    m_xVSWidth->SetSelItem(0);
    m_eMapUnit = eMapUnit;
    SvtViewOptions aWinOpt(EViewType::Window, u"PopupPanel_LineWidth"_ustr);
    if (aWinOpt.Exists())
    {
        css::uno::Sequence<css::beans::NamedValue> aSeq = aWinOpt.GetUserData();
        OUString aTmp;
        if (aSeq.hasElements())
            aSeq[0].Value >>= aTmp;

        OUString aWinData(aTmp);
        m_nCustomWidth = aWinData.toInt32();
        m_bCustom = true;
        m_xVSWidth->SetImage(m_aIMGCus);
        m_xVSWidth->SetCusEnable(true);

        OUString aStrTip = OUString::number(static_cast<double>(m_nCustomWidth) / 10) + m_sPt;
        m_xVSWidth->SetItemText(9, aStrTip);
    }
    else
    {
        m_bCustom = false;
        m_xVSWidth->SetImage(m_aIMGCusGray);
        m_xVSWidth->SetCusEnable(false);
        m_xVSWidth->SetItemText(9, maStrUnits[8]);
    }

    if (bValuable)
    {
        sal_Int64 nVal = OutputDevice::LogicToLogic(lValue, eMapUnit, MapUnit::Map100thMM);
        nVal = m_xMFWidth->normalize(nVal);
        m_xMFWidth->set_value(nVal, FieldUnit::MM_100TH);
    }
    else
    {
        m_xMFWidth->set_text(u""_ustr);
    }

    OUString strCurrValue = m_xMFWidth->get_text();
    sal_uInt16 i = 0;
    for (; i < 8; i++)
    {
        if (strCurrValue == maStrUnits[i])
        {
            m_xVSWidth->SetSelItem(i + 1);
            break;
        }
    }

    if (i >= 8)
    {
        m_bVSFocus = false;
        m_xVSWidth->SetSelItem(0);
    }

    m_xVSWidth->SetFormat();
    m_xVSWidth->Invalidate();
}

void LineWidthPopup::GrabFocus()
{
    if (m_bVSFocus)
        m_xVSWidth->GrabFocus();
    else
        m_xMFWidth->grab_focus();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
