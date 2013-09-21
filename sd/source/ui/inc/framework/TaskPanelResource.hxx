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
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include "SidebarPanelId.hxx"

#include <com/sun/star/drawing/framework/XResource.hpp>
#include <boost/scoped_ptr.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;
namespace cssdf = ::com::sun::star::drawing::framework;

class Window;

namespace sd { namespace sidebar {
    class SidebarViewShell;
} }


namespace sd { namespace framework {

typedef ::cppu::WeakComponentImplHelper1 <
    cssdf::XResource
    > TaskPanelResourceInterfaceBase;


/** A simple wrapper around a legacy task pane control that gives
    access to that control (via GetControl()).
*/
class TaskPanelResource
    : private ::cppu::BaseMutex,
      public TaskPanelResourceInterfaceBase
{
public:
    /** Create a resource object that represents the legacy taskpane
        panel.
        @param rxResourceId
            drawing framework resource id
        @param pControl
            The new TaskPanelResource object takes ownership for this control.
    */
    TaskPanelResource (
        sidebar::SidebarViewShell& rSidebarViewShell,
        sidebar::PanelId ePanelId,
        const cssu::Reference<cssdf::XResourceId>& rxResourceId);
    virtual ~TaskPanelResource (void);
    virtual void SAL_CALL disposing (void);

    // XResource
    virtual cssu::Reference<cssdf::XResourceId> SAL_CALL getResourceId (void) throw (cssu::RuntimeException);
    virtual sal_Bool SAL_CALL isAnchorOnly () throw (cssu::RuntimeException);

    ::Window* GetControl (void) const;

private:
    const cssu::Reference<cssdf::XResourceId> mxResourceId;
    // Using auto_ptr because it has release(), what scoped_ptr doesn't.
    ::std::auto_ptr< ::Window> mpControl;

    DECL_LINK(WindowEventHandler,VclWindowEvent*);
};

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
