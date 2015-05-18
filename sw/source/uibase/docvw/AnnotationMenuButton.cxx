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

#include <AnnotationMenuButton.hxx>

#include <annotation.hrc>
#include <app.hrc>
#include <access.hrc>

#include <unotools/useroptions.hxx>

#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/decoview.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>

#include <cmdid.h>
#include <SidebarWin.hxx>

namespace sw { namespace annotation {

Color ColorFromAlphaColor(const sal_uInt8 aTransparency, const Color& aFront, const Color& aBack)
{
    return Color(sal_uInt8(aFront.GetRed()   * aTransparency / 255.0 + aBack.GetRed()   * (1 - aTransparency / 255.0)),
                 sal_uInt8(aFront.GetGreen() * aTransparency / 255.0 + aBack.GetGreen() * (1 - aTransparency / 255.0)),
                 sal_uInt8(aFront.GetBlue()  * aTransparency / 255.0 + aBack.GetBlue()  * (1 - aTransparency / 255.0)));
}

AnnotationMenuButton::AnnotationMenuButton(sw::sidebarwindows::SwSidebarWin& rSidebarWin)
    : MenuButton(&rSidebarWin)
    , mrSidebarWin(rSidebarWin)
{
    AddEventListener(LINK(&mrSidebarWin, sw::sidebarwindows::SwSidebarWin, WindowEventListener));

    SetAccessibleName(SW_RES(STR_ACCESS_ANNOTATION_BUTTON_NAME));
    SetAccessibleDescription(SW_RES(STR_ACCESS_ANNOTATION_BUTTON_DESC));
    SetQuickHelpText(GetAccessibleDescription());
}

AnnotationMenuButton::~AnnotationMenuButton()
{
    disposeOnce();
}

void AnnotationMenuButton::dispose()
{
    RemoveEventListener(LINK(&mrSidebarWin, sw::sidebarwindows::SwSidebarWin, WindowEventListener));
    MenuButton::dispose();
}

void AnnotationMenuButton::Select()
{
    mrSidebarWin.ExecuteCommand( GetCurItemId() );
}

void AnnotationMenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    PopupMenu* pButtonPopup(GetPopupMenu());
    if (mrSidebarWin.IsReadOnly())
    {
        pButtonPopup->EnableItem(FN_REPLY, false );
        pButtonPopup->EnableItem(FN_DELETE_COMMENT, false );
        pButtonPopup->EnableItem(FN_DELETE_NOTE_AUTHOR, false );
        pButtonPopup->EnableItem(FN_DELETE_ALL_NOTES, false );
        pButtonPopup->EnableItem(FN_FORMAT_ALL_NOTES, false );
    }
    else
    {
        pButtonPopup->EnableItem(FN_DELETE_COMMENT, !mrSidebarWin.IsProtected() );
        pButtonPopup->EnableItem(FN_DELETE_NOTE_AUTHOR, true );
        pButtonPopup->EnableItem(FN_DELETE_ALL_NOTES, true );
        pButtonPopup->EnableItem(FN_FORMAT_ALL_NOTES, true );
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
                sAuthor = SW_RES(STR_REDLINE_UNKNOWN_AUTHOR);
            }
        }
        // do not allow to reply to ourself and no answer possible if this note is in a protected section
        if (sAuthor == mrSidebarWin.GetAuthor())
        {
            pButtonPopup->EnableItem(FN_REPLY, false);
        }
        else
        {
            pButtonPopup->EnableItem(FN_REPLY, true);
        }
    }

    MenuButton::MouseButtonDown(rMEvt);
}

void AnnotationMenuButton::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    bool bHighContrast = rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode();

    if (bHighContrast)
        rRenderContext.SetFillColor(COL_BLACK);
    else
        rRenderContext.SetFillColor(mrSidebarWin.ColorDark());
    rRenderContext.SetLineColor();
    const Rectangle aRect(Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));
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
            aGradient = Gradient(GradientStyle_LINEAR,
                                 ColorFromAlphaColor(80, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()),
                                 ColorFromAlphaColor(15, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()));
        else
            aGradient = Gradient(GradientStyle_LINEAR,
                                 ColorFromAlphaColor(15, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()),
                                 ColorFromAlphaColor(80, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()));
        rRenderContext.DrawGradient(aRect, aGradient);

        //draw rect around button
        rRenderContext.SetFillColor();
        rRenderContext.SetLineColor(ColorFromAlphaColor(90, mrSidebarWin.ColorAnchor(), mrSidebarWin.ColorDark()));
    }
    rRenderContext.DrawRect(aRect);

    if (mrSidebarWin.IsPreview())
    {
        vcl::Font aOldFont(mrSidebarWin.GetFont());
        vcl::Font aFont(aOldFont);
        Color aCol(COL_BLACK);
        aFont.SetColor(aCol);
        aFont.SetHeight(200);
        aFont.SetWeight(WEIGHT_MEDIUM);
        rRenderContext.SetFont(aFont);
        rRenderContext.DrawText(aRect, OUString("Edit Note"), DrawTextFlags::Center);
        rRenderContext.SetFont(aOldFont);
    }
    else
    {
        Rectangle aSymbolRect(aRect);
        // 25% distance to the left and right button border
        const long nBorderDistanceLeftAndRight = ((aSymbolRect.GetWidth() * 250) + 500) / 1000;
        aSymbolRect.Left() += nBorderDistanceLeftAndRight;
        aSymbolRect.Right() -= nBorderDistanceLeftAndRight;
        // 40% distance to the top button border
        const long nBorderDistanceTop = ((aSymbolRect.GetHeight() * 400) + 500) / 1000;
        aSymbolRect.Top()+=nBorderDistanceTop;
        // 15% distance to the bottom button border
        const long nBorderDistanceBottom = ((aSymbolRect.GetHeight() * 150) + 500) / 1000;
        aSymbolRect.Bottom() -= nBorderDistanceBottom;
        DecorationView aDecoView(&rRenderContext);
        aDecoView.DrawSymbol(aSymbolRect, SymbolType::SPIN_DOWN, (bHighContrast ? Color(COL_WHITE) : Color(COL_BLACK)));
    }
}

void AnnotationMenuButton::KeyInput(const KeyEvent& rKeyEvt)
{
    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    const sal_uInt16 nKey = rKeyCode.GetCode();
    if (nKey == KEY_TAB)
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
