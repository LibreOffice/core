/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <StylesPreviewWindow.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>
#include <svl/itemset.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/tplpitem.hxx>
#include <sfx2/viewsh.hxx>

#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/brushitem.hxx>

#include <svx/xfillit0.hxx>
#include <svx/svddef.hxx>
#include <svx/xflclit.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>

#include <sal/log.hxx>

StyleStatusListener::StyleStatusListener(
    StylesPreviewWindow_Base* pPreviewControl,
    css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider)
    : SfxStatusListener(xDispatchProvider, SID_STYLE_FAMILY2, ".uno:ParaStyle")
    , m_pPreviewControl(pPreviewControl)
{
    ReBind();
}

void StyleStatusListener::StateChanged(SfxItemState /*eState*/, const SfxPoolItem* pState)
{
    const SfxTemplateItem* pStateItem = dynamic_cast<const SfxTemplateItem*>(pState);
    if (pStateItem)
        m_pPreviewControl->Select(pStateItem->GetStyleName());
}

StyleItemController::StyleItemController(
    const std::pair<OUString, OUString>& aStyleName,
    css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider)
    : m_eStyleFamily(SfxStyleFamily::Para)
    , m_aStyleName(aStyleName)
    , m_bSelected(false)
    , m_xDispatchProvider(xDispatchProvider)
{
}

void StyleItemController::Paint(vcl::RenderContext& rRenderContext,
                                const tools::Rectangle& /*rRect*/)
{
    rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::FONT | PushFlags::TEXTCOLOR);

    DrawEntry(rRenderContext);

    rRenderContext.Pop();
}

void StyleItemController::SetStyle(const std::pair<OUString, OUString>& sStyleName)
{
    m_aStyleName = sStyleName;
    Invalidate();
}

void StyleItemController::Select(bool bSelect)
{
    m_bSelected = bSelect;
    Invalidate();
}

bool StyleItemController::MouseButtonDown(const MouseEvent&)
{
    css::uno::Sequence<css::beans::PropertyValue> aArgs(2);
    aArgs[0].Value <<= m_aStyleName.second;
    aArgs[1].Name = "Family";
    aArgs[1].Value <<= sal_Int16(m_eStyleFamily);

    aArgs[0].Name = "Template";
    SfxToolBoxControl::Dispatch(m_xDispatchProvider, ".uno:StyleApply", aArgs);

    return false;
}

static Color GetTextColorFromItemSet(std::unique_ptr<const SfxItemSet> const& pItemSet)
{
    const SfxPoolItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_COLOR);
    if (pItem)
        return static_cast<const SvxColorItem*>(pItem)->GetValue();

    return COL_AUTO;
}

static Color GetHighlightColorFromItemSet(std::unique_ptr<const SfxItemSet> const& pItemSet)
{
    const SfxPoolItem* pItem = pItemSet->GetItem(SID_ATTR_BRUSH_CHAR);
    if (pItem)
        return static_cast<const SvxBrushItem*>(pItem)->GetColor();

    return COL_AUTO;
}

static Color GetBackgroundColorFromItemSet(std::unique_ptr<const SfxItemSet> const& pItemSet)
{
    const SfxPoolItem* pItem = pItemSet->GetItem(XATTR_FILLCOLOR);
    if (pItem)
        return static_cast<const XFillColorItem*>(pItem)->GetColorValue();

    return COL_AUTO;
}

static css::drawing::FillStyle
GetFillStyleFromItemSet(std::unique_ptr<const SfxItemSet> const& pItemSet)
{
    const SfxPoolItem* pItem = pItemSet->GetItem(XATTR_FILLSTYLE);
    if (pItem)
        return static_cast<const XFillStyleItem*>(pItem)->GetValue();

    return css::drawing::FillStyle_NONE;
}

static SvxFont GetFontFromItems(const SvxFontItem* pFontItem, Size aPixelFontSize,
                                std::unique_ptr<const SfxItemSet> const& pItemSet)
{
    SvxFont aFont;

    aFont.SetFamilyName(pFontItem->GetFamilyName());
    aFont.SetStyleName(pFontItem->GetStyleName());
    aFont.SetFontSize(aPixelFontSize);

    const SfxPoolItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_WEIGHT);
    if (pItem)
        aFont.SetWeight(static_cast<const SvxWeightItem*>(pItem)->GetWeight());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_POSTURE);
    if (pItem)
        aFont.SetItalic(static_cast<const SvxPostureItem*>(pItem)->GetPosture());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_CONTOUR);
    if (pItem)
        aFont.SetOutline(static_cast<const SvxContourItem*>(pItem)->GetValue());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_SHADOWED);
    if (pItem)
        aFont.SetShadow(static_cast<const SvxShadowedItem*>(pItem)->GetValue());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_RELIEF);
    if (pItem)
        aFont.SetRelief(static_cast<const SvxCharReliefItem*>(pItem)->GetValue());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE);
    if (pItem)
        aFont.SetUnderline(static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE);
    if (pItem)
        aFont.SetOverline(static_cast<const SvxOverlineItem*>(pItem)->GetValue());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_STRIKEOUT);
    if (pItem)
        aFont.SetStrikeout(static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_CASEMAP);
    if (pItem)
        aFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());

    pItem = pItemSet->GetItem(SID_ATTR_CHAR_EMPHASISMARK);
    if (pItem)
        aFont.SetEmphasisMark(static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark());

    return aFont;
}

void StyleItemController::DrawEntry(vcl::RenderContext& rRenderContext)
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if (!pShell)
        return;

    SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
    SfxStyleSheetBase* pStyle = nullptr;

    if (pPool)
    {
        pPool->SetSearchMask(m_eStyleFamily);
        pStyle = pPool->First();
        while (pStyle && pStyle->GetName() != m_aStyleName.first
               && pStyle->GetName() != m_aStyleName.second)
            pStyle = pPool->Next();

        if (!pStyle)
            return;

        Size aSize(rRenderContext.GetOutputSizePixel());
        tools::Rectangle aFullRect(Point(0, 0), aSize);
        aSize = Size(aSize.getWidth() - 6, aSize.getHeight() - 6);
        tools::Rectangle aContentRect(aFullRect);

        Color aOriginalColor = rRenderContext.GetFillColor();
        Color aOriginalLineColor = rRenderContext.GetLineColor();
        vcl::Region aOriginalClipRegion(aFullRect);

        if (m_bSelected)
        {
            aContentRect = tools::Rectangle(Point(3, 3), aSize);
            DrawSelection(rRenderContext);
        }

        DrawContentBackground(rRenderContext, aContentRect, aOriginalColor);

        vcl::Region aClipRegion(aContentRect);
        rRenderContext.SetClipRegion(aClipRegion);

        std::unique_ptr<const SfxItemSet> const pItemSet(pStyle->GetItemSetForPreview());
        if (!pItemSet)
            return;

        Color aFontHighlight = COL_AUTO;

        const SvxFontItem* const pFontItem = pItemSet->GetItem<SvxFontItem>(SID_ATTR_CHAR_FONT);
        const SvxFontHeightItem* const pFontHeightItem
            = pItemSet->GetItem<SvxFontHeightItem>(SID_ATTR_CHAR_FONTHEIGHT);

        if (pFontItem && pFontHeightItem)
        {
            Size aFontSize(0, pFontHeightItem->GetHeight());
            Size aPixelSize(rRenderContext.LogicToPixel(aFontSize, MapMode(pShell->GetMapUnit())));

            SvxFont aFont = GetFontFromItems(pFontItem, aPixelSize, pItemSet);
            rRenderContext.SetFont(aFont);

            Color aFontCol = GetTextColorFromItemSet(pItemSet);
            if (aFontCol != COL_AUTO)
                rRenderContext.SetTextColor(aFontCol);

            aFontHighlight = GetHighlightColorFromItemSet(pItemSet);

            css::drawing::FillStyle style = GetFillStyleFromItemSet(pItemSet);

            switch (style)
            {
                case css::drawing::FillStyle_SOLID:
                {
                    Color aBackCol = GetBackgroundColorFromItemSet(pItemSet);
                    if (aBackCol != COL_AUTO)
                        DrawContentBackground(rRenderContext, aContentRect, aBackCol);
                }
                break;

                default:
                    break;
                    //TODO Draw the other background styles: gradient, hatching and bitmap
            }
        }

        if (aFontHighlight != COL_AUTO)
            DrawHighlight(rRenderContext, aFontHighlight);

        DrawText(rRenderContext);

        rRenderContext.SetFillColor(aOriginalColor);
        rRenderContext.SetLineColor(aOriginalLineColor);
        rRenderContext.SetClipRegion(aOriginalClipRegion);
    }
}

void StyleItemController::DrawContentBackground(vcl::RenderContext& rRenderContext,
                                                tools::Rectangle& aContentRect, Color& aColor)
{
    rRenderContext.SetLineColor(aColor);
    rRenderContext.SetFillColor(aColor);
    rRenderContext.DrawRect(aContentRect);
}

void StyleItemController::DrawSelection(vcl::RenderContext& rRenderContext)
{
    Size aSize(rRenderContext.GetOutputSizePixel());
    tools::Rectangle aFullRect(Point(0, 0), aSize);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    rRenderContext.SetLineColor(rStyleSettings.GetActiveBorderColor());
    rRenderContext.SetFillColor(rStyleSettings.GetActiveBorderColor());

    rRenderContext.DrawRect(aFullRect);
}

void StyleItemController::DrawHighlight(vcl::RenderContext& rRenderContext, Color aFontBack)
{
    tools::Rectangle aTextRect;
    rRenderContext.GetTextBoundRect(aTextRect, m_aStyleName.second);

    Size aSize = aTextRect.GetSize();
    aSize.AdjustHeight(aSize.getHeight());
    aTextRect.SetSize(aSize);

    Point aPos(0, 0);
    aPos.AdjustX(LEFT_MARGIN);
    aPos.AdjustY((rRenderContext.GetOutputHeightPixel() - aTextRect.Bottom()) / 2);
    aTextRect.SetPos(aPos);

    rRenderContext.SetLineColor(aFontBack);
    rRenderContext.SetFillColor(aFontBack);

    rRenderContext.DrawRect(aTextRect);
}

void StyleItemController::DrawText(vcl::RenderContext& rRenderContext)
{
    tools::Rectangle aTextRect;
    rRenderContext.GetTextBoundRect(aTextRect, m_aStyleName.second);

    Point aPos(0, 0);
    aPos.AdjustX(LEFT_MARGIN);
    aPos.AdjustY((rRenderContext.GetOutputHeightPixel() - aTextRect.Bottom()) / 2);

    rRenderContext.DrawText(aPos, m_aStyleName.second);
}

StylesPreviewWindow_Base::StylesPreviewWindow_Base(
    weld::Builder& xBuilder, std::vector<std::pair<OUString, OUString>>& aDefaultStyles,
    css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider)
    : m_xUp(xBuilder.weld_toolbar("uptoolbar"))
    , m_xDown(xBuilder.weld_toolbar("downtoolbar"))
    , m_aDefaultStyles(aDefaultStyles)
    , m_nStyleIterator(0)
{
    for (unsigned int i = 0; i < STYLES_COUNT; i++)
    {
        auto aStyle
            = i < aDefaultStyles.size() ? aDefaultStyles[i] : std::pair<OUString, OUString>("", "");
        m_xStyleControllers[i].reset(new StyleItemController(aStyle, xDispatchProvider));

        OUString sIdOUString = "style" + OUString::number(i + 1);
        OString sId = OUStringToOString(sIdOUString, RTL_TEXTENCODING_ASCII_US);

        m_xStyleControllersWeld[i].reset(
            new weld::CustomWeld(xBuilder, sId, *m_xStyleControllers[i]));
        m_xStyleControllersWeld[i]->set_size_request(100, 60);
    }

    m_xUp->connect_clicked(LINK(this, StylesPreviewWindow_Base, GoUp));
    m_xDown->connect_clicked(LINK(this, StylesPreviewWindow_Base, GoDown));

    m_pStatusListener = new StyleStatusListener(this, xDispatchProvider);
    m_xStatusListener.set(static_cast<cppu::OWeakObject*>(m_pStatusListener), css::uno::UNO_QUERY);
}

StylesPreviewWindow_Base::~StylesPreviewWindow_Base()
{
    m_pStatusListener->UnBind();

    try
    {
        m_xStatusListener->dispose();
    }
    catch (css::uno::Exception&)
    {
    }

    m_pStatusListener = nullptr;
}

std::pair<OUString, OUString> StylesPreviewWindow_Base::GetVisibleStyle(unsigned nPosition)
{
    if (nPosition >= STYLES_COUNT || !m_aAllStyles.size())
        return std::make_pair<OUString, OUString>("", "");

    return m_aAllStyles[(m_nStyleIterator + nPosition) % m_aAllStyles.size()];
}

void StylesPreviewWindow_Base::Select(const OUString& rStyleName)
{
    m_sSelectedStyle = rStyleName;

    UpdateStylesList();
    MakeCurrentStyleVisible();
    Update();
}

void StylesPreviewWindow_Base::MakeCurrentStyleVisible()
{
    if (m_aAllStyles.size())
    {
        unsigned nNewIterator = m_nStyleIterator;
        auto aIt = m_aAllStyles.begin();
        auto aFound = m_aAllStyles.end();
        while (aIt != m_aAllStyles.end())
        {
            if (aIt->first == m_sSelectedStyle || aIt->second == m_sSelectedStyle)
            {
                aFound = aIt;
                break;
            }
            aIt++;
        }
        if (aFound != m_aAllStyles.end())
            nNewIterator = aFound - m_aAllStyles.begin();

        bool bIsAlreadyVisible
            = nNewIterator >= m_nStyleIterator % m_aAllStyles.size()
              && nNewIterator < m_nStyleIterator % m_aAllStyles.size() + STYLES_COUNT;
        if (!bIsAlreadyVisible)
            m_nStyleIterator = nNewIterator;
    }
}

void StylesPreviewWindow_Base::Update()
{
    UpdateStylesList();

    for (unsigned int i = 0; i < STYLES_COUNT; i++)
    {
        std::pair<OUString, OUString> sStyleName = GetVisibleStyle(i);
        m_xStyleControllers[i]->SetStyle(sStyleName);
        m_xStyleControllersWeld[i]->set_tooltip_text(sStyleName.second);

        if (sStyleName.first == m_sSelectedStyle || sStyleName.second == m_sSelectedStyle)
            m_xStyleControllers[i]->Select(true);
        else
            m_xStyleControllers[i]->Select(false);
    }
}

void StylesPreviewWindow_Base::UpdateStylesList()
{
    m_aAllStyles = m_aDefaultStyles;

    SfxObjectShell* pDocShell = SfxObjectShell::Current();
    const SfxStyleFamily eFamily = SfxStyleFamily::Para;
    SfxStyleSheetBasePool* pStyleSheetPool = nullptr;

    if (pDocShell)
        pStyleSheetPool = pDocShell->GetStyleSheetPool();

    if (pStyleSheetPool)
    {
        auto xIter = pStyleSheetPool->CreateIterator(eFamily, SfxStyleSearchBits::UserDefined);

        SfxStyleSheetBase* pStyle = xIter->First();

        while (pStyle)
        {
            m_aAllStyles.push_back(std::pair<OUString, OUString>("", pStyle->GetName()));
            pStyle = xIter->Next();
        }
    }
}

IMPL_LINK(StylesPreviewWindow_Base, GoUp, const OString&, /*rItem*/, void)
{
    if (m_nStyleIterator == 0)
        m_nStyleIterator = m_aAllStyles.size();
    else
        m_nStyleIterator--;

    Update();
}

IMPL_LINK(StylesPreviewWindow_Base, GoDown, const OString&, /*rItem*/, void)
{
    m_nStyleIterator++;
    Update();
}

StylesPreviewWindow_Impl::StylesPreviewWindow_Impl(
    vcl::Window* pParent, std::vector<std::pair<OUString, OUString>>& aDefaultStyles,
    css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider)
    : InterimItemWindow(pParent, "svx/ui/stylespreview.ui", "ApplyStyleBox",
                        reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
    , StylesPreviewWindow_Base(*m_xBuilder, aDefaultStyles, xDispatchProvider)
{
    SetOptimalSize();
}

StylesPreviewWindow_Impl::~StylesPreviewWindow_Impl() { disposeOnce(); }

void StylesPreviewWindow_Impl::dispose()
{
    m_xUp.reset();
    m_xDown.reset();

    for (unsigned int i = 0; i < STYLES_COUNT; i++)
        m_xStyleControllersWeld[i].reset();

    InterimItemWindow::dispose();
}

void StylesPreviewWindow_Impl::SetOptimalSize() { SetSizePixel(get_preferred_size()); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */