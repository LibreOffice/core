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
        const OUString& rsName,
        const OUString& rsDescription);
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
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::sd::toolpanel::ScrollPanel& GetScrollPanel (void) const;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
