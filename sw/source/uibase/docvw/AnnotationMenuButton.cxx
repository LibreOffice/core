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

#include "AnnotationMenuButton.hxx"

#include <app.hrc>
#include <strings.hrc>

#include <unotools/useroptions.hxx>

#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/decoview.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>

#include <cmdid.h>
#include <AnnotationWin.hxx>
#include <swtypes.hxx>

namespace sw { namespace annotation {

static Color ColorFromAlphaColor(const sal_uInt8 aTransparency, const Color& aFront, const Color& aBack)
{
    return Color(sal_uInt8(aFront.GetRed()   * aTransparency / 255.0 + aBack.GetRed()   * (1 - aTransparency / 255.0)),
                 sal_uInt8(aFront.GetGreen() * aTransparency / 255.0 + aBack.GetGreen() * (1 - aTransparency / 255.0)),
                 sal_uInt8(aFront.GetBlue()  * aTransparency / 255.0 + aBack.GetBlue()  * (1 - aTransparency / 255.0)));
}

AnnotationMenuButton::AnnotationMenuButton(sw::annotation::SwAnnotationWin& rSidebarWin)
    : MenuButton(&rSidebarWin)
    , mrSidebarWin(rSidebarWin)
{
    AddEventListener(LINK(&mrSidebarWin, sw::annotation::SwAnnotationWin, WindowEventListener));

    SetAccessibleName(SwResId(STR_ACCESS_ANNOTATION_BUTTON_NAME));
    SetAccessibleDescription(SwResId(STR_ACCESS_ANNOTATION_BUTTON_DESC));
    SetQuickHelpText(GetAccessibleDescription());
}

AnnotationMenuButton::~AnnotationMenuButton()
{
    disposeOnce();
}

void AnnotationMenuButton::dispose()
{
    RemoveEventListener(LINK(&mrSidebarWin, sw::annotation::SwAnnotationWin, WindowEventListener));
    MenuButton::dispose();
}

void AnnotationMenuButton::Select()
{
    OString sIdent = GetCurItemIdent();
    if (sIdent == "reply")
        mrSidebarWin.ExecuteCommand(FN_REPLY);
    else if (sIdent == "delete")
        mrSidebarWin.ExecuteCommand(FN_DELETE_COMMENT);
    else if (sIdent == "deleteby")
        mrSidebarWin.ExecuteCommand(FN_DELETE_NOTE_AUTHOR);
    else if (sIdent == "deleteall")
        mrSidebarWin.ExecuteCommand(FN_DELETE_ALL_NOTES);
    else if (sIdent == "formatall")
        mrSidebarWin.ExecuteCommand(FN_FORMAT_ALL_NOTES);
}

void AnnotationMenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    PopupMenu* pButtonPopup(GetPopupMenu());
    if (mrSidebarWin.IsReadOnly())
    {
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("reply"), false);
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("delete"), false );
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("deleteby"), false );
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("deleteall"), false );
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("formatall"), false );
    }
    else
    {
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("delete"), !mrSidebarWin.IsProtected());
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("deleteby"));
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("deleteall"));
        pButtonPopup->EnableItem(pButtonPopup->GetItemId("formatall"));
    }

    if (mrSidebarWin.IsProtected())
    {
        pButtonPopup->EnableItem(FN_REPLY, false);
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
        if (sAuthor == mrSidebarWin.GetAuthor())
        {
            pButtonPopup->EnableItem(FN_REPLY, false);
        }
        else
        {
            pButtonPopup->EnableItem(FN_REPLY);
        }
    }

    MenuButton::MouseButtonDown(rMEvt);
}

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
    const long nBorderDistanceLeftAndRight = ((aSymbolRect.GetWidth() * 250) + 500) / 1000;
    aSymbolRect.AdjustLeft(nBorderDistanceLeftAndRight );
    aSymbolRect.AdjustRight( -nBorderDistanceLeftAndRight );
    // 40% distance to the top button border
    const long nBorderDistanceTop = ((aSymbolRect.GetHeight() * 400) + 500) / 1000;
    aSymbolRect.AdjustTop(nBorderDistanceTop );
    // 15% distance to the bottom button border
    const long nBorderDistanceBottom = ((aSymbolRect.GetHeight() * 150) + 500) / 1000;
    aSymbolRect.AdjustBottom( -nBorderDistanceBottom );
    DecorationView aDecoView(&rRenderContext);
    aDecoView.DrawSymbol(aSymbolRect, SymbolType::SPIN_DOWN, (bHighContrast ? COL_WHITE : COL_BLACK));
}

void AnnotationMenuButton::KeyInput(const KeyEvent& rKeyEvt)
{
    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    if (rKeyCode.GetCode() == KEY_TAB)
    {
        mrSidebarWin.ActivatePostIt();
        mrSidebarWin.GrabFocus();
    }
    else
    {
        MenuButton::KeyInput(rKeyEvt);
    }
}

}} // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
