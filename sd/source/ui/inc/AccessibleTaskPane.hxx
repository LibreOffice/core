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

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_TASK_PANE_HXX
#define SD_ACCESSIBILITY_ACCESSIBLE_TASK_PANE_HXX

#include "AccessibleTreeNode.hxx"

namespace sd { namespace toolpanel {
class ToolPanel;
} }


namespace accessibility {

/** Make the task pane as implemented by sd::toolpanel::ToolPanel
    accessible.
    This derived class exists in order to disable the FOCUSED state.
*/
class AccessibleTaskPane
    : public AccessibleTreeNode
{
public:
    AccessibleTaskPane (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible> & rxParent,
        const ::rtl::OUString& rsName,
        const ::rtl::OUString& rsDescription,
        ::sd::toolpanel::ToolPanel& rTaskPane);
    ~AccessibleTaskPane (void);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual void UpdateStateSet (void);
};

} // end of namespace accessibility

#endif
