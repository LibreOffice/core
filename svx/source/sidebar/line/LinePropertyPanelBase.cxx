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

#include <memory>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/linectrl.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xtable.hxx>
#include <bitmaps.hlst>

using namespace css;
using namespace css::uno;

constexpr OUString SELECTWIDTH = u"SelectWidth"_ustr;

namespace svx::sidebar {

// trigger disabling the arrows if the none line style is selected
class LineStyleNoneChange
{
private:
    LinePropertyPanelBase& m_rPanel;

public:
    LineStyleNoneChange(LinePropertyPanelBase& rPanel)
        : m_rPanel(rPanel)
    {
    }

    void operator()(bool bLineStyleNone)
    {
        m_rPanel.SetNoneLineStyle(bLineStyleNone);
    }
};

namespace
{
    SvxLineStyleToolBoxControl* getLineStyleToolBoxControl(const ToolbarUnoDispatcher& rToolBoxColor)
    {
        css::uno::Reference<css::frame::XToolbarController> xController = rToolBoxColor.GetControllerForCommand(u".uno:XLineStyle"_ustr);
        SvxLineStyleToolBoxControl* pToolBoxLineStyleControl = dynamic_cast<SvxLineStyleToolBoxControl*>(xController.get());
        return pToolBoxLineStyleControl;
    }
}


LinePropertyPanelBase::LinePropertyPanelBase(
    weld::Widget* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame)
:   PanelLayout(pParent, u"LinePropertyPanel"_ustr, u"svx/ui/sidebarline.ui"_ustr),
    mxTBColor(m_xBuilder->weld_toolbar(u"color"_ustr)),
    mxColorDispatch(new ToolbarUnoDispatcher(*mxTBColor, *m_xBuilder, rxFrame)),
    mxLineStyleTB(m_xBuilder->weld_toolbar(u"linestyle"_ustr)),
    mxLineStyleDispatch(new ToolbarUnoDispatcher(*mxLineStyleTB, *m_xBuilder, rxFrame)),
    mnWidthCoreValue(0),
    mxFTWidth(m_xBuilder->weld_label(u"widthlabel"_ustr)),
    mxTBWidth(m_xBuilder->weld_toolbar(u"width"_ustr)),
    mxFTTransparency(m_xBuilder->weld_label(u"translabel"_ustr)),
    mxMFTransparent(m_xBuilder->weld_metric_spin_button(u"linetransparency"_ustr, FieldUnit::PERCENT)),
    mxArrowHeadStyleFT(m_xBuilder->weld_label(u"arrowlabel"_ustr)),
    mxArrowHeadStyleTB(m_xBuilder->weld_toolbar(u"arrowheadstyle"_ustr)),
    mxArrowHeadStyleDispatch(new ToolbarUnoDispatcher(*mxArrowHeadStyleTB, *m_xBuilder, rxFrame)),
    mxLineWidthPopup(new LineWidthPopup(mxTBWidth.get(), *this)),
    mxLineStyleNoneChange(new LineStyleNoneChange(*this)),
    mnTrans(0),
    meMapUnit(MapUnit::MapMM),
    maIMGNone(BMP_NONE_ICON),
    mbWidthValuable(true),
    mbArrowSupported(true),
    mbNoneLineStyle(false)
{
    Initialize();
}

LinePropertyPanelBase::~LinePropertyPanelBase()
{
    mxLineWidthPopup.reset();
    mxFTWidth.reset();
    mxTBWidth.reset();
    mxColorDispatch.reset();
    mxTBColor.reset();
    mxFTTransparency.reset();
    mxMFTransparent.reset();
    mxLineStyleDispatch.reset();
    mxLineStyleTB.reset();
    mxArrowHeadStyleDispatch.reset();
    mxArrowHeadStyleTB.reset();
    mxArrowHeadStyleFT.reset();
}

void LinePropertyPanelBase::Initialize()
{
    mxTBWidth->set_item_popover(SELECTWIDTH, mxLineWidthPopup->getTopLevel());

    maIMGWidthIcon[0] = XDashList::CreateBitmapForXDash(nullptr, 1);
    maIMGWidthIcon[1] = XDashList::CreateBitmapForXDash(nullptr, 2);
    maIMGWidthIcon[2] = XDashList::CreateBitmapForXDash(nullptr, 3);
    maIMGWidthIcon[3] = XDashList::CreateBitmapForXDash(nullptr, 4);
    maIMGWidthIcon[4] = XDashList::CreateBitmapForXDash(nullptr, 5);
    maIMGWidthIcon[5] = XDashList::CreateBitmapForXDash(nullptr, 6);
    maIMGWidthIcon[6] = XDashList::CreateBitmapForXDash(nullptr, 7);
    maIMGWidthIcon[7] = XDashList::CreateBitmapForXDash(nullptr, 8);

    Graphic aGraf(maIMGWidthIcon[0]);
    mxTBWidth->set_item_image(SELECTWIDTH, aGraf.GetXGraphic());
    mxTBWidth->connect_clicked(LINK(this, LinePropertyPanelBase, ToolboxWidthSelectHdl));

    mxMFTransparent->connect_value_changed(LINK(this, LinePropertyPanelBase, ChangeTransparentHdl));

    SvxLineStyleToolBoxControl* pLineStyleControl = getLineStyleToolBoxControl(*mxLineStyleDispatch);
    pLineStyleControl->setLineStyleIsNoneFunction(*mxLineStyleNoneChange);
}

void LinePropertyPanelBase::updateLineTransparence(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxFTTransparency->set_sensitive(false);
        mxMFTransparent->set_sensitive(false);
    }
    else
    {
        mxFTTransparency->set_sensitive(true);
        mxMFTransparent->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineTransparenceItem* pItem = dynamic_cast<const XLineTransparenceItem*>(pState))
        {
            mnTrans = pItem->GetValue();
            mxMFTransparent->set_value(mnTrans, FieldUnit::PERCENT);
            return;
        }
    }

    mxMFTransparent->set_value(0, FieldUnit::PERCENT);//add
    mxMFTransparent->set_text(OUString());
}

void LinePropertyPanelBase::updateLineWidth(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxTBWidth->set_sensitive(false);
        mxFTWidth->set_sensitive(false);
    }
    else
    {
        mxTBWidth->set_sensitive(true);
        mxFTWidth->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineWidthItem* pItem = dynamic_cast<const XLineWidthItem*>(pState))
        {
            mnWidthCoreValue = pItem->GetValue();
            mbWidthValuable = true;
            SetWidthIcon();
            return;
        }
    }

    mbWidthValuable = false;
    SetWidthIcon();
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ToolboxWidthSelectHdl, const OUString&, void)
{
    mxTBWidth->set_menu_item_active(SELECTWIDTH, !mxTBWidth->get_menu_item_active(SELECTWIDTH));
}

void LinePropertyPanelBase::EndLineWidthPopup()
{
    mxTBWidth->set_menu_item_active(SELECTWIDTH, false);
}

IMPL_LINK_NOARG( LinePropertyPanelBase, ChangeTransparentHdl, weld::MetricSpinButton&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(mxMFTransparent->get_value(FieldUnit::PERCENT));
    XLineTransparenceItem aItem( nVal );

    setLineTransparency(aItem);
}

void LinePropertyPanelBase::SetWidthIcon(int n)
{
    if (n == 0)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGNone);
    else
    {
        Graphic aGraf(maIMGWidthIcon[n-1]);
        mxTBWidth->set_item_image(SELECTWIDTH, aGraf.GetXGraphic());
    }
}

void LinePropertyPanelBase::SetWidthIcon()
{
    if(!mbWidthValuable)
    {
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGNone);
        return;
    }

    tools::Long nVal = OutputDevice::LogicToLogic(mnWidthCoreValue * 10, meMapUnit, MapUnit::MapPoint);

    Graphic aGraf;
    if(nVal <= 6)
        aGraf = Graphic(maIMGWidthIcon[0]);
    else if (nVal <= 9)
        aGraf = Graphic(maIMGWidthIcon[1]);
    else if (nVal <= 12)
        aGraf = Graphic(maIMGWidthIcon[2]);
    else if (nVal <= 19)
        aGraf = Graphic(maIMGWidthIcon[3]);
    else if (nVal <= 26)
        aGraf = Graphic(maIMGWidthIcon[4]);
    else if (nVal <= 37)
        aGraf = Graphic(maIMGWidthIcon[5]);
    else if (nVal <= 52)
        aGraf = Graphic(maIMGWidthIcon[6]);
    else
        aGraf = Graphic(maIMGWidthIcon[7]);
    mxTBWidth->set_item_image(SELECTWIDTH, aGraf.GetXGraphic());
}

void LinePropertyPanelBase::SetWidth(tools::Long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::ActivateControls()
{
    mxArrowHeadStyleTB->set_item_sensitive(u".uno:LineEndStyle"_ustr, !mbNoneLineStyle);

    mxArrowHeadStyleFT->set_visible(mbArrowSupported);
    mxArrowHeadStyleTB->set_item_visible(u".uno:LineEndStyle"_ustr, mbArrowSupported);
}

void LinePropertyPanelBase::setMapUnit(MapUnit eMapUnit)
{
    meMapUnit = eMapUnit;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::disableArrowHead()
{
    mbArrowSupported = false;
    ActivateControls();
}

void LinePropertyPanelBase::enableArrowHead()
{
    mbArrowSupported = true;
    ActivateControls();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
