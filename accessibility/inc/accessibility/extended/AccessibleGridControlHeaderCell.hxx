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
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADERCELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADERCELL_HXX

#include "accessibility/extended/AccessibleGridControlTableCell.hxx"

namespace accessibility
{
    class AccessibleGridControlHeaderCell : public AccessibleGridControlCell, public ::com::sun::star::accessibility::XAccessible
    {
        sal_Int32   m_nColumnRowId;
    public:
        AccessibleGridControlHeaderCell(sal_Int32 _nColumnRowId,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
                        ::svt::table::IAccessibleTable& _rTable,
                        ::svt::table::AccessibleTableControlObjType  _eObjType);
        /** @return  The count of visible children. */
        virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw ( ::com::sun::star::uno::RuntimeException );

        /** @return  The XAccessible interface of the specified child. */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
            getAccessibleChild( sal_Int32 nChildIndex ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException );

        /** @return  The index of this object among the parent's children. */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw ( ::com::sun::star::uno::RuntimeException );

        /** Grabs the focus to the GridControl. */
        virtual void SAL_CALL grabFocus() throw ( ::com::sun::star::uno::RuntimeException );

        // XInterface -------------------------------------------------------------

        /** Queries for a new interface. */
        ::com::sun::star::uno::Any SAL_CALL queryInterface(
                const ::com::sun::star::uno::Type& rType )
            throw ( ::com::sun::star::uno::RuntimeException );

        /** Aquires the object (calls acquire() on base class). */
        virtual void SAL_CALL acquire() throw ();

        /** Releases the object (calls release() on base class). */
        virtual void SAL_CALL release() throw ();
        // XAccessible ------------------------------------------------------------

        /** @return  The XAccessibleContext interface of this object. */
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
        getAccessibleContext()
            throw ( ::com::sun::star::uno::RuntimeException );
        //-------------------------------------------------------------------------
        inline sal_Bool isRowBarCell() const
        {
            return getType() == ::svt::table::TCTYPE_ROWHEADERCELL;
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
    private:
        ::rtl::OUString m_sHeaderName;
    };
}

#endif // ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADERCELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
