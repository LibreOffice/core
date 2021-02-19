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
#ifndef INCLUDED_SFX2_SIDEBAR_SIDEBARTOOLBOX_HXX
#define INCLUDED_SFX2_SIDEBAR_SIDEBARTOOLBOX_HXX

#include <config_options.h>
#include <sfx2/dllapi.h>
#include <vcl/builder.hxx>
#include <vcl/toolbox.hxx>
#include <map>

namespace com::sun::star::frame { class XToolbarController; }

namespace sfx2::sidebar {

/** The sidebar tool box has two responsibilities:
    1. Coordinated location, size, and other states with its parent
       background window.
    2. Create and handle tool bar controller for its items.
*/
class SidebarToolBox : public ToolBox
{
public:
    SidebarToolBox(vcl::Window* pParentWindow);
    virtual ~SidebarToolBox() override;
    virtual void dispose() override;

    virtual ToolBoxButtonSize GetDefaultButtonSize() const;

    using ToolBox::InsertItem;
    virtual void InsertItem(const OUString& rCommand,
            const css::uno::Reference<css::frame::XFrame>& rFrame,
            ToolBoxItemBits nBits,
            const Size& rRequestedSize,
            ImplToolItems::size_type nPos = APPEND) override;

    virtual bool EventNotify(NotifyEvent& rEvent) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    void InitToolBox(VclBuilder::stringmap& rMap);

protected:
    typedef std::map<sal_uInt16, css::uno::Reference<css::frame::XToolbarController>> ControllerContainer;
    ControllerContainer maControllers;
    bool mbAreHandlersRegistered;
    bool mbUseDefaultButtonSize;
    bool mbSideBar;

    DECL_LINK(DropDownClickHandler, ToolBox*, void);
    DECL_LINK(ClickHandler, ToolBox*, void);
    DECL_LINK(DoubleClickHandler, ToolBox*, void);
    DECL_LINK(SelectHandler, ToolBox*, void);
    DECL_LINK(ChangedIconSizeHandler, LinkParamNone*, void );

    css::uno::Reference<css::frame::XToolbarController> GetControllerForItemId(const sal_uInt16 nItemId) const;

    void CreateController(const sal_uInt16 nItemId,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                          const sal_Int32 nItemWidth, bool bSideBar);
    void RegisterHandlers();
};


} // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
