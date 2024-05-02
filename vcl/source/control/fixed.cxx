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

#include <vcl/cvtgrf.hxx>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/settings.hxx>

#include <comphelper/base64.hxx>
#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>
#include <tools/stream.hxx>

#define FIXEDLINE_TEXT_BORDER    4

constexpr auto FIXEDTEXT_VIEW_STYLE = WB_3DLOOK |
                                 WB_LEFT | WB_CENTER | WB_RIGHT |
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |
                                 WB_WORDBREAK | WB_NOLABEL |
                                 WB_PATHELLIPSIS;
constexpr auto FIXEDLINE_VIEW_STYLE = WB_3DLOOK | WB_NOLABEL;
constexpr auto FIXEDBITMAP_VIEW_STYLE = WB_3DLOOK |
                                 WB_LEFT | WB_CENTER | WB_RIGHT |
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |
                                 WB_SCALE;
constexpr auto FIXEDIMAGE_VIEW_STYLE = WB_3DLOOK |
                                 WB_LEFT | WB_CENTER | WB_RIGHT |
                                 WB_TOP | WB_VCENTER | WB_BOTTOM |
                                 WB_SCALE;

static Point ImplCalcPos( WinBits nStyle, const Point& rPos,
                          const Size& rObjSize, const Size& rWinSize )
{
    tools::Long    nX;
    tools::Long    nY;

    if ( nStyle & WB_LEFT )
        nX = 0;
    else if ( nStyle & WB_RIGHT )
        nX = rWinSize.Width()-rObjSize.Width();
    else
        nX = (rWinSize.Width()-rObjSize.Width())/2;

    if ( nStyle & WB_TOP )
        nY = 0;
    else if ( nStyle & WB_BOTTOM )
        nY = rWinSize.Height()-rObjSize.Height();
    else
        nY = (rWinSize.Height()-rObjSize.Height())/2;

    Point aPos( nX+rPos.X(), nY+rPos.Y() );
    return aPos;
}

void FixedText::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );
    ApplySettings(*GetOutDev());
}

WinBits FixedText::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& FixedText::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetLabelFont();
}

const Color& FixedText::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetLabelTextColor();
}

FixedText::FixedText(vcl::Window* pParent, WinBits nStyle, WindowType eType)
    : Control(eType)
    , m_nMaxWidthChars(-1)
    , m_nMinWidthChars(-1)
    , m_pMnemonicWindow(nullptr)
{
    ImplInit( pParent, nStyle );
}

DrawTextFlags FixedText::ImplGetTextStyle( WinBits nWinStyle )
{
    DrawTextFlags nTextStyle = DrawTextFlags::Mnemonic | DrawTextFlags::EndEllipsis;

    if( ! (nWinStyle & WB_NOMULTILINE) )
        nTextStyle |= DrawTextFlags::MultiLine;

    if ( nWinStyle & WB_RIGHT )
        nTextStyle |= DrawTextFlags::Right;
    else if ( nWinStyle & WB_CENTER )
        nTextStyle |= DrawTextFlags::Center;
    else
        nTextStyle |= DrawTextFlags::Left;
    if ( nWinStyle & WB_BOTTOM )
        nTextStyle |= DrawTextFlags::Bottom;
    else if ( nWinStyle & WB_VCENTER )
        nTextStyle |= DrawTextFlags::VCenter;
    else
        nTextStyle |= DrawTextFlags::Top;
    if ( nWinStyle & WB_WORDBREAK )
        nTextStyle |= DrawTextFlags::WordBreak;
    if ( nWinStyle & WB_NOLABEL )
        nTextStyle &= ~DrawTextFlags::Mnemonic;

    return nTextStyle;
}

void FixedText::ImplDraw(OutputDevice* pDev, SystemTextColorFlags nSystemTextColorFlags,
                         const Point& rPos, const Size& rSize,
                         bool bFillLayout) const
{
    const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();
    WinBits nWinStyle = GetStyle();
    OUString aText(GetText());
    DrawTextFlags nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
    Point aPos = rPos;

    if ( nWinStyle & WB_EXTRAOFFSET )
        aPos.AdjustX(2 );

    if ( nWinStyle & WB_PATHELLIPSIS )
    {
        nTextStyle &= ~DrawTextFlags(DrawTextFlags::EndEllipsis | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak);
        nTextStyle |= DrawTextFlags::PathEllipsis;
    }
    if ( !IsEnabled() )
        nTextStyle |= DrawTextFlags::Disable;
    if ( (nSystemTextColorFlags & SystemTextColorFlags::Mono) ||
         (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) )
        nTextStyle |= DrawTextFlags::Mono;

    if( bFillLayout )
        mxLayoutData->m_aDisplayText.clear();

    const tools::Rectangle aRect(aPos, rSize);
    DrawControlText(*pDev, aRect, aText, nTextStyle,
        bFillLayout ? &mxLayoutData->m_aUnicodeBoundRects : nullptr,
        bFillLayout ? &mxLayoutData->m_aDisplayText : nullptr);
}

void FixedText::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Control::ApplySettings(rRenderContext);

    vcl::Window* pParent = GetParent();
    bool bEnableTransparent = true;
    if (!pParent->IsChildTransparentModeEnabled() || IsControlBackground())
    {
        EnableChildTransparentMode(false);
        SetParentClipMode();
        SetPaintTransparent(false);

        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else
            rRenderContext.SetBackground(pParent->GetBackground());

        if (rRenderContext.IsBackground())
            bEnableTransparent = false;
    }

    if (bEnableTransparent)
    {
        EnableChildTransparentMode();
        SetParentClipMode(ParentClipMode::NoClip);
        SetPaintTransparent(true);
        rRenderContext.SetBackground();
    }
}

void FixedText::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    ImplDraw(&rRenderContext, SystemTextColorFlags::NONE, Point(), GetOutputSizePixel());
}

void FixedText::Draw( OutputDevice* pDev, const Point& rPos,
                      SystemTextColorFlags nFlags )
{
    ApplySettings(*pDev);

    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = GetSizePixel();
    vcl::Font   aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & SystemTextColorFlags::Mono )
        pDev->SetTextColor( COL_BLACK );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    bool bBorder = (GetStyle() & WB_BORDER);
    bool bBackground = IsControlBackground();
    if ( bBorder || bBackground )
    {
        tools::Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            ImplDrawFrame( pDev, aRect );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

void FixedText::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedText::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::Enable) ||
         (nType == StateChangedType::Text) ||
         (nType == StateChangedType::UpdateMode) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDTEXT_VIEW_STYLE) !=
             (GetStyle() & FIXEDTEXT_VIEW_STYLE) )
        {
            ApplySettings(*GetOutDev());
            Invalidate();
        }
    }
    else if ( (nType == StateChangedType::Zoom)  ||
              (nType == StateChangedType::ControlFont) )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

void FixedText::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

Size FixedText::getTextDimensions(Control const *pControl, const OUString &rTxt, tools::Long nMaxWidth)
{
    DrawTextFlags nStyle = ImplGetTextStyle( pControl->GetStyle() );
    if ( !( pControl->GetStyle() & WB_NOLABEL ) )
        nStyle |= DrawTextFlags::Mnemonic;

    return pControl->GetTextRect(tools::Rectangle( Point(), Size(nMaxWidth, 0x7fffffff)),
                                       rTxt, nStyle).GetSize();
}

Size FixedText::CalcMinimumTextSize( Control const *pControl, tools::Long nMaxWidth )
{
    Size aSize = getTextDimensions(pControl, pControl->GetText(), nMaxWidth);

    if ( pControl->GetStyle() & WB_EXTRAOFFSET )
        aSize.AdjustWidth(2 );

    // GetTextRect cannot take an empty string
    if ( aSize.Width() < 0 )
        aSize.setWidth( 0 );
    if ( aSize.Height() <= 0 )
        aSize.setHeight( pControl->GetTextHeight() );

    return aSize;
}

Size FixedText::CalcMinimumSize( tools::Long nMaxWidth ) const
{
    return CalcWindowSize( CalcMinimumTextSize ( this, nMaxWidth ) );
}

Size FixedText::GetOptimalSize() const
{
    sal_Int32 nMaxAvailWidth = 0x7fffffff;
    if (m_nMaxWidthChars != -1)
    {
        OUStringBuffer aBuf(m_nMaxWidthChars);
        comphelper::string::padToLength(aBuf, m_nMaxWidthChars, 'x');
        nMaxAvailWidth = getTextDimensions(this,
            aBuf.makeStringAndClear(), 0x7fffffff).Width();
    }
    Size aRet = CalcMinimumSize(nMaxAvailWidth);
    if (m_nMinWidthChars != -1)
    {
        OUStringBuffer aBuf(m_nMinWidthChars);
        comphelper::string::padToLength(aBuf, m_nMinWidthChars, 'x');
        Size aMinAllowed = getTextDimensions(this,
            aBuf.makeStringAndClear(), 0x7fffffff);
        aRet.setWidth(std::max(aMinAllowed.Width(), aRet.Width()));
    }
    return aRet;
}

void FixedText::FillLayoutData() const
{
    mxLayoutData.emplace();
    ImplDraw(const_cast<FixedText*>(this)->GetOutDev(), SystemTextColorFlags::NONE, Point(), GetOutputSizePixel(), true);
    //const_cast<FixedText*>(this)->Invalidate();
}

void FixedText::setMaxWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_nMaxWidthChars)
    {
        m_nMaxWidthChars = nWidth;
        queue_resize();
    }
}

void FixedText::setMinWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_nMinWidthChars)
    {
        m_nMinWidthChars = nWidth;
        queue_resize();
    }
}

bool FixedText::set_property(const OUString &rKey, const OUString &rValue)
{
    if (rKey == "max-width-chars")
        setMaxWidthChars(rValue.toInt32());
    else if (rKey == "width-chars")
        setMinWidthChars(rValue.toInt32());
    else if (rKey == "ellipsize")
    {
        WinBits nBits = GetStyle();
        nBits &= ~WB_PATHELLIPSIS;
        if (rValue != "none")
        {
            SAL_WARN_IF(rValue != "end", "vcl.layout", "Only endellipsis support for now");
            nBits |= WB_PATHELLIPSIS;
        }
        SetStyle(nBits);
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

vcl::Window* FixedText::getAccessibleRelationLabelFor() const
{
    vcl::Window *pWindow = Control::getAccessibleRelationLabelFor();
    if (pWindow)
        return pWindow;
    return get_mnemonic_widget();
}

void FixedText::set_mnemonic_widget(vcl::Window *pWindow)
{
    if (pWindow == m_pMnemonicWindow)
        return;
    if (m_pMnemonicWindow)
    {
        vcl::Window *pTempReEntryGuard = m_pMnemonicWindow;
        m_pMnemonicWindow = nullptr;
        pTempReEntryGuard->remove_mnemonic_label(this);
    }
    m_pMnemonicWindow = pWindow;
    if (m_pMnemonicWindow)
        m_pMnemonicWindow->add_mnemonic_label(this);
}

FixedText::~FixedText()
{
    disposeOnce();
}

void FixedText::dispose()
{
    set_mnemonic_widget(nullptr);
    m_pMnemonicWindow.clear();
    Control::dispose();
}

SelectableFixedText::SelectableFixedText(vcl::Window* pParent, WinBits nStyle)
    : Edit(pParent, nStyle)
{
    // no border
    SetBorderStyle( WindowBorderStyle::NOBORDER );
    // read-only
    SetReadOnly();
    // make it transparent
    SetPaintTransparent(true);
    SetControlBackground();
}

void SelectableFixedText::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetBackground();
}

void SelectableFixedText::LoseFocus()
{
    Edit::LoseFocus();
    // clear cursor
    Invalidate();
}

void SelectableFixedText::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    Edit::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("type", "fixedtext");
    rJsonWriter.put("selectable", true);
}

void FixedLine::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );
    ApplySettings(*GetOutDev());
}

WinBits FixedLine::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& FixedLine::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupFont();
}

const Color& FixedLine::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupTextColor();
}

void FixedLine::ImplDraw(vcl::RenderContext& rRenderContext)
{
    // we need to measure according to the window, not according to the
    // RenderContext we paint to
    Size aOutSize = GetOutputSizePixel();

    OUString aText = GetText();
    WinBits nWinStyle = GetStyle();

    DecorationView aDecoView(&rRenderContext);
    if (aText.isEmpty())
    {
        if (nWinStyle & WB_VERT)
        {
            tools::Long nX = (aOutSize.Width() - 1) / 2;
            aDecoView.DrawSeparator(Point(nX, 0), Point(nX, aOutSize.Height() - 1));
        }
        else
        {
            tools::Long nY = (aOutSize.Height() - 1) / 2;
            aDecoView.DrawSeparator(Point(0, nY), Point(aOutSize.Width() - 1, nY), false);
        }
    }
    else if (nWinStyle & WB_VERT)
    {
        tools::Long nWidth = rRenderContext.GetTextWidth(aText);
        rRenderContext.Push(vcl::PushFlags::FONT);
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetOrientation(900_deg10);
        SetFont(aFont);
        Point aStartPt(aOutSize.Width() / 2, aOutSize.Height() - 1);
        if (nWinStyle & WB_VCENTER)
            aStartPt.AdjustY( -((aOutSize.Height() - nWidth) / 2) );
        Point aTextPt(aStartPt);
        aTextPt.AdjustX( -(GetTextHeight() / 2) );
        rRenderContext.DrawText(aTextPt, aText, 0, aText.getLength());
        rRenderContext.Pop();
        if (aOutSize.Height() - aStartPt.Y() > FIXEDLINE_TEXT_BORDER)
            aDecoView.DrawSeparator(Point(aStartPt.X(), aStartPt.Y() + FIXEDLINE_TEXT_BORDER),
                                    Point(aStartPt.X(), aOutSize.Height() - 1));
        if (aStartPt.Y() - nWidth - FIXEDLINE_TEXT_BORDER > 0)
            aDecoView.DrawSeparator(Point(aStartPt.X(), 0),
                                    Point(aStartPt.X(), aStartPt.Y() - nWidth - FIXEDLINE_TEXT_BORDER));
    }
    else
    {
        DrawTextFlags nStyle = DrawTextFlags::Mnemonic | DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::EndEllipsis;
        tools::Rectangle aRect(0, 0, aOutSize.Width(), aOutSize.Height());
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        if (nWinStyle & WB_CENTER)
            nStyle |= DrawTextFlags::Center;

        if (!IsEnabled())
            nStyle |= DrawTextFlags::Disable;
        if (GetStyle() & WB_NOLABEL)
            nStyle &= ~DrawTextFlags::Mnemonic;
        if (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono)
            nStyle |= DrawTextFlags::Mono;

        aRect = DrawControlText(*GetOutDev(), aRect, aText, nStyle, nullptr, nullptr);

        tools::Long nTop = aRect.Top() + ((aRect.GetHeight() - 1) / 2);
        aDecoView.DrawSeparator(Point(aRect.Right() + FIXEDLINE_TEXT_BORDER, nTop), Point(aOutSize.Width() - 1, nTop), false);
        if (aRect.Left() > FIXEDLINE_TEXT_BORDER)
            aDecoView.DrawSeparator(Point(0, nTop), Point(aRect.Left() - FIXEDLINE_TEXT_BORDER, nTop), false);
    }
}

FixedLine::FixedLine( vcl::Window* pParent, WinBits nStyle ) :
    Control( WindowType::FIXEDLINE )
{
    ImplInit( pParent, nStyle );
    SetSizePixel( Size( 2, 2 ) );
}

void FixedLine::FillLayoutData() const
{
    mxLayoutData.emplace();
    const_cast<FixedLine*>(this)->Invalidate();
}

void FixedLine::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Control::ApplySettings(rRenderContext);

    vcl::Window* pParent = GetParent();
    if (pParent->IsChildTransparentModeEnabled() && !IsControlBackground())
    {
        EnableChildTransparentMode();
        SetParentClipMode(ParentClipMode::NoClip);
        SetPaintTransparent(true);
        rRenderContext.SetBackground();
    }
    else
    {
        EnableChildTransparentMode(false);
        SetParentClipMode();
        SetPaintTransparent(false);

        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else
            rRenderContext.SetBackground(pParent->GetBackground());
    }
}

void FixedLine::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDraw(rRenderContext);
}

void FixedLine::Draw( OutputDevice*, const Point&, SystemTextColorFlags )
{
}

void FixedLine::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedLine::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::Enable) ||
         (nType == StateChangedType::Text) ||
         (nType == StateChangedType::UpdateMode) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDLINE_VIEW_STYLE) !=
             (GetStyle() & FIXEDLINE_VIEW_STYLE) )
            Invalidate();
    }
    else if ( (nType == StateChangedType::Zoom)  ||
              (nType == StateChangedType::Style) ||
              (nType == StateChangedType::ControlFont) )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

void FixedLine::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

Size FixedLine::GetOptimalSize() const
{
    return CalcWindowSize( FixedText::CalcMinimumTextSize ( this ) );
}

void FixedLine::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    Control::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("type", "separator");
    rJsonWriter.put("orientation", (GetStyle() & WB_VERT) ? "vertical" : "horizontal");
}

void FixedBitmap::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );
    ApplySettings(*GetOutDev());
}

WinBits FixedBitmap::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

FixedBitmap::FixedBitmap( vcl::Window* pParent, WinBits nStyle ) :
    Control( WindowType::FIXEDBITMAP )
{
    ImplInit( pParent, nStyle );
}

void FixedBitmap::ImplDraw( OutputDevice* pDev, const Point& rPos, const Size& rSize )
{
    // do we have a Bitmap?
    if ( !maBitmap.IsEmpty() )
    {
        if ( GetStyle() & WB_SCALE )
            pDev->DrawBitmapEx( rPos, rSize, maBitmap );
        else
        {
            Point aPos = ImplCalcPos( GetStyle(), rPos, maBitmap.GetSizePixel(), rSize );
            pDev->DrawBitmapEx( aPos, maBitmap );
        }
    }
}

void FixedBitmap::ApplySettings(vcl::RenderContext& rRenderContext)
{
    vcl::Window* pParent = GetParent();
    if (pParent->IsChildTransparentModeEnabled() && !IsControlBackground())
    {
        EnableChildTransparentMode();
        SetParentClipMode(ParentClipMode::NoClip);
        SetPaintTransparent(true);
        rRenderContext.SetBackground();
    }
    else
    {
        EnableChildTransparentMode(false);
        SetParentClipMode();
        SetPaintTransparent(false);

        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else
            rRenderContext.SetBackground(pParent->GetBackground());
    }
}

void FixedBitmap::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDraw(&rRenderContext, Point(), GetOutputSizePixel());
}

void FixedBitmap::Draw( OutputDevice* pDev, const Point& rPos,
                        SystemTextColorFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = GetSizePixel();
    tools::Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( GetStyle() & WB_BORDER )
    {
        DecorationView aDecoView( pDev );
        aRect = aDecoView.DrawFrame( aRect, DrawFrameStyle::DoubleIn );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( pDev, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

void FixedBitmap::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedBitmap::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::Data) ||
         (nType == StateChangedType::UpdateMode) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDBITMAP_VIEW_STYLE) !=
             (GetStyle() & FIXEDBITMAP_VIEW_STYLE) )
            Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

void FixedBitmap::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

void FixedBitmap::SetBitmap( const BitmapEx& rBitmap )
{
    maBitmap = rBitmap;
    CompatStateChanged( StateChangedType::Data );
    queue_resize();
}

void FixedImage::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );
    ApplySettings(*GetOutDev());
}

WinBits FixedImage::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

FixedImage::FixedImage( vcl::Window* pParent, WinBits nStyle ) :
    Control( WindowType::FIXEDIMAGE )
{
    ImplInit( pParent, nStyle );
}

void FixedImage::ImplDraw( OutputDevice* pDev,
                           const Point& rPos, const Size& rSize )
{
    DrawImageFlags nStyle = DrawImageFlags::NONE;
    if ( !IsEnabled() )
        nStyle |= DrawImageFlags::Disable;

    Image *pImage = &maImage;

    // do we have an image?
    if ( !(!(*pImage)) )
    {
        if ( GetStyle() & WB_SCALE )
            pDev->DrawImage( rPos, rSize, *pImage, nStyle );
        else
        {
            Point aPos = ImplCalcPos( GetStyle(), rPos, pImage->GetSizePixel(), rSize );
            pDev->DrawImage( aPos, *pImage, nStyle );
        }
    }
}

void FixedImage::ApplySettings(vcl::RenderContext& rRenderContext)
{
    vcl::Window* pParent = GetParent();
    if (pParent && pParent->IsChildTransparentModeEnabled() && !IsControlBackground())
    {
        EnableChildTransparentMode();
        SetParentClipMode(ParentClipMode::NoClip);
        SetPaintTransparent(true);
        rRenderContext.SetBackground();
    }
    else
    {
        EnableChildTransparentMode(false);
        SetParentClipMode();
        SetPaintTransparent(false);

        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else if (pParent)
            rRenderContext.SetBackground(pParent->GetBackground());
    }
}


void FixedImage::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDraw(&rRenderContext, Point(), GetOutputSizePixel());
}

Size FixedImage::GetOptimalSize() const
{
    return maImage.GetSizePixel();
}

void FixedImage::Draw( OutputDevice* pDev, const Point& rPos,
                       SystemTextColorFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = GetSizePixel();
    tools::Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( GetStyle() & WB_BORDER )
    {
        ImplDrawFrame( pDev, aRect );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( pDev, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

void FixedImage::Resize()
{
    Control::Resize();
    Invalidate();
}

void FixedImage::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::Enable) ||
         (nType == StateChangedType::Data) ||
         (nType == StateChangedType::UpdateMode) )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & FIXEDIMAGE_VIEW_STYLE) !=
             (GetStyle() & FIXEDIMAGE_VIEW_STYLE) )
            Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

void FixedImage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
}

void FixedImage::SetImage( const Image& rImage )
{
    if ( rImage != maImage )
    {
        maImage = rImage;
        CompatStateChanged( StateChangedType::Data );
        queue_resize();
    }
}

Image FixedImage::loadThemeImage(const OUString &rFileName)
{
    return Image(StockImage::Yes, rFileName);
}

bool FixedImage::set_property(const OUString &rKey, const OUString &rValue)
{
    if (rKey == "icon-size")
    {
        WinBits nBits = GetStyle();
        nBits &= ~WB_SMALLSTYLE;
        if (rValue == "2")
            nBits |= WB_SMALLSTYLE;
        SetStyle(nBits);
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

void FixedImage::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    Control::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("id", get_id());
    rJsonWriter.put("type", "image");
    if (!!maImage)
    {
        SvMemoryStream aOStm(6535, 6535);
        if(GraphicConverter::Export(aOStm, maImage.GetBitmapEx(), ConvertDataFormat::PNG) == ERRCODE_NONE)
        {
            css::uno::Sequence<sal_Int8> aSeq( static_cast<sal_Int8 const *>(aOStm.GetData()), aOStm.Tell());
            OStringBuffer aBuffer("data:image/png;base64,");
            ::comphelper::Base64::encode(aBuffer, aSeq);
            rJsonWriter.put("image", aBuffer);
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
