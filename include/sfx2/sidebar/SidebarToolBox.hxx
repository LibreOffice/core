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

#include <sfx2/dllapi.h>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <vcl/toolbox.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/util/URL.hpp>
#include <map>


namespace sfx2 { namespace sidebar {

/** The sidebar tool box has two responsibilities:
    1. Coordinated location, size, and other states with its parent
       background window.
    2. Create and handle tool bar controller for its items.
*/
class SFX2_DLLPUBLIC SidebarToolBox : public ToolBox
{
public:
    SidebarToolBox(vcl::Window* pParentWindow);
    virtual ~SidebarToolBox();
    virtual void dispose() override;

    using ToolBox::InsertItem;
    virtual void InsertItem(const OUString& rCommand,
            const css::uno::Reference<css::frame::XFrame>& rFrame,
            ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
            const Size& rRequestedSize = Size(),
            sal_uInt16 nPos = TOOLBOX_APPEND) override;

    virtual void Paint (vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;

    virtual bool Notify (NotifyEvent& rEvent) override;

    css::uno::Reference<css::frame::XToolbarController> GetControllerForItemId(const sal_uInt16 nItemId) const;
    sal_uInt16 GetItemIdForSubToolbarName (const OUString& rsCOmmandName) const;

    void SetController(const sal_uInt16 nItemId,
                       const css::uno::Reference<css::frame::XToolbarController>& rxController,
                       const OUString& rsCommandName);

    css::uno::Reference<css::frame::XToolbarController> GetFirstController();

private:
    Image maItemSeparator;

    class ItemDescriptor
    {
    public:
        css::uno::Reference<css::frame::XToolbarController> mxController;
        css::util::URL maURL;
        rtl::OUString msCurrentCommand;
    };

    typedef std::map<sal_uInt16, ItemDescriptor> ControllerContainer;
    ControllerContainer maControllers;
    bool mbAreHandlersRegistered;

    DECL_LINK_TYPED(DropDownClickHandler, ToolBox*, void);
    DECL_LINK_TYPED(ClickHandler, ToolBox*, void);
    DECL_LINK_TYPED(DoubleClickHandler, ToolBox*, void);
    DECL_LINK_TYPED(SelectHandler, ToolBox*, void);

    void CreateController(const sal_uInt16 nItemId,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                          const sal_Int32 nItemWidth = 0);
    void RegisterHandlers();
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
