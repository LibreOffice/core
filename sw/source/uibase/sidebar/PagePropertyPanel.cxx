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

#include <sal/config.h>

#include <cstdlib>

#include "PagePropertyPanel.hxx"

#include <svx/sidebar/PopupContainer.hxx>

#include <swtypes.hxx>
#include <cmdid.h>

#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>

#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>

namespace sw { namespace sidebar {

PagePropertyPanel::PagePropertyPanel(
            vcl::Window* pParent,
            const css::uno::Reference< css::frame::XFrame>& rxFrame,
            SfxBindings* )
    : PanelLayout(pParent, "PagePropertyPanel", "modules/swriter/ui/sidebarpage.ui", rxFrame)
{
}

PagePropertyPanel::~PagePropertyPanel()
{
    disposeOnce();
}

void PagePropertyPanel::dispose()
{
    PanelLayout::dispose();
}

} } // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
