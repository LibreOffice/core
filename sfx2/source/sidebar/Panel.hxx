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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_PANEL_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_PANEL_HXX

#include "Context.hxx"
#include <vcl/window.hxx>

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebarPanel.hpp>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

class PanelDescriptor;
class TitleBar;
class PanelTitleBar;


class Panel
    : public Window
{
public:
    Panel (
        const PanelDescriptor& rPanelDescriptor,
        Window* pParentWindow,
        const bool bIsInitiallyExpanded,
        const ::boost::function<void(void)>& rDeckLayoutTrigger,
        const ::boost::function<Context(void)>& rContextAccess);
    virtual ~Panel (void);

    void Dispose (void);

    PanelTitleBar* GetTitleBar (void) const;
    bool IsTitleBarOptional (void) const;
    void SetUIElement (const cssu::Reference<css::ui::XUIElement>& rxElement);
    cssu::Reference<css::ui::XSidebarPanel> GetPanelComponent (void) const;
    cssu::Reference<css::awt::XWindow> GetElementWindow (void);
    void SetExpanded (const bool bIsExpanded);
    bool IsExpanded (void) const;
    bool HasIdPredicate (const ::rtl::OUString& rsId) const;
    const ::rtl::OUString& GetId (void) const;

    virtual void Paint (const Rectangle& rUpdateArea);
    virtual void Resize (void);
    virtual void DataChanged (const DataChangedEvent& rEvent);
    virtual void Activate (void);

private:
    const ::rtl::OUString msPanelId;
    ::boost::scoped_ptr<PanelTitleBar> mpTitleBar;
    const bool mbIsTitleBarOptional;
    cssu::Reference<css::ui::XUIElement> mxElement;
    cssu::Reference<css::ui::XSidebarPanel> mxPanelComponent;
    bool mbIsExpanded;
    const ::boost::function<void(void)> maDeckLayoutTrigger;
    const ::boost::function<Context(void)> maContextAccess;
};
typedef ::boost::shared_ptr<Panel> SharedPanel;
typedef ::std::vector<SharedPanel> SharedPanelContainer;


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
