/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <DefaultShapesPanel.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <utility>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/image.hxx>

using namespace svx::sidebar::commands;
using namespace css;
using namespace css::uno;
using namespace css::frame;

namespace svx::sidebar {

DefaultShapesPanel::DefaultShapesPanel (
    weld::Widget* pParent,
    css::uno::Reference<css::frame::XFrame> xFrame)
    : PanelLayout(pParent, u"DefaultShapesPanel"_ustr, u"svx/ui/defaultshapespanel.ui"_ustr)
    , mxLineArrowSet(m_xBuilder->weld_icon_view(u"LinesArrows"_ustr))
    , mxCurveSet(m_xBuilder->weld_icon_view(u"Curves"_ustr))
    , mxConnectorSet(m_xBuilder->weld_icon_view(u"Connectors"_ustr))
    , mxBasicShapeSet(m_xBuilder->weld_icon_view(u"BasicShapes"_ustr))
    , mxSymbolShapeSet(m_xBuilder->weld_icon_view(u"SymbolShapes"_ustr))
    , mxBlockArrowSet(m_xBuilder->weld_icon_view(u"BlockArrows"_ustr))
    , mxFlowchartSet(m_xBuilder->weld_icon_view(u"Flowcharts"_ustr))
    , mxCalloutSet(m_xBuilder->weld_icon_view(u"Callouts"_ustr))
    , mxStarSet(m_xBuilder->weld_icon_view(u"Stars"_ustr))
    , mx3DObjectSet(m_xBuilder->weld_icon_view(u"3DObjects"_ustr))
    , mxFrame(std::move(xFrame))
{
    Initialize();
    pParent->set_size_request(pParent->get_approximate_digit_width() * 20, -1);
    m_xContainer->set_size_request(m_xContainer->get_approximate_digit_width() * 25, -1);
}

std::unique_ptr<PanelLayout> DefaultShapesPanel::Create(
    weld::Widget* pParent,
    const Reference< XFrame >& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to DefaultShapesPanel::Create"_ustr, nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(u"no XFrame given to DefaultShapesPanel::Create"_ustr, nullptr, 1);

    return std::make_unique<DefaultShapesPanel>(pParent, rxFrame);
}

void DefaultShapesPanel::Initialize()
{
    maShapeGalleries = {
        { mxLineArrowSet.get(),   &gaLineShapes },
        { mxCurveSet.get(),       &gaCurveShapes },
        { mxConnectorSet.get(),   &gaConnectorShapes },
        { mxBasicShapeSet.get(),  &gaBasicShapes },
        { mxSymbolShapeSet.get(), &gaSymbolShapes },
        { mxBlockArrowSet.get(),  &gaBlockArrowShapes },
        { mxFlowchartSet.get(),   &gaFlowchartShapes },
        { mxCalloutSet.get(),     &gaCalloutShapes },
        { mxStarSet.get(),        &gaStarShapes },
        { mx3DObjectSet.get(),    &ga3DShapes }
    };
    populateShapes();
    for (const auto& rGallery : maShapeGalleries)
        rGallery.first->connect_item_activated(LINK(this, DefaultShapesPanel, ShapeSelectHdl));
}

DefaultShapesPanel::~DefaultShapesPanel()
{
    maShapeGalleries.clear();
    mxLineArrowSet.reset();
    mxCurveSet.reset();
    mxConnectorSet.reset();
    mxBasicShapeSet.reset();
    mxSymbolShapeSet.reset();
    mxBlockArrowSet.reset();
    mxFlowchartSet.reset();
    mxCalloutSet.reset();
    mxStarSet.reset();
    mx3DObjectSet.reset();
}

IMPL_LINK(DefaultShapesPanel, ShapeSelectHdl, weld::IconView&, rIconView, bool)
{
    const OUString sCommand = rIconView.get_selected_id();

    for (const auto& rGallery : maShapeGalleries)
    {
        if (rGallery.first != &rIconView)
            rGallery.first->unselect_all();
    }

    if (sCommand.isEmpty())
        return false;

    comphelper::dispatchCommand(sCommand, {});
    return true;
}

void DefaultShapesPanel::populateShapes()
{
    const OUString sModuleId = vcl::CommandInfoProvider::GetModuleIdentifier(mxFrame);

    for (const auto& rGallery : maShapeGalleries)
    {
        weld::IconView& rIconView = *rGallery.first;

        rIconView.freeze();
        for (const auto& rShape : *rGallery.second)
        {
            const OUString& rCommand = rShape.second;
            auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rCommand, sModuleId);
            const OUString sLabel = vcl::CommandInfoProvider::GetTooltipForCommand(
                rCommand, aProperties, mxFrame);
            Bitmap aShapeBitmap
                = vcl::CommandInfoProvider::GetImageForCommand(rCommand, mxFrame).GetBitmap();

            // The command is the item id, so the handler dispatches what was activated
            // without having to map an index back onto the gallery it came from.
            rIconView.append(rCommand, sLabel, &aShapeBitmap);
        }
        rIconView.thaw();
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
