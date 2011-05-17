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

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_SCROLL_PANEL_HXX
#define SD_ACCESSIBILITY_ACCESSIBLE_SCROLL_PANEL_HXX

#include "AccessibleTreeNode.hxx"

namespace sd { namespace toolpanel {
class ScrollPanel;
} }


namespace accessibility {

/** This derived class handles the accessible children different from its
    base class.  It adds the scroll bars as children when they are visible.
*/
class AccessibleScrollPanel
    : public AccessibleTreeNode
{
public:
    AccessibleScrollPanel (
        ::sd::toolpanel::ScrollPanel& rScrollPanel,
        const ::rtl::OUString& rsName,
        const ::rtl::OUString& rsDescription);
    ~AccessibleScrollPanel (void);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or throw exception.
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::sd::toolpanel::ScrollPanel& GetScrollPanel (void) const;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
