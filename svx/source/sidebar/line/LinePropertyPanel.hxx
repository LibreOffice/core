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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANEL_HXX

#include <svx/xdash.hxx>
#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <memory>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/xtable.hxx>
#include "LineWidthPopup.hxx"


class XLineStyleItem;
class XLineDashItem;
class XLineStartItem;
class XLineEndItem;
class XLineEndList;
class XDashList;
class ListBox;
class ToolBox;
class FloatingWindow;

namespace
{

#define SIDEBAR_LINE_WIDTH_GLOBAL_VALUE "PopupPanel_LineWidth"

} //end of anonymous namespace

namespace svx
{
namespace sidebar
{

class PopupContainer;
class LineWidthControl;

class LinePropertyPanel : public PanelLayout,
                          public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~LinePropertyPanel();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) SAL_OVERRIDE;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) SAL_OVERRIDE;

    SfxBindings* GetBindings() { return mpBindings;}

    void SetWidth(long nWidth);
    void SetWidthIcon(int n);
    void SetWidthIcon();

    void EndLineWidthPopupMode();

    // constructor/destuctor
    LinePropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

private:
    //ui controls
    VclPtr<FixedText>   mpFTWidth;
    VclPtr<ToolBox>     mpTBWidth;
    VclPtr<FixedText>   mpFTColor;
    VclPtr<ListBox>     mpLBColor;
    VclPtr<FixedText>   mpFTStyle;
    VclPtr<ListBox>     mpLBStyle;
    VclPtr<FixedText>   mpFTTransparency;
    VclPtr<MetricField> mpMFTransparent;
    VclPtr<FixedText>   mpFTArrow;
    VclPtr<ListBox>     mpLBStart;
    VclPtr<ListBox>     mpLBEnd;
    VclPtr<FixedText>   mpFTEdgeStyle;
    VclPtr<ListBox>     mpLBEdgeStyle;
    VclPtr<FixedText>   mpFTCapStyle;
    VclPtr<ListBox>     mpLBCapStyle;

    //ControllerItem
    sfx2::sidebar::ControllerItem maStyleControl;
    sfx2::sidebar::ControllerItem maDashControl;
    sfx2::sidebar::ControllerItem maWidthControl;
    sfx2::sidebar::ControllerItem maStartControl;
    sfx2::sidebar::ControllerItem maEndControl;
    sfx2::sidebar::ControllerItem maLineEndListControl;
    sfx2::sidebar::ControllerItem maLineStyleListControl;
    sfx2::sidebar::ControllerItem maTransControl;
    sfx2::sidebar::ControllerItem maEdgeStyle;
    sfx2::sidebar::ControllerItem maCapStyle;

    std::unique_ptr<XLineStyleItem> mpStyleItem;
    std::unique_ptr<XLineDashItem>  mpDashItem;

    sal_uInt16      mnTrans;
    SfxMapUnit      meMapUnit;
    sal_Int32       mnWidthCoreValue;
    XLineEndListRef mxLineEndList;
    XDashListRef    mxLineStyleList;
    std::unique_ptr<XLineStartItem> mpStartItem;
    std::unique_ptr<XLineEndItem>   mpEndItem;

    //popup windows
    LineWidthPopup maLineWidthPopup;

    // images from resource
    Image maIMGNone;

    // multi-images
    std::unique_ptr<Image[]> mpIMGWidthIcon;

    css::uno::Reference<css::frame::XFrame> mxFrame;
    SfxBindings* mpBindings;

    /// bitfield
    bool                mbWidthValuable : 1;

    void Initialize();
    void FillLineEndList();
    void FillLineStyleList();
    void SelectEndStyle(bool bStart);
    void SelectLineStyle();
    void ActivateControls();

    DECL_LINK(ChangeLineStyleHdl, void*);
    DECL_LINK_TYPED(ToolboxWidthSelectHdl, ToolBox*, void);
    DECL_LINK(ChangeTransparentHdl , void *);
    DECL_LINK(ChangeStartHdl, void *);
    DECL_LINK(ChangeEndHdl, void *);
    DECL_LINK(ChangeEdgeStyleHdl, void *);
    DECL_LINK(ChangeCapStyleHdl, void *);

    VclPtr<PopupControl> CreateLineWidthPopupControl (PopupContainer* pParent);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
