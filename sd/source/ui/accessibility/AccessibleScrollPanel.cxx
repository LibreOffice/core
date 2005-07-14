/*************************************************************************
 *
 *  $RCSfile: AccessibleScrollPanel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:10:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "AccessibleScrollPanel.hxx"

#include "taskpane/ScrollPanel.hxx"
#include "taskpane/ControlContainer.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
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
