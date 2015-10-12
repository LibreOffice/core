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


#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXTABLE_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXTABLE_HXX

#include "accessibility/extended/AccessibleBrowseBoxTableBase.hxx"



namespace accessibility {



/** This class represents the accessible object of the data table of a
    BrowseBox control. */
class AccessibleBrowseBoxTable : public AccessibleBrowseBoxTableBase
{
    friend class AccessibleBrowseBox;   // to create header bars

public:
    AccessibleBrowseBoxTable(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider& rBrowseBox );

protected:
    virtual ~AccessibleBrowseBoxTable();

public:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The XAccessible interface of the specified child. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleComponent ---------------------------------------------------

    /** @return  The accessible child rendered under the given point. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const css::awt::Point& rPoint )
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** Grabs the focus to (the current cell of) the data table. */
    virtual void SAL_CALL grabFocus()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleTable -------------------------------------------------------

    /** @return  The description text of the specified row. */
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return  The description text of the specified column. */
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return  The XAccessibleTable interface of the row header bar. */
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
    getAccessibleRowHeaders()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The XAccessibleTable interface of the column header bar. */
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
    getAccessibleColumnHeaders()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  An index list of completely selected rows. */
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
    getSelectedAccessibleRows()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  An index list of completely selected columns. */
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
    getSelectedAccessibleColumns()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  <TRUE/>, if the specified row is completely selected. */
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return  <TRUE/>, if the specified column is completely selected. */
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return The XAccessible interface of the cell object at the specified
                cell position. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return  <TRUE/>, if the specified cell is selected. */
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo -----------------------------------------------------------

    /** @return  The name of this class. */
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;

protected:
    // internal virtual methods -----------------------------------------------

    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox() override;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen() override;

    // internal helper methods ------------------------------------------------

    /** @attention  This method requires a locked mutex.
        @return  The XAccessibleTable interface of the specified header bar. */
    css::uno::Reference< css::accessibility::XAccessibleTable >
    implGetHeaderBar( sal_Int32 nChildIndex )
        throw ( css::uno::RuntimeException );
};



} // namespace accessibility



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
