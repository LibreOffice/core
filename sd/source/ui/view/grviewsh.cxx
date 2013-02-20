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


#include "GraphicViewShell.hxx"
#include "LayerTabBar.hxx"
#include "FrameView.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/scrbar.hxx>
#include <tools/helpers.hxx>

namespace sd {

static const int TABCONTROL_INITIAL_SIZE = 350;

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

GraphicViewShell::GraphicViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    FrameView* pFrameView)
    : DrawViewShell (
        pFrame,
        rViewShellBase,
        pParentWindow,
        PK_STANDARD,
        pFrameView)
{
    ConstructGraphicViewShell();
}

GraphicViewShell::~GraphicViewShell (void)
{
}




void GraphicViewShell::ConstructGraphicViewShell(void)
{
    meShellType = ST_DRAW;

    mpLayerTabBar.reset (new LayerTabBar(this,GetParentWindow()));
    mpLayerTabBar->SetSplitHdl(LINK(this,GraphicViewShell,TabBarSplitHandler));

    // pb: #i67363# no layer tabbar on preview mode
    if ( !GetObjectShell()->IsPreview() )
        mpLayerTabBar->Show();
}




void GraphicViewShell::ChangeEditMode (
    EditMode eMode,
    bool )
{
    // There is no page tab that could be shown instead of the layer tab.
    // Therefore we have it always visible regardless of what the caller
    // said. (We have to change the callers behaviour, of course.)
    DrawViewShell::ChangeEditMode (eMode, true);
}




void GraphicViewShell::ArrangeGUIElements (void)
{
    if (mpLayerTabBar.get()!=NULL && mpLayerTabBar->IsVisible())
    {
        Size aSize = mpLayerTabBar->GetSizePixel();
        const Size aFrameSize (
            GetViewFrame()->GetWindow().GetOutputSizePixel());

        if (aSize.Width() == 0)
        {
            if (mpFrameView->GetTabCtrlPercent() == 0.0)
                aSize.Width() = TABCONTROL_INITIAL_SIZE;
            else
                aSize.Width() = FRound(aFrameSize.Width()
                    * mpFrameView->GetTabCtrlPercent());
        }
        aSize.Height() = GetParentWindow()->GetSettings().GetStyleSettings()
            .GetScrollBarSize();

        Point aPos (0, maViewSize.Height() - aSize.Height());

        mpLayerTabBar->SetPosSizePixel (aPos, aSize);

        if (aFrameSize.Width() > 0)
            mpFrameView->SetTabCtrlPercent (
                (double) maTabControl.GetSizePixel().Width()
                / aFrameSize.Width());
        else
            mpFrameView->SetTabCtrlPercent( 0.0 );
    }

    DrawViewShell::ArrangeGUIElements();
}




IMPL_LINK(GraphicViewShell, TabBarSplitHandler, TabBar*, pTabBar)
{
    const long int nMax = maViewSize.Width()
        - maScrBarWH.Width()
        - pTabBar->GetPosPixel().X();

    Size aTabSize = pTabBar->GetSizePixel();
    aTabSize.Width() = Min(pTabBar->GetSplitSize(), (long)(nMax-1));

    pTabBar->SetSizePixel (aTabSize);

    Point aPos = pTabBar->GetPosPixel();
    aPos.X() += aTabSize.Width();

    Size aScrSize (nMax - aTabSize.Width(), maScrBarWH.Height());
    mpHorizontalScrollBar->SetPosSizePixel(aPos, aScrSize);

    return 0;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
