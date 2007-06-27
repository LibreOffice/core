/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleBrowseBoxHeaderBar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:18:12 $
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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERBAR_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXHEADERBAR_HXX

#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXTABLEBASE_HXX
#include "accessibility/extended/AccessibleBrowseBoxTableBase.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

// ============================================================================

namespace accessibility {

// ============================================================================

typedef ::cppu::ImplHelper1<
            ::com::sun::star::accessibility::XAccessibleSelection >
        AccessibleBrowseBoxHeaderBarImplHelper;

/** This class represents the accessible object of a header bar of a BrowseBox
    control (row or column header bar). This object supports the
    XAccessibleSelection interface. Selecting a child of this object selects
    complete rows or columns of the data table. */
class AccessibleBrowseBoxHeaderBar :
    public AccessibleBrowseBoxTableBase,
    public AccessibleBrowseBoxHeaderBarImplHelper
{
public:
    /** @param eObjType  One of the two allowed types BBTYPE_ROWHEADERBAR or
                         BBTYPE_COLUMNHEADERBAR. */
    AccessibleBrowseBoxHeaderBar(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        ::svt::AccessibleBrowseBoxObjType  eObjType );

protected:
    virtual ~AccessibleBrowseBoxHeaderBar();

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
    virtual ::rtl::OUString SAL_CALL
    getAccessibleRowDescription( sal_Int32 nRow )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The description text of the specified column. */
    virtual ::rtl::OUString SAL_CALL
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
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  An unique implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( ::com::sun::star::uno::RuntimeException );

protected:
    // internal virtual methods -----------------------------------------------

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

    /** Returns the specified row or column. Uses one of the parameters,
        depending on object type.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified column/row. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    implGetChild( sal_Int32 nRow, sal_uInt16 nColumnPos );

    /** @attention  This method requires locked mutex's and a living object.
        @return  The absolute child index from the index of selected children.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified index is invalid. */
    sal_Int32 implGetChildIndexFromSelectedIndex( sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );

    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified row/column index (depending on type) is invalid. */
    void ensureIsValidHeaderIndex( sal_Int32 nIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );
};

// inlines --------------------------------------------------------------------

inline sal_Bool AccessibleBrowseBoxHeaderBar::isRowBar() const
{
    return getType() == ::svt::BBTYPE_ROWHEADERBAR;
}

inline sal_Bool AccessibleBrowseBoxHeaderBar::isColumnBar() const
{
    return getType() == ::svt::BBTYPE_COLUMNHEADERBAR;
}

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif

