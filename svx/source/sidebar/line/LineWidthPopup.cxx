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
#include <svtools/viewoptions.hxx>
#include <svx/sidebar/LineWidthPopup.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xlnwtit.hxx>
#include <tools/fldunit.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/weldutils.hxx>
#include <bitmaps.hlst>

namespace svx::sidebar
{
LineWidthPopup::LineWidthPopup(weld::Widget* pParent, LinePropertyPanelBase& rParent)
    : WeldToolbarPopup(nullptr, pParent, u"svx/ui/floatinglineproperty.ui"_ustr,
                       u"FloatingLineProperty"_ustr)
    , m_rParent(rParent)
    , m_sPt(SvxResId(RID_SVXSTR_PT))
    , m_eMapUnit(MapUnit::MapTwip)
    , m_bTreeViewFocus(true)
    , m_bCustom(false)
    , m_nCustomWidth(0)
    , m_aIMGCus(StockImage::Yes, RID_SVXBMP_WIDTH_CUSTOM)
    , m_xMFWidth(m_xBuilder->weld_metric_spin_button(u"spin"_ustr, FieldUnit::POINT))
    , m_xWidthTreeView(m_xBuilder->weld_tree_view("linetreeview"))
{
    maStrUnits = { u"0.5"_ustr, u"0.8"_ustr, u"1.0"_ustr,
                   u"1.5"_ustr, u"2.3"_ustr, u"3.0"_ustr,
                   u"4.5"_ustr, u"6.0"_ustr, SvxResId(RID_SVXSTR_WIDTH_LAST_CUSTOM) };

    const LocaleDataWrapper& rLocaleWrapper(Application::GetSettings().GetLocaleDataWrapper());
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    for (int i = 0; i <= 7; i++)
    {
        maStrUnits[i] = maStrUnits[i].replace('.', cSep); //Modify
        maStrUnits[i] += " ";
        maStrUnits[i] += m_sPt;
    }

    for (int i = 0; i < 9; ++i)
    {
        m_xWidthTreeView->insert(i);
        ScopedVclPtr<VirtualDevice> pImage = CreateImage(i);
        m_xWidthTreeView->set_image(i, *pImage, 0);
        m_xWidthTreeView->set_text(i, maStrUnits.at(i), 1);
        m_xWidthTreeView->set_sensitive(i, true);
    }
    m_xWidthTreeView->columns_autosize();

    m_xWidthTreeView->set_id(0, OUString::number(5));
    m_xWidthTreeView->set_id(1, OUString::number(8));
    m_xWidthTreeView->set_id(2, OUString::number(10));
    m_xWidthTreeView->set_id(3, OUString::number(15));
    m_xWidthTreeView->set_id(4, OUString::number(23));
    m_xWidthTreeView->set_id(5, OUString::number(30));
    m_xWidthTreeView->set_id(6, OUString::number(45));
    m_xWidthTreeView->set_id(7, OUString::number(60));

    UnselectTreeViewItems();

    m_xWidthTreeView->connect_item_activated(LINK(this, LineWidthPopup, TreeViewItemActivatedHdl));
    m_xMFWidth->connect_value_changed(LINK(this, LineWidthPopup, MFModifyHdl));
}

LineWidthPopup::~LineWidthPopup() {}

ScopedVclPtr<VirtualDevice> LineWidthPopup::CreateImage(int nIndex)
{
    ScopedVclPtr<VirtualDevice> pDev = m_xWidthTreeView->create_virtual_device();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    pDev->SetBackground(rStyleSettings.GetFieldColor());
    pDev->SetLineColor(rStyleSettings.GetFieldTextColor());
    pDev->SetFillColor(pDev->GetLineColor());
    pDev->SetOutputSizePixel(Size(50, 26));

    if (nIndex >= 0 && nIndex <= 7)
    {
        pDev->DrawRect(tools::Rectangle(5, 10, 40, 11 + nIndex));
    }
    else
    {
        assert(nIndex == 8 && "Invalid index");
        const Point aStartPoint(
            (pDev->GetOutputWidthPixel() - m_aIMGCus.GetSizePixel().getWidth()) / 2,
            (pDev->GetOutputHeightPixel() - m_aIMGCus.GetSizePixel().getHeight()) / 2);
        pDev->DrawImage(aStartPoint, m_aIMGCus);
    }

    return pDev;
}

void LineWidthPopup::UnselectTreeViewItems()
{
    m_xWidthTreeView->unselect_all();
    m_xWidthTreeView->set_cursor(-1);
}

IMPL_LINK(LineWidthPopup, TreeViewItemActivatedHdl, const weld::TreeIter&, rIter, bool)
{
    const int nPos = m_xWidthTreeView->get_iter_index_in_parent(rIter);
    if (nPos >= 0 && nPos <= 7)
    {
        sal_Int64 nVal = OutputDevice::LogicToLogic(m_xWidthTreeView->get_id(rIter).toInt64(),
                                                    MapUnit::MapPoint, m_eMapUnit);
        nVal = m_xMFWidth->denormalize(nVal);
        XLineWidthItem aWidthItem(nVal);
        m_rParent.setLineWidth(aWidthItem);
        m_rParent.SetWidthIcon(nPos + 1);
        m_rParent.SetWidth(nVal);
    }
    else if (nPos == 8)
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
            // add: set no selection and keep the last selected item
            m_xWidthTreeView->unselect_all();
        }
        //modify end
    }

    if ((nPos >= 0 && nPos <= 7) || (nPos == 8 && m_bCustom)) //add
    {
        m_rParent.EndLineWidthPopup();
    }

    return true;
}

IMPL_LINK_NOARG(LineWidthPopup, MFModifyHdl, weld::MetricSpinButton&, void)
{
    UnselectTreeViewItems();
    tools::Long nTmp = static_cast<tools::Long>(m_xMFWidth->get_value(FieldUnit::NONE));
    tools::Long nVal = OutputDevice::LogicToLogic(nTmp, MapUnit::MapPoint, m_eMapUnit);
    sal_Int32 nNewWidth = static_cast<short>(m_xMFWidth->denormalize(nVal));
    XLineWidthItem aWidthItem(nNewWidth);
    m_rParent.setLineWidth(aWidthItem);
}

void LineWidthPopup::SetWidthSelect(tools::Long lValue, bool bValuable, MapUnit eMapUnit)
{
    m_bTreeViewFocus = true;
    UnselectTreeViewItems();
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
        m_xWidthTreeView->set_sensitive(8, true);

        OUString aStrTip = OUString::number(static_cast<double>(m_nCustomWidth) / 10) + m_sPt;
        m_xWidthTreeView->set_text(8, aStrTip);
    }
    else
    {
        m_bCustom = false;
        m_xWidthTreeView->set_sensitive(8, false);
        m_xWidthTreeView->set_text(8, maStrUnits.at(8));
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
    int i = 0;
    for (; i < 8; i++)
    {
        if (strCurrValue == maStrUnits[i])
        {
            m_xWidthTreeView->select(i);
            m_xWidthTreeView->grab_focus();
            break;
        }
    }

    if (i >= 8)
    {
        m_bTreeViewFocus = false;
        UnselectTreeViewItems();
    }
}

void LineWidthPopup::GrabFocus()
{
    if (m_bTreeViewFocus)
        m_xWidthTreeView->grab_focus();
    else
        m_xMFWidth->grab_focus();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
