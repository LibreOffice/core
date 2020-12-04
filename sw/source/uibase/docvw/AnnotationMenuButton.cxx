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

#include <AnnotationWin.hxx>
#include <strings.hrc>

#include <unotools/useroptions.hxx>

#if 0

#include <vcl/menu.hxx>
#include <vcl/decoview.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#endif
#include <vcl/event.hxx>

#include <cmdid.h>

#include <swtypes.hxx>

namespace sw::annotation {

#if 0

static Color ColorFromAlphaColor(const sal_uInt8 aTransparency, const Color& aFront, const Color& aBack)
{
    return Color(sal_uInt8(aFront.GetRed()   * aTransparency / 255.0 + aBack.GetRed()   * (1 - aTransparency / 255.0)),
                 sal_uInt8(aFront.GetGreen() * aTransparency / 255.0 + aBack.GetGreen() * (1 - aTransparency / 255.0)),
                 sal_uInt8(aFront.GetBlue()  * aTransparency / 255.0 + aBack.GetBlue()  * (1 - aTransparency / 255.0)));
}

#endif

IMPL_LINK(SwAnnotationWin, SelectHdl, const OString&, rIdent, void)
{
    if (rIdent.isEmpty())
        return;

    // tdf#136682 ensure this is the currently active sidebar win so the command
    // operates in an active sidebar context
    bool bSwitchedFocus = SetActiveSidebarWin();

    if (rIdent == "reply")
        ExecuteCommand(FN_REPLY);
    if (rIdent == "resolve" || rIdent == "unresolve")
        ExecuteCommand(FN_RESOLVE_NOTE);
    else if (rIdent == "resolvethread" || rIdent == "unresolvethread")
        ExecuteCommand(FN_RESOLVE_NOTE_THREAD);
    else if (rIdent == "delete")
        ExecuteCommand(FN_DELETE_COMMENT);
    else if (rIdent == "deletethread")
        ExecuteCommand(FN_DELETE_COMMENT_THREAD);
    else if (rIdent == "deleteby")
        ExecuteCommand(FN_DELETE_NOTE_AUTHOR);
    else if (rIdent == "deleteall")
        ExecuteCommand(FN_DELETE_ALL_NOTES);
    else if (rIdent == "formatall")
        ExecuteCommand(FN_FORMAT_ALL_NOTES);

    if (bSwitchedFocus)
        UnsetActiveSidebarWin();
    GrabFocusToDocument();
}

IMPL_LINK_NOARG(SwAnnotationWin, ToggleHdl, weld::ToggleButton&, void)
{
    if (!mxMenuButton->get_active())
        return;

    bool bReadOnly = IsReadOnly();
    if (bReadOnly)
    {
        mxMenuButton->set_item_sensitive("reply", false);
        mxMenuButton->set_item_sensitive("resolve", false);
        mxMenuButton->set_item_sensitive("unresolve", false);
        mxMenuButton->set_item_sensitive("resolvethread", false);
        mxMenuButton->set_item_sensitive("unresolvethread", false);
        mxMenuButton->set_item_sensitive("delete", false );
    }
    else
    {
        mxMenuButton->set_item_sensitive("resolve", !IsResolved());
        mxMenuButton->set_item_sensitive("unresolve", IsResolved());
        mxMenuButton->set_item_sensitive("resolvethread", !IsThreadResolved());
        mxMenuButton->set_item_sensitive("unresolvethread", IsThreadResolved());
        mxMenuButton->set_item_sensitive("delete", !IsProtected());
    }

    mxMenuButton->set_item_sensitive("deletethread", !bReadOnly);
    mxMenuButton->set_item_sensitive("deleteby", !bReadOnly);
    mxMenuButton->set_item_sensitive("deleteall", !bReadOnly);
    mxMenuButton->set_item_sensitive("formatall", !bReadOnly);

    if (IsProtected())
    {
        mxMenuButton->set_item_sensitive("reply", false);
    }
    else
    {
        SvtUserOptions aUserOpt;
        OUString sAuthor;
        if ((sAuthor = aUserOpt.GetFullName()).isEmpty())
        {
            if ((sAuthor = aUserOpt.GetID()).isEmpty())
            {
                sAuthor = SwResId(STR_REDLINE_UNKNOWN_AUTHOR);
            }
        }
        // do not allow to reply to ourself and no answer possible if this note is in a protected section
        mxMenuButton->set_item_sensitive("reply", sAuthor != GetAuthor());
    }
}

#if 0

void AnnotationMenuButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    bool bHighContrast = rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode();

    if (bHighContrast)
        rRenderContext.SetFillColor(COL_BLACK);
    else
        rRenderContext.SetFillColor(mrSidebarWin.ColorDark());
    rRenderContext.SetLineColor();
    const tools::Rectangle aRect(tools::Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));
    rRenderContext.DrawRect(aRect);

    if (bHighContrast)
    {
        //draw rect around button
        rRenderContext.SetFillColor(COL_BLACK);
        rRenderContext.SetLineColor(COL_WHITE);
    }
    else
    {
        //draw button
        Gradient aGradient;
        if (IsMouseOver())
            aGradient = Gradient(GradientStyle::Linear,
                                 ColorFromAlphaColor(80, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()),
                                 ColorFromAlphaColor(15, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()));
        else
            aGradient = Gradient(GradientStyle::Linear,
                                 ColorFromAlphaColor(15, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()),
                                 ColorFromAlphaColor(80, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()));
        rRenderContext.DrawGradient(aRect, aGradient);

        //draw rect around button
        rRenderContext.SetFillColor();
        rRenderContext.SetLineColor(ColorFromAlphaColor(90, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()));
    }
    rRenderContext.DrawRect(aRect);

    tools::Rectangle aSymbolRect(aRect);
    // 25% distance to the left and right button border
    const tools::Long nBorderDistanceLeftAndRight = ((aSymbolRect.GetWidth() * 250) + 500) / 1000;
    aSymbolRect.AdjustLeft(nBorderDistanceLeftAndRight );
    aSymbolRect.AdjustRight( -nBorderDistanceLeftAndRight );
    // 40% distance to the top button border
    const tools::Long nBorderDistanceTop = ((aSymbolRect.GetHeight() * 400) + 500) / 1000;
    aSymbolRect.AdjustTop(nBorderDistanceTop );
    // 15% distance to the bottom button border
    const tools::Long nBorderDistanceBottom = ((aSymbolRect.GetHeight() * 150) + 500) / 1000;
    aSymbolRect.AdjustBottom( -nBorderDistanceBottom );
    DecorationView aDecoView(&rRenderContext);
    aDecoView.DrawSymbol(aSymbolRect, SymbolType::SPIN_DOWN, (bHighContrast ? COL_WHITE : COL_BLACK));
}

#endif

IMPL_LINK(SwAnnotationWin, KeyInputHdl, const KeyEvent&, rKeyEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    if (rKeyCode.GetCode() == KEY_TAB)
    {
        ActivatePostIt();
        GrabFocus();
        return true;
    }
    return false;
}

} // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
