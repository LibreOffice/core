/*************************************************************************
 *
 *  $RCSfile: AccessibleTableBase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2002-01-22 14:17:14 $
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

#ifndef _SVTOOLS_ACCESSIBLE_CONTEXT_BASE_HXX
#include <svtools/AccessibleContextBase.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleTable.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETVIEW_HDL_
#include <com/sun/star/sheet/XSpreadsheetView.hdl>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif

/** @descr
        This base class provides an implementation of the
        <code>AccessibleTable</code> service.
*/
class ScAccessibleTableBase
    :   public  ::drafts::com::sun::star::accessibility::XAccessibleTable,
        public  SvAccessibleContextBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleTableBase (
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        const com::sun::star::uno::Reference <
        com::sun::star::sheet::XSpreadsheetView >& rxSheetView,
        const com::sun::star::table::CellRangeAddress& rRange);
    virtual ~ScAccessibleTableBase ();

    //=====  XInterface  ======================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                    const ::com::sun::star::uno::Type & rType )
                    throw(::com::sun::star::uno::RuntimeException);

    /** Increase the reference count.
    */
    virtual void SAL_CALL
                acquire (void)
                    throw ();

    /** Decrease the reference count.
    */
    virtual void SAL_CALL
                release (void)
                    throw ();

    //=====  XAccessibleTable  ================================================

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
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the description text of the specified column in the table.
    virtual ::rtl::OUString SAL_CALL
                getAccessibleColumnDescription( sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    /** Returns the number of rows occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of rows the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32 SAL_CALL
                getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    /** Returns the number of columns occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of columns the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32 SAL_CALL
                getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the row headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleRowHeaders(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the column headers as an AccessibleTable.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
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
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the Accessible at a specified row and column in the table.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the caption for the table.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleCaption(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the summary description of the table.
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleSummary(  )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleExtendedTable  ========================================

    /// Returns the index of the cell on the given position.
    virtual sal_Int32 SAL_CALL
                getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the row number of an index in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleRow( sal_Int32 nChildIndex )
                    throw (::com::sun::star::uno::RuntimeException);

    /// Returns the column number of an index in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleColumn( sal_Int32 nChildIndex )
                    throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    // is overloaded to calculate this on demand
    virtual long SAL_CALL
        getAccessibleChildCount (void);

    /// Return the specified child or NULL if index is invalid.
    // is overloaded to calculate this on demand
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (long nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

protected:
    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

public:
    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    // perhaps sometimes to be implemented
    virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that are
        the AccessibleContext and AccessibleTable service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ===================================================

    /** Returns a sequence of all supported interfaces.
    */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceName  ====================================================

    /** Returns the name of the service that can be used to create another
        object like this one.
    */
    virtual ::rtl::OUString SAL_CALL
        getServiceName (void)
        throw (::com::sun::star::uno::RuntimeException);


protected:

    /** UNO API representation of the view of the current document.
    */
    com::sun::star::uno::Reference < com::sun::star::sheet::XSpreadsheetView > mxSheetView;

    /** UNO API representation of the current sheet.
    */
    com::sun::star::uno::Reference < com::sun::star::sheet::XSpreadsheet > mxSheet;

    /// contains the range of the table, because it could be a subrange of the complete table
    com::sun::star::table::CellRangeAddress maRange;
};


#endif
