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
#include <bitmaps.hlst>
#include <comphelper/lok.hxx>
#include <vcl/virdev.hxx>
#include <i18nlangtag/mslangid.hxx>

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
    , m_xIVWidth(m_xBuilder->weld_icon_view(u"line_iconview"_ustr))
    , aPreviewSize(300, 20)
{
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

    PopulateIconView();

    m_xIVWidth->connect_item_activated(LINK(this, LineWidthPopup, ItemActivatedHdl));

    // Avoid LibreOffice Kit crash: tooltip handlers cause segfault during JSDialog
    // serialization when popup widgets are destroyed/recreated during character formatting resets.
    // Tooltip event binding is not needed for LibreOffice Kit
    if (!comphelper::LibreOfficeKit::isActive())
    {
        m_xIVWidth->connect_query_tooltip(LINK(this, LineWidthPopup, QueryTooltipHdl));
    }

    m_xMFWidth->connect_value_changed(LINK(this, LineWidthPopup, MFModifyHdl));
}

LineWidthPopup::~LineWidthPopup() {}

IMPL_LINK_NOARG(LineWidthPopup, ItemActivatedHdl, weld::IconView&, bool)
{
    OUString sSelectedId = m_xIVWidth->get_selected_id();
    if (sSelectedId.isEmpty())
        return false;

    sal_uInt32 iPos = sSelectedId.toUInt32();

    if (iPos >= 1 && iPos <= 8)
    {
        sal_IntPtr nVal = 0;
        switch (iPos - 1)
        {
            case 0:
                nVal = 5;
                break; // 0.5pt
            case 1:
                nVal = 8;
                break; // 0.8pt
            case 2:
                nVal = 10;
                break; // 1.0pt
            case 3:
                nVal = 15;
                break; // 1.5pt
            case 4:
                nVal = 23;
                break; // 2.3pt
            case 5:
                nVal = 30;
                break; // 3.0pt
            case 6:
                nVal = 45;
                break; // 4.5pt
            case 7:
                nVal = 60;
                break; // 6.0pt
        }

        nVal = OutputDevice::LogicToLogic(nVal, MapUnit::MapPoint, m_eMapUnit);
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
        //modify end
    }

    if ((iPos >= 1 && iPos <= 8) || (iPos == 9 && m_bCustom)) //add
    {
        m_rParent.EndLineWidthPopup();
    }

    return true;
}

IMPL_LINK_NOARG(LineWidthPopup, MFModifyHdl, weld::MetricSpinButton&, void)
{
    m_xIVWidth->unselect_all();

    tools::Long nTmp = static_cast<tools::Long>(m_xMFWidth->get_value(FieldUnit::NONE));
    tools::Long nVal = OutputDevice::LogicToLogic(nTmp, MapUnit::MapPoint, m_eMapUnit);
    sal_Int32 nNewWidth = static_cast<short>(m_xMFWidth->denormalize(nVal));
    XLineWidthItem aWidthItem(nNewWidth);
    m_rParent.setLineWidth(aWidthItem);
}

IMPL_LINK(LineWidthPopup, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = m_xIVWidth->get_id(rIter);
    sal_uInt16 iPos = sId.toUInt32();
    if (iPos >= 1 && iPos <= 8)
        return maStrUnits[iPos - 1];
    else if (iPos == 9)
        return m_bCustom ? (OUString::number(static_cast<double>(m_nCustomWidth) / 10) + m_sPt)
                         : maStrUnits[8];
    return OUString();
}

void LineWidthPopup::SetWidthSelect(tools::Long lValue, bool bValuable, MapUnit eMapUnit)
{
    m_bVSFocus = true;
    m_xIVWidth->unselect_all();
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

        OUString aStrTip = OUString::number(static_cast<double>(m_nCustomWidth) / 10) + m_sPt;
        VclPtr<VirtualDevice> aCustomVDev = CreateCustomPreview(m_aIMGCus, aStrTip, true);
        m_xIVWidth->set_image(8, aCustomVDev);
        m_xIVWidth->set_text(8, aStrTip);
    }
    else
    {
        m_bCustom = false;
        VclPtr<VirtualDevice> aCustomVDev
            = CreateCustomPreview(m_aIMGCusGray, maStrUnits[8], false);
        m_xIVWidth->set_image(8, aCustomVDev);
        m_xIVWidth->set_text(8, maStrUnits[8]);
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
            m_xIVWidth->select(i);
            break;
        }
    }

    if (i >= 8)
    {
        m_bVSFocus = false;
        m_xIVWidth->unselect_all();
    }
}

void LineWidthPopup::GrabFocus()
{
    if (m_bVSFocus)
        m_xIVWidth->grab_focus();
    else
        m_xMFWidth->grab_focus();
}

VclPtr<VirtualDevice> LineWidthPopup::CreateLinePreview(sal_uInt16 nLineWidth,
                                                        const OUString& rText)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    pVDev->SetOutputSizePixel(aPreviewSize);

    // Set background
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    pVDev->SetFillColor(rStyleSettings.GetWindowColor());

    // Set font
    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS,
                                                 MsLangId::getConfiguredSystemLanguage(),
                                                 GetDefaultFontFlags::OnlyOne));
    Size aFontSize = aFont.GetFontSize();
    aFontSize.setHeight(aPreviewSize.Height() * 3 / 5);
    aFont.SetFontSize(aFontSize);
    aFont.SetColor(rStyleSettings.GetFieldTextColor());
    pVDev->SetFont(aFont);

    // Draw text
    Point aTextStart(aPreviewSize.Width() * 7 / 9, aPreviewSize.Height() / 6);
    pVDev->DrawText(aTextStart, rText);

    // Draw line with specified width
    pVDev->SetLineColor(rStyleSettings.GetFieldTextColor());
    Point aLineStart(5, (aPreviewSize.Height() - nLineWidth) / 2);
    Point aLineEnd(aPreviewSize.Width() * 7 / 9 - 10, (aPreviewSize.Height() - nLineWidth) / 2);

    for (sal_uInt16 i = 1; i <= nLineWidth; i++)
    {
        pVDev->DrawLine(aLineStart, aLineEnd);
        aLineStart.AdjustY(1);
        aLineEnd.AdjustY(1);
    }

    return pVDev;
}

VclPtr<VirtualDevice> LineWidthPopup::CreateCustomPreview(const Image& rImage,
                                                          const OUString& rText, bool bEnabled)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    pVDev->SetOutputSizePixel(aPreviewSize);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    pVDev->SetFillColor(rStyleSettings.GetWindowColor());

    // Draw image
    Point aImgStart(5, (aPreviewSize.Height() - 23) / 2);
    pVDev->DrawImage(aImgStart, rImage);

    // Set font and color
    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS,
                                                 MsLangId::getConfiguredSystemLanguage(),
                                                 GetDefaultFontFlags::OnlyOne));
    Size aFontSize = aFont.GetFontSize();
    aFontSize.setHeight(aPreviewSize.Height() * 3 / 5);
    aFont.SetFontSize(aFontSize);

    if (bEnabled)
        aFont.SetColor(rStyleSettings.GetFieldTextColor());
    else
        aFont.SetColor(rStyleSettings.GetDisableColor());

    pVDev->SetFont(aFont);

    // Draw text
    tools::Rectangle aStrRect(Point(rImage.GetSizePixel().Width() + 20, aPreviewSize.Height() / 6),
                              Size(aPreviewSize.Width() - rImage.GetSizePixel().Width() - 25,
                                   aPreviewSize.Height() - aPreviewSize.Height() / 3));
    pVDev->DrawText(aStrRect, rText, DrawTextFlags::EndEllipsis);

    return pVDev;
}

void LineWidthPopup::PopulateIconView()
{
    m_xIVWidth->clear();

    for (sal_uInt16 i = 1; i <= 8; ++i)
    {
        OUString sId = OUString::number(i);
        VclPtr<VirtualDevice> aPreview = CreateLinePreview(i, maStrUnits[i - 1]);
        m_xIVWidth->insert(-1, &maStrUnits[i - 1], &sId, aPreview, nullptr);
    }

    OUString sCustomId = OUString::number(9);
    VclPtr<VirtualDevice> aCustomPreview = CreateCustomPreview(m_aIMGCusGray, maStrUnits[8], false);
    m_xIVWidth->insert(-1, &maStrUnits[8], &sCustomId, aCustomPreview, nullptr);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
