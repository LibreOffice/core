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
#include <com/sun/star/drawing/LineStyle.hpp>
#include <sfx2/weldutils.hxx>
#include <svx/linectrl.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include <bitmaps.hlst>

using namespace css;
using namespace css::uno;

const char SELECTWIDTH[] = "SelectWidth";

namespace svx::sidebar {

// trigger disabling the arrows if the none line style is selected
class DisableArrowsWrapper
{
private:
    LinePropertyPanelBase& m_rPanel;

public:
    DisableArrowsWrapper(LinePropertyPanelBase& rPanel)
        : m_rPanel(rPanel)
    {
    }

    bool operator()(const OUString& rCommand, const css::uno::Any& rValue)
    {
        if (rCommand == ".uno:XLineStyle")
        {
            css::drawing::LineStyle eLineStyle(css::drawing::LineStyle_NONE);
            rValue >>= eLineStyle;
            m_rPanel.SetNoneLineStyle(eLineStyle == css::drawing::LineStyle_NONE);
        }
        return false;
    }
};

namespace
{
    SvxLineStyleToolBoxControl* getLineStyleToolBoxControl(const ToolbarUnoDispatcher& rToolBoxColor)
    {
        css::uno::Reference<css::frame::XToolbarController> xController = rToolBoxColor.GetControllerForCommand(".uno:XLineStyle");
        SvxLineStyleToolBoxControl* pToolBoxLineStyleControl = dynamic_cast<SvxLineStyleToolBoxControl*>(xController.get());
        return pToolBoxLineStyleControl;
    }
}


LinePropertyPanelBase::LinePropertyPanelBase(
    vcl::Window* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame)
:   PanelLayout(pParent, "LinePropertyPanel", "svx/ui/sidebarline.ui", rxFrame),
    mxTBColor(m_xBuilder->weld_toolbar("color")),
    mxColorDispatch(new ToolbarUnoDispatcher(*mxTBColor, *m_xBuilder, rxFrame)),
    mxLineStyleTB(m_xBuilder->weld_toolbar("linestyle")),
    mxLineStyleDispatch(new ToolbarUnoDispatcher(*mxLineStyleTB, *m_xBuilder, rxFrame)),
    mxFTWidth(m_xBuilder->weld_label("widthlabel")),
    mxTBWidth(m_xBuilder->weld_toolbar("width")),
    mxFTTransparency(m_xBuilder->weld_label("translabel")),
    mxMFTransparent(m_xBuilder->weld_metric_spin_button("linetransparency", FieldUnit::PERCENT)),
    mxFTEdgeStyle(m_xBuilder->weld_label("cornerlabel")),
    mxLBEdgeStyle(m_xBuilder->weld_combo_box("edgestyle")),
    mxFTCapStyle(m_xBuilder->weld_label("caplabel")),
    mxLBCapStyle(m_xBuilder->weld_combo_box("linecapstyle")),
    mxGridLineProps(m_xBuilder->weld_widget("lineproperties")),
    mxBoxArrowProps(m_xBuilder->weld_widget("arrowproperties")),
    mxLineWidthPopup(new LineWidthPopup(mxTBWidth.get(), *this)),
    mxDisableArrowsWrapper(new DisableArrowsWrapper(*this)),
    mnTrans(0),
    meMapUnit(MapUnit::MapMM),
    mnWidthCoreValue(0),
    maIMGNone(BMP_NONE_ICON),
    mbWidthValuable(true),
    mbArrowSupported(true),
    mbNoneLineStyle(false)
{
    Initialize();
}

LinePropertyPanelBase::~LinePropertyPanelBase()
{
    disposeOnce();
}

void LinePropertyPanelBase::dispose()
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
    mxFTEdgeStyle.reset();
    mxLBEdgeStyle.reset();
    mxFTCapStyle.reset();
    mxLBCapStyle.reset();
    mxGridLineProps.reset();
    mxBoxArrowProps.reset();

    PanelLayout::dispose();
}

void LinePropertyPanelBase::Initialize()
{
    mxTBWidth->set_item_popover(SELECTWIDTH, mxLineWidthPopup->getTopLevel());

    maIMGWidthIcon[0] = BMP_WIDTH1_ICON;
    maIMGWidthIcon[1] = BMP_WIDTH2_ICON;
    maIMGWidthIcon[2] = BMP_WIDTH3_ICON;
    maIMGWidthIcon[3] = BMP_WIDTH4_ICON;
    maIMGWidthIcon[4] = BMP_WIDTH5_ICON;
    maIMGWidthIcon[5] = BMP_WIDTH6_ICON;
    maIMGWidthIcon[6] = BMP_WIDTH7_ICON;
    maIMGWidthIcon[7] = BMP_WIDTH8_ICON;

    mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[0]);
    mxTBWidth->connect_clicked(LINK(this, LinePropertyPanelBase, ToolboxWidthSelectHdl));

    mxMFTransparent->connect_value_changed(LINK(this, LinePropertyPanelBase, ChangeTransparentHdl));

    mxLBEdgeStyle->connect_changed( LINK( this, LinePropertyPanelBase, ChangeEdgeStyleHdl ) );

    mxLBCapStyle->connect_changed( LINK( this, LinePropertyPanelBase, ChangeCapStyleHdl ) );

    SvxLineStyleToolBoxControl* pLineStyleControl = getLineStyleToolBoxControl(*mxLineStyleDispatch);
    pLineStyleControl->setLineStyleSelectFunction(*mxDisableArrowsWrapper);
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

void LinePropertyPanelBase::updateLineJoint(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxLBEdgeStyle->set_sensitive(false);
        mxFTEdgeStyle->set_sensitive(false);
    }
    else
    {
        mxLBEdgeStyle->set_sensitive(true);
        mxFTEdgeStyle->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineJointItem* pItem = dynamic_cast<const XLineJointItem*>(pState))
        {
            sal_Int32 nEntryPos(0);

            switch(pItem->GetValue())
            {
                case drawing::LineJoint_ROUND:
                {
                    nEntryPos = 1;
                    break;
                }
                case drawing::LineJoint_NONE:
                {
                    nEntryPos = 2;
                    break;
                }
                case drawing::LineJoint_MIDDLE:
                case drawing::LineJoint_MITER:
                {
                    nEntryPos = 3;
                    break;
                }
                case drawing::LineJoint_BEVEL:
                {
                    nEntryPos = 4;
                    break;
                }

                default:
                break;
            }

            if(nEntryPos)
            {
                mxLBEdgeStyle->set_active(nEntryPos - 1);
                return;
            }
        }
    }

    mxLBEdgeStyle->set_active(-1);
}

void LinePropertyPanelBase::updateLineCap(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxLBCapStyle->set_sensitive(false);
        mxFTCapStyle->set_sensitive(false);
    }
    else
    {
        mxLBCapStyle->set_sensitive(true);
        mxLBCapStyle->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineCapItem* pItem = dynamic_cast<const XLineCapItem*>(pState))
        {
            sal_Int32 nEntryPos(0);

            switch(pItem->GetValue())
            {
                case drawing::LineCap_BUTT:
                {
                    nEntryPos = 1;
                    break;
                }
                case drawing::LineCap_ROUND:
                {
                    nEntryPos = 2;
                    break;
                }
                case drawing::LineCap_SQUARE:
                {
                    nEntryPos = 3;
                    break;
                }

                default:
                break;
            }

            if(nEntryPos)
            {
                mxLBCapStyle->set_active(nEntryPos - 1);
                return;
            }
        }
    }

    mxLBCapStyle->set_active(-1);
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeEdgeStyleHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos(mxLBEdgeStyle->get_active());

    if (nPos == -1 || !mxLBEdgeStyle->get_value_changed_from_saved())
        return;

    std::unique_ptr<XLineJointItem> pItem;

    switch(nPos)
    {
        case 0: // rounded
        {
            pItem.reset(new XLineJointItem(drawing::LineJoint_ROUND));
            break;
        }
        case 1: // none
        {
            pItem.reset(new XLineJointItem(drawing::LineJoint_NONE));
            break;
        }
        case 2: // mitered
        {
            pItem.reset(new XLineJointItem(drawing::LineJoint_MITER));
            break;
        }
        case 3: // beveled
        {
            pItem.reset(new XLineJointItem(drawing::LineJoint_BEVEL));
            break;
        }
    }

    setLineJoint(pItem.get());
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeCapStyleHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos(mxLBCapStyle->get_active());

    if (!(nPos != -1 && mxLBCapStyle->get_value_changed_from_saved()))
        return;

    std::unique_ptr<XLineCapItem> pItem;

    switch(nPos)
    {
        case 0: // flat
        {
            pItem.reset(new XLineCapItem(drawing::LineCap_BUTT));
            break;
        }
        case 1: // round
        {
            pItem.reset(new XLineCapItem(drawing::LineCap_ROUND));
            break;
        }
        case 2: // square
        {
            pItem.reset(new XLineCapItem(drawing::LineCap_SQUARE));
            break;
        }
    }

    setLineCap(pItem.get());
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ToolboxWidthSelectHdl, const OString&, void)
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
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[n-1]);
}

void LinePropertyPanelBase::SetWidthIcon()
{
    if(!mbWidthValuable)
    {
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGNone);
        return;
    }

    long nVal = LogicToLogic(mnWidthCoreValue * 10, meMapUnit, MapUnit::MapPoint);

    if(nVal <= 6)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[0]);
    else if (nVal <= 9)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[1]);
    else if (nVal <= 12)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[2]);
    else if (nVal <= 19)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[3]);
    else if (nVal <= 26)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[4]);
    else if (nVal <= 37)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[5]);
    else if (nVal <= 52)
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[6]);
    else
        mxTBWidth->set_item_icon_name(SELECTWIDTH, maIMGWidthIcon[7]);

}

void LinePropertyPanelBase::SetWidth(long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::ActivateControls()
{
    mxGridLineProps->set_sensitive(!mbNoneLineStyle);
    mxBoxArrowProps->set_sensitive(!mbNoneLineStyle);
    mxLineStyleTB->set_item_visible(".uno:LineEndStyle", mbArrowSupported);
    mxLineStyleTB->set_item_sensitive(".uno:LineEndStyle", !mbNoneLineStyle);
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
