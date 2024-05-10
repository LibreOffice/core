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
#include <DefaultShapesPanel.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <utility>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

namespace svx::sidebar {

DefaultShapesPanel::DefaultShapesPanel (
    weld::Widget* pParent,
    css::uno::Reference<css::frame::XFrame> xFrame)
    : PanelLayout(pParent, u"DefaultShapesPanel"_ustr, u"svx/ui/defaultshapespanel.ui"_ustr)
    , mxLineArrowSet(new ValueSet(nullptr))
    , mxLineArrowSetWin(new weld::CustomWeld(*m_xBuilder, u"LinesArrows"_ustr, *mxLineArrowSet))
    , mxCurveSet(new ValueSet(nullptr))
    , mxCurveSetWin(new weld::CustomWeld(*m_xBuilder, u"Curves"_ustr, *mxCurveSet))
    , mxConnectorSet(new ValueSet(nullptr))
    , mxConnectorSetWin(new weld::CustomWeld(*m_xBuilder, u"Connectors"_ustr, *mxConnectorSet))
    , mxBasicShapeSet(new ValueSet(nullptr))
    , mxBasicShapeSetWin(new weld::CustomWeld(*m_xBuilder, u"BasicShapes"_ustr, *mxBasicShapeSet))
    , mxSymbolShapeSet(new ValueSet(nullptr))
    , mxSymbolShapeSetWin(new weld::CustomWeld(*m_xBuilder, u"SymbolShapes"_ustr, *mxSymbolShapeSet))
    , mxBlockArrowSet(new ValueSet(nullptr))
    , mxBlockArrowSetWin(new weld::CustomWeld(*m_xBuilder, u"BlockArrows"_ustr, *mxBlockArrowSet))
    , mxFlowchartSet(new ValueSet(nullptr))
    , mxFlowchartSetWin(new weld::CustomWeld(*m_xBuilder, u"Flowcharts"_ustr, *mxFlowchartSet))
    , mxCalloutSet(new ValueSet(nullptr))
    , mxCalloutSetWin(new weld::CustomWeld(*m_xBuilder, u"Callouts"_ustr, *mxCalloutSet))
    , mxStarSet(new ValueSet(nullptr))
    , mxStarSetWin(new weld::CustomWeld(*m_xBuilder, u"Stars"_ustr, *mxStarSet))
    , mx3DObjectSet(new ValueSet(nullptr))
    , mx3DObjectSetWin(new weld::CustomWeld(*m_xBuilder, u"3DObjects"_ustr, *mx3DObjectSet))
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
    mpShapesSetMap = decltype(mpShapesSetMap){
        { mxLineArrowSet.get(),   mpLineShapes },
        { mxCurveSet.get(),       mpCurveShapes },
        { mxConnectorSet.get(),   mpConnectorShapes },
        { mxBasicShapeSet.get(),  mpBasicShapes },
        { mxSymbolShapeSet.get(), mpSymbolShapes },
        { mxBlockArrowSet.get(),  mpBlockArrowShapes },
        { mxFlowchartSet.get(),   mpFlowchartShapes },
        { mxCalloutSet.get(),     mpCalloutShapes },
        { mxStarSet.get(),        mpStarShapes },
        { mx3DObjectSet.get(),    mp3DShapes }
    };
    populateShapes();
    for(auto& aSetMap: mpShapesSetMap)
    {
        aSetMap.first->SetColor(Application::GetSettings().GetStyleSettings().GetDialogColor());
        aSetMap.first->SetSelectHdl(LINK(this, DefaultShapesPanel, ShapeSelectHdl));
    }
}

DefaultShapesPanel::~DefaultShapesPanel()
{
    mpShapesSetMap.clear();
    mxLineArrowSetWin.reset();
    mxLineArrowSet.reset();
    mxCurveSetWin.reset();
    mxCurveSet.reset();
    mxConnectorSetWin.reset();
    mxConnectorSet.reset();
    mxBasicShapeSetWin.reset();
    mxBasicShapeSet.reset();
    mxSymbolShapeSetWin.reset();
    mxSymbolShapeSet.reset();
    mxBlockArrowSetWin.reset();
    mxBlockArrowSet.reset();
    mxFlowchartSetWin.reset();
    mxFlowchartSet.reset();
    mxCalloutSetWin.reset();
    mxCalloutSet.reset();
    mxStarSetWin.reset();
    mxStarSet.reset();
    mx3DObjectSetWin.reset();
    mx3DObjectSet.reset();
}

IMPL_LINK(DefaultShapesPanel, ShapeSelectHdl, ValueSet*, rValueSet, void)
{
    for(auto& aSetMap : mpShapesSetMap)
    {
        if(rValueSet == aSetMap.first)
        {
            sal_uInt16 nSelectionId = aSetMap.first->GetSelectedItemId();
            comphelper::dispatchCommand(aSetMap.second[nSelectionId - 1], {});
        }
        else
            aSetMap.first->SetNoSelection();
    }
}

void DefaultShapesPanel::populateShapes()
{
    OUString sSlotStr, sLabel;
    Image aSlotImage;
    for(auto& aSet : mpShapesSetMap)
    {
        aSet.first->SetColCount(6);
        for(std::map<sal_uInt16, OUString>::size_type i = 0; i < aSet.second.size(); i++)
        {
            sSlotStr = aSet.second[i];
            aSlotImage = vcl::CommandInfoProvider::GetImageForCommand(sSlotStr, mxFrame);
            auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(sSlotStr,
                vcl::CommandInfoProvider::GetModuleIdentifier(mxFrame));
            sLabel = vcl::CommandInfoProvider::GetTooltipForCommand(sSlotStr, aProperties, mxFrame);
            sal_uInt16 nSelectionId = i + 1; // tdf#142767 id 0 is reserved for nothing-selected
            aSet.first->InsertItem(nSelectionId, aSlotImage, sLabel);
        }
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
