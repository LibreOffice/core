/*************************************************************************
 *
 *  $RCSfile: AccessibleTableBase.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:16:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _SC_ACCESSIBLETABLEBASE_HXX
#define _SC_ACCESSIBLETABLEBASE_HXX

#ifndef _SC_ACCESSIBLECONTEXTBASE_HXX
#include "AccessibleContextBase.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

class ScTabViewShell;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleTable</code> service.
*/

typedef cppu::ImplHelper2< ::com::sun::star::accessibility::XAccessibleTable,
                    ::com::sun::star::accessibility::XAccessibleSelection>
                    ScAccessibleTableBaseImpl;

class ScAccessibleTableBase :
            public ScAccessibleContextBase,
            public ScAccessibleTableBaseImpl
{
public:
    //=====  internal  ========================================================
    ScAccessibleTableBase(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScRange& rRange);
protected:
    virtual ~ScAccessibleTableBase();
public:

     virtual void SAL_CALL disposing();

    ///=====  XInterface  =====================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ();

    virtual void SAL_CALL release() throw ();

    ///=====  XAccessibleTable  ================================================

    /// Returns the number of rows in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleRowCount(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the number of columns in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleColumnCount(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the description of the specified row in the table.
    virtual ::rtl::OUString SAL_CALL
                getAccessibleRowDescription( sal_Int32 nRow )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the description text of the specified column in the table.
    virtual ::rtl::OUString SAL_CALL
                getAccessibleColumnDescription( sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /** Returns the number of rows occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of rows the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32 SAL_CALL
                getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /** Returns the number of columns occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of columns the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32 SAL_CALL
                getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the row headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleRowHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the column headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleColumnHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the selected rows in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleRows(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the selected columns in a table.
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleColumns(  )
                    throw (::com::sun::star::uno::RuntimeException);

    ///  Returns a boolean value indicating whether the specified row is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleRowSelected( sal_Int32 nRow )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the Accessible at a specified row and column in the table.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                            ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the caption for the table.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCaption(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the summary description of the table.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleSummary(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    ///=====  XAccessibleExtendedTable  ========================================

    /// Returns the index of the cell on the given position.
    virtual sal_Int32 SAL_CALL
                getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the row number of an index in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleRow( sal_Int32 nChildIndex )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Returns the column number of an index in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleColumn( sal_Int32 nChildIndex )
                    throw (::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::lang::IndexOutOfBoundsException);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    // is overloaded to calculate this on demand
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount(void)
                    throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    // is overloaded to calculate this on demand
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

protected:
    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

public:
    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    // perhaps sometimes to be implemented
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        selectAllAccessible(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException);

    ///=====  XServiceInfo  ===================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XTypeProvider  ===================================================

    /// returns the possible types
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// contains the range of the table, because it could be a subrange of the complete table
    ScRange maRange;

    ScDocument* mpDoc;

    void CommitTableModelChange(sal_Int32 nStartRow, sal_Int32 nStartCol, sal_Int32 nEndRow, sal_Int32 nEndCol, sal_uInt16 nId);
};


#endif
