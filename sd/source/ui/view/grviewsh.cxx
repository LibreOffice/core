/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "GraphicViewShell.hxx"
#include "LayerTabBar.hxx"
#include "FrameView.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/salbtype.hxx>     // FRound

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
    // Therefore we have it allways visible regardless of what the caller
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
