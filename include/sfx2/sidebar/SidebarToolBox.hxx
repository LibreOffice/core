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
#ifndef SFX_SIDEBAR_TOOLBOX_HXX
#define SFX_SIDEBAR_TOOLBOX_HXX

#include "sfx2/dllapi.h"
#include "vcl/toolbox.hxx"
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/util/URL.hpp>
#include <map>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** The sidebar tool box has two responsibilities:
    1. Coordinated location, size, and other states with its parent
       background window.
    2. Create and handle tool bar controller for its items.
*/
class SFX2_DLLPUBLIC SidebarToolBox
    : public ToolBox
{
public:
    SidebarToolBox(Window* pParentWindow);
    virtual ~SidebarToolBox (void);

    using ToolBox::InsertItem;
    virtual void InsertItem(const OUString& rCommand,
            const com::sun::star::uno::Reference<com::sun::star::frame::XFrame>& rFrame,
            ToolBoxItemBits nBits = 0,
            const Size& rRequestedSize = Size(),
            sal_uInt16 nPos = TOOLBOX_APPEND);

    virtual void Paint (const Rectangle& rRect);

    virtual long Notify (NotifyEvent& rEvent);

    cssu::Reference<css::frame::XToolbarController> GetControllerForItemId (
        const sal_uInt16 nItemId) const;
    sal_uInt16 GetItemIdForSubToolbarName (
        const ::rtl::OUString& rsCOmmandName) const;

    void SetController (
        const sal_uInt16 nItemId,
        const cssu::Reference<css::frame::XToolbarController>& rxController,
        const ::rtl::OUString& rsCommandName);

private:
    Image maItemSeparator;
    class ItemDescriptor
    {
    public:
        cssu::Reference<css::frame::XToolbarController> mxController;
        css::util::URL maURL;
        rtl::OUString msCurrentCommand;
    };
    typedef ::std::map<sal_uInt16, ItemDescriptor> ControllerContainer;
    ControllerContainer maControllers;
    bool mbAreHandlersRegistered;

    DECL_LINK(DropDownClickHandler, ToolBox*);
    DECL_LINK(ClickHandler, ToolBox*);
    DECL_LINK(DoubleClickHandler, ToolBox*);
    DECL_LINK(SelectHandler, ToolBox*);
    DECL_LINK(ActivateToolBox, ToolBox*);
    DECL_LINK(DeactivateToolBox, ToolBox*);

    void CreateController (
        const sal_uInt16 nItemId,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        const sal_Int32 nItemWidth = 0);
    void RegisterHandlers (void);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
