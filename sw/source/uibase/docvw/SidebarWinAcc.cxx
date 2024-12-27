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

#include "SidebarWinAcc.hxx"
#include <AnnotationWin.hxx>

#include <viewsh.hxx>
#include <accmap.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

namespace sw::sidebarwindows
{
SidebarWinAccessible::SidebarWinAccessible(sw::annotation::SwAnnotationWin& rSidebarWin,
                                           SwViewShell& rViewShell,
                                           const SwAnnotationItem& rSidebarItem)
    : ImplInheritanceHelper(&rSidebarWin)
    , mrViewShell(rViewShell)
    , mpAnchorFrame(rSidebarItem.maLayoutInfo.mpAnchorFrame)
{
    rSidebarWin.SetAccessibleRole(css::accessibility::AccessibleRole::COMMENT);
}

SidebarWinAccessible::~SidebarWinAccessible() {}

void SidebarWinAccessible::ChangeSidebarItem(const SwAnnotationItem& rSidebarItem)
{
    SolarMutexGuard aGuard;

    mpAnchorFrame = rSidebarItem.maLayoutInfo.mpAnchorFrame;
}

css::uno::Reference<css::accessibility::XAccessibleContext>
SidebarWinAccessible::getAccessibleContext()
{
    ensureAlive();

    return this;
}

css::uno::Reference<css::accessibility::XAccessible> SidebarWinAccessible::getAccessibleParent()
{
    SolarMutexGuard aGuard;

    css::uno::Reference<css::accessibility::XAccessible> xAccParent;

    if (mpAnchorFrame && mrViewShell.GetAccessibleMap())
    {
        xAccParent = mrViewShell.GetAccessibleMap()->GetContext(mpAnchorFrame, false);
    }

    return xAccParent;
}

sal_Int64 SAL_CALL SidebarWinAccessible::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;

    sal_Int64 nIndex(-1);

    if (mpAnchorFrame && GetWindow() && mrViewShell.GetAccessibleMap())
    {
        nIndex = mrViewShell.GetAccessibleMap()->GetChildIndex(*mpAnchorFrame, *GetWindow());
    }

    return nIndex;
}

} // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
