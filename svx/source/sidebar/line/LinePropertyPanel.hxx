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
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <memory>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/xtable.hxx>
#include <svx/sidebar/LinePropertyPanelBase.hxx>


class XLineStyleItem;
class XLineDashItem;
class XLineStartItem;
class XLineEndItem;
class XLineEndList;
class XDashList;
class ListBox;
class ToolBox;
class FloatingWindow;

#define SIDEBAR_LINE_WIDTH_GLOBAL_VALUE "PopupPanel_LineWidth"

namespace svx
{
namespace sidebar
{

class LinePropertyPanel : public LinePropertyPanelBase,
                          public sfx2::sidebar::IContextChangeReceiver,
                          public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~LinePropertyPanel() override;
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destructor
    LinePropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void setLineWidth(const XLineWidthItem& rItem) override;

protected:

    virtual void setLineStyle(const XLineStyleItem& rItem) override;
    virtual void setLineDash(const XLineDashItem& rItem) override;
    virtual void setLineEndStyle(const XLineEndItem* pItem) override;
    virtual void setLineStartStyle(const XLineStartItem* pItem) override;
    virtual void setLineTransparency(const XLineTransparenceItem& rItem) override;
    virtual void setLineJoint(const XLineJointItem* pItem) override;
    virtual void setLineCap(const XLineCapItem* pItem) override;

private:
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

    SfxBindings* mpBindings;
    vcl::EnumContext maContext;
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
