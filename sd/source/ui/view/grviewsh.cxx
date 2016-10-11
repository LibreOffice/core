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
#include <vcl/settings.hxx>

#include <tools/helpers.hxx>

namespace sd {

GraphicViewShell::GraphicViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    vcl::Window* pParentWindow,
    FrameView* pFrameView)
    : DrawViewShell (
        pFrame,
        rViewShellBase,
        pParentWindow,
        PageKind::Standard,
        pFrameView)
{
    ConstructGraphicViewShell();
}

GraphicViewShell::~GraphicViewShell()
{
}

void GraphicViewShell::ConstructGraphicViewShell()
{
    meShellType = ST_DRAW;

    mpLayerTabBar.reset (VclPtr<LayerTabBar>::Create(this, GetParentWindow()));

    // #i67363# no layer tabbar in preview mode
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

void GraphicViewShell::ArrangeGUIElements()
{
    if (mpLayerTabBar.get()!=nullptr && mpLayerTabBar->IsVisible())
    {
        Size aSize = mpLayerTabBar->GetSizePixel();
        const Size aFrameSize (GetViewFrame()->GetWindow().GetOutputSizePixel());

        aSize.Height() = GetParentWindow()->GetFont().GetFontHeight() + 4;
        aSize.Width() = aFrameSize.Width();

        Point aPos (0, maViewSize.Height() - aSize.Height());

        mpLayerTabBar->SetPosSizePixel (aPos, aSize);
    }

    DrawViewShell::ArrangeGUIElements();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
