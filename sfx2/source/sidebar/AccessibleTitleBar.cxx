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

#include <sfx2/sidebar/AccessibleTitleBar.hxx>
#include <sfx2/sidebar/Accessible.hxx>
#include <sfx2/sidebar/TitleBar.hxx>

#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <unotools/accessiblestatesethelper.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

Reference<accessibility::XAccessible> AccessibleTitleBar::Create (TitleBar& rTitleBar)
{
    rTitleBar.GetComponentInterface();
    VCLXWindow* pWindow = rTitleBar.GetWindowPeer();
    if (pWindow != nullptr)
        return new Accessible(new AccessibleTitleBar(pWindow));
    else
        return nullptr;
}

AccessibleTitleBar::AccessibleTitleBar (VCLXWindow* pWindow)
    : VCLXAccessibleComponent(pWindow)
{
}

AccessibleTitleBar::~AccessibleTitleBar()
{
}

void AccessibleTitleBar::FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet)
{
    VCLXAccessibleComponent::FillAccessibleStateSet(rStateSet);
    rStateSet.AddState(accessibility::AccessibleStateType::FOCUSABLE);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
