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


#include "AccessibleScrollPanel.hxx"

#include "taskpane/ScrollPanel.hxx"
#include "taskpane/ControlContainer.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::sd::toolpanel;

namespace accessibility {

AccessibleScrollPanel::AccessibleScrollPanel (
    ::sd::toolpanel::ScrollPanel& rScrollPanel,
    const OUString& rsName,
    const OUString& rsDescription)
    : AccessibleTreeNode(
        rScrollPanel,
        rsName,
        rsDescription,
        AccessibleRole::PANEL)
{
}




AccessibleScrollPanel::~AccessibleScrollPanel (void)
{
}




//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
    AccessibleScrollPanel::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    sal_Int32 nChildCount (mrTreeNode.GetControlContainer().GetControlCount());
    if (GetScrollPanel().IsVerticalScrollBarVisible())
        ++nChildCount;
    if (GetScrollPanel().IsHorizontalScrollBarVisible())
        ++nChildCount;

    return nChildCount;
}




Reference<XAccessible> SAL_CALL
    AccessibleScrollPanel::getAccessibleChild (sal_Int32 nIndex)
    throw (lang::IndexOutOfBoundsException,
        RuntimeException)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    Reference<XAccessible> xChild;

    ScrollPanel& rPanel (GetScrollPanel());

    sal_uInt32 nControlCount (mrTreeNode.GetControlContainer().GetControlCount());

    // The children of this accessible object include the tree node children
    // and the two scroll bars (when they are visible).
    if (nIndex < 0)
        throw lang::IndexOutOfBoundsException();
    else if ((sal_uInt32)nIndex < nControlCount)
        xChild = AccessibleTreeNode::getAccessibleChild(nIndex);
    else if ((sal_uInt32)nIndex == nControlCount)
    {
        if (rPanel.IsVerticalScrollBarVisible())
            xChild = rPanel.GetVerticalScrollBar().GetAccessible();
        else if (rPanel.IsHorizontalScrollBarVisible())
            xChild = rPanel.GetHorizontalScrollBar().GetAccessible();
    }
    else if ((sal_uInt32)nIndex == nControlCount+1)
    {
        if (rPanel.IsVerticalScrollBarVisible() && rPanel.IsHorizontalScrollBarVisible())
            xChild = rPanel.GetHorizontalScrollBar().GetAccessible();
    }
    else
        throw lang::IndexOutOfBoundsException();

    return xChild;
}




//=====  XServiceInfo  ========================================================

OUString SAL_CALL
    AccessibleScrollPanel::getImplementationName (void)
    throw (RuntimeException)
{
    return OUString("AccessibleScrollPanel");
}




ScrollPanel& AccessibleScrollPanel::GetScrollPanel (void) const
{
    return static_cast<ScrollPanel&>(mrTreeNode);
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
