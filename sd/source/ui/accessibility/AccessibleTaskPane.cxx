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

#include "AccessibleTaskPane.hxx"

#include "AccessibleTreeNode.hxx"

#include "taskpane/ToolPanel.hxx"
#include "taskpane/ControlContainer.hxx"
#include "taskpane/TitledControl.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

namespace accessibility {

AccessibleTaskPane::AccessibleTaskPane (
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> & rxParent,
    const OUString& rsName,
    const OUString& rsDescription,
    ::sd::toolpanel::ToolPanel& rTaskPane)
    : AccessibleTreeNode(
        rxParent,
        rTaskPane,
        rsName,
        rsDescription,
        AccessibleRole::PANEL)
{
}




AccessibleTaskPane::~AccessibleTaskPane (void)
{
}




//=====  XServiceInfo  ========================================================

OUString SAL_CALL
    AccessibleTaskPane::getImplementationName (void)
    throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleTaskPane"));
}




void AccessibleTaskPane::UpdateStateSet (void)
{
    // The task pane itself can not be focused.
    UpdateState(AccessibleStateType::FOCUSED, false);
}


} // end of namespace accessibility
