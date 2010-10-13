/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "AccessibleScrollPanel.hxx"

#include "taskpane/ScrollPanel.hxx"
#include "taskpane/ControlContainer.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::sd::toolpanel;

namespace accessibility {

AccessibleScrollPanel::AccessibleScrollPanel (
    ::sd::toolpanel::ScrollPanel& rScrollPanel,
    const ::rtl::OUString& rsName,
    const ::rtl::OUString& rsDescription)
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleScrollPanel"));
}




ScrollPanel& AccessibleScrollPanel::GetScrollPanel (void) const
{
    return static_cast<ScrollPanel&>(mrTreeNode);
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
