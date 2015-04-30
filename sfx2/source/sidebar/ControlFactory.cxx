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
#include <sfx2/sidebar/ControlFactory.hxx>

#include <sfx2/sidebar/MenuButton.hxx>
#include <sfx2/sidebar/TabItem.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <vcl/toolbox.hxx>

namespace sfx2 { namespace sidebar {

VclPtr<CheckBox> ControlFactory::CreateMenuButton (vcl::Window* pParentWindow)
{
    return VclPtr<CheckBox>(new MenuButton(pParentWindow), SAL_NO_ACQUIRE);
}

VclPtr<ImageRadioButton> ControlFactory::CreateTabItem (vcl::Window* pParentWindow)
{
    return VclPtr<ImageRadioButton>(new TabItem(pParentWindow), SAL_NO_ACQUIRE);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
