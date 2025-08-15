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

#include <com/sun/star/drawing/FillStyle.hpp>

#include <memory>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>

#include "ChartThemeControl.hxx"
#include <svx/ChartThemeType.hxx>

#include <vcl/event.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>

using namespace css;

namespace chart::sidebar
{
ChartThemeControl::ChartThemeControl(const uno::Reference<uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

ChartThemeControl::~ChartThemeControl() = default;

void ChartThemeControl::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    svt::PopupWindowController::initialize(rArguments);

    if (m_pToolbar)
    {
        mxPopoverContainer = std::make_unique<ToolbarPopupContainer>(m_pToolbar);
        m_pToolbar->set_item_popover(m_aCommandURL, mxPopoverContainer->getTopLevel());
        m_pToolbar->set_item_sensitive(m_aCommandURL, true);
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
    {
        pToolBox->SetItemBits(nId, pToolBox->GetItemBits(nId) | ToolBoxItemBits::DROPDOWNONLY);
        pToolBox->EnableItem(nId, true);
    }
}

void ChartThemeControl::execute(sal_Int16)
{
    if (m_pToolbar)
    {
        // Toggle the popup also when toolbutton is activated
        m_pToolbar->set_menu_item_active(m_aCommandURL,
                                         !m_pToolbar->get_menu_item_active(m_aCommandURL));
    }
    else
    {
        // Open the popup also when Enter key is pressed.
        createPopupWindow();
    }
}

void ChartThemeControl::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
        return;

    if (rEvent.FeatureURL.Complete == m_aCommandURL)
    {
        updateStatus();
    }
}

void ChartThemeControl::updateStatus(bool bForce)
{
    if (!mpHandler)
        return;
    // Todo: render something to the dropdown image
    if (bForce) // || changed ??
    {
        /*
        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
            return;

        auto pDev = VclPtr<VirtualDevice>::Create();
        // render the actual chart into pDev
        auto aSelItemImg(pDev->GetBitmapEx(Point(), pDev->GetOutputSizePixel()));
        if (m_pToolbar)
        {
            m_pToolbar->set_item_image(m_aCommandURL, Graphic(aSelItemImg).GetXGraphic());
        }
        else
        {
            pToolBox->SetItemImage(nId, Image(aSelItemImg));
        }
        */
    }
}
void ChartThemeControl::setThemeHandler(std::shared_ptr<IThemeHandler> rThemeHandler)
{
    if (!mpHandler)
    {
        mpHandler = rThemeHandler;
        updateStatus(true);
    }
}

std::unique_ptr<WeldToolbarPopup> ChartThemeControl::weldPopupWindow()
{
    return std::make_unique<ChartThemePopup>(this, m_pToolbar);
}

VclPtr<vcl::Window> ChartThemeControl::createVclPopupWindow(vcl::Window* pParent)
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(
        getFrameInterface(), pParent,
        std::make_unique<ChartThemePopup>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString ChartThemeControl::getImplementationName()
{
    return "com.sun.star.comp.chart2.ChartThemeControl";
}

uno::Sequence<OUString> ChartThemeControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_chart2_ChartThemeControl_get_implementation(uno::XComponentContext* rContext,
                                                              uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new ChartThemeControl(rContext));
}

ChartThemePopup::ChartThemePopup(ChartThemeControl* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent,
                       "modules/schart/ui/chartthemepopup.ui", "ThemeWindow")
    , mxControl(pControl)
    , mxThemesIconView(m_xBuilder->weld_icon_view(u"themeview"_ustr))
{
    mxThemesIconView->connect_item_activated(LINK(this, ChartThemePopup, ThemeSelectedHdl));
    mxThemesIconView->set_item_width(ChartThemeThumbSizeX);

    int nThemeCount = ChartThemesType::getInstance().getThemesCount();
    for (int i = 0; i < nThemeCount; i++)
    {
        ScopedVclPtr<VirtualDevice> device1 = makeImage(i);
        OUString sId = OUString::number(i);
        OUString sLayoutName = OUString::number(i);
        mxThemesIconView->insert(i, &sLayoutName, &sId, device1, nullptr);

        device1.disposeAndClear();
    }
}

ChartThemePopup::~ChartThemePopup() { mxThemesIconView.reset(); }

VclPtr<VirtualDevice> ChartThemePopup::makeImage(int nIndex)
{
    VclPtr<VirtualDevice> device1 = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device1->SetOutputSizePixel(Size(ChartThemeThumbSizeX, ChartThemeThumbSizeY));

    VclPtr<VirtualDevice> pDefaultOut = mxControl->mpHandler->makePictureFromThemedChart(nIndex);

    if (pDefaultOut)
    {
        BitmapEx aBmpEx = pDefaultOut->GetBitmapEx(Point(), pDefaultOut->GetOutputSizePixel());
        aBmpEx.Scale(Size(ChartThemeThumbSizeX, ChartThemeThumbSizeY), BmpScaleFlag::Fast);

        device1->DrawBitmapEx(Point(0, 0), aBmpEx);
        pDefaultOut.disposeAndClear();
    }
    return device1;
}

void ChartThemePopup::GrabFocus() {}

IMPL_LINK_NOARG(ChartThemePopup, ThemeSelectedHdl, weld::IconView&, bool)
{
    OUString sId = mxThemesIconView->get_selected_id();

    if (sId.isEmpty())
        return false;
    int nIndex = sId.toInt32();

    mxControl->mpHandler->select(nIndex);

    return true;
}

} // end namespace chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
