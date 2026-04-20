/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/base64.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>
#include <svl/itemset.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/tplpitem.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/glyphitemcache.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

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

#include <i18nlangtag/mslangid.hxx>

#include <svx/xfillit0.hxx>
#include <svx/xdef.hxx>
#include <svx/xflclit.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <vcl/commandevent.hxx>

namespace
{
class StylePreviewCache
{
private:
    class JsonStylePreviewCacheClear final : public Timer
    {
    public:
        JsonStylePreviewCacheClear()
            : Timer("Json Style Preview Cache clear callback")
        {
            // a generous 30 secs
            SetTimeout(30000);
            SetStatic();
        }
        virtual void Invoke() override { StylePreviewCache::gJsonStylePreviewCache.clear(); }
    };

    static std::map<int, std::map<OUString, Bitmap>> gStylePreviewCache;
    static std::map<int, std::map<OUString, OString>> gJsonStylePreviewCache;
    static int gStylePreviewCacheClients;
    static JsonStylePreviewCacheClear gJsonIdleClear;

public:
    static std::map<int, std::map<OUString, Bitmap>>& Get() { return gStylePreviewCache; }
    static std::map<int, std::map<OUString, OString>>& GetJson() { return gJsonStylePreviewCache; }

    static void ClearCache(bool bHard)
    {
        gStylePreviewCache.clear();
        if (bHard)
        {
            gJsonStylePreviewCache.clear();
            gJsonIdleClear.Stop();
        }
        else
        {
            // tdf#155720 don't immediately clear the json representation
            gJsonIdleClear.Start();
        }
    }

    static void RegisterClient()
    {
        if (!gStylePreviewCacheClients)
            gJsonIdleClear.Stop();
        gStylePreviewCacheClients++;
    }

    static void UnregisterClient()
    {
        gStylePreviewCacheClients--;
        if (!gStylePreviewCacheClients)
            ClearCache(false);
    }
};

std::map<int, std::map<OUString, Bitmap>> StylePreviewCache::gStylePreviewCache;
std::map<int, std::map<OUString, OString>> StylePreviewCache::gJsonStylePreviewCache;
int StylePreviewCache::gStylePreviewCacheClients;
StylePreviewCache::JsonStylePreviewCacheClear StylePreviewCache::gJsonIdleClear;
}

StyleStatusListener::StyleStatusListener(
    StylesPreviewWindow_Base* pPreviewControl,
    const css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider)
    : SfxStatusListener(xDispatchProvider, SID_STYLE_FAMILY2, u".uno:ParaStyle"_ustr)
    , m_pPreviewControl(pPreviewControl)
{
    ReBind();
}

void StyleStatusListener::StateChangedAtStatusListener(SfxItemState /*eState*/,
                                                       const SfxPoolItem* pState)
{
    const SfxTemplateItem* pStateItem = dynamic_cast<const SfxTemplateItem*>(pState);
    if (pStateItem)
    {
        if (pStateItem->GetStyleIdentifier().isEmpty())
            m_pPreviewControl->Select(pStateItem->GetStyleName());
        else
            m_pPreviewControl->Select(pStateItem->GetStyleIdentifier());
    }
}

StylePoolChangeListener::StylePoolChangeListener(StylesPreviewWindow_Base* pPreviewControl)
    : m_pPreviewControl(pPreviewControl)
{
    SfxObjectShell* pDocShell = SfxObjectShell::Current();

    m_pStyleSheetPool = pDocShell ? pDocShell->GetStyleSheetPool() : nullptr;

    if (m_pStyleSheetPool)
    {
        StartListening(*m_pStyleSheetPool);
    }
}

StylePoolChangeListener::~StylePoolChangeListener()
{
    if (m_pStyleSheetPool)
        EndListening(*m_pStyleSheetPool);
}

void StylePoolChangeListener::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::StyleSheetModified
        || rHint.GetId() == SfxHintId::StyleSheetModifiedExtended)
        StylePreviewCache::ClearCache(true);
    m_pPreviewControl->RequestStylesListUpdate();
}

StyleItemController::StyleItemController(StylePreviewDescriptor aStyleName)
    : m_eStyleFamily(SfxStyleFamily::Para)
    , m_aStyleName(std::move(aStyleName))
{
}

void StyleItemController::Paint(vcl::RenderContext& rRenderContext)
{
    auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::FILLCOLOR | vcl::PushFlags::FONT
                                           | vcl::PushFlags::TEXTCOLOR);

    DrawEntry(rRenderContext);
}

bool StylesPreviewWindow_Base::Command(const CommandEvent& rEvent)
{
    if (rEvent.GetCommand() != CommandEventId::ContextMenu)
        return false;

    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(m_xStylesView.get(), u"svx/ui/stylemenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));
    OUString rIdent = xMenu->popup_at_rect(m_xStylesView.get(),
                                           tools::Rectangle(rEvent.GetMousePosPixel(), Size(1, 1)));
    if (rIdent == "update" || rIdent == "edit")
    {
        css::uno::Sequence<css::beans::PropertyValue> aArgs(0);

        const css::uno::Reference<css::frame::XDispatchProvider> xProvider(m_xFrame,
                                                                           css::uno::UNO_QUERY);
        SfxToolBoxControl::Dispatch(
            xProvider,
            rIdent == "update" ? u".uno:StyleUpdateByExample"_ustr : u".uno:EditStyle"_ustr, aArgs);

        return true;
    }

    return false;
}

static Color GetTextColorFromItemSet(std::optional<SfxItemSet> const& pItemSet)
{
    if (const SvxColorItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_COLOR))
        return pItem->GetValue();

    return COL_AUTO;
}

static Color GetHighlightColorFromItemSet(std::optional<SfxItemSet> const& pItemSet)
{
    if (const SvxBrushItem* pItem = pItemSet->GetItem(SID_ATTR_BRUSH_CHAR))
        return pItem->GetColor();

    return COL_AUTO;
}

static Color GetBackgroundColorFromItemSet(std::optional<SfxItemSet> const& pItemSet)
{
    if (const XFillColorItem* pItem = pItemSet->GetItem(XATTR_FILLCOLOR))
        return pItem->GetColorValue();

    return COL_AUTO;
}

static css::drawing::FillStyle GetFillStyleFromItemSet(std::optional<SfxItemSet> const& pItemSet)
{
    if (const XFillStyleItem* pItem = pItemSet->GetItem(XATTR_FILLSTYLE))
        return pItem->GetValue();

    return css::drawing::FillStyle_NONE;
}

static SvxFont GetFontFromItems(const SvxFontItem* pFontItem, Size aPixelFontSize,
                                std::optional<SfxItemSet> const& pItemSet)
{
    SvxFont aFont;

    aFont.SetFamilyName(pFontItem->GetFamilyName());
    aFont.SetStyleName(pFontItem->GetStyleName());
    aFont.SetFontSize(aPixelFontSize);

    if (const SvxWeightItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_WEIGHT))
        aFont.SetWeight(pItem->GetWeight());

    if (const SvxPostureItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_POSTURE))
        aFont.SetItalic(pItem->GetPosture());

    if (const SvxContourItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_CONTOUR))
        aFont.SetOutline(pItem->GetValue());

    if (const SvxShadowedItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_SHADOWED))
        aFont.SetShadow(pItem->GetValue());

    if (const SvxCharReliefItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_RELIEF))
        aFont.SetRelief(pItem->GetValue());

    if (const SvxUnderlineItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE))
        aFont.SetUnderline(pItem->GetLineStyle());

    if (const SvxOverlineItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE))
        aFont.SetOverline(pItem->GetValue());

    if (const SvxCrossedOutItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_STRIKEOUT))
        aFont.SetStrikeout(pItem->GetStrikeout());

    if (const SvxCaseMapItem* pItem = pItemSet->GetItem(SID_ATTR_CHAR_CASEMAP))
        aFont.SetCaseMap(pItem->GetCaseMap());

    if (const SvxEmphasisMarkItem* pEmphasisItem = pItemSet->GetItem(SID_ATTR_CHAR_EMPHASISMARK))
        aFont.SetEmphasisMark(pEmphasisItem->GetEmphasisMark());

    return aFont;
}

void StyleItemController::DrawEntry(vcl::RenderContext& rRenderContext)
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if (!pShell)
        return;

    SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
    SfxStyleSheetBase* pStyle = nullptr;

    if (!pPool)
        return;

    pStyle = pPool->First(m_eStyleFamily);
    while (pStyle && pStyle->GetName() != m_aStyleName.commonName
           && pStyle->GetName() != m_aStyleName.translatedName)
        pStyle = pPool->Next();

    if (!pStyle)
        return;

    Size aSize(rRenderContext.GetOutputSizePixel());
    tools::Rectangle aFullRect(Point(0, 0), aSize);
    tools::Rectangle aContentRect(aFullRect);

    Color aOriginalColor = rRenderContext.GetFillColor();
    Color aOriginalLineColor = rRenderContext.GetLineColor();

    DrawContentBackground(rRenderContext, aContentRect, aOriginalColor);

    std::optional<SfxItemSet> const pItemSet(pStyle->GetItemSetForPreview());
    if (!pItemSet)
        return;

    Color aFontHighlight = COL_AUTO;

    sal_Int16 nScriptType
        = MsLangId::getScriptType(Application::GetSettings().GetUILanguageTag().getLanguageType());

    sal_uInt16 nFontSlot = SID_ATTR_CHAR_FONT;
    if (nScriptType == css::i18n::ScriptType::ASIAN)
        nFontSlot = SID_ATTR_CHAR_CJK_FONT;
    else if (nScriptType == css::i18n::ScriptType::COMPLEX)
        nFontSlot = SID_ATTR_CHAR_CTL_FONT;

    const SvxFontItem* const pFontItem = pItemSet->GetItem<SvxFontItem>(nFontSlot);
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
}

void StyleItemController::DrawContentBackground(vcl::RenderContext& rRenderContext,
                                                const tools::Rectangle& rContentRect,
                                                const Color& rColor)
{
    rRenderContext.SetLineColor(rColor);
    rRenderContext.SetFillColor(rColor);
    rRenderContext.DrawRect(rContentRect);
}

void StyleItemController::DrawHighlight(vcl::RenderContext& rRenderContext, Color aFontBack)
{
    tools::Rectangle aTextRect;
    rRenderContext.GetTextBoundRect(aTextRect, m_aStyleName.translatedName);

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
    const SalLayoutGlyphs* layoutGlyphs = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(
        &rRenderContext, m_aStyleName.translatedName);
    tools::Rectangle aTextRect;
    rRenderContext.GetTextBoundRect(aTextRect, m_aStyleName.translatedName, 0, 0, -1, 0, {}, {},
                                    layoutGlyphs);

    Point aPos(0, 0);
    aPos.AdjustX(LEFT_MARGIN);
    aPos.AdjustY((rRenderContext.GetOutputHeightPixel() - aTextRect.Bottom()) / 2);

    rRenderContext.DrawText(aPos, m_aStyleName.translatedName, 0, -1, nullptr, nullptr,
                            layoutGlyphs);
}

StylesPreviewWindow_Base::StylesPreviewWindow_Base(
    weld::Builder& xBuilder, const StylePreviewList& rDefaultStyles,
    const css::uno::Reference<css::frame::XFrame>& xFrame)
    : m_xFrame(xFrame)
    , m_xStylesView(xBuilder.weld_icon_view(u"stylesview"_ustr))
    , m_aUpdateTask(*this)
    , m_aDefaultStyles(rDefaultStyles)
{
    StylePreviewCache::RegisterClient();

    m_xStylesView->connect_selection_changed(LINK(this, StylesPreviewWindow_Base, Selected));
    m_xStylesView->connect_item_activated(LINK(this, StylesPreviewWindow_Base, DoubleClick));
    m_xStylesView->connect_command(LINK(this, StylesPreviewWindow_Base, DoCommand));
    m_xStylesView->connect_get_image(LINK(this, StylesPreviewWindow_Base, GetPreviewImage));

    const css::uno::Reference<css::frame::XDispatchProvider> xProvider(m_xFrame,
                                                                       css::uno::UNO_QUERY);
    m_xStatusListener = new StyleStatusListener(this, xProvider);

    m_pStylePoolChangeListener.reset(new StylePoolChangeListener(this));

    RequestStylesListUpdate();
}

IMPL_LINK(StylesPreviewWindow_Base, Selected, weld::IconView&, rIconView, void)
{
    OUString sStyleName = rIconView.get_selected_text();

    css::uno::Sequence<css::beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"Template"_ustr, sStyleName),
        comphelper::makePropertyValue(u"Family"_ustr, sal_Int16(SfxStyleFamily::Para))
    };
    const css::uno::Reference<css::frame::XDispatchProvider> xProvider(m_xFrame,
                                                                       css::uno::UNO_QUERY);
    SfxToolBoxControl::Dispatch(xProvider, u".uno:StyleApply"_ustr, aArgs);
}

IMPL_LINK(StylesPreviewWindow_Base, DoubleClick, weld::IconView&, rIconView, bool)
{
    OUString sStyleName = rIconView.get_selected_text();

    css::uno::Sequence<css::beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"Param"_ustr, sStyleName),
        comphelper::makePropertyValue(u"Family"_ustr, sal_Int16(SfxStyleFamily::Para))
    };
    const css::uno::Reference<css::frame::XDispatchProvider> xProvider(m_xFrame,
                                                                       css::uno::UNO_QUERY);
    SfxToolBoxControl::Dispatch(xProvider, u".uno:EditStyle"_ustr, aArgs);

    return true;
}

IMPL_LINK(StylesPreviewWindow_Base, DoCommand, const CommandEvent&, rPos, bool)
{
    return Command(rPos);
}

StylesPreviewWindow_Base::~StylesPreviewWindow_Base()
{
    m_xStatusListener->UnBind();

    m_aUpdateTask.Stop();

    StylePreviewCache::UnregisterClient();

    try
    {
        m_xStatusListener->dispose();
    }
    catch (css::uno::Exception&)
    {
    }

    m_xStatusListener = nullptr;
}

void StylesPreviewWindow_Base::Select(const OUString& rStyleName)
{
    m_sSelectedStyle = rStyleName;

    UpdateSelection();
}

void StylesPreviewWindow_Base::UpdateSelection()
{
    if (!m_xStylesView || !m_xStylesView->n_children())
        return;

    for (StylePreviewList::size_type i = 0; i < m_aAllStyles.size(); ++i)
    {
        if (m_aAllStyles[i].commonName == m_sSelectedStyle
            || m_aAllStyles[i].translatedName == m_sSelectedStyle)
        {
            m_xStylesView->select(i);
            break;
        }
    }
}

void StylesPreviewWindow_Base::RequestStylesListUpdate() { m_aUpdateTask.Start(); }

void StylesListUpdateTask::Invoke()
{
    m_rStylesList.UpdateStylesList();
    m_rStylesList.UpdateSelection();
}

static OString extractPngString(const Bitmap& rBitmap)
{
    SvMemoryStream aOStm(65535, 65535);
    // Use fastest compression "1"
    css::uno::Sequence<css::beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Compression"_ustr, sal_Int32(1)),
    };
    vcl::PngImageWriter aPNGWriter(aOStm);
    aPNGWriter.setParameters(aFilterData);
    if (aPNGWriter.write(rBitmap))
    {
        css::uno::Sequence<sal_Int8> aSeq(static_cast<sal_Int8 const*>(aOStm.GetData()),
                                          aOStm.Tell());
        OStringBuffer aBuffer("data:image/png;base64,");
        ::comphelper::Base64::encode(aBuffer, aSeq);
        return aBuffer.makeStringAndClear();
    }

    return ""_ostr;
}

// 0: OUString, 1: TreeIter, 2: dpiscale, returns true if supported
IMPL_LINK(StylesPreviewWindow_Base, GetPreviewImage, const weld::encoded_image_query&, rQuery, bool)
{
    const weld::TreeIter& rIter = std::get<1>(rQuery);
    int nDpiScale = std::get<2>(rQuery);
    if (nDpiScale <= 0)
        nDpiScale = 100;
    OUString sStyleId(m_xStylesView->get_id(rIter));
    OUString sStyleName(m_xStylesView->get_text(rIter));
    OString sBase64Png(GetCachedPreviewJson({ sStyleId, sStyleName }, nDpiScale));
    if (sBase64Png.isEmpty())
        return false;

    OUString& rResult = std::get<0>(rQuery);
    rResult = OStringToOUString(sBase64Png, RTL_TEXTENCODING_ASCII_US);

    return true;
}

Bitmap StylesPreviewWindow_Base::GetCachedPreview(const StylePreviewDescriptor& rStyle,
                                                  int nDpiScale)
{
    auto& rDpiCache = StylePreviewCache::Get()[nDpiScale];
    auto aFound = rDpiCache.find(rStyle.translatedName);
    if (aFound != rDpiCache.end())
        return aFound->second;

    ScopedVclPtrInstance<VirtualDevice> pImg;
    const Size aSize(100 * nDpiScale / 100, 30 * nDpiScale / 100);
    pImg->SetDPIX(96.0 * nDpiScale / 100);
    pImg->SetDPIY(96.0 * nDpiScale / 100);
    pImg->SetOutputSizePixel(aSize);

    StyleItemController aStyleController(rStyle);
    aStyleController.Paint(*pImg);
    Bitmap aBitmap(pImg->GetBitmap(Point(0, 0), aSize));
    rDpiCache[rStyle.translatedName] = aBitmap;

    return aBitmap;
}

OString StylesPreviewWindow_Base::GetCachedPreviewJson(const StylePreviewDescriptor& rStyle,
                                                       int nDpiScale)
{
    auto& rDpiJsonCache = StylePreviewCache::GetJson()[nDpiScale];
    auto aJsonFound = rDpiJsonCache.find(rStyle.translatedName);
    if (aJsonFound != rDpiJsonCache.end())
        return aJsonFound->second;

    Bitmap aBitmap = GetCachedPreview(rStyle, nDpiScale);
    OString sResult = extractPngString(aBitmap);
    rDpiJsonCache[rStyle.translatedName] = sResult;
    return sResult;
}

namespace
{
struct StylePaneFormatFilter
{
    bool bValid = false;
    bool bAllStyles = false;
    bool bCustomStyles = false;
    bool bStylesInUse = false;
};

StylePaneFormatFilter lcl_GetStylePaneFormatFilter(SfxObjectShell* pDocShell)
{
    StylePaneFormatFilter aFilter;
    if (!pDocShell)
        return aFilter;

    try
    {
        css::uno::Reference<css::beans::XPropertySet> xDocProps(pDocShell->GetModel(),
                                                                css::uno::UNO_QUERY);
        if (!xDocProps.is()
            || !xDocProps->getPropertySetInfo()->hasPropertyByName(u"InteropGrabBag"_ustr))
            return aFilter;

        comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue(u"InteropGrabBag"_ustr));
        auto it = aGrabBag.find(u"StylePaneFormatFilterProps"_ustr);
        if (it == aGrabBag.end())
            return aFilter;

        css::uno::Sequence<css::beans::PropertyValue> aFilterProps;
        it->second >>= aFilterProps;

        for (const auto& rProp : aFilterProps)
        {
            if (rProp.Name == "allStyles")
                rProp.Value >>= aFilter.bAllStyles;
            else if (rProp.Name == "customStyles")
                rProp.Value >>= aFilter.bCustomStyles;
            else if (rProp.Name == "stylesInUse")
                rProp.Value >>= aFilter.bStylesInUse;
        }

        aFilter.bValid = true;
    }
    catch (const css::uno::Exception&)
    {
    }

    return aFilter;
}

inline void lcl_AppendParaStyle(StylePreviewList& rAllStyles, const OUString& rName)
{
    const auto aFound = std::find_if(
        rAllStyles.begin(), rAllStyles.end(), [&rName](const StylePreviewDescriptor& element) {
            return element.commonName == rName || element.translatedName == rName;
        });

    if (aFound == rAllStyles.end())
        rAllStyles.emplace_back<StylePreviewDescriptor>({ rName, rName });
}

void lcl_AppendParaStyles(StylePreviewList& rAllStyles, SfxStyleSheetBasePool* pPool,
                          SfxStyleSearchBits eBits)
{
    if (!pPool)
        return;

    auto xIter = pPool->CreateIterator(SfxStyleFamily::Para, eBits);
    for (SfxStyleSheetBase* pStyle = xIter->First(); pStyle; pStyle = xIter->Next())
        lcl_AppendParaStyle(rAllStyles, pStyle->GetName());
}

void lcl_AppendFilteredParaStyles(StylePreviewList& rAllStyles, SfxStyleSheetBasePool* pPool,
                                  const StylePaneFormatFilter& rFilter)
{
    if (!pPool)
        return;

    lcl_AppendParaStyles(rAllStyles, pPool, SfxStyleSearchBits::Favourite);

    auto xIter = pPool->CreateIterator(SfxStyleFamily::Para, SfxStyleSearchBits::AllVisible);
    for (SfxStyleSheetBase* pStyle = xIter->First(); pStyle; pStyle = xIter->Next())
    {
        bool bInclude = false;
        if (rFilter.bCustomStyles && pStyle->IsUserDefined())
            bInclude = true;
        if (rFilter.bStylesInUse && pStyle->IsUsed())
            bInclude = true;

        if (bInclude)
            lcl_AppendParaStyle(rAllStyles, pStyle->GetName());
    }
}
}

void StylesPreviewWindow_Base::UpdateStylesList()
{
    SfxObjectShell* pDocShell = SfxObjectShell::Current();
    SfxStyleSheetBasePool* pStyleSheetPool = nullptr;
    StylePaneFormatFilter aFilter;

    if (pDocShell)
    {
        pStyleSheetPool = pDocShell->GetStyleSheetPool();
        aFilter = lcl_GetStylePaneFormatFilter(pDocShell);
    }

    // When the document specifies a style pane filter, skip the hardcoded
    // default styles and let the filter control what is shown.
    if (aFilter.bValid)
        m_aAllStyles.clear();
    else
        m_aAllStyles = m_aDefaultStyles;

    if (pStyleSheetPool)
    {
        if (aFilter.bValid)
            lcl_AppendFilteredParaStyles(m_aAllStyles, pStyleSheetPool, aFilter);
        else
            lcl_AppendParaStyles(m_aAllStyles, pStyleSheetPool,
                                 SfxStyleSearchBits::Favourite | SfxStyleSearchBits::UserDefined);
    }

    m_xStylesView->freeze();
    m_xStylesView->clear();
    // for online we can skip inserting the preview into the IconView and rely
    // on DoJsonProperty to provide the image to clients
    const bool bNeedInsertPreview = !comphelper::COKit::isActive();
    for (const auto& rStyle : m_aAllStyles)
    {
        Bitmap aPreview = GetCachedPreview(rStyle);
        Bitmap* pPreview = bNeedInsertPreview ? &aPreview : nullptr;
        m_xStylesView->append(rStyle.commonName, rStyle.translatedName, pPreview);
    }
    m_xStylesView->thaw();
}

StylesPreviewWindow_Impl::StylesPreviewWindow_Impl(
    vcl::Window* pParent, const StylePreviewList& rDefaultStyles,
    const css::uno::Reference<css::frame::XFrame>& xFrame)
    : InterimItemWindow(pParent, u"svx/ui/stylespreview.ui"_ustr, u"ApplyStyleBox"_ustr, true,
                        reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
    , StylesPreviewWindow_Base(*m_xBuilder, rDefaultStyles, xFrame)
{
    SetOptimalSize();
}

StylesPreviewWindow_Impl::~StylesPreviewWindow_Impl() { disposeOnce(); }

void StylesPreviewWindow_Impl::dispose()
{
    m_xStylesView.reset();

    InterimItemWindow::dispose();
}

void StylesPreviewWindow_Impl::SetOptimalSize() { SetSizePixel(get_preferred_size()); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
