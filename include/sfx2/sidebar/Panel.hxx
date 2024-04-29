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
#pragma once

#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>
#include <vector>

namespace com::sun::star::frame
{
class XFrame;
}
namespace com::sun::star::ui
{
class XSidebarPanel;
}
namespace com::sun::star::ui
{
class XUIElement;
}
namespace com::sun::star::awt
{
class XWindow;
}

namespace sfx2::sidebar
{
class Context;
class Deck;
class PanelDescriptor;
class PanelTitleBar;

/**
 * Multiple panels form a single deck.
 * E.g. the Properties deck has panels like Styles, Character, paragraph.
 */
class SFX2_DLLPUBLIC Panel final
{
public:
    SAL_DLLPRIVATE Panel(const PanelDescriptor& rPanelDescriptor, weld::Widget* pParentWindow,
                         const bool bIsInitiallyExpanded, Deck* pDeck,
                         std::function<Context()> aContextAccess,
                         const css::uno::Reference<css::frame::XFrame>& rxFrame);

    SAL_DLLPRIVATE ~Panel();

    SAL_DLLPRIVATE PanelTitleBar* GetTitleBar() const;
    SAL_DLLPRIVATE weld::Box* GetContents() const;
    SAL_DLLPRIVATE void Show(bool bShow);
    bool IsTitleBarOptional() const { return mbIsTitleBarOptional; }
    SAL_DLLPRIVATE void SetUIElement(const css::uno::Reference<css::ui::XUIElement>& rxElement);
    const css::uno::Reference<css::ui::XSidebarPanel>& GetPanelComponent() const
    {
        return mxPanelComponent;
    }
    SAL_DLLPRIVATE const css::uno::Reference<css::awt::XWindow>& GetElementParentWindow();
    SAL_DLLPRIVATE css::uno::Reference<css::awt::XWindow> GetElementWindow();
    SAL_DLLPRIVATE void SetExpanded(const bool bIsExpanded);
    bool IsExpanded() const { return mbIsExpanded; }
    SAL_DLLPRIVATE bool HasIdPredicate(std::u16string_view rsId) const;
    const OUString& GetId() const { return msPanelId; }
    const OUString& GetTitle() const { return msTitle; }
    void TriggerDeckLayouting();

    SAL_DLLPRIVATE void SetHeightPixel(int nHeight);

    SAL_DLLPRIVATE bool get_extents(tools::Rectangle& rExtents) const;

    /// Set whether a panel should be present but invisible / inactive
    SAL_DLLPRIVATE void SetLurkMode(bool bLurk);
    bool IsLurking() const { return mbLurking; }

    SAL_DLLPRIVATE void set_margin_top(int nMargin);
    SAL_DLLPRIVATE void set_margin_bottom(int nMargin);
    SAL_DLLPRIVATE void set_vexpand(bool bExpand);

    SAL_DLLPRIVATE weld::Window* GetFrameWeld();

    SAL_DLLPRIVATE void DataChanged();

private:
    std::unique_ptr<weld::Builder> mxBuilder;
    const OUString msPanelId;
    const OUString msTitle;
    const bool mbIsTitleBarOptional;
    const bool mbWantsAWT;
    css::uno::Reference<css::ui::XUIElement> mxElement;
    css::uno::Reference<css::ui::XSidebarPanel> mxPanelComponent;
    bool mbIsExpanded;
    bool mbLurking;
    const std::function<Context()> maContextAccess;
    const css::uno::Reference<css::frame::XFrame>& mxFrame;
    weld::Widget* mpParentWindow;
    VclPtr<Deck> mxDeck;
    std::unique_ptr<weld::Box> mxContainer;
    std::unique_ptr<PanelTitleBar> mxTitleBar;
    std::unique_ptr<weld::Box> mxContents;
    css::uno::Reference<css::awt::XWindow> mxXWindow;

    DECL_DLLPRIVATE_LINK(DumpAsPropertyTreeHdl, tools::JsonWriter&, void);
};

typedef std::vector<std::shared_ptr<Panel>> SharedPanelContainer;

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
