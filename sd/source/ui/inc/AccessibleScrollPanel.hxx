/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleScrollPanel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:59:58 $
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
