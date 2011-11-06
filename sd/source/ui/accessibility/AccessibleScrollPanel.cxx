/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

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
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

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
