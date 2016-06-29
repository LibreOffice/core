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
#include "ViewShellBase.hxx"
#include <app.hrc>
#include <comphelper/dispatchcommand.hxx>
#include "ShapesUtil.cxx"
#include <vcl/outdev.hxx>

using ::rtl::OUString;

namespace sd { namespace sidebar {


ShapesPanel::ShapesPanel (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "ShapesPanel", "modules/sdraw/ui/defaultshapespanel.ui", rxFrame),
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

ShapesPanel::~ShapesPanel()
{
    disposeOnce();
}

void ShapesPanel::Initialize()
{
    populateShapes();
    mpLineArrowSet->SetSelectHdl(LINK(this, ShapesPanel, LineSelectHdl));
    mpCurveSet->SetSelectHdl(LINK(this, ShapesPanel, CurveSelectHdl));
    mpConnectorSet->SetSelectHdl(LINK(this, ShapesPanel, ConnectorSelectHdl));
    mpBasicShapeSet->SetSelectHdl(LINK(this, ShapesPanel, BasicShapeSelectHdl));
    mpSymbolShapeSet->SetSelectHdl(LINK(this, ShapesPanel, SymbolSelectHdl));
    mpBlockArrowSet->SetSelectHdl(LINK(this, ShapesPanel, BlockArrowSelectHdl));
    mpFlowchartSet->SetSelectHdl(LINK(this, ShapesPanel, FlowchartSelectHdl));
    mpCalloutSet->SetSelectHdl(LINK(this, ShapesPanel, CalloutSelectHdl));
    mpStarSet->SetSelectHdl(LINK(this, ShapesPanel, StarSelectHdl));
    mp3DObjectSet->SetSelectHdl(LINK(this, ShapesPanel, T3DObjectSelectHdl));
}

void ShapesPanel::dispose()
{
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

IMPL_LINK_NOARG_TYPED(ShapesPanel, LineSelectHdl, ValueSet*, void)
{
    int aSelection = mpLineArrowSet->GetSelectItemId();
    comphelper::dispatchCommand(aLineShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, CurveSelectHdl, ValueSet*, void)
{
    int aSelection = mpCurveSet->GetSelectItemId();
    comphelper::dispatchCommand(aCurveShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, ConnectorSelectHdl, ValueSet*, void)
{
    int aSelection = mpConnectorSet->GetSelectItemId();
    comphelper::dispatchCommand(aConnectorShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, BasicShapeSelectHdl, ValueSet*, void)
{
    int aSelection = mpBasicShapeSet->GetSelectItemId();
    comphelper::dispatchCommand(aBasicShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, SymbolSelectHdl, ValueSet*, void)
{
    int aSelection = mpSymbolShapeSet->GetSelectItemId();
    comphelper::dispatchCommand(aSymbolShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, BlockArrowSelectHdl, ValueSet*, void)
{
    int aSelection = mpBlockArrowSet->GetSelectItemId();
    comphelper::dispatchCommand(aBlockArrowShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, FlowchartSelectHdl, ValueSet*, void)
{
    int aSelection = mpFlowchartSet->GetSelectItemId();
    comphelper::dispatchCommand(aFlowchartShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, CalloutSelectHdl, ValueSet*, void)
{
    int aSelection = mpCalloutSet->GetSelectItemId();
    comphelper::dispatchCommand(aCalloutShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, StarSelectHdl, ValueSet*, void)
{
    int aSelection = mpStarSet->GetSelectItemId();
    comphelper::dispatchCommand(aStarShapes[aSelection], {});
}

IMPL_LINK_NOARG_TYPED(ShapesPanel, T3DObjectSelectHdl, ValueSet*, void)
{
    int aSelection = mp3DObjectSet->GetSelectItemId();
    comphelper::dispatchCommand(a3DShapes[aSelection], {});
}

void ShapesPanel::populateShapes()
{
    OUString sSlotStr, sLabel;
    Image aSlotImage;
    mpLineArrowSet->SetColCount(6);
    mpCurveSet->SetColCount(6);
    mpConnectorSet->SetColCount(6);
    mpBasicShapeSet->SetColCount(6);
    mpSymbolShapeSet->SetColCount(6);
    mpBlockArrowSet->SetColCount(6);
    mpFlowchartSet->SetColCount(6);
    mpCalloutSet->SetColCount(6);
    mpStarSet->SetColCount(6);
    mp3DObjectSet->SetColCount(6);

    for(sal_uInt16 i = 0; i < aLineShapes.size(); i++)
    {
        sSlotStr = aLineShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpLineArrowSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aCurveShapes.size(); i++)
    {
        sSlotStr = aCurveShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpCurveSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aConnectorShapes.size(); i++)
    {
        sSlotStr = aConnectorShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpConnectorSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aBasicShapes.size(); i++)
    {
        sSlotStr = aBasicShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpBasicShapeSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aSymbolShapes.size(); i++)
    {
        sSlotStr = aSymbolShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpSymbolShapeSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aBlockArrowShapes.size(); i++)
    {
        sSlotStr = aBlockArrowShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpBlockArrowSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aFlowchartShapes.size(); i++)
    {
        sSlotStr = aFlowchartShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpFlowchartSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aCalloutShapes.size(); i++)
    {
        sSlotStr = aCalloutShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpCalloutSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < aStarShapes.size(); i++)
    {
        sSlotStr = aStarShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mpStarSet->InsertItem(i, aSlotImage, sLabel);
    }

    for(sal_uInt16 i = 0; i < a3DShapes.size(); i++)
    {
        sSlotStr = a3DShapes[i];
        aSlotImage = ::GetImage(mxFrame, sSlotStr, false);
        sLabel = ImplRetrieveLabelFromCommand(mxFrame, sSlotStr);
        mp3DObjectSet->InsertItem(i, aSlotImage, sLabel);
    }
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */