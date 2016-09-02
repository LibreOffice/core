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

#include "BigToolBox.hxx"
#include <sfx2/sidebar/ControllerFactory.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/viewfrm.hxx>

#include <vcl/builderfactory.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/miscopt.hxx>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <framework/addonsoptions.hxx>

namespace sfx2 { namespace notebookbar {

BigToolBox::BigToolBox(vcl::Window* pParent)
    : SidebarToolBox(pParent)
{
    SvtMiscOptions aMiscOptions;
    aMiscOptions.RemoveListenerLink(LINK(this, SidebarToolBox, ChangedIconSizeHandler));

    SetToolboxButtonSize(ToolBoxButtonSize::Large);
    SetButtonType(ButtonType::SYMBOLTEXT);
    SetToolBoxTextPosition(ToolBoxTextPosition::Bottom);
}

VCL_BUILDER_FACTORY(BigToolBox)

} } // end of namespace sfx2::notebookbar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
