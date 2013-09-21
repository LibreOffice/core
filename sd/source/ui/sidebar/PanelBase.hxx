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
#ifndef SD_SIDEBAR_PANELS_PANEL_BASE_HXX
#define SD_SIDEBAR_PANELS_PANEL_BASE_HXX

#include "IDisposable.hxx"
#include "ISidebarReceiver.hxx"
#include <sfx2/sidebar/ILayoutableWindow.hxx>

#include <vcl/ctrl.hxx>

#include <boost/scoped_ptr.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sd {
    class ViewShellBase;
}




namespace sd { namespace sidebar {


class PanelBase
    : public Control,
      public sfx2::sidebar::ILayoutableWindow,
      public IDisposable,
      public ISidebarReceiver
{
public:
    PanelBase (
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase);
    virtual ~PanelBase (void);

    virtual void Resize (void);

    // IDisposable
    virtual void Dispose (void);

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth);

    // ISidebarReceiver
    virtual void SetSidebar (const cssu::Reference<css::ui::XSidebar>& rxSidebar);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

protected:
    ::boost::scoped_ptr< ::Window> mpWrappedControl;
    virtual ::Window* CreateWrappedControl (
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase) = 0;

private:
    cssu::Reference<css::ui::XSidebar> mxSidebar;
    ViewShellBase& mrViewShellBase;

    bool ProvideWrappedControl (void);
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
