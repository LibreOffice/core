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
#ifndef SD_SIDEBAR_NAVIGATOR_WRAPPER_HXX
#define SD_SIDEBAR_NAVIGATOR_WRAPPER_HXX

#include <sfx2/sidebar/ILayoutableWindow.hxx>
#include <vcl/ctrl.hxx>
#include "navigatr.hxx"


class SfxBindings;
namespace sd { class ViewShellBase; }

namespace css = ::com::sun::star;

namespace sd { namespace sidebar {

/** Present the navigator as control that can be displayed inside the
    sidebar.
    This wrapper has two main responsibilities:
    - Watch for document changes and update the navigator when one
    happens.
    - Forward size changes from sidebar to navigator.
*/
class NavigatorWrapper
    : public Control,
      public sfx2::sidebar::ILayoutableWindow
{
public:
    NavigatorWrapper (
        ::Window* pParent,
        sd::ViewShellBase& rViewShellBase,
        SfxBindings* pBindings);

    virtual ~NavigatorWrapper (void);

    // Control
    virtual void Resize (void);
    virtual void GetFocus (void);

    // From ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth);

private:
    ViewShellBase& mrViewShellBase;
    SdNavigatorWin maNavigator;

    void UpdateNavigator (void);
};


} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
