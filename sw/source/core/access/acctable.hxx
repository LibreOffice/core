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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCTABLE_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCTABLE_HXX

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <vector>
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
        public css::accessibility::XAccessibleTable,
        public css::accessibility::XAccessibleSelection,
        public css::accessibility::XAccessibleTableSelection,
        public SwClient
{
    SwAccessibleTableData_Impl *mpTableData;    // the table's data, prot by Sol-Mutex
    OUString sDesc;
    const SwSelBoxes *GetSelBoxes() const;

    void FireTableChangeEvent( const SwAccessibleTableData_Impl& rTableData );

    /** get the SwTableBox* for the given child */
    const SwTableBox* GetTableBox( sal_Int32 ) const;

    bool IsChildSelected( sal_Int32 nChildIndex ) const;

    sal_Int32 GetIndexOfSelectedChild( sal_Int32 nSelectedChildIndex ) const;

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets MULTISELECTABLE(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) override;

    virtual ~SwAccessibleTable();

    // #i77106#
    inline void SetDesc( const OUString& sNewDesc )
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
    bool HasTableData() const { return (mpTableData != 0); }

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwAccessibleTable( SwAccessibleMap* pInitMap, const SwTabFrm* pTableFrm );

    // XInterface

    // (XInterface methods need to be implemented to disambigouate
    // between those inherited through SwAcessibleContext and
    // XAccessibleTable).

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleTable

    virtual sal_Int32 SAL_CALL getAccessibleRowCount()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleRowDescription(
            sal_Int32 nRow )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription(
            sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt(
            sal_Int32 nRow, sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt(
               sal_Int32 nRow, sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleRowHeaders(  )
           throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleColumnHeaders(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleRows(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
        getSelectedAccessibleColumns(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleCaption(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleSummary(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected(
            sal_Int32 nRow, sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex(
            sal_Int32 nRow, sal_Int32 nColumn )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;
    // XAccessibleTableSelection
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;
    // XServiceInfo

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // C++ interface

    // The object has been moved by the layout
    virtual void InvalidatePosOrSize( const SwRect& rOldBox ) override;

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( bool bRecursive = false ) override;

    virtual void DisposeChild( const sw::access::SwAccessibleChild& rFrmOrObj,
                               bool bRecursive ) override;
    virtual void InvalidateChildPosOrSize( const sw::access::SwAccessibleChild& rFrmOrObj,
                                           const SwRect& rFrm ) override;

    // XAccessibleSelection

    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;

    // index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    // XAccessibleComponent
    sal_Int32 SAL_CALL getBackground()
        throw (css::uno::RuntimeException, std::exception) override;
    typedef std::vector< ::std::pair<SwAccessibleContext*,
        css::uno::WeakReference<css::accessibility::XAccessible> > > Cells_t;
    Cells_t m_vecCellAdd;
    Cells_t m_vecCellRemove;
    void FireSelectionEvent( );
    void AddSelectionCell(SwAccessibleContext*, bool bAddOrRemove);
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

    virtual SwAccessibleTableData_Impl* CreateNewTableData() override;
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwAccessibleTableColHeaders( SwAccessibleMap *pMap, const SwTabFrm *pTabFrm );

    // XInterface

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { SwAccessibleContext::release(); };

    // XAccessibleContext

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
                css::lang::IndexOutOfBoundsException, std::exception) override;

    // XAccessibleTable

    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleRowHeaders(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::accessibility::XAccessibleTable >
        SAL_CALL getAccessibleColumnHeaders(  )
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
