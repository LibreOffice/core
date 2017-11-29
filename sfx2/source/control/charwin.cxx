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

#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/charwin.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

using namespace com::sun::star;


SvxCharView::SvxCharView(vcl::Window* pParent)
    : Control(pParent, WB_TABSTOP | WB_BORDER)
    , mnY(0)
    , maPosition(0,0)
{
}

VCL_BUILDER_FACTORY(SvxCharView)

void SvxCharView::MouseButtonDown( const MouseEvent& rMEvt )
{
    Control::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() )
    {
        if ( !(rMEvt.GetClicks() % 2) )
        {
            InsertCharToDoc();
        }

        maMouseClickHdl.Call(this);
    }

    // suppress context menu on floating windows
    if (rMEvt.IsRight() && GetSystemWindow()->GetType() != WindowType::FLOATINGWINDOW)
    {
        Point aPosition (rMEvt.GetPosPixel());
        maPosition = aPosition;
        GrabFocus();
        Invalidate();
        createContextMenu();
    }
}

void SvxCharView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();

    switch (aCode.GetCode())
    {
        case KEY_SPACE:
        case KEY_RETURN:
            InsertCharToDoc();
            break;
    }
    Control::KeyInput(rKEvt);
}

void SvxCharView::InsertCharToDoc()
{
    if(GetText().isEmpty())
        return;

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    uno::Sequence<beans::PropertyValue> aArgs(2);
    aArgs[0].Name = "Symbols";
    aArgs[0].Value <<= GetText();

    aArgs[1].Name = "FontName";
    aArgs[1].Value <<= maFont.GetFamilyName();

    comphelper::dispatchCommand(".uno:InsertSymbol", aArgs);
}

void SvxCharView::createContextMenu()
{
    ScopedVclPtrInstance<PopupMenu> pItemMenu;
    pItemMenu->InsertItem(0,SfxResId(STR_CLEAR_CHAR));
    pItemMenu->InsertItem(1,SfxResId(STR_CLEAR_ALL_CHAR));
    pItemMenu->SetSelectHdl(LINK(this, SvxCharView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, tools::Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK(SvxCharView, ContextMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case 0:
        maClearClickHdl.Call(this);
        break;
    case 1:
        maClearAllClickHdl.Call(this);
        break;
    default:
        break;
    }
    return false;
}

void SvxCharView::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.SetFont(maFont);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetFieldTextColor());
    Color aHighlightColor(rStyleSettings.GetHighlightColor());
    Color aHighlightTextColor(rStyleSettings.GetHighlightTextColor());
    Color aFillColor(rStyleSettings.GetWindowColor());
    Color aTextColor(rStyleSettings.GetWindowTextColor());

    const OUString aText = GetText();
    const Size aSize(GetOutputSizePixel());

    long nAvailWidth = aSize.Width();
    long nWinHeight = GetOutputSizePixel().Height();

    bool bGotBoundary = true;
    bool bShrankFont = false;
    vcl::Font aOrigFont(rRenderContext.GetFont());
    Size aFontSize(aOrigFont.GetFontSize());
    ::tools::Rectangle aBoundRect;

    for (long nFontHeight = aFontSize.Height(); nFontHeight > 0; nFontHeight -= 1)
    {
        if (!rRenderContext.GetTextBoundRect( aBoundRect, aText ) || aBoundRect.IsEmpty())
        {
            bGotBoundary = false;
            break;
        }

        //only shrink in the single glyph large view mode
        long nTextWidth = aBoundRect.GetWidth();
        if (nAvailWidth > nTextWidth)
            break;
        vcl::Font aFont(aOrigFont);
        aFontSize.Height() = nFontHeight;
        aFont.SetFontSize(aFontSize);
        rRenderContext.SetFont(aFont);
        mnY = (nWinHeight - GetTextHeight()) / 2;
        bShrankFont = true;
    }

    Point aPoint(2, mnY);

    if (!bGotBoundary)
        aPoint.X() = (aSize.Width() - rRenderContext.GetTextWidth(aText)) / 2;
    else
    {
        // adjust position
        aBoundRect += aPoint;

        // vertical adjustment
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if( nYLDelta <= 0 )
            aPoint.Y() -= nYLDelta - 1;
        else if( nYHDelta <= 0 )
            aPoint.Y() += nYHDelta - 1;

        // centrally align glyph
        aPoint.X() = -aBoundRect.Left() + (aSize.Width() - aBoundRect.GetWidth()) / 2;
    }

    if (HasFocus())
    {
        rRenderContext.SetFillColor(aHighlightColor);
        rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), Size(GetOutputSizePixel().Width(), GetOutputSizePixel().Height())));

        rRenderContext.SetTextColor(aHighlightTextColor);
        rRenderContext.DrawText(aPoint, aText);
    }
    else
    {
        rRenderContext.SetFillColor(aFillColor);
        rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), Size(GetOutputSizePixel().Width(), GetOutputSizePixel().Height())));

        rRenderContext.SetTextColor(aWindowTextColor);
        rRenderContext.DrawText(aPoint, aText);
    }
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.SetTextColor(aTextColor);

    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}

void SvxCharView::setMouseClickHdl(const Link<SvxCharView*,void> &rLink)
{
    maMouseClickHdl = rLink;
}

void SvxCharView::setClearClickHdl(const Link<SvxCharView*,void> &rLink)
{
    maClearClickHdl = rLink;
}

void SvxCharView::setClearAllClickHdl(const Link<SvxCharView*,void> &rLink)
{
    maClearAllClickHdl = rLink;
}

void SvxCharView::SetFont( const vcl::Font& rFont )
{
    long nWinHeight = GetOutputSizePixel().Height();
    maFont = vcl::Font(rFont);
    maFont.SetWeight(WEIGHT_NORMAL);
    maFont.SetAlignment(ALIGN_TOP);
    maFont.SetFontSize(PixelToLogic(Size(0, nWinHeight / 2)));
    maFont.SetTransparent(true);
    Control::SetFont(maFont);

    mnY = (nWinHeight - GetTextHeight()) / 2;

    Invalidate();
}

Size SvxCharView::GetOptimalSize() const
{
    const vcl::Font &rFont = GetFont();
    const Size rFontSize = rFont.GetFontSize();
    long nWinHeight = LogicToPixel(rFontSize).Height() * 2;
    return Size( GetTextWidth( GetText() ) + 2 * 12, nWinHeight );
}

void SvxCharView::Resize()
{
    Control::Resize();
    SetFont(GetFont());
}


void SvxCharView::SetText( const OUString& rText )
{
    Control::SetText( rText );
    Invalidate();
}
