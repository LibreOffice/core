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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADER_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEGRIDCONTROLHEADER_HXX


#include "accessibility/extended/AccessibleGridControlHeaderCell.hxx"
#include "accessibility/extended/AccessibleGridControlTableBase.hxx"

// ============================================================================

namespace accessibility {

// ============================================================================

/** This class represents the accessible object of a header bar of a Grid Control
   (row or column header bar). This object supports the
    XAccessibleSelection interface. Selecting a child of this object selects
    complete rows or columns of the data table. */
class AccessibleGridControlHeader : public AccessibleGridControlTableBase
{
public:
    /** @param eObjType  One of the two allowed types TCTYPE_ROWHEADERBAR or
                         TCTYPE_COLUMNHEADERBAR. */
    AccessibleGridControlHeader(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
            ::svt::table::IAccessibleTable& rTable,
            ::svt::table::AccessibleTableControlObjType  eObjType );

protected:
    virtual ~AccessibleGridControlHeader();

public:
    // XAccessibleContext -----------------------------------------------------

    /** @return
        The XAccessible interface of the specified child.
    */
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
    virtual OUString SAL_CALL
    getAccessibleRowDescription( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The description text of the specified column. */
    virtual OUString SAL_CALL
    getAccessibleColumnDescription( sal_Int32 nColumn )
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

    // XServiceInfo -----------------------------------------------------------

    /** @return  The name of this class. */
    virtual OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  An unique implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( ::com::sun::star::uno::RuntimeException );

protected:
    // internal virtual methods -----------------------------------------------
    /** @attention  This method requires locked mutex's and a living object.
        @return  The absolute child index from the index of selected children.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified index is invalid. */
    //sal_Int32 implGetChildIndexFromSelectedIndex( sal_Int32 nSelectedChildIndex )
    //    throw ( ::com::sun::star::lang::IndexOutOfBoundsException );

    /** Returns the specified row or column. Uses one of the parameters,
        depending on object type.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified column/row. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    implGetChild( sal_Int32 nRow, sal_uInt32 nColumnPos );
    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox();
   /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen();

    /** @attention  This method requires locked mutex's and a living object.
        @return  The count of used rows. */
    virtual sal_Int32 implGetRowCount() const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The count of used columns. */
    virtual sal_Int32 implGetColumnCount() const;

    // internal helper methods ------------------------------------------------

    /** @return  <TRUE/>, if the objects is a header bar for rows. */
    inline sal_Bool isRowBar() const;
    /** @return  <TRUE/>, if the objects is a header bar for columns. */
    inline sal_Bool isColumnBar() const;
};

// inlines --------------------------------------------------------------------

inline sal_Bool AccessibleGridControlHeader::isRowBar() const
{
    return getType() == ::svt::table::TCTYPE_ROWHEADERBAR;
}

inline sal_Bool AccessibleGridControlHeader::isColumnBar() const
{
    return getType() == ::svt::table::TCTYPE_COLUMNHEADERBAR;
}

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif // ACCESSIBILITY_EXT_ACCESSIBILEGRIDCONTROLHEADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
