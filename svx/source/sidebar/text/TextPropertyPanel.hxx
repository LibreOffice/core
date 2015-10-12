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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTPROPERTYPANEL_HXX

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svtools/ctrlbox.hxx>
#include <editeng/fhgtitem.hxx>

#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/PanelLayout.hxx>

class ToolBox;

namespace svx { namespace sidebar {

class PopupControl;
class PopupContainer;

class TextPropertyPanel
    : public PanelLayout,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~TextPropertyPanel();
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const ::sfx2::sidebar::EnumContext& rContext);

    virtual void DataChanged (const DataChangedEvent& rEvent) override;

    ::sfx2::sidebar::ControllerItem& GetSpaceController() { return maSpacingControl;}
    long GetSelFontSize();
    void EndSpacingPopupMode();
    void EndUnderlinePopupMode();
    Color& GetUnderlineColor() { return meUnderlineColor;}


    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext& rContext) override;


    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    TextPropertyPanel (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const ::sfx2::sidebar::EnumContext& rContext);

private:
    //ui controls
    VclPtr<ToolBox> mpToolBoxFont;
    VclPtr<ToolBox> mpToolBoxIncDec;
    VclPtr<ToolBox> mpToolBoxSpacing;
    VclPtr<ToolBox> mpToolBoxFontColorSw;
    VclPtr<ToolBox> mpToolBoxFontColor;
    VclPtr<ToolBox> mpToolBoxBackgroundColor;

    //control items
    ::sfx2::sidebar::ControllerItem maFontSizeControl;
    ::sfx2::sidebar::ControllerItem maUnderlineControl;
    ::sfx2::sidebar::ControllerItem maSpacingControl;

    FontUnderline               meUnderline;
    Color                       meUnderlineColor;
    bool                        mbKernAvailable;
    bool                        mbKernLBAvailable;
    long                        mlKerning;
    SvxFontHeightItem*          mpHeightItem;

    TextCharacterSpacingPopup maCharSpacePopup;
    TextUnderlinePopup maUnderlinePopup;

    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;

    VclPtr<PopupControl> CreateCharacterSpacingControl (PopupContainer* pParent);
    VclPtr<PopupControl> CreateUnderlinePopupControl (PopupContainer* pParent);
    DECL_LINK_TYPED(SpacingClickHdl, ToolBox*, void);
    DECL_LINK_TYPED(UnderlineClickHdl, ToolBox*, void);

    void SetupToolboxItems();
    void InitToolBoxFont();
    void InitToolBoxSpacing();
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
