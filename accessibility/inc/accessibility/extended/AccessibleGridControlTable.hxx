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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLTABLE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLTABLE_HXX

#include "accessibility/extended/AccessibleGridControlTableBase.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <accessibility/extended/AccessibleGridControlTableCell.hxx>

// ============================================================================

namespace accessibility {

// ============================================================================
typedef ::cppu::ImplHelper1<
            ::com::sun::star::accessibility::XAccessibleSelection >
        AccessibleGridControlTableImplHelper1;
/** This class represents the accessible object of the data table of a
    Grid control. */
class AccessibleGridControlTable : public AccessibleGridControlTableBase,
                                public AccessibleGridControlTableImplHelper1
{
public:
    AccessibleGridControlTable(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
            ::svt::table::IAccessibleTable& rTable,
            ::svt::table::AccessibleTableControlObjType _eType);

protected:
    virtual ~AccessibleGridControlTable();
private:
    std::vector< AccessibleGridControlTableCell* > m_pCellVector;
    std::vector< com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible> > m_pAccessCellVector;
public:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The XAccessible interface of the specified child. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw ( ::com::sun::star::uno::RuntimeException );

    // XAccessibleComponent ---------------------------------------------------

    /** @return  The accessible child rendered under the given point. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** Grabs the focus to (the current cell of) the data table. */
    virtual void SAL_CALL grabFocus()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The key bindings associated with this object. */
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding()
        throw ( ::com::sun::star::uno::RuntimeException );

    // XAccessibleTable -------------------------------------------------------

    /** @return  The description text of the specified row. */
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The description text of the specified column. */
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The XAccessibleTable interface of the row header bar. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
    getAccessibleRowHeaders()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The XAccessibleTable interface of the column header bar. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
    getAccessibleColumnHeaders()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  An index list of completely selected rows. */
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
    getSelectedAccessibleRows()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  An index list of completely selected columns. */
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
    getSelectedAccessibleColumns()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the specified row is completely selected. */
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the specified column is completely selected. */
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return The XAccessible interface of the cell object at the specified
                cell position. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the specified cell is selected. */
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    // XAccessibleSelection ---------------------------------------------------

    /** Selects the specified child (row or column of the table). */
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the specified child (row/column) is selected. */
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** Clears the complete selection. */
    virtual void SAL_CALL clearAccessibleSelection()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** Selects all children or first, if multiselection is not supported. */
    virtual void SAL_CALL selectAllAccessibleChildren()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The number of selected rows/columns. */
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The specified selected row/column. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** Removes the specified row/column from the selection. */
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
     // XInterface -------------------------------------------------------------

    /** Queries for a new interface. */
    ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& rType )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** Aquires the object (calls acquire() on base class). */
    virtual void SAL_CALL acquire() throw ();

    /** Releases the object (calls release() on base class). */
    virtual void SAL_CALL release() throw ();
    // XServiceInfo -----------------------------------------------------------

    /** @return  The name of this class. */
    virtual OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException );

    /**@return m_pCellVector*/
    std::vector< AccessibleGridControlTableCell* >& getCellVector();
    /**@return m_xAccessCellVector*/
    std::vector< com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible > >& getAccessibleCellVector();

protected:
    // internal virtual methods -----------------------------------------------

    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox();
    ///** @attention  This method requires locked mutex's and a living object.
    //    @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen();


    //// internal helper methods ------------------------------------------------
    ///** @attention  This method requires a locked mutex.
    //    @return  The XAccessibleTable interface of the specified header bar. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleTable >
    implGetHeaderBar( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::uno::RuntimeException );
};

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif // ACCESSIBILITY_EXT_ACCESSIBILEGRIDCONTROLTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
