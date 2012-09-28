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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOXTABLE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOXTABLE_HXX

#include "AccessibleBrowseBoxTable.hxx"
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SvHeaderTabListBox;

// ============================================================================

namespace accessibility {

typedef ::cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleSelection >
            AccessibleTabListBoxTableImplHelper;

class AccessibleTabListBoxTable : public AccessibleBrowseBoxTable, public AccessibleTabListBoxTableImplHelper
{
private:
    SvHeaderTabListBox*     m_pTabListBox;

    void                    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    DECL_LINK(              WindowEventListener, VclSimpleEvent* );

    // helpers ----------------------------------------------------------------

    /** Throws an exception, if nIndex is not a valid child index. */
    void ensureValidIndex( sal_Int32 _nIndex ) const
        SAL_THROW( ( ::com::sun::star::lang::IndexOutOfBoundsException ) );

    /** Returns true, if the specified row is selected. */
    sal_Bool implIsRowSelected( sal_Int32 _nRow ) const;
    /** Selects the specified row. */
    void implSelectRow( sal_Int32 _nRow, sal_Bool _bSelect );

    /** Returns the count of rows in the table. */
    sal_Int32 implGetRowCount() const;
    /** Returns the total column count in the table. */
    sal_Int32 implGetColumnCount() const;
    /** Returns the count of selected rows in the table. */
    sal_Int32 implGetSelRowCount() const;
    /** Returns the total cell count in the table (including header). */
    inline sal_Int32 implGetCellCount() const { return implGetRowCount() * implGetColumnCount(); }

    /** Returns the row index from cell index. */
    inline sal_Int32 implGetRow( sal_Int32 _nIndex ) const { return _nIndex / implGetColumnCount(); }
    /** Returns the column index from cell index. */
    inline sal_Int32 implGetColumn( sal_Int32 _nIndex ) const { return _nIndex % implGetColumnCount(); }
    /** Returns the absolute row index of the nSelRow-th selected row. */
    sal_Int32 implGetSelRow( sal_Int32 _nSelRow ) const;
    /** Returns the child index from cell position. */
    inline sal_Int32 implGetIndex( sal_Int32 _nRow, sal_Int32 _nColumn ) const { return _nRow * implGetColumnCount() + _nColumn; }

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBoxTable(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

protected:
    /** dtor() */
    virtual ~AccessibleTabListBoxTable();

public:
    // XInterface
    DECLARE_XINTERFACE( )

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
};

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif // ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
