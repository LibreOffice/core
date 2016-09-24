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
#ifndef INCLUDED_SFX2_SIDEBAR_SIDEBARCHILDWINDOW_HXX
#define INCLUDED_SFX2_SIDEBAR_SIDEBARCHILDWINDOW_HXX

#include <sfx2/childwin.hxx>


namespace sfx2 { namespace sidebar {

/** Outer container of the sidebar window.

    Has to be registered for every application via the
    RegisterChildWindow() method from the RegisterControllers() method
    of the applications DLL.
*/
class SFX2_DLLPUBLIC SidebarChildWindow : public SfxChildWindow
{
public:
    SidebarChildWindow(vcl::Window* pParent, sal_uInt16 nId,
                       SfxBindings* pBindings, SfxChildWinInfo* pInfo);

    SFX_DECL_CHILDWINDOW_WITHID(SidebarChildWindow);

    static sal_Int32 GetDefaultWidth(vcl::Window* pWindow);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
