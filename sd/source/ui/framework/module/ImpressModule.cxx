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

#include "framework/ImpressModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include "ViewTabBarModule.hxx"
#include "CenterViewFocusModule.hxx"
#include "SlideSorterModule.hxx"
#include "ToolPanelModule.hxx"
#include "ToolBarModule.hxx"
#include "ShellStackGuard.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace framework {

void ImpressModule::Initialize (Reference<frame::XController>& rxController)
{
    new CenterViewFocusModule(rxController);
    new ViewTabBarModule(
        rxController,
        FrameworkHelper::CreateResourceId(
            FrameworkHelper::msViewTabBarURL,
            FrameworkHelper::msCenterPaneURL));
    new SlideSorterModule(
        rxController,
        FrameworkHelper::msLeftImpressPaneURL);
    new ToolPanelModule(
        rxController,
        FrameworkHelper::msSidebarViewURL);
    new ToolBarModule(rxController);
    new ShellStackGuard(rxController);
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
