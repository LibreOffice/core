/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleTaskPane.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:27:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "AccessibleTaskPane.hxx"

#include "AccessibleTreeNode.hxx"

#include "taskpane/ToolPanel.hxx"
#include "taskpane/ControlContainer.hxx"
#include "taskpane/TitledControl.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

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
