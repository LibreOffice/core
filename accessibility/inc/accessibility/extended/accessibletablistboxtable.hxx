/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletablistboxtable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:21:31 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOXTABLE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOXTABLE_HXX

#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXTABLE_HXX
#include "AccessibleBrowseBoxTable.hxx"
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

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

