/*************************************************************************
 *
 *  $RCSfile: acctable.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:13:45 $
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
#ifndef _ACCTABLE_HXX
#define _ACCTABLE_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#ifndef _ACCCONTEXT_HXX
#include "acccontext.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

class SwTabFrm;
class SwAccessibleTableData_Impl;
class SwAccessibleTableEventList_Impl;
class SwTableBox;

class SwAccessibleTable :
        public SwAccessibleContext,
        public ::com::sun::star::accessibility::XAccessibleTable,
        public ::com::sun::star::accessibility::XAccessibleSelection,
        public SwClient
{
    SwAccessibleTableData_Impl *mpTableData;    // the table's data, prot by Sol-Mutex
    ::rtl::OUString sDesc;
    const SwSelBoxes *GetSelBoxes() const;

    void FireTableChangeEvent( const SwAccessibleTableData_Impl& rTableData );

    /** get the SwTableBox* for the given child */
    const SwTableBox* GetTableBox( sal_Int32 ) const;

    sal_Bool IsChildSelected( sal_Int32 nChildIndex ) const;

    sal_Int32 GetIndexOfSelectedChild( sal_Int32 nSelectedChildIndex ) const;

protected:

    // Set states for getAccessibleStateSet.
    // This drived class additinaly sets MULTISELECTABLE(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual ~SwAccessibleTable();

    // force update of table data
    void UpdateTableData();

    // remove the current table data
    void ClearTableData();

    // get table data, update if necessary
    inline SwAccessibleTableData_Impl& GetTableData();

    // Is table data evailable?
    sal_Bool HasTableData() const { return (mpTableData != 0); }

public:

    SwAccessibleTable( SwAccessibleMap *pMap, const SwTabFrm *pTableFrm );

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    //=====  XInterface  ======================================================

    // (XInterface methods need to be implemented to disambigouate
    // between those inherited through SwAcessibleContext and
    // XAccessibleTable).

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire(  ) throw ()
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw ()
        { SwAccessibleContext::release(); };

    //====== XTypeProvider ====================================================
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XAccessibleTable  ================================================

    virtual sal_Int32 SAL_CALL getAccessibleRowCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleRowDescription(
            sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleColumnDescription(
            sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt(
            sal_Int32 nRow, sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt(
               sal_Int32 nRow, sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleRowHeaders(  )
           throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleColumnHeaders(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleRows(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleColumns(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleCaption(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleSummary(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleSelected(
            sal_Int32 nRow, sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndex(
            sal_Int32 nRow, sal_Int32 nColumn )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //===== C++ interface ======================================================

    // The object has been moved by the layout
    virtual void InvalidatePosOrSize( const SwRect& rOldBox );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False );

    virtual void DisposeChild( const SwFrmOrObj& rFrmOrObj, sal_Bool bRecursive );
    virtual void InvalidateChildPosOrSize( const SwFrmOrObj& rFrmOrObj,
                                        const SwRect& rFrm );

    //=====  XAccessibleSelection  ============================================

    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL selectAllAccessible(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

};

inline SwAccessibleTableData_Impl& SwAccessibleTable::GetTableData()
{
    if( !mpTableData )
        UpdateTableData();
    return *mpTableData;
}

#endif

