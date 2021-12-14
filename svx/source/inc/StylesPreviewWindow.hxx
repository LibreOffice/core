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

#ifndef INCLUDED_SVX_SOURCE_INC_STYLES_PREVIEW_WINDOW_HXX
#define INCLUDED_SVX_SOURCE_INC_STYLES_PREVIEW_WINDOW_HXX

#include <vcl/InterimItemWindow.hxx>
#include <vcl/customweld.hxx>
#include <svl/style.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <sfx2/sfxstatuslistener.hxx>

class StylesPreviewWindow_Base;

/// Listener for style selection
class StyleStatusListener : public SfxStatusListener
{
    StylesPreviewWindow_Base* m_pPreviewControl;

public:
    StyleStatusListener(
        StylesPreviewWindow_Base* pPreviewControl,
        const css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider);

    void StateChanged(SfxItemState eState, const SfxPoolItem* pState) override;
};

/// Listener for styles creation or modification
class StylePoolChangeListener : public SfxListener
{
    StylesPreviewWindow_Base* m_pPreviewControl;
    SfxStyleSheetBasePool* m_pStyleSheetPool;

public:
    StylePoolChangeListener(StylesPreviewWindow_Base* pPreviewControl);
    ~StylePoolChangeListener();

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
};

class StyleItemController
{
    static constexpr unsigned LEFT_MARGIN = 8;

    SfxStyleFamily m_eStyleFamily;
    std::pair<OUString, OUString> m_aStyleName;

public:
    StyleItemController(const std::pair<OUString, OUString>& aStyleName);

    void Paint(vcl::RenderContext& rRenderContext);

    void SetStyle(const std::pair<OUString, OUString>& sStyleName);

    void Select(bool bSelect);

private:
    void DrawEntry(vcl::RenderContext& rRenderContext);
    void DrawText(vcl::RenderContext& rRenderContext);
    void DrawHighlight(vcl::RenderContext& rRenderContext, Color aFontBack);
    static void DrawSelection(vcl::RenderContext& rRenderContext);
    static void DrawContentBackground(vcl::RenderContext& rRenderContext,
                                      const tools::Rectangle& aContentRect, const Color& aColor);
};

class StylesListUpdateTask : public Idle
{
    StylesPreviewWindow_Base& m_rStylesList;

public:
    StylesListUpdateTask(StylesPreviewWindow_Base& rStylesList)
        : m_rStylesList(rStylesList)
    {
    }

    virtual void Invoke() override;
};

class StylesPreviewWindow_Base
{
    friend class StylesListUpdateTask;

protected:
    static constexpr unsigned STYLES_COUNT = 6;

    css::uno::Reference<css::frame::XDispatchProvider> m_xDispatchProvider;

    std::unique_ptr<weld::IconView> m_xStylesView;

    StylesListUpdateTask m_aUpdateTask;

    StyleStatusListener* m_pStatusListener;
    css::uno::Reference<css::lang::XComponent> m_xStatusListener;
    std::unique_ptr<StylePoolChangeListener> m_pStylePoolChangeListener;

    std::vector<std::pair<OUString, OUString>> m_aDefaultStyles;
    std::vector<std::pair<OUString, OUString>> m_aAllStyles;

    OUString m_sSelectedStyle;

    DECL_LINK(Selected, weld::IconView&, void);
    DECL_LINK(DoubleClick, weld::IconView&, bool);
    DECL_LINK(DoCommand, const CommandEvent&, bool);

public:
    StylesPreviewWindow_Base(
        weld::Builder& xBuilder, std::vector<std::pair<OUString, OUString>>& aDefaultStyles,
        const css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider);
    ~StylesPreviewWindow_Base();

    void Select(const OUString& rStyleName);
    void RequestStylesListUpdate();

private:
    void UpdateStylesList();
    void UpdateSelection();
    bool Command(const CommandEvent& rEvent);
};

class StylesPreviewWindow_Impl : public InterimItemWindow, public StylesPreviewWindow_Base
{
public:
    StylesPreviewWindow_Impl(
        vcl::Window* pParent, std::vector<std::pair<OUString, OUString>>& aDefaultStyles,
        const css::uno::Reference<css::frame::XDispatchProvider>& xDispatchProvider);
    ~StylesPreviewWindow_Impl();

    void dispose();

    void SetOptimalSize();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
