/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleBrowseBoxHeaderCell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:18:25 $
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
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX

#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXBASE_HXX
#include "accessibility/extended/AccessibleBrowseBoxBase.hxx"
#endif

namespace accessibility
{
    class AccessibleBrowseBoxHeaderCell : public BrowseBoxAccessibleElement
    {
        sal_Int32   m_nColumnRowId;
    public:
        AccessibleBrowseBoxHeaderCell(sal_Int32 _nColumnRowId,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                                          ::svt::IAccessibleTableProvider&                  _rBrowseBox,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                                          ::svt::AccessibleBrowseBoxObjType  _eObjType);
        /** @return  The count of visible children. */
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw ( ::com::sun::star::uno::RuntimeException );

        /** @return  The XAccessible interface of the specified child. */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
            getAccessibleChild( sal_Int32 nChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );

        /** @return  The index of this object among the parent's children. */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw ( ::com::sun::star::uno::RuntimeException );

        /** Grabs the focus to the BrowseBox. */
        virtual void SAL_CALL grabFocus() throw ( ::com::sun::star::uno::RuntimeException );

        inline sal_Bool isRowBarCell() const
        {
            return getType() == ::svt::BBTYPE_ROWHEADERCELL;
        }

        /** @return
                The name of this class.
        */
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( ::com::sun::star::uno::RuntimeException );

        /** Creates a new AccessibleStateSetHelper and fills it with states of the
            current object.
            @return
                A filled AccessibleStateSetHelper.
        */
        ::utl::AccessibleStateSetHelper* implCreateStateSetHelper();

    protected:
        virtual Rectangle implGetBoundingBox();

        virtual Rectangle implGetBoundingBoxOnScreen();
    };
}

#endif // ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERCELL_HXX

