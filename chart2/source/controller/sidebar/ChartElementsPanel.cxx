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

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include "ChartElementsPanel.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

using namespace css;
using namespace css::uno;
using ::sfx2::sidebar::Theme;

namespace chart { namespace sidebar {

ChartElementsPanel::ChartElementsPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
  : PanelLayout(pParent, "ChartElementsPanel", "modules/schart/ui/sidebarelements.ui", rxFrame),
    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings)
{
    get(mpCBTitle,  "checkbutton_title");
    get(mpCBSubtitle,  "checkbutton_subtitle");
    get(mpCBXAxis,  "checkbutton_x_axis");
    get(mpCBXAxisTitle,  "checkbutton_x_axis_title");
    get(mpCBYAxis,  "checkbutton_y_axis");
    get(mpCBYAxisTitle,  "checkbutton_y_axis_title");
    get(mpCBZAxis,  "checkbutton_z_axis");
    get(mpCBZAxisTitle,  "checkbutton_z_axis_title");
    get(mpCB2ndXAxis,  "checkbutton_2nd_x_axis");
    get(mpCB2ndXAxisTitle,  "checkbutton_2nd_x_axis_title");
    get(mpCB2ndYAxis,  "checkbutton_2nd_y_axis");
    get(mpCB2ndYAxisTitle,  "checkbutton_2nd_y_axis_title");
    get(mpCBLegend,  "checkbutton_legend");
    get(mpCBGridVertical,  "checkbutton_gridline_vertical");
    get(mpCBGridHorizontal,  "checkbutton_gridline_horizontal");
    get(mpCBShowLabel,  "checkbutton_label");
    get(mpCBTrendline,  "checkbutton_trendline");

    Initialize();
}

ChartElementsPanel::~ChartElementsPanel()
{
    disposeOnce();
}

void ChartElementsPanel::dispose()
{
    mpCBTitle.clear();
    mpCBSubtitle.clear();
    mpCBXAxis.clear();
    mpCBXAxisTitle.clear();
    mpCBYAxis.clear();
    mpCBYAxisTitle.clear();
    mpCBZAxis.clear();
    mpCBZAxisTitle.clear();
    mpCB2ndXAxis.clear();
    mpCB2ndXAxisTitle.clear();
    mpCB2ndYAxis.clear();
    mpCB2ndYAxisTitle.clear();
    mpCBLegend.clear();
    mpCBGridVertical.clear();
    mpCBGridHorizontal.clear();
    mpCBShowLabel.clear();
    mpCBTrendline.clear();

    PanelLayout::dispose();
}

void ChartElementsPanel::Initialize()
{
}

VclPtr<vcl::Window> ChartElementsPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to ChartElementsPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartElementsPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to ChartElementsPanel::Create", NULL, 2);

    return  VclPtr<ChartElementsPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void ChartElementsPanel::DataChanged(
    const DataChangedEvent& )
{
}

void ChartElementsPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if(maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
}

void ChartElementsPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool )
{
    switch(nSID)
    {
        default:
        ;
    }
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
