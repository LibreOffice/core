/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibletablistboxtable.hxx,v $
 * $Revision: 1.3 $
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
    /** Throws an exception, if the specified position is invalid. */
    void ensureValidPosition( sal_Int32 _nRow, sal_Int32 _nColumn ) const
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
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
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

