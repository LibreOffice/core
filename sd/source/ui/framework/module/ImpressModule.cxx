/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

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
    ToolPanelModule::Initialize(rxController);
    new ToolBarModule(rxController);
    new ShellStackGuard(rxController);
}


} } // end of namespace sd::framework
