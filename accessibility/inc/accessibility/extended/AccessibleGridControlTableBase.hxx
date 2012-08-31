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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLTABLEBASE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLTABLEBASE_HXX

#include "accessibility/extended/AccessibleGridControlBase.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>

// ============================================================================

namespace accessibility {

typedef ::cppu::ImplHelper1<
            ::com::sun::star::accessibility::XAccessibleTable >
        AccessibleGridControlTableImplHelper;

/** The Grid Control accessible table objects inherit from this base class. It
    implements basic functionality for the XAccessibleTable interface.
    Grid COntrol table objects are: the data table, the column header bar and the
    row header bar. */
class AccessibleGridControlTableBase :
    public GridControlAccessibleElement,
    public AccessibleGridControlTableImplHelper
{
public:
    /** Constructor sets specified name and description.
        @param rxParent  XAccessible interface of the parent object.
        @param rTable  The Table control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text. */
    AccessibleGridControlTableBase(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >& rxParent,
    ::svt::table::IAccessibleTable& rTable,
    ::svt::table::AccessibleTableControlObjType  eObjType );

protected:
    virtual ~AccessibleGridControlTableBase();

public:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The role of this object (a table). */
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw ( ::com::sun::star::uno::RuntimeException );

    /*  Derived classes have to implement:
        -   getAccessibleChild,
        -   getAccessibleIndexInParent. */

    // XAccessibleComponent ---------------------------------------------------

    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus,
        -   getAccessibleKeyBinding. */

    // XAccessibleTable -------------------------------------------------------

    /** @return  The number of used rows in the table (0 = empty table). */
    virtual sal_Int32 SAL_CALL getAccessibleRowCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The number of used columns in the table (0 = empty table). */
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The row extent of the specified cell (always 1). */
    virtual sal_Int32 SAL_CALL
    getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The column extent of the specified cell (always 1). */
    virtual sal_Int32 SAL_CALL
    getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The caption cell of the table (not supported). */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleCaption()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The summary object of the table (not supported). */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleSummary()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The child index of the specified cell. */
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The row index of the specified child cell. */
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The column index of the specified child cell. */
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /*  Derived classes have to implement:
        -   getAccessibleRowDescription,
        -   getAccessibleColumnDescription,
        -   getAccessibleRowHeaders,
        -   getAccessibleColumnHeaders,
        -   getSelectedAccessibleRows,
        -   getSelectedAccessibleColumns,
        -   isAccessibleRowSelected,
        -   isAccessibleColumnSelected,
        -   getAccessibleCellAt,
        -   isAccessibleSelected. */

    // XInterface -------------------------------------------------------------

    /** Queries for a new interface. */
    ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& rType )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** Aquires the object (calls acquire() on base class). */
    virtual void SAL_CALL acquire() throw ();

    /** Releases the object (calls release() on base class). */
    virtual void SAL_CALL release() throw ();

    // XTypeProvider ----------------------------------------------------------

    /** @return  A sequence of possible types (received from base classes). */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  An unique implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( ::com::sun::star::uno::RuntimeException );

protected:
    // internal helper methods ------------------------------------------------

//    /** @attention  This method requires locked mutex's and a living object.
//        @param nColumn
//            the position of the column in the Accessible world
//        @return
//            the position of the column in VCL the Accessible world
//    */
//    sal_uInt16 implToVCLColumnPos( sal_Int32 nColumn ) const;

    /** @attention  This method requires locked mutex's and a living object.
        @return  The number of cells of the table. */
    sal_Int32 implGetChildCount() const;

    /** @attention  This method requires locked mutex's and a living object.
        @return  The row index of the specified cell index. */
    sal_Int32 implGetRow( sal_Int32 nChildIndex ) const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The column index of the specified cell index. */
    sal_Int32 implGetColumn( sal_Int32 nChildIndex ) const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The child index of the specified cell address. */
    sal_Int32 implGetChildIndex( sal_Int32 nRow, sal_Int32 nColumn ) const;

    /** Fills a sequence with sorted indexes of completely selected rows.
        @attention  This method requires locked mutex's and a living object.
        @param rSeq  Out-parameter that takes the sorted row index list. */
    void implGetSelectedRows( ::com::sun::star::uno::Sequence< sal_Int32 >& rSeq );
    /** Fills a sequence with sorted indexes of completely selected columns.
        @attention  This method requires locked mutex's and a living object.
        @param rSeq  Out-parameter that takes the sorted column index list. */
    //void implGetSelectedColumns( ::com::sun::star::uno::Sequence< sal_Int32 >& rSeq );

    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified row index is invalid. */
    void ensureIsValidRow( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified column index is invalid. */
    void ensureIsValidColumn( sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified cell address is invalid. */
    void ensureIsValidAddress( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified child index is invalid. */
    void ensureIsValidIndex( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
};

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif // ACCESSIBILITY_EXT_ACCESSIBILEGRIDCONTROLTABLEBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
