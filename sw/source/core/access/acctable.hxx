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

#ifndef _ACCTABLE_HXX
#define _ACCTABLE_HXX

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

#include <acccontext.hxx>

class SwTabFrm;
class SwAccessibleTableData_Impl;
class SwTableBox;
class SwSelBoxes;

namespace sw { namespace access {
    class SwAccessibleChild;
} }

class SwAccessibleTable :
        public SwAccessibleContext,
        public ::com::sun::star::accessibility::XAccessibleTable,
        public ::com::sun::star::accessibility::XAccessibleSelection,
        public SwClient
{
    SwAccessibleTableData_Impl *mpTableData;    // the table's data, prot by Sol-Mutex
    OUString sDesc;
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

    // #i77106#
    inline void SetDesc( OUString sNewDesc )
    {
        sDesc = sNewDesc;
    }

    virtual SwAccessibleTableData_Impl* CreateNewTableData(); // #i77106#

    // force update of table data
    void UpdateTableData();

    // remove the current table data
    void ClearTableData();

    // get table data, update if necessary
    inline SwAccessibleTableData_Impl& GetTableData();

    // Is table data evailable?
    sal_Bool HasTableData() const { return (mpTableData != 0); }

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwAccessibleTable( SwAccessibleMap* pInitMap, const SwTabFrm* pTableFrm );

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
    virtual OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XAccessibleTable  ================================================

    virtual sal_Int32 SAL_CALL getAccessibleRowCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleRowDescription(
            sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleColumnDescription(
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
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //===== C++ interface ======================================================

    // The object has been moved by the layout
    virtual void InvalidatePosOrSize( const SwRect& rOldBox );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False );

    virtual void DisposeChild( const sw::access::SwAccessibleChild& rFrmOrObj,
                               sal_Bool bRecursive );
    virtual void InvalidateChildPosOrSize( const sw::access::SwAccessibleChild& rFrmOrObj,
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

    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    // index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

};

inline SwAccessibleTableData_Impl& SwAccessibleTable::GetTableData()
{
    if( !mpTableData )
        UpdateTableData();
    return *mpTableData;
}

// #i77106# - subclass to represent table column headers
class SwAccessibleTableColHeaders : public SwAccessibleTable
{
protected:
    virtual ~SwAccessibleTableColHeaders()
    {}

    virtual SwAccessibleTableData_Impl* CreateNewTableData();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwAccessibleTableColHeaders( SwAccessibleMap *pMap, const SwTabFrm *pTabFrm );

    //=====  XInterface  ======================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire(  ) throw ()
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw ()
        { SwAccessibleContext::release(); };

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    //=====  XAccessibleTable  ================================================

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleRowHeaders(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleColumnHeaders(  )
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
