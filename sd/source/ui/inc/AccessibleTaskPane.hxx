/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleTaskPane.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:00:58 $
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
