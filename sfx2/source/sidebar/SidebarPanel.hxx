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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_SIDEBARPANEL_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_SIDEBARPANEL_HXX

#include <tools/link.hxx>
#include <com/sun/star/ui/XSidebarPanel.hpp>

#include <boost/noncopyable.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

namespace cssu = ::com::sun::star::uno;

namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::ui::XSidebarPanel
        > SidebarPanelInterfaceBase;
}


class DockingWindow;
class VclWindowEvent;

namespace sfx2 { namespace sidebar {

class Panel;

class SidebarPanel
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SidebarPanelInterfaceBase
{
public:
    static cssu::Reference<css::ui::XSidebarPanel> Create (Panel* pPanel);

protected:
    SidebarPanel(
        Panel* pPanel);
    virtual ~SidebarPanel (void);

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEventObject)
        throw(cssu::RuntimeException);

    virtual void SAL_CALL disposing (void) SAL_OVERRIDE;

private:
    Panel* mpPanel;
    cssu::Reference<css::rendering::XCanvas> mxCanvas;

    DECL_LINK(HandleWindowEvent, VclWindowEvent*);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
