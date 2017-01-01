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
#include "DefaultShapesPanel.hxx"

#include <sfx2/imagemgr.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <vcl/outdev.hxx>
#include <vcl/commandinfoprovider.hxx>

using ::rtl::OUString;

namespace svx { namespace sidebar {

DefaultShapesPanel::DefaultShapesPanel (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "DefaultShapesPanel", "svx/ui/defaultshapespanel.ui", rxFrame),
    SvxShapeCommandsMap(),
    mxFrame(rxFrame)
{
    get(mpScrollWindow, "scrolledwindow1");
    get(mpLineArrowSet, "LinesArrows");
    get(mpCurveSet, "Curves");
    get(mpConnectorSet, "Connectors");
    get(mpBasicShapeSet, "BasicShapes");
    get(mpSymbolShapeSet, "SymbolShapes");
    get(mpBlockArrowSet, "BlockArrows");
    get(mpFlowchartSet, "Flowcharts");
    get(mpCalloutSet, "Callouts");
    get(mpStarSet, "Stars");
    get(mp3DObjectSet, "3DObjects");
    Initialize();
}

VclPtr< vcl::Window > DefaultShapesPanel::Create(
    vcl::Window* pParent,
    const Reference< XFrame >& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to DefaultShapesPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to DefaultShapesPanel::Create", nullptr, 1);

    return VclPtr<DefaultShapesPanel>::Create(
                pParent,
                rxFrame);
}

DefaultShapesPanel::~DefaultShapesPanel()
{
    disposeOnce();
}

void DefaultShapesPanel::Initialize()
{
    mpShapesSetMap = decltype(mpShapesSetMap){
        { mpLineArrowSet,   mpLineShapes },
        { mpCurveSet,       mpCurveShapes },
        { mpConnectorSet,   mpConnectorShapes },
        { mpBasicShapeSet,  mpBasicShapes },
        { mpSymbolShapeSet, mpSymbolShapes },
        { mpBlockArrowSet,  mpBlockArrowShapes },
        { mpFlowchartSet,   mpFlowchartShapes },
        { mpCalloutSet,     mpCalloutShapes },
        { mpStarSet,        mpStarShapes },
        { mp3DObjectSet,    mp3DShapes }
    };
    populateShapes();
    for(auto& aSetMap: mpShapesSetMap)
    {
        aSetMap.first->SetColor(GetSettings().GetStyleSettings().GetDialogColor());
        aSetMap.first->SetSelectHdl(LINK(this, DefaultShapesPanel, ShapeSelectHdl));
    }
}

void DefaultShapesPanel::dispose()
{
    mpShapesSetMap.clear();
    mpScrollWindow.clear();
    mpLineArrowSet.clear();
    mpCurveSet.clear();
    mpConnectorSet.clear();
    mpBasicShapeSet.clear();
    mpSymbolShapeSet.clear();
    mpBlockArrowSet.clear();
    mpFlowchartSet.clear();
    mpCalloutSet.clear();
    mpStarSet.clear();
    mp3DObjectSet.clear();
    PanelLayout::dispose();
}

IMPL_LINK(DefaultShapesPanel, ShapeSelectHdl, ValueSet*, rValueSet, void)
{
    for(auto& aSetMap : mpShapesSetMap)
    {
        if(rValueSet == aSetMap.first)
        {
            int aSelection = aSetMap.first->GetSelectItemId();
            comphelper::dispatchCommand(aSetMap.second[aSelection], {});
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
        for(std::map<sal_uInt16, rtl::OUString>::size_type i = 0; i < aSet.second.size(); i++)
        {
            sSlotStr = aSet.second[i];
            aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
            sLabel = vcl::CommandInfoProvider::GetTooltipForCommand(sSlotStr, mxFrame);
            aSet.first->InsertItem(i, aSlotImage, sLabel);
        }
    }
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
