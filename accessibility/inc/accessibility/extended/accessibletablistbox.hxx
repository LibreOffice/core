/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletablistbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:21:15 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOX_HXX
#include "AccessibleBrowseBox.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#ifndef _SVTOOLS_ACCESSIBLETABLEPROVIDER_HXX
#include <svtools/accessibletableprovider.hxx>
#endif

class SvHeaderTabListBox;

// ============================================================================

namespace accessibility {

class AccessibleBrowseBoxTable;

typedef ::cppu::ImplHelper1  <   ::com::sun::star::accessibility::XAccessible
                            >   AccessibleTabListBox_Base;

/** !!! */
class AccessibleTabListBox
                :public AccessibleBrowseBox
                ,public AccessibleTabListBox_Base
                ,public ::svt::IAccessibleTabListBox
{
private:
    SvHeaderTabListBox*         m_pTabListBox;

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBox(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

public:
    // XInterface
    DECLARE_XINTERFACE( )
    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The XAccessible interface of the specified child. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    // XAccessibleContext
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException );

    // IAccessibleTabListBox
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself()
    {
        return this;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType )
    {
        return AccessibleBrowseBox::getHeaderBar( _eObjType );
    }

protected:
    /** dtor() */
    virtual ~AccessibleTabListBox();

    /** This method creates and returns an accessible table.
        @return  An AccessibleBrowseBoxTable. */
    virtual AccessibleBrowseBoxTable*   createAccessibleTable();
};

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif // ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

